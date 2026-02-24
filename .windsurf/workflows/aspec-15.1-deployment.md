---
description: Generate Deployment Plan for local development and remote (prototype/production) environments
arguments:
  send: true
---

# Deployment Plan Generator (Step 15.1)

Generate a comprehensive deployment plan covering local development environment setup and remote (prototype/production) deployment, tailored to the project's actual tech stack and architecture.

## Usage

```
/aspec-15.1-deployment [input_files]
```

**Examples:**
- `/aspec-15.1-deployment docs/` — reads all spec docs for context
- `/aspec-15.1-deployment docs/12-architecture-summary.md docs/01-project-spec.md` — reads specific files
- `/aspec-15.1-deployment` — auto-reads default context files listed below

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths or a directory, read those files as input documents.
If empty, read the following files for context:
- `docs/12-architecture-summary.md` (deployment architecture, scaling, networking)
- `docs/01-project-spec.md` (tech stack, constraints)
- `AGENTS.md` (directory structure, tech stack, security guidelines)
- `docs/11-persistence-design.md` (database schema, migrations)
- `docs/14-api-design.md` (API endpoints, protocols)
- `docs/project-scope-and-phases.md` (phases, milestones)
- `docs/phase-task-list.md` (Phase 0 infrastructure tasks)

## Prerequisites

Ensure you have completed the following documents:
- Project Spec Description (`docs/01-project-spec.md`)
- Architecture Summary (`docs/12-architecture-summary.md`)
- Persistence Design (`docs/11-persistence-design.md`)
- API Design (`docs/14-api-design.md`)
- Project AGENTS.md

## Prompt

```
You are a DevOps and infrastructure specialist. Based on the project's architecture summary, tech stack, and specification documents provided as input, generate a comprehensive Deployment Plan covering both local development and remote (prototype/production) environments.

**IMPORTANT — Tech Stack Discovery:**
Do NOT assume any specific technology. Read the input documents to identify:
- Backend language/framework (e.g., Go, Rust, Node.js, Python, Java, etc.)
- Frontend framework (e.g., React, Vue, Svelte, Flutter, etc.)
- Database(s) (e.g., PostgreSQL, MySQL, MongoDB, etc.)
- Cache layer (e.g., Redis, Memcached, or none)
- Message queue / event bus (e.g., NATS, RabbitMQ, SQS, Kafka, or none)
- Real-time services (e.g., WebSocket, SSE, WebRTC/SFU, or none)
- Object storage (e.g., S3, GCS, MinIO, or none)
- Cloud provider (e.g., AWS, GCP, Azure, self-hosted, or not yet decided)
- Container orchestration (e.g., Kubernetes, Docker Compose only, ECS, Cloud Run, etc.)
- CI/CD platform (e.g., GitHub Actions, GitLab CI, Jenkins, etc.)
- Observability stack (e.g., Prometheus+Grafana, Datadog, CloudWatch, etc.)

If a technology is not mentioned in the input documents, mark it as "TBD — decide before deployment" and provide a short recommendation with trade-offs.

**Output the following sections, adapting each to the discovered tech stack:**

---

### Part 1: Local Development Environment

1. **System Prerequisites**
   - Required tools and versions (derived from tech stack)
   - OS-specific notes (macOS, Linux, Windows/WSL2)
   - IDE recommendations and extensions

2. **Local Service Stack (Docker Compose)**
   - Full `docker-compose.yml` specification for all infrastructure services identified in the architecture (database, cache, queue, storage emulator, etc.)
   - Port mapping table
   - Volume mounts for data persistence
   - Health checks for each service

3. **Backend Setup**
   - Project/workspace configuration (derived from backend language)
   - Environment variables (`.env.example` with all required vars)
   - Database migration workflow (using the project's migration tool)
   - Running the backend server locally
   - Hot-reload / watch mode setup

4. **Frontend Setup** (if applicable)
   - Package manager and dependency installation
   - Environment variables (`.env.local` with API URL, etc.)
   - Dev server configuration
   - Proxy configuration for API and WebSocket (if applicable)

5. **Additional Services Setup** (if applicable)
   - Any project-specific services (e.g., WebRTC SFU, search engine, ML model server)
   - Local configuration for each
   - Port requirements and networking notes

6. **Development Workflow**
   - Step-by-step "first run" guide (clone → setup → run)
   - Database seeding (test data)
   - Running tests locally (unit, integration)
   - Debugging tips per layer

7. **Common Issues & Troubleshooting**
   - Port conflicts
   - Database connection issues
   - Service dependency startup order
   - OS-specific gotchas

---

### Part 2: Remote Environment — Prototype / Staging / Production

1. **Infrastructure Overview**
   - Environment tiers (e.g., Dev → Staging → Production)
   - Infrastructure-as-Code tool (Terraform, Pulumi, CDK, etc.)
   - Cloud provider and core managed services

2. **Infrastructure Provisioning**
   - Network layout (VPC/VNet, subnets, NAT, firewall rules)
   - Compute (container orchestration cluster, serverless, VMs — as appropriate)
   - Managed database (single-node for dev, HA for staging/prod)
   - Cache / message queue managed services
   - Object storage buckets
   - Load balancer with TLS termination
   - DNS configuration

3. **Container / Deployment Manifests**
   - Namespace / project / resource group strategy
   - Deployment specs for each service (replicas, resource limits, health probes, autoscaling)
   - Service and Ingress / Gateway definitions
   - Configuration and secret injection
   - Disruption budgets and network policies

4. **CI/CD Pipeline**
   - Pipeline stages:
     - Lint (language-specific linters and formatters)
     - Test (unit, integration, migration dry-run)
     - Build (container image, registry push)
     - Deploy (GitOps sync or direct deploy)
   - Environment promotion strategy (staging → production)
   - Progressive rollout (canary, blue-green, or rolling)
   - Rollback procedures

5. **Secret Management**
   - Secret store (cloud provider secret manager, Vault, etc.)
   - List of required secrets (database URL, API keys, tokens, etc.)
   - Secret rotation schedule
   - Never-commit policy enforcement (`.gitignore`, pre-commit hooks)

6. **Observability Setup**
   - Metrics collection and dashboards
   - Log aggregation
   - Distributed tracing (if applicable)
   - Alert rules and notification routing

7. **Database Migration Strategy**
   - Migration execution in CI/CD (pre-deploy job)
   - Rollback procedures
   - Schema versioning
   - Zero-downtime migration patterns

8. **SSL/TLS & Domain Configuration**
   - Certificate provisioning (managed or Let's Encrypt)
   - TLS termination point
   - CDN / static asset distribution (if applicable)

9. **Environment Parity Matrix**
   - Table comparing Dev / Staging / Production configurations
   - Resource sizing per environment
   - Feature flags for environment-specific behavior

---

### Part 3: Operational Runbook

1. **Health Check Endpoints**
   - List health/readiness endpoints for every service
   - Expected responses and failure modes

2. **Scaling Procedures**
   - Horizontal scaling triggers and thresholds
   - Database scaling (read replicas, instance upgrade)
   - Manual vs. automatic scaling guidance

3. **Incident Response**
   - Per-service failure scenarios and impact
   - Failover and degradation strategies
   - Communication and escalation path

4. **Backup & Recovery**
   - Database backups (retention, point-in-time recovery)
   - Object storage versioning
   - Cache persistence (if applicable)
   - Recovery time objectives (RTO/RPO)

5. **Cost Estimation**
   - MVP sizing table (instance types/tiers, counts, estimated monthly cost)
   - Growth triggers and scaling cost impact

---

Format the output as a structured markdown document. Include actual configuration snippets (docker-compose.yml, Kubernetes YAML / Dockerfile / Terraform HCL / CI pipeline YAML as applicable) — use realistic values with placeholder secrets marked as `<REPLACE_ME>`.

Save as `docs/15.1-deployment-plan.md`. If the output exceeds 30K characters, split into:
- `docs/15.1-deployment-plan.md` (Part 1: Local Development)
- `docs/15.1-deployment-plan-remote.md` (Part 2: Remote + Part 3: Operational Runbook)
```

## Output

Save the generated documents in your project:
- `docs/15.1-deployment-plan.md` — Local development environment (prerequisites, Docker Compose, backend/frontend setup, dev workflow, troubleshooting)
- `docs/15.1-deployment-plan-remote.md` — Remote deployment (infrastructure, CI/CD, observability, operational runbook)

If the local section is very large, further split into:
- `docs/15.1-deployment-plan.md` — Overview, prerequisites, Docker Compose, port mapping
- `docs/15.1-deployment-plan-services.md` — Backend, frontend, additional services setup, dev workflow, troubleshooting

## Next Steps

After completing this step, proceed to:
- `/aspec-16-ux-design` — Generate UX Wireframe Flow & UI Design (Step 16)
- `/aspec-17-plan` — Generate Project Scope and Phases (Step 17)
- `/aspec-18-tasks` — Generate Phase Task List (Step 18)
