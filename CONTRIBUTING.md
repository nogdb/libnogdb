# Contributing to the NogDB core source code 

We'd love to say **"You are welcome to contribute to NogDB!"**. Just three simple steps that you need to follow before we accept your code:
- Fork the NogDB official repository (normally from `develop` branch).
- Make your change and develop on a feature branch of your fork.
- Submit your pull request and provide a description about your change.

Once you have submitted your pull request to `develop`, we will start the discussion, request for some changes (if any, but we'd like to submit your PR straight away),
and merge it. Please make sure that you basically understand and follow our development guideline prior to the beginning of your contribution.

## Contributor License Agreements

We need to add you to the list of contributors if you want to contribute to NogDB. It is compulsory to sign [our CLA](http://nogdb.org/community/cla) before starting the contribution. 
Please take a few second to access the CLA and sign it.

## Workflow
We simply follow Gitflow workflow by Vincent Driessen for branching model. There are six kinds of branches in our project:
 - `master` - The most stable branch that is ready to use in production.
 - `develop` - The most up-to-date branch that contains latest changes which pass all tests (but not yet ready for production).
 - `release/<version>` - The stable branch with a particular version as a snapshot of `develop` that contains some patches which are ready to use in production.
 - `feature/<what to implement>` - The unstable branch for feature development (highly not recommended to use in production). 
 - `hotfix/<what to fix>` - The branch that is created for fixing issues in `master`.
 - `document/<what to add>` - The branch that contains only changes in documentation.

## The project structure
We are currently working on six main directories:
 - `include` - All essential header files (*.h) that are published and can be accessed externally. Noted that adding only `#include <nogdb/nogb.h>` in a client program's header file should be sufficient to use all library functions. It is recommended to create a new header file if its underlying declaration and definition cannot fit into any existing ones.
 - `src` - All library implementation including internal header files (\*.hpp) and source files (\*.cpp) which underlying declaration and definition must not be exposed and should be potentially accessed by internal functions only.
 - `lib` - All library dependencies which are strongly required and not originally written by NogDB developers or contributors. The original source code and implementation must be preserved in a way that some changes could be made for compatibility and flexibility in term of external library usage, but still under the original license & copyright requirements.
 - `test` - All header files (\*.h) and source files (\*.cpp) for testing purposes. 
 - `doc` - All documentation and manual related files.
 - `benchmark` - All header files (\*.h) and source files (\*.cpp) for performance benchmarking.

New header and source files can be added if necessary. Make sure that they would be placed in appropriate directories with correct file extensions. Sub directories can be created in any directories except `include` and `src` which we intend to keep them in a flat structure.

## Code style
We usually develop NogDB by following [CppCoreGuidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md). However, we are pleased to accept any of your code style if it is quite reasonable and doesn't break any C++ best practice and common rules. Only few things that we strongly concern:
 - Use `clang-format` for the default formatting style complying with WebKit's style guide.
 ```bash
 clang-format -i -style=WebKit *
 ```
 - For namespace, always use lower-case letters without special characters and try to have only single word.
 ```cpp
 // good
 namespace nogdb {
    namespace inner {
    }
 }
 
 // okay but please avoid
 namespace nogdb {
    namespace inner_nogdb {
    }
 }
 
 // bad
 namespace nogdb {
    namespace InnerNogDB {
    }
 }
 ```
 - For type/class/struct, always use Pascal case with reasonable name (should be related to what it does).
 ```cpp
 // good
 class MyNewClass;
 
 // bad
 class my_new_class;
 class myNewClass;
 ```
 - For function/variable, always use Camel case with reasonable name (should be related to what it does).
 ```cpp
 // good
 void writeToFile(const std::vector<RecordDescriptor>& vertexDescriprtors);
 
 // bad
 void write_to_file(std::vector<RecordDescriptor>& vertex_descriptors);
 ```
 - Use `auto` as much as possible to increase code readability.
 ```cpp
 // good
 auto recordDescriptors = std::vector<RecordDescriptor>{};
 
 // bad
 std::vector<RecordDescriptor> recordDescriptors;
 ```
 - Use const pass-by-reference as much as possible to avoid over-copying objects.
 ```cpp
 // good
 void writeToFile(const std::vector<RecordDescriptor>& vertexDescriprtors);
 
 // bad
 void writeToFile(std::vector<RecordDescriptor> vertexDescriprtors);
 ```
 - Prefer RAII, e.g. choose smart pointers rather than raw pointers to prevent memory leak problems.
 ```cpp
 // good
 auto recordDescriptorPtr = std::make_shared<RecordDescriptor>();
 
 // bad
 RecordDescriptor *recordDescriptorPtr = new RecordDescriptor();
 ```
 - Follow the rule of three/five/zero as much as possible. Declare its copy constructor and assignment operator with `= delete` if your type/class/struct is non-copyable.
 - Use `#pragma once` instead of a define guard with `#ifndef` and `#define` in header files.
 - Prefer to include header files in the following order:
   - C system headers (`<unistd.h>`, etc.)
   - Standard C++ headers (`<vector>`, `<algorithm>`, etc.)
   - Local (internal) headers (`"generic.hpp"`, `"graph.hpp"`, etc.)
   - Global (exposing) headers (`"nogdb.h"`, `"nogdb_errors.h"`, etc.)
 - Prefer to include all dependencies to in \*.h/\*.hpp/\*.cpp files if only required.
 ```cpp
 // in search.hpp
 #include <iostream>    // <-- bad; don't use anywhere
 #include <vector>      // <-- good
 #include <string>      // <-- good
 #include <algorithm>   // <-- bad; don't use in this file
 
 bool searchString(const std::vector<std::string>& data);
 
 // in search.cpp
 #include <vector>      // <-- avoid; already have in search.hpp
 #include <string>      // <-- avoid; already have in search.hpp
 #include <algorithm>   // <-- good
 
 #include "search.hpp"
 
 bool searchString(const std::vector<std::string>& data) {
    std::find_if(....);
    ...
 }
 ```

## Submit your PR
1. Once your patches are ready, you then need to submit your pull request to the official repository. Make sure that you have already signed our contributors license agreement above.

2. Re-check your branch name and decide what kind of changes you want to submit.
   1. If your changes are related to performance enhancement, new features, adding more tests, and fixing non-critical bugs in `develop`, you should have your branch prefix name as `feature/` and generate a pull request from your branch against `develop`.
   2. If your changes are all about fixing critical issues in production, you should have your branch prefix name as `hotfix/` and generate a pull request from your branch against `master`.
   3. In a similar way, if your changes are in documentation, you should have your branch name as `document/` and generate a pull request from your branch against `develop`.

3. It is important to add your pull request details of your change including the appropriate tests especially for your change. Your pull request cannot be merged if there are no tests provided for your change or it causes any test failures.

4. Include the issue number you worked on (if applicable) and the name you used to sign the CLA.

## Continuous integration testing
NogDB uses Travis CI for continuous integration testing. It significantly verifies the build and performs the test process via Makefile on multiple platforms with different compilers for `master` and `develop` including any branches beginning with `feature/*`, `hotfix/*`, and `release/*`.
We also test the NogDB library with memory profiler such as Valgrind and AddressSanitizer to troubleshoot memory leak problems. Ensure that your pull request can pass all continuous integration testing modules. Check out [.travis.yml](https://github.com/nogdb/nogdb/blob/master/.travis.yml) to fully see how the code is built and tested.
Again, your changes in Makefile and CI scripts are not forbidden. We'd love to see it improved if you idea is awesome.

