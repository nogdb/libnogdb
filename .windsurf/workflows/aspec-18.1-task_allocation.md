---
description: Allocate tasks from Phase Task List to team members and AI agents based on roles, skills, and capacity (Step 18.1)
arguments:
  send: true
---

# Task Allocation Generator

Use this workflow after completing Step 18 (Phase Task List and Dependencies) to assign tasks to specific team members and AI agents.

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
You are a technical project manager specializing in AI-augmented development teams. Based on the Phase Task List (Step 18), architecture summary, and project AGENTS.md, generate a Task Allocation Plan that assigns every task to the right team member and AI agent pairing.

**IMPORTANT — Sprint Model: Agent-Coding Sprint**
This team uses a special sprint called an **Agent-Coding Sprint**. Each sprint compresses one normal week of work into **2 accelerated days** by leveraging AI agent assistance:
- **Day 1 — Spec & Flex Dev:** Flexible depending on scope — can be full-day spec (for large features), spec + dev in the same day, or parallel spec/dev tracks. Output: approved specs (and possibly early implementation).
- **Day 2 — Code Only:** Implement remaining features with AI agents, write tests, code review, documentation, deploy to staging. Output: working code, passing tests, merged PRs.
- **Cadence:** Monday (retro/tech debt) → Sprint 1 (Tue-Wed) → Sprint 2 (Thu-Fri) → Monday (retro/tech debt) → ...
All task scheduling must follow this two-phase sprint structure.

**IMPORTANT — Team Discovery:**
Read the input documents to identify the available team members and their roles. The standard roles are:

| Role | Primary Responsibilities | AI Agent Pairing |
|------|------------------------|------------------|
| **Oracle** | AI strategy, prompt optimization, agent coordination, license compliance | Multi-Agent Coordination |
| **Tech Lead** | Architecture decisions, code review, sprint planning, spec oversight | Cascade + Review Agent |
| **Backend Dev** | APIs, databases, services, adapter implementation (licensed code) | Cascade + Testing Agent |
| **Frontend Dev** | UI components, client-side features, design token integration | Cascade + Design Agent |
| **UI/UX Designer** | User research, wireframes, prototyping, design system | Design Agent |
| **DevOps Engineer** | CI/CD, infrastructure, deployment, monitoring | Cascade |
| **QA Engineer** | Test strategy, test execution, quality validation | Testing Agent + Review Agent |

If the project has fewer or more roles, adapt the allocation accordingly. Not every project has all seven roles — some team members may cover multiple roles.

**Output the following sections:**

---

### 1. Team Roster & Capacity

| Member | Role(s) | Availability | Primary AI Agent | Skills |
|--------|---------|-------------|-----------------|--------|
| [Name/Placeholder] | [Role] | [hours/day or %] | [Agent] | [Key skills] |

If team member names are not available, use role placeholders (e.g., "Backend Dev 1").

---

### 2. Task Allocation Table

For EACH task from `phase-task-list.md`, assign:

| Task ID | Task Name | Branch | Assigned To | AI Agent Support | Human-Only? | Priority | Est. Effort | Sprint | Phase | Dependencies | Status |
|---------|-----------|--------|-------------|-----------------|-------------|----------|-------------|--------|-------|--------------|--------|
| P1-T001 | [name] | `feature/P1-T001-short-slug` | [role/member] | [agent(s)] | Yes/No | High/Med/Low | [hours] | S1 | Spec / Coding | [task IDs] | 🔴 Not Started |

**Status values:** `🔴 Not Started` · `🟡 In Progress` · `🟢 Done` · `⚫ Blocked`

**Status update rule:** When `/aspec-20-implement` completes a task, it MUST update the Status column in `docs/task-allocation.md` for that task ID:
- Set to `🟡 In Progress` when implementation begins.
- Set to `🟢 Done` when all acceptance criteria pass and PR is merged.
- Set to `⚫ Blocked` if a dependency or issue prevents progress (add reason in the implementation log).

**Sprint Model — Agent-Coding Sprint (2 days = 1 week equivalent):**
- Each sprint packs one normal week of work into 2 accelerated days using AI agent assistance.
- **Day 1 — Spec & Flex Dev:** Flexible — full-day spec, spec + dev, or parallel spec/dev depending on scope.
- **Day 2 — Code Only:** Implementation, testing, code review, documentation, deployment.

**Branch Naming Convention:**
Auto-generate the branch name from: `{type}/{task-id}-{short-slug}` using the Git branch types from AGENTS.md:
- `feature/*` — New features
- `fix/*` — Bug fixes
- `design/*` — Design updates (UI/UX, Figma)
- `infra/*` — Infrastructure / DevOps
- `test/*` — Test-only tasks
- `docs/*` — Documentation

Examples: `feature/P1-T001-user-auth-api`, `infra/P1-T005-ci-pipeline`, `design/P1-T003-profile-wireframe`, `test/P1-T007-auth-e2e`

**Allocation Rules:**
1. **Licensed code tasks** (adapter implementation, legacy integration) → **Human-Only = Yes**, assign to Backend Dev or the role with licensed code access. AI agents must NOT be involved in these tasks.
2. **Interface definition tasks** → Human writes the interface, AI agents can review.
3. **UI implementation tasks** → Frontend Dev + Cascade + Design Agent.
4. **API implementation tasks** → Backend Dev + Cascade + Testing Agent.
5. **Infrastructure tasks** → DevOps Engineer + Cascade.
6. **Test generation tasks** → QA Engineer + Testing Agent.
7. **Design tasks** → UI/UX Designer + Design Agent.
8. **Spec/planning tasks** → Tech Lead + Oracle.
9. **Code review tasks** → Tech Lead + Review Agent.
10. **Documentation tasks** → Assigned developer + Documentation Agent.

---

### 3. Agent-Coding Sprint Allocation

Each **Agent-Coding Sprint** compresses one normal week of work into **2 accelerated days** using AI agent assistance. Map tasks to the two-phase sprint structure:

#### Day 1 — Spec & Flex Dev Phase

| Time Block | Oracle | Tech Lead | Backend Dev | Frontend Dev | Designer | DevOps | QA |
|------------|--------|-----------|-------------|--------------|----------|--------|----|
| 9:00-9:30 (Sprint Kickoff) | Facilitate, set AI strategy | Define scope & priorities, decide Day 1 mode | Report | Report | Report | Report | Report |
| 9:30-12:00 (Morning) | Optimize prompts, coordinate agents | Write/review specs (Steps 1-15) | Data model & API spec | UI spec & component list | Wireframes & design tokens | Infra & deployment spec | Test plan draft |
| 12:00-12:15 (Sync) | Sync — assess if specs are ready to start coding in afternoon | Sync | Sync | Sync | Sync | Sync | Sync |
| 1:00-4:30 (Afternoon) | License compliance review | If specs approved: begin code review; else: continue spec | If specs approved: start implementation; else: continue spec | If specs approved: start implementation; else: continue spec | Figma handoff / design QA | If specs approved: start infra; else: continue spec | If specs approved: start test gen; else: acceptance criteria |
| 4:30-5:00 (Review) | Validate AI readiness | Approve remaining specs, finalize task list | Confirm API contracts | Confirm UI contracts | Validate designs | Confirm infra plan | Confirm test plan |

**Day 1 Deliverables:** Approved specs, task list, interface definitions, design tokens, test plan — everything AI agents need to start coding. If scope is small enough, early implementation may also begin on Day 1 afternoon.

#### Day 2 — Code Only Phase

| Time Block | Oracle | Tech Lead | Backend Dev | Frontend Dev | Designer | DevOps | QA |
|------------|--------|-----------|-------------|--------------|----------|--------|----|
| 9:00-9:30 (Coding Kickoff) | Dispatch agents, confirm task assignments | Prioritize coding tasks | Report | Report | Report | Report | Report |
| 9:30-12:00 (Coding Morning) | Monitor agent performance | Code review (Review Agent) | Implement APIs + tests (Cascade) | Implement UI (Cascade + Design Agent) | Validate implementations | Deploy infra (Cascade) | Generate tests (Testing Agent) |
| 12:00-12:15 (Sync) | Sync | Sync | Sync | Sync | Sync | Sync | Sync |
| 1:00-4:30 (Coding Afternoon) | Optimize prompts based on results | Final reviews, merge PRs | Integration, adapter tasks (Human-Only) | Integration, polish | Design QA | Staging deploy | Execute test suite |
| 4:30-5:00 (Sprint Demo) | AI metrics report | Sprint summary | Demo features | Demo UI | Validate vs design | Deploy status | Test report |

**Day 2 Deliverables:** Working code, passing tests, merged PRs, updated documentation, deployment to staging.

#### Sprint Cadence

```
Monday (retro/tech debt) → Sprint 1 (Tue-Wed) → Sprint 2 (Thu-Fri) → Monday (retro/tech debt) → ...
                                    Day 1: Spec    Day 1: Spec
                                    Day 2: Code    Day 2: Code
```

Monday of each week is reserved for: retrospective, tech debt reduction, knowledge sharing, and Oracle AI optimization.

---

### 4. Parallel Execution Plan

Identify which tasks can run in parallel across team members within each sprint:

```
Sprint 1 (small scope — spec + dev on Day 1):
  Day 1 AM (Spec):  [Tech Lead: spec P1-T001] ──parallel── [Designer: wireframe P1-T003]
  Day 1 PM (Dev):   [Backend Dev: impl P1-T001] ──parallel── [Frontend Dev: impl P1-T003]
  Day 2 (Code):     [QA: test P1-T001] ──parallel── [DevOps: deploy] ──parallel── [Backend Dev: remaining tasks]

Sprint 2 (large scope — full-day spec on Day 1):
  Day 1 (Spec):  [Tech Lead: spec P2-T001..T005] ──parallel── [Backend Dev: API contract P2-T002] ──parallel── [DevOps: infra spec]
  Day 2 (Code):  [Backend Dev: impl P2-T001] ──parallel── [Frontend Dev: impl P2-T003] ──parallel── [QA: test suite]
```

Include:
- Critical path tasks (must be sequential across sprints)
- Parallel tracks per role within each sprint day
- **Day 1 → Day 2 handoff:** specs approved on Day 1 unlock coding on Day 2 (or in Day 1 afternoon if specs finish early)
- **Cross-sprint dependencies:** which Sprint N tasks depend on Sprint N-1 deliverables
- AI agent bottlenecks (if multiple roles need the same agent simultaneously)

---

### 5. Licensed Code Task Isolation

If the project involves licensed code, create a separate section:

| Task ID | Task Name | Assigned To | Why Human-Only | Interface Dependency |
|---------|-----------|-------------|---------------|---------------------|
| [ID] | [name] | [role] | Licensed code / Adapter | [interface task ID] |

**Rules:**
- List all tasks that touch licensed code
- Mark them as Human-Only
- Identify which interface tasks must complete before adapter tasks begin
- Ensure AI agents are never assigned to these tasks

---

### 6. Risk & Bottleneck Analysis

| Risk | Impact | Mitigation |
|------|--------|------------|
| Single point of failure (one person owns critical path) | High | Cross-train or pair program |
| AI agent unavailability | Medium | Have manual fallback procedures |
| Licensed code tasks blocking AI tasks | High | Prioritize interface/adapter tasks early |
| Overloaded team member | Medium | Redistribute or defer lower-priority tasks |

---

### 7. Handoff Checklist

Define what each role must deliver before the next role can start:

| From | To | Handoff Artifact | Acceptance Criteria |
|------|----|-----------------|-------------------|
| Tech Lead | Backend Dev | Approved spec + task list | Spec reviewed, tasks in board |
| Designer | Frontend Dev | Figma designs + tokens | Design tokens extracted, components specced |
| Backend Dev | Frontend Dev | API endpoints + docs | API deployed to staging, docs updated |
| Backend Dev | QA Engineer | Feature + unit tests | >80% coverage, no critical bugs |
| Any Dev | Tech Lead | Pull request | Tests pass, Review Agent approved |
| QA Engineer | Tech Lead | Test report | All acceptance criteria verified |

---

**Guidelines:**
1. Balance workload across team members — no one should be idle or overloaded
2. Maximize parallel execution by identifying independent task tracks within each sprint day
3. Front-load licensed code interface/adapter tasks to unblock AI-assisted work — schedule interface definitions on Day 1 (Spec) so adapters can be built on Day 2 (Coding)
4. Pair junior developers with AI agents for mentoring opportunities
5. Reserve 10-15% buffer time per sprint for unexpected issues and code review
6. Ensure every task has both a human owner AND an AI agent pairing (except Human-Only tasks)
7. Day 1 is flexible: assign spec tasks (Steps 1-15) to Day 1 morning; if specs are approved early, implementation can begin in Day 1 afternoon. Day 2 is code only.
8. Each agent-coding sprint = 1 week equivalent of work compressed into 2 days — plan task volume accordingly. Choose Day 1 mode (full spec / spec+dev / parallel) based on feature scope.
9. Use Monday of each week for retrospective, tech debt reduction, and Oracle AI optimization
```

## Output

Save the generated document as `docs/task-allocation.md` in your project.

**Important:** All Status columns must be initialized to `🔴 Not Started`. The `/aspec-20-implement` workflow is responsible for updating this file as tasks progress. This makes `docs/task-allocation.md` the single source of truth for task status.

## Task Import Formats

### For Jira CSV Import

```csv
Summary,Description,Issue Type,Priority,Assignee,Labels,Story Points,Branch
"[P1-T001] Task Name","[Description]",Task,Medium,"[Assignee]","phase-1,backend",2,"feature/P1-T001-short-slug"
```

### For Gitea Issues (tea CLI)

```bash
tea issue create --title "[P1-T001] Task Name" --description "Branch: feature/P1-T001-short-slug\nAssigned: [Role]\nAI Agent: [Agent]\nDependencies: [IDs]" --labels "phase-1" --assignees "[username]"
```

### For Linear

Use Linear's CSV import with columns: Title, Description, Priority, Assignee, Labels, Estimate

## Next Steps

After completing this step, proceed to:
- `/aspec-19-testplan` — Generate Test Plan (Step 19)
- `/aspec-20-implement` — Begin implementation execution
