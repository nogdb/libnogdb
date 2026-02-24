---
name: api-design-principles
description: RESTful API design patterns and best practices
---

# API Design Principles

Guidelines for designing consistent, maintainable REST APIs.

## When to Use

- Designing new API endpoints
- Reviewing API contracts
- Documenting APIs
- Versioning strategies

## URL Structure

### Resource Naming

```
# ✅ GOOD: Nouns, plural, lowercase, kebab-case
GET    /users
GET    /users/{id}
GET    /users/{id}/orders
GET    /order-items

# ❌ BAD: Verbs, singular, camelCase
GET    /getUser
GET    /user/{id}
POST   /createOrder
GET    /orderItems
```

### Hierarchical Resources

```
# Parent-child relationships
GET    /users/{userId}/orders           # User's orders
GET    /users/{userId}/orders/{orderId} # Specific order
POST   /users/{userId}/orders           # Create order for user

# Limit nesting to 2 levels
# ❌ BAD: Too deep
GET    /users/{userId}/orders/{orderId}/items/{itemId}/reviews

# ✅ GOOD: Flatten with query params or separate endpoint
GET    /order-items/{itemId}/reviews
GET    /reviews?orderId={orderId}&itemId={itemId}
```

## HTTP Methods

| Method | Purpose | Idempotent | Safe |
|--------|---------|------------|------|
| GET | Retrieve resource | Yes | Yes |
| POST | Create resource | No | No |
| PUT | Replace resource | Yes | No |
| PATCH | Partial update | Yes | No |
| DELETE | Remove resource | Yes | No |

```typescript
// Express routes
router.get('/users', listUsers);           // List all
router.get('/users/:id', getUser);         // Get one
router.post('/users', createUser);         // Create
router.put('/users/:id', replaceUser);     // Full replace
router.patch('/users/:id', updateUser);    // Partial update
router.delete('/users/:id', deleteUser);   // Delete
```

## Request/Response Format

### Request Body

```typescript
// POST /users
{
  "email": "user@example.com",
  "name": "John Doe",
  "role": "user"
}

// PATCH /users/{id}
{
  "name": "Jane Doe"  // Only fields to update
}
```

### Response Format

```typescript
// Single resource
{
  "id": "usr_123",
  "email": "user@example.com",
  "name": "John Doe",
  "createdAt": "2024-01-15T10:30:00Z",
  "updatedAt": "2024-01-15T10:30:00Z"
}

// Collection with pagination
{
  "data": [
    { "id": "usr_123", "name": "John" },
    { "id": "usr_456", "name": "Jane" }
  ],
  "pagination": {
    "page": 1,
    "pageSize": 20,
    "totalItems": 150,
    "totalPages": 8
  }
}

// Error response
{
  "error": {
    "code": "VALIDATION_ERROR",
    "message": "Invalid request data",
    "details": [
      { "field": "email", "message": "Invalid email format" }
    ]
  }
}
```

## Status Codes

### Success (2xx)

| Code | When to Use |
|------|-------------|
| 200 | GET, PUT, PATCH success |
| 201 | POST created resource |
| 204 | DELETE success (no content) |

### Client Errors (4xx)

| Code | When to Use |
|------|-------------|
| 400 | Invalid request body/params |
| 401 | Not authenticated |
| 403 | Not authorized |
| 404 | Resource not found |
| 409 | Conflict (duplicate) |
| 422 | Validation failed |
| 429 | Rate limited |

### Server Errors (5xx)

| Code | When to Use |
|------|-------------|
| 500 | Unexpected server error |
| 502 | Bad gateway |
| 503 | Service unavailable |
| 504 | Gateway timeout |

## Pagination

### Offset-based (Simple)

```
GET /users?page=2&pageSize=20

Response:
{
  "data": [...],
  "pagination": {
    "page": 2,
    "pageSize": 20,
    "totalItems": 150,
    "totalPages": 8
  }
}
```

### Cursor-based (Scalable)

```
GET /users?cursor=eyJpZCI6MTAwfQ&limit=20

Response:
{
  "data": [...],
  "pagination": {
    "nextCursor": "eyJpZCI6MTIwfQ",
    "hasMore": true
  }
}
```

## Filtering & Sorting

```
# Filtering
GET /users?status=active&role=admin
GET /orders?createdAt[gte]=2024-01-01&createdAt[lte]=2024-12-31
GET /products?price[lt]=100&category=electronics

# Sorting
GET /users?sort=createdAt:desc
GET /products?sort=price:asc,name:asc

# Field selection
GET /users?fields=id,name,email
```

## Versioning

### URL Versioning (Recommended)

```
GET /v1/users
GET /v2/users
```

### Header Versioning

```
GET /users
Accept: application/vnd.api+json; version=2
```

## Authentication

```typescript
// Bearer token
Authorization: Bearer eyJhbGciOiJIUzI1NiIs...

// API Key
X-API-Key: sk_live_abc123

// Middleware
async function authenticate(req, res, next) {
  const token = req.headers.authorization?.replace('Bearer ', '');
  if (!token) {
    return res.status(401).json({ error: { code: 'UNAUTHORIZED' } });
  }
  
  try {
    req.user = await verifyToken(token);
    next();
  } catch {
    res.status(401).json({ error: { code: 'INVALID_TOKEN' } });
  }
}
```

## Rate Limiting

```typescript
// Response headers
X-RateLimit-Limit: 100
X-RateLimit-Remaining: 95
X-RateLimit-Reset: 1640995200

// 429 response
{
  "error": {
    "code": "RATE_LIMITED",
    "message": "Too many requests",
    "retryAfter": 60
  }
}
```

## OpenAPI Documentation

```yaml
openapi: 3.0.0
info:
  title: User API
  version: 1.0.0

paths:
  /users:
    get:
      summary: List users
      parameters:
        - name: page
          in: query
          schema:
            type: integer
            default: 1
      responses:
        '200':
          description: Success
          content:
            application/json:
              schema:
                $ref: '#/components/schemas/UserList'
    post:
      summary: Create user
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: '#/components/schemas/CreateUser'
      responses:
        '201':
          description: Created

components:
  schemas:
    User:
      type: object
      properties:
        id:
          type: string
        email:
          type: string
          format: email
        name:
          type: string
```

## Best Practices

1. **Use nouns, not verbs** - Resources are things
2. **Be consistent** - Same patterns everywhere
3. **Use proper status codes** - Not just 200 and 500
4. **Version your API** - Plan for changes
5. **Document everything** - OpenAPI/Swagger
6. **Validate inputs** - Never trust client data
7. **Use HTTPS** - Always encrypt in transit

## Checklist

- [ ] RESTful URL structure
- [ ] Proper HTTP methods
- [ ] Consistent response format
- [ ] Appropriate status codes
- [ ] Pagination implemented
- [ ] Filtering and sorting
- [ ] API versioning strategy
- [ ] Authentication/authorization
- [ ] Rate limiting
- [ ] OpenAPI documentation
