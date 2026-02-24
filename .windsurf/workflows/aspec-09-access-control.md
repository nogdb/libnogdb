---
description: Generate Access Control Design with roles and permissions (Step 9)
arguments:
  send: true
---

# Access Control Design Generator

Use this workflow after completing Step 8 (Action-Function Table).

## Usage

```
/aspec-09-access-control [input_files]
```

**Examples:**
- `/aspec-09-access-control docs/06-actor-list.md docs/07-function-list.md docs/08-action-function-table.md`
- `/aspec-09-access-control` (will look for files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths, read those files as input documents.
If empty, look for `docs/06-actor-list.md`, `docs/07-function-list.md`, and `docs/08-action-function-table.md`.

## Prerequisites

Ensure you have completed:
- Actor List (Step 6)
- Function List (Step 7)
- Action-Function Table (Step 8)

## Prompt

```
You are a security architect. Design the Access Control system based on actors and functions.

**Input Documents:**
- Actor List (Step 6)
- Function List (Step 7)
- Action-Function Table (Step 8)

**Output Requirements:**

1. **Role Definitions**
   | Role ID | Role Name | Description | Inherits From |
   |---------|-----------|-------------|---------------|
   | R-001 | Guest | Unauthenticated user | - |
   | R-002 | User | Registered user | R-001 |
   | R-003 | Admin | System administrator | R-002 |

2. **Permission Definitions**
   | Permission ID | Permission Name | Resource | Action | Description |
   |---------------|-----------------|----------|--------|-------------|
   | P-001 | user:read | User | READ | View user profile |
   | P-002 | user:write | User | WRITE | Edit user profile |
   | P-003 | user:delete | User | DELETE | Delete user account |

3. **Role-Permission Matrix**
   | Role | user:read | user:write | user:delete | admin:* |
   |------|-----------|------------|-------------|---------|
   | Guest | Own only | - | - | - |
   | User | Own + Public | Own | Own | - |
   | Admin | All | All | All | ✓ |

4. **Resource-Level Access Rules**
   ```
   Resource: User
   - Owner: full access to own record
   - Admin: full access to all records
   - Other users: read public fields only
   
   Resource: Order
   - Owner: read own orders
   - Admin: read/write all orders
   - Other users: no access
   ```

5. **Access Control Implementation**
   - Authentication method (JWT, Session, OAuth)
   - Authorization strategy (RBAC, ABAC, or hybrid)
   - Token structure and claims
   - Permission checking pseudocode

6. **Security Constraints**
   - Password policies
   - Session management rules
   - Rate limiting rules
   - IP restrictions (if any)

**Guidelines:**
1. Follow principle of least privilege
2. Define both role-based and resource-based access
3. Consider row-level security needs
4. Document permission inheritance
5. Include API endpoint protection mapping
```

## Output

Save the generated document as `docs/09-access-control.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-10-object-lifecycle` - Generate Object Life Cycle (Step 10)
