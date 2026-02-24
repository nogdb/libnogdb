---
description: Multi-agent coordination workflow for parallel swarm execution (standalone — works with any project methodology)
---

<!--
  COMMAND ROUTING — Cascade reads this file top-to-bottom.
  The first section it encounters is what it executes.
  Commands are listed in order of most-common usage.
  Reference documentation is at the bottom of the file.
-->

## `start` — Execute when $ARGUMENTS is "start"

> Run these steps NOW if `$ARGUMENTS` equals `start`. Do not scan docs, do not ask for a project idea. Execute the numbered steps below in order.

**Step 0 — Detect worktree**

Run `run_command` (single line):
```bash
CWD=$(pwd) && BASE=$(basename "$CWD") && [[ "$BASE" =~ -[0-9a-f]{8}$ ]] && echo "CWD=$CWD" && echo "HASH=${BASE: -8}" && echo "MAIN=$HOME/${BASE%-*}" || echo "ERROR=not_a_worktree"
```

Parse output:
- If output contains `ERROR=not_a_worktree` → **stop**. Tell the user: *"This tab is the main repo, not a worktree. Open a Windsurf worktree tab and run `/agent-coordination start` from there."* Do not proceed.
- `CWD=` line gives `{cwd}` — the current worktree directory.
- `HASH=` line gives the **canonical 8-char hash** (e.g. `40852dbb` from `neuchat-40852dbb`).
- `MAIN=` line gives `{main-repo-root}` (e.g. `/Users/part/neuchat`).
- Continue to Step 1.

**Step 1 — Determine role**

Ask the user (every time — do NOT skip) using `ask_user_question` with the following numbered options:

```
What role should this agent take?
  1) orchestrator
  2) backend
  3) frontend
  4) unit-test
  5) general
```

Wait for the user's reply. If they type a number, map it to the corresponding role name. If they type a custom role not in the list, use it as-is. Use the resolved value as `{role}`.

Compute agent name: `{role}-{last8}` (e.g. `backend-0582ea97`).

**Step 2 — Register agent** (writes .agent-info, creates .agent-signals/, bootstraps comm-log, registers atomically)

Use `run_command` (Blocking: true). Substitute `{role}` and `{main-repo-root}` with actual values:

```bash
{main-repo-root}/.windsurf/workflow_script/agent-register.sh "{role}" "{main-repo-root}"
```

Expected output:
```
REGISTERED: {role}-{last8}
INFO: {cwd}/.agent-info
SIGNALS: {cwd}/.agent-signals/
```

**Step 3b — Spawn watcher**

Use `run_command` to check liveness:
```bash
PID_FILE="{cwd}/.agent-signals/watcher.pid"
if [[ -f "$PID_FILE" ]] && kill -0 "$(cat "$PID_FILE")" 2>/dev/null; then
  echo "WATCHER_ALIVE"
else
  echo "WATCHER_DEAD"
fi
```

If output is `WATCHER_DEAD`, use `run_command` (non-blocking, `Blocking: false`):
```bash
WATCHER="{main-repo-root}/.windsurf/workflow_script/agent-tab-watcher.sh"
LOG="{cwd}/.agent-signals/watcher.log"
nohup bash "$WATCHER" --agent "{role}-{last8}" --role "{role}" --root "{main-repo-root}" > "$LOG" 2>&1 &
echo $! > "{cwd}/.agent-signals/watcher.pid"
echo "Watcher PID: $!"
```

Output confirmation:
```
✅ Agent setup complete
  Name:      {role}-{last8}
  Role:      {role}
  Worktree:  {cwd}
  Signals:   {cwd}/.agent-signals/
  Watcher:   {cwd}/.agent-signals/watcher.log
  Comm-log:  {main-repo-root}/agent_workspace/communication-log.md
  Status:    🟢 Online
```

**Step 4 — Scan for work**

Read `{main-repo-root}/agent_workspace/communication-log.md`.
- If any `DISTRIBUTE` messages contain tasks assigned to this agent's name or role that have no `STARTED` entry → execute them (follow `do` command steps in the reference section below).
- If no pending tasks → run the Environment Scan (Mode 1 in the reference section below) to determine current phase and suggest next action.

---

## `work` — Execute when $ARGUMENTS is "work"

> Run these steps NOW if `$ARGUMENTS` equals `work`.

**Step 1** — Read `agent_workspace/communication-log.md`. Verify this agent is registered and 🟢 Online. If not, tell user to run `start` first.

**Step 2 — Work loop**

Repeat until no pending tasks remain:

1. Re-read `agent_workspace/communication-log.md` (full re-read each cycle).
2. Find all `DISTRIBUTE` entries. Extract tasks assigned to this agent (by name or role).
3. Filter out tasks that already have a `STARTED` or `COMPLETED` entry from this agent.
4. If pending tasks found:
   - Pick the first pending task.
   - Append `STARTED` message to comm-log.
   - Execute the task (implement, test, review, or document as appropriate).
   - Append `COMPLETED` (or `BLOCKED`) message to comm-log.
   - Go back to step 1.
5. If no pending tasks → check for worktree signals (Step 3).

**Step 3 — Wait for signals or exit**

If `.agent-info` exists (running in a worktree):
- Use `run_command` (non-blocking): `.windsurf/workflow_script/agent-signal-listener.sh --timeout 120`
- Use `command_status` with `WaitDurationSeconds: 60`.
- If signal received → re-read comm-log and go back to Step 2.
- If timeout → output idle status and exit.

Output when idle:
```
🔄 Work complete for {agent-name}.
- Tasks executed: {count}
- Status: Idle — no pending work for {role} role.

To check for new assignments: /agent-coordination work
```

---

## Other Commands — Execute when $ARGUMENTS matches

- **`$ARGUMENTS` = `team "<description>"`** → See `team` command in reference section below.
- **`$ARGUMENTS` = `status`** → See `status` command in reference section below.
- **`$ARGUMENTS` = `stop`** → See `stop` command in reference section below.
- **`$ARGUMENTS` = `register: ...`** → See `register` command in reference section below.
- **`$ARGUMENTS` = `do: ...`** → See `do` command in reference section below.
- **`$ARGUMENTS` = `distribute: ...`** → See `distribute` command in reference section below.
- **`$ARGUMENTS` = `logout: ...`** → See `logout` command in reference section below.
- **`$ARGUMENTS` = `clear`** → See `clear` command in reference section below.
- **`$ARGUMENTS` is empty** → Run Mode 1: Auto-Detect (Environment Scan) in reference section below.

---

# Reference Documentation

> Everything below is reference material. Do NOT execute it unless routed here by the commands above.

# Multi-Agent Coordination Workflow

Orchestrate multiple AI agents working **in parallel swarms** for complex feature development. This workflow is **standalone** — it works with any spec pipeline or project methodology.

---

## Quick Start

```bash
# Option A: Smart start (auto-detects role, registers, starts working)
/agent-coordination start

# Option B: Collaborative execution (auto-orchestrates across agents)
/agent-coordination team "implement user authentication feature"

# Option C: Explicit registration + work
/agent-coordination register: "Alice" role: "backend"
/agent-coordination work
```

---

## Usage

```
/agent-coordination [<command>] [: "<value>"] [options]
```

### Command Summary

| Command | Syntax | Description |
|---------|--------|-------------|
| `start` | `start` | Auto-detect role, register, begin working |
| `work` | `work` | Pick up assigned tasks and auto-continue until idle |
| `team` | `team "<description>"` | Auto-orchestrate multi-agent collaboration |
| `status` | `status` | Show all agents, tasks, and coordination health |
| `stop` | `stop` | Clean shutdown |
| `register` | `register: "<name>" [role: "<role>"]` | Manual registration with explicit name/role |
| `do` | `do: "<task IDs or description>"` | Execute specific task(s) |
| `distribute` | `distribute: "<assignments>"` | Assign tasks to agents |
| `logout` | `logout: "<name>"` | Mark agent offline |
| `clear` | `clear` | Reset communication log |

### Mode 1: Auto-Detect (no command)

```
/agent-coordination
```

When invoked with no command, the workflow scans the project environment and determines what to do next.

#### Environment Scan Steps

1. **Check `agent_workspace/`** — if missing, bootstrap it (create directory + communication log template)
2. **Read `agent_workspace/communication-log.md`** — check registered agents, recent messages, blockers, pending handoffs
3. **Scan `.windsurf/rules/`** — read project rules for coding standards, conventions, and constraints
4. **Scan project root** for source of truth:

```
Scan project root
    │
    ├── 1. AGENTS.md exists?
    │       ├── Yes → Extract: tech stack, directory structure, conventions, module map
    │       └── No  → Flag: needs generation
    │
    ├── 2. Package / dependency files?
    │       Scan for: package.json, Cargo.toml, go.mod, pyproject.toml,
    │       pom.xml, build.gradle, Gemfile, composer.json, pubspec.yaml, etc.
    │       → Extract: language, framework, dependencies, monorepo structure
    │
    ├── 3. Directory structure?
    │       Scan top-level + 2 levels deep
    │       → Identify: backend dir, frontend dir, shared/common, tests, docs,
    │          infra, migrations, configs
    │
    ├── 4. Existing specs / docs?
    │       Scan: docs/, specs/, .github/, wiki/, README.md
    │       → Identify: existing specs, ADRs, API docs, OpenAPI/Swagger files,
    │          ERD diagrams, Figma links, Storybook
    │
    ├── 5. Config files?
    │       Scan for: .env.example, docker-compose.yml, Dockerfile,
    │       tsconfig.json, eslint/prettier configs, CI configs (.github/workflows,
    │       .gitlab-ci.yml, Jenkinsfile), terraform/, k8s/
    │       → Extract: runtime, deployment model, CI/CD pipeline, env vars
    │
    ├── 6. Database / persistence?
    │       Scan for: migrations/, prisma/schema.prisma, drizzle/, sqlx/,
    │       diesel.toml, alembic/, knexfile, ormconfig, schema.rb
    │       → Extract: DB type, ORM, migration tool, existing schema
    │
    ├── 7. Git history (lightweight)?
    │       Read: git log --oneline -20, git branch -a, git tag -l
    │       → Extract: recent activity, branch strategy, release tags,
    │          active contributors
    │
    └── 8. Compile results → Output: docs/environment-scan.md
```

5. **Read task tracking files** — find tasks by status: 🔴 Not Started, 🟡 In Progress, ⚫ Blocked
6. **Determine current state** and **report findings** with suggested next actions.

> **Environment scan runs once.** After `docs/environment-scan.md` exists, subsequent invocations skip the project root scan and go straight to state detection.

### Mode 2: Command (explicit action)

```
/agent-coordination <command>: "<value>" [options]
```

When invoked with a command, the workflow executes that specific action. See Command Reference below.

### Examples

```
# Auto-detect
/agent-coordination

# Smart start
/agent-coordination start

# Explicit commands
/agent-coordination register: "Alice" role: "orchestrator"
/agent-coordination register: "Bob" role: "backend"
/agent-coordination do: "P1-T001,P1-T002"
/agent-coordination do: "implement user login API per spec"
/agent-coordination do: "continue"
/agent-coordination work
/agent-coordination team "implement admin mail configuration feature"
/agent-coordination status
/agent-coordination distribute: "P1-T001 → Backend Agent, P1-T002 → Frontend Agent"
/agent-coordination stop
/agent-coordination clear
```

---

## Windsurf Integration

This workflow leverages Windsurf's advanced features for effective multi-agent coordination:

- **Memories**: Store phase completion context, architecture decisions, and agent session state across conversations. Use `create_memory` after completing each phase to persist progress.
- **Rules**: Read `.windsurf/rules/` during environment scan to apply project coding standards, commit conventions, and quality constraints to all agent work.
- **Nested Workflow Triggers**: Use `Call /workflow-name` to chain workflows automatically within phases.
- **Cascade Queued Messages**: Queue sequential tasks for unattended execution.
- **DeepWiki**: During implementation, agents working on unfamiliar code should use DeepWiki (`Cmd+Shift+Click`) to understand existing symbols before modifying them.
- **MCP Servers**: If configured, leverage MCP for database access during implementation and API testing.

---

## Git Worktree Signaling (Real-Time Agent Communication)

By default, agents discover new work by re-reading `communication-log.md` during their work loop. This is **pull-based** — agents only see changes when they actively check. To enable **push-based** real-time signaling between Cascade sessions, use the external worktree watcher scripts.

### Problem

Cascade sessions have no inter-process communication. When Agent A posts a DISTRIBUTE message, Agent B won't know until it manually re-reads the log.

### Solution: Worktree Watcher

An external script (`agent-worktree-watcher.sh`) runs in a standalone terminal, watches `communication-log.md` for changes via `fswatch`, and writes `.signal` files into each agent's git worktree. Each agent runs in its own worktree (separate Windsurf window) and can block on `agent-signal-listener.sh` to wait for signals.

```
Main Repo
  │
  ├── agent_workspace/communication-log.md  ◄── all agents write here
  │         │
  │         │  fswatch (real-time)
  │         ▼
  │  agent-worktree-watcher.sh  (standalone terminal)
  │         │
  │         ├──► .agent-worktrees/backend-01/.agent-signals/*.signal
  │         ├──► .agent-worktrees/frontend-01/.agent-signals/*.signal
  │         └──► .agent-worktrees/test-01/.agent-signals/*.signal
  │
  └── .agent-worktrees/
        ├── backend-01/    ← git worktree, own branch, own Windsurf window
        ├── frontend-01/
        └── test-01/
```

### Scripts Location

```
template/windsurf_template/workflow_script/
├── agent-worktree-watcher.sh    # External watcher daemon (runs outside Cascade)
├── agent-signal-listener.sh     # Signal listener (runs inside Cascade via run_command)
└── README.md                    # Full documentation
```

### Setup Steps

**Step 1 — Create agent worktrees** (run in a regular terminal, not Cascade):

```bash
# From the project root
./template/windsurf_template/workflow_script/agent-worktree-watcher.sh --setup backend-01 backend
./template/windsurf_template/workflow_script/agent-worktree-watcher.sh --setup frontend-01 frontend
./template/windsurf_template/workflow_script/agent-worktree-watcher.sh --setup test-01 unit-test
```

This creates a git worktree per agent at `.agent-worktrees/<name>/` with:
- Its own branch (`agent/<name>`)
- `.agent-info` file (name, role, metadata)
- `.agent-signals/` directory (signal inbox)
- Symlinks to shared `agent_workspace/` and `docs/`

**Step 2 — Start the watcher daemon**:

```bash
./template/windsurf_template/workflow_script/agent-worktree-watcher.sh
```

**Step 3 — Open each worktree in its own Windsurf window**:

```bash
windsurf .agent-worktrees/backend-01
windsurf .agent-worktrees/frontend-01
```

**Step 4 — Each agent runs** `/agent-coordination start` in its Windsurf session.

### Signal Types

| Signal | Trigger | Meaning |
|--------|---------|--------|
| `DISTRIBUTE` | New task assignment posted | Agent has new work to pick up |
| `HANDOFF` | Task completed with handoff target | Previous task done, this agent's turn |
| `BLOCKER_RESOLVED` | Blocker cleared | Previously blocked work can resume |
| `PHASE_ADVANCE` | Phase gate passed | New phase starting |

### How `start` and `work` Use Signals

When running in a worktree (detected by presence of `.agent-info`):

1. **`start`** reads `.agent-info` to auto-detect name and role (skips heuristic detection)
2. **`work`** loop, after completing all tasks and finding no more work:
   - Instead of exiting idle, runs `agent-signal-listener.sh --timeout 120` as a non-blocking command
   - When a signal arrives, re-reads the communication log and continues the work loop
   - On timeout (no signal in 120s), outputs idle status and exits

This turns the exit-on-idle behavior into a **wait-and-react** behavior.

### Tab Mode — Single Windsurf Window (Recommended for Local Dev)

Run any number of parallel Cascade agents inside **one Windsurf window** using Windsurf's built-in worktree tabs. Windsurf **automatically creates a git worktree** when you open a new Cascade tab in worktree mode — no pre-setup needed.

| Command | Runs in | Who triggers it |
|---------|---------|---------------|
| `/agent-coordination start` | **Cascade chat panel** in each worktree tab | You, once per tab |
| `launch-multi-agent.sh` | System terminal (outside Windsurf) | You, once for all agents |

#### Step 1 — Open a new Cascade tab in worktree mode \[Windsurf UI\]

Windsurf creates the worktree automatically. Repeat for each agent you want to run.

#### Step 2 — Start the agent \[Cascade chat in each worktree tab\]

In the **Cascade chat panel** of each worktree tab:

```
/agent-coordination start
```

When `.agent-info` is not found, `start` self-initializes:
- Asks for the agent's role
- Writes `.agent-info` and creates `.agent-signals/`
- **Auto-spawns `agent-tab-watcher.sh` in the background** (no terminal action needed)
- Registers in `communication-log.md` and begins the work loop

#### Step 3 — Start the coordination driver \[system terminal, once\]

In a **system terminal** (outside Windsurf), after all tabs have registered:

```bash
.windsurf/workflow_script/launch-multi-agent.sh
```

This watches the comm-log and routes signals to each agent's `.agent-signals/` inbox.

#### Teardown

```bash
.windsurf/workflow_script/cleanup-multi-agent.sh
```

Stops all watchers, clears signals, removes `.agent-info`, and resets `communication-log.md` to a blank template.

---

### Fallback

If the watcher is not running or agents are not in worktrees, everything falls back to the original pull-based behavior. The worktree signaling is an **optional enhancement**, not a requirement.

---

## Communication Log Template

All commands that create or reset the communication log MUST use this template:

```markdown
# Agent Communication Log

> **Purpose**: Central coordination point for all agents in the workspace. Agents register here on startup and post messages to communicate with each other.
> **Rules**: Append-only. Never edit or delete previous entries (except Status column updates for register/logout).

---

## Registered Agents

| Agent Name | Role | Registered At | Status | Session Info |
|------------|------|---------------|--------|--------------|

---

## Messages

<!-- Agents append messages below this line -->
```

---

## Command Reference

### `start` — Smart Auto-Detect Registration

**Syntax:** `/agent-coordination start`

This is the **recommended entry point** for any agent session. It auto-detects the agent's role, registers, and begins working — all in one command.

#### Executable Steps

**Step 1 — Detect role from project context**

0. **Worktree detection gate**: Automatically detect whether this Cascade session is running in a dedicated git worktree.

   **Sub-step 1** — Detect worktree via git: use `run_command`:
   ```bash
   pwd && git worktree list
   ```
   Parse the output. Match the `pwd` result against the worktree list.
   Windsurf agent worktrees appear as non-first rows with paths like `<project>-<hash>` and branches like `cascade/<description>-<hash>`.

   - Extract the last 8 chars of `cwd` path as the **canonical hash** (e.g. `aef9e332` from `neuchat-aef9e332`). This hash is the **canonical agent identity** for this session — use it regardless of what `.agent-info` contains.

   **Sub-step 2** — Check for returning agent: use `read_file` to check if `<cwd>/.agent-info` exists.
   - **If exists** → read `role=` from it (to skip the role question). The agent name is still `{role}-{last8}` from Sub-step 1 — do NOT use the `name=` value from `.agent-info` (it may be stale). Mark this session as **returning** (skip Step 3b watcher spawn only if watcher.pid already exists and process is alive).
   - **If missing** → fresh init. Ask for role (step a below), write `.agent-info`, create `.agent-signals/`.

   a. Use `ask_user_question` to ask: *"What role should this agent take?"* with options presented as a numbered list:

      ```
      1) orchestrator
      2) backend
      3) frontend
      4) unit-test
      5) general
      ```

      Accept a number (1–5) or a custom role name typed freely. Map numbers to role names before proceeding. (Skip entirely if role was already read from `.agent-info` or passed as an argument.)

   b. **Deterministic agent name**: `{role}-{last8}` (e.g. `backend-aef9e332`). Always derived from the worktree hash — stable across restarts, collision-free across simultaneous agents.

   c. Use `write_to_file` to create `<cwd>/.agent-info`:
      ```
      name={agent-name}
      role={role}
      worktree={cwd}
      project_root={main-repo-root}
      started={ISO-8601-timestamp}
      ```
      Where `{main-repo-root}` is the path of the **first** entry from `git worktree list` (the main worktree, e.g. `/Users/part/neuchat`).

   d. Use `run_command` to create the signal directory: `mkdir -p <cwd>/.agent-signals`

   Continue to Step 2 (bootstrap workspace) and Step 3 (register in comm-log). After registration is complete, proceed to Step 3b (spawn watcher + confirm setup), then Step 4.

1. Use `list_dir` on `<project_root>` (top-level + 2 levels deep).
2. Use `run_command` to read `git branch --show-current` and `git log --oneline -5`.
3. Use `grep_search` to scan for file patterns:
   - Frontend files (`.tsx`, `.jsx`, `.css`, `.scss`, `components/`) → `frontend` role
   - Backend files (`.rs`, `.go`, `.py`, `.ts` in `src/api/`, `src/server/`, `src/modules/`) → `backend` role
   - Test files (`.test.ts`, `.spec.ts`, `__tests__/`) → `unit-test` role
   - Spec/doc files (`docs/`, `specs/`) → `spec` role
   - Design files (`figma`, `tokens`, `theme`) → `design` role
4. Check git branch name:
   - `feature/*` → implementation role (backend or frontend based on files)
   - `test/*` → testing role
   - `docs/*` → documentation role
5. If project is a monorepo (multiple `package.json` or workspace config) → `orchestrator` role
6. If ambiguous, use `ask_user_question` with top 3 detected roles as options.

**Step 2 — Bootstrap workspace**

1. Use `list_dir` to check if `<project_root>/agent_workspace/` exists.
2. **If missing** → use `run_command` with `mkdir -p agent_workspace/`.
3. Use `read_file` to check `agent_workspace/communication-log.md`.
4. **If missing** → use `write_to_file` with the Communication Log Template above.
5. **If exists** → verify it contains `## Registered Agents` and `## Messages`. If malformed, report and STOP.

**Step 3 — Register in comm-log**

> **Always run this step** — every agent session must register, even returning agents. Never skip because the agent name already appears in the log (each session is a new registration with a fresh timestamp).

1. Agent name is `{role}-{last8}` from Step 0. Use that name — do NOT regenerate.

2. **Stagger startup** to reduce simultaneous write collisions: use `run_command`:
   ```bash
   sleep $(( (RANDOM % 3) + 1 ))
   ```

3. **Acquire comm-log write lock** before appending: use `run_command`:
   ```bash
   LOCK=/tmp/neuchat-comm-log.lock
   while ! mkdir "$LOCK" 2>/dev/null; do sleep 0.2; done
   ```

4. Read `agent_workspace/communication-log.md`. Find the `## Registered Agents` table. Use `edit` to **append** a new row (do not update or deduplicate existing rows):
   ```
   | {agent-name} | {role} | {ISO-8601 timestamp} | 🟢 Online | worktree={cwd} |
   ```

5. Append a REGISTERED message at the end of the file using `edit`.

6. **Release the lock**: use `run_command`:
   ```bash
   rm -rf /tmp/neuchat-comm-log.lock
   ```

**Step 3b — Spawn watcher and confirm setup**

> Run for fresh agents. For returning agents: check if `<cwd>/.agent-signals/watcher.pid` exists AND `kill -0 $(cat <cwd>/.agent-signals/watcher.pid) 2>/dev/null` succeeds (watcher already alive). If alive, skip spawn. Otherwise spawn it.

1. Use `run_command` to check if watcher is already running:
   ```bash
   PID_FILE="<cwd>/.agent-signals/watcher.pid"
   if [[ -f "$PID_FILE" ]] && kill -0 "$(cat "$PID_FILE")" 2>/dev/null; then
     echo "WATCHER_ALIVE"
   else
     echo "WATCHER_DEAD"
   fi
   ```

2. If output is `WATCHER_DEAD`, use `run_command` (non-blocking, `Blocking: false`) to spawn the watcher:
   ```bash
   WATCHER="{project_root}/.windsurf/workflow_script/agent-tab-watcher.sh"
   LOG="<cwd>/.agent-signals/watcher.log"
   nohup bash "$WATCHER" --agent "{agent-name}" --role "{role}" --root "{project_root}" > "$LOG" 2>&1 &
   echo $! > "<cwd>/.agent-signals/watcher.pid"
   echo "Watcher PID: $!"
   ```
   Replace `{project_root}`, `{agent-name}`, and `{role}` with the actual values from this session.

   The watcher runs independently and survives the Cascade session. Its output is at `<cwd>/.agent-signals/watcher.log`.

3. Output the setup confirmation:
   ```
   ✅ Agent setup complete
     Name:      {agent-name}
     Role:      {role}
     Info:      <cwd>/.agent-info
     Signals:   <cwd>/.agent-signals/
     Watcher:   started (background) → <cwd>/.agent-signals/watcher.log
     Comm-log:  agent_workspace/communication-log.md
     Registered: 🟢 Online
   ```

**Step 4 — Scan for existing work and begin**

1. Read the communication log for any DISTRIBUTE messages with tasks assigned to this role.
2. If pending tasks found → automatically execute them (follow `do` command flow).
3. If no pending tasks → run the Environment Scan (Mode 1) to determine current state and suggest next action.
4. Output:

```
✅ Smart registration complete.
- Detected role: {role}
- Agent name: {agent-name}
- Context: {detected file types, branch, structure}
- Pending tasks: {count or "none — ran environment scan"}

Ready. Use `team` for collaboration, `do` for specific tasks, or `work` to pick up assignments.
```

---

### `work` — Pick Up Tasks and Auto-Continue

**Syntax:** `/agent-coordination work`

Reads the communication log, picks up any tasks assigned to this agent (via DISTRIBUTE messages) that haven't been STARTED yet, executes them sequentially, and continues until no pending work remains.

#### Executable Steps

**Step 1 — Verify registration**

1. Read `agent_workspace/communication-log.md`.
2. Verify this agent is registered and 🟢 Online. If not, ask user to `register` or `start` first.

**Step 2 — Enter the work loop**

```
WORK LOOP:
┌─────────────────────────────────────────────────────────────────┐
│ 1. Read agent_workspace/communication-log.md (full re-read)    │
│ 2. Scan for DISTRIBUTE messages                                │
│ 3. Extract tasks assigned to THIS agent (by name or role)      │
│ 4. Filter out tasks that already have a STARTED or COMPLETED   │
│    entry from this agent                                       │
│ 5. If pending tasks found:                                     │
│    ├── Pick the first pending task                             │
│    ├── Log STARTED message                                     │
│    ├── Execute the task (follow `do` command flow)             │
│    ├── Log COMPLETED (or BLOCKED) message                      │
│    └── Continue to next iteration (go to step 1)               │
│ 6. If NO pending tasks found:                                  │
│    ├── Output idle status                                      │
│    └── EXIT the loop                                           │
└─────────────────────────────────────────────────────────────────┘
```

**Step 3 — Task extraction logic**

1. Find all `DISTRIBUTE` entries in the Messages section.
2. Parse the `**Assignments**` field — format: `TASK_ID → Agent Name, ...`
3. Collect all task IDs assigned to this agent's name (case-insensitive match).
4. For each task ID, scan the log for any `STARTED` or `COMPLETED` entry from this agent.
5. Tasks with no STARTED entry are **pending**.

**Step 4 — Execute each pending task**

For each pending task, follow the `do` command flow:

1. Log a STARTED message (see message format in `do` command).
2. Execute the task — read relevant specs, generate code, write files, etc.
3. Log a COMPLETED message (or BLOCKED if stuck).
4. Re-read the log before next task (other agents may have added entries).

**Step 5 — Wait for signals or exit when idle**

1. **If running in a worktree** (`.agent-info` exists at project root):
   - Use `run_command` (non-blocking) to run `agent-signal-listener.sh --timeout 120` from the project root.
   - Use `command_status` with `WaitDurationSeconds: 60` to wait for the listener.
   - **If signal received** (exit code 0): parse the signal output, re-read `communication-log.md`, and **go back to Step 2** (re-enter the work loop).
   - **If timeout** (exit code 2): output idle status and EXIT.
   - **If listener not found**: fall back to exit-on-idle (step 2 below).

2. **If NOT in a worktree** (no `.agent-info`): exit immediately.

```
🔄 Work complete for {agent-name}.
- Tasks executed: {count}
- Worktree signals: {listening / not available}
- Status: Idle — no pending work for {role} role.

To check for new assignments later, run:
  /agent-coordination work
```

**Key Design Decisions:**
- **Full re-read each cycle** — the log may have been modified by other agents between cycles.
- **One task at a time** — execute sequentially to avoid conflicts.
- **Wait-on-idle in worktrees** — when running in a worktree with the watcher active, agents wait for signals instead of exiting immediately. This enables push-based reactivity.
- **Exit on idle without worktree** — without the watcher infrastructure, don't spin forever. The user can re-run `work` later.
- **Idempotent** — running `work` twice won't re-execute already-STARTED tasks.

---

### `team` — Intelligent Multi-Agent Collaboration

**Syntax:** `/agent-coordination team "<task description or task IDs>"`

Orchestrates multi-agent collaboration automatically. The calling agent becomes the Orchestrator (or finds an existing one), analyzes the work, distributes tasks to available agents by role, and begins execution.

#### Executable Steps

**Step 1 — Verify registration**

1. Read `agent_workspace/communication-log.md`.
2. Verify this agent is registered and 🟢 Online. If not, run `start` first.

**Step 2 — Find or become the Orchestrator**

1. Scan `## Registered Agents` for an agent with `orchestrator` role and 🟢 Online status.
2. If no orchestrator found → this agent becomes the Orchestrator for this session.

**Step 3 — Analyze work**

1. **If task IDs provided** (e.g., `"P1-T001,P1-T002,P2-T001"`):
   - Read task tracking files to look up each task's required role, dependencies, and priority
2. **If free-text description** (e.g., `"implement user authentication"`):
   - Read available specs and task files
   - Break into sub-tasks and identify required roles

**Step 4 — Distribute tasks**

1. Identify all 🟢 Online agents from the Registered Agents table.
2. Match tasks to agents by role.
3. Post a DISTRIBUTE message:

```markdown
---

### [{timestamp}] — {Orchestrator} — DISTRIBUTE

| Field | Value |
|-------|-------|
| **Action** | Distribute |
| **Triggered By** | team command from {agent-name} |
| **Status** | 🟡 In Progress |
| **Assignments** | P1-T001 → backend-09-30, P1-T002 → frontend-09-31 |
| **Notes** | {rationale for assignment} |
```

4. Post a COLLAB_READY message:

```markdown
---

### [{timestamp}] — {Orchestrator} — COLLAB_READY

| Field | Value |
|-------|-------|
| **Action** | Collaboration Ready |
| **Status** | 🟢 Distributed |
| **Agents Involved** | backend-09-30, frontend-09-31 |
| **Task Count** | {N} tasks across {M} agents |
| **Notes** | All agents should run `work` to pick up assignments. |
```

**Step 5 — Begin own work**

1. If this agent has assigned tasks → execute them (follow `do` command flow).
2. Output summary:

```
📋 Team collaboration initiated.
- Tasks distributed: {count}
- Agents involved: {list}
- This agent's tasks: {list or "none — orchestrating only"}

Other agents should run:
  /agent-coordination work
```

---

### `status` — Coordination Status Report

**Syntax:** `/agent-coordination status`

#### Executable Steps

1. Read `agent_workspace/communication-log.md`.
2. Parse `## Registered Agents` table → list all agents with status.
3. Scan `## Messages` → find last action per agent.
4. Count pending DISTRIBUTE assignments (not yet STARTED).
5. Output:

```
📊 Multi-Agent Coordination Status
┌──────────────────┬──────────────┬─────────────────────────┬──────────┬─────────────────────┐
│ Agent Name       │ Role         │ Registered At           │ Status   │ Last Action         │
├──────────────────┼──────────────┼─────────────────────────┼──────────┼─────────────────────┤
│ {name}           │ {role}       │ {timestamp}             │ {status} │ {last action}       │
└──────────────────┴──────────────┴─────────────────────────┴──────────┴─────────────────────┘

⚡ Pending Tasks: {count}
🟢 Completed Tasks: {count}
⚫ Blocked Tasks: {count}
```

---

### `stop` — Clean Agent Shutdown

**Syntax:** `/agent-coordination stop`

#### Executable Steps

1. Read `agent_workspace/communication-log.md`.
2. Find this agent's row in `## Registered Agents`.
3. Use `edit` to update Status from `🟢 Online` to `⚪ Offline`.
4. Append a SHUTDOWN message:

```markdown
---

### [{timestamp}] — {agent-name} — SHUTDOWN

| Field | Value |
|-------|-------|
| **Action** | Shutdown |
| **Status** | ⚪ Offline |
| **Tasks Completed** | {count from this session} |
| **Notes** | Agent session ended. |
```

5. Output: `🛑 {agent-name} is now offline. Use 'start' to begin a new session.`

---

### `register` — Bootstrap & Register

**Syntax:** `/agent-coordination register: "<agent-name>" [role: "<role>"]`

Manual registration with explicit name and role. Use `start` for auto-detection.

#### Role List

If `role` is omitted, use `ask_user_question` to present options:

| Role | Description | Used By |
|------|-------------|---------|
| `orchestrator` | Coordinates agents, distributes tasks, monitors progress | Orchestrator Agent |
| `backend` | Backend code generation | Cascade-Backend |
| `frontend` | Frontend code generation | Cascade-Frontend |
| `spec` | Generates specifications and design docs | Spec Agent |
| `design` | Figma token extraction, theme files, UI assets | Design Agent |
| `unit-test` | Unit test generation | Unit Test Agent |
| `integration-test` | Integration test generation | Integration Test Agent |
| `e2e-test` | End-to-end test generation | E2E Test Agent |
| `security-review` | Security analysis and review | Security Review Agent |
| `performance-review` | Performance analysis and review | Performance Review Agent |
| `standards-review` | Coding standards compliance review | Standards Review Agent |
| `api-doc` | API documentation generation | API Doc Agent |
| `readme` | README and guide updates | README Agent |
| `changelog` | Changelog entry generation | Changelog Agent |
| `general` | General-purpose agent (can do any task) | Any |

#### Executable Steps

**Step 1 — Check `agent_workspace/` directory**

1. Use `list_dir` or `find_by_name` to check if `<project_root>/agent_workspace/` exists.
2. **If missing** → create it with `mkdir -p agent_workspace/`.

**Step 2 — Check `agent_workspace/communication-log.md`**

1. Use `read_file` to read `<project_root>/agent_workspace/communication-log.md`.
2. **If missing** → create it with the Communication Log Template using `write_to_file`.
3. **If exists** → verify it contains `## Registered Agents` and `## Messages`. If malformed, report and STOP.

**Step 3 — Register self in the communication log**

1. Read `agent_workspace/communication-log.md`.
2. Find the `## Registered Agents` table. Locate the last `|` row.
3. Use `edit` to append a new row:
   ```
   | {agent-name} | {role} | {ISO-8601 timestamp} | 🟢 Online | Cascade session |
   ```
4. Append a REGISTERED message at the end of the file:

```markdown
---

### [{timestamp}] — {agent-name} — REGISTERED

| Field | Value |
|-------|-------|
| **Action** | Registered |
| **Role** | {role} |
| **Status** | 🟢 Online |
| **Notes** | Agent started. Ready to coordinate. |
```

**Step 4 — Report and STOP**

```
✅ Bootstrap complete.
- agent_workspace/ directory: OK
- communication-log.md: OK
- Registered as: "{agent-name}"
- Role: {role}
- Status: 🟢 Online

Awaiting instructions. Use `do`, `work`, `team`, or `stop`.
```

**Do NOT proceed to any other logic.** Wait for the user's next command.

---

### `do` — Execute Task(s)

**Syntax:** `/agent-coordination do: "<task description, task IDs, or options>"`

Executes one or more tasks. The agent must already be registered.

#### Supported Value Formats

| Format | Example | Behavior |
|--------|---------|----------|
| **Task IDs** | `"P1-T001,P1-T002"` | Look up tasks in task tracking files |
| **Free-text** | `"implement user login API"` | Treat as ad-hoc instruction |
| **Continue** | `"continue"` or `"next"` | Auto-detect next action via Environment Scan |

#### Executable Steps

1. Read `agent_workspace/communication-log.md` — verify this agent is registered and 🟢 Online.
2. **Parse the value** using the format table above.
3. **Log a STARTED message**:

```markdown
---

### [{timestamp}] — {agent-name} — STARTED

| Field | Value |
|-------|-------|
| **Action** | Started |
| **Task** | {task IDs or description} |
| **Status** | 🟡 In Progress |
| **Notes** | {brief plan of action} |
```

4. **Execute the work** — implement, test, review, or document as appropriate.
5. **Log a COMPLETED or BLOCKED message**:

```markdown
---

### [{timestamp}] — {agent-name} — COMPLETED

| Field | Value |
|-------|-------|
| **Action** | Completed |
| **Task** | {task IDs or description} |
| **Status** | 🟢 Done |
| **Output** | {files created/modified} |
| **Handoff To** | {next agent or None} |
| **Blockers** | None |
| **Notes** | {summary of what was done} |
```

---

### `distribute` — Assign Tasks to Agents

**Syntax:** `/agent-coordination distribute: "<assignment description>"`

Posts task assignments to the communication log so other agents can pick them up via `work`.

#### Executable Steps

1. Read `agent_workspace/communication-log.md` — verify this agent is registered and 🟢 Online.
2. **Parse the value**:
   - **Explicit mapping**: `"P1-T001 → Backend Agent, P1-T002 → Frontend Agent"`
   - **Auto-distribute**: `"P1-T001,P1-T002,P1-T003"` → assign to registered 🟢 Online agents round-robin by role
3. **Append** a DISTRIBUTE message at the end of the file:

```markdown
---

### [{timestamp}] — {agent-name} — DISTRIBUTE

| Field | Value |
|-------|-------|
| **Action** | Distribute |
| **Status** | 🟡 In Progress |
| **Assignments** | P1-T001 → Backend Agent, P1-T002 → Frontend Agent |
| **Notes** | {rationale for assignment} |
```

4. Output the assignment summary.

---

### `logout` — Mark Agent Offline

**Syntax:** `/agent-coordination logout: "<agent-name>"`

#### Executable Steps

1. Read `agent_workspace/communication-log.md`.
2. Find the agent's row in `## Registered Agents`.
3. Use `edit` to update Status from `🟢 Online` to `⚪ Offline`.
4. Append a LOGOUT message:

```markdown
---

### [{timestamp}] — {agent-name} — LOGOUT

| Field | Value |
|-------|-------|
| **Action** | Logout |
| **Status** | ⚪ Offline |
| **Notes** | Agent session ended. |
```

5. Output: `⚪ {agent-name} is now offline.`

---

### `clear` — Reset Communication Log

**Syntax:** `/agent-coordination clear`

Resets the communication log to its empty template. **Destructive** — all history is lost.

#### Executable Steps

**Step 1 — Confirm**

Use `ask_user_question`:
- **Yes, clear it** → proceed
- **Cancel** → abort

**Step 2 — Overwrite**

Replace entire contents of `agent_workspace/communication-log.md` with the Communication Log Template.

**Step 3 — Report**

```
🗑️ Communication log cleared.
- All registered agents removed
- All messages removed
- Log reset to empty template

Agents must re-register:
  /agent-coordination register: "<name>" role: "<role>"
  /agent-coordination start
```

---

## Available Agents

| Agent | Role | Concurrency | Triggers |
|-------|------|-------------|----------|
| **Spec Agent** | Generates specifications and design docs | Per-phase swarm | Feature request received |
| **Cascade-Backend** | Backend code generation | Per-phase swarm | Spec finalized, backend tasks identified |
| **Cascade-Frontend** | Frontend code generation | Per-phase swarm | Spec finalized, frontend tasks identified |
| **Design Agent** | Figma token extraction, theme files | Per-phase swarm | UI work identified |
| **Unit Test Agent** | Unit test generation | Per-phase swarm | New code written |
| **Integration Test Agent** | Integration test generation | Per-phase swarm | Services/repos implemented |
| **E2E Test Agent** | End-to-end test generation | Per-phase swarm | Feature complete |
| **Security Review Agent** | Security analysis | Per-phase swarm | Code ready for review |
| **Performance Review Agent** | Performance analysis | Per-phase swarm | Code ready for review |
| **Standards Review Agent** | Coding standards compliance | Per-phase swarm | Code ready for review |
| **API Doc Agent** | API documentation | Per-phase swarm | Endpoints finalized |
| **README Agent** | README & guides | Per-phase swarm | Feature complete |
| **Changelog Agent** | Changelog entries | Per-phase swarm | Feature merged |

---

## Architecture Overview

```
Oracle (Human) ─── strategic decisions only
    │
    ▼
Orchestrator Agent ─── automated task decomposition + phase gates
    │
    ├── Shared Artifact Bus (read/write intermediate results)
    │
    ├── Phase 0: Discovery ─────────── Oracle + Spec Agent
    │       │ (gate: project spec approved)
    │
    ├── Phase 1: Specification ────── [parallel Spec Agents]
    │       │ (gate: spec completeness ≥ 90%)
    │
    ├── Phase 2: Planning ──────────── Orchestrator + Oracle
    │       │ (gate: tasks allocated, test plan approved)
    │
    ├── Phase 3: Implementation ────── [parallel swarm]
    │       ├── Cascade-Backend
    │       ├── Cascade-Frontend
    │       ├── Design Agent
    │       └── Review Agent (streaming inline checks)
    │       │ (gate: builds pass, 0 type errors)
    │
    ├── Phase 4: Testing ───────────── [parallel swarm]
    │       ├── Unit Test Agent
    │       ├── Integration Test Agent
    │       └── E2E Test Agent
    │       │ (gate: coverage ≥ 80%)
    │
    ├── Phase 5: Final Review ──────── [parallel swarm]
    │       ├── Security Review Agent
    │       ├── Performance Review Agent
    │       └── Standards Review Agent
    │       │ (gate: 0 critical findings)
    │
    └── Phase 6: Documentation ─────── [parallel swarm]
            ├── API Doc Agent
            ├── README Agent
            └── Changelog Agent
            │ (gate: all artifacts updated)
```

---

## Orchestrator Agent

The Orchestrator replaces manual Oracle dispatch for routine coordination:

- **Decompose** — breaks tasks into parallelizable sub-tasks automatically
- **Fan-out** — dispatches sub-tasks to agent swarms concurrently
- **Fan-in** — collects and merges results when all swarm agents complete
- **Gate-check** — validates automated criteria before advancing phases
- **Conflict resolution** — flags contradictions for Oracle review
- **Status tracking** — updates task status in the project's task tracking file

The Oracle retains authority over strategic decisions, force-proceed overrides, and final sign-off.

---

## Shared Artifact Bus

Agents communicate through a shared artifact store — no direct agent-to-agent messaging required.

```
┌──────────────────────────────────────────────────────────────────────┐
│                        Shared Artifact Bus                          │
├──────────────────────┬───────────────────────────────────────────────┤
│  Artifact            │ Written By              │ Read By             │
├──────────────────────┼─────────────────────────┼─────────────────────┤
│  docs/spec-*.md      │ Spec Agent              │ All agents          │
│  docs/plan-*.md      │ Orchestrator            │ All agents          │
│  docs/test-plan.md   │ Orchestrator            │ Test Agent Swarm    │
│  backend/src/**      │ Cascade-Backend         │ Test + Review Agents│
│  frontend/src/**     │ Cascade-Frontend        │ Test + Review Agents│
│  design-tokens/      │ Design Agent            │ Cascade-Frontend    │
│  tests/unit/**       │ Unit Test Agent         │ Review Agents       │
│  tests/integ/**      │ Integration Test Agent  │ Review Agents       │
│  tests/e2e/**        │ E2E Test Agent          │ Review Agents       │
│  review-sec.md       │ Security Review Agent   │ Orchestrator        │
│  review-perf.md      │ Performance Review Agent│ Orchestrator        │
│  review-std.md       │ Standards Review Agent  │ Orchestrator        │
│  CHANGELOG.md        │ Changelog Agent         │ Oracle              │
│  docs/api/**         │ API Doc Agent           │ Oracle              │
│  README.md           │ README Agent            │ Oracle              │
│  agent_workspace/    │ All agents              │ All agents          │
│  communication-log.md│                         │                     │
└──────────────────────┴─────────────────────────┴─────────────────────┘
```

---

## Agent Communication Log

All agents write to a single **append-only** log file (`agent_workspace/communication-log.md`) as the central coordination point. This provides full traceability of who did what, when, and why — and lets any agent pick up context from where another left off.

The communication log serves dual purposes:
1. **Agent registry** — tracks which agents are online and available
2. **Message bus** — agents post structured messages to coordinate work, report status, and hand off tasks

### Log Location

```
agent_workspace/communication-log.md
```

### Entry Format

Every agent MUST append an entry when it:
- **Starts** a task
- **Completes** a task (with output summary)
- **Hands off** work to another agent
- **Encounters a blocker** or conflict
- **Requests** Oracle decision

```markdown
## [TIMESTAMP] — [AGENT NAME] — [ACTION]

| Field | Value |
|-------|-------|
| **Phase** | Phase 3: Implementation |
| **Task** | P1-T003 — User login API |
| **Action** | Completed / Started / Blocked / Handoff / Request |
| **Status** | 🟢 Done / 🟡 In Progress / ⚫ Blocked |
| **Output** | `backend/src/modules/auth/login.service.ts` created |
| **Handoff To** | Unit Test Agent — ready for unit tests |
| **Blockers** | None |
| **Notes** | Implements POST /auth/login per API spec. Uses bcrypt for password comparison. |
```

### Example Log

```markdown
# Agent Communication Log

## 2026-02-17 09:35 — Orchestrator — DISPATCH
| Field | Value |
|-------|-------|
| **Phase** | Phase 3: Implementation |
| **Task** | P1-T001, P1-T002, P1-T003 |
| **Action** | Started |
| **Status** | 🟡 In Progress |
| **Output** | Dispatched: P1-T001 → Cascade-Backend, P1-T002 → Cascade-Frontend, P1-T003 → Cascade-Backend |
| **Handoff To** | Cascade-Backend, Cascade-Frontend |
| **Blockers** | None |
| **Notes** | Day 2 coding kickoff. Review Agent streaming enabled. |

---

## 2026-02-17 10:12 — Cascade-Backend — COMPLETED
| Field | Value |
|-------|-------|
| **Phase** | Phase 3: Implementation |
| **Task** | P1-T001 — Database schema migration |
| **Action** | Completed |
| **Status** | 🟢 Done |
| **Output** | `backend/src/migrations/001_create_users.sql`, `backend/src/repos/user.repo.ts` |
| **Handoff To** | Unit Test Agent — repo methods ready for testing |
| **Blockers** | None |
| **Notes** | 3 tables created: users, sessions, tokens. All indexes applied. |

---

## 2026-02-17 10:15 — Review Agent — FEEDBACK
| Field | Value |
|-------|-------|
| **Phase** | Phase 3: Implementation |
| **Task** | P1-T001 — Database schema migration |
| **Action** | Completed |
| **Status** | 🟢 Done |
| **Output** | Inline review: 1 suggestion (add index on sessions.expires_at) |
| **Handoff To** | Cascade-Backend — apply suggestion |
| **Blockers** | None |
| **Notes** | No critical findings. 1 performance suggestion. |

---

## 2026-02-17 11:30 — Cascade-Frontend — BLOCKED
| Field | Value |
|-------|-------|
| **Phase** | Phase 3: Implementation |
| **Task** | P1-T002 — Login page component |
| **Action** | Blocked |
| **Status** | ⚫ Blocked |
| **Output** | `frontend/src/pages/Login.tsx` — partial (form layout done) |
| **Handoff To** | Design Agent — need design tokens for login card |
| **Blockers** | Design tokens not yet available for login card styling |
| **Notes** | Can continue after Design Agent publishes tokens. |
```

### Rules

1. **Append-only** — never edit or delete previous entries. The log is an immutable audit trail.
2. **Every agent writes** — no exceptions. Even the Orchestrator and Oracle log their decisions.
3. **Handoff To is mandatory** — if work continues, name the next agent. If done, write `None`.
4. **Blockers must be specific** — name the dependency, agent, or artifact that is blocking.
5. **Orchestrator reads the log** before dispatching new work to check for blockers and completed handoffs.
6. **Oracle reads the log** at midday sync and sprint demo for status overview.
7. **Gate checks reference the log** — phase gate validation scans recent entries to confirm all tasks in the phase are `🟢 Done`.

---

## Phases

### Phase 0: Discovery

Oracle provides a feature request. Orchestrator determines scope.

```
Oracle → Provides feature request or project idea
    ↓
Orchestrator → Determines scope: new project, sub-feature, bug fix, or refactor
    ↓
Spec Agent → Generates initial project spec / feature spec
    ↓
Oracle → Reviews and approves spec
```

**Phase gate**: Project/feature spec approved by Oracle.

### Phase 1: Specification (Parallel Spec Swarm)

Orchestrator dispatches spec tasks to Spec Agents in parallel tracks, respecting dependencies between spec documents.

```
Spec approved (gate passed)
    │
    ├──► Spec-Agent-A: User stories + Use cases         ← parallel
    ├──► Spec-Agent-B: Data model + Data structure       ← parallel
    │
    │  After A + B converge:
    ├──► Spec-Agent-C: Functions + Access control         ← parallel
    ├──► Spec-Agent-D: Object lifecycle + Persistence     ← parallel
    │
    │  After all converge:
    ├──► Spec-Agent-E: Architecture + Module design       ← sequential
    ├──► Spec-Agent-F: API design                         ← after E
    ├──► Spec-Agent-G: UX wireframes + Page flows         ← parallel with E-F
    └──► Spec-Agent-H: Deployment spec                    ← parallel with E-F
```

**Estimated speedup**: ~2x wall-clock time vs fully sequential spec writing.

**Phase gate**: Spec completeness ≥ 90%, Oracle approves.

### Phase 2: Planning

Sequential — each step depends on the prior.

```
Specs approved (gate passed)
    ↓
Orchestrator → Generates project scope & phases
    ↓
Orchestrator → Generates task list & dependencies
    ↓
    ├──► Task allocation (team assignment)       ← parallel
    └──► Branch grouping (branch-based batches)  ← parallel
    ↓
Orchestrator → Generates test plan
    ↓
Oracle → Reviews and approves plan
```

**Phase gate**: Tasks allocated, test plan approved, task tracking file populated.

### Phase 3: Implementation Swarm

All agents run **concurrently**. The Review Agent provides **streaming inline feedback**. Each agent works on its assigned tasks.

```
Plan approved (gate passed)
    │
    ├──► Cascade-Backend ──► implements backend tasks
    │         │
    │         ├──► Review Agent (streaming) ──► immediate feedback
    │         │                                      │
    │         │                              Cascade-Backend fixes inline
    │         │
    ├──► Cascade-Frontend ──► implements frontend tasks
    │         │
    │         ├──► Review Agent (streaming) ──► immediate feedback
    │         │                                      │
    │         │                              Cascade-Frontend fixes inline
    │         │
    └──► Design Agent ──► extracts tokens, writes theme files
              │
              └──► Cascade-Frontend reads tokens as they appear
```

**Phase gate**: Build passes (e.g., type checks, compilation), 0 type errors.

### Phase 4: Testing Swarm

All test agents run **concurrently** against the implementation artifacts.

```
Implementation complete (gate passed)
    │
    ├──► Unit Test Agent ──► generates unit tests for services, repos
    │
    ├──► Integration Test Agent ──► generates integration tests for API endpoints
    │
    └──► E2E Test Agent ──► generates end-to-end test scripts
```

**Phase gate**: Combined coverage ≥ 80%, all tests pass.

### Phase 5: Final Review (Parallel Swarm)

All review agents run **concurrently**, each producing an independent report.

```
Tests passing (gate passed)
    │
    ├──► Security Review Agent ──► produces review-sec.md
    │
    ├──► Performance Review Agent ──► produces review-perf.md
    │
    └──► Standards Review Agent ──► produces review-std.md
              │
              ▼
    Orchestrator merges findings into unified review
              │
              ▼
    Oracle resolves any critical findings
```

**Phase gate**: 0 critical findings across all reports.

### Phase 6: Documentation (Parallel Swarm)

All doc agents run **concurrently**.

```
Review approved (gate passed)
    │
    ├──► API Doc Agent ──► updates endpoint documentation
    │
    ├──► README Agent ──► updates README, guides, setup instructions
    │
    └──► Changelog Agent ──► writes changelog entry
              │
              ▼
    Oracle → Final documentation review
```

**Phase gate**: All doc artifacts updated and consistent.

---

## Agent Communication Patterns

### Sequential
Strict dependency between phases — output of one is input to next.
```
Spec Swarm → Planning → Implementation Swarm → Testing Swarm → Review Swarm → Doc Swarm
```

### Parallel Fan-out / Fan-in
Independent sub-tasks within a phase run concurrently; Orchestrator collects all results before advancing.
```
                    ┌──► Cascade-Backend ───┐
Orchestrator ───────┼──► Cascade-Frontend ──┼──► Orchestrator (fan-in)
                    └──► Design Agent ──────┘
```

### Iterative
Refinement loops between agents until quality criteria met.
```
Cascade ←──► Review Agent (until 0 critical findings)
```

### Streaming (Continuous Feedback)
Review Agent monitors implementation in real-time, providing immediate feedback without waiting for phase completion.
```
Cascade writes code ──► Review Agent (streaming check) ──► immediate feedback
                                                              │
                                                    Cascade fixes inline
```

### Pipeline (Overlapping Phases)
Next phase starts on partial output before current phase fully completes.
```
Cascade-Backend (50% done) ──► Unit Test Agent starts on completed modules
Cascade-Frontend (30% done) ──► Component Test Agent starts on completed components
```

---

## Phase Gates (Automated Criteria)

| Gate | Auto-Check | Human Override |
|------|-----------|----------------|
| Discovery → Spec | Project/feature spec exists and approved | Oracle can force-proceed |
| Spec → Planning | Spec completeness score ≥ 90% | Oracle can force-proceed |
| Planning → Impl | Tasks allocated, test plan approved | Oracle reviews edge cases |
| Impl → Test | Build passes, 0 type errors | Oracle reviews edge cases |
| Test → Review | Coverage ≥ 80%, all tests pass | Oracle reviews test quality |
| Review → Doc | 0 critical findings across all review reports | Oracle accepts known risks |
| Doc → Merge | All doc artifacts updated and consistent | Oracle final sign-off |

---

## Agent-Coding Sprint Integration

This coordination maps to the **2-day Agent-Coding Sprint** model where each sprint compresses one normal week of work into 2 accelerated days using AI agent assistance.

### Day 1 — Spec & Flex Dev (Phases 0–2, optionally start Phase 3)

```
┌─────────────────────────────────────────────────────────────────────────┐
│ Day 1 — Spec & Flex Dev                                                │
├──────────┬──────────────────────────────────────────────────────────────┤
│ 9:00 AM  │ Sprint Kickoff: Oracle sets scope, Orchestrator decomposes  │
│          │ Determine: new project, sub-feature, bug fix, or refactor   │
├──────────┼──────────────────────────────────────────────────────────────┤
│ 9:30 AM  │ Phase 1: Spec Swarm — parallel spec tracks                 │
│          │   Track A: User stories → Use cases → UX wireframes        │
│          │   Track B: Data model → Data structure → Persistence        │
│          │   (Spec Agents run in parallel)                             │
├──────────┼──────────────────────────────────────────────────────────────┤
│ 12:00 PM │ Midday Sync: Orchestrator checks spec completeness          │
│          │   If ≥ 90%: approve and start Phase 2 (Planning)            │
│          │   If < 90%: continue spec work in afternoon                 │
├──────────┼──────────────────────────────────────────────────────────────┤
│ 1:00 PM  │ Phase 2: Planning — scope → tasks → allocation → test plan │
│          │   OR: Continue Phase 1 if specs not yet approved            │
├──────────┼──────────────────────────────────────────────────────────────┤
│ 3:00 PM  │ If planning complete: start Phase 3 (early implementation)  │
│          │   Cascade-Backend + Cascade-Frontend begin coding            │
│          │   Review Agent starts streaming feedback                    │
├──────────┼──────────────────────────────────────────────────────────────┤
│ 4:30 PM  │ Day 1 Review: Oracle approves specs + plan                  │
│          │   Deliverables: approved specs, task allocation,             │
│          │   test plan, possibly early implementation started           │
└──────────┴──────────────────────────────────────────────────────────────┘
```

### Day 2 — Code Only (Phases 3–6)

```
┌─────────────────────────────────────────────────────────────────────────┐
│ Day 2 — Code Only                                                      │
├──────────┬──────────────────────────────────────────────────────────────┤
│ 9:00 AM  │ Coding Kickoff: Orchestrator dispatches Implementation Swarm│
│          │   Cascade-Backend: backend tasks                             │
│          │   Cascade-Frontend: frontend tasks                           │
│          │   Design Agent: extract tokens (if UI feature)              │
│          │   Review Agent: streaming inline checks                     │
├──────────┼──────────────────────────────────────────────────────────────┤
│ 12:00 PM │ Midday Sync: Orchestrator checks build status               │
│          │   Gate: build passes, 0 type errors                         │
│          │   Testing Swarm starts on completed modules (Pipeline)       │
├──────────┼──────────────────────────────────────────────────────────────┤
│ 1:00 PM  │ Phase 4: Testing Swarm (parallel with remaining impl)       │
│          │   Unit Test Agent + Integration Test Agent + E2E Test Agent  │
├──────────┼──────────────────────────────────────────────────────────────┤
│ 3:00 PM  │ Phase 5: Review Swarm (parallel)                            │
│          │   Security + Performance + Standards agents                  │
│          │   Orchestrator merges findings                              │
├──────────┼──────────────────────────────────────────────────────────────┤
│ 4:00 PM  │ Phase 6: Doc Swarm (parallel)                               │
│          │   API Doc + README + Changelog agents                       │
├──────────┼──────────────────────────────────────────────────────────────┤
│ 4:30 PM  │ Sprint Demo: Orchestrator reports metrics                   │
│          │   Deliverables: working code, passing tests, merged PRs,    │
│          │   updated docs, deployment to staging                       │
└──────────┴──────────────────────────────────────────────────────────────┘
```

### Sprint Cadence

```
Monday (retro/tech debt) → Sprint 1 (Tue-Wed) → Sprint 2 (Thu-Fri) → Monday → ...
                                Day 1: Spec (Phases 0-2)
                                Day 2: Code (Phases 3-6)
```

---

## Scenario Matrix

Different project scenarios activate different subsets of phases and agents:

| Scenario | Phases | Agents Activated | Notes |
|----------|--------|-----------------|-------|
| **New feature (full)** | 0 → 1 → 2 → 3 → 4 → 5 → 6 | All 13 agents | Full pipeline |
| **New feature (small)** | 0 → 2 → 3 → 4 → 5 → 6 | Skip Spec Swarm, single Spec Agent | Spec + impl in Day 1 |
| **Bug fix** | 0 → 3 → 4 → 5 | Cascade + Test Agents + Review Agents | No spec swarm, no docs |
| **Refactor** | 0 → 3 → 4 → 5 → 6 | Cascade + Test Agents + Review + Doc Agents | Focus on tests + review |
| **Infrastructure** | 0 → 2 → 3 → 5 | Cascade-Backend + Review Agents | No frontend, no E2E |
| **Documentation only** | 6 | Doc Agent Swarm | Standalone doc update |
| **Design update** | 0 → 3 → 4 | Design Agent + Cascade-Frontend + Test Agents | UI-focused |

---

## Performance Metrics

| Metric | Target | Monitored By |
|--------|--------|--------------|
| Spec swarm throughput | >1.5x sequential | Orchestrator |
| Impl swarm throughput | >2x sequential baseline | Orchestrator |
| Agent efficiency | >85% | Orchestrator |
| Task accuracy | >90% | Orchestrator |
| Code quality | >80% coverage | Testing Swarm |
| Security score | 0 critical issues | Security Review Agent |
| Performance score | No regressions | Performance Review Agent |
| Standards compliance | 0 violations | Standards Review Agent |
| Doc completeness | 100% artifacts updated | Doc Swarm |
| Phase gate pass rate | >90% first attempt | Orchestrator |

---

## Oracle Responsibilities

1. **Strategic decisions** — Feature prioritization, scope, trade-offs
2. **Override gates** — Force-proceed when automated checks are too strict
3. **Resolve conflicts** — Arbitrate when agents produce contradictory outputs
4. **Final sign-off** — Approve spec, review findings, documentation
5. **Report** — Share swarm metrics in sprint kickoff
6. **Optimize** — Refine agent prompts and swarm composition based on metrics

## Orchestrator Responsibilities

1. **Decompose** — Break feature into parallelizable sub-tasks
2. **Fan-out** — Dispatch sub-tasks to agent swarms concurrently
3. **Fan-in** — Collect and merge results from swarm agents
4. **Gate-check** — Validate automated criteria before phase transitions
5. **Monitor** — Track per-agent and per-swarm performance metrics
6. **Conflict detection** — Flag contradictions for Oracle review
7. **Artifact bus** — Manage shared artifact read/write coordination
8. **Status tracking** — Update task status in the project's task tracking file
9. **Communication log** — Read `agent_workspace/communication-log.md` before dispatching; ensure all agents log entries for every action
10. **Agent registration** — Verify all active agents are registered in the communication log before dispatching work

---

## Licensed Code Handling

When working with licensed code:
1. **Never** share licensed code with any agent in the swarm
2. **Only** share interfaces and summaries via the artifact bus
3. **Human** implements adapters
4. **Agents** work with interfaces only
5. **Orchestrator** monitors compliance (100% target)

---

## Example: Full Feature Development (Parallel Swarms)

```
1. Oracle submits feature request
   └── "Build user profile page with avatar upload"

2. Phase 0 — Discovery
   └── Spec Agent generates feature spec (data model, API, UI)
   └── Gate: Oracle approves feature spec

3. Phase 1 — Spec Swarm (parallel tracks)
   ├── Spec-Agent-A: user stories + use cases (parallel)
   ├── Spec-Agent-B: data model + data structure (parallel)
   │   ... (parallel tracks per dependency graph) ...
   ├── Spec-Agent-G: UX wireframes + page flows (parallel)
   └── Gate: completeness 94% ✓, Oracle approves

4. Phase 2 — Planning
   ├── Scope & phases → task list → 12 tasks across 3 phases
   ├── Task allocation + branch groups (parallel)
   └── Test plan
   └── Gate: Oracle approves plan

5. Phase 3 — Implementation Swarm (concurrent, Day 2 AM)
   ├── Cascade-Backend → profile API, avatar upload service (parallel)
   ├── Cascade-Frontend → profile page, avatar component (parallel)
   ├── Design Agent → extracts profile card tokens (parallel)
   └── Review Agent → streaming feedback on both (continuous)
   └── Gate: build passes ✓, 0 type errors ✓

6. Phase 4 — Testing Swarm (concurrent, Day 2 PM)
   ├── Unit Test Agent → 10 unit tests (parallel)
   ├── Integration Test Agent → 4 integration tests (parallel)
   └── E2E Test Agent → profile page E2E scripts (parallel)
   └── Gate: coverage 83% ✓, all tests pass ✓

7. Phase 5 — Review Swarm (concurrent)
   ├── Security Review Agent → 0 critical, 1 low (parallel)
   ├── Performance Review Agent → 0 regressions (parallel)
   └── Standards Review Agent → 0 violations (parallel)
   └── Gate: 0 critical findings ✓

8. Phase 6 — Documentation Swarm (concurrent)
   ├── API Doc Agent → 3 endpoints documented (parallel)
   ├── README Agent → setup guide updated (parallel)
   └── Changelog Agent → entry added (parallel)
   └── Gate: all artifacts updated ✓

9. Orchestrator reports metrics
   └── Spec swarm throughput: 1.8x sequential
   └── Impl swarm throughput: 2.3x sequential
   └── Agent efficiency: 92%
   └── Task accuracy: 95%
   └── Coverage: 83%
   └── Total wall-clock time: 40% of sequential estimate
```
