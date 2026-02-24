---
description: Generate Project Scope and Phase Plan from completed spec documents (Steps 1-16)
arguments:
  send: true
---

# Project Scope and Phase Plan Generator

Use this workflow after completing Steps 1-16 of your coding specification.

## Usage

```
/aspec-17-plan [input_files]
```

**Examples:**
- `/aspec-17-plan docs/`
- `/aspec-17-plan` (will look for all files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths or a directory, read those files as input documents.
If empty, look for all `docs/01-*.md` through `docs/16-*.md` files.

## Prerequisites

Ensure you have completed the following documents:
- Project Spec Description
- User Stories
- Use Cases
- Data Model & Data Structure
- Actor List
- Function List & Action-Function Table
- Access Control Design
- Object Life Cycle
- Persistence Design
- Architecture Summary
- Internal Module Design & Abstract Interfaces
- API Design
- Project AGENTS.md
- UX Wireframe Flow & UI Design

## Prompt

```
You are a project planning specialist. Based on the following completed specification documents, create a Project Scope and Phase Plan.

**Input Documents (already completed):**
- Project Spec Description
- User Stories
- Use Cases
- Data Model & Data Structure
- Actor List
- Function List & Action-Function Table
- Access Control Design
- Object Life Cycle
- Persistence Design
- Architecture Summary
- Internal Module Design & Abstract Interfaces
- API Design
- Project AGENTS.md
- UX Wireframe Flow
- UI Design/Figma Design

**Output Requirements:**
1. **Project Scope Statement** - Clear boundaries of what is IN and OUT of scope
2. **Phase Breakdown** - Divide the project into logical phases (e.g., Phase 1: Core Infrastructure, Phase 2: User Features, Phase 3: Integration)
3. **Phase Objectives** - For each phase, define:
   - Primary deliverables
   - Success criteria
   - Dependencies on previous phases
4. **Milestone Summary** - Key milestones with estimated completion criteria
5. **Risk Assessment** - Identify phase-specific risks and mitigation strategies

Format the output as a structured markdown document suitable for team review.
```

## Output

Save the generated document as `docs/project-scope-and-phases.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-18-tasks` - Generate Phase Task List and Dependencies (Step 18)
