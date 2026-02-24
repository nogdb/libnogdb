# NogDB - Native Graph Database (C++) Library 

[![Gitter](https://img.shields.io/gitter/room/gitterHQ/gitter.svg)](https://gitter.im/nogdb/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Build Status](https://travis-ci.org/nogdb/libnogdb.svg?branch=develop)](https://travis-ci.org/nogdb/libnogdb)
[![Version](https://img.shields.io/badge/version-1.2.1-blue.svg)]()
[![Production Status](https://img.shields.io/badge/status-stable-green.svg)]()

## Welcome
**NogDB is a fast & lightweight native graph database library that provides features in graph manipulations.**

## Key Features
* Considerably fast graph database written in C++
* Low-level graph database operations
* Lightweight and native with no client-server protocols needed
* Transactions with MVCC (full ACID semantics in data storage)
* Multiple readers and single writer (no blocking between readers and writer)
* SQL support for performing graph executions and querying graph information
* Inheritance support for vertex and edge classes
* B+Tree indexing with support for equality **and range** queries (LESS, GREATER, BETWEEN)
* **BFS and DFS graph traversal** with configurable depth, edge/vertex filters, and direction (IN / OUT / UNDIRECTED)
* **Shortest path** — unweighted BFS and **weighted Dijkstra** (reads edge weight from a named property)
* **Batch write API** (`beginBatchTxn()`) for coalescing multiple writes into a single LMDB transaction
* **Lambda/closure filter support** — all record filter callbacks use `std::function<bool(const Record&)>`
* **Transaction-level schema cache** for reduced LMDB lookups on repeated schema access within a transaction
* Buildable under **C++11** (default) and **C++17** (`-Dnogdb_CXX17=ON`)
* Aim to support for multiple platforms (currently available on Unix, Linux, BSD, and Mac OS X/macOS, Windows is under experiment)

## Dependencies
* GCC (gcc/g++ 5.1.0 or above) or LLVM (clang/clang++) compiler — C++11 or C++17
* CMake 3.5.1 or above
* On Windows, use MinGW-w64 (gcc/g++ 7.2, mingw-w64 5.0 or above)
* [Google Test](https://github.com/google/googletest) — for development/testing only

## Limitations
* The current data storage architecture supports only up to 65,535 classes and 65,536 properties.
* For a large graph database, a maximum size of a data storage may need to be customized and appropriately defined in advance.

## Build and Installation

```bash
$ git clone https://github.com/nogdb/nogdb
$ cd nogdb

# Build (C++11, default)
$ cmake -B build
$ cmake --build build
$ ctest --test-dir build --verbose
$ sudo cmake --install build
```

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `nogdb_BuildTests` | `ON` | Build functional and unit test suites |
| `nogdb_CXX17` | `OFF` | Build with C++17 instead of C++11 |
| `nogdb_BuildBenchmarks` | `OFF` | Build the micro-benchmark executable |

```bash
# Build with C++17
$ cmake -B build -Dnogdb_CXX17=ON
$ cmake --build build

# Build and run micro-benchmarks
$ cmake -B build -Dnogdb_BuildBenchmarks=ON
$ cmake --build build --target nogdb_bench
$ ./build/nogdb_bench
```

## Usage

Include the main header:

```cpp
#include <nogdb/nogdb.h>
```

Link against the library:

```
-lnogdb
```

### Quick Examples

```cpp
// Open / create a database
nogdb::ContextInitializer("/tmp/mydb").init();
nogdb::Context ctx("/tmp/mydb");

// Schema DDL
auto txn = ctx.beginTxn(nogdb::TxnMode::READ_WRITE);
txn.addClass("Person",   nogdb::ClassType::VERTEX);
txn.addClass("Knows",    nogdb::ClassType::EDGE);
txn.addProperty("Person", "name", nogdb::PropertyType::TEXT);
txn.addProperty("Person", "age",  nogdb::PropertyType::INTEGER);
txn.addProperty("Knows",  "since",nogdb::PropertyType::INTEGER);
txn.commit();

// Insert vertices and an edge
txn = ctx.beginTxn(nogdb::TxnMode::READ_WRITE);
auto alice = txn.addVertex("Person", nogdb::Record{}.set("name","Alice").set("age", int32_t{30}));
auto bob   = txn.addVertex("Person", nogdb::Record{}.set("name","Bob")  .set("age", int32_t{25}));
txn.addEdge("Knows", alice, bob, nogdb::Record{}.set("since", int32_t{2020}));
txn.commit();

// Query — find by condition
txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
auto rs = txn.find("Person")
             .where(nogdb::Condition("age").gt(int32_t{20}))
             .get();
for (auto& r : rs) { /* r.record.get("name").toText() */ }
txn.rollback();

// Query — lambda filter
txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
auto rs2 = txn.find("Person")
              .where([](const nogdb::Record& r) {
                  return r.get("age").toInt() > 20;
              })
              .get();
txn.rollback();

// BFS traversal
txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
auto bfs = txn.traverseOut(alice).depth(1, 3).get();
txn.rollback();

// DFS traversal
txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
auto dfs = txn.traverseOutDFS(alice).depth(1, 5).get();
txn.rollback();

// Shortest path (BFS unweighted)
txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
auto path = txn.shortestPath(alice, bob).get();
txn.rollback();

// Shortest path (Dijkstra, weighted)
txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
auto wpath = txn.shortestPath(alice, bob).withWeight("since").get();
txn.rollback();

// Batch insert
txn = ctx.beginBatchTxn();
for (int i = 0; i < 10000; ++i) {
    txn.addVertex("Person", nogdb::Record{}.set("name", std::string("user") + std::to_string(i)));
}
txn.commit();
```

## Documentation

See the [NogDB Docs](https://github.com/nogdb/nogdb/wiki) on Wiki pages for more details.

## Getting support
We are happy to help you using NogDB library. If you have any questions, you can find help by just simply [opening a Github issue here](https://github.com/nogdb/nogdb/issues) on this repository.

For feature requests and bug reports, before you create an issue, please search existing issues in case perhaps it has already been created or even fixed. When you are going to create feature requests, please be clear about your requirements. It would be perfect if you can provide some use cases and examples of proposing features. 
In the other hands, when you are going to create bug reports, please include information as much as possible. They could be, for example, the version of NogDB, details of your environment (OS, CPU, memory, the version of compiler, etc.), some test cases to demonstrate the issue, and/or anything you see it is useful to make us reproduce the problem easier if applicable.

## Contributing
Contributions are very welcome!

NogDB is an open source project that allows you to contribute to the project by creating additional features, or even fixing bugs if you see an issue and would like to implement it.

See the [Contributing Guidelines](https://github.com/nogdb/nogdb/blob/develop/CONTRIBUTING.md) for more details about the workflow, how to prepare your pull request, and some sorts of code conventions plus style, 

## License & copyright
Copyright (c) 2019 NogDB contributors.

![](https://www.gnu.org/graphics/agplv3-155x51.png)
![](https://www.openldap.org/images/headers/LDAPlogo.gif)

NogDB is licensed under the [GNU Affero General Public License v3.0](https://www.gnu.org/licenses/agpl-3.0.en.html). See the included LICENSE file for more details.

NogDB library build and distribution include LMDB, which is licensed under [The OpenLDAP Public License](http://www.openldap.org/software/release/license.html).
