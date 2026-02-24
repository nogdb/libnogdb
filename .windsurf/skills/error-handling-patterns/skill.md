---
name: error-handling-patterns
description: Robust error handling patterns for TypeScript/JavaScript applications
---

# Error Handling Patterns

Comprehensive error handling strategies for production applications.

## When to Use

- Designing error handling strategy
- Implementing API error responses
- Building resilient applications
- Debugging production issues

## Error Types

### Custom Error Classes

```typescript
// errors/base.ts
export class AppError extends Error {
  constructor(
    message: string,
    public readonly code: string,
    public readonly statusCode: number = 500,
    public readonly isOperational: boolean = true,
    public readonly context?: Record<string, unknown>
  ) {
    super(message);
    this.name = this.constructor.name;
    Error.captureStackTrace(this, this.constructor);
  }

  toJSON() {
    return {
      error: {
        code: this.code,
        message: this.message,
        ...(process.env.NODE_ENV === 'development' && {
          stack: this.stack,
          context: this.context,
        }),
      },
    };
  }
}

// errors/http.ts
export class NotFoundError extends AppError {
  constructor(resource: string, id?: string) {
    super(
      id ? `${resource} with id ${id} not found` : `${resource} not found`,
      'NOT_FOUND',
      404
    );
  }
}

export class ValidationError extends AppError {
  constructor(message: string, public readonly fields?: Record<string, string>) {
    super(message, 'VALIDATION_ERROR', 400, true, { fields });
  }
}

export class UnauthorizedError extends AppError {
  constructor(message = 'Authentication required') {
    super(message, 'UNAUTHORIZED', 401);
  }
}

export class ForbiddenError extends AppError {
  constructor(message = 'Access denied') {
    super(message, 'FORBIDDEN', 403);
  }
}

export class ConflictError extends AppError {
  constructor(message: string) {
    super(message, 'CONFLICT', 409);
  }
}

export class RateLimitError extends AppError {
  constructor(retryAfter?: number) {
    super('Too many requests', 'RATE_LIMITED', 429, true, { retryAfter });
  }
}
```

## Error Handling Middleware

```typescript
// middleware/errorHandler.ts
import { Request, Response, NextFunction } from 'express';
import { AppError } from '../errors/base';
import { logger } from '../utils/logger';

export function errorHandler(
  error: Error,
  req: Request,
  res: Response,
  next: NextFunction
) {
  // Log error
  logger.error('Request error', {
    error: error.message,
    stack: error.stack,
    path: req.path,
    method: req.method,
    userId: req.user?.id,
  });

  // Handle known errors
  if (error instanceof AppError) {
    return res.status(error.statusCode).json(error.toJSON());
  }

  // Handle Zod validation errors
  if (error.name === 'ZodError') {
    return res.status(400).json({
      error: {
        code: 'VALIDATION_ERROR',
        message: 'Invalid request data',
        details: error.errors,
      },
    });
  }

  // Handle unknown errors
  const isProduction = process.env.NODE_ENV === 'production';
  res.status(500).json({
    error: {
      code: 'INTERNAL_ERROR',
      message: isProduction ? 'An unexpected error occurred' : error.message,
      ...(!isProduction && { stack: error.stack }),
    },
  });
}
```

## Result Pattern

```typescript
// utils/result.ts
type Success<T> = { success: true; data: T };
type Failure<E> = { success: false; error: E };
type Result<T, E = Error> = Success<T> | Failure<E>;

export function ok<T>(data: T): Success<T> {
  return { success: true, data };
}

export function err<E>(error: E): Failure<E> {
  return { success: false, error };
}

// Usage
async function getUser(id: string): Promise<Result<User, AppError>> {
  try {
    const user = await db.users.findUnique({ where: { id } });
    if (!user) {
      return err(new NotFoundError('User', id));
    }
    return ok(user);
  } catch (error) {
    return err(new AppError('Failed to fetch user', 'DB_ERROR'));
  }
}

// Consuming
const result = await getUser('123');
if (!result.success) {
  // Handle error
  console.error(result.error.message);
  return;
}
// Use result.data safely
console.log(result.data.name);
```

## Try-Catch Wrapper

```typescript
// utils/tryCatch.ts
type AsyncFn<T> = () => Promise<T>;

export async function tryCatch<T, E = Error>(
  fn: AsyncFn<T>
): Promise<[T, null] | [null, E]> {
  try {
    const result = await fn();
    return [result, null];
  } catch (error) {
    return [null, error as E];
  }
}

// Usage
const [user, error] = await tryCatch(() => userService.getById(id));
if (error) {
  return res.status(500).json({ error: 'Failed to fetch user' });
}
return res.json(user);
```

## Retry Pattern

```typescript
// utils/retry.ts
interface RetryOptions {
  maxAttempts: number;
  delayMs: number;
  backoff?: 'linear' | 'exponential';
  shouldRetry?: (error: Error) => boolean;
}

export async function retry<T>(
  fn: () => Promise<T>,
  options: RetryOptions
): Promise<T> {
  const { maxAttempts, delayMs, backoff = 'exponential', shouldRetry } = options;
  
  let lastError: Error;
  
  for (let attempt = 1; attempt <= maxAttempts; attempt++) {
    try {
      return await fn();
    } catch (error) {
      lastError = error as Error;
      
      // Check if we should retry
      if (shouldRetry && !shouldRetry(lastError)) {
        throw lastError;
      }
      
      // Don't wait after last attempt
      if (attempt < maxAttempts) {
        const delay = backoff === 'exponential' 
          ? delayMs * Math.pow(2, attempt - 1)
          : delayMs * attempt;
        
        await new Promise(resolve => setTimeout(resolve, delay));
      }
    }
  }
  
  throw lastError!;
}

// Usage
const data = await retry(
  () => fetch('https://api.example.com/data').then(r => r.json()),
  {
    maxAttempts: 3,
    delayMs: 1000,
    backoff: 'exponential',
    shouldRetry: (error) => error.message.includes('timeout'),
  }
);
```

## Circuit Breaker

```typescript
// utils/circuitBreaker.ts
type State = 'closed' | 'open' | 'half-open';

export class CircuitBreaker {
  private state: State = 'closed';
  private failures = 0;
  private lastFailure?: Date;
  
  constructor(
    private readonly threshold: number = 5,
    private readonly resetTimeout: number = 30000
  ) {}

  async execute<T>(fn: () => Promise<T>): Promise<T> {
    if (this.state === 'open') {
      if (this.shouldReset()) {
        this.state = 'half-open';
      } else {
        throw new Error('Circuit breaker is open');
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
    this.state = 'closed';
  }

  private onFailure() {
    this.failures++;
    this.lastFailure = new Date();
    
    if (this.failures >= this.threshold) {
      this.state = 'open';
    }
  }

  private shouldReset(): boolean {
    if (!this.lastFailure) return true;
    return Date.now() - this.lastFailure.getTime() > this.resetTimeout;
  }
}

// Usage
const apiBreaker = new CircuitBreaker(5, 30000);

async function callExternalAPI() {
  return apiBreaker.execute(() => 
    fetch('https://external-api.com/data')
  );
}
```

## Graceful Degradation

```typescript
// services/userService.ts
async function getUserWithFallback(id: string): Promise<User> {
  try {
    // Try primary source
    return await primaryDb.users.findUnique({ where: { id } });
  } catch (primaryError) {
    logger.warn('Primary DB failed, trying replica', { error: primaryError });
    
    try {
      // Fallback to replica
      return await replicaDb.users.findUnique({ where: { id } });
    } catch (replicaError) {
      logger.warn('Replica failed, trying cache', { error: replicaError });
      
      // Fallback to cache
      const cached = await cache.get(`user:${id}`);
      if (cached) {
        return JSON.parse(cached);
      }
      
      throw new AppError('User service unavailable', 'SERVICE_UNAVAILABLE', 503);
    }
  }
}
```

## Error Logging

```typescript
// utils/logger.ts
import pino from 'pino';

export const logger = pino({
  level: process.env.LOG_LEVEL || 'info',
  formatters: {
    level: (label) => ({ level: label }),
  },
  redact: ['password', 'token', 'authorization'],
});

// Structured error logging
function logError(error: Error, context?: Record<string, unknown>) {
  logger.error({
    err: {
      message: error.message,
      name: error.name,
      stack: error.stack,
      ...(error instanceof AppError && {
        code: error.code,
        statusCode: error.statusCode,
        isOperational: error.isOperational,
      }),
    },
    ...context,
  });
}
```

## Best Practices

1. **Use custom error classes** - Distinguish error types
2. **Always log errors** - With context for debugging
3. **Return consistent error format** - Same structure for all errors
4. **Don't expose internals** - Hide stack traces in production
5. **Handle async errors** - Use try-catch or error boundaries
6. **Implement retries** - For transient failures
7. **Use circuit breakers** - Prevent cascade failures

## Checklist

- [ ] Custom error classes defined
- [ ] Global error handler middleware
- [ ] Consistent error response format
- [ ] Errors logged with context
- [ ] Sensitive data redacted from logs
- [ ] Retry logic for external calls
- [ ] Graceful degradation implemented
