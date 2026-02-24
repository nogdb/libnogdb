---
trigger: glob
globs: ["*_test.go", "*.go"]
---

# Testing Standards

## Test Philosophy

**Prefer automated tests.** If automated testing is not feasible, create a manual test recipe.

| Priority | Test Type | When to Use |
|----------|-----------|-------------|
| 1st | Automated test | Default choice for all testable code |
| 2nd | Manual test recipe | When automation is impossible or cost-prohibitive |

## Test Types and Execution Frequency

| Test Type | Scope | Execution | Description |
|-----------|-------|-----------|-------------|
| **Unit Test** | Single function/method | Every commit, CI | Fast, isolated, fully mocked |
| **Integration Test** | Multiple components | Periodic, CI nightly | Tests component interactions |
| **Contract Test** | API boundaries | Periodic, before release | Validates API contracts |
| **E2E Test** | Full system | Before release | Full workflow validation |

## Test File Naming

```
<file>_test.go              # Unit tests
<file>_integration_test.go  # Integration tests (if needed)
```

## Test Function Naming

```go
func Test<Function>_<Scenario>_<ExpectedBehavior>(t *testing.T)

// Examples
func TestGetUser_ValidID_ReturnsUser(t *testing.T)
func TestGetUser_InvalidID_ReturnsError(t *testing.T)
func TestGetUser_NotFound_ReturnsErrNotFound(t *testing.T)
```

## Test Structure (AAA Pattern)

```go
func TestSubmitFiling_ValidRequest_ReturnsSuccess(t *testing.T) {
    // Arrange
    mockDB := new(MockDB)
    mockDB.On("Insert", mock.Anything).Return(nil)
    service := NewFilingService(mockDB)
    req := FilingRequest{...}

    // Act
    result, err := service.Submit(context.Background(), req)

    // Assert
    assert.NoError(t, err)
    assert.NotNil(t, result)
    assert.Equal(t, "submitted", result.Status)
    mockDB.AssertExpectations(t)
}
```

## Mock Guidelines

- Use `testify/mock` for interface mocking
- Keep mocks in `*_test.go` files or `mocks/` directory
- Don't mock what you don't own (wrap external dependencies first)

```go
// ❌ BAD: Mocking external library directly
type MockHTTPClient struct { ... }

// ✅ GOOD: Wrap external dependency, mock the wrapper
type HTTPClient interface {
    Do(req *http.Request) (*http.Response, error)
}

type MockHTTPClient struct {
    mock.Mock
}

func (m *MockHTTPClient) Do(req *http.Request) (*http.Response, error) {
    args := m.Called(req)
    return args.Get(0).(*http.Response), args.Error(1)
}
```

## Test Coverage Requirements

| Type | Minimum Coverage |
|------|------------------|
| Unit Tests | 80% |
| Critical Paths | 100% |
| Error Handling | 100% |

## Manual Test Recipe Template

When automated testing is not possible, create a manual test recipe:

**Location:** `docs/test-recipes/<feature>-test-recipe.md`

```markdown
# Test Recipe: <Feature Name>

| **Test ID** | TR-<number> |
|-------------|-------------|
| **Feature** | <feature name> |
| **Last Verified** | <date> |
| **Verified By** | <name> |

## Prerequisites

- [ ] Environment setup (describe)
- [ ] Test data prepared (describe)
- [ ] Dependencies running (list)

## Test Steps

### Scenario 1: <Happy Path>

| Step | Action | Expected Result | Actual Result | Pass/Fail |
|------|--------|-----------------|---------------|----------|
| 1 | <action> | <expected> | | |

## Cleanup

- [ ] Reset test data
- [ ] Stop test services
```

## Forbidden Patterns

- ❌ Tests without assertions
- ❌ Tests that depend on external services (use mocks)
- ❌ Tests with hardcoded sleep/delays
- ❌ Tests that modify shared state
- ❌ Skipping error case testing
