#!/usr/bin/env bash
# =============================================================================
# cleanup-multi-agent.sh
# =============================================================================
# Cleans up all multi-agent session state:
#   - Stops running agent-tab-watcher.sh processes
#   - Removes .agent-signals/ contents (signal files, trigger, pid, last-line)
#   - Removes .agent-info from all known worktrees
#   - Removes ALL non-main git worktrees (via git worktree list --porcelain, not just discovered)
#   - Runs git worktree prune to clean up stale refs
#   - Resets communication-log.md to a blank template (does NOT delete it)
#   - Removes launcher state (.launcher-last-line)
#
# Worktree discovery (for watcher/signals/.agent-info cleanup):
#   1. 🟢 Online agents in communication-log.md (worktree= in Session Info)
#   2. .agent-worktrees/ directory (Approach 1 worktrees)
#   3. Current project root itself (single-worktree setup)
#
# Git worktree removal is independent of discovery — always removes every
# non-main worktree registered in git, including stale unregistered tabs.
#
# Usage:
#   ./cleanup-multi-agent.sh              Full cleanup (interactive confirm)
#   ./cleanup-multi-agent.sh --yes        Full cleanup (no confirm)
#   ./cleanup-multi-agent.sh --signals    Clear signals only (keep .agent-info + comm-log)
#   ./cleanup-multi-agent.sh --log        Reset comm-log only
#   ./cleanup-multi-agent.sh --dry-run    Show what would be removed without doing it
#   ./cleanup-multi-agent.sh --help       Show this help
#
# =============================================================================

set -euo pipefail

# ---------------------------------------------------------------------------
# Resolve project root
# ---------------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(git -C "${SCRIPT_DIR}" rev-parse --show-toplevel 2>/dev/null || pwd)"
COMM_LOG="${PROJECT_ROOT}/agent_workspace/communication-log.md"
AGENT_WORKSPACE="${PROJECT_ROOT}/agent_workspace"
WORKTREES_DIR="${PROJECT_ROOT}/.agent-worktrees"

# ---------------------------------------------------------------------------
# Colors
# ---------------------------------------------------------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

# ---------------------------------------------------------------------------
# Flags
# ---------------------------------------------------------------------------
DRY_RUN=0
AUTO_YES=0
SIGNALS_ONLY=0
LOG_ONLY=0

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
say()     { echo -e "$*"; }
info()    { echo -e "  ${GREEN}✓${NC} $*"; }
warn()    { echo -e "  ${YELLOW}!${NC} $*"; }
skip()    { echo -e "  ${CYAN}–${NC} $*"; }
dryrun()  { echo -e "  ${BLUE}[dry-run]${NC} $*"; }
section() { echo -e "\n${BOLD}${BLUE}$*${NC}"; }

run() {
    if [[ "${DRY_RUN}" -eq 1 ]]; then
        dryrun "$*"
    else
        eval "$*"
    fi
}

# ---------------------------------------------------------------------------
# Discover all worktree roots that have agent state
# ---------------------------------------------------------------------------
discover_worktrees() {
    local found=()

    # 1. From communication-log.md — extract worktree= from Session Info column
    if [[ -f "${COMM_LOG}" ]]; then
        local in_table=0
        while IFS= read -r line; do
            if [[ "${line}" =~ "## Registered Agents" ]]; then
                in_table=1; continue
            fi
            if [[ "${in_table}" -eq 1 ]] && [[ "${line}" =~ ^##[[:space:]] ]]; then
                in_table=0
            fi
            if [[ "${in_table}" -eq 1 ]] && [[ "${line}" =~ \| ]]; then
                local session_info
                session_info=$(echo "${line}" | awk -F'|' '{print $6}' | xargs 2>/dev/null || true)
                local wt
                wt=$(echo "${session_info}" | grep -o 'worktree=[^ ;|]*' | cut -d= -f2 || true)
                if [[ -n "${wt}" ]] && [[ -d "${wt}" ]]; then
                    found+=("${wt}")
                fi
            fi
        done < "${COMM_LOG}"
    fi

    # 2. From .agent-worktrees/ directory (Approach 1)
    if [[ -d "${WORKTREES_DIR}" ]]; then
        while IFS= read -r wt; do
            [[ -d "${wt}" ]] && found+=("${wt}")
        done < <(find "${WORKTREES_DIR}" -mindepth 1 -maxdepth 1 -type d 2>/dev/null)
    fi

    # 3. Project root itself (single-worktree / main repo agent)
    if [[ -d "${PROJECT_ROOT}/.agent-info" ]] || [[ -d "${PROJECT_ROOT}/.agent-signals" ]]; then
        found+=("${PROJECT_ROOT}")
    fi

    # Deduplicate
    local seen=()
    local unique=()
    for wt in "${found[@]+"${found[@]}"}"; do
        local already=0
        for s in "${seen[@]+"${seen[@]}"}"; do
            [[ "${s}" == "${wt}" ]] && already=1 && break
        done
        if [[ "${already}" -eq 0 ]]; then
            seen+=("${wt}")
            unique+=("${wt}")
        fi
    done

    printf '%s\n' "${unique[@]+"${unique[@]}"}"
}

# ---------------------------------------------------------------------------
# Stop watcher process for a worktree
# ---------------------------------------------------------------------------
stop_watcher() {
    local wt="$1"
    local pid_file="${wt}/.agent-signals/watcher.pid"

    if [[ -f "${pid_file}" ]]; then
        local pid
        pid=$(cat "${pid_file}")
        if kill -0 "${pid}" 2>/dev/null; then
            run "kill '${pid}' 2>/dev/null || true"
            info "Stopped watcher PID ${pid} in $(basename "${wt}")"
        else
            skip "Watcher PID ${pid} already stopped in $(basename "${wt}")"
        fi
        run "rm -f '${pid_file}'"
    else
        skip "No watcher.pid in $(basename "${wt}")/.agent-signals/"
    fi
}

# ---------------------------------------------------------------------------
# Clear .agent-signals/ for a worktree
# ---------------------------------------------------------------------------
clear_signals() {
    local wt="$1"
    local sdir="${wt}/.agent-signals"

    if [[ -d "${sdir}" ]]; then
        local count
        count=$(find "${sdir}" -name "*.signal" 2>/dev/null | wc -l | tr -d ' ')
        run "find '${sdir}' -name '*.signal' -delete 2>/dev/null || true"
        run "rm -f '${sdir}/.trigger' '${sdir}/.listening' '${sdir}/.watcher-last-line'"
        run "rm -f '${sdir}/watcher.log'"
        info "Cleared ${count} signal(s) + state files from $(basename "${wt}")/.agent-signals/"
    else
        skip "No .agent-signals/ in $(basename "${wt}")"
    fi
}

# ---------------------------------------------------------------------------
# Remove .agent-info for a worktree (new dir structure: .agent-info/<wt-name>/agent-info)
# ---------------------------------------------------------------------------
remove_agent_info() {
    local wt="$1"
    local wt_name
    wt_name=$(basename "${wt}")
    local info_dir="${wt}/.agent-info"
    local info_file="${info_dir}/${wt_name}/agent-info"

    if [[ -f "${info_file}" ]]; then
        local name
        name=$(grep '^name=' "${info_file}" 2>/dev/null | cut -d= -f2 || echo "unknown")
        run "rm -rf '${info_dir}'"
        info "Removed .agent-info dir (${name}) from ${wt_name}"
    elif [[ -d "${info_dir}" ]]; then
        run "rm -rf '${info_dir}'"
        info "Removed stale .agent-info dir from ${wt_name}"
    else
        skip "No .agent-info in $(basename "${wt}")"
    fi
}

# ---------------------------------------------------------------------------
# Reset communication-log.md to blank template
# ---------------------------------------------------------------------------
reset_comm_log() {
    if [[ ! -f "${COMM_LOG}" ]]; then
        skip "No communication-log.md found at ${COMM_LOG}"
        return
    fi

    local template
    template='# Agent Communication Log

> **Purpose**: Central coordination point for all agents in the workspace. Agents register here on startup and post messages to communicate with each other.
> **Rules**: Append-only. Never edit or delete previous entries (except Status column updates for register/logout).

---

## Registered Agents

| Agent Name | Role | Registered At | Status | Session Info |
|------------|------|---------------|--------|--------------|

---

## Messages

<!-- Agents append messages below this line -->'

    if [[ "${DRY_RUN}" -eq 1 ]]; then
        dryrun "Reset ${COMM_LOG} to blank template"
    else
        printf '%s\n' "${template}" > "${COMM_LOG}"
        info "Reset communication-log.md to blank template"
    fi
}

# ---------------------------------------------------------------------------
# Remove git worktrees for agent worktrees (skip main repo root)
# Always removes ALL non-main worktrees, not just discovered ones.
# ---------------------------------------------------------------------------
remove_git_worktrees() {
    local main_root
    main_root=$(git -C "${PROJECT_ROOT}" rev-parse --show-toplevel 2>/dev/null || echo "${PROJECT_ROOT}")
    local real_main
    real_main=$(cd "${main_root}" 2>/dev/null && pwd || echo "")

    # Collect all registered worktree paths from git
    local all_worktrees=()
    while IFS= read -r wt_path; do
        [[ -n "${wt_path}" ]] && all_worktrees+=("${wt_path}")
    done < <(git -C "${PROJECT_ROOT}" worktree list --porcelain 2>/dev/null | grep '^worktree ' | sed 's/^worktree //')

    for wt_path in "${all_worktrees[@]}"; do
        local real_wt
        real_wt=$(cd "${wt_path}" 2>/dev/null && pwd || echo "${wt_path}")

        if [[ "${real_wt}" == "${real_main}" ]]; then
            skip "Skipping main repo root: ${wt_path}"
            continue
        fi

        run "git -C '${PROJECT_ROOT}' worktree remove --force '${wt_path}' 2>/dev/null || true"
        info "Removed git worktree: ${wt_path}"
    done

    run "git -C '${PROJECT_ROOT}' worktree prune"
    info "Pruned stale git worktree refs"
}

# ---------------------------------------------------------------------------
# Remove launcher state
# ---------------------------------------------------------------------------
remove_launcher_state() {
    local last_line="${AGENT_WORKSPACE}/.launcher-last-line"
    if [[ -f "${last_line}" ]]; then
        run "rm -f '${last_line}'"
        info "Removed launcher state (.launcher-last-line)"
    else
        skip "No launcher state to remove"
    fi
}

# ---------------------------------------------------------------------------
# Confirm prompt
# ---------------------------------------------------------------------------
confirm() {
    local prompt="$1"
    if [[ "${AUTO_YES}" -eq 1 ]]; then
        return 0
    fi
    echo -e "\n${YELLOW}${prompt}${NC} [y/N] \c"
    read -r answer
    [[ "${answer}" =~ ^[Yy]$ ]]
}

# ---------------------------------------------------------------------------
# Help
# ---------------------------------------------------------------------------
cmd_help() {
    cat <<EOF
${CYAN}${BOLD}cleanup-multi-agent.sh${NC} — Clean up all multi-agent session state

${YELLOW}${BOLD}Usage:${NC}
  $(basename "$0")              Full cleanup (interactive confirm)
  $(basename "$0") --yes        Full cleanup, skip confirmation
  $(basename "$0") --signals    Clear .agent-signals/ only (keep .agent-info + comm-log)
  $(basename "$0") --log        Reset communication-log.md only
  $(basename "$0") --dry-run    Show what would be removed, make no changes
  $(basename "$0") --help       Show this help

${YELLOW}${BOLD}What full cleanup does:${NC}
  1. Stops running agent-tab-watcher.sh processes (via watcher.pid)
  2. Clears .agent-signals/ in every discovered worktree
     (signal files, .trigger, .listening, watcher.log, .watcher-last-line)
  3. Removes .agent-info from every discovered worktree
  4. Removes ALL non-main git worktrees (reads git worktree list --porcelain,
     removes every entry that is not the main repo root — including stale tabs
     that were never registered in the comm-log)
  5. Runs git worktree prune to clean up stale refs
  6. Resets communication-log.md to a blank template (does NOT delete the file)
  7. Removes launcher state (agent_workspace/.launcher-last-line)

${YELLOW}${BOLD}Worktree discovery (steps 1-3 only):${NC}
  - worktree= entries in communication-log.md Registered Agents table
  - Directories under .agent-worktrees/ (Approach 1)
  - Project root itself (if .agent-info or .agent-signals/ present)

${YELLOW}${BOLD}Examples:${NC}
  $(basename "$0") --dry-run     Preview what would be cleaned
  $(basename "$0") --yes         Clean everything without prompting
  $(basename "$0") --signals     Only clear pending signals (keep identity)
  $(basename "$0") --log         Only reset the comm-log
EOF
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
main() {
    # Parse args
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --dry-run)    DRY_RUN=1 ;;
            --yes|-y)     AUTO_YES=1 ;;
            --signals)    SIGNALS_ONLY=1 ;;
            --log)        LOG_ONLY=1 ;;
            --help|-h)    cmd_help; exit 0 ;;
            *)
                echo -e "${RED}Error:${NC} Unknown argument: $1"
                echo "Run: $(basename "$0") --help"
                exit 1
                ;;
        esac
        shift
    done

    say "${BLUE}${BOLD}═══════════════════════════════════════════════════════${NC}"
    say "${BLUE}${BOLD}  Multi-Agent Cleanup${NC}"
    say "${BLUE}  Project: ${PROJECT_ROOT}${NC}"
    [[ "${DRY_RUN}" -eq 1 ]] && say "${YELLOW}  Mode: DRY RUN — no changes will be made${NC}"
    say "${BLUE}${BOLD}═══════════════════════════════════════════════════════${NC}"

    # --log only
    if [[ "${LOG_ONLY}" -eq 1 ]]; then
        section "Resetting communication-log.md"
        if confirm "Reset communication-log.md to blank template?"; then
            reset_comm_log
        fi
        say "\n${GREEN}Done.${NC}"
        return
    fi

    # Discover worktrees
    section "Discovering agent worktrees..."
    local worktrees=()
    while IFS= read -r wt; do
        [[ -n "${wt}" ]] && worktrees+=("${wt}")
    done < <(discover_worktrees)

    if [[ ${#worktrees[@]} -eq 0 ]]; then
        warn "No agent worktrees found. Nothing to clean."
        say ""
        return
    fi

    say ""
    say "  Found ${#worktrees[@]} worktree(s):"
    for wt in "${worktrees[@]}"; do
        local name_tag=""
        local _wt_name _info_file
        _wt_name=$(basename "${wt}")
        _info_file="${wt}/.agent-info/${_wt_name}/agent-info"
        if [[ -f "${_info_file}" ]]; then
            name_tag=" ($(grep '^name=' "${_info_file}" 2>/dev/null | cut -d= -f2 || echo "?"), $(grep '^role=' "${_info_file}" 2>/dev/null | cut -d= -f2 || echo "?"))"
        fi
        say "    ${CYAN}●${NC} ${wt}${name_tag}"
    done

    # --signals only
    if [[ "${SIGNALS_ONLY}" -eq 1 ]]; then
        say ""
        if confirm "Clear .agent-signals/ in all ${#worktrees[@]} worktree(s)?"; then
            section "Stopping watchers"
            for wt in "${worktrees[@]}"; do stop_watcher "${wt}"; done
            section "Clearing signals"
            for wt in "${worktrees[@]}"; do clear_signals "${wt}"; done
            remove_launcher_state
        fi
        say "\n${GREEN}Done.${NC}"
        return
    fi

    # Full cleanup
    say ""
    if ! confirm "Full cleanup: stop watchers, clear signals, remove .agent-info, reset comm-log?"; then
        say "${YELLOW}Aborted.${NC}"
        exit 0
    fi

    section "Stopping watchers"
    for wt in "${worktrees[@]}"; do stop_watcher "${wt}"; done

    section "Clearing .agent-signals/"
    for wt in "${worktrees[@]}"; do clear_signals "${wt}"; done

    section "Removing .agent-info"
    for wt in "${worktrees[@]}"; do remove_agent_info "${wt}"; done

    section "Removing agent_workspace symlinks"
    for wt in "${worktrees[@]}"; do
        local link="${wt}/agent_workspace"
        if [[ -L "${link}" ]]; then
            run "rm -f '${link}'"
            info "Removed agent_workspace symlink from $(basename "${wt}")"
        fi
    done

    section "Removing git worktrees"
    remove_git_worktrees

    section "Resetting communication-log.md"
    reset_comm_log

    section "Removing launcher state"
    remove_launcher_state

    say "\n${GREEN}${BOLD}✅ Cleanup complete.${NC}"
    say "   All agents stopped. Comm-log reset. Ready for a fresh session."
    say ""
    say "   To start again: run ${CYAN}/agent-01-coordination start${NC} in each Cascade tab."
}

main "$@"
