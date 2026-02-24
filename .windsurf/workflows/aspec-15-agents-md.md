---
description: Generate Project AGENTS.md for AI coding assistant context (Step 15)
arguments:
  send: true
---

# Project AGENTS.md Generator

Use this workflow after completing Step 14 (API Design).

## Usage

```
/aspec-15-agents-md [input_files]
```

**Examples:**
- `/aspec-15-agents-md docs/`
- `/aspec-15-agents-md` (will look for all files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths or a directory, read those files as input documents.
If empty, look for all `docs/01-*.md` through `docs/14-*.md` files.

## Prerequisites

Ensure you have completed:
- All documents from Steps 1-14
- Team structure and roles
- Technology stack decisions

## Prompt

```
You are an AI integration specialist. Create a comprehensive AGENTS.md file that provides context for AI coding assistants working on this project.

**Input Documents:**
- All documents from Steps 1-14 (especially Architecture Summary, API Design, Data Structure)

**Output Requirements:**

1. **Project Overview Section**
   ```markdown
   # Project Development Guidelines
   
   ## Project Context
   - Project name and purpose
   - Key business domain concepts
   - Target users and their needs
   ```

2. **Architecture Context**
   - High-level architecture summary
   - Key components and their responsibilities
   - Technology stack with versions
   - Directory structure overview

3. **Code Standards**
   ```markdown
   ## Code Quality Standards
   - Naming conventions (files, functions, variables)
   - Code style (formatting, linting rules)
   - Documentation requirements
   - Test coverage targets
   ```

4. **Git Workflow**
   - Branch naming conventions
   - Commit message format (conventional commits)
   - PR/MR requirements
   - Protected branches

5. **API Guidelines**
   - Endpoint naming patterns
   - Request/response formats
   - Error handling standards
   - Authentication requirements

6. **Database Guidelines**
   - Schema conventions
   - Migration practices
   - Query optimization rules
   - Data validation requirements

7. **Security Guidelines**
   - Authentication/authorization patterns
   - Input validation requirements
   - Sensitive data handling
   - Forbidden actions (what AI should never do)

8. **Performance Targets**
   | Metric | Target |
   |--------|--------|
   | API response time | < 200ms |
   | Database query time | < 100ms |
   | Test coverage | > 80% |

9. **AI Agent Coordination** (if using multiple agents)
   - Available agents and their roles
   - Handoff protocols
   - Shared context requirements

**Template Structure:**
```markdown
# Project Development Guidelines

## Project Context
[Brief description]

## Core Principles
### [Principle 1]
### [Principle 2]

## Technology Stack
| Layer | Technology | Version |
|-------|------------|---------|

## Code Standards
### Naming Conventions
### File Structure
### Documentation

## Git Workflow
### Branch Naming
### Commit Format

## API Guidelines

## Database Guidelines

## Security Guidelines

## Performance Targets

## Forbidden Actions
- Never [action 1]
- Never [action 2]
```

**Guidelines:**
1. Keep it concise but comprehensive
2. Use tables for quick reference
3. Include specific examples where helpful
4. Focus on what AI agents need to know
5. Update as project evolves
```

## Output

Save the generated document as `AGENTS.md` in your project root.

Also save a copy as `docs/15-agents-md.md` for documentation purposes.

## Next Steps

After completing this step, proceed to:
- `/aspec-16-ux-design` - Generate UX Wireframe Flow & UI Design (Step 16)
