---
description: Generate Data Structure with schemas and validation rules (Step 5)
arguments:
  send: true
---

# Data Structure Generator

Use this workflow after completing Step 4 (Data Model).

## Usage

```
/aspec-05-data-structure [input_file]
```

**Examples:**
- `/aspec-05-data-structure docs/04-data-model.md`
- `/aspec-05-data-structure` (will look for file in docs/ folder)

## Input

```text
$ARGUMENTS
```

If `$ARGUMENTS` contains a file path, read that file as the Data Model.
If empty, look for `docs/04-data-model.md`.

## Prerequisites

Ensure you have completed:
- Data Model (Step 4)

## Prompt

```
You are a software architect. Based on the Data Model, define concrete Data Structures with schemas and validation rules.

**Input Document:**
- Data Model (Step 4)

**Output Requirements:**

For each entity, provide:

1. **Schema Definition** (TypeScript/JSON Schema format)
   ```typescript
   interface EntityName {
     id: string;
     field1: string;
     field2: number;
     // ...
   }
   ```

2. **Validation Rules**
   | Field | Type | Required | Min | Max | Pattern | Custom Rules |
   |-------|------|----------|-----|-----|---------|--------------|

3. **Enums and Constants**
   ```typescript
   enum Status {
     DRAFT = 'draft',
     ACTIVE = 'active',
     ARCHIVED = 'archived'
   }
   ```

4. **DTOs (Data Transfer Objects)**
   - CreateEntityDTO (for creation)
   - UpdateEntityDTO (for updates)
   - EntityResponseDTO (for API responses)
   - EntityListDTO (for list responses with pagination)

5. **Indexes**
   | Entity | Index Name | Fields | Type | Purpose |
   |--------|------------|--------|------|---------|

6. **Sample Data**
   Provide 2-3 sample records for each entity in JSON format.

**Guidelines:**
1. Use appropriate data types for each field
2. Define all validation constraints (min/max length, patterns, ranges)
3. Include nullable vs required specifications
4. Define default values where applicable
5. Consider API serialization needs
```

## Output

Save the generated document as `docs/05-data-structure.md` in your project.

## Next Steps

After completing this step, proceed to:
- `/aspec-06-actor-list` - Generate Actor List (Step 6)
