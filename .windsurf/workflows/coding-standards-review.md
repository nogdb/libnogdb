---
name: coding-standards-review
description: Review code against organization coding standards
---

# Coding Standards Review Workflow

Invoke with `/coding-standards-review` to perform a comprehensive code review against organization standards.

## Workflow Steps

### Step 1: Architecture Review

Check clean architecture compliance:

- [ ] **Dependency Direction**: Dependencies flow inward (outer → inner)
- [ ] **Channel-Agnostic Core**: Business logic independent of HTTP/NATS
- [ ] **Layer Separation**: Handlers, services, and domain properly separated
- [ ] **Import Rules**: No forbidden imports (domain importing gin, service importing handler)

```
handler/ → service/ → domain/
   ↓          ↓          ↓
Can import  Can import  Standard
service,    domain,     library
domain      interfaces  only
```

### Step 2: Data Flow Review (ASVS V1.1)

Verify data flow pattern:

- [ ] **Input Handling**: Raw data never trusted
- [ ] **Decode/Normalize**: Single decode point at entry
- [ ] **Validate/Sanitize**: After decode, before process
- [ ] **Process**: Works with validated data only
- [ ] **Store**: Original form, not encoded
- [ ] **Encode**: Context-appropriate encoding
- [ ] **Output**: Only encoded data sent

### Step 3: Error Handling Review

Check error patterns:

- [ ] **Error Wrapping**: Errors wrapped with context (`fmt.Errorf("context: %w", err)`)
- [ ] **Sentinel Errors**: Domain errors defined (`ErrNotFound`, `ErrInvalidInput`)
- [ ] **No Panic**: No `panic()` for recoverable errors
- [ ] **Error Checking**: No ignored errors (`_, _ = fn()`)
- [ ] **Boundary Logging**: Errors logged at boundary only

### Step 4: Logging Review

Verify structured logging:

- [ ] **Context Fields**: `x_request_id`, `source`, `userid` present
- [ ] **Structured Format**: Using `WithFields()` not string interpolation
- [ ] **No Sensitive Data**: No passwords, tokens, PII logged
- [ ] **Context Propagation**: Using `getLog(c)` or `xrequestid.GetLog(ctx)`

### Step 5: Security Review

Check security compliance:

- [ ] **Input Validation**: All input validated at boundaries
- [ ] **SQL Injection**: Parameterized queries used
- [ ] **No Hardcoded Secrets**: Secrets from env vars or secret manager
- [ ] **TLS Verification**: No `InsecureSkipVerify: true` in production
- [ ] **Auth Checks**: Authorization verified before operations

### Step 6: Testing Review

Verify test coverage:

- [ ] **Test Naming**: `Test<Function>_<Scenario>_<ExpectedBehavior>`
- [ ] **AAA Pattern**: Arrange, Act, Assert structure
- [ ] **Mock Usage**: Interfaces mocked, not external libraries
- [ ] **Coverage**: Critical paths and error cases covered

### Step 7: Code Style Review

Check Go conventions:

- [ ] **Naming**: Follows conventions (PascalCase exports, camelCase internal)
- [ ] **Import Order**: stdlib, third-party, internal
- [ ] **Comments**: Exported items documented, explains "why" not "what"
- [ ] **File Organization**: interface.go, service.go, errors.go pattern

### Step 8: Git Commit Review

Verify conventional commits:

- [ ] **Format**: `<type>[scope]: <subject>`
- [ ] **Types**: feat, fix, docs, style, refactor, perf, test, chore, ci
- [ ] **Subject**: Imperative mood, 50 chars max
- [ ] **References**: Task IDs in footer (`Refs: EDOC-123`)

## Review Output

Generate a review report:

```markdown
# Code Review Report

## Summary
- **Files Reviewed**: X
- **Issues Found**: Y
- **Severity**: Critical/High/Medium/Low

## Findings

### Critical Issues
1. [Issue description and location]

### High Priority
1. [Issue description and location]

### Medium Priority
1. [Issue description and location]

### Recommendations
1. [Improvement suggestions]

## Checklist Status
- Architecture: ✅/❌
- Data Flow: ✅/❌
- Error Handling: ✅/❌
- Logging: ✅/❌
- Security: ✅/❌
- Testing: ✅/❌
- Code Style: ✅/❌
- Git Commits: ✅/❌
```

## Quick Commands

```bash
# Run linters
gofmt -l .
go vet ./...
golint ./...

# Run tests with coverage
go test -cover ./...

# Check for security issues
gosec ./...
```
