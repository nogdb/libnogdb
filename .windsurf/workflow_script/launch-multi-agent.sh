#!/usr/bin/env bash
# =============================================================================
# launch-multi-agent.sh
# =============================================================================
# System terminal coordination driver for multi-agent Windsurf sessions.
#
# Run this from a SYSTEM TERMINAL (outside Windsurf) after each agent tab has
# run /agent-01-coordination start (which registers the agent in the comm-log
# and creates its .agent-info + .agent-signals/ directory).
#
# This script:
#   1. Reads agent_workspace/communication-log.md to discover online agents
#   2. Watches the comm-log via fswatch (or polling) for new messages
#   3. Routes DISTRIBUTE / HANDOFF / PHASE_ADVANCE signals to each agent's
#      .agent-signals/ inbox (same logic as agent-tab-watcher.sh but centralised)
#   4. Provides status, signal, broadcast, and teardown commands
#
# Usage:
#   ./launch-multi-agent.sh                          Watch mode (default)
#   ./launch-multi-agent.sh status                   Show online agents + signals
#   ./launch-multi-agent.sh signal <agent> <type> "<payload>"
#   ./launch-multi-agent.sh broadcast <type> "<payload>"
#   ./launch-multi-agent.sh teardown                 Graceful shutdown all agents
#   ./launch-multi-agent.sh help                     Show this help
#
# Prerequisites:
#   - fswatch (brew install fswatch) — falls back to polling if unavailable
#   - Agents must have run /agent-01-coordination start first
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
POLL_INTERVAL=2

# ---------------------------------------------------------------------------
# Colors
# ---------------------------------------------------------------------------
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
BOLD='\033[1m'
NC='\033[0m'

# ---------------------------------------------------------------------------
# Logging
# ---------------------------------------------------------------------------
ts() { date '+%Y-%m-%dT%H:%M:%S'; }
log_info()    { echo -e "$(ts) ${GREEN}[INFO]${NC}  $*"; }
log_warn()    { echo -e "$(ts) ${YELLOW}[WARN]${NC}  $*"; }
log_error()   { echo -e "$(ts) ${RED}[ERROR]${NC} $*"; }
log_signal()  { echo -e "$(ts) ${MAGENTA}[SIGNAL]${NC} $*"; }
log_route()   { echo -e "$(ts) ${CYAN}[ROUTE]${NC}  $*"; }

# ---------------------------------------------------------------------------
# Detect best file watcher
# ---------------------------------------------------------------------------
detect_watcher() {
    if command -v fswatch &>/dev/null; then echo "fswatch"
    elif command -v inotifywait &>/dev/null; then echo "inotifywait"
    else echo "poll"
    fi
}

# ---------------------------------------------------------------------------
# Parse comm-log: extract online agents
# Returns lines: "name|role|worktree"
# ---------------------------------------------------------------------------
get_online_agents() {
    if [[ ! -f "${COMM_LOG}" ]]; then
        return
    fi

    # Parse the Registered Agents table
    # Format: | Agent Name | Role | Registered At | Status | Session Info |
    # We look for rows with 🟢 Online status
    local in_table=0
    while IFS= read -r line; do
        if [[ "${line}" =~ "## Registered Agents" ]]; then
            in_table=1
            continue
        fi
        if [[ "${in_table}" -eq 1 ]] && [[ "${line}" =~ ^##[[:space:]] ]]; then
            in_table=0
        fi
        if [[ "${in_table}" -eq 1 ]] && [[ "${line}" =~ 🟢 ]]; then
            # Extract pipe-delimited fields
            local name role session_info worktree
            name=$(echo "${line}" | awk -F'|' '{print $2}' | xargs)
            role=$(echo "${line}" | awk -F'|' '{print $3}' | xargs)
            session_info=$(echo "${line}" | awk -F'|' '{print $6}' | xargs)
            # Extract worktree= from session info
            worktree=$(echo "${session_info}" | grep -o 'worktree=[^ ]*' | cut -d= -f2 || echo "")
            if [[ -n "${name}" ]] && [[ "${name}" != "Agent Name" ]]; then
                echo "${name}|${role}|${worktree}"
            fi
        fi
    done < "${COMM_LOG}"
}

# ---------------------------------------------------------------------------
# Get signal dir for a given agent (reads their .agent-info)
# ---------------------------------------------------------------------------
get_signal_dir() {
    local worktree="$1"
    if [[ -z "${worktree}" ]] || [[ ! -d "${worktree}" ]]; then
        return 1
    fi
    echo "${worktree}/.agent-signals"
}

# ---------------------------------------------------------------------------
# Write a signal file into an agent's .agent-signals/ inbox
# ---------------------------------------------------------------------------
write_signal() {
    local signal_dir="$1"
    local signal_type="$2"
    local payload="$3"
    local source="${4:-launch-multi-agent}"

    mkdir -p "${signal_dir}"

    local ts_file
    ts_file="$(date '+%Y%m%d-%H%M%S')"
    local signal_file="${signal_dir}/${ts_file}-${signal_type}.signal"

    cat > "${signal_file}" <<EOF
signal_type=${signal_type}
timestamp=$(date -u '+%Y-%m-%dT%H:%M:%SZ')
source=${source}
payload=${payload}
EOF

    touch "${signal_dir}/.trigger"
    log_route "→ ${signal_dir##*/}/.agent-signals [${signal_type}] ${payload}"
}

# ---------------------------------------------------------------------------
# Parse new lines from comm-log and route signals to matching agents
# ---------------------------------------------------------------------------
LAST_LINE_FILE="${AGENT_WORKSPACE}/.launcher-last-line"

parse_and_route() {
    if [[ ! -f "${COMM_LOG}" ]]; then
        return
    fi

    local last_line=0
    if [[ -f "${LAST_LINE_FILE}" ]]; then
        last_line=$(cat "${LAST_LINE_FILE}")
    fi

    local total_lines
    total_lines=$(wc -l < "${COMM_LOG}" | tr -d ' ')

    if [[ "${total_lines}" -le "${last_line}" ]]; then
        return
    fi

    local new_content
    new_content=$(tail -n +"$((last_line + 1))" "${COMM_LOG}")
    echo "${total_lines}" > "${LAST_LINE_FILE}"

    # Load current online agents
    local agents_raw
    agents_raw=$(get_online_agents)

    # Route DISTRIBUTE messages
    while IFS= read -r line; do
        if [[ "${line}" =~ \*\*Assignments\*\* ]]; then
            local assignments
            assignments=$(echo "${line}" \
                | sed 's/.*\*\*Assignments\*\*[[:space:]]*|[[:space:]]*//' \
                | sed 's/[[:space:]]*|$//')

            IFS=',' read -ra pairs <<< "${assignments}"
            for pair in "${pairs[@]}"; do
                pair=$(echo "${pair}" | xargs)
                if [[ "${pair}" =~ →[[:space:]]*(.+) ]]; then
                    local target
                    target=$(echo "${BASH_REMATCH[1]}" | xargs | tr '[:upper:]' '[:lower:]')

                    # Find matching agent
                    while IFS='|' read -r aname arole aworktree; do
                        local aname_lc arole_lc
                        aname_lc=$(echo "${aname}" | tr '[:upper:]' '[:lower:]')
                        arole_lc=$(echo "${arole}" | tr '[:upper:]' '[:lower:]')
                        if [[ "${target}" == *"${aname_lc}"* ]] || [[ "${target}" == *"${arole_lc}"* ]]; then
                            local sdir
                            sdir=$(get_signal_dir "${aworktree}") || continue
                            write_signal "${sdir}" "DISTRIBUTE" "${pair}" "launch-multi-agent"
                        fi
                    done <<< "${agents_raw}"
                fi
            done
        fi

        # Route HANDOFF messages
        if [[ "${line}" =~ \*\*Handoff\ To\*\* ]]; then
            local handoff_to
            handoff_to=$(echo "${line}" \
                | sed 's/.*\*\*Handoff To\*\*[[:space:]]*|[[:space:]]*//' \
                | sed 's/[[:space:]]*|$//')

            if [[ "${handoff_to}" != "None" ]] && [[ -n "${handoff_to}" ]]; then
                local target
                target=$(echo "${handoff_to}" | sed 's/ — .*//' | xargs | tr '[:upper:]' '[:lower:]')

                while IFS='|' read -r aname arole aworktree; do
                    local aname_lc arole_lc
                    aname_lc=$(echo "${aname}" | tr '[:upper:]' '[:lower:]')
                    arole_lc=$(echo "${arole}" | tr '[:upper:]' '[:lower:]')
                    if [[ "${target}" == *"${aname_lc}"* ]] || [[ "${target}" == *"${arole_lc}"* ]]; then
                        local sdir
                        sdir=$(get_signal_dir "${aworktree}") || continue
                        write_signal "${sdir}" "HANDOFF" "${handoff_to}" "launch-multi-agent"
                    fi
                done <<< "${agents_raw}"
            fi
        fi

        # Route PHASE_ADVANCE — broadcast to all agents
        if [[ "${line}" =~ PHASE_ADVANCE ]]; then
            log_signal "PHASE_ADVANCE detected — broadcasting to all agents"
            while IFS='|' read -r aname arole aworktree; do
                local sdir
                sdir=$(get_signal_dir "${aworktree}") || continue
                write_signal "${sdir}" "PHASE_ADVANCE" "${line}" "launch-multi-agent"
            done <<< "${agents_raw}"
        fi

    done <<< "${new_content}"
}

# ---------------------------------------------------------------------------
# Command: status
# ---------------------------------------------------------------------------
cmd_status() {
    echo -e "${BLUE}${BOLD}═══════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}${BOLD}  Multi-Agent Status${NC}"
    echo -e "${BLUE}  Project: ${PROJECT_ROOT}${NC}"
    echo -e "${BLUE}  Comm log: ${COMM_LOG}${NC}"
    echo -e "${BLUE}${BOLD}═══════════════════════════════════════════════════════${NC}"
    echo ""

    if [[ ! -f "${COMM_LOG}" ]]; then
        echo -e "  ${YELLOW}No communication log found.${NC}"
        echo -e "  Run /agent-01-coordination start in a Cascade tab first."
        return
    fi

    local agents_raw
    agents_raw=$(get_online_agents)

    if [[ -z "${agents_raw}" ]]; then
        echo -e "  ${YELLOW}No online agents found in comm-log.${NC}"
        echo -e "  Run /agent-01-coordination start in each Cascade worktree tab."
        return
    fi

    local count=0
    while IFS='|' read -r aname arole aworktree; do
        count=$((count + 1))
        echo -e "  ${GREEN}●${NC} ${BOLD}${aname}${NC}  (${arole})"
        echo -e "    Worktree: ${aworktree:-unknown}"

        local sdir="${aworktree}/.agent-signals"
        if [[ -d "${sdir}" ]]; then
            local sig_count
            sig_count=$(find "${sdir}" -name "*.signal" 2>/dev/null | wc -l | tr -d ' ')
            local watcher_pid_file="${sdir}/watcher.pid"
            local watcher_status="${RED}watcher not running${NC}"
            if [[ -f "${watcher_pid_file}" ]]; then
                local pid
                pid=$(cat "${watcher_pid_file}")
                if kill -0 "${pid}" 2>/dev/null; then
                    watcher_status="${GREEN}watcher running (PID ${pid})${NC}"
                else
                    watcher_status="${YELLOW}watcher stale PID${NC}"
                fi
            fi
            echo -e "    Signals:  ${sig_count} pending"
            echo -e "    Watcher:  ${watcher_status}"
        else
            echo -e "    ${YELLOW}No .agent-signals/ dir — watcher not started yet${NC}"
        fi
        echo ""
    done <<< "${agents_raw}"

    echo -e "  ${BOLD}Total online agents: ${count}${NC}"
}

# ---------------------------------------------------------------------------
# Command: signal <agent-name-or-role> <type> <payload>
# ---------------------------------------------------------------------------
cmd_signal() {
    local target="${1:-}"
    local signal_type="${2:-DISTRIBUTE}"
    local payload="${3:-}"

    if [[ -z "${target}" ]] || [[ -z "${payload}" ]]; then
        echo -e "${RED}Error:${NC} Usage: $(basename "$0") signal <agent-name-or-role> <type> \"<payload>\""
        exit 1
    fi

    local target_lc
    target_lc=$(echo "${target}" | tr '[:upper:]' '[:lower:]')
    local matched=0

    while IFS='|' read -r aname arole aworktree; do
        local aname_lc arole_lc
        aname_lc=$(echo "${aname}" | tr '[:upper:]' '[:lower:]')
        arole_lc=$(echo "${arole}" | tr '[:upper:]' '[:lower:]')
        if [[ "${target_lc}" == "${aname_lc}" ]] || [[ "${target_lc}" == "${arole_lc}" ]]; then
            local sdir
            sdir=$(get_signal_dir "${aworktree}") || continue
            write_signal "${sdir}" "${signal_type}" "${payload}" "manual"
            matched=$((matched + 1))
        fi
    done <<< "$(get_online_agents)"

    if [[ "${matched}" -eq 0 ]]; then
        echo -e "${YELLOW}No online agent matched '${target}'.${NC}"
        echo -e "Run: $(basename "$0") status"
    else
        echo -e "${GREEN}Signal sent to ${matched} agent(s).${NC}"
    fi
}

# ---------------------------------------------------------------------------
# Command: broadcast <type> <payload>
# ---------------------------------------------------------------------------
cmd_broadcast() {
    local signal_type="${1:-PHASE_ADVANCE}"
    local payload="${2:-}"

    if [[ -z "${payload}" ]]; then
        echo -e "${RED}Error:${NC} Usage: $(basename "$0") broadcast <type> \"<payload>\""
        exit 1
    fi

    local count=0
    while IFS='|' read -r aname arole aworktree; do
        local sdir
        sdir=$(get_signal_dir "${aworktree}") || continue
        write_signal "${sdir}" "${signal_type}" "${payload}" "broadcast"
        count=$((count + 1))
    done <<< "$(get_online_agents)"

    echo -e "${GREEN}Broadcast [${signal_type}] sent to ${count} agent(s).${NC}"
}

# ---------------------------------------------------------------------------
# Command: teardown — mark all agents offline, clear signals
# ---------------------------------------------------------------------------
cmd_teardown() {
    echo -e "${YELLOW}Tearing down all agents...${NC}"

    local count=0
    while IFS='|' read -r aname arole aworktree; do
        echo -e "  Stopping ${aname} (${arole})..."

        # Stop watcher if running
        local pid_file="${aworktree}/.agent-signals/watcher.pid"
        if [[ -f "${pid_file}" ]]; then
            local pid
            pid=$(cat "${pid_file}")
            if kill -0 "${pid}" 2>/dev/null; then
                kill "${pid}" 2>/dev/null || true
                echo -e "    ${GREEN}Watcher stopped (PID ${pid})${NC}"
            fi
        fi

        # Clear signal files
        local sdir="${aworktree}/.agent-signals"
        if [[ -d "${sdir}" ]]; then
            find "${sdir}" -name "*.signal" -delete 2>/dev/null || true
            rm -f "${sdir}/.trigger" "${sdir}/.listening"
            echo -e "    ${GREEN}Signals cleared${NC}"
        fi

        count=$((count + 1))
    done <<< "$(get_online_agents)"

    # Mark all agents offline in comm-log
    if [[ -f "${COMM_LOG}" ]] && [[ "${count}" -gt 0 ]]; then
        # Replace 🟢 Online with 🔴 Offline in the Registered Agents table
        sed -i '' 's/🟢 Online/🔴 Offline/g' "${COMM_LOG}" 2>/dev/null || \
        sed -i 's/🟢 Online/🔴 Offline/g' "${COMM_LOG}"
        echo -e "  ${GREEN}All agents marked 🔴 Offline in comm-log${NC}"
    fi

    # Clean up launcher state
    rm -f "${LAST_LINE_FILE}"

    echo -e "${GREEN}Teardown complete. ${count} agent(s) stopped.${NC}"
}

# ---------------------------------------------------------------------------
# Watch mode: continuous coordination loop
# ---------------------------------------------------------------------------
cmd_watch() {
    if [[ ! -f "${COMM_LOG}" ]]; then
        echo -e "${YELLOW}Waiting for communication log to appear at:${NC}"
        echo -e "  ${COMM_LOG}"
        echo -e "${YELLOW}Run /agent-01-coordination start in a Cascade tab first.${NC}"
        echo ""
    fi

    local method
    method=$(detect_watcher)

    echo -e "${BLUE}${BOLD}═══════════════════════════════════════════════════════${NC}"
    echo -e "${BLUE}${BOLD}  Multi-Agent Coordination Driver${NC}"
    echo -e "${BLUE}  Project:  ${PROJECT_ROOT}${NC}"
    echo -e "${BLUE}  Comm log: ${COMM_LOG}${NC}"
    echo -e "${BLUE}  Method:   ${method}${NC}"
    echo -e "${BLUE}${BOLD}═══════════════════════════════════════════════════════${NC}"
    echo ""
    echo -e "Watching for agent messages and routing signals..."
    echo -e "Press ${BOLD}Ctrl+C${NC} to stop."
    echo ""

    # Initialise last-line tracker
    mkdir -p "${AGENT_WORKSPACE}"
    if [[ -f "${COMM_LOG}" ]]; then
        wc -l < "${COMM_LOG}" | tr -d ' ' > "${LAST_LINE_FILE}"
    else
        echo "0" > "${LAST_LINE_FILE}"
    fi

    # Print initial agent status
    local agents_raw
    agents_raw=$(get_online_agents)
    if [[ -n "${agents_raw}" ]]; then
        log_info "Online agents:"
        while IFS='|' read -r aname arole aworktree; do
            log_info "  ${GREEN}●${NC} ${aname} (${arole})  ${aworktree}"
        done <<< "${agents_raw}"
    else
        log_warn "No online agents yet — waiting for registrations..."
    fi
    echo ""

    trap 'echo ""; log_info "Launcher stopped."; rm -f "${LAST_LINE_FILE}"; exit 0' INT TERM

    case "${method}" in
        fswatch)
            # Watch the whole agent_workspace dir so new comm-log creation is caught too
            fswatch -o "${AGENT_WORKSPACE}" 2>/dev/null | while read -r _; do
                parse_and_route
            done
            ;;
        inotifywait)
            while true; do
                inotifywait -q -e modify,create "${AGENT_WORKSPACE}" 2>/dev/null || true
                parse_and_route
            done
            ;;
        poll)
            log_warn "fswatch not found — polling every ${POLL_INTERVAL}s (brew install fswatch for better performance)"
            local last_checksum=""
            while true; do
                if [[ -f "${COMM_LOG}" ]]; then
                    local current_checksum
                    current_checksum=$(md5 -q "${COMM_LOG}" 2>/dev/null \
                        || md5sum "${COMM_LOG}" 2>/dev/null | cut -d' ' -f1 || echo "")
                    if [[ "${current_checksum}" != "${last_checksum}" ]] && [[ -n "${last_checksum}" ]]; then
                        parse_and_route
                    fi
                    last_checksum="${current_checksum}"
                fi
                sleep "${POLL_INTERVAL}"
            done
            ;;
    esac
}

# ---------------------------------------------------------------------------
# Help
# ---------------------------------------------------------------------------
cmd_help() {
    cat <<EOF
${CYAN}${BOLD}launch-multi-agent.sh${NC} — System terminal coordination driver for multi-agent Windsurf sessions

${YELLOW}${BOLD}Usage:${NC}
  $(basename "$0")                                   Watch mode (default)
  $(basename "$0") status                            Show online agents + pending signals
  $(basename "$0") signal <agent> <type> "<payload>" Send signal to one agent
  $(basename "$0") broadcast <type> "<payload>"      Send signal to all online agents
  $(basename "$0") teardown                          Stop all watchers, mark agents offline
  $(basename "$0") help                              Show this help

${YELLOW}${BOLD}Signal types:${NC}
  DISTRIBUTE       New task assignment
  HANDOFF          Task completed, next agent's turn
  PHASE_ADVANCE    New phase starting (broadcast)
  BLOCKER_RESOLVED A blocker has been cleared

${YELLOW}${BOLD}Examples:${NC}
  $(basename "$0")
  $(basename "$0") status
  $(basename "$0") signal backend-01 DISTRIBUTE "P1-T003 → backend-01"
  $(basename "$0") signal frontend HANDOFF "integrate the new API"
  $(basename "$0") broadcast PHASE_ADVANCE "phase: 2 starting"
  $(basename "$0") teardown

${YELLOW}${BOLD}Prerequisites:${NC}
  1. Each Cascade worktree tab must have run /agent-01-coordination start
     (this creates .agent-info, .agent-signals/, and registers in comm-log)
  2. Each tab's terminal panel must be running agent-tab-watcher.sh
  3. fswatch recommended: brew install fswatch

${YELLOW}${BOLD}How agent discovery works:${NC}
  Reads agent_workspace/communication-log.md → parses ## Registered Agents table
  → finds rows with 🟢 Online → extracts worktree= from Session Info column
  → uses <worktree>/.agent-signals/ as each agent's signal inbox
EOF
}

# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------
case "${1:-}" in
    status)   cmd_status ;;
    signal)   shift; cmd_signal "$@" ;;
    broadcast) shift; cmd_broadcast "$@" ;;
    teardown) cmd_teardown ;;
    help|--help|-h) cmd_help ;;
    "")       cmd_watch ;;
    *)
        echo -e "${RED}Error:${NC} Unknown command: $1"
        echo "Run: $(basename "$0") help"
        exit 1
        ;;
esac
