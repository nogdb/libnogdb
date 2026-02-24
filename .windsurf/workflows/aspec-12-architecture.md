---
description: Generate Architecture Summary with diagrams and tech stack (Step 12)
arguments:
  send: true
---

# Architecture Summary Generator

Use this workflow after completing Step 11 (Persistence Design).

## Usage

```
/aspec-12-architecture [input_files]
```

**Examples:**
- `/aspec-12-architecture docs/`
- `/aspec-12-architecture` (will look for all files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths or a directory, read those files as input documents.
If empty, look for all `docs/01-*.md` through `docs/11-*.md` files.

## Prerequisites

Ensure you have completed:
- All documents from Steps 1-11

## Architecture Context

Before generating architecture summary, specify:
- **Architecture Type:** `monolith` | `modular-monolith` | `microservices`
- **Technology Stacks:** (for microservices) list of technologies used across services

Example:
```
/aspec-12-architecture --arch=microservices docs/
/aspec-12-architecture --arch=modular-monolith docs/
/aspec-12-architecture --arch=monolith docs/
```

**For microservices with existing services:**
If you have existing services, first run `/service-extract` on each service to gather their architecture details, then include those summaries:
```
/aspec-12-architecture --arch=microservices docs/ \
  user-service/docs/user-service-service-summary.md \
  order-service/docs/order-service-service-summary.md
```

## Architecture Context

Before generating architecture summary, specify:
- **Architecture Type:** `monolith` | `modular-monolith` | `microservices`
- **Technology Stacks:** (for microservices) list of technologies used across services

Example:
```
/aspec-12-architecture --arch=microservices docs/
/aspec-12-architecture --arch=modular-monolith docs/
/aspec-12-architecture --arch=monolith docs/
```

**For microservices with existing services:**
If you have existing services, first run `/service-extract` on each service to gather their architecture details, then include those summaries:
```
/aspec-12-architecture --arch=microservices docs/ \
  user-service/docs/user-service-service-summary.md \
  order-service/docs/order-service-service-summary.md
```

## Prompt

```
You are a solutions architect. Create an Architecture Summary that ties together all design decisions.

**Input Documents:**
- All documents from Steps 1-11
- Service summaries (for microservices with existing services)

**Architecture Type:** [monolith | modular-monolith | microservices]

---

## Output Requirements by Architecture Type

### For Monolith Architecture

1. **Architecture Overview Diagram**
   Generate a Mermaid diagram based on the actual system components identified from the input documents. Include:
   - Client layer (web, mobile, or other clients as described in the spec)
   - Application layer (the monolith application with its actual name)
   - Data layer (databases, caches as defined in the persistence design)
   - Any external integrations mentioned in the spec

2. **Layer Structure**
   Document each layer based on the actual technology stack from the input documents:
   | Layer | Responsibility | Technology |
   |-------|---------------|------------|
   | [Layer name from spec] | [Actual responsibility] | [Actual technology chosen] |
   *(Add one row per layer identified in the architecture)*

---

### For Modular Monolith Architecture

1. **Architecture Overview Diagram**
   Generate a Mermaid diagram based on the actual modules identified from the input documents. Include:
   - Client layer (actual clients from the spec)
   - API layer (actual API gateway or entry point)
   - Each domain module as a subgraph (derived from the domain model and function list)
   - Shared infrastructure (event bus, shared database)
   - Connections showing actual data flow between modules

2. **Module Boundaries**
   Document each module based on the actual domain model from the input documents:
   | Module | Responsibility | Public Interface | Internal Events |
   |--------|---------------|------------------|----------------|
   | [Module name from domain model] | [Actual responsibility] | `[IActualServiceName]` | `[actual.event.names]` |
   *(Add one row per domain module identified in the spec)*

3. **Module Communication Rules**
   - Modules communicate via **interfaces only** (no direct database access across modules)
   - Use **internal event bus** for async communication
   - Shared database with **schema separation** per module

---

### For Microservices Architecture

1. **Architecture Overview Diagram**
   Generate a Mermaid diagram based on the actual services identified from the input documents and service summaries. Include:
   - Client subgraph (actual client applications from the spec)
   - API layer subgraph (actual gateway or BFF from the spec)
   - Services subgraph (each actual service with its real name and tech stack)
   - Data layer subgraph (each service's actual database and cache)
   - Messaging subgraph (actual message broker if used)
   - Connections showing actual service-to-service calls and data flows

2. **Service Registry**
   List each service identified from the input documents and service summaries:
   | Service | Tech Stack | Owner Team | Repository | Status |
   |---------|------------|------------|------------|--------|
   | [actual-service-name] | [actual language, framework] | [actual team] | [actual repo URL] | [🟢 Production / 🟡 Staging / 🔵 Planned] |
   *(Add one row per service identified in the architecture)*

3. **Technology Stack Matrix**
   Document the actual technology choices for each service:
   | Service | Language | Framework | Database | Cache | Queue |
   |---------|----------|-----------|----------|-------|-------|
   | [actual-service-name] | [actual language] | [actual framework] | [actual DB] | [actual cache or -] | [actual queue or -] |
   *(Add one row per service)*

4. **Service Dependencies**
   Generate a Mermaid diagram showing actual dependencies between services based on the input documents:
   ```mermaid
   graph LR
       [ServiceA] --> [ServiceB]
   ```
   *(Map actual service-to-service calls identified from the spec and service summaries)*

5. **Event Contracts**
   Document actual events from the input documents:
   | Event | Publisher | Consumers | Topic |
   |-------|-----------|-----------|-------|
   | [actual.event.name] | [actual-publisher-service] | [actual-consumer-services] | [actual-topic-name] |
   *(Add one row per event identified in the databus design or service summaries)*

6. **API Contracts**
   Document actual API contracts from the input documents:
   | Service | Protocol | Base Path | Auth | Spec File |
   |---------|----------|-----------|------|-----------|
   | [actual-service-name] | [REST/gRPC/GraphQL] | [actual base path] | [actual auth method] | [actual spec file path] |
   *(Add one row per service)*

---

## Common Sections (All Architecture Types)

1. **Technology Stack Summary**
   Document the actual technology choices from the input documents:
   | Layer | Technology | Justification |
   |-------|------------|---------------|
   | [actual layer] | [actual technology chosen] | [actual justification from the spec] |
   *(Add one row per technology layer identified in the spec)*

2. **Communication Patterns**
   Based on the actual patterns identified from the input documents:
   - **Synchronous:** [actual protocol(s) used, e.g., REST, gRPC, GraphQL]
   - **Asynchronous:** [actual message broker if used, e.g., Kafka, RabbitMQ, NATS — or "N/A"]
   - **Real-time:** [actual real-time mechanism if used, e.g., WebSocket, SSE — or "N/A"]

3. **Cross-Cutting Concerns**
   Document the actual observability and operational tools from the input documents:
   | Concern | Technology | Details |
   |---------|------------|--------|
   | [concern type] | [actual tool/technology chosen] | [actual implementation details] |
   *(Add rows for logging, monitoring, tracing, error handling as applicable)*

4. **Deployment Architecture**
   Based on the actual deployment decisions from the input documents:
   - **Environments:** [list actual environments defined in the spec]
   - **Orchestration:** [actual container orchestration platform]
   - **CI/CD:** [actual CI/CD platform]
   - **Infrastructure:** [actual IaC tooling if defined]

5. **Security Architecture**
   Document the actual security mechanisms from the access control design and spec:
   | Layer | Mechanism | Details |
   |-------|-----------|--------|
   | [actual security layer] | [actual mechanism] | [actual implementation details] |
   *(Add rows for each security layer defined in the access control design)*

**Guidelines:**
1. Keep diagrams at appropriate abstraction level
2. Document key architectural decisions (ADRs)
3. Include scalability considerations
4. Address fault tolerance and resilience
5. Consider cost implications
6. **For microservices:** Clearly define service boundaries and ownership
7. **For modular monolith:** Enforce module boundaries via interfaces
```

## Output

Save the generated document as `docs/12-architecture-summary.md` in your project.

## Next Steps

After completing this step, proceed to:

**For microservices:**
1. `/aspec-12.1-service-registry` - Document existing services (if any)
2. `/aspec-13-module-design --arch=microservices --service=<service-name> --stack=<stack>` - Design each service

**For modular monolith:**
1. `/aspec-13-module-design --arch=modular-monolith --stack=<stack>` - Design module structure

**For monolith:**
1. `/aspec-13-module-design --arch=monolith --stack=<stack>` - Design internal modules
