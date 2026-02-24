#!/usr/bin/env bash
# =============================================================================
# agent-worktree-watcher.sh
# =============================================================================
# External orchestrator script that watches the communication log for changes
# and signals agents in their respective git worktrees.
#
# This script runs OUTSIDE of Cascade (in a regular terminal) and bridges the
# gap between Cascade sessions that cannot notify each other directly.
#
# Architecture:
#   Main repo (communication-log.md) --fswatch--> this script --signal--> worktrees
#
# Each agent gets its own git worktree. When the communication log changes,
# this script parses the new entries and writes signal files into the
# appropriate agent worktree's .agent-signals/ directory.
#
# Prerequisites:
#   - fswatch (brew install fswatch) or inotifywait (Linux)
#   - git (for worktree management)
#   - jq (optional, for JSON signal payloads)
#
# Usage:
#   ./agent-worktree-watcher.sh [--project-root <path>] [--interval <seconds>]
#   ./agent-worktree-watcher.sh --setup <agent-name> <agent-role>
#   ./agent-worktree-watcher.sh --teardown <agent-name>
#   ./agent-worktree-watcher.sh --list
#   ./agent-worktree-watcher.sh --status
#
# =============================================================================

set -euo pipefail

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${PROJECT_ROOT:-$(git rev-parse --show-toplevel 2>/dev/null || pwd)}"
AGENT_WORKSPACE="${PROJECT_ROOT}/agent_workspace"
COMM_LOG="${AGENT_WORKSPACE}/communication-log.md"
WORKTREE_BASE="${PROJECT_ROOT}/.agent-worktrees"
SIGNAL_DIR_NAME=".agent-signals"
POLL_INTERVAL=2  # seconds between checks when fswatch is unavailable
LOG_FILE="${AGENT_WORKSPACE}/watcher.log"
PID_FILE="${AGENT_WORKSPACE}/watcher.pid"
LAST_LINE_FILE="${AGENT_WORKSPACE}/.watcher-last-line"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# ---------------------------------------------------------------------------
# Logging
# ---------------------------------------------------------------------------
log() {
    local level="$1"; shift
    local msg="$*"
    local ts
    ts="$(date '+%Y-%m-%dT%H:%M:%S')"
    echo -e "${ts} [${level}] ${msg}" | tee -a "${LOG_FILE}" 2>/dev/null || true
}

log_info()  { log "${GREEN}INFO${NC}" "$@"; }
log_warn()  { log "${YELLOW}WARN${NC}" "$@"; }
log_error() { log "${RED}ERROR${NC}" "$@"; }
log_debug() { [[ "${DEBUG:-0}" == "1" ]] && log "${CYAN}DEBUG${NC}" "$@" || true; }

# ---------------------------------------------------------------------------
# Utility: detect watcher tool
# ---------------------------------------------------------------------------
detect_watcher() {
    if command -v fswatch &>/dev/null; then
        echo "fswatch"
    elif command -v inotifywait &>/dev/null; then
        echo "inotifywait"
    else
        echo "poll"
    fi
}

# ---------------------------------------------------------------------------
# Utility: get all agent worktrees
# ---------------------------------------------------------------------------
get_agent_worktrees() {
    if [[ ! -d "${WORKTREE_BASE}" ]]; then
        return
    fi
    for d in "${WORKTREE_BASE}"/*/; do
        [[ -d "$d" ]] && basename "$d"
    done
}

# ---------------------------------------------------------------------------
# Utility: get agent info from worktree
# ---------------------------------------------------------------------------
get_agent_role() {
    local agent_name="$1"
    local info_file="${WORKTREE_BASE}/${agent_name}/.agent-info"
    if [[ -f "${info_file}" ]]; then
        grep '^role=' "${info_file}" | cut -d= -f2
    else
        echo "unknown"
    fi
}

get_agent_status() {
    local agent_name="$1"
    local signal_dir="${WORKTREE_BASE}/${agent_name}/${SIGNAL_DIR_NAME}"
    if [[ -f "${signal_dir}/.listening" ]]; then
        echo "listening"
    elif [[ -d "${signal_dir}" ]]; then
        echo "idle"
    else
        echo "offline"
    fi
}

# ---------------------------------------------------------------------------
# Command: setup — Create a git worktree for an agent
# ---------------------------------------------------------------------------
cmd_setup() {
    local agent_name="${1:?Usage: --setup <agent-name> <agent-role>}"
    local agent_role="${2:-general}"
    local branch_name="agent/${agent_name}"
    local worktree_path="${WORKTREE_BASE}/${agent_name}"

    if [[ -d "${worktree_path}" ]]; then
        log_warn "Worktree already exists for ${agent_name} at ${worktree_path}"
        echo -e "${YELLOW}Worktree already exists.${NC} Use --teardown first to recreate."
        return 1
    fi

    # Ensure base directory exists
    mkdir -p "${WORKTREE_BASE}"

    # Create a branch for this agent (from current HEAD)
    local current_branch
    current_branch="$(git -C "${PROJECT_ROOT}" rev-parse --abbrev-ref HEAD)"

    if git -C "${PROJECT_ROOT}" show-ref --verify --quiet "refs/heads/${branch_name}" 2>/dev/null; then
        log_info "Branch ${branch_name} already exists, reusing"
    else
        git -C "${PROJECT_ROOT}" branch "${branch_name}" "${current_branch}"
        log_info "Created branch ${branch_name} from ${current_branch}"
    fi

    # Create the worktree
    git -C "${PROJECT_ROOT}" worktree add "${worktree_path}" "${branch_name}"
    log_info "Created worktree at ${worktree_path}"

    # Create signal directory and agent info
    mkdir -p "${worktree_path}/${SIGNAL_DIR_NAME}"
    cat > "${worktree_path}/.agent-info" <<EOF
name=${agent_name}
role=${agent_role}
branch=${branch_name}
created=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
worktree=${worktree_path}
project_root=${PROJECT_ROOT}
EOF

    # Symlink agent_workspace so the agent can read/write the shared communication log
    if [[ ! -e "${worktree_path}/agent_workspace" ]]; then
        ln -s "${AGENT_WORKSPACE}" "${worktree_path}/agent_workspace"
        log_info "Symlinked agent_workspace into worktree"
    fi

    # Symlink docs so the agent can access shared artifacts
    if [[ -d "${PROJECT_ROOT}/docs" ]] && [[ ! -e "${worktree_path}/docs" ]]; then
        ln -s "${PROJECT_ROOT}/docs" "${worktree_path}/docs"
        log_info "Symlinked docs into worktree"
    fi

    echo -e "${GREEN}✅ Agent worktree created${NC}"
    echo "  Agent:     ${agent_name}"
    echo "  Role:      ${agent_role}"
    echo "  Branch:    ${branch_name}"
    echo "  Worktree:  ${worktree_path}"
    echo "  Signals:   ${worktree_path}/${SIGNAL_DIR_NAME}/"
    echo ""
    echo "Open this worktree in a new Windsurf window to start the agent:"
    echo "  windsurf ${worktree_path}"
    echo ""
    echo "Then run in that session:"
    echo "  /agent-01-coordination start"
}

# ---------------------------------------------------------------------------
# Command: teardown — Remove an agent's worktree
# ---------------------------------------------------------------------------
cmd_teardown() {
    local agent_name="${1:?Usage: --teardown <agent-name>}"
    local worktree_path="${WORKTREE_BASE}/${agent_name}"
    local branch_name="agent/${agent_name}"

    if [[ ! -d "${worktree_path}" ]]; then
        log_error "No worktree found for ${agent_name}"
        return 1
    fi

    # Remove worktree
    git -C "${PROJECT_ROOT}" worktree remove "${worktree_path}" --force 2>/dev/null || {
        log_warn "git worktree remove failed, removing directory manually"
        rm -rf "${worktree_path}"
        git -C "${PROJECT_ROOT}" worktree prune
    }

    # Optionally remove branch
    if git -C "${PROJECT_ROOT}" show-ref --verify --quiet "refs/heads/${branch_name}" 2>/dev/null; then
        git -C "${PROJECT_ROOT}" branch -D "${branch_name}" 2>/dev/null || true
        log_info "Deleted branch ${branch_name}"
    fi

    echo -e "${GREEN}✅ Agent worktree removed:${NC} ${agent_name}"
}

# ---------------------------------------------------------------------------
# Command: list — Show all agent worktrees
# ---------------------------------------------------------------------------
cmd_list() {
    echo -e "${BLUE}═══════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}  Agent Worktrees${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════${NC}"

    local agents
    agents=$(get_agent_worktrees)

    if [[ -z "${agents}" ]]; then
        echo "  No agent worktrees found."
        echo "  Create one with: ./agent-worktree-watcher.sh --setup <name> <role>"
        return
    fi

    printf "  %-20s %-15s %-12s %s\n" "AGENT" "ROLE" "STATUS" "PATH"
    printf "  %-20s %-15s %-12s %s\n" "─────" "────" "──────" "────"

    for agent in ${agents}; do
        local role status path
        role=$(get_agent_role "${agent}")
        status=$(get_agent_status "${agent}")
        path="${WORKTREE_BASE}/${agent}"

        local status_color="${NC}"
        case "${status}" in
            listening) status_color="${GREEN}" ;;
            idle)      status_color="${YELLOW}" ;;
            offline)   status_color="${RED}" ;;
        esac

        printf "  %-20s %-15s ${status_color}%-12s${NC} %s\n" \
            "${agent}" "${role}" "${status}" "${path}"
    done
    echo ""
}

# ---------------------------------------------------------------------------
# Command: status — Show watcher and agent status
# ---------------------------------------------------------------------------
cmd_status() {
    echo -e "${BLUE}═══════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}  Worktree Watcher Status${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════${NC}"

    # Watcher process
    if [[ -f "${PID_FILE}" ]]; then
        local pid
        pid=$(cat "${PID_FILE}")
        if kill -0 "${pid}" 2>/dev/null; then
            echo -e "  Watcher:  ${GREEN}Running${NC} (PID ${pid})"
        else
            echo -e "  Watcher:  ${RED}Stale PID${NC} (${pid} not running)"
        fi
    else
        echo -e "  Watcher:  ${RED}Not running${NC}"
    fi

    # Watcher method
    local method
    method=$(detect_watcher)
    echo -e "  Method:   ${method}"
    echo -e "  Log:      ${LOG_FILE}"
    echo -e "  Comm Log: ${COMM_LOG}"
    echo ""

    # Pending signals
    local total_signals=0
    for agent in $(get_agent_worktrees); do
        local signal_dir="${WORKTREE_BASE}/${agent}/${SIGNAL_DIR_NAME}"
        if [[ -d "${signal_dir}" ]]; then
            local count
            count=$(find "${signal_dir}" -name "*.signal" 2>/dev/null | wc -l | tr -d ' ')
            if [[ "${count}" -gt 0 ]]; then
                echo -e "  ${YELLOW}⚡ ${agent}:${NC} ${count} pending signal(s)"
                total_signals=$((total_signals + count))
            fi
        fi
    done
    if [[ "${total_signals}" -eq 0 ]]; then
        echo "  No pending signals."
    fi
    echo ""

    cmd_list
}

# ---------------------------------------------------------------------------
# Signal: write a signal file to an agent's worktree
# ---------------------------------------------------------------------------
write_signal() {
    local agent_name="$1"
    local signal_type="$2"  # DISTRIBUTE, HANDOFF, BLOCKER_RESOLVED, PHASE_ADVANCE
    local payload="$3"      # free-text or JSON
    local signal_dir="${WORKTREE_BASE}/${agent_name}/${SIGNAL_DIR_NAME}"

    if [[ ! -d "${signal_dir}" ]]; then
        log_warn "Signal dir missing for ${agent_name}, skipping"
        return 1
    fi

    local ts
    ts="$(date '+%Y%m%d-%H%M%S')"
    local signal_file="${signal_dir}/${ts}-${signal_type}.signal"

    cat > "${signal_file}" <<EOF
signal_type=${signal_type}
timestamp=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
source=worktree-watcher
payload=${payload}
EOF

    log_info "Signal sent: ${signal_type} → ${agent_name} (${signal_file})"

    # Also touch a trigger file that the agent's listener can watch
    touch "${signal_dir}/.trigger"
}

# ---------------------------------------------------------------------------
# Parse: extract new messages from communication log
# ---------------------------------------------------------------------------
parse_new_messages() {
    local last_line=0
    if [[ -f "${LAST_LINE_FILE}" ]]; then
        last_line=$(cat "${LAST_LINE_FILE}")
    fi

    if [[ ! -f "${COMM_LOG}" ]]; then
        return
    fi

    local total_lines
    total_lines=$(wc -l < "${COMM_LOG}" | tr -d ' ')

    if [[ "${total_lines}" -le "${last_line}" ]]; then
        return
    fi

    # Extract new lines
    local new_content
    new_content=$(tail -n +"$((last_line + 1))" "${COMM_LOG}")

    # Update last line marker
    echo "${total_lines}" > "${LAST_LINE_FILE}"

    # Parse for actionable messages
    parse_distribute_messages "${new_content}"
    parse_completed_messages "${new_content}"
    parse_blocked_messages "${new_content}"
    parse_handoff_messages "${new_content}"
}

# ---------------------------------------------------------------------------
# Parse: DISTRIBUTE messages → signal assigned agents
# ---------------------------------------------------------------------------
parse_distribute_messages() {
    local content="$1"

    # Look for DISTRIBUTE entries and extract assignments
    # Format: | **Assignments** | P1-T001 → agent-name, P1-T002 → agent-name |
    while IFS= read -r line; do
        if [[ "${line}" =~ \*\*Assignments\*\* ]]; then
            # Extract the assignments value
            local assignments
            assignments=$(echo "${line}" | sed 's/.*\*\*Assignments\*\*[[:space:]]*|[[:space:]]*//' | sed 's/[[:space:]]*|$//')

            log_debug "Found assignments: ${assignments}"

            # Parse each assignment: "TASK_ID → agent-name"
            IFS=',' read -ra pairs <<< "${assignments}"
            for pair in "${pairs[@]}"; do
                pair=$(echo "${pair}" | xargs)  # trim whitespace
                if [[ "${pair}" =~ →[[:space:]]*(.+) ]]; then
                    local target_agent
                    target_agent=$(echo "${BASH_REMATCH[1]}" | xargs)

                    # Check if this agent has a worktree
                    for wt_agent in $(get_agent_worktrees); do
                        # Match by exact name or by role
                        local wt_role
                        wt_role=$(get_agent_role "${wt_agent}")
                        if [[ "${target_agent}" == "${wt_agent}" ]] || \
                           [[ "${target_agent,,}" == *"${wt_role,,}"* ]]; then
                            write_signal "${wt_agent}" "DISTRIBUTE" "${pair}"
                        fi
                    done
                fi
            done
        fi
    done <<< "${content}"
}

# ---------------------------------------------------------------------------
# Parse: COMPLETED messages → signal agents waiting on handoffs
# ---------------------------------------------------------------------------
parse_completed_messages() {
    local content="$1"
    local in_completed=0
    local handoff_to=""

    while IFS= read -r line; do
        if [[ "${line}" =~ COMPLETED ]]; then
            in_completed=1
            handoff_to=""
        fi

        if [[ "${in_completed}" -eq 1 ]] && [[ "${line}" =~ \*\*Handoff\ To\*\* ]]; then
            handoff_to=$(echo "${line}" | sed 's/.*\*\*Handoff To\*\*[[:space:]]*|[[:space:]]*//' | sed 's/[[:space:]]*|$//')

            if [[ "${handoff_to}" != "None" ]] && [[ -n "${handoff_to}" ]]; then
                # Extract agent name (before any " — " description)
                local target
                target=$(echo "${handoff_to}" | sed 's/ — .*//' | xargs)

                for wt_agent in $(get_agent_worktrees); do
                    local wt_role
                    wt_role=$(get_agent_role "${wt_agent}")
                    if [[ "${target}" == "${wt_agent}" ]] || \
                       [[ "${target,,}" == *"${wt_role,,}"* ]]; then
                        write_signal "${wt_agent}" "HANDOFF" "Handoff from completed task: ${handoff_to}"
                    fi
                done
            fi
            in_completed=0
        fi
    done <<< "${content}"
}

# ---------------------------------------------------------------------------
# Parse: BLOCKED resolved — signal blocked agents when blocker clears
# ---------------------------------------------------------------------------
parse_blocked_messages() {
    local content="$1"
    # When a COMPLETED message references an artifact that was a blocker,
    # signal the blocked agent. This is a best-effort heuristic.

    while IFS= read -r line; do
        if [[ "${line}" =~ \*\*Blockers\*\*.*None ]]; then
            # A task completed with no blockers — check if any agent was
            # previously blocked waiting for this type of work
            # This is handled more precisely by the HANDOFF logic above
            :
        fi
    done <<< "${content}"
}

# ---------------------------------------------------------------------------
# Parse: HANDOFF messages → signal target agents
# ---------------------------------------------------------------------------
parse_handoff_messages() {
    local content="$1"

    while IFS= read -r line; do
        if [[ "${line}" =~ HANDOFF ]] && [[ "${line}" =~ "###" ]]; then
            # A dedicated HANDOFF message entry
            log_debug "Found HANDOFF message entry"
        fi
    done <<< "${content}"
}

# ---------------------------------------------------------------------------
# Watch loop: main event loop
# ---------------------------------------------------------------------------
watch_loop() {
    local method
    method=$(detect_watcher)

    log_info "Starting watcher (method: ${method})"
    log_info "Project root: ${PROJECT_ROOT}"
    log_info "Watching: ${COMM_LOG}"
    log_info "Worktree base: ${WORKTREE_BASE}"

    # Initialize last line tracker
    if [[ -f "${COMM_LOG}" ]]; then
        wc -l < "${COMM_LOG}" | tr -d ' ' > "${LAST_LINE_FILE}"
    else
        echo "0" > "${LAST_LINE_FILE}"
    fi

    echo -e "${GREEN}🔭 Worktree watcher started${NC}"
    echo "  Method:   ${method}"
    echo "  Watching: ${COMM_LOG}"
    echo "  PID:      $$"
    echo ""
    echo "Press Ctrl+C to stop."
    echo ""

    # Save PID
    echo "$$" > "${PID_FILE}"

    # Cleanup on exit
    trap cleanup EXIT INT TERM

    case "${method}" in
        fswatch)
            watch_with_fswatch
            ;;
        inotifywait)
            watch_with_inotifywait
            ;;
        poll)
            watch_with_poll
            ;;
    esac
}

watch_with_fswatch() {
    # Ensure agent_workspace exists
    mkdir -p "${AGENT_WORKSPACE}"

    fswatch -o "${COMM_LOG}" 2>/dev/null | while read -r _; do
        log_debug "Change detected in communication log"
        parse_new_messages
    done
}

watch_with_inotifywait() {
    mkdir -p "${AGENT_WORKSPACE}"

    while true; do
        inotifywait -q -e modify "${COMM_LOG}" 2>/dev/null
        log_debug "Change detected in communication log"
        parse_new_messages
    done
}

watch_with_poll() {
    log_warn "Neither fswatch nor inotifywait found. Falling back to polling (${POLL_INTERVAL}s)."
    log_warn "Install fswatch for better performance: brew install fswatch"

    local last_checksum=""

    while true; do
        if [[ -f "${COMM_LOG}" ]]; then
            local current_checksum
            current_checksum=$(md5 -q "${COMM_LOG}" 2>/dev/null || md5sum "${COMM_LOG}" 2>/dev/null | cut -d' ' -f1)

            if [[ "${current_checksum}" != "${last_checksum}" ]]; then
                if [[ -n "${last_checksum}" ]]; then
                    log_debug "Change detected in communication log (poll)"
                    parse_new_messages
                fi
                last_checksum="${current_checksum}"
            fi
        fi

        sleep "${POLL_INTERVAL}"
    done
}

# ---------------------------------------------------------------------------
# Cleanup
# ---------------------------------------------------------------------------
cleanup() {
    log_info "Watcher shutting down"
    rm -f "${PID_FILE}"
    echo -e "\n${YELLOW}Watcher stopped.${NC}"
}

# ---------------------------------------------------------------------------
# Command: stop — Stop the running watcher
# ---------------------------------------------------------------------------
cmd_stop() {
    if [[ -f "${PID_FILE}" ]]; then
        local pid
        pid=$(cat "${PID_FILE}")
        if kill -0 "${pid}" 2>/dev/null; then
            kill "${pid}"
            echo -e "${GREEN}Watcher stopped${NC} (PID ${pid})"
        else
            echo -e "${YELLOW}Watcher was not running${NC} (stale PID ${pid})"
            rm -f "${PID_FILE}"
        fi
    else
        echo -e "${YELLOW}No watcher PID file found.${NC}"
    fi
}

# ---------------------------------------------------------------------------
# Main: argument parsing
# ---------------------------------------------------------------------------
main() {
    # Parse arguments
    case "${1:-}" in
        --setup)
            shift
            cmd_setup "$@"
            ;;
        --teardown)
            shift
            cmd_teardown "$@"
            ;;
        --list)
            cmd_list
            ;;
        --status)
            cmd_status
            ;;
        --stop)
            cmd_stop
            ;;
        --help|-h)
            echo "Usage: $(basename "$0") [command] [options]"
            echo ""
            echo "Commands:"
            echo "  (no args)              Start the watcher daemon"
            echo "  --setup <name> <role>  Create a git worktree for an agent"
            echo "  --teardown <name>      Remove an agent's worktree"
            echo "  --list                 List all agent worktrees"
            echo "  --status               Show watcher and agent status"
            echo "  --stop                 Stop the running watcher"
            echo "  --help                 Show this help"
            echo ""
            echo "Environment:"
            echo "  PROJECT_ROOT           Override project root (default: git root)"
            echo "  DEBUG=1                Enable debug logging"
            echo ""
            echo "Examples:"
            echo "  # Set up agents"
            echo "  $(basename "$0") --setup backend-01 backend"
            echo "  $(basename "$0") --setup frontend-01 frontend"
            echo ""
            echo "  # Start watching"
            echo "  $(basename "$0")"
            echo ""
            echo "  # In another terminal, check status"
            echo "  $(basename "$0") --status"
            ;;
        --project-root)
            PROJECT_ROOT="$2"
            shift 2
            watch_loop
            ;;
        --interval)
            POLL_INTERVAL="$2"
            shift 2
            watch_loop
            ;;
        "")
            watch_loop
            ;;
        *)
            log_error "Unknown command: $1"
            echo "Run with --help for usage."
            exit 1
            ;;
    esac
}

main "$@"
