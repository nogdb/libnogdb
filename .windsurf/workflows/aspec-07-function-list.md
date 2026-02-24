---
description: Generate Function List documenting all system capabilities (Step 7)
arguments:
  send: true
---

# Function List Generator

Use this workflow after completing Step 6 (Actor List).

## Usage

```
/aspec-07-function-list [input_files]
```

**Examples:**
- `/aspec-07-function-list docs/03-use-cases.md docs/04-data-model.md docs/06-actor-list.md`
- `/aspec-07-function-list` (will look for files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths, read those files as input documents.
If empty, look for `docs/03-use-cases.md`, `docs/04-data-model.md`, and `docs/06-actor-list.md`.

## Prerequisites

Ensure you have completed:
- Use Cases (Step 3)
- Data Model (Step 4)
- Actor List (Step 6)

## Prompt

```
You are a systems architect. Create a comprehensive Function List documenting all system capabilities.

**Input Documents:**
- Use Cases (Step 3)
- Data Model (Step 4)
- Actor List (Step 6)

**Output Requirements:**

1. **Function Catalog**
   For each function:
   | Function ID | Function Name | Category | Description | Input | Output | Related UC |
   |-------------|---------------|----------|-------------|-------|--------|------------|
   | F-001 | createUser | User Management | Creates a new user account | UserCreateDTO | User | UC-001 |

2. **Function Categories**
   - Authentication & Authorization
   - User Management
   - [Domain-specific categories]
   - Administration
   - Reporting
   - Integration

3. **Function Details**
   For complex functions, provide:
   ```
   Function ID: F-001
   Name: createUser
   Description: Creates a new user account with email verification
   
   Input Parameters:
   - email: string (required, valid email format)
   - password: string (required, min 8 chars)
   - name: string (required)
   
   Output:
   - Success: User object with id, email, name, createdAt
   - Error: ValidationError | DuplicateEmailError
   
   Side Effects:
   - Sends verification email
   - Creates audit log entry
   
   Business Rules:
   - BR-001: Email must be unique
   - BR-002: Password must meet complexity requirements
   ```

4. **Function Dependencies**
   | Function | Depends On | Called By |
   |----------|------------|-----------|
   | F-001 | F-010 (sendEmail) | UC-001 |

**Guidelines:**
1. Use consistent naming conventions (verb + noun)
2. Group functions by domain/module
3. Identify reusable functions
4. Document all side effects
5. Link to related use cases
```

## Output

Save the generated document as `docs/07-function-list.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-08-action-function` - Generate Action-Function Table (Step 8)
