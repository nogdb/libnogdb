---
description: Design a new sub-feature spec from a feature description, referencing existing project specs as sources of truth. Outputs 0x-prefixed documents (0a, 0b, 0c, ...) into a dedicated docs subfolder.
arguments:
  send: true
---

# Sub-Feature Spec Generator

Use this workflow to design a **new sub-feature** that extends the existing project. It produces a complete spec suite inside `docs/<feature-slug>/` using the `0x-name.md` naming convention (where `x` is an alphabetical character: `0a`, `0b`, `0c`, …).

This workflow is **stack-agnostic**. It reads `AGENTS.md` at the project root to discover the technology stack, directory structure, naming conventions, and code patterns. All generated documents adapt to whatever stack the project uses.

## Usage

```
/aspec-01.1-add-subspec <feature_description> [--sources <file1> <file2> ...]
```

**Examples:**
- `/aspec-01.1-add-subspec "Add emoji reactions to messages"`
- `/aspec-01.1-add-subspec docs/ideas/file-attachments.md --sources docs/01-project-spec.md docs/04-data-model.md`
- `/aspec-01.1-add-subspec "Thread messaging with nested replies" --sources docs/direct_messaging/`

## Input

```text
$ARGUMENTS
```

- If `$ARGUMENTS` contains a file path, read that file for the feature description.
- If `$ARGUMENTS` is a text description, use it directly.
- If `--sources` is provided, read those files/directories as the primary sources of truth.
- If `--sources` is omitted, auto-discover sources (see Step 0).
- If empty, ask the user to provide a feature description.

## Prerequisites

- `AGENTS.md` must exist at the project root. This is the **primary source of truth** for the project's tech stack, directory layout, conventions, and module responsibilities.
- The main project spec chain (`docs/01-*.md` through `docs/14-*.md`) should exist.
- The codebase should be in a working state so the spec can reference actual file paths and interfaces.

---

## Workflow Steps

### Step 0 — Read AGENTS.md and Discover Project Stack

**This step runs first, before anything else.**

1. Read `AGENTS.md` from the project root.
2. Extract the following into working context (referred to as `$PROJECT` throughout):

| Context Variable | Where to Find in AGENTS.md | Example |
|-----------------|---------------------------|---------|
| `$PROJECT.name` | Project Context → Product | "NeuChat" |
| `$PROJECT.stack` | Technology Stack table | Full table of layers, technologies, versions |
| `$PROJECT.backend_lang` | Technology Stack → Backend row | "Rust 1.76+, Axum 0.7" or "Go 1.22, Gin" or "Python 3.12, FastAPI" etc. |
| `$PROJECT.frontend_lang` | Technology Stack → Frontend row | "React + TypeScript" or "Vue 3 + TypeScript" or "Flutter" etc. |
| `$PROJECT.db` | Technology Stack → Database row | "PostgreSQL 16" or "MySQL 8" or "MongoDB 7" etc. |
| `$PROJECT.dir_structure` | Directory Structure section | Full tree with annotated paths |
| `$PROJECT.backend_dir` | Directory Structure → backend root | `backend/src/modules/` or `src/` or `app/` etc. |
| `$PROJECT.frontend_dir` | Directory Structure → frontend root | `frontend/src/` or `client/src/` or `web/` etc. |
| `$PROJECT.naming` | Code Standards → Naming & Style | Language-specific conventions |
| `$PROJECT.interface_convention` | Code Standards or Core Principles | e.g., `I`-prefix traits, Go interfaces, abstract classes, etc. |
| `$PROJECT.api_conventions` | API Guidelines section | Base URL, envelope format, auth, pagination, rate limiting |
| `$PROJECT.db_conventions` | Database Guidelines section | Migration tool, ID strategy, timestamp conventions |
| `$PROJECT.test_conventions` | Documentation & Testing section | Coverage targets, test patterns, mock strategies |
| `$PROJECT.realtime` | Technology Stack → Real-time row | "WebSockets (Axum)" or "Socket.IO" or "SSE" or "gRPC streams" etc. |

3. If `--sources` was omitted, auto-discover sources of truth by scanning:
   - `AGENTS.md` → Reference Docs field (lists the spec chain paths)
   - `docs/` directory for numbered spec files (`01-*.md` through `14-*.md`)
   - `$PROJECT.backend_dir` for existing modules, interfaces, services
   - `$PROJECT.frontend_dir` for existing components, API client, hooks/stores

> **All subsequent steps use `$PROJECT.*` variables instead of hardcoded stack references.**

### Step 1 — Understand the Feature

1. Read the feature description provided by the user.
2. Read all source-of-truth documents (auto-discovered or explicitly provided).
3. Scan the existing codebase to identify:
   - **Existing tables/schemas** that the feature can reuse or must extend (scan migrations dir or persistence spec).
   - **Existing modules, services, and interfaces** that the feature touches (scan `$PROJECT.backend_dir`).
   - **Existing frontend components and pages** that need modification (scan `$PROJECT.frontend_dir`).
   - **Existing real-time events** if the feature involves real-time behavior (scan `$PROJECT.realtime` related code).
4. Produce a brief **impact analysis** listing what exists vs. what is new.

### Step 2 — Derive the Feature Slug

- Convert the feature name to a kebab-case slug (e.g., `direct-messaging`, `thread-messaging`, `emoji-reactions`).
- Create the output directory: `docs/<feature-slug>/`.
- If the directory already exists, **ask the user** whether to overwrite or append.

### Step 3 — Generate Spec Documents

Generate the following documents **in order**. Each document must:
- Reference the source-of-truth documents explicitly (quote table schemas, interface signatures, endpoint patterns).
- Clearly separate **existing** (reused) elements from **new** (to be created) elements.
- Follow the conventions and style of the existing project (as defined in `AGENTS.md`).
- Use the correct language, framework, and patterns from `$PROJECT.stack`.

#### Required Documents

| File | Title | Description | Generator |
|------|-------|-------------|----------|
| `README.md` | Feature Spec Index | Problem statement, goal, scope (in/out), design decisions, acceptance criteria, key files, links to all 0x docs | Inline (Step 4) |
| `02-user-stories.md` | User Stories | User stories with Story ID, priority, story points, Given/When/Then acceptance criteria | **Delegate to `/aspec-02-user-stories`** |
| `03-use-cases.md` | Use Cases | Detailed use cases with numbered flows, alternative/exception flows, business rules | **Delegate to `/aspec-03-use-cases`** |
| `0a-data-model.md` | Data Model | Existing schema analysis + new tables/columns/indexes/migrations needed | Inline (Step 5) |
| `0b-api-design.md` | API Design | New endpoints following project API conventions from `$PROJECT.api_conventions` | Inline (Step 5) |
| `0c-backend-changes.md` | Backend Changes | Module structure using `$PROJECT.backend_lang` patterns, interfaces-first, service/repository/controller | Inline (Step 5) |
| `0d-frontend-changes.md` | Frontend Changes | Component tree using `$PROJECT.frontend_lang` patterns, API client extensions, state management | Inline (Step 5) |

#### Conditional Documents (generate only if applicable)

| File | Title | When to Generate |
|------|-------|-----------------|
| `0e-realtime-events.md` | Real-time Events | Feature involves real-time updates via `$PROJECT.realtime` (WebSockets, SSE, gRPC streams, etc.) |
| `0f-notification-design.md` | Notification Design | Feature triggers in-app or push notifications |
| `0g-access-control.md` | Access Control | Feature introduces new permissions or authorization rules |
| `0h-migration-plan.md` | Migration Plan | Feature requires data migration for existing records |
| `0i-test-plan.md` | Test Plan | Complex feature needing explicit test strategy |
| `16-ux-wireframes.md` | UX Wireframes | Feature has user-facing UI — **delegate to `/aspec-16-ux-design`** |

> Continue with `0j`, `0k`, … for any additional documents the feature requires.

### Step 4 — Generate README.md

The `README.md` must follow this structure. Replace `$PROJECT.name` with the actual project name from `AGENTS.md`:

```markdown
# <Feature Name> Feature Spec

> **MVP specification for <feature summary> in $PROJECT.name.**

| Field | Value |
|-------|-------|
| **Status** | Specification (not yet implemented) |
| **Created** | <date> |
| **Tech Stack** | <summarize from $PROJECT.stack: e.g., "Rust/Axum + React/TS + PostgreSQL"> |
| **Source of truth (backend)** | <relevant paths from $PROJECT.backend_dir> |
| **Source of truth (frontend)** | <relevant paths from $PROJECT.frontend_dir> |
| **Depends on** | <modules this feature depends on> |

---

## Problem Statement
<What gap exists today?>

## Goal
<What does this feature deliver?>

---

## Scope (Minimum)

### In Scope
- <bullet list>

### Out of Scope (Future)
- <bullet list>

---

## Spec Documents

| # | Document | Description |
|---|----------|-------------|
| a | [0a-data-model.md](0a-data-model.md) | ... |
| b | [0b-api-design.md](0b-api-design.md) | ... |
| ... | ... | ... |

---

## Design Decisions
### 1. <Decision title>
<Rationale>

---

## Acceptance Criteria (High-Level)
- [ ] <criterion>

---

## Key Files (Current State)

### Backend ($PROJECT.backend_lang)
| File | Relevance |
|------|-----------|

### Frontend ($PROJECT.frontend_lang)
| File | Relevance |
|------|-----------|

---

*Document Version: 1.0*
```

### Step 5 — Generate Delegated and Inline Documents

#### Delegated Documents

The following documents are generated by invoking existing aspec workflows. Each delegation must:
- Pass the feature's `README.md` (generated in Step 4) as the primary input instead of the root-level project spec.
- Override the output path from `docs/` to `docs/<feature-slug>/`.
- Scope the content to **this feature only** (not the entire project).

##### 02-user-stories.md — Delegate to `/aspec-02-user-stories`

1. Read the `/aspec-02-user-stories` workflow template from `.windsurf/workflows/aspec-02-user-stories.md`.
2. Use the template's **Output Requirements** format (Story ID `US-<FEATURE>-XXX`, priority, story points, Given/When/Then acceptance criteria, INVEST compliance).
3. Input: `docs/<feature-slug>/README.md` (the feature spec, not the root project spec).
4. Output: `docs/<feature-slug>/02-user-stories.md`.
5. Scope: Generate stories **only** for this sub-feature's use cases, not the entire project.

##### 03-use-cases.md — Delegate to `/aspec-03-use-cases`

1. Read the `/aspec-03-use-cases` workflow template from `.windsurf/workflows/aspec-03-use-cases.md`.
2. Use the template's **Output Requirements** format (Use Case ID `UC-<FEATURE>-XXX`, numbered main flow, alternative flows, exception flows, business rules, related user stories).
3. Input: `docs/<feature-slug>/README.md` + `docs/<feature-slug>/02-user-stories.md`.
4. Output: `docs/<feature-slug>/03-use-cases.md`.
5. Scope: Generate use cases **only** for this sub-feature.

##### 16-ux-wireframes.md — Delegate to `/aspec-16-ux-design` (conditional)

Only generate if the feature has user-facing UI.

1. Read the `/aspec-16-ux-design` workflow template from `.windsurf/workflows/aspec-16-ux-design.md`.
2. Use the template's **Output Requirements** format (information architecture, Mermaid user flows, screen inventory, wireframe specs, navigation design, component library, responsive breakpoints, interaction patterns, accessibility requirements).
3. Input: `docs/<feature-slug>/02-user-stories.md` + `docs/<feature-slug>/03-use-cases.md`.
4. Output: `docs/<feature-slug>/16-ux-wireframes.md`.
5. Scope: Generate wireframes **only** for this sub-feature's screens. Reference existing screens from the main `docs/16-ux-wireframes.md` where navigation connects to them, but do not regenerate them.

#### Inline Documents (0x series)

For each document below, follow these conventions. **Adapt all code examples, patterns, and tooling to `$PROJECT.stack`.**

#### 0a-data-model.md
- Start with **existing schema** (copy actual DDL from migrations or persistence spec).
- Then list **new tables / columns / indexes / constraints**.
- Include migration SQL using the project's migration tool (from `$PROJECT.db_conventions`).
- Reference the project's data model and persistence design docs.

#### 0b-api-design.md
- Follow the response envelope format from `$PROJECT.api_conventions`.
- Include path params, query params, request body, response examples, error responses.
- Reference existing endpoint patterns for consistency.
- Include permission/authorization codes if the project uses RBAC (check `AGENTS.md`).

#### 0c-backend-changes.md
- Show the new module directory structure under `$PROJECT.backend_dir`.
- Define **interfaces first** using the project's interface convention from `$PROJECT.interface_convention`:
  - **Rust**: traits with `I`-prefix (e.g., `IFooService`), `Arc<dyn Trait>`
  - **Go**: interfaces in consumer package
  - **Python**: abstract base classes or Protocol types
  - **Java/Kotlin**: interfaces with dependency injection
  - **TypeScript (Node)**: interfaces or abstract classes
  - *(Adapt to whatever `$PROJECT.backend_lang` specifies)*
- Then show service, repository, and controller/handler implementations.
- List all modified existing files with the specific changes.
- Include routing registration and dependency wiring per the project's patterns.

#### 0d-frontend-changes.md
- List all new and modified files in a summary table.
- Show type definitions for API responses using `$PROJECT.frontend_lang`:
  - **TypeScript**: interfaces/types
  - **Dart/Flutter**: model classes
  - **Kotlin (Android)**: data classes
  - **Swift (iOS)**: structs/Codable
  - *(Adapt to whatever `$PROJECT.frontend_lang` specifies)*
- Show component signatures and props/parameters.
- Describe state management approach per project conventions (from `AGENTS.md`).
- Include API client method signatures.

#### 0e-realtime-events.md (if applicable)
- Identify the real-time transport from `$PROJECT.realtime` (WebSockets, SSE, gRPC streams, Socket.IO, etc.).
- Delivery model comparison table (broadcast vs. direct delivery).
- Server→Client and Client→Server event definitions with payloads.
- Event routing logic.

#### 16-ux-wireframes.md (if applicable)
- **Delegated** — see "Delegated Documents" section above.
- Generated using the `/aspec-16-ux-design` workflow template with all 9 output sections.

### Step 6 — Cross-Reference Validation

After generating all documents:

**Delegated document format checks:**
1. Verify `02-user-stories.md` follows the `/aspec-02-user-stories` template: Story ID format (`US-<FEATURE>-XXX`), priority, story points, Given/When/Then acceptance criteria, INVEST compliance.
2. Verify `03-use-cases.md` follows the `/aspec-03-use-cases` template: Use Case ID format (`UC-<FEATURE>-XXX`), numbered main flow, alternative flows, exception flows, business rules, related user stories.
3. Verify `16-ux-wireframes.md` (if generated) follows the `/aspec-16-ux-design` template: all 9 sections (info architecture, user flows, screen inventory, wireframe specs, navigation, component library, responsive breakpoints, interaction patterns, accessibility).

**Cross-document consistency checks:**
4. Verify every API endpoint in `0b` has a corresponding handler in `0c` and API client method in `0d`.
5. Verify every new table/column in `0a` has repository methods in `0c`.
6. Verify every user story in `02` maps to at least one use case in `03`.
7. Verify every use case in `03` maps to at least one API endpoint in `0b`.
8. Verify real-time events in `0e` (if any) are handled in both `0c` (backend) and `0d` (frontend).
9. Verify wireframe screens in `16` (if generated) map to use cases in `03`.
10. List any gaps found and resolve them before finalizing.

### Step 7 — Generate Supporting Files

After all spec documents are complete:

1. **`phase-task-list.md`** — Break the implementation into phases (e.g., Phase 1: Backend, Phase 2: Frontend, Phase 3: Integration) with numbered tasks referencing the spec documents.
2. **`implementation-log.md`** — Create an empty log template:
   ```markdown
   # <Feature Name> — Implementation Log

   ## Phase 1 (Backend) — <date>
   - **Status:** Not started
   - **Progress:**
     - (tasks will be logged here)
   ```

---

## Output

All documents are saved to `docs/<feature-slug>/`:

```
docs/<feature-slug>/
├── README.md
├── 02-user-stories.md
├── 03-use-cases.md
├── 0a-data-model.md
├── 0b-api-design.md
├── 0c-backend-changes.md
├── 0d-frontend-changes.md
├── 0e-realtime-events.md           (conditional)
├── 0f-notification-design.md       (conditional)
├── 0g-access-control.md            (conditional)
├── 0h-migration-plan.md            (conditional)
├── 0i-test-plan.md                 (conditional)
├── 16-ux-wireframes.md             (conditional)
├── phase-task-list.md
└── implementation-log.md
```

## Quality Checklist

Before finalizing, verify:
- [ ] `AGENTS.md` was read and `$PROJECT.*` context was extracted before generating any documents
- [ ] All documents reference actual source-of-truth files (not hypothetical paths)
- [ ] Existing schema/code is quoted verbatim, not paraphrased
- [ ] New vs. existing elements are clearly separated in every document
- [ ] Interface-first design: interfaces/traits/protocols defined before implementations (per `$PROJECT.interface_convention`)
- [ ] API follows project conventions from `$PROJECT.api_conventions` (URL style, envelope format, auth, pagination)
- [ ] Backend code examples use `$PROJECT.backend_lang` patterns (correct language, framework, idioms)
- [ ] Frontend code examples use `$PROJECT.frontend_lang` patterns (correct language, framework, idioms)
- [ ] Database migrations use `$PROJECT.db_conventions` tooling and ID strategy
- [ ] Test strategy follows `$PROJECT.test_conventions` (coverage targets, mock patterns)
- [ ] Cross-references between documents are consistent (no orphan endpoints or unhandled events)
- [ ] Acceptance criteria in README cover all user stories
- [ ] Phase task list covers all implementation work from all spec documents
- [ ] No hardcoded stack assumptions — everything derived from `AGENTS.md`

## Next Steps

After completing this workflow, proceed to:
- Review and refine the generated spec with stakeholders
- `/aspec-18-tasks` — Generate detailed task list from the phase plan
- `/aspec-20-implement` — Begin implementation following the spec
