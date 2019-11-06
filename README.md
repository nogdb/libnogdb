# NogDB - Native Graph Database (C++) Library 

[![Gitter](https://img.shields.io/gitter/room/gitterHQ/gitter.svg)](https://gitter.im/nogdb/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Build Status](https://travis-ci.org/nogdb/libnogdb.svg?branch=develop)](https://travis-ci.org/nogdb/libnogdb)
[![Version](https://img.shields.io/badge/version-1.2.0%20beta-blue.svg)]()
[![Production Status](https://img.shields.io/badge/status-stable-green.svg)]()

## Welcome
**NogDB is a fast & lightweight native graph database library that provides features in graph manipulations.**

## Key Features
* Considerably fast graph database written in C++
* Lightweight and native with no client-server protocols needed
* Transactions with MVCC (full ACID semantics in data storage)
* Multiple readers and single writer (no blocking between readers and writer)
* SQL support for performing graph executions and querying graph information
* Inheritance support for vertex and edge classes
* Basic database indexing support based on B+Tree
* Aim to support for multiple platforms (currently available on Unix, Linux, BSD, and Mac OS X/macOS, Windows is under experiment)

## Dependencies
* GCC (gcc/g++ 5.1.0 or above) or LLVM (clang/clang++) compiler that supports C++11
* On Windows, use MinGW-w64 (gcc/g++ 7.2, mingw-w64 5.0 or above)
* [Google Test](https://github.com/google/googletest) - for development only

## Limitations
* The current data storage architecture supports only up to 65,535 classes and 65,536 properties.
* For a large graph database, a maximum size of a data storage may need to be customized and appropriately defined in advance.

## Build and Installation
```
$ git clone https://github.com/nogdb/nogdb
$ cd nogdb
$ cmake .
$ cmake --build .
$ ctest --verbose
$ sudo make install
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
Copyright (c) 2019 NogDB contributors.

![](https://www.gnu.org/graphics/agplv3-155x51.png)
![](https://www.openldap.org/images/headers/LDAPlogo.gif)

NogDB is licensed under the [GNU Affero General Public License v3.0](https://www.gnu.org/licenses/agpl-3.0.en.html). See the included LICENSE file for more details.

NogDB library build and distribution include LMDB, which is licensed under [The OpenLDAP Public License](http://www.openldap.org/software/release/license.html).
