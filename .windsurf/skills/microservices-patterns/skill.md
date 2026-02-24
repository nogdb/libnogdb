---
name: microservices-patterns
description: Patterns for building and operating microservices architectures
---

# Microservices Patterns

Design patterns for distributed microservices systems.

## When to Use

- Scaling specific components independently
- Multiple teams working on different services
- Different technology requirements per service
- Complex domain with bounded contexts

## Service Communication

### Synchronous (REST/gRPC)

```typescript
// REST client with retry
class OrderServiceClient {
  private readonly baseUrl = process.env.ORDER_SERVICE_URL;
  
  async getOrder(id: string): Promise<Order> {
    const response = await fetch(`${this.baseUrl}/orders/${id}`, {
      headers: { 'Authorization': `Bearer ${this.token}` }
    });
    
    if (!response.ok) {
      throw new ServiceError('order-service', response.status);
    }
    
    return response.json();
  }
}

// gRPC client
const orderClient = new OrderServiceClient(
  'order-service:50051',
  grpc.credentials.createInsecure()
);

const order = await orderClient.getOrder({ id: 'order_123' });
```

### Asynchronous (Message Queue)

```typescript
// Publisher
class OrderService {
  async createOrder(data: CreateOrderInput) {
    const order = await this.repo.create(data);
    
    // Publish event for other services
    await this.nats.publish('orders.created', {
      orderId: order.id,
      userId: data.userId,
      items: data.items,
      timestamp: new Date().toISOString()
    });
    
    return order;
  }
}

// Subscriber
class InventoryService {
  async start() {
    await this.nats.subscribe('orders.created', async (msg) => {
      const event = JSON.parse(msg.data);
      
      for (const item of event.items) {
        await this.reserveStock(item.productId, item.quantity);
      }
      
      msg.ack();
    });
  }
}
```

## Service Discovery

### Client-Side Discovery

```typescript
// Service registry
class ServiceRegistry {
  private services = new Map<string, ServiceInstance[]>();
  
  register(name: string, instance: ServiceInstance) {
    const instances = this.services.get(name) || [];
    instances.push(instance);
    this.services.set(name, instances);
  }
  
  discover(name: string): ServiceInstance {
    const instances = this.services.get(name) || [];
    // Round-robin load balancing
    return instances[this.counter++ % instances.length];
  }
}

// Usage
const orderService = registry.discover('order-service');
await fetch(`http://${orderService.host}:${orderService.port}/orders`);
```

### Server-Side Discovery (Kubernetes)

```yaml
# Kubernetes Service
apiVersion: v1
kind: Service
metadata:
  name: order-service
spec:
  selector:
    app: order-service
  ports:
    - port: 80
      targetPort: 3000
---
# Access via DNS: http://order-service/orders
```

## API Gateway

```typescript
// Gateway routing
const routes = {
  '/api/users/*': 'http://user-service:3000',
  '/api/orders/*': 'http://order-service:3000',
  '/api/products/*': 'http://product-service:3000',
};

app.use('/api/*', async (req, res) => {
  const target = findRoute(req.path);
  
  // Add auth header
  const token = await validateAndRefreshToken(req);
  
  // Proxy request
  const response = await fetch(target + req.path, {
    method: req.method,
    headers: { ...req.headers, 'Authorization': `Bearer ${token}` },
    body: req.body
  });
  
  res.status(response.status).json(await response.json());
});
```

## Circuit Breaker

```typescript
enum CircuitState {
  CLOSED = 'closed',
  OPEN = 'open',
  HALF_OPEN = 'half-open'
}

class CircuitBreaker {
  private state = CircuitState.CLOSED;
  private failures = 0;
  private lastFailure?: Date;
  
  constructor(
    private readonly threshold = 5,
    private readonly timeout = 30000
  ) {}
  
  async call<T>(fn: () => Promise<T>): Promise<T> {
    if (this.state === CircuitState.OPEN) {
      if (this.shouldReset()) {
        this.state = CircuitState.HALF_OPEN;
      } else {
        throw new Error('Circuit is open');
      }
    }
    
    try {
      const result = await fn();
      this.onSuccess();
      return result;
    } catch (error) {
      this.onFailure();
      throw error;
    }
  }
  
  private onSuccess() {
    this.failures = 0;
    this.state = CircuitState.CLOSED;
  }
  
  private onFailure() {
    this.failures++;
    this.lastFailure = new Date();
    if (this.failures >= this.threshold) {
      this.state = CircuitState.OPEN;
    }
  }
  
  private shouldReset(): boolean {
    return Date.now() - this.lastFailure!.getTime() > this.timeout;
  }
}
```

## Saga Pattern

```typescript
// Orchestration-based saga
class CreateOrderSaga {
  async execute(input: CreateOrderInput) {
    const sagaLog: SagaStep[] = [];
    
    try {
      // Step 1: Reserve inventory
      const reservation = await this.inventoryService.reserve(input.items);
      sagaLog.push({ step: 'reserve', data: reservation });
      
      // Step 2: Process payment
      const payment = await this.paymentService.charge(input.total);
      sagaLog.push({ step: 'payment', data: payment });
      
      // Step 3: Create order
      const order = await this.orderService.create(input);
      sagaLog.push({ step: 'order', data: order });
      
      return order;
      
    } catch (error) {
      // Compensate in reverse order
      await this.compensate(sagaLog);
      throw error;
    }
  }
  
  private async compensate(log: SagaStep[]) {
    for (const step of log.reverse()) {
      switch (step.step) {
        case 'reserve':
          await this.inventoryService.release(step.data.reservationId);
          break;
        case 'payment':
          await this.paymentService.refund(step.data.paymentId);
          break;
        case 'order':
          await this.orderService.cancel(step.data.orderId);
          break;
      }
    }
  }
}
```

## Event Sourcing

```typescript
// Event store
interface DomainEvent {
  aggregateId: string;
  type: string;
  data: unknown;
  timestamp: Date;
  version: number;
}

class OrderAggregate {
  private events: DomainEvent[] = [];
  private state: OrderState = { status: 'draft', items: [] };
  
  // Apply events to rebuild state
  static fromEvents(events: DomainEvent[]): OrderAggregate {
    const order = new OrderAggregate();
    for (const event of events) {
      order.apply(event);
    }
    return order;
  }
  
  // Command creates event
  addItem(item: OrderItem) {
    this.applyAndRecord({
      type: 'ItemAdded',
      data: item
    });
  }
  
  private apply(event: DomainEvent) {
    switch (event.type) {
      case 'ItemAdded':
        this.state.items.push(event.data as OrderItem);
        break;
      case 'OrderSubmitted':
        this.state.status = 'submitted';
        break;
    }
  }
  
  private applyAndRecord(event: Partial<DomainEvent>) {
    const fullEvent = {
      ...event,
      aggregateId: this.id,
      timestamp: new Date(),
      version: this.events.length + 1
    } as DomainEvent;
    
    this.apply(fullEvent);
    this.events.push(fullEvent);
  }
}
```

## Health Checks

```typescript
// Health endpoint
app.get('/health', async (req, res) => {
  const checks = await Promise.all([
    checkDatabase(),
    checkRedis(),
    checkExternalService()
  ]);
  
  const healthy = checks.every(c => c.status === 'healthy');
  
  res.status(healthy ? 200 : 503).json({
    status: healthy ? 'healthy' : 'unhealthy',
    checks: checks.reduce((acc, c) => ({ ...acc, [c.name]: c }), {}),
    timestamp: new Date().toISOString()
  });
});

// Kubernetes probes
app.get('/health/live', (req, res) => res.status(200).send('OK'));
app.get('/health/ready', async (req, res) => {
  const ready = await checkDependencies();
  res.status(ready ? 200 : 503).send(ready ? 'OK' : 'Not Ready');
});
```

## Distributed Tracing

```typescript
// OpenTelemetry setup
import { trace, context } from '@opentelemetry/api';

const tracer = trace.getTracer('order-service');

async function createOrder(input: CreateOrderInput) {
  return tracer.startActiveSpan('createOrder', async (span) => {
    try {
      span.setAttribute('user.id', input.userId);
      span.setAttribute('items.count', input.items.length);
      
      // Child span for database
      const order = await tracer.startActiveSpan('db.insert', async (dbSpan) => {
        const result = await db.orders.create(input);
        dbSpan.end();
        return result;
      });
      
      span.setAttribute('order.id', order.id);
      return order;
      
    } catch (error) {
      span.recordException(error);
      span.setStatus({ code: SpanStatusCode.ERROR });
      throw error;
    } finally {
      span.end();
    }
  });
}
```

## Best Practices

1. **Design for failure** - Assume services will fail
2. **Use async communication** - Reduce coupling
3. **Implement idempotency** - Handle duplicate messages
4. **Centralize logging** - Aggregate logs from all services
5. **Use correlation IDs** - Trace requests across services
6. **Define clear contracts** - API versioning and schemas
7. **Automate deployment** - CI/CD for each service

## Checklist

- [ ] Service boundaries defined (bounded contexts)
- [ ] Communication patterns chosen (sync/async)
- [ ] Service discovery configured
- [ ] Circuit breakers implemented
- [ ] Health checks exposed
- [ ] Distributed tracing enabled
- [ ] Centralized logging configured
- [ ] API gateway in place
- [ ] Deployment automation ready
