---
name: test-generation
description: Generate comprehensive test suites with unit, integration, and E2E tests
---

# Test Generation Skill

This skill helps you create comprehensive test coverage for your code.

## Test Types

### Unit Tests
- Test individual functions/methods
- Mock dependencies
- Fast execution
- High coverage target (>80%)

### Integration Tests
- Test component interactions
- Use test databases
- Verify API contracts
- Medium coverage target (>60%)

### E2E Tests
- Test full user flows
- Use real browser/environment
- Critical paths only
- Focus on happy paths + key errors

## Test Structure
```typescript
describe('ComponentName', () => {
  describe('methodName', () => {
    it('should handle normal case', () => {
      // Arrange
      // Act
      // Assert
    });

    it('should handle edge case', () => {
      // Test edge cases
    });

    it('should handle error case', () => {
      // Test error handling
    });
  });
});
```

## Commands
```bash
# Run unit tests
npm test

# Run with coverage
npm run test:coverage

# Run integration tests
npm run test:integration

# Run E2E tests
npm run test:e2e
```

## Coverage Targets
| Type | Target |
|------|--------|
| Unit | >80% |
| Integration | >60% |
| E2E | Critical paths |

## Best Practices
- Test behavior, not implementation
- Use descriptive test names
- One assertion per test when possible
- Mock external dependencies
- Use factories for test data
