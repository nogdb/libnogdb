---
description: Generate Internal Module Design with interfaces and dependencies (Step 13)
arguments:
  send: true
---

# Internal Module Design Generator

Use this workflow after completing Step 12 (Architecture Summary).

## Usage

```
/aspec-13-module-design [input_files]
```

**Examples:**
- `/aspec-13-module-design docs/05-data-structure.md docs/07-function-list.md docs/12-architecture-summary.md`
- `/aspec-13-module-design` (will look for files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths, read those files as input documents.
If empty, look for `docs/05-data-structure.md`, `docs/07-function-list.md`, and `docs/12-architecture-summary.md`.

## Prerequisites

Ensure you have completed:
- Data Structure (Step 5)
- Function List (Step 7)
- Architecture Summary (Step 12)
- **For microservices:** Service Registry (Step 12.1) - `/aspec-12.1-service-registry`

## Technology Context

Before generating module design, specify:
- **Architecture Type:** `monolith` | `modular-monolith` | `microservices`
- **Service Name:** (for microservices) e.g., `user-service`, `order-service`
- **Technology Stack:** `typescript` | `golang` | `ruby-rails` | `javascript` | `flutter`

### Single Service Design
```
/aspec-13-module-design --arch=microservices --service=user-service --stack=golang docs/12.1-service-registry.md
/aspec-13-module-design --arch=modular-monolith --stack=ruby-rails
/aspec-13-module-design --arch=monolith --stack=flutter
```

### Multiple Services Design (Batch)
For designing multiple services at once, use comma-separated service names with their stacks:
```
/aspec-13-module-design --arch=microservices \
  --services=user-service:golang,order-service:typescript,payment-service:ruby-rails \
  docs/12.1-service-registry.md
```

Or design all services from the service registry:
```
/aspec-13-module-design --arch=microservices --all-services docs/12.1-service-registry.md
```

**Output for multiple services:**
```
docs/
├── 13-module-design-user-service.md
├── 13-module-design-order-service.md
├── 13-module-design-payment-service.md
└── 13-module-design-summary.md  # cross-service overview
```

### For microservices with existing services:
When designing a new service that integrates with existing services, include the service registry:
```
/aspec-13-module-design --arch=microservices --service=notification-service --stack=golang docs/12.1-service-registry.md
```

The service registry provides:
- Existing service tech stacks and APIs
- Service dependencies and boundaries
- Event contracts (published/consumed)
- Shared libraries and contracts

## Prompt

```
You are a software architect. Design the Internal Module structure with Abstract Interfaces.

**Input Documents:**
- Architecture Summary (Step 12)
- Function List (Step 7)
- Data Structure (Step 5)
- Service Registry (Step 12.1) - **required for microservices**

**Output Requirements:**

1. **Module Breakdown**

   Choose the appropriate structure based on your technology stack:

   **TypeScript/NestJS:**
   ```
   src/
   ├── modules/
   │   ├── auth/
   │   │   ├── auth.module.ts
   │   │   ├── auth.service.ts
   │   │   ├── auth.controller.ts
   │   │   └── interfaces/
   │   ├── user/
   │   │   ├── user.module.ts
   │   │   ├── user.service.ts
   │   │   ├── user.repository.ts
   │   │   └── interfaces/
   │   └── [domain]/
   ├── shared/
   │   ├── interfaces/
   │   ├── utils/
   │   └── middleware/
   └── infrastructure/
       ├── database/
       ├── cache/
       └── messaging/
   ```

   **Go (Clean Architecture):**
   ```
   internal/
   ├── domain/
   │   ├── user/
   │   │   ├── entity.go
   │   │   ├── repository.go      # interface
   │   │   └── service.go         # interface
   │   └── order/
   ├── application/
   │   └── usecase/
   ├── infrastructure/
   │   ├── persistence/
   │   └── external/
   └── interfaces/
       ├── http/
       └── grpc/
   ```

   **Ruby on Rails (Modular Monolith):**
   ```
   app/
   ├── domains/
   │   ├── auth/
   │   │   ├── services/
   │   │   ├── repositories/
   │   │   └── contracts/         # interfaces
   │   └── user/
   ├── shared/
   │   └── concerns/
   └── infrastructure/
       └── adapters/
   ```

   **JavaScript/Node.js:**
   ```
   src/
   ├── modules/
   │   ├── auth/
   │   │   ├── auth.service.js
   │   │   ├── auth.controller.js
   │   │   └── interfaces/        # JSDoc definitions
   │   └── user/
   ├── shared/
   └── infrastructure/
   ```

   **Flutter (Clean Architecture):**
   ```
   lib/
   ├── core/
   │   ├── error/
   │   │   ├── failures.dart
   │   │   └── exceptions.dart
   │   ├── usecases/
   │   │   └── usecase.dart       # base usecase interface
   │   └── utils/
   ├── features/
   │   ├── auth/
   │   │   ├── domain/
   │   │   │   ├── entities/
   │   │   │   ├── repositories/  # abstract interfaces
   │   │   │   └── usecases/
   │   │   ├── data/
   │   │   │   ├── datasources/
   │   │   │   ├── models/
   │   │   │   └── repositories/  # implementations
   │   │   └── presentation/
   │   │       ├── bloc/
   │   │       ├── pages/
   │   │       └── widgets/
   │   └── user/
   │       ├── domain/
   │       ├── data/
   │       └── presentation/
   └── injection_container.dart   # dependency injection
   ```

2. **Module Interfaces**
   For each module, define interfaces using language-appropriate patterns:

   **TypeScript:**
   ```typescript
   // IUserService interface
   interface IUserService {
     createUser(data: CreateUserDTO): Promise<User>;
     getUserById(id: string): Promise<User | null>;
     updateUser(id: string, data: UpdateUserDTO): Promise<User>;
     deleteUser(id: string): Promise<void>;
     listUsers(filter: UserFilter, pagination: Pagination): Promise<PaginatedResult<User>>;
   }
   
   // IUserRepository interface
   interface IUserRepository {
     save(user: User): Promise<User>;
     findById(id: string): Promise<User | null>;
     findByEmail(email: string): Promise<User | null>;
     update(id: string, data: Partial<User>): Promise<User>;
     delete(id: string): Promise<void>;
     findMany(filter: UserFilter, pagination: Pagination): Promise<User[]>;
     count(filter: UserFilter): Promise<number>;
   }
   ```

   **Go:**
   ```go
   // UserService interface
   type UserService interface {
       CreateUser(ctx context.Context, data CreateUserDTO) (*User, error)
       GetUserByID(ctx context.Context, id string) (*User, error)
       UpdateUser(ctx context.Context, id string, data UpdateUserDTO) (*User, error)
       DeleteUser(ctx context.Context, id string) error
       ListUsers(ctx context.Context, filter UserFilter, pagination Pagination) (*PaginatedResult[User], error)
   }

   // UserRepository interface
   type UserRepository interface {
       Save(ctx context.Context, user *User) error
       FindByID(ctx context.Context, id string) (*User, error)
       FindByEmail(ctx context.Context, email string) (*User, error)
       Update(ctx context.Context, id string, data map[string]interface{}) (*User, error)
       Delete(ctx context.Context, id string) error
       FindMany(ctx context.Context, filter UserFilter, pagination Pagination) ([]*User, error)
       Count(ctx context.Context, filter UserFilter) (int64, error)
   }
   ```

   **Ruby on Rails (using Sorbet or contracts):**
   ```ruby
   # Using Sorbet for type safety
   class UserService
     extend T::Sig
     extend T::Helpers
     abstract!

     sig { abstract.params(data: CreateUserDTO).returns(User) }
     def create_user(data); end

     sig { abstract.params(id: String).returns(T.nilable(User)) }
     def get_user_by_id(id); end

     sig { abstract.params(id: String, data: UpdateUserDTO).returns(User) }
     def update_user(id, data); end

     sig { abstract.params(id: String).void }
     def delete_user(id); end
   end

   # Or using contracts pattern
   module UserServiceContract
     def create_user(data:) = raise NotImplementedError
     def get_user_by_id(id:) = raise NotImplementedError
     def update_user(id:, data:) = raise NotImplementedError
     def delete_user(id:) = raise NotImplementedError
   end
   ```

   **JavaScript (JSDoc):**
   ```javascript
   /**
    * @typedef {Object} IUserService
    * @property {function(CreateUserDTO): Promise<User>} createUser
    * @property {function(string): Promise<User|null>} getUserById
    * @property {function(string, UpdateUserDTO): Promise<User>} updateUser
    * @property {function(string): Promise<void>} deleteUser
    */

   /**
    * @typedef {Object} IUserRepository
    * @property {function(User): Promise<User>} save
    * @property {function(string): Promise<User|null>} findById
    * @property {function(string): Promise<User|null>} findByEmail
    */
   ```

   **Flutter/Dart:**
   ```dart
   // Abstract repository interface (domain layer)
   abstract class UserRepository {
     Future<Either<Failure, User>> createUser(CreateUserParams params);
     Future<Either<Failure, User>> getUserById(String id);
     Future<Either<Failure, User>> updateUser(String id, UpdateUserParams params);
     Future<Either<Failure, void>> deleteUser(String id);
     Future<Either<Failure, PaginatedResult<User>>> listUsers(
       UserFilter filter,
       Pagination pagination,
     );
   }

   // UseCase interface
   abstract class UseCase<Type, Params> {
     Future<Either<Failure, Type>> call(Params params);
   }

   // Concrete UseCase
   class GetUserById implements UseCase<User, String> {
     final UserRepository repository;

     GetUserById(this.repository);

     @override
     Future<Either<Failure, User>> call(String id) {
       return repository.getUserById(id);
     }
   }

   // DataSource interface (data layer)
   abstract class UserRemoteDataSource {
     Future<UserModel> createUser(CreateUserParams params);
     Future<UserModel> getUserById(String id);
     Future<UserModel> updateUser(String id, UpdateUserParams params);
     Future<void> deleteUser(String id);
   }

   abstract class UserLocalDataSource {
     Future<UserModel?> getCachedUser(String id);
     Future<void> cacheUser(UserModel user);
   }
   ```

3. **Module Dependencies**
   ```mermaid
   graph LR
       Auth --> User
       Order --> User
       Order --> Product
       Order --> Payment
       Payment --> External[Payment Gateway]
   ```

4. **Shared Interfaces**
   ```typescript
   // Common interfaces used across modules
   interface Pagination {
     page: number;
     limit: number;
     sortBy?: string;
     sortOrder?: 'asc' | 'desc';
   }
   
   interface PaginatedResult<T> {
     data: T[];
     total: number;
     page: number;
     limit: number;
     totalPages: number;
   }
   ```

5. **Connector/Adapter Interfaces**

   **TypeScript:**
   ```typescript
   // External service adapters
   interface IEmailService {
     sendEmail(to: string, subject: string, body: string): Promise<void>;
     sendTemplatedEmail(to: string, template: string, data: object): Promise<void>;
   }
   
   interface IPaymentGateway {
     createPayment(amount: number, currency: string): Promise<PaymentIntent>;
     confirmPayment(paymentId: string): Promise<PaymentResult>;
     refundPayment(paymentId: string, amount?: number): Promise<RefundResult>;
   }
   ```

   **Go:**
   ```go
   type EmailService interface {
       SendEmail(ctx context.Context, to, subject, body string) error
       SendTemplatedEmail(ctx context.Context, to, template string, data map[string]interface{}) error
   }

   type PaymentGateway interface {
       CreatePayment(ctx context.Context, amount float64, currency string) (*PaymentIntent, error)
       ConfirmPayment(ctx context.Context, paymentID string) (*PaymentResult, error)
       RefundPayment(ctx context.Context, paymentID string, amount *float64) (*RefundResult, error)
   }
   ```

   **Flutter/Dart:**
   ```dart
   // External service adapters
   abstract class EmailService {
     Future<Either<Failure, void>> sendEmail({
       required String to,
       required String subject,
       required String body,
     });
     Future<Either<Failure, void>> sendTemplatedEmail({
       required String to,
       required String template,
       required Map<String, dynamic> data,
     });
   }

   abstract class PaymentGateway {
     Future<Either<Failure, PaymentIntent>> createPayment({
       required double amount,
       required String currency,
     });
     Future<Either<Failure, PaymentResult>> confirmPayment(String paymentId);
     Future<Either<Failure, RefundResult>> refundPayment(
       String paymentId, {
       double? amount,
     });
   }
   ```

6. **Dependency Injection Setup**

   **TypeScript (NestJS):**
   - Use `@Injectable()` decorators and module providers
   - Interface-to-implementation bindings via custom providers

   **Go:**
   - Use constructor injection with wire or manual DI
   - Interface-to-implementation via struct composition

   **Ruby on Rails:**
   - Use dry-container or custom service locator
   - Interface-to-implementation via initializers

   **Flutter:**
   ```dart
   // injection_container.dart
   final sl = GetIt.instance;

   Future<void> init() async {
     // Features - User
     // Bloc
     sl.registerFactory(() => UserBloc(getUser: sl(), createUser: sl()));

     // Use cases
     sl.registerLazySingleton(() => GetUserById(sl()));
     sl.registerLazySingleton(() => CreateUser(sl()));

     // Repository
     sl.registerLazySingleton<UserRepository>(
       () => UserRepositoryImpl(
         remoteDataSource: sl(),
         localDataSource: sl(),
         networkInfo: sl(),
       ),
     );

     // Data sources
     sl.registerLazySingleton<UserRemoteDataSource>(
       () => UserRemoteDataSourceImpl(client: sl()),
     );
     sl.registerLazySingleton<UserLocalDataSource>(
       () => UserLocalDataSourceImpl(sharedPreferences: sl()),
     );
   }
   ```

7. **Microservices Architecture (if applicable)**

   For microservices, also define:

   **Service Boundaries:**
   ```mermaid
   graph TB
       subgraph "User Service (Go)"
           US[User API]
           UR[User Repository]
       end
       subgraph "Order Service (TypeScript)"
           OS[Order API]
           OR[Order Repository]
       end
       subgraph "Mobile App (Flutter)"
           MA[Flutter App]
           MB[BLoC Layer]
       end
       MA --> US
       MA --> OS
       OS --> US
   ```

   **Inter-Service Communication:**
   - **Sync:** REST/gRPC contracts
   - **Async:** Event schemas (CloudEvents, Avro, Protobuf)

   **Service Contract Example (Protobuf):**
   ```protobuf
   service UserService {
     rpc GetUser(GetUserRequest) returns (User);
     rpc CreateUser(CreateUserRequest) returns (User);
     rpc UpdateUser(UpdateUserRequest) returns (User);
     rpc DeleteUser(DeleteUserRequest) returns (Empty);
   }
   ```

8. **Cross-Service Module Design Summary (for batch mode)**

   When designing multiple services, also generate a summary document:

   **Cross-Service Dependencies:**
   | From Service | To Service | Type | Contract |
   |--------------|------------|------|----------|
   | order-service | user-service | REST | GET /users/{id} |
   | order-service | payment-service | gRPC | PaymentService.CreatePayment |
   | payment-service | notification-service | Kafka | payment.completed |

   **Shared Contracts:**
   | Contract | Type | Used By | Location |
   |----------|------|---------|----------|
   | UserDTO | Protobuf | user-service, order-service | proto/user.proto |
   | PaymentEvent | CloudEvents | payment-service, order-service | schemas/payment-event.json |
   | CommonTypes | TypeScript | order-service, web-app | packages/common-types |

   **Event Flow Diagram:**
   ```mermaid
   sequenceDiagram
       participant OS as Order Service
       participant PS as Payment Service
       participant NS as Notification Service
       participant US as User Service
       
       OS->>US: GET /users/{id}
       US-->>OS: User data
       OS->>PS: CreatePayment (gRPC)
       PS-->>OS: PaymentIntent
       PS->>NS: payment.completed (Kafka)
       NS->>US: GET /users/{id}
       NS-->>NS: Send notification
   ```

   **Module Design Matrix:**
   | Service | Modules | Interfaces | Events Published | Events Consumed |
   |---------|---------|------------|------------------|-----------------|
   | user-service | auth, user, profile | IAuthService, IUserService | user.created, user.updated | - |
   | order-service | order, cart, checkout | IOrderService, ICartService | order.placed, order.completed | user.created |
   | payment-service | payment, refund | IPaymentService | payment.completed, payment.failed | order.placed |

**Guidelines:**
1. Follow SOLID principles (language-agnostic)
2. Design for testability (interfaces enable mocking in all languages)
3. Keep modules cohesive and loosely coupled
4. Define clear boundaries between modules/services
5. Use dependency injection (or equivalent pattern per language)
6. **For microservices:** Define API contracts before implementation
7. **For modular monolith:** Enforce module boundaries via package/namespace isolation
8. **For Flutter:** Follow clean architecture with clear separation between domain, data, and presentation layers
```

## Output

Save the generated document as `docs/13-module-design.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-14-api-design` - Generate API Design (Step 14)
