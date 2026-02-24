---
description: Generate Action-Function Table mapping user actions to system functions (Step 8)
arguments:
  send: true
---

# Action-Function Table Generator

Use this workflow after completing Step 7 (Function List).

## Usage

```
/aspec-08-action-function [input_files]
```

**Examples:**
- `/aspec-08-action-function docs/03-use-cases.md docs/06-actor-list.md docs/07-function-list.md`
- `/aspec-08-action-function` (will look for files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths, read those files as input documents.
If empty, look for `docs/03-use-cases.md`, `docs/06-actor-list.md`, and `docs/07-function-list.md`.

## Prerequisites

Ensure you have completed:
- Use Cases (Step 3)
- Actor List (Step 6)
- Function List (Step 7)

## Prompt

```
You are a systems analyst. Create an Action-Function Table mapping user actions to system functions.

**Input Documents:**
- Actor List (Step 6)
- Function List (Step 7)
- Use Cases (Step 3)

**Output Requirements:**

1. **Action-Function Matrix**
   | Action ID | User Action | Actor(s) | Function(s) | Use Case | UI Location |
   |-----------|-------------|----------|-------------|----------|-------------|
   | ACT-001 | Click "Sign Up" button | Guest | F-001: createUser | UC-001 | /signup page |
   | ACT-002 | Submit login form | Guest | F-002: authenticateUser | UC-002 | /login page |

2. **Action Sequences**
   For complex workflows:
   ```
   Workflow: User Registration
   1. ACT-001: Click "Sign Up" → F-001: validateEmail
   2. ACT-002: Fill form → (client-side validation)
   3. ACT-003: Submit → F-002: createUser → F-003: sendVerificationEmail
   4. ACT-004: Click email link → F-004: verifyEmail
   ```

3. **Action Triggers**
   | Trigger Type | Action | Function | Condition |
   |--------------|--------|----------|-----------|
   | User Click | Submit Order | processOrder | Cart not empty |
   | Timer | Session Timeout | logoutUser | Idle > 30 min |
   | System Event | Payment Received | fulfillOrder | Payment confirmed |

4. **Error Actions**
   | Error Condition | User Action | System Response | Function |
   |-----------------|-------------|-----------------|----------|
   | Invalid email | Form submit | Show error message | validateEmail |
   | Network error | Any action | Show retry dialog | handleNetworkError |

**Guidelines:**
1. Cover all user-initiated actions
2. Include system-triggered actions
3. Map to specific UI elements where applicable
4. Document the complete action chain for workflows
5. Include error handling actions
```

## Output

Save the generated document as `docs/08-action-function-table.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-09-access-control` - Generate Access Control Design (Step 9)
