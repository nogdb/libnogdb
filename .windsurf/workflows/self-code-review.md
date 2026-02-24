---
description: Self code review workflow for full stack developers before submitting PR
---

# Self Code Review Workflow

Comprehensive self-review checklist before submitting pull requests.

## Trigger

Invoke with: `/self-code-review`

## Workflow Steps

### Step 1: Architecture Review

- [ ] Clean architecture principles followed
- [ ] Interfaces defined at all boundaries
- [ ] Dependency injection used
- [ ] No circular dependencies
- [ ] Proper layer separation (API → Service → Repository)

### Step 2: Backend Review

#### API Layer
- [ ] RESTful conventions followed
- [ ] Input validation on all endpoints
- [ ] Proper HTTP status codes
- [ ] Error responses consistent
- [ ] Authentication/authorization checks

#### Service Layer
- [ ] Business logic isolated
- [ ] Interfaces used for dependencies
- [ ] Error handling complete
- [ ] Logging added

#### Database Layer
- [ ] Proper indexing
- [ ] No N+1 queries
- [ ] Transactions used appropriately
- [ ] Migration scripts included

### Step 3: Frontend Review

#### Components
- [ ] Design tokens used
- [ ] Responsive design
- [ ] Accessibility attributes (aria-*)
- [ ] Keyboard navigation works

#### State Management
- [ ] Minimal global state
- [ ] Loading states handled
- [ ] Error states handled
- [ ] Optimistic updates where appropriate

#### Performance
- [ ] No unnecessary re-renders
- [ ] Lazy loading for large components
- [ ] Images optimized

### Step 4: Testing Review

#### Coverage
- [ ] Unit tests for business logic
- [ ] Integration tests for APIs
- [ ] Component tests for UI
- [ ] E2E tests for critical flows
- [ ] Coverage >80%

#### Quality
- [ ] Edge cases tested
- [ ] Error scenarios tested
- [ ] Mocks used appropriately
- [ ] Tests are maintainable

### Step 5: Security Review

- [ ] Input validation (frontend + backend)
- [ ] SQL injection prevention
- [ ] XSS prevention
- [ ] CSRF protection
- [ ] Sensitive data not logged
- [ ] Credentials not hardcoded

### Step 6: Performance Review

- [ ] API response time <200ms
- [ ] Database queries <100ms
- [ ] No blocking operations
- [ ] Caching where appropriate

### Step 7: Documentation Review

- [ ] API documentation updated
- [ ] README updated if needed
- [ ] Code comments for complex logic
- [ ] Architecture decisions documented

### Step 8: Git Hygiene

- [ ] Commits follow conventional format
- [ ] Branch named correctly
- [ ] No debug code left
- [ ] No console.log/print statements
- [ ] No commented-out code

## Output

- Completed self-review checklist
- List of issues to fix before PR
- Confidence level for submission
