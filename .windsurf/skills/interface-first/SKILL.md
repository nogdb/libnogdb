---
name: interface-first
description: Create interfaces for licensed code integration without exposing implementation details to AI
---

# Interface-First Development Skill

This skill helps you safely integrate licensed code using the interface-first pattern.

## When to Use
- Integrating third-party licensed code
- Working with proprietary libraries
- Connecting to legacy systems with licensing restrictions

## Process

### Step 1: Analyze (Human Only)
Review the licensed code manually to understand:
- Public API surface
- Input/output types
- Error conditions
- Side effects

### Step 2: Create Interface
```typescript
// Example: Payment processor interface
interface PaymentProcessor {
  processPayment(amount: number, currency: string): Promise<PaymentResult>;
  refund(transactionId: string): Promise<RefundResult>;
  getStatus(transactionId: string): Promise<TransactionStatus>;
}

interface PaymentResult {
  success: boolean;
  transactionId: string;
  error?: string;
}
```

### Step 3: Implement Adapter (Human Only)
```typescript
// This file should NEVER be shared with AI
class StripeAdapter implements PaymentProcessor {
  // Implementation connecting to licensed Stripe SDK
}
```

### Step 4: Share Only Interface
- Share interface file with AI
- AI generates code using interface
- Never share adapter or licensed code

## Compliance Checklist
- [ ] Interface created without implementation details
- [ ] Adapter implemented by human only
- [ ] Licensed code never in AI context
- [ ] Performance overhead < 10%
