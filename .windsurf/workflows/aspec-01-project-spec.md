---
description: Generate Project Spec Description - high-level project overview, goals, and constraints (Step 1)
arguments:
  send: true
---

# Project Spec Description Generator

Use this workflow to start your coding specification process (Step 1 of 19).

## Usage

```
/aspec-01-project-spec [project_idea_file_or_description]
```

**Examples:**
- `/aspec-01-project-spec docs/project-idea.md`
- `/aspec-01-project-spec "Build an e-commerce platform for selling handmade crafts"`

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains a file path, read that file for the project idea.
If `$ARGUMENTS` is a text description, use it directly as the project idea.
If empty, ask the user to provide a project idea.

## Prerequisites

Prepare the following information:
- Project idea or business requirement
- Target audience
- Key business goals

## Prompt

```
You are a technical product manager. Create a comprehensive Project Specification Description for the following project idea.

**Project Idea:**
[Describe your project idea here]

**Output Requirements:**

1. **Project Overview**
   - Project name and tagline
   - Problem statement (what problem does this solve?)
   - Solution summary (how does this project solve it?)

2. **Goals and Objectives**
   - Primary business goals (measurable outcomes)
   - Secondary goals
   - Success metrics (KPIs)

3. **Target Audience**
   - Primary users
   - Secondary users
   - User demographics and characteristics

4. **Scope Summary**
   - Core features (must-have)
   - Nice-to-have features
   - Explicitly out of scope

5. **Constraints and Assumptions**
   - Technical constraints (platforms, technologies, integrations)
   - Business constraints (budget, timeline, resources)
   - Assumptions made

6. **High-Level Requirements**
   - Functional requirements summary
   - Non-functional requirements (performance, security, scalability)

Format as a structured markdown document. Be specific and avoid vague statements.
```

## Output

Save the generated document as `docs/01-project-spec.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-02-user-stories` - Generate User Stories (Step 2)
