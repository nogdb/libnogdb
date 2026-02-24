---
trigger: glob
globs: ["*.go", "*.ts", "*.tsx", "*.js"]
---

# Security Guidelines

## Input Validation

- Validate ALL input at system boundaries
- Use allowlists over denylists
- Validate type, length, format, and range
- Use JSON Schema for complex structures

```go
// ✅ GOOD: Comprehensive validation
func ValidateUserInput(input UserInput) error {
    // Type validation (automatic via struct)
    // Length validation
    if len(input.Name) > 100 {
        return ErrNameTooLong
    }
    // Format validation
    if !emailRegex.MatchString(input.Email) {
        return ErrInvalidEmail
    }
    // Range validation
    if input.Age < 0 || input.Age > 150 {
        return ErrInvalidAge
    }
    return nil
}
```

## SQL Injection Prevention

Always use parameterized queries:

```go
// ❌ BAD: String concatenation
query := "SELECT * FROM users WHERE id = '" + id + "'"

// ✅ GOOD: Parameterized query
query := "SELECT * FROM users WHERE id = $1"
db.Query(query, id)
```

## Authentication & Authorization

- Use JWT for API authentication
- Validate tokens on every request
- Check permissions before operations
- Use role-based access control (RBAC)

```go
// ✅ GOOD: Check permissions before operation
func (h *Handler) DeleteUser(c *gin.Context) {
    // 1. Authenticate
    claims, err := h.auth.ValidateToken(c)
    if err != nil {
        c.JSON(401, gin.H{"error": "unauthorized"})
        return
    }
    
    // 2. Authorize
    if !claims.HasPermission("user:delete") {
        c.JSON(403, gin.H{"error": "forbidden"})
        return
    }
    
    // 3. Proceed with operation
    h.service.DeleteUser(c.Param("id"))
}
```

## Secrets Management

- NEVER hardcode secrets in code
- Use environment variables or secret managers
- Rotate secrets regularly
- Don't log secrets

```go
// ❌ BAD: Hardcoded secret
apiKey := "sk-1234567890abcdef"

// ✅ GOOD: Environment variable
apiKey := os.Getenv("API_KEY")
if apiKey == "" {
    log.Fatal("API_KEY environment variable required")
}
```

## TLS/HTTPS

- Use TLS for all external communications
- Verify certificates (don't use `InsecureSkipVerify: true` in production)
- Use TLS 1.2+ only

```go
// ❌ BAD: Insecure TLS
client := &http.Client{
    Transport: &http.Transport{
        TLSClientConfig: &tls.Config{InsecureSkipVerify: true},
    },
}

// ✅ GOOD: Proper TLS verification
client := &http.Client{
    Transport: &http.Transport{
        TLSClientConfig: &tls.Config{
            MinVersion: tls.VersionTLS12,
        },
    },
}
```

## Required Headers

| Header | Description | Required |
|--------|-------------|----------|
| `Content-Type` | `application/json` | Yes |
| `Authorization` | `Bearer {token}` | Yes (authenticated endpoints) |
| `X-Request-ID` | Request correlation ID | Recommended |

## Forbidden Patterns

- ❌ Hardcoded credentials
- ❌ String concatenation for SQL
- ❌ `InsecureSkipVerify: true` in production
- ❌ Logging sensitive data (passwords, tokens, PII)
- ❌ Trusting client-provided data without validation
- ❌ Using deprecated crypto algorithms (MD5, SHA1 for security)
