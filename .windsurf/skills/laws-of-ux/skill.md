---
name: laws-of-ux
description: Apply Laws of UX design patterns when building or reviewing user interfaces
---

# Laws of UX Skill

Apply cognitive psychology and UX design laws to build intuitive, effective interfaces.

## When to Use

- Designing new UI components or pages
- Reviewing UI/UX decisions
- Optimizing user flows and conversion funnels
- Reducing cognitive load in complex interfaces
- Improving navigation and information architecture

## Perception & Aesthetics

### Aesthetic-Usability Effect

Users perceive aesthetically pleasing design as more usable. Visual polish increases tolerance for minor usability issues.

```markdown
# ✅ Apply
- Invest in consistent spacing, typography, and color
- Use smooth transitions and polished micro-interactions
- Maintain visual harmony across components

# ❌ Avoid
- Shipping rough/unstyled prototypes to users
- Inconsistent visual treatment across similar elements
```

### Law of Prägnanz

People interpret complex/ambiguous visuals as the simplest form possible (least cognitive effort).

```markdown
# ✅ Apply
- Use simple, recognizable shapes for icons and layouts
- Reduce visual noise — remove decorative-only elements
- Prefer clean grid layouts over complex arrangements

# ❌ Avoid
- Overly detailed icons at small sizes
- Ambiguous visual groupings
```

### Von Restorff Effect (Isolation Effect)

When multiple similar objects are present, the one that differs is most likely to be remembered.

```markdown
# ✅ Apply
- Make primary CTAs visually distinct (color, size, weight)
- Highlight key data points in dashboards
- Use contrast to draw attention to important notifications

# ❌ Avoid
- Making everything bold or colorful (nothing stands out)
- Styling primary and secondary actions identically
```

## Decision Making & Choices

### Hick's Law

Decision time increases with the number and complexity of choices.

```markdown
# ✅ Apply
- Limit navigation items to 5–7 top-level options
- Use progressive disclosure (show more on demand)
- Provide smart defaults for forms and settings
- Break complex forms into multi-step wizards

# ❌ Avoid
- Mega-menus with 50+ uncategorized links
- Settings pages with every option visible at once
```

### Choice Overload (Paradox of Choice)

People get overwhelmed when presented with too many options.

```markdown
# ✅ Apply
- Offer curated recommendations ("Most Popular", "Recommended")
- Use filters and sorting to narrow large catalogs
- Limit plan/pricing tiers to 3–4 options
- Pre-select the best default option

# ❌ Avoid
- Showing all product variants on a single page without filtering
- Presenting more than 3–4 options for a single decision
```

### Occam's Razor

Among competing solutions, prefer the one with fewest assumptions. Simplify.

```markdown
# ✅ Apply
- Remove UI elements that don't serve a clear purpose
- Prefer one clear path over multiple ambiguous ones
- Simplify copy — fewer words, clearer meaning

# ❌ Avoid
- Adding features "just in case" someone needs them
- Complex onboarding flows when a simple one suffices
```

## Memory & Cognition

### Miller's Law

The average person can keep 7 ± 2 items in working memory.

```markdown
# ✅ Apply
- Chunk navigation into groups of 5–7
- Break phone numbers, card numbers into groups (e.g., 1234 5678 9012)
- Limit visible tabs/categories to ~7

# ❌ Avoid
- Flat lists with 15+ ungrouped items
- Requiring users to remember info from previous screens
```

### Chunking

Break individual pieces of information into meaningful groups.

```typescript
// ✅ Chunked input formatting
<input placeholder="1234 5678 9012 3456" />  // Credit card
<input placeholder="(555) 123-4567" />        // Phone number

// ✅ Chunked content layout
// Group related settings under clear headings
<SettingsSection title="Notifications">
  <Toggle label="Email" />
  <Toggle label="Push" />
  <Toggle label="SMS" />
</SettingsSection>
```

### Cognitive Load

The mental resources needed to understand and interact with an interface.

```markdown
# Three types to minimize:
1. **Intrinsic** — Simplify the task itself (wizards, smart defaults)
2. **Extraneous** — Remove distractions (clean layout, no clutter)
3. **Germane** — Support learning (familiar patterns, clear labels)

# ✅ Apply
- Use familiar UI patterns (don't reinvent the dropdown)
- Show only relevant options for current context
- Use progressive disclosure for advanced features
- Provide inline help and tooltips

# ❌ Avoid
- Jargon-heavy labels
- Requiring mental math or cross-referencing
```

### Mental Model

Users carry a compressed model of how systems work based on prior experience.

```markdown
# ✅ Apply
- Follow platform conventions (e.g., swipe-to-delete on mobile)
- Use standard icons (🔍 = search, 🗑️ = delete, ⚙️ = settings)
- Match user expectations from competitor products (Jakob's Law)
- Place navigation where users expect it (top bar, sidebar)

# ❌ Avoid
- Inventing novel interaction patterns without clear affordances
- Placing critical actions in unexpected locations
```

## Interaction & Performance

### Fitts's Law

Time to acquire a target is a function of distance to and size of the target.

```css
/* ✅ Large, easy-to-hit targets for primary actions */
.primary-button {
  min-height: 44px;
  min-width: 44px;
  padding: 12px 24px;
}

/* ✅ Place related actions close together */
/* ✅ Put destructive actions away from confirm buttons */
/* ✅ Use full-width buttons on mobile */
.mobile-cta {
  width: 100%;
  min-height: 48px;
}
```

### Doherty Threshold

Productivity soars when system response time is < 400ms.

```markdown
# ✅ Apply
- Show loading skeletons instead of blank screens
- Use optimistic UI updates (show result before server confirms)
- Preload likely next pages/data
- Animate transitions to mask latency (200–300ms)

# ❌ Avoid
- Blocking the UI with full-page spinners
- No feedback after user action (feels broken)
```

```typescript
// ✅ Optimistic UI example
const handleLike = async (postId: string) => {
  setLiked(true);           // Instant visual feedback
  setCount(prev => prev + 1);
  try {
    await api.likePost(postId);
  } catch {
    setLiked(false);         // Revert on failure
    setCount(prev => prev - 1);
  }
};
```

### Paradox of the Active User

Users never read manuals — they start using software immediately.

```markdown
# ✅ Apply
- Design self-explanatory interfaces
- Use progressive onboarding (teach in context, not upfront)
- Provide undo instead of confirmation dialogs
- Make exploration safe (easy to reverse actions)

# ❌ Avoid
- Mandatory 10-step onboarding tours
- Relying on documentation for core workflows
```

## Grouping & Layout (Gestalt Principles)

### Law of Common Region

Elements sharing an area with a defined boundary are perceived as grouped.

```css
/* ✅ Use cards to group related content */
.card {
  border: 1px solid var(--border);
  border-radius: 8px;
  padding: 16px;
}

/* ✅ Use background color to define regions */
.section-highlight {
  background: var(--surface-secondary);
  padding: 24px;
  border-radius: 12px;
}
```

### Law of Proximity

Objects near each other are perceived as grouped.

```css
/* ✅ Use spacing to create logical groups */
.form-group {
  margin-bottom: 24px;  /* Space between groups */
}
.form-group label {
  margin-bottom: 4px;   /* Tight spacing within group */
}
.form-group input {
  margin-bottom: 4px;
}
```

### Law of Similarity

Similar elements are perceived as a group even if separated.

```markdown
# ✅ Apply
- Use consistent color for all clickable elements
- Style all status badges the same way (shape, size)
- Use the same icon style throughout (outline OR filled, not mixed)

# ❌ Avoid
- Mixing icon styles (some outlined, some filled)
- Using different button styles for same-level actions
```

### Law of Uniform Connectedness

Visually connected elements are perceived as more related.

```markdown
# ✅ Apply
- Use lines/connectors in step indicators and timelines
- Use shared background colors for related toolbar groups
- Connect labels to inputs visually (proximity + alignment)

# ❌ Avoid
- Orphaned labels far from their inputs
- Step indicators without connecting lines
```

## Attention & Memory Positioning

### Serial Position Effect

Users best remember the first and last items in a series.

```markdown
# ✅ Apply
- Place most important nav items first and last
- In bottom tab bars: Home (first), Profile/CTA (last)
- In feature lists: lead with strongest, end with second strongest

# ❌ Avoid
- Burying the most important action in the middle of a list
```

### Goal-Gradient Effect

Effort increases as people approach a goal.

```typescript
// ✅ Show progress to motivate completion
<ProgressBar value={75} label="3 of 4 steps complete" />

// ✅ Start progress bars slightly filled (e.g., "Step 1 of 4 — 25% done")
// ✅ Use "almost there!" messaging near completion
```

### Zeigarnik Effect

People remember uncompleted tasks better than completed ones.

```markdown
# ✅ Apply
- Show incomplete profile indicators ("Complete your profile — 60%")
- Use checklists with visible remaining items
- Send reminders for abandoned carts/forms

# ❌ Avoid
- Hiding progress on multi-step processes
```

### Selective Attention

People focus only on stimuli related to their current goal.

```markdown
# ✅ Apply
- Don't rely on users noticing banners — use inline contextual alerts
- Place warnings next to the action they affect
- Use motion sparingly to draw attention to critical changes

# ❌ Avoid
- Important info only in a dismissible toast
- Assuming users will read sidebar announcements
```

## Experience & Satisfaction

### Peak-End Rule

People judge experiences by their peak moment and ending, not the average.

```markdown
# ✅ Apply
- Invest in delightful success states (confetti, celebration animation)
- Make checkout confirmation / thank-you pages feel rewarding
- End onboarding with an immediate "win" (e.g., first result)
- Handle errors gracefully — a bad ending ruins the whole experience

# ❌ Avoid
- Generic "Success" messages with no personality
- Ending flows with an error or dead-end page
```

### Postel's Law (Robustness Principle)

Be liberal in what you accept, conservative in what you send.

```typescript
// ✅ Accept flexible input formats
function parsePhone(input: string): string {
  // Accept: (555) 123-4567, 555-123-4567, 5551234567
  return input.replace(/\D/g, '').slice(-10);
}

// ✅ Accept varied date formats
// ✅ Trim whitespace automatically
// ✅ Auto-capitalize where appropriate
// ✅ Show clean, consistent output regardless of input format
```

### Tesler's Law (Conservation of Complexity)

Every system has irreducible complexity. Absorb it so users don't have to.

```markdown
# ✅ Apply
- Auto-detect timezone, locale, currency
- Pre-fill forms with known data
- Handle edge cases in code, not with user-facing options
- Provide smart defaults that work for 80% of users

# ❌ Avoid
- Exposing system complexity to users ("Select your database shard")
- Forcing users to configure things the system can infer
```

## Productivity & Scope

### Pareto Principle (80/20 Rule)

~80% of effects come from ~20% of causes.

```markdown
# ✅ Apply
- Identify the 20% of features used 80% of the time — make them prominent
- Optimize the critical path first (signup → first value)
- Focus usability testing on top user flows

# ❌ Avoid
- Equal visual weight for all features regardless of usage
- Spending design time on rarely-used admin screens
```

### Parkinson's Law

Tasks expand to fill available time.

```markdown
# ✅ Apply
- Use character limits for inputs (bio: 160 chars)
- Set time constraints on offers ("Expires in 24h")
- Break large tasks into small, time-boxed steps

# ❌ Avoid
- Open-ended text areas with no guidance on expected length
- Unlimited-time trials with no activation nudge
```

## Quick-Reference Checklist

- [ ] Choices simplified — max 5–7 options visible (Hick's Law, Choice Overload)
- [ ] Info chunked into groups of 5–7 (Miller's Law, Chunking)
- [ ] Related elements visually grouped (Proximity, Common Region, Similarity)
- [ ] Click targets ≥ 44×44px, primary actions are large & close (Fitts's Law)
- [ ] Response time < 400ms or loading indicator shown (Doherty Threshold)
- [ ] UI matches user expectations from other apps (Jakob's Law, Mental Model)
- [ ] Key actions visually distinct from secondary ones (Von Restorff Effect)
- [ ] Flow ends with a satisfying moment (Peak-End Rule)
- [ ] Progress shown for multi-step processes (Goal-Gradient, Zeigarnik)
- [ ] Interface is visually polished and consistent (Aesthetic-Usability Effect)
