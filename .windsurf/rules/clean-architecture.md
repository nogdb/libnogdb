---
trigger: glob
globs: ["*.go", "go.mod"]
---

# Clean Architecture Standards

## Dependency Direction

Dependencies MUST flow **inward** — outer layers know about inner layers, but inner layers know nothing about outer layers.

```
External Adapters (Outer) → Application Services (Middle) → Domain Core (Inner)
```

### Import Rules

| Package | Can Import | Cannot Import |
|---------|------------|---------------|
| `handler/` | `service/`, `domain/`, `db/` | - |
| `service/` | `domain/`, interfaces | `handler/`, `gin`, `nats` |
| `domain/` | Standard library only | Everything else |

### Channel-Agnostic Core Functions

Core business logic MUST be independent of input/output channels.

```go
// ❌ BAD: Core function coupled to HTTP
func SubmitFiling(c *gin.Context) error {
    var req FilingRequest
    c.BindJSON(&req)  // HTTP-specific
    c.JSON(200, result)  // HTTP-specific
}

// ✅ GOOD: Core function is channel-agnostic
func (s *FilingService) SubmitFiling(ctx context.Context, req FilingRequest) (*FilingResult, error) {
    // Pure business logic, no channel dependencies
    return result, nil
}

// HTTP handler (input channel) - separate
func HandleSubmitFiling(s *FilingService) gin.HandlerFunc {
    return func(c *gin.Context) {
        var req FilingRequest
        if err := c.BindJSON(&req); err != nil { ... }
        result, err := s.SubmitFiling(c.Request.Context(), req)
        c.JSON(200, result)
    }
}
```

## Dependency Injection

Prefer constructor injection over global state:

```go
// ❌ BAD: Global state
var db *sql.DB

func GetUser(id string) (*User, error) {
    return db.Query(...)
}

// ✅ GOOD: Dependency injection
type UserService struct {
    db *sql.DB
}

func NewUserService(db *sql.DB) *UserService {
    return &UserService{db: db}
}

func (s *UserService) GetUser(id string) (*User, error) {
    return s.db.Query(...)
}
```

## Layer Responsibilities

| Layer | Responsibility | Examples |
|-------|----------------|----------|
| **External Adapters** | Protocol handling, I/O | HTTP handlers, NATS subscribers, CLI |
| **Application Services** | Business workflow orchestration | FilingService, PaymentService |
| **Domain Core** | Pure business rules, entities | Inst, Receipt, TaxID value objects |

## Forbidden Patterns

- ❌ Domain importing HTTP/NATS packages
- ❌ Service importing handler packages
- ❌ Global mutable state
- ❌ Business logic in handlers
- ❌ Protocol-specific code in services
