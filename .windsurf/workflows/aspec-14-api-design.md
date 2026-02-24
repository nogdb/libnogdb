---
description: Generate API Design with endpoints and request/response schemas (Step 14)
arguments:
  send: true
---

# API Design Generator

Use this workflow after completing Step 13 (Internal Module Design).

## Usage

```
/aspec-14-api-design [input_files]
```

**Examples:**
- `/aspec-14-api-design docs/05-data-structure.md docs/07-function-list.md docs/09-access-control.md docs/13-module-design.md`
- `/aspec-14-api-design` (will look for files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths, read those files as input documents.
If empty, look for `docs/05-data-structure.md`, `docs/07-function-list.md`, `docs/09-access-control.md`, and `docs/13-module-design.md`.

## Prerequisites

Ensure you have completed:
- Data Structure (Step 5)
- Function List (Step 7)
- Access Control Design (Step 9)
- Internal Module Design (Step 13)

## Prompt

```
You are an API architect. Design the complete API based on the system functions and data structures.

**Input Documents:**
- Function List (Step 7)
- Data Structure (Step 5)
- Access Control Design (Step 9)
- Internal Module Design (Step 13)

**Output Requirements:**

1. **API Overview**
   - Base URL: `https://api.example.com/v1`
   - Authentication: Bearer JWT
   - Content-Type: application/json
   - API versioning strategy

2. **Endpoint Catalog**
   | Method | Endpoint | Description | Auth | Permission |
   |--------|----------|-------------|------|------------|
   | POST | /auth/register | Register new user | No | - |
   | POST | /auth/login | User login | No | - |
   | GET | /users/:id | Get user profile | Yes | user:read |
   | PUT | /users/:id | Update user | Yes | user:write |

3. **Endpoint Details**
   For each endpoint:
   ```yaml
   POST /users
   Description: Create a new user
   Authentication: Required (Admin only)
   Permission: user:create
   
   Request:
     Headers:
       Authorization: Bearer <token>
       Content-Type: application/json
     Body:
       {
         "email": "user@example.com",
         "password": "securePassword123",
         "name": "John Doe"
       }
   
   Response 201 (Created):
     {
       "id": "uuid",
       "email": "user@example.com",
       "name": "John Doe",
       "createdAt": "2024-01-01T00:00:00Z"
     }
   
   Response 400 (Bad Request):
     {
       "error": "VALIDATION_ERROR",
       "message": "Email already exists",
       "details": [...]
     }
   
   Response 401 (Unauthorized):
     {
       "error": "UNAUTHORIZED",
       "message": "Invalid or expired token"
     }
   ```

4. **Common Response Formats**
   ```typescript
   // Success response
   interface SuccessResponse<T> {
     data: T;
     meta?: {
       pagination?: Pagination;
     };
   }
   
   // Error response
   interface ErrorResponse {
     error: string;
     message: string;
     details?: ValidationError[];
     requestId: string;
   }
   ```

5. **API Standards**
   - Naming conventions (kebab-case for URLs)
   - Pagination format (page/limit or cursor)
   - Filtering and sorting query params
   - Rate limiting headers
   - CORS configuration

6. **OpenAPI/Swagger Specification**
   Provide OpenAPI 3.0 spec for key endpoints.

**Guidelines:**
1. Follow RESTful conventions
2. Use appropriate HTTP methods and status codes
3. Design for backward compatibility
4. Include rate limiting considerations
5. Document all error responses
6. Consider HATEOAS for discoverability (optional)
```

## Output

Save the generated document as `docs/14-api-design.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-15-agents-md` - Generate Project AGENTS.md (Step 15)
