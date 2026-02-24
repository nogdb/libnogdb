---
name: web-design-guidelines
description: Review UI code for compliance with web interface best practices covering accessibility, performance, and UX
---

# Web Design Guidelines Skill

Audit your code for 100+ rules covering accessibility, performance, and UX.

## When to Use

- "Review my UI"
- "Check accessibility"
- "Audit design"
- "Review UX"
- "Check my site against best practices"

## Categories

### Accessibility

```typescript
// ✅ Semantic HTML
<button onClick={handleClick}>Submit</button>  // Not <div onClick>

// ✅ ARIA labels for icons
<button aria-label="Close dialog">
  <XIcon aria-hidden="true" />
</button>

// ✅ Form labels
<label htmlFor="email">Email</label>
<input id="email" type="email" />

// ✅ Skip links
<a href="#main-content" className="sr-only focus:not-sr-only">
  Skip to main content
</a>

// ✅ Alt text for images
<img src="chart.png" alt="Sales increased 25% in Q4 2024" />
```

### Focus States

```css
/* ✅ Visible focus indicators */
button:focus-visible {
  outline: 2px solid var(--focus-color);
  outline-offset: 2px;
}

/* ✅ Don't remove focus for mouse users who need it */
:focus:not(:focus-visible) {
  outline: none;
}

/* ✅ Focus trap for modals */
```

```typescript
// Focus trap implementation
function Modal({ isOpen, onClose, children }) {
  const modalRef = useRef();
  
  useEffect(() => {
    if (isOpen) {
      const focusableElements = modalRef.current.querySelectorAll(
        'button, [href], input, select, textarea, [tabindex]:not([tabindex="-1"])'
      );
      focusableElements[0]?.focus();
    }
  }, [isOpen]);
  
  return isOpen ? <div ref={modalRef} role="dialog">{children}</div> : null;
}
```

### Forms

```typescript
// ✅ Autocomplete attributes
<input type="email" autoComplete="email" />
<input type="password" autoComplete="current-password" />
<input type="text" autoComplete="name" />
<input type="tel" autoComplete="tel" />

// ✅ Input modes for mobile keyboards
<input type="text" inputMode="numeric" pattern="[0-9]*" /> // Number pad
<input type="text" inputMode="email" /> // Email keyboard
<input type="text" inputMode="url" /> // URL keyboard

// ✅ Validation with clear errors
<input
  aria-invalid={!!error}
  aria-describedby={error ? "email-error" : undefined}
/>
{error && <span id="email-error" role="alert">{error}</span>}
```

### Animation

```css
/* ✅ Respect reduced motion preference */
@media (prefers-reduced-motion: reduce) {
  *, *::before, *::after {
    animation-duration: 0.01ms !important;
    transition-duration: 0.01ms !important;
  }
}

/* ✅ Use compositor-friendly properties */
.animate {
  /* Good: transform, opacity */
  transform: translateX(100px);
  opacity: 0.5;
  
  /* Avoid: width, height, top, left (cause layout) */
}

/* ✅ Use will-change sparingly */
.will-animate:hover {
  will-change: transform;
}
```

### Typography

```css
/* ✅ Use proper quotes */
q::before { content: '"'; }
q::after { content: '"'; }

/* ✅ Tabular numbers for data */
.data-table td {
  font-variant-numeric: tabular-nums;
}

/* ✅ Proper ellipsis */
.truncate {
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

/* ✅ Readable line length */
.prose {
  max-width: 65ch;
}
```

### Images

```typescript
// ✅ Always specify dimensions
<img src="photo.jpg" width={800} height={600} alt="Description" />

// ✅ Use Next.js Image for optimization
import Image from 'next/image';
<Image
  src="/photo.jpg"
  alt="Description"
  width={800}
  height={600}
  placeholder="blur"
  blurDataURL={blurUrl}
/>

// ✅ Lazy load below-fold images
<img loading="lazy" src="below-fold.jpg" alt="..." />

// ✅ Responsive images
<picture>
  <source media="(min-width: 800px)" srcSet="large.jpg" />
  <source media="(min-width: 400px)" srcSet="medium.jpg" />
  <img src="small.jpg" alt="..." />
</picture>
```

### Performance

```typescript
// ✅ Preconnect to external origins
<link rel="preconnect" href="https://fonts.googleapis.com" />
<link rel="dns-prefetch" href="https://analytics.example.com" />

// ✅ Virtualize long lists
import { useVirtualizer } from '@tanstack/react-virtual';

// ✅ Avoid layout thrashing
// Batch DOM reads before writes
```

### Navigation & State

```typescript
// ✅ URL reflects application state
function Filters() {
  const [searchParams, setSearchParams] = useSearchParams();
  
  const handleFilter = (filter) => {
    setSearchParams({ ...Object.fromEntries(searchParams), filter });
  };
}

// ✅ Support deep linking
// Users can share/bookmark filtered views

// ✅ Preserve scroll position
<ScrollRestoration />
```

### Dark Mode & Theming

```css
/* ✅ System preference detection */
@media (prefers-color-scheme: dark) {
  :root {
    --bg: #1a1a1a;
    --text: #ffffff;
  }
}

/* ✅ Color scheme meta */
```

```html
<meta name="color-scheme" content="light dark" />
<meta name="theme-color" content="#ffffff" media="(prefers-color-scheme: light)" />
<meta name="theme-color" content="#1a1a1a" media="(prefers-color-scheme: dark)" />
```

### Touch & Interaction

```css
/* ✅ Appropriate touch targets (44x44px minimum) */
.button {
  min-height: 44px;
  min-width: 44px;
}

/* ✅ Disable tap highlight on custom buttons */
.custom-button {
  -webkit-tap-highlight-color: transparent;
}

/* ✅ Touch action for custom gestures */
.carousel {
  touch-action: pan-y pinch-zoom;
}
```

### Locale & i18n

```typescript
// ✅ Use Intl APIs
const formatter = new Intl.DateTimeFormat('en-US', {
  dateStyle: 'medium',
  timeStyle: 'short'
});

const currency = new Intl.NumberFormat('en-US', {
  style: 'currency',
  currency: 'USD'
});

// ✅ Relative time
const rtf = new Intl.RelativeTimeFormat('en', { numeric: 'auto' });
rtf.format(-1, 'day'); // "yesterday"
```

## Audit Checklist

- [ ] All interactive elements are keyboard accessible
- [ ] Focus indicators are visible
- [ ] Images have alt text
- [ ] Forms have proper labels and autocomplete
- [ ] Animations respect prefers-reduced-motion
- [ ] Touch targets are at least 44x44px
- [ ] Color contrast meets WCAG AA (4.5:1 for text)
- [ ] Page works without JavaScript for core content
