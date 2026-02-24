#!/usr/bin/env bash
# =============================================================================
# agent-signal-listener.sh
# =============================================================================
# Lightweight script that an agent's Cascade session can invoke to wait for
# incoming signals from the worktree watcher.
#
# This script blocks until a signal file appears in the agent's
# .agent-signals/ directory, then outputs the signal details and exits.
# Cascade can then parse the output and act on it.
#
# Usage (from within an agent's worktree):
#   ./agent-signal-listener.sh                    # wait for any signal
#   ./agent-signal-listener.sh --type DISTRIBUTE  # wait for specific type
#   ./agent-signal-listener.sh --timeout 60       # wait up to 60 seconds
#   ./agent-signal-listener.sh --peek             # check without waiting
#   ./agent-signal-listener.sh --consume          # read and delete all signals
#   ./agent-signal-listener.sh --mark-listening   # mark agent as listening
#
# Exit codes:
#   0 — signal received
#   1 — error
#   2 — timeout (no signal received)
#
# =============================================================================

set -euo pipefail

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SIGNAL_DIR_NAME=".agent-signals"

# Auto-detect: are we in a worktree?
if [[ -f ".agent-info" ]]; then
    AGENT_ROOT="$(pwd)"
elif [[ -f "${SCRIPT_DIR}/.agent-info" ]]; then
    AGENT_ROOT="${SCRIPT_DIR}"
else
    # Try to find it relative to git root
    GIT_ROOT="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
    if [[ -f "${GIT_ROOT}/.agent-info" ]]; then
        AGENT_ROOT="${GIT_ROOT}"
    else
        AGENT_ROOT="$(pwd)"
    fi
fi

SIGNAL_DIR="${AGENT_ROOT}/${SIGNAL_DIR_NAME}"
AGENT_INFO="${AGENT_ROOT}/.agent-info"

# Defaults
TIMEOUT=0          # 0 = wait forever
SIGNAL_TYPE=""     # empty = any type
MODE="wait"        # wait, peek, consume, mark

# ---------------------------------------------------------------------------
# Parse arguments
# ---------------------------------------------------------------------------
while [[ $# -gt 0 ]]; do
    case "$1" in
        --type)
            SIGNAL_TYPE="$2"
            shift 2
            ;;
        --timeout)
            TIMEOUT="$2"
            shift 2
            ;;
        --peek)
            MODE="peek"
            shift
            ;;
        --consume)
            MODE="consume"
            shift
            ;;
        --mark-listening)
            MODE="mark"
            shift
            ;;
        --signal-dir)
            SIGNAL_DIR="$2"
            shift 2
            ;;
        --help|-h)
            echo "Usage: $(basename "$0") [options]"
            echo ""
            echo "Options:"
            echo "  --type <TYPE>       Wait for specific signal type (DISTRIBUTE, HANDOFF, etc.)"
            echo "  --timeout <secs>    Maximum seconds to wait (0 = forever)"
            echo "  --peek              Check for signals without waiting or consuming"
            echo "  --consume           Read and delete all pending signals"
            echo "  --mark-listening    Mark this agent as actively listening"
            echo "  --signal-dir <dir>  Override signal directory path"
            echo "  --help              Show this help"
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            exit 1
            ;;
    esac
done

# ---------------------------------------------------------------------------
# Ensure signal directory exists
# ---------------------------------------------------------------------------
mkdir -p "${SIGNAL_DIR}"

# ---------------------------------------------------------------------------
# Utility: get agent name
# ---------------------------------------------------------------------------
get_agent_name() {
    if [[ -f "${AGENT_INFO}" ]]; then
        grep '^name=' "${AGENT_INFO}" | cut -d= -f2
    else
        echo "unknown-agent"
    fi
}

# ---------------------------------------------------------------------------
# Utility: list signal files (optionally filtered by type)
# ---------------------------------------------------------------------------
list_signals() {
    if [[ -n "${SIGNAL_TYPE}" ]]; then
        find "${SIGNAL_DIR}" -name "*-${SIGNAL_TYPE}.signal" -type f 2>/dev/null | sort
    else
        find "${SIGNAL_DIR}" -name "*.signal" -type f 2>/dev/null | sort
    fi
}

# ---------------------------------------------------------------------------
# Utility: read and output a signal file
# ---------------------------------------------------------------------------
read_signal() {
    local signal_file="$1"
    local filename
    filename=$(basename "${signal_file}")

    echo "---"
    echo "signal_file: ${filename}"
    cat "${signal_file}"
    echo ""
}

# ---------------------------------------------------------------------------
# Mode: mark-listening
# ---------------------------------------------------------------------------
if [[ "${MODE}" == "mark" ]]; then
    local_name=$(get_agent_name)
    touch "${SIGNAL_DIR}/.listening"
    echo "Agent ${local_name} marked as listening in ${SIGNAL_DIR}"
    exit 0
fi

# ---------------------------------------------------------------------------
# Mode: peek — show pending signals without consuming
# ---------------------------------------------------------------------------
if [[ "${MODE}" == "peek" ]]; then
    signals=$(list_signals)
    if [[ -z "${signals}" ]]; then
        echo "NO_SIGNALS"
        exit 2
    fi

    count=$(echo "${signals}" | wc -l | tr -d ' ')
    echo "SIGNALS_PENDING: ${count}"
    echo ""

    while IFS= read -r sig; do
        read_signal "${sig}"
    done <<< "${signals}"

    exit 0
fi

# ---------------------------------------------------------------------------
# Mode: consume — read and delete all pending signals
# ---------------------------------------------------------------------------
if [[ "${MODE}" == "consume" ]]; then
    signals=$(list_signals)
    if [[ -z "${signals}" ]]; then
        echo "NO_SIGNALS"
        exit 2
    fi

    count=$(echo "${signals}" | wc -l | tr -d ' ')
    echo "CONSUMING: ${count} signal(s)"
    echo ""

    while IFS= read -r sig; do
        read_signal "${sig}"
        rm -f "${sig}"
    done <<< "${signals}"

    # Clear trigger
    rm -f "${SIGNAL_DIR}/.trigger"

    echo "ALL_CONSUMED"
    exit 0
fi

# ---------------------------------------------------------------------------
# Mode: wait — block until a signal arrives
# ---------------------------------------------------------------------------

# Mark as listening
touch "${SIGNAL_DIR}/.listening"
trap 'rm -f "${SIGNAL_DIR}/.listening"' EXIT INT TERM

agent_name=$(get_agent_name)
type_filter=""
if [[ -n "${SIGNAL_TYPE}" ]]; then
    type_filter=" (type: ${SIGNAL_TYPE})"
fi

echo "WAITING: Agent ${agent_name} listening for signals${type_filter}..."

elapsed=0
check_interval=1  # check every 1 second

# Use fswatch if available for instant detection, otherwise poll
if command -v fswatch &>/dev/null && [[ "${TIMEOUT}" -eq 0 || "${TIMEOUT}" -gt 5 ]]; then
    # Use fswatch on the signal directory for instant detection
    # But also check existing signals first
    signals=$(list_signals)
    if [[ -n "${signals}" ]]; then
        echo ""
        first_signal=$(echo "${signals}" | head -1)
        read_signal "${first_signal}"
        rm -f "${first_signal}"
        rm -f "${SIGNAL_DIR}/.trigger"
        echo "SIGNAL_RECEIVED"
        exit 0
    fi

    # Watch for new files with optional timeout
    if [[ "${TIMEOUT}" -gt 0 ]]; then
        timeout_flag="--timeout=${TIMEOUT}"
    else
        timeout_flag=""
    fi

    # fswatch blocks until a change is detected
    # We watch for the .trigger file which the watcher touches on every signal
    fswatch -1 ${timeout_flag} "${SIGNAL_DIR}" 2>/dev/null || {
        echo "TIMEOUT: No signal received within ${TIMEOUT}s"
        exit 2
    }

    # A change was detected — read the signal
    signals=$(list_signals)
    if [[ -n "${signals}" ]]; then
        first_signal=$(echo "${signals}" | head -1)
        read_signal "${first_signal}"
        rm -f "${first_signal}"
        rm -f "${SIGNAL_DIR}/.trigger"
        echo "SIGNAL_RECEIVED"
        exit 0
    fi
else
    # Polling fallback
    while true; do
        signals=$(list_signals)
        if [[ -n "${signals}" ]]; then
            echo ""
            first_signal=$(echo "${signals}" | head -1)
            read_signal "${first_signal}"
            rm -f "${first_signal}"
            rm -f "${SIGNAL_DIR}/.trigger"
            echo "SIGNAL_RECEIVED"
            exit 0
        fi

        if [[ "${TIMEOUT}" -gt 0 ]] && [[ "${elapsed}" -ge "${TIMEOUT}" ]]; then
            echo "TIMEOUT: No signal received within ${TIMEOUT}s"
            exit 2
        fi

        sleep "${check_interval}"
        elapsed=$((elapsed + check_interval))
    done
fi
