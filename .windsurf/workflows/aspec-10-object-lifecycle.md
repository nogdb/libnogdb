---
description: Generate Object Life Cycle with state diagrams and transition rules (Step 10)
arguments:
  send: true
---

# Object Life Cycle Generator

Use this workflow after completing Step 9 (Access Control Design).

## Usage

```
/aspec-10-object-lifecycle [input_files]
```

**Examples:**
- `/aspec-10-object-lifecycle docs/03-use-cases.md docs/04-data-model.md docs/07-function-list.md`
- `/aspec-10-object-lifecycle` (will look for files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths, read those files as input documents.
If empty, look for `docs/03-use-cases.md`, `docs/04-data-model.md`, and `docs/07-function-list.md`.

## Prerequisites

Ensure you have completed:
- Use Cases (Step 3)
- Data Model (Step 4)
- Function List (Step 7)

## Prompt

```
You are a domain modeler. Define the Object Life Cycle for key domain entities.

**Input Documents:**
- Data Model (Step 4)
- Use Cases (Step 3)
- Function List (Step 7)

**Output Requirements:**

For each key entity with state transitions:

1. **State Diagram** (Mermaid format)
   ```mermaid
   stateDiagram-v2
       [*] --> Draft
       Draft --> Pending: submit()
       Pending --> Approved: approve()
       Pending --> Rejected: reject()
       Rejected --> Draft: revise()
       Approved --> [*]
   ```

2. **State Definitions**
   | State | Description | Entry Conditions | Exit Conditions | Allowed Actions |
   |-------|-------------|------------------|-----------------|-----------------|
   | Draft | Initial state | Object created | submit() called | edit, delete, submit |
   | Pending | Awaiting approval | Submitted | approve/reject | cancel |

3. **Transition Rules**
   | From State | To State | Trigger | Guard Condition | Action | Actor |
   |------------|----------|---------|-----------------|--------|-------|
   | Draft | Pending | submit() | All required fields filled | Send notification | Owner |
   | Pending | Approved | approve() | Approver has permission | Update status, notify | Admin |

4. **State-Specific Business Rules**
   ```
   State: Pending
   - Cannot be edited by owner
   - Auto-expires after 7 days → Rejected
   - Notification sent to approvers
   ```

5. **Audit Trail Requirements**
   - Which transitions to log
   - What data to capture (who, when, from, to, reason)

**Guidelines:**
1. Identify all entities with meaningful state
2. Define clear entry/exit conditions
3. Document guard conditions for transitions
4. Include timeout/automatic transitions
5. Consider parallel states if needed
```

## Output

Save the generated document as `docs/10-object-lifecycle.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-11-persistence` - Generate Persistence Design (Step 11)
