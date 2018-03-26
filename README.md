# NogDB

[![Build Status](https://travis-ci.org/nogdb/nogdb.svg?branch=develop)](https://travis-ci.org/nogdb/nogdb)
[![Version](https://img.shields.io/badge/version-0.10.0%20beta-blue.svg)]()
[![Production Status](https://img.shields.io/badge/status-unstable-red.svg)]()

## Welcome
**NogDB is a fast & lightweight native graph database library that provides features in graph manipulations.**

## Key Features
* Fast graph database written in C++
* Lightweight and native with no client-server protocols needed
* Rapid graph traversal with vertex and edge relations cached in memory
* Transactions with MVCC (full ACID semantics in data storage)
* Multiple readers and single writer (no blocking between readers and writer)
* Inheritance support for vertex and edge classes
* Basic database indexing support based on B+Tree
* Multiple platforms support, e.g. Unix, Linux, BSD and Mac OS X (macOS)

## Dependency
* GCC (gcc/g++) or LLVM (clang/clang++ 3.8.0 or above) compiler that supports C++11

## Limitations
* A particular database can be opened and accessed by only one single process at a time. For a multi-threaded application, the database context must be created as a singleton and shared between threads. However, the application can open multiple different databases by using separated database contexts.
* The current datastore architecture supports only up to 65,535 classes and 65,536 properties.
* For a large graph database, a maximum size of a data storage may need to be customized and appropriately defined in advance.

## Build and Installation
### With Makefile
```
$ sh install_make.sh
$ make && make test
$ sudo make install
$ make clean
```

### With CMake
```
$ cmake .
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

## Contributing
Contributions are very welcome!

NogDB is an open source project that allows you to contribute to the project by creating additional features, or even fixing bugs if you see an issue and would like to implement it.

See the [Contributing Guidelines](https://github.com/nogdb/nogdb/blob/master/CONTRIBUTING.md) for more details about the workflow, how to prepare your pull request, and some sorts of code conventions plus style, 

## License & copyright
Copyright (c) 2018 NogDB contributors.

NogDB is licensed under the [GNU General Public License v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html). See the included LICENSE file for more details.

NogDB library build and distribution include LMDB, which is licensed under [The OpenLDAP Public License](http://www.openldap.org/software/release/license.html).