---
description: Generate Persistence Design with database schema and storage strategy (Step 11)
arguments:
  send: true
---

# Persistence Design Generator

Use this workflow after completing Step 10 (Object Life Cycle).

## Usage

```
/aspec-11-persistence [input_files]
```

**Examples:**
- `/aspec-11-persistence docs/04-data-model.md docs/05-data-structure.md docs/10-object-lifecycle.md`
- `/aspec-11-persistence` (will look for files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths, read those files as input documents.
If empty, look for `docs/04-data-model.md`, `docs/05-data-structure.md`, and `docs/10-object-lifecycle.md`.

## Prerequisites

Ensure you have completed:
- Data Model (Step 4)
- Data Structure (Step 5)
- Object Life Cycle (Step 10)

## Prompt

```
You are a database architect. Design the Persistence layer based on the data model and structures.

**Input Documents:**
- Data Model (Step 4)
- Data Structure (Step 5)
- Object Life Cycle (Step 10)

**Output Requirements:**

1. **Database Schema** (SQL DDL)
   ```sql
   CREATE TABLE users (
       id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
       email VARCHAR(255) NOT NULL UNIQUE,
       password_hash VARCHAR(255) NOT NULL,
       name VARCHAR(100) NOT NULL,
       status VARCHAR(20) DEFAULT 'active',
       created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
       updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
   );
   
   CREATE INDEX idx_users_email ON users(email);
   ```

2. **Index Strategy**
   | Table | Index Name | Columns | Type | Purpose |
   |-------|------------|---------|------|---------|
   | users | idx_users_email | email | UNIQUE | Login lookup |
   | orders | idx_orders_user_date | user_id, created_at | BTREE | User order history |

3. **Storage Strategy**
   - Primary database: [PostgreSQL/MySQL/MongoDB]
   - Caching layer: [Redis/Memcached]
   - File storage: [S3/Local/CDN]
   - Search index: [Elasticsearch] (if needed)

4. **Data Partitioning** (if applicable)
   - Partitioning strategy (by date, by tenant, etc.)
   - Sharding approach

5. **Backup and Recovery**
   - Backup frequency
   - Retention policy
   - Recovery point objective (RPO)
   - Recovery time objective (RTO)

6. **Migration Strategy**
   - Migration tool (Flyway, Alembic, Prisma)
   - Versioning approach
   - Rollback procedures

7. **Query Patterns**
   | Query | Frequency | Tables | Expected Performance |
   |-------|-----------|--------|---------------------|
   | Get user by email | High | users | < 10ms |
   | List user orders | Medium | orders, order_items | < 50ms |

**Guidelines:**
1. Optimize for common query patterns
2. Plan for data growth
3. Consider read/write ratios
4. Document cascade delete behaviors
5. Include soft delete strategy if applicable
```

## Output

Save the generated document as `docs/11-persistence-design.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-12-architecture` - Generate Architecture Summary (Step 12)
