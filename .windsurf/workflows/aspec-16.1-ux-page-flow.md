---
description: Generate per-page UX sub-flow files applying Laws of UX patterns (Step 16.1)
arguments:
  send: true
---

# Per-Page UX Sub-Flow Generator

Use this workflow after completing Step 16 (UX Wireframe Flow & UI Design). It takes the Screen Inventory and Wireframe Specifications from `docs/16-ux-wireframes.md` and produces a separate UX sub-flow file for each page, grounded in the **laws-of-ux** skill.

## Usage

```
/aspec-16.1-ux-page-flow [input_file]
```

**Examples:**
- `/aspec-16.1-ux-page-flow docs/16-ux-wireframes.md`
- `/aspec-16.1-ux-page-flow` (defaults to `docs/16-ux-wireframes.md`)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains a file path, read that file as input.
If empty, read `docs/16-ux-wireframes.md`.

## Prerequisites

Ensure you have completed:
- UX Wireframe Flow & UI Design (Step 16 — `/aspec-16-ux-design`)
- API Design (Step 14 — `/aspec-14-api-design`)
- Data Structure (Step 5 — `/aspec-05-data-structure`)
- Internal Module Design (Step 13 — `/aspec-13-module-design`)
- Test Plan (Step 19 — `/aspec-19-testplan`) — optional but recommended
- The **laws-of-ux** skill must be available in `.windsurf/skills/laws-of-ux/`

**Additional input files** (auto-loaded if present):
- `docs/14-api-design.md` — for endpoint mapping per page
- `docs/05-data-structure.md` — for data model references
- `docs/13-module-design.md` — for file/folder mapping
- `docs/19-test-plan.md` — for acceptance criteria

## Prompt

```
You are a UX specialist who applies cognitive psychology principles to interface design. Your job is to read the UX Wireframe document (Step 16) and generate one dedicated UX sub-flow file per page/screen.

**Reference Skill:** laws-of-ux (read `.windsurf/skills/laws-of-ux/skill.md` for the full set of laws)

**Input Documents:**
- `docs/16-ux-wireframes.md` — contains Information Architecture, Screen Inventory, Wireframe Specifications, User Flow Diagrams, and Interaction Patterns.
- `docs/14-api-design.md` — API endpoints, request/response schemas (read if present).
- `docs/05-data-structure.md` — Data models, DTOs, enums (read if present).
- `docs/13-module-design.md` — Module breakdown, file/folder structure (read if present).
- `docs/19-test-plan.md` — Acceptance criteria per feature (read if present).

---

### Step-by-step Instructions

1. **Parse the Screen Inventory** from the input document. Extract every screen listed in the Screen Inventory table (SCR-xxx entries). Each screen becomes its own output file.

2. **For each screen**, create a file at:
   ```
   docs/ux-flows/<screen-id>-<screen-name-kebab>.md
   ```
   Example: `docs/ux-flows/SCR-001-login.md`, `docs/ux-flows/SCR-002-dashboard.md`

3. **Each per-page file must contain ALL of the following sections (3.1–3.15).** The goal is that a developer can implement the entire page from this single file without needing to cross-reference other documents.

   ### 3.1 Page Overview
   ```markdown
   # UX Sub-Flow: <Screen Name> (<Screen ID>)

   | Field | Value |
   |-------|-------|
   | Screen ID | SCR-xxx |
   | Screen Name | ... |
   | Purpose | ... (from Screen Inventory) |
   | Actor(s) | ... |
   | Related Use Cases | UC-xxx, UC-yyy |
   | Parent Flow | (link to the user flow diagram this screen belongs to) |
   | Route / URL | `/login`, `/dashboard`, `/users/:id/edit` |
   ```

   ### 3.2 Navigation Context
   Where the user comes from, where they can go, and breadcrumb structure.
   ```markdown
   #### Navigation
   | Direction | Screen | Condition |
   |-----------|--------|-----------|
   | Incoming from | Landing Page (SCR-000) | User clicks "Sign In" |
   | Incoming from | Register (SCR-003) | User clicks "Already have an account?" |
   | Outgoing to | Dashboard (SCR-002) | Successful login |
   | Outgoing to | Password Reset (SCR-004) | User clicks "Forgot password?" |

   #### Breadcrumb
   Home > Login (if applicable, or "none" for top-level pages)
   ```

   ### 3.3 Wireframe Layout
   Carry forward the ASCII wireframe from Step 16 and refine it for this page. This is the visual blueprint the developer will code against. **Mark required fields with `*` directly in the wireframe** so developers can see at a glance which inputs are mandatory.

   > **Wireframe State Naming Requirement:** Each distinct visual state of this page (e.g., Default, Loading, Error, Empty, Success) **MUST be given an explicit named state label** in this section. These state names are the canonical identifiers used across Section 3.6.1 (cross-screen wireflow nodes), Section 3.6.2 (in-page micro-flow nodes), Section 3.6.3 (mapping table), and Section 3.11 (interaction states table). Every wireframe state drawn here must appear in Section 3.11, and every flow node in Sections 3.6.1–3.6.2 must reference one of these named states.

   Use `*` after the field label for required fields. Draw a separate ASCII wireframe block for each named state that has a meaningfully different visual layout. Add a legend below each wireframe.
   ```
   === State: Default State ===
   Screen: Login (SCR-001)

   ┌─────────────────────────────────┐
   │           [Logo]                 │
   ├─────────────────────────────────┤
   │  ┌─────────────────────────┐    │
   │  │ Email *                 │    │
   │  └─────────────────────────┘    │
   │  ┌─────────────────────────┐    │
   │  │ Password *          [👁]│    │
   │  └─────────────────────────┘    │
   │  [ ] Remember me                │
   │  ┌─────────────────────────┐    │
   │  │      Sign in            │    │
   │  └─────────────────────────┘    │
   │  Forgot password? | Sign up     │
   └─────────────────────────────────┘

   === State: Loading State ===
   Screen: Login (SCR-001) — while POST /auth/login is in-flight

   ┌─────────────────────────────────┐
   │           [Logo]                 │
   ├─────────────────────────────────┤
   │  ┌─────────────────────────┐    │
   │  │ Email * [disabled]      │    │
   │  └─────────────────────────┘    │
   │  ┌─────────────────────────┐    │
   │  │ Password * [disabled]   │    │
   │  └─────────────────────────┘    │
   │  [ ] Remember me [disabled]     │
   │  ┌─────────────────────────┐    │
   │  │  ⟳ Signing in...        │    │  ← spinner + changed text
   │  └─────────────────────────┘    │
   └─────────────────────────────────┘

   === State: Error State ===
   Screen: Login (SCR-001) — after 401/429/5xx response

   ┌─────────────────────────────────┐
   │  ⚠ Invalid email or password    │  ← error alert banner
   ├─────────────────────────────────┤
   │  ┌─────────────────────────┐    │
   │  │ Email *                 │    │
   │  └─────────────────────────┘    │
   │  ┌─────────────────────────┐    │
   │  │ Password *          [👁]│    │
   │  └─────────────────────────┘    │
   │  ┌─────────────────────────┐    │
   │  │      Sign in            │    │
   │  └─────────────────────────┘    │
   └─────────────────────────────────┘

   Legend: * = Required field (from docs/05-data-structure.md)
   Wireframe States defined here: Default State | Loading State | Error State
   (All states must appear in Section 3.11 and be referenced in Section 3.6 flow nodes)
   ```
   Include annotations for spacing, alignment, and visual hierarchy.

   ### 3.4 Design Tokens & Styling
   Page-specific design token usage. Reference the project's design token system.
   ```markdown
   | Element | Token / Value | Notes |
   |---------|---------------|-------|
   | Page background | `--color-surface-primary` / `#FFFFFF` | |
   | Card background | `--color-surface-secondary` / `#F9FAFB` | Centered, max-width 420px |
   | Card border-radius | `--radius-lg` / `12px` | |
   | Card padding | `--spacing-6` / `24px` | |
   | Card shadow | `--shadow-md` | |
   | Heading font | `--font-heading` / Inter 600 24px | "Welcome back" |
   | Body font | `--font-body` / Inter 400 14px | Labels, links |
   | Primary button | `--color-primary` / `#2563EB` | Full-width, height 44px |
   | Error text | `--color-error` / `#DC2626` | 12px, below input |
   | Spacing between inputs | `--spacing-4` / `16px` | |
   | Spacing card to CTA | `--spacing-6` / `24px` | |
   ```
   If the project uses Tailwind, also provide the Tailwind class equivalents (e.g., `bg-white`, `rounded-xl`, `shadow-md`, `p-6`).

   ### 3.5 Content & Copy
   Every text string on the page — headings, labels, placeholders, button text, links, tooltips, error messages, success messages. This is the single source of truth for copy.
   ```markdown
   | Element | Text | Notes |
   |---------|------|-------|
   | Page title (meta) | "Login — AppName" | Browser tab title |
   | Heading | "Welcome back" | |
   | Subheading | "Sign in to your account" | |
   | Email label | "Email address" | |
   | Email placeholder | "you@example.com" | |
   | Password label | "Password" | |
   | Password placeholder | "Enter your password" | |
   | Show/hide toggle | "Show" / "Hide" | aria-label for toggle |
   | Remember me | "Remember me for 30 days" | |
   | Login button | "Sign in" | |
   | Forgot link | "Forgot password?" | |
   | Register link | "Don't have an account? Sign up" | |
   | Error: empty email | "Email is required" | |
   | Error: invalid email | "Please enter a valid email address" | |
   | Error: empty password | "Password is required" | |
   | Error: wrong credentials | "Invalid email or password. Please try again." | |
   | Error: rate limited | "Too many attempts. Please try again in {n} minutes." | |
   | Error: server error | "Something went wrong. Please try again later." | |
   | Success: (toast/redirect) | Redirect to Dashboard — no visible message | |
   | Loading: button text | "Signing in..." | Or spinner icon |
   ```

   ### 3.6 User Flow Within Page
   A Mermaid flowchart showing the micro-interactions within this single page — what happens when the user lands, interacts with each element, submits, encounters errors, etc. **All cross-screen navigation nodes MUST use `SCR-xxx` screen IDs** from the Screen Inventory so flows are traceable.

   #### 3.6.1 Cross-Screen Wireflow
   Show how this screen connects to other screens. **Every node in this diagram MUST carry a wireframe state annotation** using the mandatory format below — no node may appear without one.

   > **Mandatory Node Format:** `SCR-xxx: Screen Name\n🖼 <State Name>`
   > - `SCR-xxx` = Screen ID from the Screen Inventory
   > - `Screen Name` = human-readable name
   > - `<State Name>` = the exact named wireframe state from Section 3.3 (e.g., `Default State`, `Loading State`, `Error State`). This creates a direct, traceable link from every flow step to a drawn wireframe.
   > - The current screen node must be highlighted (e.g., `style SCR001 fill:#2563EB,...`).
   > - **Never use a bare screen name or SCR-xxx without a wireframe state.** If the target screen's state is unknown, write `🖼 Default State`.

   ```mermaid
   flowchart LR
       SCR000["SCR-000: Landing Page\n🖼 Default State"] -->|Click 'Sign In'| SCR001["SCR-001: Login\n🖼 Default State"]
       SCR003["SCR-003: Register\n🖼 Default State"] -->|Click 'Already have an account?'| SCR001
       SCR001 -->|Successful login| SCR002["SCR-002: Dashboard\n🖼 Default State"]
       SCR001 -->|Click 'Forgot password?'| SCR004["SCR-004: Password Reset\n🖼 Default State"]
       SCR001 -->|Click 'Sign up'| SCR003

       style SCR001 fill:#2563EB,color:#fff,stroke:#1e40af
   ```

   #### 3.6.2 In-Page Micro-Flow
   Show micro-interactions within this page. **Every API call must include explicit loading and confirm/result steps.** Use distinct node shapes: `([loading])` for loading states, `[[confirm]]` for confirmation dialogs, `{decision}` for API responses.

   > **Wireframe State Annotation Requirement:** Every action/state node in this diagram **MUST** include a `🖼 <State Name>` annotation on a second line inside the node label, referencing the exact wireframe state name defined in Section 3.3. This makes every step of the flow directly traceable to a drawn wireframe. Decision nodes (`{}`) and API call nodes do not require a wireframe annotation. Format: `["Node description\n🖼 State Name"]`.

   > **Loading Step Requirement:** Every operation that involves loading data from an API or confirming an operation (such as fetching a list, creating an item, updating a record, or deleting) **MUST** show an explicit loading/submitting state between the user action and the API response. Never skip the loading step. This applies to:
   > - **Page load / initial data fetch** (e.g., fetching a list on mount): show a skeleton or spinner state before data renders.
   > - **User-triggered fetch** (e.g., search, filter, pagination): show a loading indicator while the request is in-flight.
   > - **Create / Update operations**: disable the submit button and show a submitting state (spinner + changed button text) while the API call is pending.
   > - **Confirm operations** (e.g., "Are you sure?" dialogs before delete or irreversible actions): show a confirmation dialog first, then a loading state after the user confirms.

   **API operation patterns to follow:**
   - **Fetch/Read on page load** (GET — initial load): `Page Load` → `🔄 Loading State (skeleton/spinner)` → `API Call` → `{Response?}` → `✅ Render Data` or `❌ Error State`
   - **Fetch/Read on user action** (GET — search/filter/paginate): `User Action` → `🔄 Loading State (inline spinner or skeleton)` → `API Call` → `{Response?}` → `✅ Render Data` or `❌ Error State`
   - **Create/Update** (POST/PUT): `User Action` → `Client Validation` → `🔄 Submitting State (button disabled + spinner)` → `API Call` → `{Response?}` → `✅ Success Feedback` or `❌ Error`
   - **Delete/Destructive**: `User Action` → `⚠️ Confirm Dialog` → `🔄 Deleting State` → `API Call` → `{Response?}` → `✅ Success` or `❌ Error`

   ```mermaid
   flowchart TD
       A["📄 Page Load\n🖼 SCR-001: Default State"] --> B["Render Empty Form\n🖼 Default State"]
       B --> C{User Action?}
       C -->|Fill Form| D["Validate Inline on Blur\n🖼 Field Focus State"]
       D --> C
       C -->|Click 'Sign in'| E{Client Validation OK?}
       E -->|No| F["Show Inline Errors\n🖼 Inline Error State"]
       F --> C
       E -->|Yes| G["🔄 Set Loading State\n🖼 Loading State\nButton: 'Signing in...' + spinner\nInputs: disabled"]
       G --> H[Call POST /auth/login]
       H --> I{API Response?}
       I -->|200 OK| J["✅ Store Token\nRedirect to SCR-002: Dashboard"]
       I -->|401| K["❌ Show Error\n🖼 Error State\n'Invalid email or password'"]
       I -->|429| L["❌ Show Rate Limit\n🖼 Error State\n'Too many attempts'"]
       I -->|5xx| M["❌ Show Server Error\n🖼 Error State\n'Something went wrong'"]
       K --> C
       L --> C
       M --> C
   ```

   #### 3.6.3 Wireframe-to-Flow Mapping Table
   **This table is mandatory for every page.** It is the explicit bridge between the flow diagram (Section 3.6.2) and the wireframe states (Section 3.3 and 3.11). Every non-trivial node from Section 3.6.2 must appear as a row here. The `Wireframe State` column must use the exact state name defined in Section 3.3 — no paraphrasing.

   > **Completeness Rule:** The set of unique `Wireframe State` values in this table must exactly match the named states defined in Section 3.3 and the `State` column in Section 3.11. If a flow step maps to a state not yet defined in Section 3.3, add it there first, then reference it here.

   ```markdown
   | Flow Step | Wireframe State (from 3.3 & 3.11) | What User Sees | API Involved? |
   |-----------|-----------------------------------|----------------|---------------|
   | Page Load | Default State | Empty form, button enabled | No |
   | Fill Form | Field Focus State | Blue border on active input, label floats | No |
   | Validation Fail | Inline Error State | Red border, error text below field | No |
   | Click 'Sign in' | Loading State | Button shows spinner + "Signing in...", inputs disabled | Yes — POST /auth/login |
   | 200 OK | — (redirect to SCR-002) | Redirect to SCR-002: Dashboard | — |
   | 401 Error | Error State | Alert banner: "Invalid email or password" | — |
   | 429 Error | Error State | Alert banner: "Too many attempts" | — |
   | 5xx Error | Error State | Alert banner: "Something went wrong" | — |
   ```

   ### 3.7 API Endpoint Mapping
   List every API call this page makes, with request/response shapes from `docs/14-api-design.md`.
   ```markdown
   #### Endpoints Used

   | Action | Method | Endpoint | Request Body | Success Response | Error Responses |
   |--------|--------|----------|--------------|------------------|-----------------|
   | Login | POST | `/auth/login` | `{ email: string, password: string, rememberMe?: boolean }` | `{ token: string, user: User }` | 401, 422, 429, 500 |

   #### Request Example
   ```json
   POST /auth/login
   Content-Type: application/json

   {
     "email": "user@example.com",
     "password": "securePassword123",
     "rememberMe": true
   }
   ```

   #### Response Example (200)
   ```json
   {
     "data": {
       "token": "eyJhbG...",
       "refreshToken": "eyJhbG...",
       "user": {
         "id": "uuid",
         "email": "user@example.com",
         "name": "John Doe"
       }
     }
   }
   ```

   #### Error Response Example (401)
   ```json
   {
     "error": "UNAUTHORIZED",
     "message": "Invalid email or password"
   }
   ```
   ```

   ### 3.8 Data Model References & Field Requirements
   Which entities, DTOs, and enums from `docs/04-data-model.md` and `docs/05-data-structure.md` this page reads or writes. **Every field must show its required/optional status and validation rules from the data design.**

   ```markdown
   #### DTOs Used

   | DTO | Usage | Field | Type | Required | Validation (from Step 5) | Default |
   |-----|-------|-------|------|----------|--------------------------|----------|
   | LoginRequestDTO | Request body | email | string | ✅ Yes | email format, max 255 chars | — |
   | LoginRequestDTO | Request body | password | string | ✅ Yes | min 8, max 128 chars | — |
   | LoginRequestDTO | Request body | rememberMe | boolean | ❌ No | — | false |
   | AuthResponseDTO | Response | token | string | ✅ Yes | JWT format | — |
   | AuthResponseDTO | Response | refreshToken | string | ✅ Yes | JWT format | — |
   | AuthResponseDTO | Response | user | User | ✅ Yes | — | — |

   #### Entity Fields Displayed

   | Entity | Field | Type | Required | Constraints (from Step 4) | Shown As |
   |--------|-------|------|----------|---------------------------|----------|
   | User | id | string (UUID) | ✅ Yes | PK, auto-generated | Not displayed |
   | User | email | string | ✅ Yes | unique, max 255 | Pre-filled after login |
   | User | name | string | ✅ Yes | max 100 | Displayed in dashboard greeting |
   ```

   ### 3.9 Laws of UX Applied
   For each law that is relevant to this specific page, provide:
   - **Law name**
   - **How it applies** to this page specifically
   - **Concrete recommendation** (what to build / what to avoid)

   Select only the laws that are genuinely relevant. Typical coverage per page is 4–8 laws. Organize them by priority of impact.

   Example format:
   ```markdown
   #### Applied Laws

   | # | Law | Application | Recommendation |
   |---|-----|-------------|----------------|
   | 1 | Hick's Law | Login has only 2 fields + 1 CTA — minimal choices | Keep form minimal. Do not add social login options unless validated by user research. |
   | 2 | Fitts's Law | Login button is the primary target | Make button full-width on mobile (min 48px height), place directly below last input. |
   | 3 | Doherty Threshold | Auth request may take > 400ms | Show loading spinner on button immediately on click. Use optimistic disable to prevent double-submit. |
   | 4 | Peak-End Rule | Login is often the first and last touchpoint | Ensure error messages are helpful (not "Invalid credentials"). On success, transition smoothly to dashboard. |
   | 5 | Aesthetic-Usability Effect | Login is the first impression | Invest in visual polish — centered card, brand colors, clean typography. |
   ```

   ### 3.10 Component Inventory
   List every UI component on this page with its states, props, data binding, and the UX law that justifies its design. **Form fields must include the Required and Validation columns from data design.**
   ```markdown
   | Component | Props / Config | DTO Field | Required | Validation | Variants/States | UX Law Justification |
   |-----------|---------------|-----------|----------|------------|----------------|----------------------|
   | TextInput | `type="email"` `autoComplete="email"` `inputMode="email"` | LoginRequestDTO.email | ✅ Yes | email format, max 255 | default, focus, error, filled | Postel's Law — accept varied formats |
   | TextInput | `type="password"` `autoComplete="current-password"` | LoginRequestDTO.password | ✅ Yes | min 8, max 128 | default, focus, error, show/hide | Tesler's Law — absorb complexity |
   | Checkbox | `label="Remember me for 30 days"` | LoginRequestDTO.rememberMe | ❌ No | — | unchecked, checked | Paradox of Active User — optional, not blocking |
   | Button | `variant="primary"` `fullWidth` `minHeight="44px"` | — | — | — | default, hover, loading, disabled | Fitts's Law — large target; Doherty — loading state |
   | Alert | `variant="error"` | — | — | — | hidden, visible | Von Restorff — visually distinct from form |
   | Link | `href="/forgot-password"` | — | — | — | default, hover, focus | Serial Position — placed at bottom |
   ```

   ### 3.11 Interaction States & Visual Descriptions
   Document every state this page can be in, with a description of what the user sees. **Every page that loads data from an API or performs a create/update/delete operation MUST include at least one loading state row** (e.g., "Loading", "Submitting", "Deleting") in this table. Loading states must describe what the user sees during the in-flight period (skeleton, spinner, disabled button, changed button text, etc.).
   ```markdown
   | State | Trigger | What the User Sees | Exit Condition |
   |-------|---------|-------------------|----------------|
   | Default | Page load | Empty form, "Sign in" button enabled, no errors | User interacts |
   | Focused | Input focus | Blue border on active input, label floats up | User moves to next field |
   | Inline Error | Field blur with invalid value | Red border, error text below field (e.g., "Email is required") | User corrects input |
   | Submitting | CTA click | Button text changes to "Signing in..." with spinner, inputs disabled | API responds |
   | API Error | 401/422 response | Red alert banner above form: "Invalid email or password. Please try again." Button re-enabled. | User corrects & resubmits |
   | Rate Limited | 429 response | Alert: "Too many attempts. Please try again in 5 minutes." Button disabled with countdown. | Timer expires |
   | Server Error | 5xx response | Alert: "Something went wrong. Please try again later." Retry button shown. | User retries |
   | Success | 200 response | Brief fade transition, redirect to Dashboard (SCR-002) | Navigation complete |
   | Empty State | N/A | Not applicable for login | — |
   ```

   ### 3.12 Accessibility Notes
   Page-specific accessibility requirements:
   ```markdown
   #### Keyboard Navigation (Tab Order)
   1. Skip link (if present)
   2. Email input
   3. Password input
   4. Show/hide password toggle
   5. Remember me checkbox
   6. Sign in button
   7. Forgot password link
   8. Sign up link

   #### ARIA
   | Element | ARIA Attribute | Value |
   |---------|---------------|-------|
   | Form | `role` | `form` |
   | Email input | `aria-required` | `true` |
   | Email input (error) | `aria-invalid` | `true` |
   | Email input (error) | `aria-describedby` | `email-error` |
   | Error message | `role` | `alert` |
   | Error banner | `role` | `alert` + `aria-live="polite"` |
   | Password toggle | `aria-label` | `"Show password"` / `"Hide password"` |
   | Loading button | `aria-busy` | `true` |

   #### Focus Management
   - On page load: auto-focus email input
   - On validation error: focus first invalid field
   - On API error: focus error banner
   - On success: no focus change (page navigates away)

   #### Color Contrast
   - All text meets WCAG AA 4.5:1 ratio
   - Error red (#DC2626) on white background: 4.63:1 ✅
   - Primary button text (white on #2563EB): 4.56:1 ✅
   ```

   ### 3.13 Responsive Behavior
   How this page adapts across breakpoints:
   ```markdown
   | Breakpoint | Layout Change | Key Differences |
   |------------|---------------|-----------------|
   | Mobile (< 640px) | Single column, full-width card (no margin), full-width inputs & button | Logo smaller (32px), no background illustration |
   | Tablet (640–1024px) | Centered card (max-width 420px), 24px margin | Logo 40px |
   | Desktop (> 1024px) | Centered card with split layout — illustration left, form right | Logo 48px, background illustration visible |
   ```

   ### 3.14 Implementation File Mapping
   Where the source files for this page should live, based on `docs/13-module-design.md`.
   ```markdown
   | File | Path | Purpose |
   |------|------|---------|
   | Page component | `src/pages/LoginPage.tsx` | Route-level page component |
   | Form component | `src/components/auth/LoginForm.tsx` | Reusable login form |
   | Form hook | `src/hooks/useLoginForm.ts` | Form state, validation, submission |
   | API call | `src/services/auth.service.ts` | `login()` function calling POST /auth/login |
   | Types | `src/types/auth.ts` | LoginRequestDTO, AuthResponseDTO |
   | Tests | `src/pages/__tests__/LoginPage.test.tsx` | Component + integration tests |
   | E2E test | `tests/e2e/login.spec.ts` | End-to-end login flow |
   ```

   ### 3.15 Acceptance Criteria
   Testable conditions derived from `docs/19-test-plan.md` and the use cases. A developer knows the page is "done" when all of these pass.
   ```markdown
   - [ ] User can enter email and password and submit the form
   - [ ] Empty fields show inline validation errors on blur
   - [ ] Invalid email format shows "Please enter a valid email address"
   - [ ] Successful login stores token and redirects to Dashboard
   - [ ] Wrong credentials show error banner without clearing password field
   - [ ] Rate-limited response shows countdown message
   - [ ] Server error shows generic error with retry option
   - [ ] All interactive elements are keyboard accessible in correct tab order
   - [ ] Screen reader announces errors via aria-live region
   - [ ] Page renders correctly on mobile, tablet, and desktop
   - [ ] Loading state prevents double-submission
   - [ ] "Remember me" persists session for 30 days when checked
   - [ ] A loading indicator (skeleton or spinner) is shown while fetching data from any API endpoint on page load or user-triggered fetch
   - [ ] Submit/confirm buttons are disabled and show a loading indicator (spinner + changed text) while a create or update API call is in-flight
   - [ ] Confirmation dialogs are shown before any destructive or irreversible API operation, and a loading state is shown after the user confirms
   - [ ] All micro-interactions and transitions match the animation specs (duration, easing, properties)
   ```

   ### 3.16 Micro-Interaction & Animation Specs
   Document every animation, transition, and micro-interaction on this page. This ensures developers implement consistent motion design and testers can verify timing behavior.

   ```markdown
   #### Transition Definitions

   | Element | Trigger | Property | Duration | Easing | CSS / Tailwind | Notes |
   |---------|---------|----------|----------|--------|----------------|-------|
   | Input border | Focus | `border-color` | 150ms | `ease-in-out` | `transition-colors duration-150` | Blue border on focus |
   | Input border | Error | `border-color` | 150ms | `ease-in-out` | `transition-colors duration-150` | Red border on validation error |
   | Error text | Appear | `opacity`, `max-height` | 200ms | `ease-out` | `transition-all duration-200` | Slide-down + fade-in below input |
   | Error text | Disappear | `opacity`, `max-height` | 150ms | `ease-in` | `transition-all duration-150` | Collapse + fade-out |
   | Button | Hover | `background-color` | 150ms | `ease-in-out` | `transition-colors duration-150` | Darken by 10% |
   | Button | Click → Loading | `width`, `content` | 200ms | `ease-out` | `transition-all duration-200` | Text changes to spinner + "Signing in..." |
   | Button | Loading → Idle | `width`, `content` | 200ms | `ease-in` | `transition-all duration-200` | Restore original text |
   | Error banner | Appear | `opacity`, `transform` | 300ms | `ease-out` | `transition-all duration-300` | Slide-down from top + fade-in |
   | Error banner | Disappear | `opacity`, `transform` | 200ms | `ease-in` | `transition-all duration-200` | Fade-out + slide-up |
   | Page | Success redirect | `opacity` | 200ms | `ease-out` | `transition-opacity duration-200` | Fade-out before navigation |
   | Skeleton loader | Pulse | `opacity` | 1500ms | `ease-in-out` | `animate-pulse` | Repeating pulse while loading |
   | Toast notification | Appear | `opacity`, `transform` | 300ms | `cubic-bezier(0.4, 0, 0.2, 1)` | `transition-all duration-300` | Slide-in from right + fade-in |
   | Toast notification | Disappear | `opacity`, `transform` | 200ms | `ease-in` | `transition-all duration-200` | Auto-dismiss after 5s |
   | Modal overlay | Open | `opacity` | 200ms | `ease-out` | `transition-opacity duration-200` | Background dim fade-in |
   | Modal content | Open | `opacity`, `transform` | 300ms | `cubic-bezier(0.4, 0, 0.2, 1)` | `transition-all duration-300` | Scale from 95% + fade-in |
   | Modal content | Close | `opacity`, `transform` | 200ms | `ease-in` | `transition-all duration-200` | Scale to 95% + fade-out |

   #### Motion Design Principles

   | Principle | Rule | Rationale |
   |-----------|------|-----------|
   | Duration range | 100ms–500ms for UI transitions; never > 500ms | Doherty Threshold — response within 400ms feels instant |
   | Easing default | `ease-in-out` for state changes; `ease-out` for entrances; `ease-in` for exits | Natural motion perception |
   | Reduced motion | Respect `prefers-reduced-motion: reduce` — disable all non-essential animations | WCAG 2.3.3 compliance |
   | Loading indicators | Appear immediately (0ms delay); minimum display 300ms to avoid flash | Prevents jarring flash for fast responses |
   | Skeleton loaders | Use for initial page data fetch > 200ms; use spinner for user-triggered actions | Skeleton preserves layout; spinner signals action |
   | Focus ring | `outline: 2px solid var(--color-primary)` with `outline-offset: 2px`; transition 150ms | Keyboard accessibility |

   #### Reduced Motion Override

   ```css
   @media (prefers-reduced-motion: reduce) {
     *, *::before, *::after {
       animation-duration: 0.01ms !important;
       animation-iteration-count: 1 !important;
       transition-duration: 0.01ms !important;
     }
   }
   ```
   Or with Tailwind: apply `motion-reduce:transition-none motion-reduce:animate-none` to animated elements.
   ```

   Select only the transitions relevant to this specific page from the master table above. Add page-specific animations (e.g., drag-and-drop, swipe gestures, parallax) if applicable.

   ### 3.17 Component Hierarchy Tree
   Show the parent→child nesting of all components on this page as an indented tree. This tells developers exactly how to compose the component structure and which components own which children.

   ```markdown
   #### Component Tree

   ```
   LoginPage (src/pages/LoginPage.tsx)
   └── LoginForm (src/components/auth/LoginForm.tsx)
       ├── Logo (src/components/ui/Logo.tsx)
       ├── Heading — "Welcome back"
       ├── TextInput [email] — EMAIL field
       │   └── InlineError — "Email is required"
       ├── TextInput [password] — PASSWORD field
       │   ├── PasswordToggle — Show/Hide
       │   └── InlineError — "Password is required"
       ├── Checkbox — "Remember me for 30 days"
       ├── Button [submit] — "Sign in" / "Signing in..."
       │   └── Spinner (conditional — loading state)
       ├── AlertBanner (conditional — API error)
       └── LinkGroup
           ├── Link — "Forgot password?" → SCR-004
           └── Link — "Don't have an account? Sign up" → SCR-003
   ```

   #### Hierarchy Rules
   | Rule | Description |
   |------|-------------|
   | Page wraps Form | The page component handles routing, auth guards, and layout. The form component handles state and submission. |
   | Form owns Inputs | All form fields are children of the form component, not siblings. |
   | Inline errors are children of their input | Each input component renders its own validation error as a child element. |
   | Conditional components | Components that appear/disappear based on state (Spinner, AlertBanner) are rendered conditionally within their parent. |
   | Shared components | Components from the UI library (Button, TextInput, Checkbox, Alert) are imported, not inlined. |
   ```

   Build the tree from the Component Inventory (Section 3.10) and the Wireframe Layout (Section 3.3). Every component in Section 3.10 must appear in the tree. Include the source file path for custom components.

4. **Create an index file** at `docs/ux-flows/README.md`:
   ```markdown
   # UX Page Flows Index

   Per-page UX sub-flow specifications generated from `docs/16-ux-wireframes.md`.
   Each file applies Laws of UX to a specific screen.

   | Screen ID | Screen Name | File | Route | API Endpoints | Key UX Laws Applied |
   |-----------|-------------|------|-------|---------------|---------------------|
   | SCR-001 | Login | [SCR-001-login.md](SCR-001-login.md) | `/login` | POST /auth/login | Hick's, Fitts's, Peak-End |
   | SCR-002 | Dashboard | [SCR-002-dashboard.md](SCR-002-dashboard.md) | `/dashboard` | GET /users/me, GET /stats | Miller's, Serial Position, Pareto |
   | ... | ... | ... | ... | ... | ... |
   ```

**Guidelines:**
1. Only apply laws that are genuinely relevant to the page — do not force-fit all 30 laws.
2. Recommendations must be concrete and actionable (not generic advice).
3. Maintain traceability: every screen links back to its Use Case IDs and the parent user flow.
4. Use Mermaid for all flow diagrams.
5. Keep each file self-contained — a developer should be able to implement the page from this file alone, including: layout, tokens, copy, API calls, data types, file paths, and acceptance criteria.
6. If the Screen Inventory from Step 16 is missing or incomplete, list the screens you can infer from the Information Architecture and User Flow Diagrams, and flag any gaps.
7. **Design tokens**: Use CSS custom property names (e.g., `--color-primary`) with fallback hex values. If the project uses Tailwind, include Tailwind class equivalents.
8. **Copy table must be exhaustive** — every user-visible string including all error messages, loading text, empty states, and tooltips. This prevents developers from inventing copy during implementation.
9. **API mapping must include request/response JSON examples** — not just endpoint names. Pull exact shapes from `docs/14-api-design.md`.
10. **File mapping must follow the module structure** from `docs/13-module-design.md` — page component, sub-components, hooks, service calls, types, and test files.
11. **Acceptance criteria must be checkbox-style** — each item is a testable condition that can be verified in code review or QA.
12. **Data design fields are mandatory** — Section 3.8 must list every DTO field with Required (✅/❌), Type, Validation rules, and Default values pulled from `docs/04-data-model.md` and `docs/05-data-structure.md`. Section 3.10 Component Inventory must bind each form field to its DTO field with the same Required and Validation columns. Never leave required/optional ambiguous.
13. **Screen number references in all wireflows** — Every Mermaid diagram that references another screen MUST use `SCR-xxx: Screen Name` as the node label (e.g., `SCR001[SCR-001: Login]`). This applies to Section 3.6.1 Cross-Screen Wireflow, Section 3.6.2 In-Page Micro-Flow (for redirect/navigation nodes), and Section 3.2 Navigation Context. Never use bare screen names without their `SCR-xxx` ID.
14. **Required fields in wireframes** — Section 3.3 ASCII wireframes must mark required fields with `*` after the field label (e.g., `Email *`). Include a legend: `* = Required field (from docs/05-data-structure.md)`. Required/optional must match Section 3.8 Data Model References.
15. **Wireframe-screen-to-wireflow-step mapping (mandatory end-to-end traceability)** — Every wireframe state must be named in Section 3.3 (draw a separate ASCII block per state), listed in Section 3.11, annotated on every action/state node in Sections 3.6.1 and 3.6.2 using the format `🖼 <State Name>`, and cross-referenced in the Section 3.6.3 mapping table. The mapping table is not optional — it must cover every non-trivial flow node. The set of state names must be consistent across all four locations (3.3, 3.6.1, 3.6.2, 3.6.3, 3.11). Never reference a wireframe state that is not drawn in Section 3.3, and never draw a wireframe state that is not referenced in the flow.
16. **Loading and confirm steps for API operations** — Section 3.6.2 In-Page Micro-Flow must show explicit loading/submitting/deleting states for every API call, and confirmation dialogs for destructive operations. Follow the patterns: Fetch (page load or user-triggered) → `🔄 Loading (skeleton/spinner)` → Response; Create/Update → Validation → `🔄 Submitting (button disabled + spinner)` → Response; Delete → `⚠️ Confirm Dialog` → `🔄 Deleting` → Response. **Never jump directly from user action to API response without a loading step.** Section 3.11 must include at least one loading state row for every page that calls an API. Section 3.15 must include acceptance criteria verifying the loading state is visible during fetch, create, and confirm operations.
```

## Output

Save generated files under `docs/ux-flows/`:
```
docs/ux-flows/
├── README.md                    # Index of all page flows
├── SCR-001-login.md
├── SCR-002-dashboard.md
├── SCR-003-register.md
├── SCR-004-profile.md
└── ...                          # One file per screen
```

## Next Steps

After completing this step, proceed to:
- `/aspec-16.2-ux-wireframe-flow-req` - Generate Wireframe Flow Requirements with formal Screen, Component, State, and Wireflow Step IDs (Step 16.2)
- `/aspec-17-plan` - Generate Project Scope and Phase Plan (Step 17)
- Or use individual page flow files during `/aspec-20-implement` to guide frontend implementation per page.
