/*
 *  Copyright (C) 2019, NogDB <https://nogdb.org>
 *  <nogdb at throughwave dot co dot th>
 *
 *  This file is part of libnogdb, the NogDB core library in C++.
 *
 *  libnogdb is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "func_test.h"
#include <ctime>
#include <sstream>

nogdb::Context* ctx = nullptr;
auto tnum = 0;

void exec(void (*func)(), const std::string& msg)
{
    const clock_t begin_time = clock();
    std::cout << "\x1B[32m+\x1B[0m " << msg << ".... ";
    (*func)();
    std::cout << "\x1B[32m(" << float(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << std::fixed
              << std::setprecision(3) << "ms)\x1B[0m"
              << std::endl;
    ++tnum;
}

void init_context()
{
    std::string dbname { DATABASE_PATH };
    try {
        ctx = new nogdb::Context(dbname);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void destroy_context()
{
    if (ctx) {
        delete ctx;
        ctx = nullptr;
    }
}

int main()
{
    // prepare test environment
    init();

    std::cout << "#######################################################\n"
              << "########          Start Running Tests          ########\n"
              << "#######################################################\n";
    const clock_t begin_time = clock();
    // ctx
#ifdef TEST_CONTEXT_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for a database context loading should:\x1B[0m\n";
    exec(test_context, "creating a new context");
    //exec(test_locked_ctx, "creating a new invalid context from a used one");
#else
    init_context();
#endif
    // schema
#ifdef TEST_SCHEMA_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for basic schema operations should:\x1B[0m\n";
    exec(test_create_class, "creating a class");
    exec(test_create_class_with_properties, "creating a class with pre-defined properties");
    exec(test_drop_class, "dropping a class");
    exec(test_alter_class, "modifying a class name");
    exec(test_create_invalid_class, "creating an invalid class");
    exec(test_create_invalid_class_with_properties, "creating an invalid class with pre-defined properties");
    exec(test_drop_invalid_class, "dropping an invalid class");
    exec(test_alter_invalid_class, "modifying an invalid class name");
    exec(test_add_property, "creating properties");
    exec(test_delete_property, "deleting properties");
    exec(test_add_invalid_property, "creating invalid properties");
    exec(test_delete_invalid_property, "deleting invalid properties");
    exec(test_alter_property, "modifying a property");
    exec(test_alter_invalid_property, "modifying an invalid property");
#endif
    // ctx
#ifdef TEST_CONTEXT_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for a database context copying and re-opening should:\x1B[0m\n";
    exec(test_reopen_ctx, "reopening a context");
    exec(test_ctx_move, "moving contexts");
#endif
    // type
#ifdef TEST_RECORD_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for types in a database should:\x1B[0m\n";
    exec(test_bytes_only, "converting primitive types to bytes");
    exec(test_record_with_bytes, "getting/setting bytes from/to record");
    exec(test_invalid_record_with_bytes, "getting values from record with invalid properties");
    exec(test_invalid_record_property_name, "setting values into record with invalid property names");
#endif
    // vertex
#ifdef TEST_RECORD_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for basic operations for vertices should:\x1B[0m\n";
    exec(test_create_vertex, "creating a vertex");
    exec(test_create_vertices, "creating vertices more than 1 class");
    exec(test_create_invalid_vertex, "creating an invalid vertex");
    exec(test_get_vertex, "retrieving data from vertices");
    exec(test_get_vertex_v2, "retrieving data from vertices belonging to a class with all property types");
    exec(test_get_invalid_vertices, "retrieving data from invalid vertices");
    exec(test_get_vertex_cursor, "retrieving data from vertices with result set cursor");
    exec(test_get_invalid_vertex_cursor, "retrieving data from invalid vertices with result set cursor");
    exec(test_get_edge_in, "retrieving incoming edges from a vertex");
    exec(test_get_invalid_edge_in, "retrieving incoming edges from an invalid vertex");
    exec(test_get_edge_out, "retrieving outgoing edges from a vertex");
    exec(test_get_invalid_edge_out, "retrieving outgoing edges from an invalid vertex");
    exec(test_get_edge_all, "retrieving incoming and outgoing edges from a vertex");
    exec(test_get_invalid_edge_all, "retrieving incoming and outgoing edges from an invalid vertex");
    exec(test_get_edge_in_cursor, "retrieving a cursor of incoming edges from a vertex");
    exec(test_get_invalid_edge_in_cursor, "retrieving a cursor of incoming edges from an invalid vertex");
    exec(test_get_edge_out_cursor, "retrieving a cursor of outgoing edges from a vertex");
    exec(test_get_invalid_edge_out_cursor, "retrieving a cursor of outgoing edges from an invalid vertex");
    exec(test_get_edge_all_cursor, "retrieving a cursor of incoming and outgoing edges from a vertex");
    exec(test_get_invalid_edge_all_cursor, "retrieving a cursor of incoming and outgoing edges from an invalid vertex");
    exec(test_update_vertex, "updating a vertex");
    exec(test_update_invalid_vertex, "updating an invalid vertex");
    exec(test_delete_vertex_only, "deleting a vertex (without edges)");
    exec(test_delete_all_vertices, "deleting all vertices in the same class");
    exec(test_delete_invalid_vertex, "deleting an invalid vertex");
#endif
    // edge
#ifdef TEST_RECORD_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for basic operations for edges should:\x1B[0m\n";
    exec(test_create_edges, "creating edges");
    exec(test_create_invalid_edge, "creating an invalid edge");
    exec(test_get_edge, "retrieving data from edges");
    exec(test_get_invalid_edges, "retrieving data from invalid edges");
    exec(test_get_edge_cursor, "retrieving data from edges with result set cursor");
    exec(test_get_invalid_edge_cursor, "retrieving data from invalid edges with result set cursor");
    exec(test_get_vertex_src, "retrieving source vertices");
    exec(test_get_invalid_vertex_src, "retrieving source vertices from an invalid edge");
    exec(test_get_vertex_dst, "retrieving destination vertices");
    exec(test_get_invalid_vertex_dst, "retrieving destination vertices from an invalid edge");
    exec(test_get_vertex_all, "retrieving source and destination vertices");
    exec(test_get_invalid_vertex_all, "retrieving source and destination vertices from an invalid edge");
    exec(test_update_edge, "updating an edge");
    exec(test_update_invalid_edge, "updating an invalid edge");
    exec(test_update_vertex_src, "updating a source vertex of an edge");
    exec(test_update_invalid_edge_src, "updating an invalid source vertex of an edge");
    exec(test_update_vertex_dst, "updating a destination vertex of an edge");
    exec(test_update_invalid_edge_dst, "updating an invalid destination vertex of an edge");
    exec(test_delete_edge, "deleting an edge");
    exec(test_delete_invalid_edge, "deleting an invalid edge");
    exec(test_delete_all_edges, "deleting all edges in the same class");
    exec(test_get_invalid_edge, "getting an invalid edge");
#endif
    // ctx
#ifdef TEST_CONTEXT_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for a database context (more complex) should:\x1B[0m\n";
    exec(test_reopen_ctx_v2, "reopening a context with records");
    exec(test_reopen_ctx_v3, "reopening a context with records and relations");
    exec(test_reopen_ctx_v4, "reopening a context with records, relations, and renaming class/property");
#endif
    // misc
#ifdef TEST_MISC_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for advanced operations should:\x1B[0m\n";
    exec(test_get_set_empty_value, "setting and getting an empty value of a record");
    exec(test_get_invalid_record, "getting an invalid record");
    exec(test_get_set_large_record, "setting and getting a large size of value in a record");
    exec(test_overwrite_basic_info, "setting values with overwritten basic info");
    exec(test_standalone_vertex, "getting in-edges and out-edges from a standalone vertex");
    exec(test_delete_vertex_with_edges, "deleting a vertex (with edges)");
    exec(test_delete_all_vertices_with_edges, "deleting all vertices in the same class (with edges)");
    exec(test_add_delete_prop_with_records, "adding/deleting properties with records");
    exec(test_alter_class_with_records, "modifying a class name with records");
    exec(test_drop_class_with_relations, "dropping a class with some relations and reloading the database");
    exec(test_get_count_vertex, "getting a number of vertex records in result set via count()");
    exec(test_get_count_edge, "getting a number of edge records in result set via count()");

    std::cout << "\n\x1B[96mEnd-to-end tests for create/update/delete operations with record versioning should:\x1B[0m\n";
    exec(test_version_add_vertex_edge, "adding new vertices and edges with record versioning");
    exec(test_version_update_vertex_edge, "updating vertices and edges with record versioning");
    exec(test_version_update_src_dst_edge, "updating src and dst vertices of edges with record versioning");
    exec(test_version_remove_vertex_edge, "removing vertices and edges with record versioning");
    exec(test_version_remove_all_vertex_edge, "removing all vertices and edges with record versioning");
    exec(test_version_drop_vertex_edge, "droping vertice and edge classes with record versioning");
#endif
    // graph
#ifdef TEST_GRAPH_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for graph operations should:\x1B[0m\n";
    exec(init_test_graph, "initiating a graph for testing graph operations");
    exec(test_create_complex_graph, "creating a complex graph");
    exec(test_get_edge_in_more, "retrieving incoming edges with multiple classname");
    exec(test_get_invalid_edge_in_more, "retrieving invalid incoming edges with multiple classname");
    exec(test_get_edge_out_more, "retrieving outgoing edges with multiple classname");
    exec(test_get_invalid_edge_out_more, "retrieving invalid outgoing edges with multiple classname");
    exec(test_get_edge_all_more, "retrieving incoming and outgoing edges with multiple classname");
    exec(test_get_invalid_edge_all_more, "retrieving invalid incoming and outgoing edges with multiple classname");
    exec(test_bfs_traverse_in, "traversing a graph using bfs algorithm with incoming edges");
    exec(test_invalid_bfs_traverse_in, "traversing a graph using bfs algorithm with incoming edges and invalid parameters");
    exec(test_bfs_traverse_out, "traversing a graph using bfs algorithm with outgoing edges");
    exec(test_invalid_bfs_traverse_out, "traversing a graph using bfs algorithm with outgoing edges and invalid parameters");
    exec(test_bfs_traverse_all, "traversing a graph using bfs algorithm with incoming and outgoing edges");
    exec(test_invalid_bfs_traverse_all, "traversing a graph using bfs algorithm with incoming and outgoing edges and invalid parameters");
    exec(test_bfs_traverse_in_cursor, "traversing a graph and returning a cursor using bfs algorithm with incoming edges");
    exec(test_invalid_bfs_traverse_in_cursor, "traversing a graph and returning a cursor using bfs algorithm with incoming edges and invalid parameters");
    exec(test_bfs_traverse_out_cursor, "traversing a graph and returning a cursor using bfs algorithm with outgoing edges");
    exec(test_invalid_bfs_traverse_out_cursor, "traversing a graph and returning a cursor using bfs algorithm with outgoing edges and invalid parameters");
    exec(test_bfs_traverse_all_cursor, "traversing a graph and returning a cursor using bfs algorithm with incoming and outgoing edges");
    exec(test_invalid_bfs_traverse_all_cursor, "traversing a graph and returning a cursor using bfs algorithm with incoming and outgoing edges and invalid parameters");
    exec(test_shortest_path, "finding the shortest path in a graph");
    exec(test_invalid_shortest_path, "finding the shortest path with invalid parameters");
    exec(test_shortest_path_cursor, "finding a cursor of the shortest path in a graph");
    exec(test_invalid_shortest_path_cursor, "finding a cursor of the shortest path with invalid parameters");
    // exec(test_shortest_path_dijkstra, "finding the shortest path with dijkstra's algorithm");
    exec(test_bfs_traverse_with_condition, "traversing a graph using bfs algorithm with conditional functions");
    exec(test_shortest_path_with_condition, "finding the shortest path in a graph with conditional functions");
    exec(test_bfs_traverse_cursor_with_condition, "traversing a graph and returning a cursor using bfs algorithm with conditional functions");
    exec(test_shortest_path_cursor_with_condition, "finding a cursor of the shortest path in a graph with conditional functions");
    exec(test_bfs_traverse_multi_edges_with_condition, "traversing a graph using bfs algorithm with conditional functions for multi-edge vertices");
    exec(test_bfs_traverse_multi_vertices, "traversing a graph using bfs algorithm with multi-vertex sources");
    exec(test_bfs_traverse_multi_vertices_with_condition, "traversing a graph using bfs algorithm with multi-vertex sources and conditions");
    exec(destroy_test_graph, "destroying the graph for testing graph operations");
#endif
    // find
#ifdef TEST_FIND_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for find operations should:\x1B[0m\n";
    exec(test_expression, "constructing condition/expression and filtering a record correctly");
    exec(test_range_expression, "constructing condition with range comparators correctly");
    exec(test_extra_string_expression, "constructing condition with additional comparators for string type correctly");
    exec(test_negative_expression, "constructing negative condition/expression and filtering a record correctly");
    exec(test_cmp_function_expression, "constructing expression with conditional comparing functions correctly");
    exec(init_test_find, "initiating a graph for testing find operations");
    exec(test_create_informative_graph, "creating an informative graph");
    exec(test_find_vertex, "finding records from a vertex class with a given condition");
    exec(test_find_invalid_vertex, "finding records from an invalid vertex class or an invalid condition");
    exec(test_find_edge, "finding records from an edge class with a given condition");
    exec(test_find_invalid_edge, "finding records from an invalid edge class or with an invalid condition");
    exec(test_find_vertex_cursor, "finding cursors from a vertex class with a given condition");
    exec(test_find_invalid_vertex_cursor, "finding cursors from an invalid vertex class or an invalid condition");
    exec(test_find_edge_cursor, "finding cursors from an edge class with a given condition");
    exec(test_find_invalid_edge_cursor, "finding cursors from an invalid edge class or with an invalid condition");
    exec(test_find_edge_in, "finding incoming edges from a vertex with a given condition");
    exec(test_find_invalid_edge_in, "finding incoming edges from an invalid vertex or with an invalid condition");
    exec(test_find_edge_out, "finding outgoing edges from a vertex with a given condition");
    exec(test_find_invalid_edge_out, "finding outgoing edges from an invalid vertex or with an invalid condition");
    exec(test_find_edge_all, "finding incoming and outgoing edges from a vertex with a given condition");
    exec(test_find_invalid_edge_all,
        "finding incoming and outgoing edges from an invalid vertex or with an invalid condition");
    exec(test_find_edge_in_cursor, "finding a cursor of incoming edges from a vertex with a given condition");
    exec(test_find_invalid_edge_in_cursor,
        "finding a cursor of incoming edges from an invalid vertex or with an invalid condition");
    exec(test_find_edge_out_cursor, "finding a cursor of outgoing edges from a vertex with a given condition");
    exec(test_find_invalid_edge_out_cursor,
        "finding a cursor of outgoing edges from an invalid vertex or with an invalid condition");
    exec(test_find_edge_all_cursor,
        "finding a cursor of incoming and outgoing edges from a vertex with a given condition");
    exec(test_find_invalid_edge_all_cursor,
        "finding a cursor of incoming and outgoing edges from an invalid vertex or with an invalid condition");
    exec(test_find_vertex_condition_function, "finding records from a vertex class with a given function condition");
    exec(test_find_invalid_vertex_condition_function,
        "finding records from an invalid vertex class with a given function condition");
    exec(test_find_vertex_cursor_condition_function,
        "finding cursors from a vertex class with a given function condition");
    exec(test_find_invalid_vertex_cursor_condition_function,
        "finding cursors from an invalid vertex class with a given function condition");
    exec(test_find_edge_condition_function, "finding records from an edge class with a given function condition");
    exec(test_find_invalid_edge_condition_function,
        "finding records from an invalid edge class with a given function condition");
    exec(test_find_edge_cursor_condition_function, "finding cursors from an edge class with a given function condition");
    exec(test_find_invalid_edge_cursor_condition_function,
        "finding cursors from an invalid edge class with a given function condition");
    exec(test_find_edge_in_condition_function, "finding incoming edges from a vertex with a given function condition");
    exec(test_find_invalid_edge_in_condition_function,
        "finding incoming edges from an invalid vertex with a given function condition");
    exec(test_find_edge_out_condition_function, "finding outgoing edges from a vertex with a given function condition");
    exec(test_find_invalid_edge_out_condition_function,
        "finding outgoing edges from an valid vertex with a given function condition");
    exec(test_find_edge_all_condition_function,
        "finding incoming and outgoing edges from a vertex with a given function condition");
    exec(test_find_invalid_edge_all_condition_function,
        "finding incoming and outgoing edges from an invalid vertex with a given function condition");
    exec(test_find_edge_in_cursor_condition_function,
        "finding a cursor of incoming edges from a vertex with a given function condition");
    exec(test_find_invalid_edge_in_cursor_condition_function,
        "finding a cursor of incoming edges from an invalid vertex with a given function condition");
    exec(test_find_edge_out_cursor_condition_function,
        "finding a cursor of outgoing edges from a vertex with a given function condition");
    exec(test_find_invalid_edge_out_cursor_condition_function,
        "finding a cursor of outgoing edges from an valid vertex with a given function condition");
    exec(test_find_edge_all_cursor_condition_function,
        "finding a cursor of incoming and outgoing edges from a vertex with a given function condition");
    exec(test_find_invalid_edge_all_cursor_condition_function,
        "finding a cursor of incoming and outgoing edges from an invalid vertex with a given function condition");
    exec(test_find_vertex_with_expression, "finding records from a vertex class with a given expression");
    exec(test_find_invalid_vertex_with_expression,
        "finding records from an invalid vertex class or an invalid expression");
    exec(test_find_vertex_cursor_with_expression, "finding cursors from a vertex class with a given expression");
    exec(test_find_invalid_vertex_cursor_with_expression,
        "finding cursors from an invalid vertex class or an invalid expression");
    exec(test_find_edge_with_expression, "finding records from an edge class with a given expression");
    exec(test_find_invalid_edge_with_expression,
        "finding records from an invalid edge class or with an invalid expression");
    exec(test_find_edge_cursor_with_expression, "finding cursors from an edge class with a given expression");
    exec(test_find_invalid_edge_cursor_with_expression,
        "finding cursors from an invalid edge class or with an invalid expression");
    exec(test_find_edge_in_with_expression, "finding incoming edges from a vertex with a given expression");
    exec(test_find_invalid_edge_in_with_expression,
        "finding incoming edges from an invalid vertex or with an invalid expression");
    exec(test_find_edge_out_with_expression, "finding outgoing edges from a vertex with a given expression");
    exec(test_find_invalid_edge_out_with_expression,
        "finding outgoing edges from an invalid vertex or with an invalid expression");
    exec(test_find_edge_all_with_expression, "finding incoming and outgoing edges from a vertex with a given expression");
    exec(test_find_invalid_edge_all_with_expression,
        "finding incoming and outgoing edges from an invalid vertex or with an invalid expression");
    exec(test_find_edge_in_cursor_with_expression,
        "finding a cursor of incoming edges from a vertex with a given expression");
    exec(test_find_invalid_edge_in_cursor_with_expression,
        "finding a cursor of incoming edges from an invalid vertex or with an invalid expression");
    exec(test_find_edge_out_cursor_with_expression,
        "finding a cursor of outgoing edges from a vertex with a given expression");
    exec(test_find_invalid_edge_out_cursor_with_expression,
        "finding a cursor of outgoing edges from an invalid vertex or with an invalid expression");
    exec(test_find_edge_all_cursor_with_expression,
        "finding a cursor of incoming and outgoing edges from a vertex with a given expression");
    exec(test_find_invalid_edge_all_cursor_with_expression,
        "finding a cursor of incoming and outgoing edges from an invalid vertex or with an invalid expression");
    exec(destroy_test_find, "destroying a graph for testing find operations");
#endif
    // inheritance
#ifdef TEST_INHERITANCE_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for inheritance operations should:\x1B[0m\n";
    exec(test_create_class_extend, "creating an extended class");
    exec(test_create_invalid_class_extend, "creating an invalid extended class");
    exec(test_alter_class_extend, "altering an extended class name");
    exec(test_drop_class_extend, "dropping a class with inheritance model");
    exec(init_all_extended_classes, "initiating extended classes of vertices and edges");
    exec(test_add_property_extend, "adding more properties into super and sub classes");
    exec(test_add_invalid_property_extend, "adding invalid properties into super and sub classes");
    exec(test_delete_property_extend, "deleting some properties from super and sub classes");
    exec(test_delete_invalid_property_extend, "deleting invalid properties from super and sub classes");
    exec(test_alter_property_extend, "modifying some properties in super and sub classes");
    exec(test_alter_invalid_property_extend, "modifying some invalid properties in super and sub classes");
    exec(test_create_vertex_edge_extend, "creating vertices and edges with extended class");
    exec(test_create_invalid_vertex_edge_extend, "creating invalid vertices and edges with extended class");
    exec(test_delete_vertex_edge_extend, "deleting vertices and edges with extended class");
    exec(test_get_class_extend, "getting records from extended classes");
    exec(test_find_class_extend, "finding records from extended classes");
    exec(test_traverse_class_extend, "traversing a graph with inheritance model");
    exec(test_shortest_path_class_extend, "finding the shortest path in a graph with inheritance model");
    exec(destroy_all_extended_classes, "destroying all extended classes of vertices and edges");
#endif
    // misc
#ifdef TEST_MISC_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for advanced operations with inheritance should:\x1B[0m\n";
    exec(test_drop_and_find_extended_class, "finding and parsing extended classes after class/property deletion");
    exec(test_conflict_property, "creating and getting conflict properties");
#endif
    // ctx
#ifdef TEST_CONTEXT_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for a database context with inheritance should:\x1B[0m\n";
    exec(test_reopen_ctx_v5, "reopening a context with records, relations, and extended classes");
#endif
    // indexing
#ifdef TEST_INDEX_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for database indexing should:\x1B[0m\n";
    exec(test_create_index, "creating indexes for some properties");
    exec(test_create_index_extended_class, "creating indexes for some properties which belong to super classes");
    exec(test_create_invalid_index, "creating invalid indexes for some properties");
    exec(test_drop_index, "dropping indexes for some properties");
    exec(test_drop_index_extended_class, "dropping indexes for some properties which belong to super classes");
    exec(test_drop_invalid_index, "dropping invalid indexes for some properties");
    exec(test_create_index_with_records, "creating indexes for some properties with existing records");
    exec(test_create_index_extended_class_with_records, "creating indexes for some properties which belong to super classes with existing records");
    exec(test_create_invalid_index_with_records, "creating invalid indexes with existing records");
    exec(test_drop_index_with_records, "dropping indexes for some properties with existing records");
    exec(test_drop_index_extended_class_with_records, "dropping indexes for some properties which belong to super classes with existing records");
    exec(test_drop_invalid_index_with_records, "dropping invalid indexes with existing records");
    exec(test_search_by_index_unique_condition, "getting records from unique indexing with condition");
    exec(test_search_by_index_non_unique_condition, "getting records from non-unique indexing with condition");
//    exec(test_search_by_index_unique_multicondition, "getting records from unique indexing with multi-condition");
//    exec(test_search_by_index_non_unique_multicondition, "getting records from non-unique indexing with multi-condition");
    exec(test_search_by_index_unique_cursor_condition, "getting cursor from unique indexing with condition");
    exec(test_search_by_index_non_unique_cursor_condition, "getting cursor from non-unique indexing with condition");
//    exec(test_search_by_index_unique_cursor_multicondition, "getting cursor from unique indexing with multi-condition");
//    exec(test_search_by_index_non_unique_cursor_multicondition, "getting cursor from non-unique indexing with multi-condition");
    exec(test_search_by_index_extended_class_condition, "getting records from indexing with extended class with condition");
    exec(test_search_by_index_extended_class_cursor_condition, "getting cursor from indexing with extended class with condition");
//    exec(test_search_by_index_extended_class_multicondition, "getting records from indexing with extended class with condition");
//    exec(test_search_by_index_extended_class_cursor_multicondition, "getting cursor from indexing with extended class with condition");
#endif
    // ctx
#ifdef TEST_CONTEXT_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for a database context with indexing should:\x1B[0m\n";
    exec(test_reopen_ctx_v6, "reopening a context with records, extended classes, and indexing");

    std::cout << "\n\x1B[96mEnd-to-end tests for multiple database contexts should:\x1B[0m\n";
    exec(test_multiple_ctx, "opening more than two contexts at the same time in the same process");
#endif
    // schema txn
#ifdef TEST_SCHEMA_TXN_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for a database schema with transaction should:\x1B[0m\n";
    exec(test_schema_txn_commit_simple, "committing schema txn with simple changes");
    exec(test_schema_txn_create_class_commit, "committing schema txn when creating a new class");
    exec(test_schema_txn_create_class_rollback, "aborting schema txn when creating a new class");
    exec(test_schema_txn_drop_class_commit, "committing schema txn when dropping an existing class");
    exec(test_schema_txn_drop_class_rollback, "aborting schema txn when dropping an existing class");
    exec(test_schema_txn_alter_class_commit, "committing schema txn when altering a class name");
    exec(test_schema_txn_alter_class_rollback, "aborting schema txn when altering a class name");
    exec(test_schema_txn_create_class_extend_commit, "committing schema txn when creating new sub class");
    exec(test_schema_txn_create_class_extend_rollback, "aborting schema txn when creating new sub class");
    exec(test_schema_txn_drop_class_extend_commit, "committing schema txn when dropping sub class");
    exec(test_schema_txn_drop_class_extend_rollback, "aborting schema txn when dropping sub class");
    exec(test_schema_txn_add_property_commit, "committing schema txn when adding a new property");
    exec(test_schema_txn_add_property_rollback, "aborting schema txn when adding a new property");
    exec(test_schema_txn_drop_property_commit, "committing schema txn when dropping a property");
    exec(test_schema_txn_drop_property_rollback, "aborting schema txn when dropping a property");
    exec(test_schema_txn_alter_property_commit, "committing schema txn when altering a property name");
    exec(test_schema_txn_alter_property_rollback, "aborting schema txn when altering a property name");
    exec(test_schema_txn_create_index_commit, "committing schema txn when creating a new index");
    exec(test_schema_txn_create_index_rollback, "aborting schema txn when creating a new index");
    exec(test_schema_txn_drop_index_commit, "committing schema txn when dropping an index");
    exec(test_schema_txn_drop_index_rollback, "aborting schema txn when dropping an index");
    exec(test_schema_txn_create_class_multiversion_commit, "committing multi-version schema txn when creating a new class");
    exec(test_schema_txn_create_class_multiversion_rollback, "aborting multi-version schema txn when creating a new class");
    exec(test_schema_txn_drop_class_multiversion_commit, "committing multi-version schema txn when dropping an existing class");
    exec(test_schema_txn_drop_class_multiversion_rollback, "aborting multi-version schema txn when dropping an existing class");
    exec(test_schema_txn_alter_class_multiversion_commit, "committing multi-version schema txn when altering a class name");
    exec(test_schema_txn_alter_class_multiversion_rollback, "aborting multi-version schema txn when altering a class name");
    exec(test_schema_txn_create_class_extend_multiversion_commit, "committing multi-version schema txn when creating new sub class");
    exec(test_schema_txn_create_class_extend_multiversion_rollback, "aborting multi-version schema txn when creating new sub class");
    exec(test_schema_txn_drop_class_extend_multiversion_commit, "committing multi-version schema txn when dropping sub class");
    exec(test_schema_txn_drop_class_extend_multiversion_rollback, "aborting multi-version schema txn when dropping sub class");
    exec(test_schema_txn_add_property_multiversion_commit, "committing multi-version schema txn when adding a new property");
    exec(test_schema_txn_add_property_multiversion_rollback, "aborting multi-version schema txn when adding a new property");
    exec(test_schema_txn_drop_property_multiversion_commit, "committing multi-version schema txn when dropping a property");
    exec(test_schema_txn_drop_property_multiversion_rollback, "aborting multi-version schema txn when dropping a property");
    exec(test_schema_txn_alter_property_multiversion_commit, "committing multi-version schema txn when altering a property name");
    exec(test_schema_txn_alter_property_multiversion_rollback, "aborting multi-version schema txn when altering a property name");
    exec(test_schema_txn_create_index_multiversion_commit, "committing multi-version schema txn when creating a new index");
    exec(test_schema_txn_create_index_multiversion_rollback, "aborting multi-version schema txn when creating a new index");
    exec(test_schema_txn_drop_index_multiversion_commit, "committing multi-version schema txn when dropping an index");
    exec(test_schema_txn_drop_index_multiversion_rollback, "aborting multi-version schema txn when dropping an index");
#endif
    // txn
#ifdef TEST_TXN_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for a database transaction should:\x1B[0m\n";
    exec(test_txn_commit_nothing, "committing txn with no changes");
    exec(test_txn_create_only_vertex_commit, "committing txn when creating a single vertex only");
    exec(test_txn_create_only_vertex_rollback, "aborting txn when creating a single vertex only");
    exec(test_txn_rollback_when_destroy, "aborting txn after performing destruction");
    exec(test_txn_delete_only_vertex_commit, "committing txn when deleting a single vertex only");
    exec(test_txn_delete_only_vertex_rollback, "aborting txn when deleting a single vertex only");
    exec(test_txn_create_only_edge_commit, "committing txn when creating a single edge only");
    exec(test_txn_create_only_edge_rollback, "aborting txn when creating a single edge only");
    exec(test_txn_delete_only_edge_commit, "committing txn when deleting a single edge only");
    exec(test_txn_delete_only_edge_rollback, "aborting txn when deleting a single edge only");
    exec(test_txn_get_vertex_edge, "getting vertices and edges with txn");
    exec(test_txn_alter_vertex_edge_commit, "committing txn when modifying vertices and edges");
    exec(test_txn_alter_vertex_edge_rollback, "aborting txn when modifying vertices and edges");
    exec(test_txn_create_only_vertex_multiversion_commit, "committing multi-version txn when creating a single vertex only");
    exec(test_txn_create_only_vertex_multiversion_rollback, "aborting multi-version txn when creating a single vertex only");
    exec(test_txn_delete_only_vertex_multiversion_commit, "committing multi-version txn when deleting a single vertex only");
    exec(test_txn_delete_only_vertex_multiversion_rollback, "aborting multi-version txn when deleting a single vertex only");
    exec(test_txn_create_edges_multiversion_commit, "committing multi-version txn when creating edges with vertices");
    exec(test_txn_create_edges_multiversion_rollback, "aborting multi-version txn when creating edges with vertices");
    exec(test_txn_delete_edges_multiversion_commit, "committing multi-version txn when deleting edges with vertices");
    exec(test_txn_delete_edges_multiversion_rollback, "aborting multi-version txn when deleting edges with vertices");
    exec(test_txn_modify_edges_multiversion_commit, "committing multi-version txn when modifying edges with vertices");
    exec(test_txn_modify_edges_multiversion_rollback, "aborting multi-version txn when modifying edges with vertices");
    exec(test_txn_reopen_ctx, "reopening context and committing txn with vertices and edges");
    exec(test_txn_invalid_operations, "committing txn with invalid operations");
#endif

    // sql
#ifdef TEST_SQL_OPERATIONS
    std::cout << "\n\x1B[96mEnd-to-end tests for sql operations should:\x1B[0m\n";
    exec(test_sql_unrecognized_token_error, "executing unrecognized token error sql command");
    exec(test_sql_syntax_error, "executing syntax error sql command");
    exec(test_sql_create_class, "creating a class with sql command");
    exec(test_sql_create_class_if_not_exists, "creating a class with sql command (if not exists)");
    exec(test_sql_create_class_extend, "creating an extended class with sql command");
    exec(test_sql_create_invalid_class, "creating an invalid class with sql command");
    exec(test_sql_alter_class_name, "modifying a class name with sql command");
    exec(test_sql_drop_class, "dropping a class with sql command");
    exec(test_sql_drop_class_if_exists, "dropping a class with sql command (if exists)");
    exec(test_sql_drop_invalid_class, "dropping an invalid class with sql command");
    exec(test_sql_add_property, "creating properties with sql command");
    exec(test_sql_alter_property, "modifying a property with sql command");
    exec(test_sql_delete_property, "deleting properties with sql command");
    exec(test_sql_create_vertex, "creating a vertex with sql command");
    exec(test_sql_create_edges, "creating edges with sql command");
    exec(test_sql_select_vertex, "retrieving data from vertices with sql");
    exec(test_sql_select_vertex_with_rid, "retrieving data from vertices with record descriptor with sql command");
    exec(test_sql_select_property, "retrieving data with specific property with sql command");
    exec(test_sql_select_count, "retrieving count of data with sql command");
    exec(test_sql_select_walk, "retrieving data by walk on graph with sql command");
    exec(test_sql_select_method_property, "retrieving data with method property with sql command");
    exec(test_sql_select_alias_property, "retrieving data with alias property with sql command");
    exec(test_sql_select_vertex_condition, "finding records from a vertex class with a given condition with sql command");
    exec(test_sql_select_vertex_with_multi_condition, "finding records from a vertex class with a given multi-condition with sql command");
    exec(test_sql_select_nested_condition, "finding records from vertex class by nested condition with sql command");
    exec(test_sql_select_skip_limit, "retrieving data with specific length with sql command");
    exec(test_sql_select_group_by, "retrieving data with 'group by' sql command");
    exec(test_sql_update_vertex_with_rid, "updating a vertex by rid with sql command");
    exec(test_sql_update_vertex_with_condition, "updating a vertex by condition with sql command");
    exec(test_sql_delete_vertex_with_rid, "deleting a vertex and edge around vertex by rid with sql command");
    exec(test_sql_delete_vertex_with_condition, "deleting a vertex and edge around vertex by condition with sql command");
    exec(test_sql_delete_edge_with_rid, "deleting an edge by rid with sql command");
    exec(test_sql_delete_edge_with_condition, "deleting an edge by condition with sql command");
    exec(test_sql_validate_property_type, "validating every property type on sql command");
    exec(test_sql_traverse, "traversing graph with sql command");
    exec(test_sql_create_index, "creating index with sql command");
    exec(test_sql_create_index_unique, "creating unique index with sql command");
    exec(test_sql_drop_index, "droping index with sql command");
#endif

    destroy_context();

    std::cout << "\n[\x1B[32mSuccess\x1B[0m] Test passed: " << tnum << "/" << tnum << ", "
              << "Time elapse: " << float(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << "ms\n";
}
