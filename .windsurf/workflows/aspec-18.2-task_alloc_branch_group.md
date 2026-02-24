---
description: Distribute tasks from Phase Task List across balanced development branches based on dependencies, cohesion, and estimated effort (Step 18.1)
arguments:
  send: true
---

# Branch-Based Task Allocation Generator

Use this workflow after completing Step 18 (Phase Task List and Dependencies) to distribute tasks across development branches. Tasks are grouped so that each branch has a balanced total implementation time, contains tasks that naturally belong together (shared dependencies, same module/layer), and can be developed without significant cross-branch complexity.

## Usage

```
/aspec-18.1-task_allocation [input_files]
```

**Examples:**
- `/aspec-18.1-task_allocation docs/phase-task-list.md AGENTS.md`
- `/aspec-18.1-task_allocation docs/` — reads all spec docs for context
- `/aspec-18.1-task_allocation` (will look for default files listed below)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths or a directory, read those files as input documents.
If empty, read the following files for context:
- `docs/phase-task-list.md` (task breakdown, dependencies, required skills)
- `docs/12-architecture-summary.md` (tech stack, component ownership)
- `docs/17-project-scope-phases.md` (phases, milestones, timeline)
- `docs/15-agents-md.md` or `AGENTS.md` (AI agent coordination, team guidelines)
- `docs/19-test-plan.md` (test responsibilities)

## Prerequisites

Ensure you have completed:
- All documents from Steps 1-18
- Phase Task List with dependencies and required skills (Step 18)

## Prompt

```
You are a technical project manager specializing in branch-based task distribution for AI-augmented development. Based on the Phase Task List (Step 18), architecture summary, and project AGENTS.md, generate a **Branch-Based Task Allocation Plan** that groups every task into development branches optimized for balanced effort, dependency cohesion, and minimal cross-branch complexity.

**IMPORTANT — Branch Distribution Model**
Instead of assigning tasks to individual team members, this workflow distributes tasks across **development branches**. Each branch:
- Contains a cohesive set of tasks that can be implemented together in a single PR
- Has a balanced total estimated effort relative to other branches
- Groups tasks that share dependencies or belong to the same module/layer
- Minimizes cross-branch merge conflicts by isolating changes to distinct files/modules
- Can be implemented sequentially by a single developer + AI agent pair (Cascade)

**IMPORTANT — Grouping Strategy:**
When distributing tasks into branches, apply these rules in priority order:

1. **Dependency chains stay together** — If task B depends on task A and no other branch needs A first, group A and B in the same branch.
2. **Shared-dependency fan-out** — If multiple tasks depend on the same foundation task, the foundation task goes into the earliest branch (or its own branch if large enough), and dependent tasks fan out into separate branches that list the foundation branch as a prerequisite.
3. **Layer/module cohesion** — Tasks touching the same files, module, or architectural layer (e.g., all repository methods, all service implementations, all frontend components) should be grouped together when their combined effort stays balanced.
4. **Effort balancing** — Target roughly equal total hours per branch. Acceptable variance is ±30% of the average branch effort. If a single task exceeds the target branch size, it becomes its own branch.
5. **Test tasks follow implementation** — Unit/integration test tasks should be in the same branch as the code they test, unless the test task is large enough to warrant its own branch.
6. **Cross-phase boundaries** — Tasks from different phases (e.g., backend vs. frontend) should generally be in separate branches unless they are tightly coupled (e.g., API types + API client).

**Output the following sections:**

---

### 1. Branch Distribution Summary

Overview table showing all branches with their total effort and task count:

| Branch # | Branch Name | Type | Total Effort | Task Count | Layer/Module | Depends On (Branches) |
|----------|-------------|------|-------------|------------|-------------|----------------------|
| B1 | `feature/short-descriptive-name` | feature | [hours] | [count] | [e.g., Backend/DTOs+Interfaces] | — |
| B2 | `feature/short-descriptive-name` | feature | [hours] | [count] | [e.g., Backend/Services] | B1 |

**Branch Naming Convention:**
Auto-generate the branch name from: `{type}/{short-descriptive-slug}` using the Git branch types from AGENTS.md:
- `feature/*` — New features or feature modules
- `fix/*` — Bug fixes
- `test/*` — Test-only branches
- `docs/*` — Documentation-only branches

The slug should describe the **group of tasks**, not a single task. Examples:
- `feature/profile-backend-interfaces` (DTOs + traits + repo methods)
- `feature/profile-totp-services` (TOTP service implementations)
- `feature/profile-frontend-core` (API client + identity section + routing)
- `test/profile-e2e-validation` (all E2E test tasks)

Include a **balance check** after the table:
```
Average branch effort: [X]h
Min: [X]h | Max: [X]h | Variance: ±[X]%
```

---

### 2. Branch Detail Sections

For EACH branch, output a detailed section:

#### Branch B[N]: `{branch-name}`

| Field | Value |
|-------|-------|
| **Total Effort** | [hours] |
| **Depends On** | [branch names or "—" if none] |
| **Merge Target** | `main` or [parent branch] |
| **Rationale** | [Why these tasks belong together — shared deps, same module, etc.] |

**Tasks in this branch:**

| Task ID | Task Name | Priority | Est. Effort | Dependencies (within branch) | Status |
|---------|-----------|----------|-------------|------------------------------|--------|
| P1-T001 | [name] | High/Med/Low | [hours] | — | 🔴 Not Started |
| P1-T002 | [name] | High/Med/Low | [hours] | P1-T001 | 🔴 Not Started |

**Implementation order within branch:** List the recommended sequence of tasks considering intra-branch dependencies.

**Files touched:** List the primary files this branch will modify or create (helps identify potential merge conflicts with other branches).

**Status values:** `🔴 Not Started` · `🟡 In Progress` · `🟢 Done` · `⚫ Blocked`

**Status update rule:** When `/aspec-20-implement` completes a task, it MUST update the Status column in the task-allocation file for that task ID:
- Set to `🟡 In Progress` when implementation begins.
- Set to `🟢 Done` when all acceptance criteria pass and PR is merged.
- Set to `⚫ Blocked` if a dependency or issue prevents progress (add reason in the implementation log).

---

### 3. Branch Dependency Graph

Visualize the merge order and inter-branch dependencies:

```
B1 (foundation) ──► B2 (depends on B1)
                ──► B3 (depends on B1)
B2 ──► B4 (depends on B2)
B3 ──► B4 (depends on B3)
B5 (independent) ──► B6 (depends on B5)
```

Include:
- **Critical path branches** — the longest chain of dependent branches that determines minimum total duration
- **Parallel branches** — branches with no mutual dependencies that can be developed simultaneously
- **Merge order** — the sequence in which branches should be merged to `main` to avoid conflicts

---

### 4. Parallel Execution Opportunities

Identify which branches can be developed in parallel (no dependency between them):

```
Parallel Track A: B1 ──► B2 ──► B4
Parallel Track B: B3 (independent of B1/B2)
Parallel Track C: B5 ──► B6 (independent of all above)
```

Include:
- **Maximum parallelism** — how many branches can be in-flight simultaneously
- **Minimum sequential duration** — sum of critical path branch efforts (best-case with unlimited parallelism)
- **Recommended execution order** — for a single developer, the optimal branch sequence considering dependencies and context switching

---

### 5. Conflict Risk Analysis

Identify potential merge conflicts between branches that touch overlapping files:

| Branch A | Branch B | Shared Files | Conflict Risk | Mitigation |
|----------|----------|-------------|---------------|------------|
| B1 | B2 | `module/service.rs` | Low — B2 depends on B1, merge B1 first | Sequential merge |
| B3 | B4 | `pages/ProfilePage.tsx` | Medium — both add sections | Merge B3 first, rebase B4 |

---

### 6. Risk & Bottleneck Analysis

| Risk | Impact | Mitigation |
|------|--------|------------|
| Large foundation branch blocks all others | High | Keep foundation branch minimal; split if >30% of total effort |
| Merge conflicts between parallel branches | Medium | Identify shared files early; define merge order |
| Unbalanced branch sizes | Medium | Re-distribute tasks if variance exceeds ±30% |
| Long dependency chain increases total duration | High | Maximize parallelism by splitting independent task groups |
| Cross-phase dependencies create bottlenecks | Medium | Isolate phase boundaries into separate branches with clear handoff |

---

### 7. Branch Handoff Checklist

Define what must be true before a dependent branch can start:

| Prerequisite Branch | Dependent Branch | Handoff Criteria |
|--------------------|-----------------|-----------------| 
| B1 (interfaces) | B2 (implementations) | All traits compile, `cargo check` passes, branch merged to `main` |
| B[N] (backend) | B[M] (frontend) | API endpoints deployed/testable, types match response shapes |
| B[N] (implementation) | B[M] (tests) | All target code merged, test fixtures available |

---

**Guidelines:**
1. **Balance effort across branches** — target roughly equal hours per branch; acceptable variance ±30% of average
2. **Minimize branch count** — fewer, larger branches reduce merge overhead; but keep each branch focused enough to review in a single PR
3. **Dependency chains stay together** — if A→B→C and no other branch needs A independently, group them
4. **Foundation branches first** — DTOs, interfaces, and shared types that many branches depend on should be in the earliest branch(es)
5. **Tests with implementation** — unit tests belong in the same branch as the code they test
6. **Cross-layer splits** — backend and frontend tasks should generally be in separate branches unless tightly coupled
7. **Reserve 10-15% buffer** — account for merge resolution, code review, and unexpected issues when estimating branch effort
8. **Single-developer optimization** — branches should be ordered so that a single developer + AI agent (Cascade) can work through them sequentially with minimal context switching
9. **File isolation** — when possible, group tasks that touch the same files into one branch to avoid merge conflicts
```

## Output

Save the generated document in the same directory as the input `phase-task-list.md` file, named `task-allocation.md`. For example:
- If input is `docs/profile-page/phase-task-list.md` → save as `docs/profile-page/task-allocation.md`
- If input is `docs/phase-task-list.md` → save as `docs/task-allocation.md`

**Important:** All Status columns must be initialized to `🔴 Not Started`. The `/aspec-20-implement` workflow is responsible for updating this file as tasks progress. This makes the task-allocation file the single source of truth for task status.

## Task Import Formats

### For Jira CSV Import

```csv
Summary,Description,Issue Type,Priority,Labels,Story Points,Branch
"[P1-T001] Task Name","[Description]",Task,Medium,"phase-1,backend,B1",2,"feature/profile-backend-interfaces"
```

### For Gitea Issues (tea CLI)

```bash
tea issue create --title "[P1-T001] Task Name" --description "Branch: feature/profile-backend-interfaces\nBranch Group: B1\nDependencies: [IDs]" --labels "phase-1,B1"
```

### For Linear

Use Linear's CSV import with columns: Title, Description, Priority, Labels (include branch group), Estimate

## Next Steps

After completing this step, proceed to:
- `/aspec-19-testplan` — Generate Test Plan (Step 19)
- `/aspec-20-implement` — Begin implementation execution
