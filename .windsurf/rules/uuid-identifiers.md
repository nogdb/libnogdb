---
trigger: glob
globs: ["*.go", "*.ts", "*.tsx", "*.dart", "*.rs", "*.sql"]
---

# UUID for Object Identifiers

All IDs referencing objects in the system MUST be represented as UUIDs.

## Core Principle

**Use native UUID type internally; convert to string only for display/output.**

| Context | UUID Form | Example |
|---------|-----------|----------|
| Internal processing | Native type | `uuid.UUID`, `Uuid` |
| Database storage | Native UUID column | `UUID` type in PostgreSQL |
| Function parameters | Native type | `func GetUser(id uuid.UUID)` |
| Struct fields | Native type | `ID uuid.UUID` |
| JSON serialization | String (automatic) | `"id": "550e8400-..."` |
| Logging/display | String | `id.String()` |
| API input (string) | Parse immediately | `uuid.Parse(idStr)` |

## Why UUIDs?

| Benefit | Description |
|---------|-------------|
| **Globally Unique** | No collisions across distributed systems |
| **Unpredictable** | Cannot guess valid IDs (security) |
| **Decentralized** | Generate IDs without database round-trip |
| **Merge-friendly** | No conflicts when merging data from multiple sources |

## Go Implementation

```go
import "github.com/google/uuid"

// ✅ GOOD: Native UUID type for struct fields
type User struct {
    ID        uuid.UUID `json:"id" db:"id"`
    AccountID uuid.UUID `json:"account_id" db:"account_id"`
    Name      string    `json:"name" db:"name"`
}

// ✅ GOOD: Functions accept native UUID type
func (s *UserService) GetUser(ctx context.Context, id uuid.UUID) (*User, error) {
    return s.repo.FindByID(ctx, id)
}

// ✅ GOOD: Parse string to UUID at API boundary, use native type internally
func (h *Handler) GetUser(w http.ResponseWriter, r *http.Request) {
    idStr := chi.URLParam(r, "id")  // String from HTTP
    id, err := uuid.Parse(idStr)    // Parse to native UUID immediately
    if err != nil {
        h.respondError(w, http.StatusBadRequest, "Invalid ID format")
        return
    }
    user, err := h.service.GetUser(r.Context(), id)  // Pass native UUID
    // ...
}

// ✅ GOOD: Convert to string only for display/logging
func (h *Handler) logUserAccess(id uuid.UUID) {
    slog.Info("user accessed", "user_id", id.String())  // String for logging
}

// ❌ BAD: Passing string internally
func (s *UserService) GetUser(ctx context.Context, id string) (*User, error) {
    // Don't pass string - parse at boundary, use native type
}

// ❌ BAD: Storing as string in struct
type User struct {
    ID string `json:"id"`  // Use uuid.UUID instead
}
```

## TypeScript Implementation

```typescript
import { v4 as uuidv4, validate as uuidValidate } from 'uuid';

// ✅ GOOD: Branded type for type safety (TypeScript lacks native UUID)
type UUID = string & { readonly __brand: 'UUID' };

function toUUID(id: string): UUID {
  if (!uuidValidate(id)) {
    throw new Error('Invalid UUID format');
  }
  return id as UUID;
}

function newUUID(): UUID {
  return uuidv4() as UUID;
}

interface User {
  id: UUID;        // Branded UUID type
  accountId: UUID;
  name: string;
}

// ✅ GOOD: Functions accept branded UUID type
const getUserById = (id: UUID): User | null => {
  return userRepository.findById(id);
};

// ✅ GOOD: Parse/validate at API boundary
const handleGetUser = (req: Request): User | null => {
  const idStr = req.params.id;     // String from HTTP
  const id = toUUID(idStr);        // Validate and convert to UUID type
  return getUserById(id);          // Pass typed UUID internally
};

// ❌ BAD: Raw string without validation
const getUser = (id: string): User | null => {
  return userRepository.findById(id);  // No validation!
};

// ❌ BAD: Number IDs
interface User {
  id: number;
}
```

## Dart/Flutter Implementation

```dart
import 'package:uuid/uuid.dart';

// ✅ GOOD: Wrapper class for type safety (Dart lacks native UUID)
class UuidValue {
  final String _value;
  
  UuidValue._(this._value);
  
  factory UuidValue.generate() => UuidValue._(const Uuid().v4());
  
  factory UuidValue.parse(String value) {
    if (!_isValidUuid(value)) {
      throw FormatException('Invalid UUID format: $value');
    }
    return UuidValue._(value);
  }
  
  static bool _isValidUuid(String id) {
    final uuidRegex = RegExp(
      r'^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$',
      caseSensitive: false,
    );
    return uuidRegex.hasMatch(id);
  }
  
  @override
  String toString() => _value;  // String only for display
  
  @override
  bool operator ==(Object other) =>
      other is UuidValue && other._value == _value;
  
  @override
  int get hashCode => _value.hashCode;
}

// ✅ GOOD: Use UuidValue type in models
class User {
  final UuidValue id;
  final UuidValue accountId;
  final String name;

  User({
    UuidValue? id,
    required this.accountId,
    required this.name,
  }) : id = id ?? UuidValue.generate();
  
  // Convert to string only for JSON serialization
  Map<String, dynamic> toJson() => {
    'id': id.toString(),
    'accountId': accountId.toString(),
    'name': name,
  };
}

// ❌ BAD: Raw string without wrapper
class User {
  final String id;  // No type safety
}
```

## Rust Implementation

```rust
use uuid::Uuid;

// ✅ GOOD: UUID for object IDs
#[derive(Debug, Clone)]
pub struct User {
    pub id: Uuid,
    pub account_id: Uuid,
    pub name: String,
}

impl User {
    pub fn new(name: String) -> Self {
        Self {
            id: Uuid::new_v4(),
            account_id: Uuid::nil(),
            name,
        }
    }
}

// ✅ GOOD: Parse UUID from string
fn get_user(id_str: &str) -> Result<User, Error> {
    let id = Uuid::parse_str(id_str)?;
    repository.find_by_id(id)
}

// ❌ BAD: Integer IDs
pub struct User {
    pub id: i64,
}
```

## Database Schema

```sql
-- ✅ GOOD: UUID primary key (PostgreSQL)
CREATE TABLE users (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    account_id UUID NOT NULL REFERENCES accounts(id),
    name VARCHAR(255) NOT NULL,
    created_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_users_account_id ON users(account_id);

-- ❌ BAD: Integer primary key
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    name VARCHAR(255)
);
```

## API Contracts

```json
{
  "id": "550e8400-e29b-41d4-a716-446655440000",
  "accountId": "6ba7b810-9dad-11d1-80b4-00c04fd430c8",
  "name": "John Doe"
}
```

## Best Practices

| Practice | Description |
|----------|-------------|
| Use native UUID type | `uuid.UUID` (Go), `Uuid` (Rust), branded/wrapper types (TS/Dart) |
| Parse at boundary | Convert string → UUID at API entry point |
| Pass native type | Functions accept native UUID, not string |
| String only for output | Convert to string only for logging, display, JSON |
| Generate with v4 | Use random v4 UUIDs for new objects |
| Store as UUID in DB | Use native `UUID` column type in PostgreSQL |
| Index foreign keys | UUID columns used as foreign keys should be indexed |

## Forbidden Patterns

- ❌ Integer or auto-increment IDs for objects
- ❌ Passing string IDs through internal functions (parse at boundary)
- ❌ Storing UUID as string type in structs/classes
- ❌ Custom ID formats (use standard UUID)
- ❌ Storing UUIDs as binary without good reason
- ❌ Generating IDs client-side without validation server-side
