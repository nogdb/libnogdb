---
description: Generate per-page Wireframe Flow Requirements with screen IDs, component IDs, state IDs, wireflow step IDs, and traceability matrices (Step 16.2)
arguments:
  send: true
---

# Per-Page Wireframe Flow Requirements Generator

Use this workflow after completing Step 16.1 (Per-Page UX Sub-Flow). It takes the per-page UX sub-flow files from `docs/ux-flows/` and produces a structured **Wireframe Flow Requirements** document for each page, with formal ID conventions for screens, wireflow steps, components, and states — enabling end-to-end traceability from spec to code to test.

## Usage

```
/aspec-16.2-ux-wireframe-flow-req [input_dir]
```

**Examples:**
- `/aspec-16.2-ux-wireframe-flow-req docs/ux-flows/`
- `/aspec-16.2-ux-wireframe-flow-req` (defaults to `docs/ux-flows/`)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains a directory path, read all `SCR-*.md` files in that directory.
If `$ARGUMENTS` contains specific file paths, read those files.
If empty, read all `SCR-*.md` files from `docs/ux-flows/`.

## Prerequisites

Ensure you have completed:
- UX Wireframe Flow & UI Design (Step 16 — `/aspec-16-ux-design`)
- Per-Page UX Sub-Flow (Step 16.1 — `/aspec-16.1-ux-page-flow`)
- API Design (Step 14 — `/aspec-14-api-design`)
- Data Structure (Step 5 — `/aspec-05-data-structure`)
- Internal Module Design (Step 13 — `/aspec-13-module-design`)
- Test Plan (Step 19 — `/aspec-19-testplan`) — optional but recommended

**Additional input files** (auto-loaded if present):
- `docs/ux-flows/README.md` — Screen Inventory index from Step 16.1
- `docs/14-api-design.md` — for endpoint mapping per page
- `docs/05-data-structure.md` — for DTO field validation rules
- `docs/13-module-design.md` — for file/folder mapping
- `docs/16-ux-wireframes.md` — for original wireframe specifications
- `docs/19-test-plan.md` — for acceptance criteria cross-reference

## Prompt

```
You are a UX requirements engineer who transforms UX sub-flow specifications into formally structured wireframe flow requirements with consistent ID conventions. Your job is to read each per-page UX sub-flow file (from Step 16.1) and generate a companion Wireframe Flow Requirements document that assigns formal IDs to every screen, wireflow step, component, and state — creating a complete traceability chain from design spec to implementation to testing.

**Input Documents:**
- `docs/ux-flows/SCR-*.md` — Per-page UX sub-flow files (from Step 16.1)
- `docs/ux-flows/README.md` — Screen Inventory index
- `docs/14-api-design.md` — API endpoints, request/response schemas (read if present)
- `docs/05-data-structure.md` — Data models, DTOs, enums, validation rules (read if present)
- `docs/13-module-design.md` — Module breakdown, file/folder structure (read if present)
- `docs/16-ux-wireframes.md` — Original wireframe specifications (read if present)
- `docs/19-test-plan.md` — Acceptance criteria per feature (read if present)

---

### ID Naming Conventions

Before generating any output, internalize these four ID conventions. All IDs in every output file MUST follow these formats exactly.

#### Convention 1: Screen/Page ID
Assigns a unique, human-readable ID to every screen in the application.

**Format:** `<MODULE>-<SCREEN_TYPE>-<NUMBER>`

| Part | Description | Examples |
|------|-------------|----------|
| `MODULE` | Functional module (UPPER_CASE) | `AUTH`, `PROFILE`, `ADMIN`, `DASHBOARD`, `CHANNEL`, `DM`, `VIDEO`, `WORKSPACE`, `NOTIFICATION` |
| `SCREEN_TYPE` | Screen purpose (UPPER_CASE, hyphenated) | `LOGIN`, `SIGNUP`, `FORGOT-PASSWORD`, `VIEW`, `EDIT`, `LIST`, `DETAIL`, `SETTINGS`, `CALL` |
| `NUMBER` | Two-digit sequential within module+type | `01`, `02`, `03` |

**Examples:**
- `AUTH-LOGIN-01` — Login page
- `AUTH-SIGNUP-02` — Registration page
- `AUTH-FORGOT-PASSWORD-03` — Password reset page
- `AUTH-TOTP-SETUP-04` — TOTP enrollment page
- `PROFILE-VIEW-01` — View profile page
- `PROFILE-EDIT-02` — Edit profile page
- `DASHBOARD-HOME-01` — Main dashboard/messaging page
- `ADMIN-USER-LIST-01` — Admin user management list
- `ADMIN-USER-DETAIL-02` — Admin user detail panel
- `ADMIN-SETTINGS-01` — Admin settings page
- `VIDEO-CALL-01` — Video call room

**Mapping Rule:** Every `SCR-xxx` from Step 16.1 must map to exactly one Screen ID. Include a mapping table in the index file.

#### Convention 2: Wireflow Step ID
Assigns a unique ID to each step in a user flow through or within a screen.

**Format:** `WF-<FLOW>-<STEP_NUMBER>-<ACTION>`

| Part | Description | Examples |
|------|-------------|----------|
| `FLOW` | Flow name (UPPER_CASE, hyphenated) | `LOGIN`, `SIGNUP`, `EDIT-PROFILE`, `START-CALL`, `SEND-MESSAGE` |
| `STEP_NUMBER` | Two-digit sequential | `01`, `02`, `03` |
| `ACTION` | Action verb (UPPER_CASE, hyphenated) | `START`, `INPUT`, `SUBMIT`, `LOADING`, `SUCCESS`, `ERROR`, `CONFIRM`, `REDIRECT` |

**Examples:**
- `WF-LOGIN-01-START` — User lands on login screen
- `WF-LOGIN-02-INPUT` — User enters credentials
- `WF-LOGIN-03-SUBMIT` — User clicks login button
- `WF-LOGIN-04-LOADING` — API call in progress
- `WF-LOGIN-05-SUCCESS` — Login succeeds, redirect to dashboard
- `WF-LOGIN-06-ERROR` — Login fails, show error message
- `WF-SIGNUP-01-START` — User lands on registration screen
- `WF-SIGNUP-02-INPUT` — User fills registration form
- `WF-SIGNUP-03-VALIDATE` — Client-side validation runs
- `WF-SIGNUP-04-SUBMIT` — User clicks register button
- `WF-SIGNUP-05-LOADING` — Registration API call in progress
- `WF-SIGNUP-06-SUCCESS` — Registration succeeds
- `WF-SIGNUP-07-ERROR` — Registration fails

**Rules:**
- Every API call MUST have a corresponding `LOADING` step between `SUBMIT` and `SUCCESS`/`ERROR`.
- Every destructive action MUST have a `CONFIRM` step before `LOADING`.
- Steps must be numbered sequentially within a flow.

#### Convention 3: Component/Element ID
Assigns a unique ID to every interactive UI element within a screen.

**Format:** `<SCREEN_ID>-<ELEMENT_TYPE>-<NAME>`

| Part | Description | Examples |
|------|-------------|----------|
| `SCREEN_ID` | The Screen ID from Convention 1 | `AUTH-LOGIN-01`, `PROFILE-EDIT-02` |
| `ELEMENT_TYPE` | Element type abbreviation (UPPER_CASE) | `BTN`, `INPUT`, `LINK`, `IMG`, `CHK`, `SEL`, `TGL`, `TAB`, `MODAL`, `BADGE`, `MSG`, `ICON`, `LIST`, `CARD`, `FORM` |
| `NAME` | Descriptive name (UPPER_CASE, hyphenated) | `SUBMIT`, `EMAIL`, `PASSWORD`, `FORGOT`, `AVATAR`, `SAVE`, `CANCEL`, `DELETE`, `SEARCH` |

**Element Type Reference:**

| Abbreviation | Element Type |
|-------------|-------------|
| `BTN` | Button |
| `INPUT` | Text/email/password/number input |
| `LINK` | Hyperlink or navigation link |
| `IMG` | Image or avatar |
| `CHK` | Checkbox |
| `SEL` | Select/dropdown |
| `TGL` | Toggle switch |
| `TAB` | Tab control |
| `MODAL` | Modal/dialog |
| `BADGE` | Badge/counter |
| `MSG` | Message/alert/toast |
| `ICON` | Icon button |
| `LIST` | List container |
| `CARD` | Card container |
| `FORM` | Form container |
| `AREA` | Textarea |
| `GRID` | Grid/table container |
| `SKEL` | Skeleton loader |
| `SPIN` | Spinner/loading indicator |

**Examples:**
- `AUTH-LOGIN-01-BTN-SUBMIT` — Login submit button
- `AUTH-LOGIN-01-INPUT-EMAIL` — Email input field
- `AUTH-LOGIN-01-INPUT-PASSWORD` — Password input field
- `AUTH-LOGIN-01-LINK-FORGOT` — Forgot password link
- `AUTH-LOGIN-01-LINK-SIGNUP` — Sign up link
- `AUTH-LOGIN-01-CHK-REMEMBER` — Remember me checkbox
- `AUTH-LOGIN-01-MSG-ERROR` — Error message banner
- `AUTH-LOGIN-01-SPIN-SUBMIT` — Submit button spinner
- `PROFILE-EDIT-02-BTN-SAVE` — Save profile button
- `PROFILE-EDIT-02-IMG-AVATAR` — Avatar image/upload
- `PROFILE-EDIT-02-INPUT-DISPLAY-NAME` — Display name input
- `ADMIN-USER-LIST-01-INPUT-SEARCH` — User search input
- `ADMIN-USER-LIST-01-GRID-USERS` — User list table

**Rules:**
- Every component listed in Section 3.10 of the Step 16.1 output MUST receive a Component ID.
- The `data-testid` attribute in implementation MUST match the Component ID (kebab-case conversion: `AUTH-LOGIN-01-BTN-SUBMIT` → `data-testid="auth-login-01-btn-submit"`).

#### Convention 4: State ID
Assigns a unique ID to each visual state of a screen.

**Format:** `<SCREEN_ID>-STATE-<STATE_NAME>`

| Part | Description | Examples |
|------|-------------|----------|
| `SCREEN_ID` | The Screen ID from Convention 1 | `AUTH-LOGIN-01`, `PROFILE-EDIT-02` |
| `STATE_NAME` | State name (UPPER_CASE, hyphenated) | `EMPTY`, `LOADING`, `ERROR`, `SUCCESS`, `SUBMITTING`, `CONFIRM-DELETE`, `RATE-LIMITED` |

**Examples:**
- `AUTH-LOGIN-01-STATE-EMPTY` — Initial empty form
- `AUTH-LOGIN-01-STATE-LOADING` — Submitting credentials
- `AUTH-LOGIN-01-STATE-ERROR` — Validation or API error
- `AUTH-LOGIN-01-STATE-SUCCESS` — Login successful (before redirect)
- `AUTH-LOGIN-01-STATE-RATE-LIMITED` — Too many attempts
- `ADMIN-USER-LIST-01-STATE-LOADING` — Fetching user list
- `ADMIN-USER-LIST-01-STATE-LOADED` — User list displayed
- `ADMIN-USER-LIST-01-STATE-EMPTY` — No users match filter
- `ADMIN-USER-LIST-01-STATE-ERROR` — Failed to load users
- `ADMIN-USER-DETAIL-02-STATE-CONFIRM-DEACTIVATE` — Deactivation confirmation dialog

**Rules:**
- Every state listed in Section 3.11 of the Step 16.1 output MUST receive a State ID.
- Every wireframe state drawn in Section 3.3 MUST have a corresponding State ID.
- State IDs must be referenced in wireflow steps (Convention 2) to maintain traceability.

---

### Step-by-step Instructions

1. **Read the Screen Inventory** from `docs/ux-flows/README.md`. Build a master mapping from `SCR-xxx` IDs (Step 16.1) to the new Screen IDs (Convention 1).

2. **For each screen**, read the corresponding `docs/ux-flows/SCR-xxx-*.md` file and generate a Wireframe Flow Requirements file at:
   ```
   docs/ux-flow-reqs/<SCREEN_ID>.md
   ```
   Example: `docs/ux-flow-reqs/AUTH-LOGIN-01.md`, `docs/ux-flow-reqs/DASHBOARD-HOME-01.md`

3. **Each per-page requirements file must contain ALL of the following sections (3.0–3.10).** The goal is that a developer (or AI agent) can implement the entire page from this single file without needing to cross-reference project-level documents.

   ### 3.0 Project Context
   A preamble section pulled from `AGENTS.md`, `docs/01-project-spec.md`, and `docs/12-architecture-summary.md`. This makes each file self-contained for an agent that may not have access to project-level docs.

   ```markdown
   ## Project Context

   ### Technology Stack

   | Layer | Technology | Version / Notes |
   |-------|-----------|-----------------|
   | Framework | React + TypeScript | Strict mode, Vite bundler |
   | Styling | Tailwind CSS | Utility-first, custom design tokens in `tailwind.config.ts` |
   | State Management | Zustand (global), useState/useReducer (local) | Stores in `src/stores/` |
   | Icons | Lucide React | Tree-shakeable icon library |
   | UI Components | Custom component library (`src/components/ui/`) | Button, Input, Modal, Alert, etc. |
   | Routing | React Router v6 | Lazy-loaded pages, protected routes |
   | API Client | Custom fetch wrapper (`src/lib/api.ts`) | Bearer JWT, error envelope handling |
   | Real-time | Native WebSocket | Via `src/lib/websocket.ts`, auto-reconnect |
   | Video | `@livekit/components-react`, `livekit-client` | LiveKit pre-built video UI |
   | Testing | Vitest (unit), Playwright (E2E) | `data-testid` selectors |

   ### Code Style Conventions

   | Convention | Rule |
   |-----------|------|
   | Components | PascalCase files and exports (`LoginPage.tsx`, `VideoRoom.tsx`) |
   | Hooks | `use` prefix, camelCase (`useLoginForm.ts`, `useWebSocket.ts`) |
   | Routes | kebab-case paths (`/admin/call-history`, `/forgot-password`) |
   | CSS | Tailwind utility classes; no inline styles; no CSS modules |
   | Types | Co-located in `src/lib/api.ts` or `src/types/`; no `any` |
   | Imports | Absolute paths via `@/` alias (`@/components/ui`, `@/lib/api`) |
   | Exports | Named exports preferred; default export only for page components |
   | Comments | JSDoc for public component props; inline comments for non-obvious logic only |
   | Error handling | Try/catch in hooks; error state rendered by component; never swallow errors silently |

   ### Performance Targets

   | Metric | Target | Measurement |
   |--------|--------|-------------|
   | First Contentful Paint | < 1.5s | Lighthouse |
   | Largest Contentful Paint | < 2.5s | Lighthouse |
   | Time to Interactive | < 3.0s | Lighthouse |
   | Bundle size (per route) | < 200 KB gzipped | Vite build analysis |
   | API response rendering | < 300ms after response | Custom timing |
   | Lazy-loaded pages | All pages except Login | React.lazy + Suspense |
   | Image optimization | WebP format, max 500 KB | Build-time compression |

   ### Browser Compatibility

   | Browser | Version | Status |
   |---------|---------|--------|
   | Chrome | Latest 2 versions | ✅ Fully supported |
   | Safari | Latest 2 versions | ✅ Fully supported |
   | Firefox | Latest 2 versions | ✅ Supported (video calls limited in Docker dev) |
   | Edge | Latest 2 versions | ✅ Fully supported |
   | Mobile Safari (iOS) | iOS 15+ | ✅ Supported |
   | Chrome Mobile (Android) | Latest | ✅ Supported |
   | IE 11 | — | ❌ Not supported |

   ### Inline Documentation Standards

   | Artifact | Documentation Rule |
   |----------|-------------------|
   | Page components | JSDoc block: purpose, route, access level, related Screen ID |
   | Custom hooks | JSDoc block: purpose, params, return type, usage example |
   | API functions | JSDoc block: endpoint, request/response types, error codes |
   | Complex logic | Inline `//` comment explaining "why", not "what" |
   | Test files | Describe block mirrors Screen ID; each test references Wireflow Step ID |
   | `data-testid` | Comment above component: `{/* Component ID: AUTH-LOGIN-01-BTN-SUBMIT */}` (optional, for traceability) |

   ### Type Safety Requirements

   | Rule | Enforcement |
   |------|-------------|
   | No `any` types | ESLint `@typescript-eslint/no-explicit-any` = error |
   | Strict null checks | `tsconfig.json`: `"strict": true` |
   | API response typing | Every API function returns a typed Promise (never raw `fetch`) |
   | Props typing | Every component has an explicit `interface XxxProps` |
   | Event handlers | Typed event params (`React.ChangeEvent<HTMLInputElement>`) |
   | Enum/union types | Use TypeScript union types or enums for status fields, never raw strings |
   ```

   Pull the actual values from `AGENTS.md`, `docs/01-project-spec.md`, and `docs/12-architecture-summary.md`. If a value differs from the example above, use the project's actual value. If a document is missing, use the example values as defaults and flag the gap.

   ### 3.1 Screen Metadata
   ```markdown
   # Wireframe Flow Requirements: <Screen Name>

   | Field | Value |
   |-------|-------|
   | Screen ID | <MODULE>-<SCREEN_TYPE>-<NUMBER> |
   | Legacy Screen ID | SCR-xxx (from Step 16.1) |
   | Screen Name | ... |
   | Module | ... |
   | Route | `/login`, `/dashboard`, etc. |
   | Access | Public / Auth / Auth+Admin |
   | Related Use Cases | UC-xxx, UC-yyy |
   | Source UX Flow | `docs/ux-flows/SCR-xxx-<name>.md` |
   ```

   ### 3.2 Component Registry
   List every interactive UI element on this screen with its formal Component ID, element type, label, DTO field binding, validation rules, and test ID.

   ```markdown
   ## Component Registry

   | Component ID | Type | Label | DTO Field | Required | Validation | Test ID (`data-testid`) |
   |-------------|------|-------|-----------|----------|------------|------------------------|
   | AUTH-LOGIN-01-INPUT-EMAIL | INPUT | Email address | LoginRequestDTO.email | ✅ Yes | email format, max 255 | `auth-login-01-input-email` |
   | AUTH-LOGIN-01-INPUT-PASSWORD | INPUT | Password | LoginRequestDTO.password | ✅ Yes | min 8, max 128 | `auth-login-01-input-password` |
   | AUTH-LOGIN-01-CHK-REMEMBER | CHK | Remember me for 30 days | LoginRequestDTO.rememberMe | ❌ No | — | `auth-login-01-chk-remember` |
   | AUTH-LOGIN-01-BTN-SUBMIT | BTN | Sign in | — | — | — | `auth-login-01-btn-submit` |
   | AUTH-LOGIN-01-LINK-FORGOT | LINK | Forgot password? | — | — | — | `auth-login-01-link-forgot` |
   | AUTH-LOGIN-01-LINK-SIGNUP | LINK | Don't have an account? Sign up | — | — | — | `auth-login-01-link-signup` |
   | AUTH-LOGIN-01-MSG-ERROR | MSG | (dynamic error text) | — | — | — | `auth-login-01-msg-error` |
   | AUTH-LOGIN-01-SPIN-SUBMIT | SPIN | (submit spinner) | — | — | — | `auth-login-01-spin-submit` |

   ### Component-to-DTO Traceability

   | Component ID | DTO | Field | Type | Required | Validation (from Step 5) | Default |
   |-------------|-----|-------|------|----------|--------------------------|---------|
   | AUTH-LOGIN-01-INPUT-EMAIL | LoginRequestDTO | email | string | ✅ Yes | email format, max 255 chars | — |
   | AUTH-LOGIN-01-INPUT-PASSWORD | LoginRequestDTO | password | string | ✅ Yes | min 8, max 128 chars | — |
   | AUTH-LOGIN-01-CHK-REMEMBER | LoginRequestDTO | rememberMe | boolean | ❌ No | — | false |
   ```

   ### 3.3 State Registry
   List every visual state of this screen with its formal State ID, trigger, visual description, and exit condition.

   ```markdown
   ## State Registry

   | State ID | State Name | Trigger | What User Sees | Exit Condition | Wireframe Ref (Section 3.3 of 16.1) |
   |----------|-----------|---------|----------------|----------------|--------------------------------------|
   | AUTH-LOGIN-01-STATE-EMPTY | Empty | Page load | Empty form, button enabled, no errors | User interacts | Default State |
   | AUTH-LOGIN-01-STATE-FOCUS | Focus | Input focus | Blue border on active input | User moves to next field | Field Focus State |
   | AUTH-LOGIN-01-STATE-INLINE-ERROR | Inline Error | Field blur with invalid value | Red border, error text below field | User corrects input | Inline Error State |
   | AUTH-LOGIN-01-STATE-LOADING | Loading | Submit clicked | Button: "Signing in..." + spinner, inputs disabled | API responds | Loading State |
   | AUTH-LOGIN-01-STATE-ERROR | API Error | 401/422 response | Red alert banner, button re-enabled | User corrects & resubmits | Error State |
   | AUTH-LOGIN-01-STATE-RATE-LIMITED | Rate Limited | 429 response | Alert with countdown timer, button disabled | Timer expires | Error State |
   | AUTH-LOGIN-01-STATE-SERVER-ERROR | Server Error | 5xx response | Generic error alert, retry button | User retries | Error State |
   | AUTH-LOGIN-01-STATE-SUCCESS | Success | 200 response | Brief fade, redirect to DASHBOARD-HOME-01 | Navigation complete | — (redirect) |
   ```

   ### 3.4 Wireflow Specification
   Define the complete user flow through this screen using formal Wireflow Step IDs. Every step references a State ID and the components involved.

   ```markdown
   ## Wireflow Specification

   ### Flow: WF-LOGIN

   | Step ID | Screen | State ID | Action | Components Involved | API Call | Next (Success) | Next (Error) |
   |---------|--------|----------|--------|--------------------|---------|--------------:|-------------:|
   | WF-LOGIN-01-START | AUTH-LOGIN-01 | AUTH-LOGIN-01-STATE-EMPTY | User lands on page | — | — | WF-LOGIN-02-INPUT | — |
   | WF-LOGIN-02-INPUT | AUTH-LOGIN-01 | AUTH-LOGIN-01-STATE-EMPTY | User fills form | AUTH-LOGIN-01-INPUT-EMAIL, AUTH-LOGIN-01-INPUT-PASSWORD | — | WF-LOGIN-03-SUBMIT | — |
   | WF-LOGIN-03-SUBMIT | AUTH-LOGIN-01 | AUTH-LOGIN-01-STATE-LOADING | User clicks login | AUTH-LOGIN-01-BTN-SUBMIT | POST /auth/login | WF-LOGIN-05-SUCCESS | WF-LOGIN-06-ERROR |
   | WF-LOGIN-04-LOADING | AUTH-LOGIN-01 | AUTH-LOGIN-01-STATE-LOADING | API call in progress | AUTH-LOGIN-01-SPIN-SUBMIT | — | WF-LOGIN-05-SUCCESS | WF-LOGIN-06-ERROR |
   | WF-LOGIN-05-SUCCESS | AUTH-LOGIN-01 → DASHBOARD-HOME-01 | AUTH-LOGIN-01-STATE-SUCCESS | Redirect to dashboard | — | — | End | — |
   | WF-LOGIN-06-ERROR | AUTH-LOGIN-01 | AUTH-LOGIN-01-STATE-ERROR | Show error message | AUTH-LOGIN-01-MSG-ERROR | — | WF-LOGIN-02-INPUT | — |

   ### Wireflow Diagram (Mermaid)

   Reference the Mermaid diagram from Section 3.6.2 of the Step 16.1 file, but annotate every node with its formal Wireflow Step ID and State ID:

   ```mermaid
   flowchart TD
       WF01["WF-LOGIN-01-START\nAUTH-LOGIN-01-STATE-EMPTY\n📄 Page Load"] --> WF02["WF-LOGIN-02-INPUT\nAUTH-LOGIN-01-STATE-EMPTY\nUser fills form"]
       WF02 --> WF03{WF-LOGIN-03-SUBMIT\nClient validation OK?}
       WF03 -->|No| WFERR1["Inline Error\nAUTH-LOGIN-01-STATE-INLINE-ERROR"]
       WFERR1 --> WF02
       WF03 -->|Yes| WF04["WF-LOGIN-04-LOADING\nAUTH-LOGIN-01-STATE-LOADING\n🔄 Signing in..."]
       WF04 --> API[POST /auth/login]
       API --> RESP{API Response?}
       RESP -->|200| WF05["WF-LOGIN-05-SUCCESS\nAUTH-LOGIN-01-STATE-SUCCESS\n✅ Redirect to DASHBOARD-HOME-01"]
       RESP -->|401| WF06["WF-LOGIN-06-ERROR\nAUTH-LOGIN-01-STATE-ERROR\n❌ Invalid credentials"]
       RESP -->|429| WF06B["WF-LOGIN-06-ERROR\nAUTH-LOGIN-01-STATE-RATE-LIMITED\n❌ Too many attempts"]
       RESP -->|5xx| WF06C["WF-LOGIN-06-ERROR\nAUTH-LOGIN-01-STATE-SERVER-ERROR\n❌ Server error"]
       WF06 --> WF02
       WF06B --> WF02
       WF06C --> WF02
   ```
   ```

   ### 3.5 API Interaction Matrix
   Map every API call to its wireflow step, components, request/response, and loading/error states.

   ```markdown
   ## API Interaction Matrix

   | API Endpoint | Method | Wireflow Step | Trigger Component | Loading State ID | Success State ID | Error State IDs | Request Body | Success Response | Error Responses |
   |-------------|--------|--------------|-------------------|-----------------|-----------------|----------------|-------------|-----------------|----------------|
   | `/auth/login` | POST | WF-LOGIN-03-SUBMIT | AUTH-LOGIN-01-BTN-SUBMIT | AUTH-LOGIN-01-STATE-LOADING | AUTH-LOGIN-01-STATE-SUCCESS | AUTH-LOGIN-01-STATE-ERROR, AUTH-LOGIN-01-STATE-RATE-LIMITED, AUTH-LOGIN-01-STATE-SERVER-ERROR | `{ email, password, rememberMe? }` | `{ data: { token, refreshToken, user } }` | 401, 422, 429, 500 |
   ```

   ### 3.6 Cross-Screen Navigation Map
   Map navigation from/to this screen using formal Screen IDs.

   ```markdown
   ## Cross-Screen Navigation

   | Direction | Screen ID | Screen Name | Trigger Component | Condition |
   |-----------|----------|-------------|-------------------|-----------|
   | Incoming | AUTH-SIGNUP-02 | Registration | — | Click "Already have an account?" |
   | Incoming | — | Direct URL | — | Navigate to /login |
   | Outgoing | DASHBOARD-HOME-01 | Dashboard | — | Successful login (WF-LOGIN-05-SUCCESS) |
   | Outgoing | AUTH-FORGOT-PASSWORD-03 | Password Reset | AUTH-LOGIN-01-LINK-FORGOT | Click "Forgot password?" |
   | Outgoing | AUTH-SIGNUP-02 | Registration | AUTH-LOGIN-01-LINK-SIGNUP | Click "Sign up" |
   ```

   ### 3.7 State Transition Matrix
   A complete matrix showing valid transitions between states.

   ```markdown
   ## State Transition Matrix

   | From State ID | To State ID | Trigger | Wireflow Step |
   |--------------|------------|---------|---------------|
   | AUTH-LOGIN-01-STATE-EMPTY | AUTH-LOGIN-01-STATE-FOCUS | Input focus | WF-LOGIN-02-INPUT |
   | AUTH-LOGIN-01-STATE-FOCUS | AUTH-LOGIN-01-STATE-EMPTY | Input blur (valid) | WF-LOGIN-02-INPUT |
   | AUTH-LOGIN-01-STATE-FOCUS | AUTH-LOGIN-01-STATE-INLINE-ERROR | Input blur (invalid) | WF-LOGIN-02-INPUT |
   | AUTH-LOGIN-01-STATE-INLINE-ERROR | AUTH-LOGIN-01-STATE-FOCUS | Input focus | WF-LOGIN-02-INPUT |
   | AUTH-LOGIN-01-STATE-EMPTY | AUTH-LOGIN-01-STATE-LOADING | Submit (valid) | WF-LOGIN-03-SUBMIT |
   | AUTH-LOGIN-01-STATE-LOADING | AUTH-LOGIN-01-STATE-SUCCESS | 200 OK | WF-LOGIN-05-SUCCESS |
   | AUTH-LOGIN-01-STATE-LOADING | AUTH-LOGIN-01-STATE-ERROR | 401/422 | WF-LOGIN-06-ERROR |
   | AUTH-LOGIN-01-STATE-LOADING | AUTH-LOGIN-01-STATE-RATE-LIMITED | 429 | WF-LOGIN-06-ERROR |
   | AUTH-LOGIN-01-STATE-LOADING | AUTH-LOGIN-01-STATE-SERVER-ERROR | 5xx | WF-LOGIN-06-ERROR |
   | AUTH-LOGIN-01-STATE-ERROR | AUTH-LOGIN-01-STATE-FOCUS | Input focus | WF-LOGIN-02-INPUT |
   | AUTH-LOGIN-01-STATE-RATE-LIMITED | AUTH-LOGIN-01-STATE-EMPTY | Timer expires | — |
   | AUTH-LOGIN-01-STATE-SERVER-ERROR | AUTH-LOGIN-01-STATE-LOADING | Retry click | WF-LOGIN-03-SUBMIT |
   ```

   ### 3.8 Implementation Mapping
   Map formal IDs to implementation artifacts.

   ```markdown
   ## Implementation Mapping

   ### File Mapping

   | Artifact | Path | Screen ID Reference |
   |----------|------|-------------------|
   | Page component | `src/pages/LoginPage.tsx` | AUTH-LOGIN-01 |
   | Form component | `src/components/auth/LoginForm.tsx` | AUTH-LOGIN-01 |
   | Form hook | `src/hooks/useLoginForm.ts` | AUTH-LOGIN-01 |
   | API call | `src/lib/api.ts` → `login()` | AUTH-LOGIN-01 |
   | Types | `src/types/auth.ts` | AUTH-LOGIN-01 |

   ### `data-testid` Attribute Mapping

   Every Component ID maps to a `data-testid` attribute via kebab-case conversion:

   | Component ID | `data-testid` | HTML Element |
   |-------------|--------------|-------------|
   | AUTH-LOGIN-01-INPUT-EMAIL | `auth-login-01-input-email` | `<input data-testid="auth-login-01-input-email" />` |
   | AUTH-LOGIN-01-INPUT-PASSWORD | `auth-login-01-input-password` | `<input data-testid="auth-login-01-input-password" />` |
   | AUTH-LOGIN-01-BTN-SUBMIT | `auth-login-01-btn-submit` | `<button data-testid="auth-login-01-btn-submit" />` |
   | AUTH-LOGIN-01-CHK-REMEMBER | `auth-login-01-chk-remember` | `<input data-testid="auth-login-01-chk-remember" />` |
   | AUTH-LOGIN-01-LINK-FORGOT | `auth-login-01-link-forgot` | `<a data-testid="auth-login-01-link-forgot" />` |
   | AUTH-LOGIN-01-LINK-SIGNUP | `auth-login-01-link-signup` | `<a data-testid="auth-login-01-link-signup" />` |
   | AUTH-LOGIN-01-MSG-ERROR | `auth-login-01-msg-error` | `<div data-testid="auth-login-01-msg-error" />` |
   | AUTH-LOGIN-01-SPIN-SUBMIT | `auth-login-01-spin-submit` | `<span data-testid="auth-login-01-spin-submit" />` |

   ### State Management Mapping

   | State ID | State Variable | Value | Scope | Store / Hook | UI Effect |
   |----------|---------------|-------|-------|-------------|-----------|
   | AUTH-LOGIN-01-STATE-EMPTY | `formState` | `'idle'` | local | `useState` in `useLoginForm` | Default form |
   | AUTH-LOGIN-01-STATE-LOADING | `formState` | `'submitting'` | local | `useState` in `useLoginForm` | Disabled inputs, spinner |
   | AUTH-LOGIN-01-STATE-ERROR | `formState` | `'error'` | local | `useState` in `useLoginForm` | Error banner visible |
   | AUTH-LOGIN-01-STATE-SUCCESS | `formState` | `'success'` | local | `useState` in `useLoginForm` | Redirect triggered |

   #### State Scope Legend

   | Scope | Implementation | When to Use |
   |-------|---------------|-------------|
   | `local` | `useState` / `useReducer` inside component or custom hook | Form state, UI toggles, component-internal state |
   | `global` | Zustand store (`src/stores/*.ts`) | Auth token, current user, workspace context, presence, unread counts |
   | `url` | URL search params / route params (`useSearchParams`, `useParams`) | Pagination cursor, filters, selected tab, active channel ID |
   | `server` | React Query / SWR cache or direct API fetch | Data fetched from API, cached and revalidated |
   | `ws` | WebSocket event → Zustand store update | Real-time messages, presence updates, call notifications |

   ### Flow Step Logging

   Developers should log wireflow step IDs for debugging and analytics:

   ```typescript
   console.debug('[WF-LOGIN-01-START]', { screen: 'AUTH-LOGIN-01' });
   console.debug('[WF-LOGIN-03-SUBMIT]', { email: masked });
   console.debug('[WF-LOGIN-04-LOADING]', { state: 'AUTH-LOGIN-01-STATE-LOADING' });
   console.debug('[WF-LOGIN-05-SUCCESS]', { redirectTo: 'DASHBOARD-HOME-01' });
   console.debug('[WF-LOGIN-06-ERROR]', { status: 401, state: 'AUTH-LOGIN-01-STATE-ERROR' });
   ```
   ```

   ### 3.9 Test Mapping
   Map every testable condition to its formal IDs.

   ```markdown
   ## Test Mapping

   ### Unit / Component Tests

   | Test Case | Wireflow Step | State ID | Component IDs | Assertion |
   |-----------|--------------|----------|--------------|-----------|
   | Renders empty login form | WF-LOGIN-01-START | AUTH-LOGIN-01-STATE-EMPTY | AUTH-LOGIN-01-INPUT-EMAIL, AUTH-LOGIN-01-INPUT-PASSWORD, AUTH-LOGIN-01-BTN-SUBMIT | All fields empty, button enabled |
   | Shows inline error on empty email blur | WF-LOGIN-02-INPUT | AUTH-LOGIN-01-STATE-INLINE-ERROR | AUTH-LOGIN-01-INPUT-EMAIL | Error text visible below email |
   | Disables form during submit | WF-LOGIN-04-LOADING | AUTH-LOGIN-01-STATE-LOADING | AUTH-LOGIN-01-BTN-SUBMIT, AUTH-LOGIN-01-SPIN-SUBMIT | Button disabled, spinner visible |
   | Shows error banner on 401 | WF-LOGIN-06-ERROR | AUTH-LOGIN-01-STATE-ERROR | AUTH-LOGIN-01-MSG-ERROR | Error message: "Invalid email or password" |
   | Redirects on success | WF-LOGIN-05-SUCCESS | AUTH-LOGIN-01-STATE-SUCCESS | — | Navigate to /dashboard |

   ### E2E Tests (Playwright/Cypress Selectors)

   | Test Scenario | Selector (`data-testid`) | Action | Expected |
   |--------------|------------------------|--------|----------|
   | Login happy path | `auth-login-01-input-email` | fill "user@test.com" | — |
   | | `auth-login-01-input-password` | fill "password123" | — |
   | | `auth-login-01-btn-submit` | click | — |
   | | `auth-login-01-spin-submit` | waitFor visible | Spinner shown |
   | | — | waitForURL "/dashboard" | Redirected |
   | Login error path | `auth-login-01-btn-submit` | click (wrong creds) | — |
   | | `auth-login-01-msg-error` | waitFor visible | Error banner shown |

   ### Test Coverage Matrix

   | Screen ID | Component IDs | State IDs | Wireflow Steps | Test File |
   |----------|--------------|----------|---------------|-----------|
   | AUTH-LOGIN-01 | 8 components | 8 states | 6 steps | `tests/e2e/login.spec.ts`, `src/pages/__tests__/LoginPage.test.tsx` |
   ```

   ### 3.10 Requirements Checklist
   A checklist for spec agents, developers, and testers to verify completeness.

   ```markdown
   ## Requirements Checklist

   ### Spec Agent (Design Phase)
   - [ ] Project Context section (§3.0) populated with tech stack, code style, perf targets, browser compat, doc standards, and type safety rules
   - [ ] Screen ID assigned following `<MODULE>-<SCREEN_TYPE>-<NUMBER>` convention
   - [ ] All components have Component IDs following `<SCREEN_ID>-<ELEMENT_TYPE>-<NAME>` convention
   - [ ] All states have State IDs following `<SCREEN_ID>-STATE-<STATE_NAME>` convention
   - [ ] All wireflow steps have Step IDs following `WF-<FLOW>-<STEP>-<ACTION>` convention
   - [ ] Every API call has a LOADING step between SUBMIT and SUCCESS/ERROR
   - [ ] Every destructive action has a CONFIRM step before LOADING
   - [ ] State Transition Matrix covers all valid transitions
   - [ ] Cross-Screen Navigation uses formal Screen IDs
   - [ ] Component-to-DTO traceability table is complete
   - [ ] API Interaction Matrix maps every endpoint to wireflow steps and states
   - [ ] State Management Mapping includes Scope column (local/global/url/server/ws)

   ### Developer (Implementation Phase)
   - [ ] All components have `data-testid` attributes matching Component IDs (kebab-case)
   - [ ] Route matches Screen Metadata
   - [ ] State management variables map to State IDs with correct scope (useState vs Zustand vs URL params)
   - [ ] Console debug logs include Wireflow Step IDs
   - [ ] HTML `id`/`class` attributes include Component ID references where useful
   - [ ] Form validation rules match Component-to-DTO traceability table
   - [ ] Loading states are implemented for every API call
   - [ ] Error states match the State Registry
   - [ ] Animations/transitions match the Micro-Interaction specs from Step 16.1 §3.16 (duration, easing, properties)
   - [ ] `prefers-reduced-motion` is respected for all animations
   - [ ] Component hierarchy matches the Component Tree from Step 16.1 §3.17
   - [ ] TypeScript interfaces match Type Safety Requirements (no `any`, strict props typing)
   - [ ] Code style follows conventions in Project Context §3.0
   - [ ] JSDoc comments on page component, hooks, and API functions per Documentation Standards
   - [ ] Bundle size for this route is within performance target

   ### Tester (QA Phase)
   - [ ] Test cases reference Wireflow Step IDs
   - [ ] E2E selectors use `data-testid` from Component IDs
   - [ ] Every State ID has at least one test verifying its visual appearance
   - [ ] Every Wireflow Step has at least one test covering it
   - [ ] Loading states are verified (visible during API calls)
   - [ ] Error states are verified for each error response code
   - [ ] Cross-screen navigation is tested for each outgoing link
   - [ ] Test coverage matrix is complete
   - [ ] Page tested on all supported browsers from Browser Compatibility table
   - [ ] Performance targets verified (FCP, LCP, TTI) via Lighthouse
   - [ ] Reduced motion mode tested (animations disabled)
   ```

4. **Create an index file** at `docs/ux-flow-reqs/README.md`:
   ```markdown
   # Wireframe Flow Requirements Index

   Per-page wireframe flow requirements generated from `docs/ux-flows/` (Step 16.1).
   Each file assigns formal IDs to screens, components, states, and wireflow steps.

   ## ID Convention Summary

   | Convention | Format | Example |
   |-----------|--------|---------|
   | Screen ID | `<MODULE>-<SCREEN_TYPE>-<NUMBER>` | `AUTH-LOGIN-01` |
   | Wireflow Step ID | `WF-<FLOW>-<STEP>-<ACTION>` | `WF-LOGIN-03-SUBMIT` |
   | Component ID | `<SCREEN_ID>-<ELEMENT_TYPE>-<NAME>` | `AUTH-LOGIN-01-BTN-SUBMIT` |
   | State ID | `<SCREEN_ID>-STATE-<STATE_NAME>` | `AUTH-LOGIN-01-STATE-LOADING` |

   ## Screen ID Mapping (SCR-xxx → Formal ID)

   | SCR ID (Step 16.1) | Screen ID (Step 16.2) | Screen Name | Module | Route | File |
   |-------------------|-----------------------|-------------|--------|-------|------|
   | SCR-001 | AUTH-LOGIN-01 | Login | Auth | `/login` | [AUTH-LOGIN-01.md](AUTH-LOGIN-01.md) |
   | SCR-002 | DASHBOARD-HOME-01 | Dashboard | Dashboard | `/dashboard` | [DASHBOARD-HOME-01.md](DASHBOARD-HOME-01.md) |
   | SCR-003 | AUTH-SIGNUP-02 | Registration | Auth | `/register` | [AUTH-SIGNUP-02.md](AUTH-SIGNUP-02.md) |
   | ... | ... | ... | ... | ... | ... |

   ## Module Summary

   | Module | Screens | Components | States | Wireflow Steps |
   |--------|---------|-----------|--------|---------------|
   | AUTH | 4 | 32 | 28 | 24 |
   | DASHBOARD | 1 | 45 | 12 | 18 |
   | PROFILE | 2 | 22 | 16 | 14 |
   | ADMIN | 5 | 58 | 35 | 42 |
   | VIDEO | 1 | 15 | 8 | 12 |
   | ... | ... | ... | ... | ... |

   ## Cross-Reference

   - **Source**: `docs/ux-flows/` (Step 16.1 output)
   - **Upstream**: `docs/16-ux-wireframes.md` (Step 16), `docs/14-api-design.md` (Step 14), `docs/05-data-structure.md` (Step 5)
   - **Downstream**: `/aspec-17-plan` (Step 17), `/aspec-18-tasks` (Step 18), `/aspec-19-testplan` (Step 19), `/aspec-20-implement` (Step 20)
   ```

**Guidelines:**

1. **One-to-one mapping**: Every `SCR-xxx` from Step 16.1 must map to exactly one Screen ID. No screen may be skipped.
2. **ID consistency**: Once a Screen ID is assigned, it must be used identically across all sections of the file and across all files in the output directory. Never use a bare screen name without its formal ID.
3. **Complete component coverage**: Every interactive element from Section 3.10 of the Step 16.1 file must receive a Component ID. Non-interactive elements (headings, static text, decorative images) do not need IDs unless they are test targets.
4. **Complete state coverage**: Every state from Section 3.11 of the Step 16.1 file must receive a State ID. Every wireframe state drawn in Section 3.3 must also have a State ID.
5. **Loading step enforcement**: Every API call in the Wireflow Specification MUST have an explicit LOADING step between the user action and the API response. Never skip the loading step.
6. **Confirm step enforcement**: Every destructive or irreversible action (delete, deactivate, revoke) MUST have a CONFIRM step before the LOADING step.
7. **`data-testid` convention**: Component IDs are converted to kebab-case for `data-testid` attributes. This is the canonical test selector — E2E tests and component tests must use these selectors.
8. **State management mapping**: Each State ID should map to a concrete state variable and value in the implementation. This bridges the gap between design spec and code.
9. **Flow step logging**: Wireflow Step IDs should be logged in `console.debug` calls during development for debugging and analytics. This is a recommendation, not a hard requirement.
10. **Traceability chain**: The output must maintain a complete traceability chain: Screen ID → Component IDs → State IDs → Wireflow Step IDs → API Endpoints → Test Cases. Every link in this chain must be verifiable.
11. **Pull validation rules from Step 5**: The Component-to-DTO Traceability table must include Required (✅/❌), Type, Validation rules, and Default values pulled from `docs/05-data-structure.md`. Never leave required/optional ambiguous.
12. **Cross-screen references use formal IDs**: All cross-screen navigation references must use the formal Screen ID (e.g., `DASHBOARD-HOME-01`), never bare names or `SCR-xxx` IDs.
13. **Test Coverage Matrix is mandatory**: Every output file must include a Test Coverage Matrix summarizing the count of components, states, and wireflow steps covered, with references to test file paths.
```

## Output

Save generated files under `docs/ux-flow-reqs/`:
```
docs/ux-flow-reqs/
├── README.md                        # Index with ID conventions + SCR-xxx mapping
├── AUTH-LOGIN-01.md
├── AUTH-SIGNUP-02.md
├── AUTH-FORGOT-PASSWORD-03.md
├── AUTH-TOTP-SETUP-04.md
├── DASHBOARD-HOME-01.md
├── PROFILE-VIEW-01.md
├── PROFILE-EDIT-02.md
├── WORKSPACE-SETTINGS-01.md
├── ADMIN-USER-LIST-01.md
├── ADMIN-USER-DETAIL-02.md
├── ADMIN-SETTINGS-01.md
├── ADMIN-AUDIT-01.md
├── ADMIN-CALL-HISTORY-01.md
├── VIDEO-CALL-01.md
├── NOTIFICATION-PREFERENCES-01.md
└── ...                              # One file per screen
```

## Next Steps

After completing this step, proceed to:
- `/aspec-16.3-state-diagram` — Generate Data & UI State Machine Diagrams by combining Step 10 entity lifecycles with Step 16.2 screen states (Step 16.3)
- `/aspec-17-plan` — Generate Project Scope and Phase Plan (Step 17)
- `/aspec-18-tasks` — Generate Phase Task List (Step 18)
- `/aspec-19-testplan` — Generate Test Plan (Step 19) — use Component IDs and State IDs as test selectors
- `/aspec-20-implement` — Use per-page requirements files to guide implementation with correct `data-testid` attributes and state management
