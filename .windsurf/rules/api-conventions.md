---
trigger: glob
globs: ["**/handler/**", "**/api/**", "*.go"]
---

# API Conventions

## URL Structure

```
/api/v{version}/{resource}[/{id}][/{sub-resource}]
```

**Examples:**

```
GET    /api/v1/inst                    # List instruments
POST   /api/v1/inst                    # Create instrument
GET    /api/v1/inst/{id}               # Get instrument
PUT    /api/v1/inst/{id}               # Update instrument
DELETE /api/v1/inst/{id}               # Delete instrument
GET    /api/v1/inst/{id}/receipts      # Get instrument receipts
```

## Request Format

```json
{
  "data": { ... },
  "metadata": { ... }
}
```

## Response Formats

**Success Response:**
```json
{
  "code": "S00000",
  "message": "success",
  "data": { ... }
}
```

**Error Response:**
```json
{
  "code": "E00001",
  "message": "validation error",
  "errors": [
    { "field": "tax_id", "message": "required" }
  ]
}
```

## HTTP Status Codes

| Code | Usage |
|------|-------|
| 200 | Success |
| 201 | Created |
| 400 | Bad Request (validation error) |
| 401 | Unauthorized |
| 403 | Forbidden |
| 404 | Not Found |
| 409 | Conflict |
| 500 | Internal Server Error |

## Required Headers

| Header | Description | Required |
|--------|-------------|----------|
| `Content-Type` | `application/json` | Yes |
| `Authorization` | `Bearer {token}` | Yes (authenticated endpoints) |
| `X-Request-ID` | Request correlation ID | Recommended |
| `namespace` | Tenant namespace | Internal APIs |

## Handler Pattern

```go
func HandleGetUser(s *UserService) gin.HandlerFunc {
    return func(c *gin.Context) {
        // 1. Extract and validate input
        id := c.Param("id")
        if id == "" {
            c.JSON(400, gin.H{"code": "E00001", "message": "id required"})
            return
        }
        
        // 2. Call service (core function)
        user, err := s.GetUser(c.Request.Context(), id)
        
        // 3. Handle errors
        if err != nil {
            switch {
            case errors.Is(err, ErrNotFound):
                c.JSON(404, gin.H{"code": "E00404", "message": "user not found"})
            case errors.Is(err, ErrUnauthorized):
                c.JSON(401, gin.H{"code": "E00401", "message": "unauthorized"})
            default:
                getLog(c).Error(err)
                c.JSON(500, gin.H{"code": "E00500", "message": "internal error"})
            }
            return
        }
        
        // 4. Return success response
        c.JSON(200, gin.H{
            "code":    "S00000",
            "message": "success",
            "data":    user,
        })
    }
}
```

## Versioning

- Use URL path versioning: `/api/v1/`, `/api/v2/`
- Maintain backward compatibility within major versions
- Document breaking changes in CHANGELOG

## Forbidden Patterns

- ❌ Business logic in handlers (use services)
- ❌ Direct database access in handlers
- ❌ Inconsistent response formats
- ❌ Missing error handling
- ❌ Logging sensitive request data
