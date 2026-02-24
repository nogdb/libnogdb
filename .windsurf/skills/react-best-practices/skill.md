---
name: react-best-practices
description: React and Next.js performance optimization guidelines with 40+ rules across 8 categories
---

# React Best Practices Skill

React and Next.js performance optimization guidelines from Vercel Engineering.

## When to Use

- Writing new React components or Next.js pages
- Implementing data fetching (client or server-side)
- Reviewing code for performance issues
- Optimizing bundle size or load times

## Categories (by Priority)

### Critical: Eliminating Waterfalls

```typescript
// ❌ BAD: Sequential data fetching (waterfall)
async function Page() {
  const user = await getUser();
  const posts = await getPosts(user.id); // Waits for user
  const comments = await getComments(posts[0].id); // Waits for posts
}

// ✅ GOOD: Parallel data fetching
async function Page() {
  const user = await getUser();
  const [posts, profile] = await Promise.all([
    getPosts(user.id),
    getProfile(user.id)
  ]);
}

// ✅ BETTER: Use React Server Components with Suspense
async function Page() {
  return (
    <Suspense fallback={<Loading />}>
      <UserData />
      <PostsData />
    </Suspense>
  );
}
```

### Critical: Bundle Size Optimization

```typescript
// ❌ BAD: Import entire library
import { format } from 'date-fns';

// ✅ GOOD: Tree-shakeable import
import format from 'date-fns/format';

// ❌ BAD: Client component with heavy deps
'use client';
import { Chart } from 'chart.js'; // 200KB+

// ✅ GOOD: Dynamic import for heavy components
const Chart = dynamic(() => import('./Chart'), {
  loading: () => <ChartSkeleton />,
  ssr: false
});
```

### High: Server-Side Performance

```typescript
// ✅ Use Server Components by default
// app/page.tsx (no 'use client')
async function Page() {
  const data = await db.query('SELECT * FROM posts');
  return <PostList posts={data} />;
}

// ✅ Cache expensive operations
import { unstable_cache } from 'next/cache';

const getCachedPosts = unstable_cache(
  async () => db.posts.findMany(),
  ['posts'],
  { revalidate: 3600 }
);

// ✅ Use streaming for slow data
import { Suspense } from 'react';

function Page() {
  return (
    <>
      <Header /> {/* Renders immediately */}
      <Suspense fallback={<PostsSkeleton />}>
        <SlowPosts /> {/* Streams when ready */}
      </Suspense>
    </>
  );
}
```

### Medium-High: Client-Side Data Fetching

```typescript
// ✅ Use SWR or TanStack Query
import useSWR from 'swr';

function Profile() {
  const { data, error, isLoading } = useSWR('/api/user', fetcher, {
    revalidateOnFocus: false,
    dedupingInterval: 60000
  });
  
  if (isLoading) return <Skeleton />;
  if (error) return <Error />;
  return <UserCard user={data} />;
}

// ✅ Prefetch on hover/focus
function Link({ href, children }) {
  const prefetch = () => mutate(href, fetcher(href));
  return (
    <a href={href} onMouseEnter={prefetch} onFocus={prefetch}>
      {children}
    </a>
  );
}
```

### Medium: Re-render Optimization

```typescript
// ❌ BAD: Object created every render
function List({ items }) {
  return items.map(item => (
    <Item key={item.id} style={{ margin: 10 }} /> // New object each render
  ));
}

// ✅ GOOD: Stable references
const itemStyle = { margin: 10 };
function List({ items }) {
  return items.map(item => (
    <Item key={item.id} style={itemStyle} />
  ));
}

// ✅ Use React.memo for expensive components
const ExpensiveList = React.memo(function ExpensiveList({ items }) {
  return items.map(item => <ExpensiveItem key={item.id} {...item} />);
});

// ✅ Use useCallback for event handlers passed to children
function Parent() {
  const handleClick = useCallback((id) => {
    // handle click
  }, []);
  
  return <MemoizedChild onClick={handleClick} />;
}
```

### Medium: Rendering Performance

```typescript
// ✅ Virtualize long lists
import { FixedSizeList } from 'react-window';

function VirtualList({ items }) {
  return (
    <FixedSizeList
      height={400}
      itemCount={items.length}
      itemSize={50}
    >
      {({ index, style }) => (
        <div style={style}>{items[index].name}</div>
      )}
    </FixedSizeList>
  );
}

// ✅ Use CSS containment
.card {
  contain: layout style paint;
}

// ✅ Avoid layout thrashing
// ❌ BAD: Read then write in loop
elements.forEach(el => {
  const height = el.offsetHeight; // Read
  el.style.height = height + 10 + 'px'; // Write
});

// ✅ GOOD: Batch reads, then writes
const heights = elements.map(el => el.offsetHeight);
elements.forEach((el, i) => {
  el.style.height = heights[i] + 10 + 'px';
});
```

## Quick Checklist

- [ ] No sequential awaits that could be parallel
- [ ] Heavy components use dynamic imports
- [ ] Server Components used where possible
- [ ] Lists with 100+ items are virtualized
- [ ] Expensive components wrapped in React.memo
- [ ] Event handlers use useCallback when passed down
- [ ] No inline object/array literals in JSX
- [ ] Images have width/height or use next/image
