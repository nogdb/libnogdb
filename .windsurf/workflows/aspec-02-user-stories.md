---
description: Generate User Stories from Project Spec Description (Step 2)
arguments:
  send: true
---

# User Stories Generator

Use this workflow after completing Step 1 (Project Spec Description).

## Usage

```
/aspec-02-user-stories [input_file]
```

**Examples:**
- `/aspec-02-user-stories docs/01-project-spec.md`
- `/aspec-02-user-stories` (will prompt for file location)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains a file path, read that file as the Project Spec Description.
If empty, look for `docs/01-project-spec.md` or ask the user for the file location.

## Prerequisites

Ensure you have completed:
- Project Spec Description (Step 1)

## Prompt

```
You are a product owner. Based on the Project Spec Description, create a comprehensive set of User Stories.

**Input Document:**
- Project Spec Description (Step 1)

**Output Requirements:**

For each feature area, generate user stories in this format:

**Story ID**: US-[XXX]
**Title**: [Short descriptive title]
**As a** [type of user]
**I want** [goal/desire]
**So that** [benefit/value]

**Acceptance Criteria:**
- Given [context], when [action], then [expected result]
- Given [context], when [action], then [expected result]

**Priority**: Critical | High | Medium | Low
**Story Points**: [Estimate]

**Guidelines:**
1. Cover all user types identified in the Project Spec
2. Include both happy path and error scenarios
3. Group stories by feature area or epic
4. Ensure stories are INVEST compliant (Independent, Negotiable, Valuable, Estimable, Small, Testable)
5. Include at least 3 acceptance criteria per story

Generate stories for:
- User authentication and authorization
- Core feature workflows
- Administrative functions
- Error handling and edge cases
```

## Output

Save the generated document as `docs/02-user-stories.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-03-use-cases` - Generate Use Cases (Step 3)
