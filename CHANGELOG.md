# Change Log
## v0.11.0-beta
* General changes:
  * All `nogdb::Vertex::find*(...)` and `nogdb::Edge::find*(...)` functions are obsoleted and have been renamed to `nogdb::Vertex::get*(...)` and `nogdb::Edge::get*(...)` respectively.
  * `nogdb::Class::create(...)` and `nogdb::Class::createExtend(...)` with `PropertyMapType` functions are deprecated.
  * The maximum size of property value has been increased and now it can be up to 2,147,483,648 bytes (from 65,536 bytes previously).
  * Removing dash (-) from a set of valid characters for class name and property name. A valid name for class and property must be A-Z, a-z, 0-9, and underscore (_) but cannot begin with numbers (0-9).
* New features:
  * A member function `getProperties(...)` in `nogdb::Record` which returns a set of property names in a record is available.
  * Supporting SQL for graph manipulation, retrieval, and traversal via `nogdb::SQL::execute(...)`.
  * Each record now contains some basic information such as `@className`, `@recordId`, `@depth`, and `@version` by default. These values are read-only and unable to be overwritten. 
* Implemented enhancements:
  * Adding some class and property name restrictions.
  * Improving the way to internally handle a cursor pointer of `MDB_cursor` in order to prevent memory leak problems by applying RAII principle for `Datastore::CursorHandlerWrapper`.
  * Removing all C assertion in the library to prevent terminating the program unexpectedly when there are some internal errors occurring. Therefore, the library will throw an exception `CTX_INTERNAL_ERR` for any internal errors instead.
  * Storing `uint8_t` and `int8_t` as `uint32_t` and `int32_t` respectively in database indexing for all compilers and systems compatibility.
  * Using 
* Fixed bugs:
  * Fixing issue [#11](https://github.com/nogdb/nogdb/issues/11). A SQL syntax error found when using `TRAVERSE` clause inside `SELECT`.
  * Fixing issue [#12](https://github.com/nogdb/nogdb/issues/12). An exception `SQL_INVALID_PROJECTION` thrown after getting out-of-bound index in SQL syntax.
  * Fixing several bugs in database indexing retrieval operations.
  
## v0.10.0-beta [2018-03-24]
* General changes:
  * `nogdb::Condition` and `nogdb::Comparator` have been re-designed by combining them into one functionality as `nogdb::Comparator` has become member functions of `nogdb::Condition`, for example, `eq(...)`, `lt(...)`, `contain(...)`, and etc. This would help to support more comparators such as `in(...)`, `like(...)`, `regex(...)` and some comparators that utilize the advantages of NogDB indexing such as range search and `between(...)`.
  * A member function `hasTo(...)` in `nogdb::ResultSetCursor` has been renamed to `hasAt(...)` for a better meaningful function name.
* New features:
  * More comparators have been added, for instance, `in(...)`, `between(...)`, `like(...)` and `regex(...)`, as member functions of `nogdb::Condition`.
  * An implementation of NogDB indexing for properties in classes provides two more function APIs, `nogdb::Property::createInex(...)` and `nogdb::Property::dropIndex(...)`, that support creating and dropping unique/non-unique index in a database. This would help to increase a performance of searching for matching records in vertex and edge classes via `find(...)` and `findCursor(...)` functions.
  * `nogdb::Vertex::findIndex(...)`, `nogdb::Vertex::findCursorIndex(...)`, `nogdb::Edge::findIndex(...)`, and `nogdb::Edge::findCursorIndex(...)` have been added to function APIs to support getting records from index tables directly.
* Implemented enhancements:
  * Internal functions which are a part of LMDB interfaces can now directly support numeric types as a key. This would improve the performance by getting rid out of `string` to `unsigned int` conversions and also help to reduce disk space utilisation.
  * Since keys stored in LMDB are allowed to be numeric types, all position ids stored in data storage are kept as `unsigned int` keys and always sorted. A new record can be easily added at the last position of the associated data storage by using a LMDB APPEND flag which results in a small overhead and less time to perform the insert operation.
  * Eliminating the use of threads running underlying NogDB library by cleaning up obsoleted versions and increasing memory space when performing transaction commit and rollback.
  * The underlying representation of graph and transaction has been revised and improved in order to have better performance and support cleaning all obsoleted objects in memory without a separated graph cleaner thread.
  * The transaction for schema operations is now supported to allow CRUD for a database schema in the same transaction with CRUD for vertex and edge records.
* Fixed bugs:
  * Not throwing an exception `CTX_NOEXST_PROPERTY` when calling `getText(...)` with invalid properties or with an empty string as a result.
  * Removing an incorrect lock object in a locking mechanism of a transaction commit for edges.
  * Several bugs associated with database schema have been fixed during an implementation of schema with transaction.

## v0.9.0-alpha [2018-01-10]
* General changes:
  * All `nogdb::Db::getSchema` functions have been modified to support `nogdb::Context` as a parameter instead of `nogdb::Txn` in order to decrease a number of instructions when getting database schema (no transactions required).
  * A member function of `nogdb::Bytes` - `getBytes()` has been renamed to `getRaw()`.
  * Vertex and edge functions for creating multiple records at once have been removed in order to reduce the use of STL containers in function signature.
  * A function parameter `const std::string& className` (and also `const std::set<std::string>& className`) in all function signatures related to record retrieval have been removed in order to reduce the use of STL containers in function signature and make function APIs much simpler.
* New features:
  * A `NOT` operation as `operator!` for `nogdb::Condition` and `nogdb::Expression` has been implemented to fully support all possible conditions for record retrieval and conditional graph traversal functions.
  * New graph and record operation functions have been implemented to provide better memory utilization and avoid out-of-memory problems by supporting a returned object - `nogdb::ResultSetCursor` - that would be used to iterate through each metadata instead of returning the whole data records at once like `nogdb::ResultSet` which utilizes a huge size of memory to store both metadata and raw records.
  * A set of new member functions in regard to `nogdb::Record` which can return a value of properties in a record as a primitive type.
  * An empty record is now eligible for creating and updating vertex and edge records.
  * A new type - `nogdb::ClassFilter` - has been implemented for filtering class names in edge record retrieval with conditions.
* Implemented enhancements:
  * An implementation of `nogdb::Condition`, `nogdb::Expression`, `nogdb::PathFilter`, `nogdb::Bytes`, and `nogdb::Record` has been hidden by being moved from header files to internal source files.
* Fixed bugs:
  * Creating a new class with properties previously used an incorrect type to define a types of properties. This critical bug will cause a failure of property-type mapping when the database context is reloaded. The bug had been already fixed by using the actual property type instead of a class type when saving into a datastore.
  
## v0.8.0-alpha [2017-11-02]
* General changes:
  * Adding a graceful way to compile and build a library through `cmake` which supports building and installing software on multiple platforms.
  * `nogdb::Context::DBInfo` has been moved out from a database context and become a separate type `nogdb::DBInfo`.
  * Two member functions, `and_` and `or_`, for combining multiple `nogdb::Condition` and `nogdb::Expression` objects have been replaced by using overloading and/or operations such as `operator&&` and `operator||`.
  * All functions in record operations have `nogdb::Txn` as the first parameter instead of `nogdb::Context` to support a new graph transaction feature. 
* New features:
  * A graph transaction has been implemented and now available through `nogdb::Txn` on all record operations.
* Implemented enhancements:
  * An implementation of `RecordIdHash` has been represented as `boost::hash<RecordId>` due to a better performance while only relevant boost header files are included in the lib directory under the project. 
* Fixed bugs:
  * There was no mutex locks in `getDbInfo()` which may cause concurrency problem. The concurrency control on database information can be done by an implementation of a mutex over `nogdb::DBInfo` in a database context together with a transaction.
  * `nogdb::Context` cannot be initiated as rvalue by using moving constructor or assigned as rvalue by using move assignment correctly. This has been fixed by completing the rule of five in `nogdb::Context`.

## v0.7.0-alpha [2017-08-12]
* General changes:
  * All function API interfaces have been changed and all internal implementations have been significantly refactored.
  * `nogdb::current_timestamp` might not allow to be used publicly as a function api.
  * All internal implementations are now hidden from a client program.
* Implemented enhancements:
  * An internal implementation and logic related to retrieving records by path filtering has been separated from each graph traversal algorithm.
  * A usage of `boost::hash<RecordId>` has been replaced with an internal implementation of `RecordIdHash`. 
  * All usages of boost library have been removed so that a size of nogdb library and a number of dependencies will be decreased.
* Fixed bugs:
  * Makefile has been improved to be suitable for source code after refactoring.
  * Makefile has been also fixed to appropriately build a static library according to a static library compilation problem with a client program.
  * Makefile has been also fixed for uninstalling `libnogdb.*` in /usr/local/lib.
  
## v0.6.0-alpha [2017-07-17]
* General changes:
  * The majority of coding style has been improved to increase readability and support both GCC and LLVM compilers.
  * `nogdb::Executor::traverse_*` receives both `minDepth` and `maxDepth` as parameters rather than only `depth` to specify the lower and upper boundaries of graph traversal.
  * Adding two additional numeric types for properties such as `nogdb::PropertyType::SHORTINT` for signed 16-bit integer and `nogdb::PropertyType::UNSIGNED_SHORTINT` for unsigned 16-bit integer.
* New features:
  * The operations with `nogdb::Expression` have been implemented. It allows all operations having `nogdb::Condition` as their paramters to accept `nogdb::Expression` for considering only records that match expressions taking an effect of the operations as well.
  * The old operations `nogdb::Executor::traverse_*` have been changed to `nogdb::Executor::bfs_traverse_*` since they used breath first search algorithm as their internal implementation.
  * Graph operations `nogdb::Executor::dfs_traverse_*` have been implemented and introduced for graph traversal using depth first search algorithm as their internal implementation.
  * `nogdb::Comparator` provides some additional comparators in the following pattern - `nogdb::Comparator::*_CASE` - for string matching with case sensitive. 
  * All `nogdb::Executor::find_*` and `nogdb::Executor::find_edge_*` functions can now support a conditional function in order to search for records by using customised and complicated conditions which are defined in `bool condition(const nogdb::Record& record)`. 
  * All `nogdb::Executor::find_*` and `nogdb::Executor::find_edge_*` functions can now support having a class name as `@className` and a record id as `@recordId` in `nogdb::Condition` and `nogdb::Expression` in order to search for records in a specific class and record id with particular conditions.
  * A path filtering - `nogdb::PathFilter` - has been implemented and available for filtering paths in graph traversal which uses only relevant vertices and edges that match with conditions defined inside a pointer to function `bool condition(const nogdb::Record& record)`.
* Implemented enhancements:
  * Getting raw data from the datastore directly while performing graph traversal and finding the shortest path in graph algorithm functions so that it can support a path filtering mechanism.
  
## v0.5.0-alpha [2017-06-10]
* General changes:
  * `nogdb::exec::create_class` and `nogdb::exec::add_property` throw an exception `CTX_LIMIT_DBSCHEMA` when a limitation of a database schema has been reached instead of terminating a program by assertion.
  * All find operations - `nogdb::exec::find_*` - have been changed to accept `nogdb::condition` and `nogdb::expr` as one of parameters for selecting matched records instead of passing 3 parameters together such as `std::string propName`, `T value`, and `nogdb::comparator cmp` for constructing a condition. An internal implemenation of all find operations have also been improved to support `nogdb::condition` (but not yet for `nogdb::expr`).
  * `struct utils` has been removed so that everyone can use all utility-related functions directly.
* New features:
  * `nogdb::exec::create_vertex` and `nogdb::exec::create_edge` can now support multiple-record insertion (bulk operations).
  * `nogdb::exec::delete_vertex` and `nogdb::exec::delete_edge` can now support an operation for deleting all records in a class.
  * Adding two additional numeric types for properties such as `nogdb::proptype::tinyint` for signed 8-bit integer and `nogdb::proptype::utinyint` for unsigned 8-bit integer.
  * `nogdb::classtype` has been renamed to `nogdb::class_type`, `nogdb::proptype` has been renamed to `nogdb::property_type`, and `nogdb::cmp` has been renamed to `nogdb::comparator` for better understanding in term of naming and convention.
  * `nogdb::condition` and `nogdb::expr` have been implemented and introduced. They can be useful for searching records in vertices and edges with multiple conditions in a complex expression.
* Implemented enhancements:
  * A context lock file has been implemented in order to prevent a database context from being opened and used by more than one process or thread. This will restrict a multi-thread program to use the same context to connect to a particular database and not allow other programs to open and use the same database simultaneously. This property can strongly preserve a database consistency for in-memory schema and relations by belonging to only one process.
  * Replacing all numeric primitive data types (such as `int`, `unsigned int`, `long`, and `unsigned long`), whose size could be dependent on compilers and cpu architecture, with standard fixed size types (such as `int32_t`, `uint32_t`, `int64_t` and `uint64_t`) so that the datastore can be compatible with multiple platforms and with the same representation of numeric types in term of their size.
* Fixed bugs:
  * An internal generic function `parse_raw_data` previously converted raw data of deleted properties in a record incorrectly. This bug has been fixed by correcting the way to calculate an offset of each property block.
  
## v0.4.0-alpha [2017-05-28]
* General changes:
  * A new implementation of the database configuration and variables to prevent any external accesses to those values by replacing `struct default_conf` with `class db_constant`.
  * The constructor of `nogdb::context` optionally accepts 2 more variables, `maxDbNum` and `maxDbSize`, for configuring a maximum number of internal opened databases and a maximum size of databases respectively.
  * `classname` has been removed from `struct record_descriptor`.
  * An exception `CTX_RID_ERR` has been removed from a list of exceptions and it will never be thrown from the database since there are no problems regarding a mismatch between classname and rid in `struct record_descriptor` due to the removal of `classname` in `struct record_descriptor`. 
  * `nogdb::class_descriptor` has been wrapped by a new `nogdb::class_descriptor` for hiding some internal representations and pointers. There are some changes and differences between the old and new `nogdb::class_descriptor`, such as inheritance support and static values for all members of `nogdb::class_descriptor`. 
* New features:
  * `nogdb::exec::traverse_all` has been implemented for graph traversal considering both directions of edges (incoming and outgoing).
  * `nogdb::exec::create_class_extend` has been available for creating a subclass which extends from its superclass and also supporting inheritance concept.
  * `nogdb::exec::alter_class` has been available for renaming a class.
  * `nogdb::exec::create_class` can now return `struct class_descriptor` and `nogdb::exec::add_property` can also return `struct property_descriptor`.
* Implemented enhancements:
  * The function signature of `nogdb::exec::get_schema` has been changed from `context& ctx` to `const context& ctx` in order to prevent any modifications that could be made in the function.
  * `nogdb::exec::get_vertex`, `nogdb::exec::get_edge`, `nogdb::exec::find_vertex`, and `nogdb::exec::find_edge` can support record retrieval for multiple classes.
  * An internal implementation of a relation (graph) has been changed by converting `class graph` into a class with non-static member functions.
  * An internal implementation of `nogdb::schema` has been modified to be a class, instead of `typedef`, with direct and inverse mapping between class names to `class_descriptor` and class ids to `class_descriptor`.
  * Added a proper validation process.
  * All duplication removal methods regarding class and property passing via function parameters have been removed in order to reduce a number of validation steps by restricting client programs to use only `std::set<std::string>` for passing a set of class and property names as `std::string`.
  * Several internal implementations have been added and updated for supporting extended classes in terms of schema and record operations.
* Fixed bugs:
  * The program could get into an undefined stage and throw some exceptions caused by an invalid `classId` referencing in `nogdb::exec::create_class` with predefined properties. `nogdb::exec::create_class` can now create a class with predefined properties using a valid classId from `nogdb::context::dbinfo`.
  * Dropping a class of vertices using `nogdb::drop_class` actually removed data records of vertices only. However, their relations and associated edges were still remaining in the datastore and would be incorrectly loaded into a memory when the context was re-initialized which may cause some undefined behaviour. `nogdb::drop_class` can now delete all edges associated with a dropped vertex class in the datastore as well as the in-memory relations.    

## v0.3.0-alpha [2017-04-12]
* General changes:
  * `class exec` has been corrected and changed to `struct exec`.
  * `nogdb::exec::get_vertices` and `nogdb::exec::get_edges` have been renamed to `nogdb::exec::get_vertex` and `nogdb::exec::get_edge` respectively.
  * all LMDB source codes have been merged into the project; as a result, no dependencies except `boost` required to be manually installed before compiling `nogdb` library.
  * `CTX_FORBIDDEN_CLASSTYPE` and `CTX_FORBIDDEN_PROPTYPE` have been changed to `CTX_INVALID_CLASSTYPE` and `CTX_INVALID_PROPTYPE` respectively.
  * `nogdb::exec::update_edge_src` and `nogdb::exec::update_edge_dst` have been renamed to `nogdb::exec::update_vertex_src` and `nogdb::exec::update_vertex_dst` respectively for better understanding.
  * Makefile has been created and modified to support some popular Unix and Linux platforms such as macOS, Ubuntu, RHEL/CentOS, and BSD.
  * The compiler has been changed from GNU (gcc/g++) to LLVM (clang/clang++).
* New features:
  * Some major primitive data types such as `integer`, `unsigned int`, `double`, `string`, and etc., including a binary object as an array of `unsigned char` can be converted into `nogdb::bytes` through a constructor of `nogdb::bytes`.
  * `convert_to(T& object)` has become available for converting `nogdb::bytes` to an object.
  * `nogdb::exec::get_vertex_all` has been added for getting both source and destination vertices of an edge.
  * `nogdb::exec::get_edge_all` has been added for getting edges by considering both directions.
  * Vertex, edge, and record retrieval with a given condition is now available, for example, `nogdb::exec::find_vertex`, `nogdb::exec::find_edge`, `nogdb::exec::find_edge_in`, `nogdb::exec::find_edge_out`, and `nogdb::exec::find_edge_all`.
* Implemented enhancements:
  * Increasing a performance of edge record retrieval by reducing a number of created objects as a parameter when passing through a function `get_resolved_records` for getting records from `record_descriptor`.
  * A duplication in an implementation of edge retrieval functions has been revised and eliminated.
  * `nogdb::exec::create_class`, `nogdb::exec::add_property`, and `nogdb::exec::alter_property` can now prevent using an empty string as classname and property name for creating classes and properties. Additional exceptions, `CTX_INVALID_CLASSNAME` and `CTX_INVALID_PROPERTYNAME`, will be returned from those schema operations if an empty string provided.  
* Fixed bugs:
  * `nogdb::exec::update_vertex_dst` previously threw an exception of `type std::out_of_range: map::at:  key not found`. This problem has been resolved by changing `oldTarget` in `graph::alter_edge_dst` from `edge->source.lock()` to `edge->target.lock()` and now it can refer to the right direction.
  * Once an empty string has been set and stored into a database, calling `record.get(...).to_text()` can return an empty string instead of aborting a program with an assertion failure.
  * `nogdb::utils::current_timestamp` can now return the correct timestamp in milliseconds since the Unix epoch.
 
## v0.2.0-alpha [2017-03-05]
* General changes:
  * `nogdb::command` and `class command` have been renamed to `nogdb::exec` and `class exec` respectively.
  * `class_name` in `nogdb::record_descriptor` has been changed to `classname`. 
  * A new nogdb type, `nogdb::result`, has been separated from `nogdb::result_set` containing two attributes, `nogdb::record_descriptor` and `nogdb::record`.
  * A return type of `nogdb::exec::get_vertex_src` and `nogdb::exec::get_vertex_dst` has been changed to `nogdb::result`.
  * A member function `is_null()` in `nogdb::bytes` and `nogdb::record` has been changed to `empty()`.
  * A member function `length()` in `nogdb::bytes` has been changed to `size()`.
  * Some member variables and functions of internal classes has been modified, for example:
    * modified a representation of `graph::rid` from `std::pair<classid, oid>` to `std::pair<clsid, posid>` in `class graph`.
    * modified member variables in `graph::relations` from `v` and `e` to `vertices` and `edges` respectively in `class graph`.
    * modified member variables in `graph::edge` from `src` and `dst` to `source` and `target` respectively in `class graph`.
    * modified member functions in `dstore::blob` from `value()` to `bytes()` in `class dstore::blob`.
    * modified member variables in `dstore::default_value` from `flags` to `flag` in `class dstore`.
  * Code cleaning and reorganisation.
* New features:
  * `nogdb::create_class` has `std::map<std::string, nogdb::proptype>` as an optional parameter to create properties of a class within the same function after the class has been successfully created.
  * `nogdb::exec::get_edge_in` and `nogdb::exec::get_edge_out` have a classname (or a set of classnames) of edges as an optional parameter for returning only a set of results related to specified edges.
  * `nogdb::record` has some additional member functions such as `clear()` - to clear all properties and values, and `unset(const std::string& className)` - to clear a specific property and its value.
  * `nogdb::traverse_in` and `nogdb::traverse_out` are available for graph traversal.
  * `nogdb::shortest_path` is also available for finding a shortest path between two vertices in a graph.
* Implemented enhancements:
  * A graph library has been changed from BGL to an internal implementation for graph traversal development compatibility.
  * Read/write locks for database operations have been improved to prevent concurrency problems.
* Fixed bugs:
  * `boost::remove_vertex` no longer causes a Segmentation Fault issue in `drop_class()` since a graph library has been changed.

## v0.1.0-alpha [2017-01-09]
* New features:
  * All features are implemented as specified in a documentation.
* Implemented enhancements:
  * No enhancements implemented in this version.
* Fixed bugs:
  * No bugs fixed in this version.
