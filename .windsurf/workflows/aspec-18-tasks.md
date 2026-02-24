---
description: Generate Phase Task List and Dependencies from Project Scope Plan (Step 17)
arguments:
  send: true
---

# Phase Task List and Dependency Generator

Use this workflow after completing Step 17 (Project Scope and Phase Plan).

## Usage

```
/aspec-18-tasks [input_files]
```

**Examples:**
- `/aspec-18-tasks docs/ docs/17-project-scope-phases.md`
- `/aspec-18-tasks` (will look for all files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths or a directory, read those files as input documents.
If empty, look for all `docs/01-*.md` through `docs/17-*.md` files.

## Prerequisites

Ensure you have completed:
- All documents from Steps 1-16
- Project Scope and Phase Plan (Step 17)

## Prompt

```
You are a technical project manager. Based on the Project Scope and Phase Plan (Step 17) and all preceding specification documents, create a detailed Phase Task List with Dependencies.

**Input Documents:**
- All documents from Steps 1-16
- Project Scope and Phase Plan (Step 17)

**Output Requirements:**
For EACH phase defined in Step 17, generate:

1. **Task Breakdown Structure**
   - Task ID (e.g., P1-T001)
   - Task Name
   - Description (1-2 sentences)
   - Estimated Effort (hours/days)
   - Required Skills/Role (Backend Dev, Frontend Dev, QA, etc.)
   - Related Spec Reference (e.g., "API Design Section 3.2", "Use Case UC-005")

2. **Dependency Matrix**
   - Task dependencies (which tasks must complete before this one starts)
   - Parallel execution opportunities
   - Critical path identification

3. **Deliverables per Task**
   - Code artifacts
   - Documentation updates
   - Test requirements

4. **Acceptance Criteria** - How to verify task completion

Format as a table or structured list that can be imported into project management tools (Jira, Linear, GitHub Issues).
```

## Output

Save the generated document as `docs/phase-task-list.md` in your project.

## Task Import Formats

### For Jira CSV Import

```csv
Summary,Description,Issue Type,Priority,Labels,Story Points
"[Task Name]","[Description]",Task,Medium,"phase-1",2
```

### For Gitea Issues (tea CLI)

```bash
tea issue create --title "[P1-T001] Task Name" --description "Description" --labels "phase-1"
```

### For Linear

Use Linear's CSV import with columns: Title, Description, Priority, Labels, Estimate

## Next Steps

After completing this step, proceed to:
- `/aspec-19-testplan` - Generate Test Plan (Step 19)
