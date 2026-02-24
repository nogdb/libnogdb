---
name: architecture-patterns
description: Software architecture patterns for scalable applications
---

# Architecture Patterns

Common architectural patterns for building maintainable systems.

## When to Use

- Designing new systems
- Refactoring existing architecture
- Scaling applications
- Evaluating trade-offs

## Layered Architecture

```
┌─────────────────────────────────────┐
│         Presentation Layer          │  UI, API Controllers
├─────────────────────────────────────┤
│          Application Layer          │  Use Cases, Services
├─────────────────────────────────────┤
│           Domain Layer              │  Business Logic, Entities
├─────────────────────────────────────┤
│        Infrastructure Layer         │  Database, External APIs
└─────────────────────────────────────┘
```

```typescript
// Presentation Layer
class UserController {
  constructor(private userService: UserService) {}
  
  async getUser(req: Request, res: Response) {
    const user = await this.userService.findById(req.params.id);
    res.json(user);
  }
}

// Application Layer
class UserService {
  constructor(private userRepo: UserRepository) {}
  
  async findById(id: string): Promise<User> {
    return this.userRepo.findById(id);
  }
}

// Domain Layer
class User {
  constructor(
    public readonly id: string,
    public email: string,
    public name: string
  ) {}
  
  updateEmail(newEmail: string) {
    if (!this.isValidEmail(newEmail)) {
      throw new Error('Invalid email');
    }
    this.email = newEmail;
  }
}

// Infrastructure Layer
class PostgresUserRepository implements UserRepository {
  async findById(id: string): Promise<User | null> {
    const row = await db.query('SELECT * FROM users WHERE id = $1', [id]);
    return row ? this.toDomain(row) : null;
  }
}
```

## Clean Architecture

```
┌────────────────────────────────────────────┐
│              Frameworks & Drivers          │
│  ┌──────────────────────────────────────┐  │
│  │         Interface Adapters           │  │
│  │  ┌────────────────────────────────┐  │  │
│  │  │       Application Layer        │  │  │
│  │  │  ┌──────────────────────────┐  │  │  │
│  │  │  │     Domain/Entities      │  │  │  │
│  │  │  └──────────────────────────┘  │  │  │
│  │  └────────────────────────────────┘  │  │
│  └──────────────────────────────────────┘  │
└────────────────────────────────────────────┘
```

**Dependency Rule**: Dependencies point inward. Inner layers know nothing about outer layers.

```typescript
// Domain (innermost - no dependencies)
interface OrderRepository {
  save(order: Order): Promise<void>;
  findById(id: string): Promise<Order | null>;
}

class Order {
  constructor(
    public readonly id: string,
    public items: OrderItem[],
    public status: OrderStatus
  ) {}
  
  calculateTotal(): Money {
    return this.items.reduce((sum, item) => sum.add(item.subtotal), Money.zero());
  }
}

// Application Layer (depends on Domain)
class CreateOrderUseCase {
  constructor(
    private orderRepo: OrderRepository,
    private paymentService: PaymentService
  ) {}
  
  async execute(input: CreateOrderInput): Promise<Order> {
    const order = new Order(generateId(), input.items, 'pending');
    await this.paymentService.charge(order.calculateTotal());
    await this.orderRepo.save(order);
    return order;
  }
}

// Infrastructure (outermost - implements interfaces)
class PostgresOrderRepository implements OrderRepository {
  async save(order: Order): Promise<void> {
    await db.orders.upsert(this.toRow(order));
  }
}
```

## Hexagonal Architecture (Ports & Adapters)

```
                    ┌─────────────┐
                    │   Primary   │
                    │  Adapters   │
                    │ (Driving)   │
                    └──────┬──────┘
                           │
┌──────────┐    ┌──────────▼──────────┐    ┌──────────┐
│ REST API │───►│                     │◄───│ Database │
└──────────┘    │    Application      │    └──────────┘
                │       Core          │
┌──────────┐    │                     │    ┌──────────┐
│   CLI    │───►│   (Domain Logic)    │◄───│  Email   │
└──────────┘    │                     │    └──────────┘
                └──────────▲──────────┘
                           │
                    ┌──────┴──────┐
                    │  Secondary  │
                    │  Adapters   │
                    │  (Driven)   │
                    └─────────────┘
```

```typescript
// Port (interface defined by core)
interface NotificationPort {
  send(userId: string, message: string): Promise<void>;
}

// Core uses port
class OrderService {
  constructor(private notifications: NotificationPort) {}
  
  async completeOrder(orderId: string) {
    // ... business logic
    await this.notifications.send(order.userId, 'Order completed!');
  }
}

// Adapter implements port
class EmailNotificationAdapter implements NotificationPort {
  async send(userId: string, message: string) {
    const user = await this.userRepo.findById(userId);
    await this.emailClient.send(user.email, message);
  }
}

class SlackNotificationAdapter implements NotificationPort {
  async send(userId: string, message: string) {
    await this.slackClient.postMessage(userId, message);
  }
}
```

## Event-Driven Architecture

```
┌─────────┐     ┌─────────────┐     ┌─────────┐
│ Service │────►│ Event Bus   │────►│ Service │
│    A    │     │ (NATS/Kafka)│     │    B    │
└─────────┘     └─────────────┘     └─────────┘
                       │
                       ▼
                ┌─────────────┐
                │  Service C  │
                └─────────────┘
```

```typescript
// Event definition
interface OrderCreatedEvent {
  type: 'order.created';
  data: {
    orderId: string;
    userId: string;
    items: OrderItem[];
    total: number;
  };
  metadata: {
    timestamp: string;
    correlationId: string;
  };
}

// Publisher
class OrderService {
  constructor(private eventBus: EventBus) {}
  
  async createOrder(input: CreateOrderInput) {
    const order = await this.orderRepo.create(input);
    
    await this.eventBus.publish({
      type: 'order.created',
      data: { orderId: order.id, userId: input.userId, ... },
      metadata: { timestamp: new Date().toISOString() }
    });
    
    return order;
  }
}

// Subscriber
class InventoryService {
  @Subscribe('order.created')
  async handleOrderCreated(event: OrderCreatedEvent) {
    for (const item of event.data.items) {
      await this.reserveInventory(item.productId, item.quantity);
    }
  }
}

class NotificationService {
  @Subscribe('order.created')
  async handleOrderCreated(event: OrderCreatedEvent) {
    await this.sendOrderConfirmation(event.data.userId, event.data.orderId);
  }
}
```

## CQRS (Command Query Responsibility Segregation)

```
┌─────────────────────────────────────────────────────┐
│                      Client                         │
└───────────────┬─────────────────────┬───────────────┘
                │                     │
         Commands                  Queries
                │                     │
                ▼                     ▼
        ┌───────────────┐     ┌───────────────┐
        │ Command Model │     │  Query Model  │
        │   (Write)     │     │   (Read)      │
        └───────┬───────┘     └───────┬───────┘
                │                     │
                ▼                     ▼
        ┌───────────────┐     ┌───────────────┐
        │   Write DB    │────►│   Read DB     │
        │ (Normalized)  │     │(Denormalized) │
        └───────────────┘     └───────────────┘
```

```typescript
// Command
class CreateOrderCommand {
  constructor(
    public readonly userId: string,
    public readonly items: OrderItem[]
  ) {}
}

class CreateOrderHandler {
  async execute(command: CreateOrderCommand) {
    const order = new Order(command.userId, command.items);
    await this.writeDb.orders.insert(order);
    await this.eventBus.publish(new OrderCreatedEvent(order));
  }
}

// Query
class GetOrdersQuery {
  constructor(
    public readonly userId: string,
    public readonly status?: string
  ) {}
}

class GetOrdersHandler {
  async execute(query: GetOrdersQuery) {
    // Read from denormalized read model
    return this.readDb.orderViews.find({
      userId: query.userId,
      ...(query.status && { status: query.status })
    });
  }
}
```

## Microservices Patterns

### API Gateway

```
┌─────────┐     ┌─────────────┐     ┌─────────────┐
│ Client  │────►│ API Gateway │────►│ User Service│
└─────────┘     │             │     └─────────────┘
                │  - Routing  │     ┌─────────────┐
                │  - Auth     │────►│Order Service│
                │  - Rate Lim │     └─────────────┘
                └─────────────┘
```

### Service Discovery

```typescript
// Register service on startup
await serviceRegistry.register({
  name: 'order-service',
  host: process.env.HOST,
  port: process.env.PORT,
  healthCheck: '/health'
});

// Discover service
const orderService = await serviceRegistry.discover('order-service');
await fetch(`http://${orderService.host}:${orderService.port}/orders`);
```

### Circuit Breaker

```typescript
const breaker = new CircuitBreaker(async () => {
  return await externalService.call();
}, {
  timeout: 3000,
  errorThreshold: 50,
  resetTimeout: 30000
});

const result = await breaker.fire();
```

## Choosing Patterns

| Pattern | Use When |
|---------|----------|
| Layered | Simple CRUD apps, small teams |
| Clean/Hexagonal | Complex domain logic, testability |
| Event-Driven | Async workflows, loose coupling |
| CQRS | Different read/write patterns |
| Microservices | Large teams, independent scaling |

## Best Practices

1. **Start simple** - Don't over-engineer
2. **Separate concerns** - Single responsibility
3. **Depend on abstractions** - Not implementations
4. **Design for change** - Expect requirements to evolve
5. **Document decisions** - ADRs for major choices
