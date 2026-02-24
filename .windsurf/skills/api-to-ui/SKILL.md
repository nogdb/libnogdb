---
name: api-to-ui
description: Build a complete feature from API endpoint to UI component, guiding through database schema, repository, service, API handlers, and frontend components
---

# API to UI Skill

Build a complete feature from API endpoint to UI component.

## Overview

This skill guides full stack developers through implementing a feature end-to-end, from database schema to UI component.

## When to Use

- Implementing a new CRUD feature
- Adding a new entity to the system
- Building a complete user flow

## Input Required

- Entity name (e.g., "Product", "Order")
- Fields and their types
- UI requirements (list view, detail view, form)
- Validation rules

## Steps

### 1. Database Schema

```sql
-- Create migration
CREATE TABLE {entity_plural} (
    id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    -- fields here
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE INDEX idx_{entity_plural}_created_at ON {entity_plural}(created_at);
```

### 2. Repository Interface

```go
type {Entity}Repository interface {
    FindByID(ctx context.Context, id uuid.UUID) (*{Entity}, error)
    FindAll(ctx context.Context, filter {Entity}Filter) ([]*{Entity}, error)
    Create(ctx context.Context, entity *{Entity}) error
    Update(ctx context.Context, entity *{Entity}) error
    Delete(ctx context.Context, id uuid.UUID) error
}
```

### 3. Service Layer

```go
type {Entity}Service struct {
    repo {Entity}Repository
}

func (s *{Entity}Service) Get{Entity}(ctx context.Context, id uuid.UUID) (*{Entity}, error) {
    return s.repo.FindByID(ctx, id)
}
```

### 4. API Endpoints

| Method | Path | Handler |
|--------|------|---------|
| GET | /api/v1/{entities} | List{Entities} |
| GET | /api/v1/{entities}/:id | Get{Entity} |
| POST | /api/v1/{entities} | Create{Entity} |
| PUT | /api/v1/{entities}/:id | Update{Entity} |
| DELETE | /api/v1/{entities}/:id | Delete{Entity} |

### 5. Frontend Components

```
components/
├── {Entity}List.tsx       # List view with table/grid
├── {Entity}Detail.tsx     # Detail view
├── {Entity}Form.tsx       # Create/Edit form
└── {Entity}Card.tsx       # Card component for list
```

### 6. API Client

```typescript
export const {entity}Api = {
  list: (filter?: {Entity}Filter) => 
    apiClient.get<{Entity}[]>('/api/v1/{entities}', { params: filter }),
  getById: (id: UUID) => 
    apiClient.get<{Entity}>(`/api/v1/{entities}/${id}`),
  create: (data: Create{Entity}) => 
    apiClient.post<{Entity}>('/api/v1/{entities}', data),
  update: (id: UUID, data: Update{Entity}) => 
    apiClient.put<{Entity}>(`/api/v1/{entities}/${id}`, data),
  delete: (id: UUID) => 
    apiClient.delete(`/api/v1/{entities}/${id}`),
};
```

### 7. Tests

- Unit tests for service layer
- Integration tests for API endpoints
- Component tests for UI
- E2E test for complete flow

## Output

- Database migration
- Repository implementation
- Service layer
- API handlers
- Frontend components
- API client
- Test suite

## Checklist

- [ ] Database migration created
- [ ] Repository interface defined
- [ ] Service layer implemented
- [ ] API endpoints working
- [ ] Frontend components built
- [ ] API client connected
- [ ] Tests passing (>80% coverage)
- [ ] Documentation updated
