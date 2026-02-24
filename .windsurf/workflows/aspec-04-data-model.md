---
description: Generate Data Model with ER diagrams and constraints (Step 4)
arguments:
  send: true
---

# Data Model Generator

Use this workflow after completing Step 3 (Use Cases).

## Usage

```
/aspec-04-data-model [input_files]
```

**Examples:**
- `/aspec-04-data-model docs/01-project-spec.md docs/02-user-stories.md docs/03-use-cases.md`
- `/aspec-04-data-model` (will look for files in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains file paths, read those files as input documents.
If empty, look for `docs/01-project-spec.md`, `docs/02-user-stories.md`, and `docs/03-use-cases.md`.

## Prerequisites

Ensure you have completed:
- Project Spec Description (Step 1)
- User Stories (Step 2)
- Use Cases (Step 3)

## Prompt

```
You are a data architect. Based on the specification documents, create a comprehensive Data Model.

**Input Documents:**
- Project Spec Description (Step 1)
- User Stories (Step 2)
- Use Cases (Step 3)

**Output Requirements:**

1. **Entity List**
   For each entity:
   - Entity name
   - Description
   - Key attributes (name, type, constraints)
   - Primary key
   - Business rules

2. **Relationships**
   For each relationship:
   - Entity A → Entity B
   - Relationship type (1:1, 1:N, M:N)
   - Cardinality (mandatory/optional on each side)
   - Relationship description

3. **Entity-Relationship Diagram** (Mermaid format)
   ```mermaid
   erDiagram
       ENTITY_A ||--o{ ENTITY_B : "relationship"
   ```

4. **Data Dictionary**
   | Entity | Attribute | Type | Constraints | Description |
   |--------|-----------|------|-------------|-------------|

5. **Integrity Constraints**
   - Primary key constraints
   - Foreign key constraints
   - Unique constraints
   - Check constraints
   - Business rule constraints

**Guidelines:**
1. Normalize to at least 3NF
2. Identify all foreign key relationships
3. Document cascade behaviors (ON DELETE, ON UPDATE)
4. Include audit fields (created_at, updated_at, created_by)
5. Consider soft delete vs hard delete
```

## Output

Save the generated document as `docs/04-data-model.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-05-data-structure` - Generate Data Structure (Step 5)
