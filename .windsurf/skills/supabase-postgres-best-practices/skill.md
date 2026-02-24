---
name: supabase-postgres-best-practices
description: PostgreSQL and Supabase database optimization patterns for performance and security
---

# Supabase PostgreSQL Best Practices

Database design, query optimization, and security patterns for PostgreSQL/Supabase.

## When to Use

- Designing database schemas
- Optimizing slow queries
- Implementing Row Level Security (RLS)
- Setting up database migrations

## Schema Design

### Use Proper Data Types

```sql
-- ✅ Use UUID for primary keys (UUID v7 preferred for sortability)
CREATE TABLE users (
  id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
  created_at TIMESTAMPTZ DEFAULT now()
);

-- ✅ Use TIMESTAMPTZ not TIMESTAMP
created_at TIMESTAMPTZ NOT NULL DEFAULT now()

-- ✅ Use TEXT instead of VARCHAR (no performance difference in Postgres)
name TEXT NOT NULL

-- ✅ Use JSONB for flexible data (not JSON)
metadata JSONB DEFAULT '{}'::jsonb

-- ✅ Use enums for fixed values
CREATE TYPE order_status AS ENUM ('pending', 'processing', 'shipped', 'delivered');
```

### Indexing Strategy

```sql
-- ✅ Index foreign keys
CREATE INDEX idx_orders_user_id ON orders(user_id);

-- ✅ Composite index for common query patterns
CREATE INDEX idx_orders_user_status ON orders(user_id, status);

-- ✅ Partial index for filtered queries
CREATE INDEX idx_active_users ON users(email) WHERE deleted_at IS NULL;

-- ✅ GIN index for JSONB queries
CREATE INDEX idx_metadata ON products USING GIN (metadata);

-- ✅ Index for full-text search
CREATE INDEX idx_posts_search ON posts USING GIN (to_tsvector('english', title || ' ' || body));
```

### Query Optimization

```sql
-- ❌ BAD: SELECT *
SELECT * FROM orders WHERE user_id = $1;

-- ✅ GOOD: Select only needed columns
SELECT id, status, total FROM orders WHERE user_id = $1;

-- ❌ BAD: N+1 queries
-- Fetching user, then fetching each order separately

-- ✅ GOOD: Join or batch
SELECT u.*, o.* 
FROM users u 
LEFT JOIN orders o ON o.user_id = u.id 
WHERE u.id = $1;

-- ✅ Use EXPLAIN ANALYZE to check query plans
EXPLAIN ANALYZE SELECT * FROM orders WHERE user_id = $1;

-- ✅ Limit results
SELECT * FROM orders ORDER BY created_at DESC LIMIT 20 OFFSET 0;

-- ✅ Use cursor-based pagination for large datasets
SELECT * FROM orders 
WHERE created_at < $cursor 
ORDER BY created_at DESC 
LIMIT 20;
```

## Row Level Security (RLS)

```sql
-- ✅ Enable RLS on all tables
ALTER TABLE posts ENABLE ROW LEVEL SECURITY;

-- ✅ Policy for users to see their own data
CREATE POLICY "Users can view own posts" ON posts
  FOR SELECT
  USING (auth.uid() = user_id);

-- ✅ Policy for insert
CREATE POLICY "Users can create posts" ON posts
  FOR INSERT
  WITH CHECK (auth.uid() = user_id);

-- ✅ Policy for update
CREATE POLICY "Users can update own posts" ON posts
  FOR UPDATE
  USING (auth.uid() = user_id)
  WITH CHECK (auth.uid() = user_id);

-- ✅ Policy for delete
CREATE POLICY "Users can delete own posts" ON posts
  FOR DELETE
  USING (auth.uid() = user_id);

-- ✅ Admin bypass policy
CREATE POLICY "Admins can do anything" ON posts
  FOR ALL
  USING (
    EXISTS (
      SELECT 1 FROM user_roles 
      WHERE user_id = auth.uid() AND role = 'admin'
    )
  );
```

## Supabase Client Patterns

```typescript
// ✅ Type-safe queries with generated types
import { Database } from './database.types';
import { createClient } from '@supabase/supabase-js';

const supabase = createClient<Database>(url, key);

// ✅ Select with type inference
const { data: posts } = await supabase
  .from('posts')
  .select('id, title, author:users(name)')
  .eq('published', true)
  .order('created_at', { ascending: false })
  .limit(10);

// ✅ Insert with returning
const { data: newPost, error } = await supabase
  .from('posts')
  .insert({ title: 'New Post', user_id: userId })
  .select()
  .single();

// ✅ Upsert for idempotent operations
const { data } = await supabase
  .from('user_settings')
  .upsert({ user_id: userId, theme: 'dark' })
  .select();

// ✅ Real-time subscriptions
const channel = supabase
  .channel('posts')
  .on('postgres_changes', 
    { event: 'INSERT', schema: 'public', table: 'posts' },
    (payload) => console.log('New post:', payload.new)
  )
  .subscribe();
```

## Migrations

```sql
-- ✅ Use transactions for migrations
BEGIN;

ALTER TABLE users ADD COLUMN avatar_url TEXT;
UPDATE users SET avatar_url = 'default.png' WHERE avatar_url IS NULL;
ALTER TABLE users ALTER COLUMN avatar_url SET NOT NULL;

COMMIT;

-- ✅ Add indexes concurrently (no table lock)
CREATE INDEX CONCURRENTLY idx_posts_user ON posts(user_id);

-- ✅ Safe column renames (add new, migrate, drop old)
ALTER TABLE users ADD COLUMN full_name TEXT;
UPDATE users SET full_name = name;
ALTER TABLE users DROP COLUMN name;
```

## Performance Checklist

- [ ] Foreign keys are indexed
- [ ] Queries use specific columns, not SELECT *
- [ ] EXPLAIN ANALYZE shows index usage
- [ ] Pagination uses cursors for large datasets
- [ ] RLS policies are efficient (avoid complex subqueries)
- [ ] Connection pooling is configured
- [ ] Prepared statements are used for repeated queries
