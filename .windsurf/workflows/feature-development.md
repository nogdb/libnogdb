---
description: End-to-end feature development workflow for full stack developers
---

# Full Stack Feature Development Workflow

Complete workflow for implementing features across the entire stack.

## Trigger

Invoke with: `/feature-development`

## Workflow Steps

### Phase 1: Specification

1. **Define Requirements**
   - Clarify user story and acceptance criteria
   - Identify API endpoints needed
   - Define data models
   - Plan UI components

2. **Create Technical Spec**
   - API contract (OpenAPI/Swagger)
   - Database schema changes
   - Component hierarchy
   - State management approach

3. **Review Checklist**
   - [ ] Requirements clear
   - [ ] API contract defined
   - [ ] Database schema designed
   - [ ] UI mockups reviewed
   - [ ] Edge cases identified

### Phase 2: Backend Implementation

1. **Database Layer**
   - Create migration scripts
   - Define models/entities
   - Implement repository interfaces

2. **Service Layer**
   - Implement business logic
   - Define service interfaces
   - Add validation rules

3. **API Layer**
   - Implement handlers/controllers
   - Add input validation
   - Configure routes
   - Add authentication/authorization

4. **Backend Tests**
   - Unit tests for services
   - Integration tests for APIs
   - Test edge cases

### Phase 3: Frontend Implementation

1. **Component Structure**
   - Create component hierarchy
   - Define props interfaces
   - Plan state management

2. **UI Implementation**
   - Implement components
   - Apply design tokens
   - Add accessibility attributes
   - Handle loading/error states

3. **API Integration**
   - Connect to backend APIs
   - Handle responses/errors
   - Implement optimistic updates

4. **Frontend Tests**
   - Component unit tests
   - Integration tests
   - Accessibility tests

### Phase 4: End-to-End Testing

1. **E2E Test Cases**
   - Happy path scenarios
   - Error handling scenarios
   - Edge cases

2. **Performance Testing**
   - API response times
   - Frontend rendering
   - Database queries

### Phase 5: Self-Review

1. **Code Quality**
   - [ ] Clean architecture followed
   - [ ] Interfaces at boundaries
   - [ ] No code duplication
   - [ ] Proper error handling

2. **Security**
   - [ ] Input validation
   - [ ] SQL injection prevention
   - [ ] XSS prevention
   - [ ] Auth checks in place

3. **Performance**
   - [ ] API <200ms
   - [ ] DB queries <100ms
   - [ ] No N+1 queries
   - [ ] Proper indexing

4. **Testing**
   - [ ] Coverage >80%
   - [ ] All tests passing
   - [ ] Edge cases covered

### Phase 6: Documentation

1. **Update Docs**
   - API documentation
   - Component documentation
   - README updates
   - Architecture decision records

## Output

- Working feature across full stack
- Comprehensive test suite
- Updated documentation
- Ready for code review
