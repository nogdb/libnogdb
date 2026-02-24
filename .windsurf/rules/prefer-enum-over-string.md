# Prefer Enum Over String

Use enums for fixed sets of options, not string literals.

## Why
- Type safety (catch typos at compile time)
- IDE autocomplete
- Easy refactoring
- Self-documenting

## Guidelines
- Look for 3+ related string literals being compared
- Extract to enum
- Language patterns:
  - TS: `enum` or `const` objects
  - Go: Type alias + `const` block
  - Python: `Enum` class
- Keep strings for: User input, external APIs, dynamic data

## Activation
- **Mode**: Glob
- **Pattern**: `*.ts`, `*.go`, `*.py`
