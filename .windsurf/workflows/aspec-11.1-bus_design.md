---
description: Generate Databus Communication Design (Step 11.1)
arguments:
  send: true
---

# Databus Communication Design Generator

Use this workflow after completing architecture/persistence steps (Steps 11–12) and when you need to document cross-service messaging over NATS JetStream or other buses. This workflow is referenced by `/aspec-13-module-design` as Step 11.1 in the spec sequence.

## Usage

```
/aspec-11.1-bus_design [input_files]
```

**Examples:**
- `/aspec-11.1-bus_design docs/11-persistence-design.md docs/12-architecture.md`
- `/aspec-11.1-bus_design` (will scan `docs/` for relevant specs)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` includes file paths, read those documents (architecture, persistence, data model) as context. If empty, default to:
- `docs/11-persistence-design.md`
- `docs/12-architecture.md`
- `docs/07-function-list.md`

## Prompt

```
You are a distributed-systems architect documenting the Databus (NATS JetStream) communication layer.

**Input Documents:**
- Persistence design
- Architecture summary
- Function list / relevant specs

**Output Requirements:**
1. **Messaging Topology Overview**
   - Describe publishers, subscribers, queue groups, stream boundaries, retention policies.
   - Include ASCII or Mermaid diagram showing services and subjects.

2. **Stream Definitions**
   | Stream | Subjects | Max Messages | Storage | Retention | Description |
   |--------|----------|--------------|---------|-----------|-------------|
   - Cover VAT events, UI overrides, remote bridge telemetry, audit fan-out, cache invalidations.

3. **Message Schemas** (JSON/Avro-style snippets)
   - Define headers, payload fields, versioning, idempotency keys.
   - Include validation notes for security (tenant isolation, checksum, signature fields).

4. **Routing & Delivery Semantics**
   - Push vs pull consumers, ack patterns, replay strategy, dead-letter handling.
   - Mention ordered delivery, batching, backpressure controls.

5. **Operational Policies**
   - Observability metrics, alert thresholds, encryption/auth (NKeys/JWT), disaster recovery for streams.
   - Rolling upgrades, schema evolution, contract testing workflow.

6. **Integration Checklist**
   - Steps for new service onboarding (register schema, configure subject ACLs, update docs).
   - Security reviews and performance benchmarks required before go-live.

**Guidelines:**
- Align with NATS JetStream best practices from project standards.
- Reference VAT/namespace concepts when describing payload filters.
- Clearly mark multi-tenant isolation and auditing touchpoints.
- Provide actionable tables/checklists developers can follow.
```

## Output

Save the generated design as `docs/11.1-databus-design.md` (or the appropriate folder) in your project.

## Next Steps

After this step, proceed to any downstream workflows that consume the databus spec (e.g., `/aspec-21-integration-tests`).
