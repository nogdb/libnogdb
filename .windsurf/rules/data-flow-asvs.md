---
trigger: glob
globs: ["*.go", "**/handler/**", "**/validator/**", "**/service/**"]
---

# Data Flow Architecture (ASVS V1.1)

## Data Flow Pattern

Follow the ASVS V1.1 data flow pattern for all input/output processing:

```
INPUT → DECODE/NORMALIZE → VALIDATE/SANITIZE → PROCESS → STORE → ENCODE → OUTPUT
```

| Stage | Description | Rules |
|-------|-------------|-------|
| **INPUT** | Raw data from external source | Never trust, always process |
| **DECODE/NORMALIZE** | Convert to canonical form | Decode once, at entry point |
| **VALIDATE/SANITIZE** | Check constraints, clean data | After decode, before process |
| **PROCESS** | Business logic | Work with validated data only |
| **STORE** | Persist to database/file | Store in original form, not encoded |
| **ENCODE** | Prepare for output context | Apply context-appropriate encoding |
| **OUTPUT** | Send to external destination | Encoded data only |

## Input Validation Order

```go
func ValidateInput(raw []byte) (*DomainModel, error) {
    // 1. Decode (JSON, base64, URL encoding, etc.)
    var input InputDTO
    if err := json.Unmarshal(raw, &input); err != nil {
        return nil, fmt.Errorf("decode error: %w", err)
    }
    
    // 2. Normalize (trim, lowercase where appropriate)
    input.Email = strings.TrimSpace(strings.ToLower(input.Email))
    
    // 3. Validate structure (required fields, types)
    if err := validator.ValidateStruct(input); err != nil {
        return nil, fmt.Errorf("validation error: %w", err)
    }
    
    // 4. Validate business rules (ranges, patterns, relationships)
    if err := validateBusinessRules(input); err != nil {
        return nil, fmt.Errorf("business rule error: %w", err)
    }
    
    // 5. Sanitize (remove/escape dangerous content)
    input.Description = sanitize(input.Description)
    
    // 6. Transform to domain model
    return toDomainModel(input), nil
}
```

## Encoding Rules by Context

| Context | Encoding Required |
|---------|-------------------|
| HTML output | HTML entity encoding |
| JSON output | JSON encoding (automatic in Go) |
| URL parameter | URL encoding |
| SQL query | Parameterized queries (not string encoding) |
| Shell command | Avoid; if necessary, use allowlist |
| XML output | XML entity encoding |
| CSV output | Quote fields, escape quotes |

## Validation Checklist

- [ ] All input decoded at entry point
- [ ] Input normalized to canonical form
- [ ] Structure validated (required fields, types)
- [ ] Business rules validated (ranges, patterns)
- [ ] Dangerous content sanitized
- [ ] Output encoded for target context

## Data Flow Documentation

For each endpoint, document the data flow:

```markdown
## POST /api/v1/filing

| Stage | Implementation | Notes |
|-------|----------------|-------|
| Input | HTTP POST body (JSON) | Max 10MB |
| Decode | `json.Unmarshal` | Strict mode |
| Validate | JSON Schema + business rules | See `validator/rules/` |
| Process | `FilingService.Submit()` | Core function |
| Store | PostgreSQL `inst` table | Original form |
| Encode | `json.Marshal` | Response DTO |
| Output | HTTP 200 JSON | |
```

## Forbidden Patterns

- ❌ Processing raw input without validation
- ❌ Storing encoded data (encode only on output)
- ❌ Decoding at multiple points
- ❌ String concatenation for SQL queries
- ❌ Trusting client-provided data
