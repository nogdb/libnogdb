---
trigger: glob
globs: ["**/db/**", "**/repository/**", "*.go"]
---

# Database Patterns

## Transaction Management

Use explicit transactions for multi-step operations:

```go
func (s *Service) ProcessFiling(ctx context.Context, req Request) error {
    tx, err := s.db.BeginTx(ctx)
    if err != nil {
        return fmt.Errorf("begin tx: %w", err)
    }
    defer tx.Rollback()

    // Step 1
    if err := s.db.InsertFiling(tx, filing); err != nil {
        return fmt.Errorf("insert filing: %w", err)
    }

    // Step 2
    if err := s.db.UpdateStatus(tx, id, "submitted"); err != nil {
        return fmt.Errorf("update status: %w", err)
    }

    return tx.Commit()
}
```

## Concurrency Control

For long-running operations, use status-based locking:

```sql
-- Acquire lock via status update
UPDATE inst SET status = 'processing', updated_at = NOW()
WHERE id = $1 AND status = 'pending'
RETURNING id;

-- Release lock on completion
UPDATE inst SET status = 'completed', updated_at = NOW()
WHERE id = $1 AND status = 'processing';
```

## Query Patterns

### Parameterized Queries

```go
// ✅ GOOD: Parameterized query
func (m *InstModel) GetByID(id string) (*Inst, error) {
    query := `SELECT * FROM inst WHERE id = $1`
    return m.db.QueryOne(query, id)
}

// ❌ BAD: String concatenation
query := "SELECT * FROM inst WHERE id = '" + id + "'"
```

### Explicit Column Lists

```go
// ✅ GOOD: Explicit columns
func (m *InstModel) Insert(inst *Inst) error {
    query := `INSERT INTO inst (id, status, created_at) VALUES ($1, $2, $3)`
    return m.db.Exec(query, inst.ID, inst.Status, inst.CreatedAt)
}

// ❌ BAD: SELECT *
query := "SELECT * FROM inst"  // Avoid in production
```

## Repository Interface Pattern

```go
// Define interface in service package
type InstRepository interface {
    GetByID(ctx context.Context, id string) (*Inst, error)
    Create(ctx context.Context, inst *Inst) error
    Update(ctx context.Context, inst *Inst) error
    Delete(ctx context.Context, id string) error
    List(ctx context.Context, filter InstFilter) ([]*Inst, error)
}

// Implement in db package
type PostgresInstRepository struct {
    db *sql.DB
}

func (r *PostgresInstRepository) GetByID(ctx context.Context, id string) (*Inst, error) {
    query := `SELECT id, status, created_at FROM inst WHERE id = $1`
    row := r.db.QueryRowContext(ctx, query, id)
    // ...
}
```

## Context Usage

Always pass context for cancellation and timeouts:

```go
func (r *Repository) GetUser(ctx context.Context, id string) (*User, error) {
    query := `SELECT * FROM users WHERE id = $1`
    
    // Context enables cancellation and timeout
    row := r.db.QueryRowContext(ctx, query, id)
    
    var user User
    if err := row.Scan(&user.ID, &user.Name); err != nil {
        if errors.Is(err, sql.ErrNoRows) {
            return nil, ErrNotFound
        }
        return nil, fmt.Errorf("scan user: %w", err)
    }
    return &user, nil
}
```

## Connection Pool Settings

```go
db.SetMaxOpenConns(25)
db.SetMaxIdleConns(5)
db.SetConnMaxLifetime(5 * time.Minute)
db.SetConnMaxIdleTime(1 * time.Minute)
```

## Forbidden Patterns

- ❌ String concatenation for SQL queries
- ❌ Missing transaction for multi-step operations
- ❌ Ignoring context in database calls
- ❌ SELECT * in production code
- ❌ Missing error handling for database operations
- ❌ Hardcoded connection strings
