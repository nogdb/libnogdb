---
name: systematic-debugging
description: 4-phase root cause debugging process with defense-in-depth techniques
---

# Systematic Debugging Skill

A structured approach to finding and fixing bugs.

## When to Use

- Bug reports from users
- Test failures
- Unexpected behavior
- Performance issues

## The 4-Phase Process

```
┌──────────────────────────────────────────────────────────────┐
│                                                              │
│  Phase 1        Phase 2        Phase 3        Phase 4       │
│  REPRODUCE  ──► ISOLATE    ──► ROOT CAUSE ──► FIX & VERIFY  │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

## Phase 1: REPRODUCE

**Goal**: Consistently trigger the bug

```typescript
// Document reproduction steps
const bugReport = {
  description: "User sees blank page after login",
  steps: [
    "1. Go to /login",
    "2. Enter valid credentials",
    "3. Click 'Sign In'",
    "4. Observe blank page instead of dashboard"
  ],
  expected: "User sees dashboard",
  actual: "Blank white page",
  environment: {
    browser: "Chrome 120",
    os: "macOS 14",
    user: "test@example.com"
  },
  frequency: "100% reproducible"
};

// Create minimal reproduction
// Strip away unrelated code until bug remains
```

### Reproduction Checklist

- [ ] Can reproduce locally
- [ ] Documented exact steps
- [ ] Identified environment factors
- [ ] Created minimal test case

## Phase 2: ISOLATE

**Goal**: Narrow down the problem area

### Binary Search Debugging

```typescript
// If bug is in a process with many steps, use binary search
async function processOrder(order: Order) {
  const validated = await validateOrder(order);     // Step 1
  const priced = await calculatePricing(validated); // Step 2
  const stocked = await checkInventory(priced);     // Step 3
  const charged = await processPayment(stocked);    // Step 4  <-- Bug here?
  const shipped = await createShipment(charged);    // Step 5
  return shipped;
}

// Add checkpoint logging
async function processOrder(order: Order) {
  console.log('[DEBUG] Start processOrder', { orderId: order.id });
  
  const validated = await validateOrder(order);
  console.log('[DEBUG] After validate', { validated });
  
  const priced = await calculatePricing(validated);
  console.log('[DEBUG] After pricing', { priced });
  
  // Continue until you find where it breaks
}
```

### Git Bisect for Regressions

```bash
# Find the commit that introduced the bug
git bisect start
git bisect bad                 # Current commit is bad
git bisect good v1.2.0         # Last known good version

# Git will checkout commits for you to test
# Mark each as good or bad until culprit is found
git bisect good  # or
git bisect bad

# When done
git bisect reset
```

### Isolation Techniques

```typescript
// 1. Comment out code sections
async function handleRequest(req: Request) {
  // const auth = await authenticate(req);  // Temporarily disabled
  // const data = await fetchData(auth.userId);
  return { status: 'ok' }; // Minimal response
}

// 2. Replace with mock data
async function handleRequest(req: Request) {
  const auth = await authenticate(req);
  // const data = await fetchData(auth.userId);
  const data = { items: [] }; // Mock data
  return processData(data);
}

// 3. Add assertions
async function fetchData(userId: string) {
  console.assert(userId, 'userId is required');
  console.assert(typeof userId === 'string', 'userId must be string');
  // ...
}
```

## Phase 3: ROOT CAUSE

**Goal**: Understand WHY the bug occurs

### The 5 Whys

```
Problem: User sees blank page after login

Why 1: The dashboard component throws an error
Why 2: The user data is undefined
Why 3: The API returns null for user profile
Why 4: The database query returns no rows
Why 5: The user ID is not being passed correctly
        ↓
ROOT CAUSE: Session middleware sets userId on wrong property
```

### Common Root Causes

| Symptom | Common Causes |
|---------|---------------|
| Null/undefined errors | Missing null checks, async timing |
| Wrong data displayed | Stale cache, race conditions |
| Intermittent failures | Race conditions, external dependencies |
| Performance issues | N+1 queries, missing indexes, memory leaks |
| Security issues | Missing validation, improper auth checks |

### Debugging Tools

```typescript
// Console methods
console.log('Value:', value);
console.table(arrayOfObjects);
console.trace('Call stack');
console.time('operation');
// ... operation
console.timeEnd('operation');

// Debugger statement
function problematicFunction(data) {
  debugger; // Pauses execution in DevTools
  return process(data);
}

// Error boundaries (React)
class ErrorBoundary extends React.Component {
  componentDidCatch(error, errorInfo) {
    console.error('Caught error:', error);
    console.error('Component stack:', errorInfo.componentStack);
  }
}
```

## Phase 4: FIX & VERIFY

**Goal**: Fix correctly and prevent regression

### Write Test First

```typescript
// 1. Write failing test that reproduces the bug
it('should handle null user profile gracefully', async () => {
  // Arrange
  mockApi.getUserProfile.mockResolvedValue(null);
  
  // Act
  render(<Dashboard userId="123" />);
  
  // Assert - should show error, not crash
  await waitFor(() => {
    expect(screen.getByText('Profile not found')).toBeInTheDocument();
  });
});

// 2. Fix the code
function Dashboard({ userId }) {
  const { data: profile, error } = useProfile(userId);
  
  if (error || !profile) {
    return <ErrorMessage>Profile not found</ErrorMessage>;
  }
  
  return <ProfileView profile={profile} />;
}

// 3. Verify test passes
```

### Defense in Depth

```typescript
// Layer 1: Input validation
function processUser(userId: string) {
  if (!userId || typeof userId !== 'string') {
    throw new ValidationError('Invalid userId');
  }
  // ...
}

// Layer 2: Null checks
const profile = await getProfile(userId);
if (!profile) {
  return { error: 'Profile not found' };
}

// Layer 3: Error boundaries
try {
  return await riskyOperation();
} catch (error) {
  logger.error('Operation failed', { error, userId });
  return { error: 'Operation failed' };
}

// Layer 4: Monitoring
metrics.increment('profile.not_found', { userId });
```

### Fix Verification Checklist

- [ ] Wrote test that reproduces bug
- [ ] Test fails before fix
- [ ] Test passes after fix
- [ ] No other tests broken
- [ ] Fix addresses root cause (not symptom)
- [ ] Added defensive code to prevent recurrence
- [ ] Documented the fix in commit message

## Debugging Checklist

- [ ] Can reproduce consistently
- [ ] Isolated to specific component/function
- [ ] Identified root cause (not just symptom)
- [ ] Wrote regression test
- [ ] Fix is minimal and targeted
- [ ] Verified fix doesn't break other things
- [ ] Added logging/monitoring for future
