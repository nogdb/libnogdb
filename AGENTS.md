# NogDB Agent Guidelines

This file provides context and instructions for AI agents (Cascade) working in the `libnogdb` repository.

## Project Overview

**libnogdb** is a fast, lightweight, native graph database library written in **C++11**. It exposes a transaction-based API for creating and querying property graphs with vertices, edges, class inheritance, B+Tree indexing, and a SQL-like query interface. There is no client–server protocol; the library is linked directly into the consuming application.

- **Version:** 1.2.1
- **License:** GNU Affero General Public License v3.0 (AGPL-3.0)
- **Bundled dependency:** LMDB (OpenLDAP Public License) — storage engine, source lives in `lib/lmdb/`
- **Platforms:** Linux, macOS/BSD (Windows experimental)

## Repository Structure

```
libnogdb/
├── include/nogdb/       # Public API headers (*.h) — the only surface exposed to callers
│   ├── nogdb.h          # Context, Transaction, operation builders
│   ├── nogdb_types.h    # All public types: Record, Condition, ClassDescriptor, etc.
│   ├── nogdb_errors.h   # Error codes and exception macros
│   ├── nogdb_sql.h      # SQL query entry points
│   └── nogdb_version.h  # Version constants
├── src/                 # Internal implementation (*.cpp + *.hpp) — not exposed
│   ├── lmdb_engine.hpp  # Low-level LMDB C++ wrappers (Value, DBi, Cursor, Txn, Env)
│   ├── storage_engine.hpp # LMDBEnv / LMDBTxn — storage layer
│   ├── storage_adapter.hpp
│   ├── schema_adapter.hpp / schema.hpp / schema.cpp
│   ├── datarecord_adapter.hpp / datarecord.hpp / datarecord.cpp
│   ├── relation_adapter.hpp / relation.hpp / relation.cpp
│   ├── index_adapter.hpp / index.hpp / index.cpp
│   ├── dbinfo_adapter.hpp
│   ├── algorithm.hpp / algorithm.cpp  # Graph traversal algorithms
│   ├── compare.hpp / compare.cpp      # Record filtering / condition evaluation
│   ├── validate.hpp / validate.cpp    # Schema & record validation
│   ├── sql.hpp / sql.cpp              # SQL execution
│   ├── sql_context.hpp / sql_context.cpp
│   ├── sql_parser.y                   # Lemon++ grammar for SQL parser
│   ├── parser.hpp / parser.cpp        # Record binary serialization/deserialization
│   ├── operation.cpp                  # Core CRUD operations
│   ├── context.cpp / transaction.cpp  # Context & Transaction implementations
│   ├── class.cpp / property.cpp       # Schema DDL
│   ├── graph_filter.cpp               # GraphFilter evaluation
│   ├── constant.hpp                   # Internal constants (table names, property IDs)
│   ├── utils.hpp / utils.cpp          # I/O and assertion helpers
│   └── datatype.hpp / datatype.cpp    # Internal binary data types
├── lib/
│   ├── lmdb/            # LMDB source (bundled, OpenLDAP license — do not modify)
│   └── lemonxx/         # Lemon++ parser generator (bundled)
├── test/
│   ├── func_test/       # Functional (integration) tests — built per test group
│   └── unit_test/       # Unit tests using Google Test + Google Mock
├── doc/                 # Documentation
├── example/             # Example programs
├── CMakeLists.txt       # Primary build system
└── .travis.yml          # CI matrix: Linux/macOS × GCC/Clang × CI_Make/CMake
```

## Technology Stack

| Layer | Technology |
|-------|-----------|
| Language | C++11 (strict standard, no extensions) |
| Build | CMake 3.5.1+ and CI_Make (custom Makefile) |
| Storage | LMDB (Memory-Mapped B+Tree, ACID, MVCC) |
| SQL Parser | Lemon++ (generated `sql_parser.cpp` from `sql_parser.y`) |
| Testing | Google Test 1.8.0 + Google Mock |
| Memory Safety | Valgrind, AddressSanitizer |
| CI | Travis CI — Linux (GCC-6, Clang) and macOS (Xcode 8) |
| Formatting | `clang-format` with WebKit style |

## Architecture

### Layered Design

```
Public API (include/nogdb/)
        ↓
Transaction / Context  (src/transaction.cpp, context.cpp)
        ↓
Domain Utils: SchemaUtils, DataRecordUtils, IndexUtils, GraphUtils, Validator, GraphTraversal
        ↓
Adapters: ClassAccess, PropertyAccess, IndexAccess, DBInfoAccess, RelationAdapters
        ↓
Storage Engine: LMDBTxn / LMDBEnv  (src/storage_engine.hpp, lmdb_engine.hpp)
        ↓
LMDB (lib/lmdb/)
```

### Key Internal Namespaces

| Namespace | Purpose |
|-----------|---------|
| `nogdb::storage_engine` | `LMDBEnv`, `LMDBTxn` — wraps raw LMDB handles |
| `nogdb::storage_engine::lmdb` | `Value`, `DBi`, `Cursor`, `Env`, `Transaction` |
| `nogdb::adapter::schema` | `ClassAccess`, `PropertyAccess`, `IndexAccess` |
| `nogdb::adapter::metadata` | `DBInfoAccess` |
| `nogdb::schema` | `SchemaUtils` — schema resolution helpers |
| `nogdb::datarecord` | `DataRecordUtils` — record read/write |
| `nogdb::relation` | `GraphUtils` — in/out edge adjacency |
| `nogdb::index` | `IndexUtils` — B+Tree index operations |
| `nogdb::compare` | `RecordCompare` — condition and filter evaluation |
| `nogdb::validate` | `Validator` — pre-operation checks |
| `nogdb::algorithm` | `GraphTraversal` — BFS/DFS, shortest path |
| `nogdb::parser` | `RecordParser` — binary record serialization |
| `nogdb::sql_parser` | SQL AST and execution context |

### LMDB Internal Tables

| Table Name | Content |
|-----------|---------|
| `.dbinfo` | Database-level metadata (max IDs, counts) |
| `.classes` | Class registry |
| `.properties` | Property registry |
| `.relations#in` / `.relations#out` | Edge adjacency lists |
| `.indexes` | Index registry |
| `.index_<name>` | Per-index B+Tree data |

### System Properties

Every record carries these reserved properties:
- `@className` (ID 0) — class name
- `@recordId` (ID 1) — record identifier
- `@depth` (ID 2) — traversal depth
- `@version` (ID 3) — record version (when `enableVersion()` is set)

### Limits

- Max classes: 65,535
- Max properties: 65,536
- Max class/property name length: 128 characters
- Name pattern: `^[A-Za-z_][A-Za-z0-9_]*$`
- Default max DB size: 1 GB (configurable via `ContextInitializer::setMaxDBSize`)
- Default max DBs: 1024 (configurable via `ContextInitializer::setMaxDB`)

## Build Instructions

```bash
# CMake build
mkdir build && cd build
cmake ..
cmake --build .
ctest --verbose

# Legacy CI_Make build
sh install_ci_make.sh
make CXX=g++ CC=gcc
make test_unit
make test TESTPREFIX=func_test
make test_address TESTPREFIX=func_test   # AddressSanitizer
valgrind --track-origins=yes --leak-check=full ./func_test
```

## Testing

### Functional Tests (integration)

Built as separate executables per domain — each compiled with a `-DTEST_<NAME>_OPERATIONS` flag:

| Target | Domain |
|--------|--------|
| `func_test_context` | DB open/close, settings |
| `func_test_schema` | Class/property DDL |
| `func_test_record` | Vertex/edge CRUD |
| `func_test_misc` | Miscellaneous |
| `func_test_graph` | Graph operations |
| `func_test_find` | Find/filter queries |
| `func_test_inheritance` | Class inheritance |
| `func_test_index` | Indexing |
| `func_test_schema_txn` | Schema transactions |
| `func_test_txn` | Transaction semantics |
| `func_test_sql` | SQL interface |

### Unit Tests

`unit_test_all` — covers internal data types (`datatype.cpp`, `utils.cpp`) and LMDB engine wrappers. Requires `gtest` and `gmock`.

### Memory Safety

Always run Valgrind and AddressSanitizer before submitting a PR. Both are exercised in CI.

## Code Style

Follow [CppCoreGuidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md) and the rules below.

- **Formatter:** `clang-format -i -style=WebKit *`
- **Namespaces:** lowercase single word (`nogdb`, `schema`, `adapter`)
- **Types/classes/structs:** PascalCase (`ClassDescriptor`, `RecordParser`)
- **Functions/variables:** camelCase (`addVertex`, `recordDescriptor`)
- **Use `auto`** wherever it improves readability
- **Pass by const reference** for non-trivial objects
- **Prefer RAII** — smart pointers over raw `new`/`delete`
- **Rule of three/five/zero** — declare copy/move explicitly; use `= delete` for non-copyable types
- **Header guards:** `#pragma once` only — never `#ifndef/#define` guards
- **Include order:**
  1. C system headers (`<unistd.h>`)
  2. Standard C++ headers (`<vector>`, `<algorithm>`)
  3. Internal headers (`"schema.hpp"`)
  4. Public headers (`"nogdb/nogdb.h"`)
- Only include what the translation unit actually uses

## Git Workflow

### Branches (Gitflow)

| Branch | Purpose |
|--------|---------|
| `master` | Production-stable |
| `develop` | Integration (latest passing tests) |
| `feature/<name>` | New features, test additions, non-critical fixes |
| `hotfix/<name>` | Critical production fixes against `master` |
| `release/<version>` | Release candidates |
| `document/<name>` | Documentation-only changes |

### Pull Request Rules

- All PRs targeting `develop` (features) or `master` (hotfixes)
- Tests **must** be included for every change
- PR must not break any existing test
- Signed CLA required for external contributors
- CI must pass (Travis: Linux+macOS × GCC/Clang × CI_Make/CMake)

## Code Review Checklist

### Correctness
- Transaction boundaries are correct (`commit` / `rollback`)
- MVCC semantics respected (multiple readers, single writer)
- No use of raw `new`/`delete` where RAII applies
- Exception safety: destructors are `noexcept` where appropriate

### Performance
- Adapter objects not heap-allocated unnecessarily inside hot paths
- Cursor usage is properly scoped and closed
- B+Tree index used where applicable for filtered queries

### Quality
- New public API changes reflected in `include/nogdb/nogdb.h` and `nogdb_types.h`
- Internal symbols stay in `src/` — never leak into public headers
- `clang-format` applied before commit
- Unit or functional tests added/updated
- No memory leaks (Valgrind clean)

## Forbidden Actions

- **Never modify `lib/lmdb/`** — LMDB source must remain pristine under its OpenLDAP license
- **Never expose internal `src/` headers** in `include/nogdb/`
- **Never hardcode absolute paths** (the only exception is `/usr/local/include` in CMakeLists for system deps)
- **Never leave raw owning pointers** without RAII wrappers in new code
- **Never commit generated files** (`sql_parser.cpp` is generated by Lemon++ — do not hand-edit)
