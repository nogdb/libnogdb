#!/usr/bin/env bash
# =============================================================================
# agent-tab-watcher.sh
# =============================================================================
# Per-agent signal watcher designed for Windsurf's built-in worktree tabs.
#
# Each Windsurf agent tab opens its own git worktree. This script runs inside
# that worktree's terminal tab — one bash process per tab, no extra Windsurf
# windows needed.
#
# Setup (once per worktree):
#   ./agent-tab-watcher.sh --init agent-01 backend
#
# Start watching (every time after):
#   ./agent-tab-watcher.sh
#
# Architecture:
#
#   Windsurf Tab A  (worktree: agent/backend)   → agent-tab-watcher.sh
#   Windsurf Tab B  (worktree: agent/frontend)  → agent-tab-watcher.sh
#   Windsurf Tab C  (worktree: agent/qa)        → agent-tab-watcher.sh
#                                                        │
#                          agent_workspace/communication-log.md
#                                                        │
#                              .agent-signals/<ts>-<TYPE>.signal
#
# Each worktree stores its own .agent-info (written by --init). On plain
# invocation the script reads that file automatically — no flags required.
#
# Prerequisites:
#   - fswatch (brew install fswatch) or falls back to polling
#
# Usage:
#   ./agent-tab-watcher.sh --init <name> <role>  One-time worktree setup
#   ./agent-tab-watcher.sh                        Start watcher (reads .agent-info)
#   ./agent-tab-watcher.sh --status               Show agent/signal status
#   ./agent-tab-watcher.sh --clear                Remove all pending signals
#   ./agent-tab-watcher.sh --stop                 Stop running watcher
#   ./agent-tab-watcher.sh --help                 Show this help
#
# Options (override .agent-info values):
#   --agent  <name>      Agent identifier, e.g. agent-01, backend-01
#   --role   <role>      Agent role, e.g. backend, frontend, qa
#   --root   <path>      Override project root (default: git root of CWD)
#   --interval <secs>    Poll interval when fswatch unavailable (default: 2)
#   --debug              Enable debug output
#
# =============================================================================

set -euo pipefail

# ---------------------------------------------------------------------------
# Defaults (overridden by CLI args)
# ---------------------------------------------------------------------------
WORKTREE_ROOT="$(pwd)"
AGENT_NAME=""
AGENT_ROLE="general"
POLL_INTERVAL=2
DEBUG="${DEBUG:-0}"

# ---------------------------------------------------------------------------
# Derived paths (set after arg parsing)
# ---------------------------------------------------------------------------
AGENT_WORKSPACE=""
COMM_LOG=""
SIGNAL_DIR=""
AGENT_INFO_FILE=""
LOG_FILE=""
PID_FILE=""
LAST_LINE_FILE=""

# ---------------------------------------------------------------------------
# Colors
# ---------------------------------------------------------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

# ---------------------------------------------------------------------------
# Logging
# ---------------------------------------------------------------------------
log() {
    local level="$1"; shift
    local msg="$*"
    local ts
    ts="$(date '+%Y-%m-%dT%H:%M:%S')"
    local line="${ts} [${level}] [${AGENT_NAME:-unset}] ${msg}"
    echo -e "${line}"
    if [[ -n "${LOG_FILE}" ]]; then
        # Strip ANSI codes when writing to file
        echo -e "${line}" | sed 's/\x1b\[[0-9;]*m//g' >> "${LOG_FILE}" 2>/dev/null || true
    fi
}

log_info()  { log "${GREEN}INFO${NC}"  "$@"; }
log_warn()  { log "${YELLOW}WARN${NC}" "$@"; }
log_error() { log "${RED}ERROR${NC}"  "$@"; }
log_debug() { [[ "${DEBUG}" == "1" ]] && log "${CYAN}DEBUG${NC}" "$@" || true; }
log_signal(){ log "${MAGENTA}SIGNAL${NC}" "$@"; }

# ---------------------------------------------------------------------------
# Utility: detect best file watcher available
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
# Init: set all derived paths and create directories
# ---------------------------------------------------------------------------
init_paths() {
    AGENT_WORKSPACE="${WORKTREE_ROOT}/agent_workspace"
    COMM_LOG="${AGENT_WORKSPACE}/communication-log.md"
    SIGNAL_DIR="${WORKTREE_ROOT}/.agent-signals"
    local worktree_name
    worktree_name=$(basename "${WORKTREE_ROOT}")
    AGENT_INFO_FILE="${WORKTREE_ROOT}/.agent-info/${worktree_name}/agent-info"
    LOG_FILE="${SIGNAL_DIR}/watcher.log"
    PID_FILE="${SIGNAL_DIR}/watcher.pid"
    LAST_LINE_FILE="${SIGNAL_DIR}/.watcher-last-line"

    mkdir -p "${SIGNAL_DIR}"
    mkdir -p "${AGENT_WORKSPACE}"
}

# ---------------------------------------------------------------------------
# Init: write .agent-info into this worktree (--init command)
# ---------------------------------------------------------------------------
cmd_init() {
    local name="${1:-}"
    local role="${2:-general}"

    if [[ -z "${name}" ]]; then
        echo -e "${RED}Error:${NC} --init requires <name> and optionally <role>"
        echo "  Example: $(basename "$0") --init agent-01 backend"
        exit 1
    fi

    init_paths

    cat > "${AGENT_INFO_FILE}" <<EOF
name=${name}
role=${role}
worktree=${WORKTREE_ROOT}
project_root=${WORKTREE_ROOT}
EOF

    echo -e "${GREEN}✅ Worktree initialised${NC}"
    echo -e "  Agent:    ${CYAN}${name}${NC}"
    echo -e "  Role:     ${CYAN}${role}${NC}"
    echo -e "  Info:     ${AGENT_INFO_FILE}"
    echo -e "  Signals:  ${SIGNAL_DIR}"
    echo ""
    echo -e "Start watching with: ${CYAN}$(basename "$0")${NC}"
}

# ---------------------------------------------------------------------------
# Init: load .agent-info from this worktree into AGENT_NAME / AGENT_ROLE
# ---------------------------------------------------------------------------
load_agent_info() {
    if [[ ! -f "${AGENT_INFO_FILE}" ]]; then
        return 1
    fi
    local name role
    name=$(grep '^name=' "${AGENT_INFO_FILE}" | cut -d= -f2)
    role=$(grep '^role=' "${AGENT_INFO_FILE}" | cut -d= -f2)
    [[ -n "${name}" ]] && AGENT_NAME="${name}"
    [[ -n "${role}" ]] && AGENT_ROLE="${role}"
    return 0
}

# ---------------------------------------------------------------------------
# Init: write started/pid fields into existing .agent-info on watch start
# ---------------------------------------------------------------------------
write_agent_info() {
    cat > "${AGENT_INFO_FILE}" <<EOF
name=${AGENT_NAME}
role=${AGENT_ROLE}
worktree=${WORKTREE_ROOT}
project_root=${WORKTREE_ROOT}
started=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
pid=$$
EOF
    log_info "Agent info written to ${AGENT_INFO_FILE}"
}

# ---------------------------------------------------------------------------
# Signal: write a signal file into THIS worktree's .agent-signals/
# ---------------------------------------------------------------------------
write_signal() {
    local signal_type="$1"   # DISTRIBUTE, HANDOFF, BLOCKER_RESOLVED, PHASE_ADVANCE
    local payload="$2"

    local ts
    ts="$(date '+%Y%m%d-%H%M%S')"
    local signal_file="${SIGNAL_DIR}/${ts}-${signal_type}.signal"

    cat > "${signal_file}" <<EOF
signal_type=${signal_type}
timestamp=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
agent=${AGENT_NAME}
role=${AGENT_ROLE}
source=agent-tab-watcher
payload=${payload}
EOF

    # Touch trigger file — Cascade can watch this via file polling
    touch "${SIGNAL_DIR}/.trigger"

    log_signal "Received ${signal_type}: ${payload}"
    echo -e "${MAGENTA}⚡ SIGNAL${NC} [${signal_type}] → ${payload}"
}

# ---------------------------------------------------------------------------
# Parse: extract new lines from communication log since last check
# ---------------------------------------------------------------------------
parse_new_messages() {
    local last_line=0
    if [[ -f "${LAST_LINE_FILE}" ]]; then
        last_line=$(cat "${LAST_LINE_FILE}")
    fi

    if [[ ! -f "${COMM_LOG}" ]]; then
        log_debug "Communication log not found: ${COMM_LOG}"
        return
    fi

    local total_lines
    total_lines=$(wc -l < "${COMM_LOG}" | tr -d ' ')

    if [[ "${total_lines}" -le "${last_line}" ]]; then
        return
    fi

    local new_content
    new_content=$(tail -n +"$((last_line + 1))" "${COMM_LOG}")

    echo "${total_lines}" > "${LAST_LINE_FILE}"

    log_debug "Processing lines $((last_line + 1))–${total_lines} from comm log"

    parse_distribute_messages "${new_content}"
    parse_completed_messages  "${new_content}"
    parse_handoff_messages    "${new_content}"
    parse_phase_advance       "${new_content}"
}

# ---------------------------------------------------------------------------
# Parse: DISTRIBUTE — look for assignments addressed to this agent or role
#
# Matches comm-log table rows like:
#   | **Assignments** | P1-T001 → agent-01, P1-T002 → backend-01 |
# ---------------------------------------------------------------------------
parse_distribute_messages() {
    local content="$1"

    while IFS= read -r line; do
        if [[ "${line}" =~ \*\*Assignments\*\* ]]; then
            local assignments
            assignments=$(echo "${line}" \
                | sed 's/.*\*\*Assignments\*\*[[:space:]]*|[[:space:]]*//' \
                | sed 's/[[:space:]]*|$//')

            log_debug "Found assignments line: ${assignments}"

            IFS=',' read -ra pairs <<< "${assignments}"
            for pair in "${pairs[@]}"; do
                pair=$(echo "${pair}" | xargs)
                if [[ "${pair}" =~ →[[:space:]]*(.+) ]]; then
                    local target
                    target=$(echo "${BASH_REMATCH[1]}" | xargs)

                    if _matches_this_agent "${target}"; then
                        write_signal "DISTRIBUTE" "${pair}"
                    fi
                fi
            done
        fi
    done <<< "${content}"
}

# ---------------------------------------------------------------------------
# Parse: COMPLETED — look for Handoff To entries addressed to this agent
#
# Matches:
#   | **Handoff To** | agent-01 — integrate the new API |
# ---------------------------------------------------------------------------
parse_completed_messages() {
    local content="$1"
    local in_completed=0

    while IFS= read -r line; do
        if [[ "${line}" =~ COMPLETED ]]; then
            in_completed=1
        fi

        if [[ "${in_completed}" -eq 1 ]] && [[ "${line}" =~ \*\*Handoff\ To\*\* ]]; then
            local handoff_to
            handoff_to=$(echo "${line}" \
                | sed 's/.*\*\*Handoff To\*\*[[:space:]]*|[[:space:]]*//' \
                | sed 's/[[:space:]]*|$//')

            if [[ "${handoff_to}" != "None" ]] && [[ -n "${handoff_to}" ]]; then
                local target
                target=$(echo "${handoff_to}" | sed 's/ — .*//' | xargs)

                if _matches_this_agent "${target}"; then
                    write_signal "HANDOFF" "${handoff_to}"
                fi
            fi
            in_completed=0
        fi
    done <<< "${content}"
}

# ---------------------------------------------------------------------------
# Parse: explicit HANDOFF message blocks addressed to this agent
#
# Matches section headers like:
#   ### HANDOFF | from: agent-02 | to: agent-01
# ---------------------------------------------------------------------------
parse_handoff_messages() {
    local content="$1"

    while IFS= read -r line; do
        if [[ "${line}" =~ "### HANDOFF" ]] || [[ "${line}" =~ "##.*HANDOFF" ]]; then
            # Extract "to:" field
            if [[ "${line}" =~ to:[[:space:]]*([^|[:space:]]+) ]]; then
                local target="${BASH_REMATCH[1]}"
                if _matches_this_agent "${target}"; then
                    write_signal "HANDOFF" "${line}"
                fi
            fi
        fi
    done <<< "${content}"
}

# ---------------------------------------------------------------------------
# Parse: PHASE_ADVANCE — broadcast to all agents (no target filter)
#
# Matches:
#   ### PHASE_ADVANCE | phase: 2
# ---------------------------------------------------------------------------
parse_phase_advance() {
    local content="$1"

    while IFS= read -r line; do
        if [[ "${line}" =~ PHASE_ADVANCE ]]; then
            write_signal "PHASE_ADVANCE" "${line}"
        fi
    done <<< "${content}"
}

# ---------------------------------------------------------------------------
# Helper: check if a target string refers to this agent (by name or role)
# ---------------------------------------------------------------------------
_matches_this_agent() {
    local target="${1,,}"   # lowercase
    local name="${AGENT_NAME,,}"
    local role="${AGENT_ROLE,,}"

    [[ "${target}" == "${name}" ]] || \
    [[ "${target}" == *"${name}"* ]] || \
    [[ "${target}" == "${role}" ]] || \
    [[ "${target}" == *"${role}"* ]]
}

# ---------------------------------------------------------------------------
# Command: --status — show this agent's current state
# ---------------------------------------------------------------------------
cmd_status() {
    init_paths

    echo -e "${BLUE}═══════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}  Agent Tab Watcher — Status${NC}"
    echo -e "${BLUE}═══════════════════════════════════════════════════${NC}"
    echo -e "  Worktree:  ${WORKTREE_ROOT}"
    echo -e "  Comm Log:  ${COMM_LOG}"
    echo -e "  Signal Dir:${SIGNAL_DIR}"
    echo ""

    # Agent info
    if [[ -f "${AGENT_INFO_FILE}" ]]; then
        echo -e "  ${GREEN}Agent Info:${NC}"
        while IFS='=' read -r key val; do
            printf "    %-12s %s\n" "${key}:" "${val}"
        done < "${AGENT_INFO_FILE}"
    else
        echo -e "  ${YELLOW}No .agent-info found (watcher not started yet)${NC}"
    fi
    echo ""

    # Watcher process
    if [[ -f "${PID_FILE}" ]]; then
        local pid
        pid=$(cat "${PID_FILE}")
        if kill -0 "${pid}" 2>/dev/null; then
            echo -e "  Watcher:   ${GREEN}Running${NC} (PID ${pid})"
        else
            echo -e "  Watcher:   ${RED}Stale PID${NC} (${pid} not running)"
        fi
    else
        echo -e "  Watcher:   ${RED}Not running${NC}"
    fi

    # Pending signals
    local count=0
    count=$(find "${SIGNAL_DIR}" -name "*.signal" 2>/dev/null | wc -l | tr -d ' ')
    if [[ "${count}" -gt 0 ]]; then
        echo -e "  Signals:   ${YELLOW}${count} pending${NC}"
        find "${SIGNAL_DIR}" -name "*.signal" -exec basename {} \; 2>/dev/null \
            | sort | while read -r f; do echo "    - ${f}"; done
    else
        echo -e "  Signals:   ${GREEN}none pending${NC}"
    fi
    echo ""
}

# ---------------------------------------------------------------------------
# Command: --clear — remove all pending signal files
# ---------------------------------------------------------------------------
cmd_clear() {
    init_paths
    local count=0
    count=$(find "${SIGNAL_DIR}" -name "*.signal" 2>/dev/null | wc -l | tr -d ' ')
    find "${SIGNAL_DIR}" -name "*.signal" -delete 2>/dev/null || true
    rm -f "${SIGNAL_DIR}/.trigger"
    echo -e "${GREEN}Cleared ${count} signal file(s) from ${SIGNAL_DIR}${NC}"
}

# ---------------------------------------------------------------------------
# Command: --stop — stop a running watcher for this agent
# ---------------------------------------------------------------------------
cmd_stop() {
    init_paths
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
        echo -e "${YELLOW}No watcher PID file found in ${SIGNAL_DIR}${NC}"
    fi
}

# ---------------------------------------------------------------------------
# Cleanup on exit
# ---------------------------------------------------------------------------
cleanup() {
    log_info "Watcher shutting down"
    rm -f "${PID_FILE}"
    rm -f "${SIGNAL_DIR}/.listening"
    echo -e "\n${YELLOW}[${AGENT_NAME}] Watcher stopped.${NC}"
}

# ---------------------------------------------------------------------------
# Watch loop
# ---------------------------------------------------------------------------
watch_loop() {
    local method
    method=$(detect_watcher)

    log_info "Starting watcher (method: ${method})"
    log_info "Worktree:  ${WORKTREE_ROOT}"
    log_info "Comm log:  ${COMM_LOG}"
    log_info "Signals:   ${SIGNAL_DIR}"

    # Initialise last-line tracker from current log size
    if [[ -f "${COMM_LOG}" ]]; then
        wc -l < "${COMM_LOG}" | tr -d ' ' > "${LAST_LINE_FILE}"
    else
        echo "0" > "${LAST_LINE_FILE}"
    fi

    # Mark as listening
    touch "${SIGNAL_DIR}/.listening"

    # Save PID
    echo "$$" > "${PID_FILE}"

    trap cleanup EXIT INT TERM

    echo ""
    echo -e "${GREEN}🔭 Agent tab watcher started${NC}"
    echo -e "  Agent:    ${CYAN}${AGENT_NAME}${NC}"
    echo -e "  Role:     ${CYAN}${AGENT_ROLE}${NC}"
    echo -e "  Method:   ${method}"
    echo -e "  Watching: ${COMM_LOG}"
    echo -e "  Signals → ${SIGNAL_DIR}"
    echo -e "  PID:      $$"
    echo ""
    echo "Press Ctrl+C to stop."
    echo ""

    case "${method}" in
        fswatch)    _watch_fswatch ;;
        inotifywait) _watch_inotifywait ;;
        poll)       _watch_poll ;;
    esac
}

_watch_fswatch() {
    fswatch -o "${COMM_LOG}" 2>/dev/null | while read -r _; do
        log_debug "fswatch: change detected"
        parse_new_messages
    done
}

_watch_inotifywait() {
    while true; do
        inotifywait -q -e modify "${COMM_LOG}" 2>/dev/null
        log_debug "inotifywait: change detected"
        parse_new_messages
    done
}

_watch_poll() {
    log_warn "fswatch/inotifywait not found — polling every ${POLL_INTERVAL}s"
    log_warn "Install fswatch for better performance: brew install fswatch"

    local last_checksum=""
    while true; do
        if [[ -f "${COMM_LOG}" ]]; then
            local current_checksum
            current_checksum=$(md5 -q "${COMM_LOG}" 2>/dev/null \
                || md5sum "${COMM_LOG}" 2>/dev/null | cut -d' ' -f1)

            if [[ "${current_checksum}" != "${last_checksum}" ]]; then
                if [[ -n "${last_checksum}" ]]; then
                    log_debug "poll: change detected"
                    parse_new_messages
                fi
                last_checksum="${current_checksum}"
            fi
        fi
        sleep "${POLL_INTERVAL}"
    done
}

# ---------------------------------------------------------------------------
# Usage
# ---------------------------------------------------------------------------
usage() {
    cat <<EOF
${CYAN}agent-tab-watcher.sh${NC} — per-worktree signal watcher for Windsurf agent tabs

${YELLOW}Setup (once per worktree tab):${NC}
  $(basename "$0") --init <name> <role>

${YELLOW}Daily use (no flags needed after init):${NC}
  $(basename "$0")

${YELLOW}Other commands:${NC}
  $(basename "$0") --status               Show agent info and pending signals
  $(basename "$0") --clear                Remove all pending signal files
  $(basename "$0") --stop                 Stop the running watcher
  $(basename "$0") --help                 Show this help

${YELLOW}Override options (take precedence over .agent-info):${NC}
  --agent    <name>    Agent identifier (e.g. agent-01, backend-01)
  --role     <role>    Agent role       (e.g. backend, frontend, qa)
  --root     <path>    Override project root (default: git root of CWD)
  --interval <secs>    Poll interval when fswatch unavailable (default: 2)
  --debug              Enable debug logging

${YELLOW}Typical workflow:${NC}
  # In each Windsurf worktree tab — run once:
  $(basename "$0") --init agent-01 backend
  $(basename "$0") --init agent-02 frontend
  $(basename "$0") --init agent-03 qa

  # Then every time you open that tab, just:
  $(basename "$0")

${YELLOW}Signal files${NC} are written to:
  <worktree>/.agent-signals/<timestamp>-<TYPE>.signal

${YELLOW}Cascade agents${NC} in the same tab can poll .agent-signals/.trigger or
read individual .signal files to react to incoming work.
EOF
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
main() {
    local do_watch=0

    # Handle --init early (before path init) since it takes positional args
    if [[ "${1:-}" == "--init" ]]; then
        shift
        # Allow --root before --init
        if [[ "${1:-}" == "--root" ]]; then
            WORKTREE_ROOT="$2"; shift 2
        fi
        init_paths
        cmd_init "$@"
        return
    fi

    # First pass: extract flags so init_paths works for all commands
    local args=("$@")
    local i=0
    while [[ $i -lt ${#args[@]} ]]; do
        case "${args[$i]}" in
            --root)     i=$((i+1)); WORKTREE_ROOT="${args[$i]}" ;;
            --agent)    i=$((i+1)); AGENT_NAME="${args[$i]}" ;;
            --role)     i=$((i+1)); AGENT_ROLE="${args[$i]}" ;;
            --interval) i=$((i+1)); POLL_INTERVAL="${args[$i]}" ;;
            --debug)    DEBUG=1 ;;
        esac
        i=$((i+1))
    done

    # Second pass: dispatch commands
    case "${1:-}" in
        --status)
            cmd_status
            return
            ;;
        --clear)
            cmd_clear
            return
            ;;
        --stop)
            cmd_stop
            return
            ;;
        --help|-h)
            usage
            return
            ;;
        "")
            # No args — try to load identity from .agent-info in this worktree
            init_paths
            if [[ -z "${AGENT_NAME}" ]]; then
                if load_agent_info; then
                    log_info "Loaded identity from ${AGENT_INFO_FILE}"
                else
                    echo -e "${RED}Error:${NC} No .agent-info found in this worktree."
                    echo -e "Run ${CYAN}$(basename "$0") --init <name> <role>${NC} first."
                    echo ""
                    usage
                    exit 1
                fi
            fi
            do_watch=1
            ;;
        --agent|--role|--root|--interval|--debug)
            # Flags only — load .agent-info then apply overrides
            init_paths
            load_agent_info || true   # best-effort; CLI flags already applied
            if [[ -z "${AGENT_NAME}" ]]; then
                echo -e "${RED}Error:${NC} No agent name. Run --init first or pass --agent <name>."
                exit 1
            fi
            do_watch=1
            ;;
        *)
            echo -e "${RED}Error:${NC} Unknown argument: $1"
            echo "Run with --help for usage."
            exit 1
            ;;
    esac

    if [[ "${do_watch}" -eq 1 ]]; then
        write_agent_info
        watch_loop
    fi
}

main "$@"
