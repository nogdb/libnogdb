---
description: Generate Use Cases from Project Spec and User Stories (Step 3)
arguments:
  send: true
---

# Use Cases Generator

Use this workflow after completing Step 2 (User Stories).

## Usage

```
/aspec-03-use-cases [input_files]
```

**Examples:**
- `/aspec-03-use-cases docs/01-project-spec.md docs/02-user-stories.md`
- `/aspec-03-use-cases` (will look for files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths, read those files as input documents.
If empty, look for `docs/01-project-spec.md` and `docs/02-user-stories.md`.

## Prerequisites

Ensure you have completed:
- Project Spec Description (Step 1)
- User Stories (Step 2)

## Prompt

```
You are a systems analyst. Based on the Project Spec and User Stories, create detailed Use Cases.

**Input Documents:**
- Project Spec Description (Step 1)
- User Stories (Step 2)

**Output Requirements:**

For each major feature, create a use case with:

**Use Case ID**: UC-[XXX]
**Use Case Name**: [Descriptive name]
**Actor(s)**: [Primary and secondary actors]
**Description**: [Brief description]

**Preconditions:**
- [Condition that must be true before use case starts]

**Postconditions:**
- [State of system after successful completion]

**Main Flow (Happy Path):**
1. [Actor action]
2. [System response]
3. [Actor action]
4. [System response]
...

**Alternative Flows:**
- **[Alt Flow ID]**: [Description]
  - At step [X], if [condition], then [alternative steps]

**Exception Flows:**
- **[Exception ID]**: [Description]
  - At step [X], if [error condition], then [error handling steps]

**Business Rules:**
- BR-[XXX]: [Rule description]

**Related User Stories**: US-[XXX], US-[XXX]

**Guidelines:**
1. Number all steps clearly
2. Keep steps atomic (one action per step)
3. Include system responses for each user action
4. Cover all error scenarios
5. Reference related user stories for traceability
```

## Output

Save the generated document as `docs/03-use-cases.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-04-data-model` - Generate Data Model (Step 4)
