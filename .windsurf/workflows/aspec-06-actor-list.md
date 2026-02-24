---
description: Generate Actor List identifying all users, systems, and external entities (Step 6)
arguments:
  send: true
---

# Actor List Generator

Use this workflow after completing Step 5 (Data Structure).

## Usage

```
/aspec-06-actor-list [input_files]
```

**Examples:**
- `/aspec-06-actor-list docs/01-project-spec.md docs/02-user-stories.md docs/03-use-cases.md`
- `/aspec-06-actor-list` (will look for files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths, read those files as input documents.
If empty, look for `docs/01-project-spec.md`, `docs/02-user-stories.md`, and `docs/03-use-cases.md`.

## Prerequisites

Ensure you have completed:
- Project Spec Description (Step 1)
- User Stories (Step 2)
- Use Cases (Step 3)

## Prompt

```
You are a systems analyst. Create a comprehensive Actor List identifying all users, systems, and external entities.

**Input Documents:**
- Project Spec Description (Step 1)
- User Stories (Step 2)
- Use Cases (Step 3)

**Output Requirements:**

1. **Human Actors**
   For each actor:
   | Actor ID | Actor Name | Description | Goals | Characteristics |
   |----------|------------|-------------|-------|-----------------|
   | A-001 | [Name] | [Who they are] | [What they want to achieve] | [Technical skill, frequency of use] |

2. **System Actors**
   | Actor ID | System Name | Type | Integration | Purpose |
   |----------|-------------|------|-------------|---------|
   | S-001 | [Name] | Internal/External | API/Event/File | [Purpose] |

3. **Actor Hierarchy**
   ```
   Guest (unauthenticated)
   └── Registered User
       ├── Regular User
       └── Premium User
   Administrator
   └── Super Admin
   ```

4. **Actor-Use Case Matrix**
   | Actor | UC-001 | UC-002 | UC-003 | ... |
   |-------|--------|--------|--------|-----|
   | A-001 | ✓ | ✓ | - | |
   | A-002 | - | ✓ | ✓ | |

5. **Actor Interactions**
   - Which actors interact with each other?
   - What information flows between actors?

**Guidelines:**
1. Include both primary and secondary actors
2. Identify actor inheritance/hierarchy
3. Document actor permissions at high level
4. Include external systems (payment gateways, email services, etc.)
5. Consider time-based actors (schedulers, cron jobs)
```

## Output

Save the generated document as `docs/06-actor-list.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-07-function-list` - Generate Function List (Step 7)
