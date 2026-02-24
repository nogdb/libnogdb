#!/usr/bin/env bash
# agent-register.sh — Called by /agent-01-coordination start (Step 2 + 3a)
# Usage: agent-register.sh <role> <main-repo-root>
# Writes .agent-info, creates .agent-signals/, registers in comm-log atomically.

set -euo pipefail

ROLE="$1"
MAIN="$2"
CWD=$(pwd)
BASE=$(basename "$CWD")

# Detect if running from a worktree (basename must match <name>-<8hex>)
if [[ ! "$BASE" =~ -[0-9a-f]{8}$ ]]; then
    echo "ERROR: Not in a git worktree (cwd=$CWD)."
    echo "       Run /agent-01-coordination start from a Windsurf worktree tab, not the main repo tab."
    exit 1
fi

HASH=${BASE: -8}
AGENT_NAME="${ROLE}-${HASH}"
TS=$(date -u +%Y-%m-%dT%H:%M:%SZ)
COMM_LOG="${MAIN}/agent_workspace/communication-log.md"

# Step 2a — Write .agent-info to directory structure
AGENT_INFO_DIR="${CWD}/.agent-info/${BASE}"
mkdir -p "${AGENT_INFO_DIR}"
cat > "${AGENT_INFO_DIR}/agent-info" << EOF
name=${AGENT_NAME}
role=${ROLE}
worktree=${CWD}
project_root=${MAIN}
started=${TS}
EOF

# Step 2b — Create .agent-signals/
mkdir -p "${CWD}/.agent-signals"

# Step 2c — Symlink agent_workspace into worktree so comm-log is readable via relative path
if [[ ! -e "${CWD}/agent_workspace" ]]; then
    ln -s "${MAIN}/agent_workspace" "${CWD}/agent_workspace"
    echo "SYMLINK: ${CWD}/agent_workspace -> ${MAIN}/agent_workspace"
fi

# Step 3 — Bootstrap comm-log if missing
mkdir -p "${MAIN}/agent_workspace"
if [[ ! -f "$COMM_LOG" ]]; then
    cat > "$COMM_LOG" << 'TMPL'
# Agent Communication Log

> **Purpose**: Central coordination point for all agents in the workspace.
> **Rules**: Append-only. Never edit or delete previous entries (except Status column updates).

---

## Registered Agents

| Agent Name | Role | Registered At | Status | Session Info |
|------------|------|---------------|--------|--------------|

---

## Messages

<!-- Agents append messages below this line -->
TMPL
fi

# Step 3a — Register atomically (awk + mv)
TMP="${COMM_LOG}.tmp.$$"
awk -v name="$AGENT_NAME" -v role="$ROLE" -v ts="$TS" -v cwd="$CWD" '
  /^\|[-| ]+\|$/ && !inserted {
    print
    print "| " name " | " role " | " ts " | \xf0\x9f\x9f\xa2 Online | worktree=" cwd " |"
    inserted=1
    next
  }
  { print }
' "$COMM_LOG" > "$TMP"

cat >> "$TMP" << MSGEOF

---

### [${TS}] — ${AGENT_NAME} — REGISTERED

| Field | Value |
|-------|-------|
| **Action** | Registered |
| **Role** | ${ROLE} |
| **Status** | 🟢 Online |
| **Notes** | Agent started. Ready to work. |
MSGEOF

mv "$TMP" "$COMM_LOG"

echo "REGISTERED: ${AGENT_NAME}"
echo "INFO: ${AGENT_INFO_DIR}/agent-info"
echo "SIGNALS: ${CWD}/.agent-signals/"
