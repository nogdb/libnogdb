# NogDB

[![Build Status](https://travis-ci.org/nogdb/nogdb.svg?branch=develop)](https://travis-ci.org/nogdb/nogdb)
[![Version](https://img.shields.io/badge/version-0.11.0%20beta-blue.svg)]()
[![Production Status](https://img.shields.io/badge/status-unstable-red.svg)]()

## Welcome
**NogDB is a fast & lightweight native graph database library that provides features in graph manipulations.**

## Key Features
* Fast graph database written in C++
* Lightweight and native with no client-server protocols needed
* Rapid graph traversal with vertex and edge relations cached in memory
* Transactions with MVCC (full ACID semantics in data storage)
* Multiple readers and single writer (no blocking between readers and writer)
* SQL support for performing graph execution and querying graph information
* Inheritance support for vertex and edge classes
* Basic database indexing support based on B+Tree
* Multiple platforms support, e.g. Unix, Linux, BSD and Mac OS X (macOS)

## Dependency
* GCC (gcc/g++ 5.1.0 or above) or LLVM (clang/clang++) compiler that supports C++11
* On Windows, use MinGW-w64 (gcc/g++ 7.2, mingw-w64 5.0 or above) 

## Limitations
* A particular database can be opened and accessed by only one single process at a time. For a multi-threaded application, the database context must be created as a singleton and shared between threads. However, the application can open multiple different databases by using separated database contexts.
* The current datastore architecture supports only up to 65,535 classes and 65,536 properties.
* For a large graph database, a maximum size of a data storage may need to be customized and appropriately defined in advance.

## Build and Installation
### With CMake (recommended)
```
$ cmake .
$ cmake --build .
$ ctest --verbose
$ sudo make install
```

### With Makefile (debug mode by default)
```
$ sh install_make.sh
$ make && make test
$ sudo make install
$ make clean
```

## Usage
* include the following file in a program's header:
```
#include <nogdb/nogdb.h>
```
* include the following flags in Makefile:
```
-lnogdb
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
Copyright (c) 2018 NogDB contributors.

NogDB is licensed under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html). See the included LICENSE file for more details.

NogDB library build and distribution include LMDB, which is licensed under [The OpenLDAP Public License](http://www.openldap.org/software/release/license.html).
