---
name: data-flow-validation
description: Validate that code follows the ASVS V1.1 data flow architecture pattern for secure input/output processing
---

# Data Flow Validation Skill

## Overview

This skill validates that code follows the ASVS V1.1 data flow architecture pattern for secure input/output processing.

## When to Use

- Reviewing new API endpoints
- Auditing existing handlers
- Security code reviews
- Before merging PRs with input handling

## Data Flow Pattern

```
INPUT → DECODE/NORMALIZE → VALIDATE/SANITIZE → PROCESS → STORE → ENCODE → OUTPUT
```

## Validation Steps

### Step 1: Identify Entry Points

Find all input entry points:

```bash
# HTTP handlers
grep -r "func.*gin.HandlerFunc" --include="*.go"
grep -r "c.BindJSON\|c.ShouldBindJSON" --include="*.go"

# NATS subscribers
grep -r "nats.MsgHandler\|msg.Data" --include="*.go"

# CLI inputs
grep -r "flag\.\|os.Args" --include="*.go"
```

### Step 2: Trace Data Flow

For each entry point, trace the data:

1. **INPUT**: Where does raw data enter?
2. **DECODE**: Is it decoded once at entry?
3. **NORMALIZE**: Is it normalized (trim, lowercase)?
4. **VALIDATE**: Is structure validated?
5. **SANITIZE**: Is dangerous content removed?
6. **PROCESS**: Is only validated data used?
7. **STORE**: Is original form stored?
8. **ENCODE**: Is output context-encoded?

### Step 3: Check Validation Order

```go
// Correct order:
func ValidateInput(raw []byte) (*DomainModel, error) {
    // 1. Decode
    var input InputDTO
    json.Unmarshal(raw, &input)
    
    // 2. Normalize
    input.Email = strings.TrimSpace(strings.ToLower(input.Email))
    
    // 3. Validate structure
    validator.ValidateStruct(input)
    
    // 4. Validate business rules
    validateBusinessRules(input)
    
    // 5. Sanitize
    input.Description = sanitize(input.Description)
    
    // 6. Transform to domain
    return toDomainModel(input), nil
}
```

### Step 4: Verify Encoding Rules

| Context | Required Encoding |
|---------|-------------------|
| HTML output | HTML entity encoding |
| JSON output | JSON encoding |
| URL parameter | URL encoding |
| SQL query | Parameterized queries |
| Shell command | Avoid or use allowlist |

### Step 5: Document Data Flow

Create documentation for each endpoint:

```markdown
## POST /api/v1/filing

| Stage | Implementation | Notes |
|-------|----------------|-------|
| Input | HTTP POST body (JSON) | Max 10MB |
| Decode | `json.Unmarshal` | Strict mode |
| Validate | JSON Schema + business rules | |
| Process | `FilingService.Submit()` | |
| Store | PostgreSQL `inst` table | Original form |
| Encode | `json.Marshal` | Response DTO |
| Output | HTTP 200 JSON | |
```

## Common Issues

### Issue 1: Multiple Decode Points

```go
// ❌ BAD: Decoding at multiple points
func Handler(c *gin.Context) {
    var req Request
    c.BindJSON(&req)  // First decode
    
    // Later in service...
    json.Unmarshal(req.Data, &nested)  // Second decode - WRONG
}

// ✅ GOOD: Single decode at entry
func Handler(c *gin.Context) {
    var req FullRequest  // Include nested structure
    c.BindJSON(&req)     // Single decode
}
```

### Issue 2: Processing Before Validation

```go
// ❌ BAD: Processing raw input
func Handler(c *gin.Context) {
    var req Request
    c.BindJSON(&req)
    db.Save(req)  // No validation!
}

// ✅ GOOD: Validate before process
func Handler(c *gin.Context) {
    var req Request
    c.BindJSON(&req)
    if err := validate(req); err != nil {
        c.JSON(400, err)
        return
    }
    db.Save(req)
}
```

### Issue 3: Storing Encoded Data

```go
// ❌ BAD: Storing HTML-encoded data
user.Name = html.EscapeString(input.Name)
db.Save(user)  // Stored encoded - WRONG

// ✅ GOOD: Store original, encode on output
user.Name = input.Name  // Original
db.Save(user)
// On output:
response.Name = html.EscapeString(user.Name)
```

### Issue 4: SQL String Concatenation

```go
// ❌ BAD: String concatenation
query := "SELECT * FROM users WHERE id = '" + id + "'"

// ✅ GOOD: Parameterized query
query := "SELECT * FROM users WHERE id = $1"
db.Query(query, id)
```

## Validation Report Template

```markdown
# Data Flow Validation Report

## Endpoint: [METHOD /path]

### Data Flow Trace

| Stage | Status | Implementation | Issues |
|-------|--------|----------------|--------|
| Input | ✅/❌ | | |
| Decode | ✅/❌ | | |
| Normalize | ✅/❌ | | |
| Validate | ✅/❌ | | |
| Sanitize | ✅/❌ | | |
| Process | ✅/❌ | | |
| Store | ✅/❌ | | |
| Encode | ✅/❌ | | |
| Output | ✅/❌ | | |

### Findings

1. [Finding description]

### Recommendations

1. [Recommendation]
```

## Quick Checks

```bash
# Find potential SQL injection
grep -r "fmt.Sprintf.*SELECT\|fmt.Sprintf.*INSERT" --include="*.go"

# Find missing validation
grep -r "c.BindJSON" --include="*.go" -A 5 | grep -v "validate\|Validate"

# Find direct output without encoding
grep -r "c.String\|c.Data" --include="*.go"
```
