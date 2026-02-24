# workflow_script

Scripts and workflows that power multi-agent coordination and service architecture extraction in Windsurf. Deployed automatically to `your-project/.windsurf/workflow_script/` by any `deploy-windsurf.sh` role.

---

## Contents

| File | Type | Purpose |
|------|------|---------|
| `launch-multi-agent.sh` | Shell script | Central coordination driver — watches comm-log and routes signals to all agents |
| `agent-tab-watcher.sh` | Shell script | Per-agent signal watcher — auto-spawned by `start`, writes `.signal` files into agent inbox |
| `agent-signal-listener.sh` | Shell script | Signal listener (Cascade-side) — blocks until a signal arrives in `.agent-signals/` |
| `agent-register.sh` | Shell script | Atomic agent registration — writes `.agent-info`, creates `.agent-signals/`, registers in comm-log |
| `cleanup-multi-agent.sh` | Shell script | Session cleanup — stops watchers, removes worktrees, resets comm-log |
| `agent-merge-worktrees.sh` | Shell script | Agent branch merger — merges all active agent worktree branches into `main` |
| `agent-worktree-watcher.sh` | Shell script | External daemon for separate-window mode — manages worktrees and signal routing |
| `service-extract.md` | Workflow | Extracts normalized architecture summaries from existing service codebases |

---

## Multi-Agent Scripts

These scripts implement the real-time push signaling system for running multiple parallel Cascade agents. For full usage documentation see `multi-agent-handbook.md` (deployed alongside these scripts).

### How It Works

Cascade sessions cannot notify each other directly. This system bridges that gap:

1. All agents share `agent_workspace/communication-log.md` as a **message bus**
2. `agent-tab-watcher.sh` (one per agent, background) watches the log via `fswatch` and writes `.signal` files into each agent's `.agent-signals/` inbox
3. `launch-multi-agent.sh` (one per session, system terminal) routes signals across all agents

```
Windsurf Window
├── Tab 1 (orchestrator) ─┐
├── Tab 2 (backend)      ─┼─► agent_workspace/communication-log.md
├── Tab 3 (frontend)     ─┤              │
└── Tab 4 (unit-test)    ─┘         fswatch
                                         │
                            agent-tab-watcher.sh (per tab)
                                         │
                   .agent-signals/DISTRIBUTE.signal  (per agent)

System terminal: launch-multi-agent.sh  ← coordination driver
```

---

### `launch-multi-agent.sh`

Central coordination driver. Run **once** in a system terminal after all agents have registered.

```bash
./launch-multi-agent.sh                                    # watch mode (default)
./launch-multi-agent.sh status                             # show online agents + pending signals
./launch-multi-agent.sh signal <agent-or-role> <type> "<payload>"
./launch-multi-agent.sh broadcast <type> "<payload>"
./launch-multi-agent.sh teardown                           # stop all, mark offline
```

| Command | Description |
|---------|-------------|
| *(no args)* | Watch mode — routes signals continuously |
| `status` | Show all 🟢 Online agents, pending signals, watcher status |
| `signal <target> <type> "<payload>"` | Send signal to one agent (by name or role) |
| `broadcast <type> "<payload>"` | Send signal to all online agents |
| `teardown` | Stop watchers, clear signals, mark all agents 🔴 Offline |

---

### `agent-tab-watcher.sh`

Per-agent signal watcher. Auto-spawned by `/agent-coordination start`. Can also be run manually.

```bash
./agent-tab-watcher.sh                        # start (reads .agent-info automatically)
./agent-tab-watcher.sh --status               # show agent + signal state
./agent-tab-watcher.sh --clear                # remove pending signals
./agent-tab-watcher.sh --stop                 # stop running watcher
./agent-tab-watcher.sh --init <name> <role>   # manual one-time init
```

| Flag | Description | Default |
|------|-------------|---------|
| *(no args)* | Start watcher, reads `.agent-info` automatically | — |
| `--init <name> <role>` | Write `.agent-info` (one-time setup) | — |
| `--agent <name>` | Override agent name | from `.agent-info` |
| `--role <role>` | Override role | from `.agent-info` |
| `--root <path>` | Override worktree root | git root of CWD |
| `--interval <secs>` | Poll interval (fswatch fallback) | `2` |
| `--debug` | Verbose logging | off |

Watches `agent_workspace/communication-log.md` via `fswatch` (or polling fallback). Writes `.signal` files into `<worktree>/.agent-signals/` when a message targets this agent.

---

### `agent-signal-listener.sh`

Used internally by Cascade's work loop to block until a signal arrives. Can also be called manually to inspect signals.

```bash
./agent-signal-listener.sh                    # wait for any signal
./agent-signal-listener.sh --type DISTRIBUTE  # wait for specific type
./agent-signal-listener.sh --timeout 120      # wait up to 120 s
./agent-signal-listener.sh --peek             # check without waiting
./agent-signal-listener.sh --consume          # read and delete all
```

Exit codes: `0` = signal received, `1` = error, `2` = timeout.

---

### `agent-register.sh`

Called by `start` to atomically write `.agent-info`, create `.agent-signals/`, bootstrap the comm-log, and register the agent.

```bash
./agent-register.sh "<role>" "<project-root>"
```

Expected output:
```
REGISTERED: {role}-{last8}
INFO: {cwd}/.agent-info
SIGNALS: {cwd}/.agent-signals/
```

Agent names are derived deterministically from the worktree path: `{role}-{last8hex}` (e.g. `backend-aef9e332`). Names are stable across restarts and collision-free across simultaneous agents.

---

### `cleanup-multi-agent.sh`

Resets all multi-agent state. Run at the end of a session or before starting fresh.

```bash
./cleanup-multi-agent.sh              # full cleanup (confirm prompt)
./cleanup-multi-agent.sh --yes        # full cleanup, no prompt
./cleanup-multi-agent.sh --dry-run    # preview what would be removed
./cleanup-multi-agent.sh --signals    # clear signals only
./cleanup-multi-agent.sh --log        # reset comm-log only
```

Full cleanup:
1. Stops all running `agent-tab-watcher.sh` processes
2. Clears `.agent-signals/` in every discovered worktree
3. Removes `.agent-info` from every discovered worktree
4. Removes all non-main git worktrees
5. Runs `git worktree prune`
6. Resets `communication-log.md` to blank template
7. Removes launcher state (`agent_workspace/.launcher-last-line`)

---

### `agent-merge-worktrees.sh` — Agent Branch Merger

Called by `/agent-01-coordination merge`. Merges all active agent worktree branches into `main`.

```bash
./agent-merge-worktrees.sh <project-root>
./agent-merge-worktrees.sh <project-root> --dry-run
```

Output per branch:
```
BRANCH: agent/backend-aef9e332
STATUS: merged_clean | needs_resolution | skipped | error
CONFLICTS: path/to/file.ts   (only when needs_resolution)
---
```

Exit codes: `0` = all branches processed, `1` = fatal error (not on main, dirty repo, etc.)

---

### `agent-worktree-watcher.sh`

Alternative to tab mode. Use when you need full branch isolation with each agent in its own Windsurf window.

```bash
./agent-worktree-watcher.sh --setup <name> <role>   # create worktree + branch
./agent-worktree-watcher.sh                          # start daemon
./agent-worktree-watcher.sh --list                   # list managed worktrees
./agent-worktree-watcher.sh --teardown <name>        # remove one worktree
./agent-worktree-watcher.sh --stop                   # stop daemon
```

Creates `.agent-worktrees/<name>/` with its own branch (`agent/<name>`), `.agent-info`, `.agent-signals/`, and symlinks to shared `agent_workspace/` and `docs/`.

**Tab Mode vs Separate Windows:**

| | Tab Mode (recommended) | Separate Windows |
|---|---|---|
| **Script** | `agent-tab-watcher.sh` + `launch-multi-agent.sh` | `agent-worktree-watcher.sh` |
| **Windsurf windows** | One window, N worktree tabs | One window per agent |
| **Worktree creation** | Windsurf auto-creates on new tab | `--setup` per agent |
| **Watcher start** | Auto-spawned by `start` | Manual daemon |
| **Best for** | Local dev, fast setup | Full isolation, CI-like environments |

---

## Workflow: `service-extract.md`

A Cascade workflow (invoked via `/service-extract`) that analyzes existing service codebases and generates normalized architecture summaries. This is **not** a spec workflow — it extracts what already exists in code.

### Usage

```
/service-extract /path/to/user-service        # single service
/service-extract /path/to/services-folder     # all services in a folder
/service-extract /path/to/svc-a /path/to/svc-b  # multiple specific paths
/service-extract .                            # current directory
```

### What It Extracts

- Technology stack (language, framework, runtime, DB, cache, message queue)
- Service metadata (name, version, description, repository)
- Architectural pattern (Clean Architecture, MVC, Hexagonal, etc.)
- API endpoints (REST, gRPC, authentication methods)
- Event contracts (published/consumed Kafka, RabbitMQ events)
- Service and external dependencies
- Health/observability endpoints
- Existing diagrams (Mermaid, PlantUML, Draw.io, images)

### Supported Stacks

| File | Detected As |
|------|-------------|
| `package.json` + `tsconfig.json` | TypeScript/Node.js |
| `package.json` (no tsconfig) | JavaScript/Node.js |
| `go.mod` | Go |
| `Gemfile` + `config/routes.rb` | Ruby on Rails |
| `pubspec.yaml` | Flutter/Dart |
| `requirements.txt` / `pyproject.toml` | Python |
| `pom.xml` / `build.gradle` | Java/Kotlin |
| `Cargo.toml` | Rust |

### Output

**Single service:**
- `docs/{service-name}-service-summary.md` — human-readable architecture summary
- `docs/{service-name}-service-summary.yaml` — optional, for programmatic use

**Folder of services:**
- `service-name/docs/{service-name}-service-summary.md` per service
- `docs/services-summary.md` — consolidated overview of all services

### Next Step

Feed extracted summaries into the service registry:

```
/aspec-12.1-service-registry docs/{service-name}-service-summary.yaml
```

---

## Prerequisites

### `fswatch` (recommended for real-time signaling)

```bash
brew install fswatch          # macOS
sudo apt install inotify-tools # Debian/Ubuntu
sudo yum install inotify-tools # RHEL/CentOS
```

Without `fswatch`, the system falls back to 2-second polling — still functional but with higher latency.

---

## Deployment

These files are deployed automatically by `script/deploy-windsurf.sh` for every role:

```bash
cd script
./deploy-windsurf.sh full-stack-dev ~/projects/my-app
./deploy-windsurf.sh backend-dev ~/projects/api-service
```

The deploy script copies this entire directory to `your-project/.windsurf/workflow_script/` and sets `chmod +x` on all `.sh` files. To skip multi-agent components:

```bash
./deploy-windsurf.sh backend-dev ~/projects/api-service --no-multi-agent
```

For manual installation:

```bash
cp -r template/windsurf_template/workflow_script/ your-project/.windsurf/workflow_script/
chmod +x your-project/.windsurf/workflow_script/*.sh
```
