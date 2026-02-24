---
description: Generate UX Wireframe Flow and UI Design specifications (Step 16)
arguments:
  send: true
---

# UX Wireframe Flow & UI Design Generator

Use this workflow after completing Step 15 (Project AGENTS.md).

## Usage

```
/aspec-16-ux-design [input_files]
```

**Examples:**
- `/aspec-16-ux-design docs/02-user-stories.md docs/03-use-cases.md docs/06-actor-list.md docs/08-action-function-table.md`
- `/aspec-16-ux-design` (will look for files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths, read those files as input documents.
If empty, look for `docs/02-user-stories.md`, `docs/03-use-cases.md`, `docs/04-data-model.md`, `docs/05-data-structure.md`, `docs/06-actor-list.md`, and `docs/08-action-function-table.md`.

## Prerequisites

Ensure you have completed:
- User Stories (Step 2)
- Use Cases (Step 3)
- Data Model (Step 4)
- Data Structure (Step 5)
- Actor List (Step 6)
- Action-Function Table (Step 8)

## Prompt

```
You are a UX designer. Create UX Wireframe Flows and UI Design specifications based on the user stories and use cases.

**Input Documents:**
- User Stories (Step 2)
- Use Cases (Step 3)
- Data Model (Step 4) — entity attributes, constraints, relationships
- Data Structure (Step 5) — validation rules, required/optional fields, DTOs
- Actor List (Step 6)
- Action-Function Table (Step 8)

**Output Requirements:**

1. **Information Architecture**
   ```
   App Structure:
   ├── Public Pages
   │   ├── Landing Page
   │   ├── Login
   │   ├── Register
   │   └── Password Reset
   ├── Authenticated Pages
   │   ├── Dashboard
   │   ├── Profile
   │   │   ├── View Profile
   │   │   └── Edit Profile
   │   └── [Feature Areas]
   └── Admin Pages
       ├── User Management
       └── Settings
   ```

2. **User Flow Diagrams** (Mermaid format)
   For each major use case:
   ```mermaid
   flowchart TD
       A[Landing Page] --> B{Logged In?}
       B -->|No| C[Login Page]
       B -->|Yes| D[Dashboard]
       C --> E[Enter Credentials]
       E --> F{Valid?}
       F -->|Yes| D
       F -->|No| G[Show Error]
       G --> E
   ```

3. **Screen Inventory**
   | Screen ID | Screen Name | Purpose | Actor(s) | Related UC |
   |-----------|-------------|---------|----------|------------|
   | SCR-001 | Login | User authentication | Guest | UC-001 |
   | SCR-002 | Dashboard | Main user interface | User | UC-002 |

4. **Wireframe Specifications**
   For each key screen:
   ```
   Screen: Login (SCR-001)
   
   Layout:
   ┌─────────────────────────────────┐
   │           Logo                   │
   ├─────────────────────────────────┤
   │  ┌─────────────────────────┐    │
   │  │ Email                   │    │
   │  └─────────────────────────┘    │
   │  ┌─────────────────────────┐    │
   │  │ Password                │    │
   │  └─────────────────────────┘    │
   │  [ ] Remember me                 │
   │  ┌─────────────────────────┐    │
   │  │      Login Button       │    │
   │  └─────────────────────────┘    │
   │  Forgot password? | Sign up     │
   └─────────────────────────────────┘
   
   Components:
   - Logo: Company logo, centered
   - Email input: Required, email validation
   - Password input: Required, masked
   - Remember me: Checkbox, optional
   - Login button: Primary CTA
   - Links: Secondary actions
   
   Form-to-Data Mapping (from Data Structure Step 5):
   | Field | DTO | Attribute | Type | Required | Validation | UI Hint |
   |-------|-----|-----------|------|----------|------------|---------|
   | Email | LoginRequestDTO | email | string | ✅ Yes | email format, max 255 | inputMode="email" |
   | Password | LoginRequestDTO | password | string | ✅ Yes | min 8, max 128 | type="password" |
   | Remember me | LoginRequestDTO | rememberMe | boolean | ❌ No | — | checkbox, default unchecked |

   States:
   - Default: Empty form
   - Loading: Button shows spinner
   - Error: Red border on invalid fields
   - Success: Redirect to dashboard
   ```

5. **Navigation Design**
   - Primary navigation structure
   - Secondary navigation
   - Breadcrumb patterns
   - Mobile navigation (hamburger menu)

6. **Component Library Requirements**
   | Component | Variants | Usage |
   |-----------|----------|-------|
   | Button | Primary, Secondary, Danger, Ghost | CTAs, actions |
   | Input | Text, Email, Password, Number | Form fields |
   | Card | Default, Elevated, Outlined | Content containers |
   | Modal | Alert, Confirm, Form | Overlays |

7. **Responsive Breakpoints**
   | Breakpoint | Width | Layout Changes |
   |------------|-------|----------------|
   | Mobile | < 640px | Single column, hamburger nav |
   | Tablet | 640-1024px | Two columns, condensed nav |
   | Desktop | > 1024px | Full layout, sidebar nav |

8. **Interaction Patterns**
   - Form validation (inline vs submit)
   - Loading states
   - Error handling
   - Success feedback
   - Empty states
   - Pagination/infinite scroll

9. **Accessibility Requirements**
   - WCAG 2.1 AA compliance targets
   - Keyboard navigation requirements
   - Screen reader considerations
   - Color contrast ratios

**Figma/Design Tool Integration:**
If using Figma:
- Design token structure (colors, typography, spacing)
- Component naming conventions
- Auto-layout guidelines
- Handoff specifications

**Guidelines:**
1. Start with mobile-first approach
2. Ensure consistency across screens
3. Document all interactive states
4. Include error and empty states
5. Consider accessibility from the start
6. Map screens to use cases for traceability
7. **Every screen with form inputs MUST include a Form-to-Data Mapping table** — pull field name, type, required/optional, and validation rules directly from `docs/05-data-structure.md` (DTOs and Validation Rules). Mark required fields with ✅ and optional with ❌.
```

## Output

Save the generated document as `docs/16-ux-wireframes.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-16.1-ux-page-flow` - Generate Per-Page UX Sub-Flow files (Step 16.1)
- `/aspec-16.2-ux-wireframe-flow-req` - Generate Wireframe Flow Requirements with formal IDs (Step 16.2)
- `/aspec-17-plan` - Generate Project Scope and Phase Plan (Step 17)
