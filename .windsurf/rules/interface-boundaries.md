---
trigger: model_decision
description: Enforce abstract interfaces at all module boundaries for loose coupling and testability
---

# Abstract Interfaces at All Boundaries

## Core Principle

**Always define module boundaries using abstract interfaces, never concrete implementations.**

All services, repositories, gateways, and external integrations must be accessed **only through interfaces**, so any module can be swapped without affecting others.

## Why Interfaces at Boundaries?

| Benefit | Description |
|---------|-------------|
| **Loose Coupling** | Modules depend on abstractions, not implementations |
| **Testability** | Easy to mock dependencies in unit tests |
| **Swappability** | Replace implementations without changing consumers |
| **Clear Contracts** | Interfaces define explicit API contracts |
| **Parallel Development** | Teams can work independently against interfaces |

## Boundary Types

| Boundary | Interface Required | Description |
|----------|-------------------|-------------|
| Service Layer | Yes | Business logic services |
| Repository/Data Access | Yes | Database and storage access |
| External APIs | Yes | Third-party API integrations |
| Message Queues | Yes | Event publishers/subscribers |
| File Storage | Yes | File system operations |
| Cache | Yes | Caching layer |
| Internal utilities | Optional | Helper functions may be concrete |

## Interface Design Principles

### 1. Consumer-Defined Interfaces

Define interfaces where they are **consumed**, not where they are implemented:

```
service/
├── interfaces.go      # Consumer defines what it needs
└── user_service.go    # Uses the interfaces

repository/
└── postgres/
    └── user_repo.go   # Implements the interface
```

### 2. Interface Segregation

Keep interfaces **small and focused**. A client should not depend on methods it doesn't use.

```
// ✅ GOOD: Small, focused interfaces
UserReader    { FindByID(), FindByEmail() }
UserWriter    { Create(), Update(), Delete() }

// ❌ BAD: Large interface with unrelated methods
UserRepository { FindByID(), Create(), Update(), Delete(), 
                 FindByEmail(), FindByRole(), CountAll(), ... }
```

### 3. No Implementation Leakage

Interface methods should not expose implementation details:

```
// ✅ GOOD: Abstract contract
Save(entity *Entity) error

// ❌ BAD: Exposes SQL implementation
ExecuteSQL(query string) error
```

## Dependency Injection Pattern

```
┌─────────────────────────────────────────────────────────┐
│                   Composition Root                       │
│  (main.go / app bootstrap)                              │
│                                                         │
│  1. Create concrete implementations                     │
│  2. Wire dependencies via constructor injection         │
│  3. Pass interfaces to consumers                        │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│                    Service Layer                         │
│                                                         │
│  - Depends on interfaces only                           │
│  - No knowledge of concrete implementations             │
│  - Receives dependencies via constructor                │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│                 Infrastructure Layer                     │
│                                                         │
│  - Implements interfaces                                │
│  - Contains concrete implementations                    │
│  - PostgresRepo, RedisCache, NATSPublisher, etc.       │
└─────────────────────────────────────────────────────────┘
```

## Interface Location Strategy

| Approach | When to Use |
|----------|-------------|
| **Consumer-side** (recommended) | Interface defined in the package that uses it |
| **Shared package** | When multiple consumers need the same interface |
| **Provider-side** | Only for widely-used library interfaces |

## Best Practices

| Practice | Description |
|----------|-------------|
| Interface Segregation | Small, focused interfaces over large ones |
| Consumer-Defined | Define interfaces where they're consumed |
| Constructor Injection | Pass interfaces via constructor, not setter |
| No Implementation Leakage | Interface methods shouldn't expose implementation details |
| Mock in Tests | Use interface mocks for unit testing |
| Single Responsibility | Each interface should have one reason to change |

## Forbidden Patterns

- ❌ Depending on concrete types across module boundaries
- ❌ Importing implementation packages in domain/service layer
- ❌ Defining interfaces with implementation-specific methods
- ❌ Using global/singleton instances instead of injected interfaces
- ❌ Type assertions to concrete types (defeats interface purpose)
- ❌ Interfaces with too many methods (violates ISP)
- ❌ Setter injection instead of constructor injection

## Language-Specific Implementation

See stack-specific rules for implementation examples:
- **Go**: `rules/go-react-flutter-jetstream/go-standards.md`
- **Rust**: `rules/rust-flutter-jetstream/rust-standards.md`
- **TypeScript**: `rules/go-react-flutter-jetstream/react-typescript-standards.md`
- **Dart/Flutter**: `rules/go-react-flutter-jetstream/flutter-mobile-standards.md`
