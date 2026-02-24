---
description: Execute one or more tasks (sorted by priority) from the implementation plan using outputs from Steps 1-19
arguments:
  send: true
---

# Implementation Execution Workflow (Step 20)

Use this workflow after completing Steps 1–19 (specs, scope, tasks, test plan) to start coding against the generated plans.

## Usage

```
/aspec-20-implement [task_ids...] [optional_context_files]
```

- **`task_ids...`** — (Optional) One or more task IDs from `phase-task-list.md` (e.g., `P1-T004`, `P2-T003 P2-T005 P2-T001`). When multiple IDs are provided, they are **automatically sorted by priority** (high → low) from `phase-task-list.md` and executed sequentially. When omitted, the workflow picks the highest-priority open task.
- **`optional_context_files`** — Additional spec/plan files to load as context.

**Examples:**
- `/aspec-20-implement P2-T003 P2-T005 P2-T001` — execute three tasks, sorted by priority automatically
- `/aspec-20-implement P2-T003 docs/phase-task-list.md` — execute only task P2-T003 with explicit context
- `/aspec-20-implement P1-T004` — execute a single task using default docs
- `/aspec-20-implement docs/phase-task-list.md docs/19-test-plan.md` — auto-select highest-priority open task
- `/aspec-20-implement` — auto-select using default docs

### Resuming from an Implementation Log

To resume work after switching products or sessions, pass the implementation log:

```
/aspec-20-implement P2-T003 P2-T005 docs/features/<feature>/implementation-log.md
```

The agent will read the log to restore context (last milestone, known issues, next steps) before continuing the specified task(s).

## Input

```text
$ARGUMENTS
```

**Argument parsing:**
1. Collect all arguments that match a task ID pattern (e.g., `P1-T001`, `P2-T003`) into a **task list**.
2. Collect all remaining arguments (file paths — contains `/` or ends in `.md`) as **context files**.
3. If the task list has multiple IDs, look up their priorities in `phase-task-list.md` and **sort high → low priority**.
4. If the task list is empty, auto-select the highest-priority open task.
5. If no context files are provided, use the default context files.

**Default context files** (used when no file paths are provided):
- `docs/phase-task-list.md`
- `docs/19-test-plan.md`
- `docs/12-architecture-summary.md`
- `AGENTS.md`

## Workflow Steps

1. **Task Selection**
   - If an `implementation-log.md` was provided, read it first to restore prior context (last milestone, known issues, next steps).
   - If **task IDs were provided**, locate them in `phase-task-list.md`, verify they exist and are not already marked complete, then sort by priority (high → low). Pick the first (highest-priority) task from this sorted list.
   - If **no task IDs were provided**, review `phase-task-list.md` and choose the highest-priority open task.
   - Confirm prerequisites satisfied per dependency matrix.

2. **Context Gathering**
   - Load relevant specs (API, module, UX) for the chosen task.
   - Note acceptance criteria and test requirements from `19-test-plan.md`.

3. **Implementation Plan**
   - Break task into sub-steps (data model, API, UI, etc.).
   - Coordinate via `/agent-01-coordination` if additional agents needed (Testing, Review, Documentation, Design). Use `collab:` to request multi-agent collaboration or `distribute:` to assign tasks directly.

4. **Coding & Tests**
   - Implement code changes following project standards.
   - Write/extend unit tests immediately.
   - Run local/CI-equivalent tests relevant to the task.

5. **Validation**
   - Ensure acceptance criteria met.
   - Capture test results (logs, screenshots) for the task record.

6. **Agent Handoffs**
   - Trigger Testing Agent for expanded coverage once code passes smoke tests.
   - Trigger Review Agent before merge; provide diff, risks, and test evidence.
   - Notify Documentation Agent of required doc updates.

7. **Status Update**
   - Update the **Status** column in `docs/task-allocation.md` for the current task ID: set `🟡 In Progress` → `🟢 Done` (or `⚫ Blocked` if stuck).
   - Note blockers and next steps.
   - If `docs/features/<feature>/implementation-log.md` exists, append a handoff entry:
     ```
     ## [Task ID] - [Date]
     - **Status:** Done | In Progress | Blocked
     - **Progress:** [What was built/changed]
     - **Known Issues:** [Any bugs or concerns]
     - **Next Steps:** [What the next session should do]
     ```

## Output

- Code, tests, and documentation updates per task.
- Status notes referencing task IDs (e.g., `P2-T003`), test evidence, and agent coordination log entries.

## Next Steps

- If **multiple task IDs** were provided, after completing one task, automatically proceed to the next task in the sorted list (high → low priority). Repeat Steps 1–7 for each task until the list is exhausted.
- If a **single task ID** was provided, the workflow ends after that task.
- If **auto-selecting**, repeat from Step 1 for the next highest-priority item.
- Proceed to deployment workflows when all tasks in a phase are complete.
