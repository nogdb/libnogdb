---
name: test-suite-generation
description: Generate comprehensive test suites for full stack features including unit tests, integration tests, component tests, and E2E tests across all application layers
---

# Test Suite Generation Skill

Generate comprehensive test suites for full stack features.

## Overview

This skill helps full stack developers create complete test coverage across all layers of the application.

## When to Use

- After implementing a new feature
- When adding tests to existing code
- Before submitting a PR

## Input Required

- Feature/component to test
- Existing code files
- Expected behaviors

## Test Layers

### 1. Unit Tests (Backend)

```go
func TestUserService_GetUser(t *testing.T) {
    // Arrange
    mockRepo := mocks.NewUserRepository(t)
    mockRepo.On("FindByID", mock.Anything, testUserID).Return(testUser, nil)
    
    service := NewUserService(mockRepo)
    
    // Act
    user, err := service.GetUser(context.Background(), testUserID)
    
    // Assert
    require.NoError(t, err)
    assert.Equal(t, testUser.ID, user.ID)
    mockRepo.AssertExpectations(t)
}
```

### 2. Integration Tests (API)

```go
func TestAPI_GetUser(t *testing.T) {
    // Setup test server with real DB
    ts := setupTestServer(t)
    defer ts.Close()
    
    // Create test data
    user := createTestUser(t, ts.DB)
    
    // Make request
    resp, err := http.Get(ts.URL + "/api/v1/users/" + user.ID.String())
    require.NoError(t, err)
    defer resp.Body.Close()
    
    // Assert
    assert.Equal(t, http.StatusOK, resp.StatusCode)
    
    var result User
    json.NewDecoder(resp.Body).Decode(&result)
    assert.Equal(t, user.ID, result.ID)
}
```

### 3. Component Tests (Frontend)

```typescript
describe('UserCard', () => {
  it('displays user information', () => {
    render(<UserCard user={mockUser} />);
    
    expect(screen.getByText(mockUser.name)).toBeInTheDocument();
    expect(screen.getByText(mockUser.email)).toBeInTheDocument();
  });
  
  it('calls onEdit when edit button clicked', async () => {
    const onEdit = vi.fn();
    render(<UserCard user={mockUser} onEdit={onEdit} />);
    
    await userEvent.click(screen.getByRole('button', { name: /edit/i }));
    
    expect(onEdit).toHaveBeenCalledWith(mockUser);
  });
  
  it('is accessible', async () => {
    const { container } = render(<UserCard user={mockUser} />);
    const results = await axe(container);
    expect(results).toHaveNoViolations();
  });
});
```

### 4. E2E Tests

```typescript
test('user can create and view a product', async ({ page }) => {
  // Login
  await page.goto('/login');
  await page.fill('[name="email"]', 'test@example.com');
  await page.fill('[name="password"]', 'password');
  await page.click('button[type="submit"]');
  
  // Navigate to products
  await page.click('text=Products');
  await expect(page).toHaveURL('/products');
  
  // Create new product
  await page.click('text=Add Product');
  await page.fill('[name="name"]', 'Test Product');
  await page.fill('[name="price"]', '99.99');
  await page.click('button[type="submit"]');
  
  // Verify product appears
  await expect(page.locator('text=Test Product')).toBeVisible();
});
```

## Test Categories

| Category | Coverage Target | Focus |
|----------|-----------------|-------|
| Unit | >80% | Business logic |
| Integration | >60% | API contracts |
| Component | >70% | UI behavior |
| E2E | Critical paths | User flows |

## Edge Cases to Test

- Empty states
- Error responses
- Validation failures
- Unauthorized access
- Network failures
- Concurrent operations
- Boundary values

## Output

- Unit test files
- Integration test files
- Component test files
- E2E test files
- Coverage report

## Checklist

- [ ] Unit tests for all services
- [ ] Integration tests for all endpoints
- [ ] Component tests for all UI components
- [ ] E2E tests for critical flows
- [ ] Edge cases covered
- [ ] Error scenarios tested
- [ ] Coverage >80%
