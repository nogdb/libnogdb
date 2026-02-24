---
name: test-driven-development
description: RED-GREEN-REFACTOR cycle for test-driven development with anti-patterns reference
---

# Test-Driven Development Skill

Systematic approach to writing tests before implementation.

## When to Use

- Starting new features
- Fixing bugs (write failing test first)
- Refactoring existing code
- Designing APIs

## The TDD Cycle

```
┌─────────────────────────────────────────────────────┐
│                                                     │
│   RED ──────────► GREEN ──────────► REFACTOR       │
│    │                                    │          │
│    │  Write failing    Make it pass    │          │
│    │  test first       (minimal code)  │          │
│    │                                    │          │
│    └────────────────────────────────────┘          │
│                                                     │
└─────────────────────────────────────────────────────┘
```

### Phase 1: RED - Write a Failing Test

```typescript
// Start with the simplest test case
describe('Calculator', () => {
  it('should add two numbers', () => {
    const calc = new Calculator();
    expect(calc.add(2, 3)).toBe(5);
  });
});

// Run test - it should FAIL (Calculator doesn't exist)
// This confirms the test is actually testing something
```

### Phase 2: GREEN - Make It Pass

```typescript
// Write MINIMAL code to pass the test
class Calculator {
  add(a: number, b: number): number {
    return a + b;
  }
}

// Run test - it should PASS
// Don't add extra features yet!
```

### Phase 3: REFACTOR - Improve the Code

```typescript
// Now improve without changing behavior
// Tests ensure you don't break anything

class Calculator {
  add(...numbers: number[]): number {
    return numbers.reduce((sum, n) => sum + n, 0);
  }
}

// Run tests - should still PASS
```

## TDD Workflow Example

### Feature: User Registration

```typescript
// Step 1: RED - Define expected behavior
describe('UserService.register', () => {
  it('should create user with hashed password', async () => {
    const service = new UserService(mockDb, mockHasher);
    
    const user = await service.register({
      email: 'test@example.com',
      password: 'SecurePass123'
    });
    
    expect(user.id).toBeDefined();
    expect(user.email).toBe('test@example.com');
    expect(user.password).not.toBe('SecurePass123'); // Hashed
    expect(mockHasher.hash).toHaveBeenCalledWith('SecurePass123');
  });
  
  it('should reject duplicate email', async () => {
    const service = new UserService(mockDb, mockHasher);
    mockDb.findByEmail.mockResolvedValue({ id: '1', email: 'test@example.com' });
    
    await expect(service.register({
      email: 'test@example.com',
      password: 'password'
    })).rejects.toThrow('Email already registered');
  });
  
  it('should validate email format', async () => {
    const service = new UserService(mockDb, mockHasher);
    
    await expect(service.register({
      email: 'invalid-email',
      password: 'password'
    })).rejects.toThrow('Invalid email format');
  });
});

// Step 2: GREEN - Implement to pass tests
class UserService {
  constructor(private db: Database, private hasher: Hasher) {}
  
  async register(data: RegisterData): Promise<User> {
    // Validate email
    if (!this.isValidEmail(data.email)) {
      throw new Error('Invalid email format');
    }
    
    // Check duplicate
    const existing = await this.db.findByEmail(data.email);
    if (existing) {
      throw new Error('Email already registered');
    }
    
    // Hash password and create user
    const hashedPassword = await this.hasher.hash(data.password);
    return this.db.createUser({
      email: data.email,
      password: hashedPassword
    });
  }
  
  private isValidEmail(email: string): boolean {
    return /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(email);
  }
}

// Step 3: REFACTOR - Extract validation, improve structure
```

## Testing Anti-Patterns to Avoid

### ❌ Testing Implementation Details

```typescript
// ❌ BAD: Testing internal state
it('should set isLoading to true', () => {
  component.fetchData();
  expect(component.isLoading).toBe(true); // Implementation detail
});

// ✅ GOOD: Test observable behavior
it('should show loading spinner while fetching', async () => {
  render(<DataList />);
  fireEvent.click(screen.getByText('Load'));
  expect(screen.getByRole('progressbar')).toBeInTheDocument();
});
```

### ❌ Flaky Tests

```typescript
// ❌ BAD: Depends on timing
it('should update after delay', async () => {
  component.startTimer();
  await sleep(1000); // Flaky!
  expect(component.value).toBe('updated');
});

// ✅ GOOD: Wait for condition
it('should update after delay', async () => {
  component.startTimer();
  await waitFor(() => expect(component.value).toBe('updated'));
});
```

### ❌ Test Interdependence

```typescript
// ❌ BAD: Tests depend on order
let user: User;

it('should create user', () => {
  user = createUser(); // Sets state for next test
});

it('should delete user', () => {
  deleteUser(user.id); // Depends on previous test
});

// ✅ GOOD: Each test is independent
it('should delete user', () => {
  const user = createUser(); // Own setup
  deleteUser(user.id);
  expect(getUser(user.id)).toBeNull();
});
```

### ❌ Over-Mocking

```typescript
// ❌ BAD: Mock everything
it('should process order', () => {
  const mockValidator = jest.fn().mockReturnValue(true);
  const mockCalculator = jest.fn().mockReturnValue(100);
  const mockNotifier = jest.fn();
  // ... testing mocks, not real behavior
});

// ✅ GOOD: Use real implementations where practical
it('should process order', () => {
  const order = new Order(realValidator, realCalculator, mockNotifier);
  order.process(items);
  expect(mockNotifier.send).toHaveBeenCalledWith(expect.objectContaining({
    total: 100
  }));
});
```

## Test Structure: AAA Pattern

```typescript
it('should calculate discount for premium users', () => {
  // Arrange - Set up test data
  const user = createUser({ tier: 'premium' });
  const cart = createCart({ items: [{ price: 100 }] });
  const discountService = new DiscountService();
  
  // Act - Execute the behavior
  const discount = discountService.calculate(user, cart);
  
  // Assert - Verify the result
  expect(discount).toBe(20); // 20% premium discount
});
```

## TDD Checklist

- [ ] Write test BEFORE implementation
- [ ] Test fails for the right reason
- [ ] Write minimal code to pass
- [ ] Refactor only when tests pass
- [ ] Each test is independent
- [ ] Tests are readable (describe intent)
- [ ] No testing of implementation details
- [ ] Edge cases covered
