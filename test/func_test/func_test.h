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

#pragma once

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "nogdb/nogdb.h"

#include "func_test_config.h"
#include "func_test_cursor_utils.h"
#include "func_test_index_utils.h"
#include "func_test_utils.h"

extern nogdb::Context* ctx;

// context operations testing
#ifdef TEST_CONTEXT_OPERATIONS
extern void test_context();
extern void test_ctx_move();
extern void test_reopen_ctx(); // without records
extern void test_reopen_ctx_v2(); // with records
extern void test_reopen_ctx_v3(); // with records and relations
extern void test_reopen_ctx_v4(); // with records, relations, and renaming class/property
extern void test_reopen_ctx_v5(); // with records, relations, and extended classes
extern void test_reopen_ctx_v6(); // with records, extended classes, and indexing
// extern void test_locked_ctx();
extern void test_invalid_ctx();
extern void test_multiple_ctx();

#endif

// schema operations testing
#ifdef TEST_SCHEMA_OPERATIONS
extern void test_create_class();
extern void test_create_class_with_properties();
extern void test_drop_class();
extern void test_alter_class();
extern void test_create_invalid_class();
extern void test_create_invalid_class_with_properties();
extern void test_drop_invalid_class();
extern void test_alter_invalid_class();
extern void test_add_property();
extern void test_delete_property();
extern void test_add_invalid_property();
extern void test_delete_invalid_property();
extern void test_alter_property();
extern void test_alter_invalid_property();
#endif

// record operations testing
#ifdef TEST_RECORD_OPERATIONS
extern void test_bytes_only();
extern void test_record_with_bytes();
extern void test_invalid_record_with_bytes();
extern void test_invalid_record_property_name();
extern void test_create_vertex();
extern void test_create_vertices();
extern void test_create_invalid_vertex();
extern void test_get_vertex();
extern void test_get_vertex_v2();
extern void test_get_invalid_vertices();
extern void test_get_vertex_cursor();
extern void test_get_invalid_vertex_cursor();
extern void test_update_vertex();
extern void test_update_invalid_vertex();
extern void test_delete_vertex_only();
extern void test_delete_invalid_vertex();
extern void test_delete_all_vertices();
extern void test_create_edges();
extern void test_create_invalid_edge();
extern void test_get_edge();
extern void test_get_invalid_edges();
extern void test_get_edge_cursor();
extern void test_get_invalid_edge_cursor();
extern void test_get_vertex_src();
extern void test_get_vertex_dst();
extern void test_get_vertex_all();
extern void test_get_invalid_vertex_src();
extern void test_get_invalid_vertex_dst();
extern void test_get_invalid_vertex_all();
extern void test_get_edge_in();
extern void test_get_edge_out();
extern void test_get_edge_all();
extern void test_get_invalid_edge_in();
extern void test_get_invalid_edge_out();
extern void test_get_invalid_edge_all();
extern void test_get_edge_in_cursor();
extern void test_get_edge_out_cursor();
extern void test_get_edge_all_cursor();
extern void test_get_invalid_edge_in_cursor();
extern void test_get_invalid_edge_out_cursor();
extern void test_get_invalid_edge_all_cursor();
extern void test_update_edge();
extern void test_update_invalid_edge();
extern void test_update_vertex_src();
extern void test_update_vertex_dst();
extern void test_update_invalid_edge_src();
extern void test_update_invalid_edge_dst();
extern void test_delete_edge();
extern void test_delete_invalid_edge();
extern void test_delete_all_edges();
extern void test_get_invalid_edge();
#endif

// misc operations testing
#ifdef TEST_MISC_OPERATIONS
extern void test_get_set_empty_value();
extern void test_get_invalid_record();
extern void test_get_set_large_record();
extern void test_overwrite_basic_info();
extern void test_standalone_vertex();
extern void test_delete_vertex_with_edges();
extern void test_delete_all_vertices_with_edges();
extern void test_add_delete_prop_with_records();
extern void test_alter_class_with_records();
extern void test_drop_class_with_relations();
extern void test_drop_and_find_extended_class();
extern void test_conflict_property();
extern void test_version_add_vertex_edge();
extern void test_version_update_vertex_edge();
extern void test_version_update_src_dst_edge();
extern void test_version_remove_vertex_edge();
extern void test_version_remove_all_vertex_edge();
extern void test_version_drop_vertex_edge();
extern void test_get_count_vertex();
extern void test_get_count_edge();
#endif

// graph operations testing
#ifdef TEST_GRAPH_OPERATIONS
extern void init_test_graph();
extern void destroy_test_graph();
extern void test_create_complex_graph();
extern void test_get_edge_in_more();
extern void test_get_edge_out_more();
extern void test_get_edge_all_more();
extern void test_get_invalid_edge_in_more();
extern void test_get_invalid_edge_out_more();
extern void test_get_invalid_edge_all_more();
extern void test_bfs_traverse_in();
extern void test_bfs_traverse_out();
extern void test_bfs_traverse_all();
extern void test_invalid_bfs_traverse_in();
extern void test_invalid_bfs_traverse_out();
extern void test_invalid_bfs_traverse_all();
extern void test_bfs_traverse_in_cursor();
extern void test_bfs_traverse_out_cursor();
extern void test_bfs_traverse_all_cursor();
extern void test_invalid_bfs_traverse_in_cursor();
extern void test_invalid_bfs_traverse_out_cursor();
extern void test_invalid_bfs_traverse_all_cursor();
extern void test_shortest_path();
extern void test_invalid_shortest_path();
extern void test_shortest_path_cursor();
extern void test_invalid_shortest_path_cursor();
extern void test_bfs_traverse_with_condition();
extern void test_shortest_path_with_condition();
extern void test_bfs_traverse_cursor_with_condition();
extern void test_shortest_path_cursor_with_condition();
extern void test_bfs_traverse_multi_edges_with_condition();
extern void test_bfs_traverse_multi_vertices();
extern void test_bfs_traverse_multi_vertices_with_condition();
// extern void test_shortest_path_dijkstra();
#endif

// find operations testing
#ifdef TEST_FIND_OPERATIONS
extern void test_expression();
extern void test_range_expression();
extern void test_extra_string_expression();
extern void test_negative_expression();
extern void test_cmp_function_expression();
extern void init_test_find();
extern void test_create_informative_graph();
extern void test_find_vertex();
extern void test_find_invalid_vertex();
extern void test_find_vertex_cursor();
extern void test_find_invalid_vertex_cursor();
extern void test_find_edge();
extern void test_find_invalid_edge();
extern void test_find_edge_cursor();
extern void test_find_invalid_edge_cursor();
extern void test_find_edge_in();
extern void test_find_edge_out();
extern void test_find_edge_all();
extern void test_find_invalid_edge_in();
extern void test_find_invalid_edge_out();
extern void test_find_invalid_edge_all();
extern void test_find_edge_in_cursor();
extern void test_find_edge_out_cursor();
extern void test_find_edge_all_cursor();
extern void test_find_invalid_edge_in_cursor();
extern void test_find_invalid_edge_out_cursor();
extern void test_find_invalid_edge_all_cursor();
extern void test_find_vertex_condition_function();
extern void test_find_edge_condition_function();
extern void test_find_vertex_cursor_condition_function();
extern void test_find_edge_cursor_condition_function();
extern void test_find_edge_in_condition_function();
extern void test_find_edge_out_condition_function();
extern void test_find_edge_all_condition_function();
extern void test_find_edge_in_cursor_condition_function();
extern void test_find_edge_out_cursor_condition_function();
extern void test_find_edge_all_cursor_condition_function();
extern void test_find_invalid_vertex_condition_function();
extern void test_find_invalid_edge_condition_function();
extern void test_find_invalid_vertex_cursor_condition_function();
extern void test_find_invalid_edge_cursor_condition_function();
extern void test_find_invalid_edge_in_condition_function();
extern void test_find_invalid_edge_out_condition_function();
extern void test_find_invalid_edge_all_condition_function();
extern void test_find_invalid_edge_in_cursor_condition_function();
extern void test_find_invalid_edge_out_cursor_condition_function();
extern void test_find_invalid_edge_all_cursor_condition_function();
extern void test_find_vertex_with_expression();
extern void test_find_invalid_vertex_with_expression();
extern void test_find_vertex_cursor_with_expression();
extern void test_find_invalid_vertex_cursor_with_expression();
extern void test_find_edge_with_expression();
extern void test_find_invalid_edge_with_expression();
extern void test_find_edge_cursor_with_expression();
extern void test_find_invalid_edge_cursor_with_expression();
extern void test_find_edge_in_with_expression();
extern void test_find_edge_out_with_expression();
extern void test_find_edge_all_with_expression();
extern void test_find_invalid_edge_in_with_expression();
extern void test_find_invalid_edge_out_with_expression();
extern void test_find_invalid_edge_all_with_expression();
extern void test_find_edge_in_cursor_with_expression();
extern void test_find_edge_out_cursor_with_expression();
extern void test_find_edge_all_cursor_with_expression();
extern void test_find_invalid_edge_in_cursor_with_expression();
extern void test_find_invalid_edge_out_cursor_with_expression();
extern void test_find_invalid_edge_all_cursor_with_expression();
extern void destroy_test_find();
#endif

// inheritance testing
#ifdef TEST_INHERITANCE_OPERATIONS
extern void test_create_class_extend();
extern void test_create_invalid_class_extend();
extern void test_drop_class_extend();
extern void test_alter_class_extend();
extern void test_add_property_extend();
extern void test_add_invalid_property_extend();
extern void test_delete_property_extend();
extern void test_delete_invalid_property_extend();
extern void test_alter_property_extend();
extern void test_alter_invalid_property_extend();
extern void init_all_extended_classes();
extern void test_create_vertex_edge_extend();
extern void test_create_invalid_vertex_edge_extend();
extern void test_delete_vertex_edge_extend();
extern void test_get_class_extend();
extern void test_find_class_extend();
extern void test_traverse_class_extend();
extern void test_shortest_path_class_extend();
extern void destroy_all_extended_classes();
#endif

// indexing
#ifdef TEST_INDEX_OPERATIONS
extern void test_create_index();
extern void test_drop_index();
extern void test_create_index_extended_class();
extern void test_drop_index_extended_class();
extern void test_create_invalid_index();
extern void test_drop_invalid_index();
extern void test_create_index_with_records();
extern void test_drop_index_with_records();
extern void test_create_index_extended_class_with_records();
extern void test_drop_index_extended_class_with_records();
extern void test_create_invalid_index_with_records();
extern void test_drop_invalid_index_with_records();
extern void test_search_by_index_unique_condition();
extern void test_search_by_index_non_unique_condition();
extern void test_search_by_index_unique_multicondition();
extern void test_search_by_index_non_unique_multicondition();
extern void test_search_by_index_unique_cursor_condition();
extern void test_search_by_index_non_unique_cursor_condition();
extern void test_search_by_index_unique_cursor_multicondition();
extern void test_search_by_index_non_unique_cursor_multicondition();
extern void test_search_by_index_extended_class_condition();
extern void test_search_by_index_extended_class_cursor_condition();
extern void test_search_by_index_extended_class_multicondition();
extern void test_search_by_index_extended_class_cursor_multicondition();
#endif

// schema transaction testing
#ifdef TEST_SCHEMA_TXN_OPERATIONS
extern void test_schema_txn_commit_simple();
extern void test_schema_txn_create_class_commit();
extern void test_schema_txn_create_class_rollback();
extern void test_schema_txn_drop_class_commit();
extern void test_schema_txn_drop_class_rollback();
extern void test_schema_txn_alter_class_commit();
extern void test_schema_txn_alter_class_rollback();
extern void test_schema_txn_create_class_extend_commit();
extern void test_schema_txn_create_class_extend_rollback();
extern void test_schema_txn_drop_class_extend_commit();
extern void test_schema_txn_drop_class_extend_rollback();
extern void test_schema_txn_add_property_commit();
extern void test_schema_txn_add_property_rollback();
extern void test_schema_txn_drop_property_commit();
extern void test_schema_txn_drop_property_rollback();
extern void test_schema_txn_alter_property_commit();
extern void test_schema_txn_alter_property_rollback();
extern void test_schema_txn_create_index_commit();
extern void test_schema_txn_create_index_rollback();
extern void test_schema_txn_drop_index_commit();
extern void test_schema_txn_drop_index_rollback();
extern void test_schema_txn_create_class_multiversion_commit();
extern void test_schema_txn_create_class_multiversion_rollback();
extern void test_schema_txn_drop_class_multiversion_commit();
extern void test_schema_txn_drop_class_multiversion_rollback();
extern void test_schema_txn_alter_class_multiversion_commit();
extern void test_schema_txn_alter_class_multiversion_rollback();
extern void test_schema_txn_create_class_extend_multiversion_commit();
extern void test_schema_txn_create_class_extend_multiversion_rollback();
extern void test_schema_txn_drop_class_extend_multiversion_commit();
extern void test_schema_txn_drop_class_extend_multiversion_rollback();
extern void test_schema_txn_add_property_multiversion_commit();
extern void test_schema_txn_add_property_multiversion_rollback();
extern void test_schema_txn_drop_property_multiversion_commit();
extern void test_schema_txn_drop_property_multiversion_rollback();
extern void test_schema_txn_alter_property_multiversion_commit();
extern void test_schema_txn_alter_property_multiversion_rollback();
extern void test_schema_txn_create_index_multiversion_commit();
extern void test_schema_txn_create_index_multiversion_rollback();
extern void test_schema_txn_drop_index_multiversion_commit();
extern void test_schema_txn_drop_index_multiversion_rollback();
#endif

// transaction testing
#ifdef TEST_TXN_OPERATIONS
extern void test_txn_commit_nothing();
extern void test_txn_create_only_vertex_commit();
extern void test_txn_create_only_vertex_rollback();
extern void test_txn_delete_only_vertex_commit();
extern void test_txn_delete_only_vertex_rollback();
extern void test_txn_create_only_edge_commit();
extern void test_txn_create_only_edge_rollback();
extern void test_txn_delete_only_edge_commit();
extern void test_txn_delete_only_edge_rollback();
extern void test_txn_get_vertex_edge();
extern void test_txn_alter_vertex_edge_commit();
extern void test_txn_alter_vertex_edge_rollback();
extern void test_txn_create_only_vertex_multiversion_commit();
extern void test_txn_create_only_vertex_multiversion_rollback();
extern void test_txn_delete_only_vertex_multiversion_commit();
extern void test_txn_delete_only_vertex_multiversion_rollback();
extern void test_txn_create_edges_multiversion_commit();
extern void test_txn_create_edges_multiversion_rollback();
extern void test_txn_delete_edges_multiversion_commit();
extern void test_txn_delete_edges_multiversion_rollback();
extern void test_txn_modify_edges_multiversion_commit();
extern void test_txn_modify_edges_multiversion_rollback();
extern void test_txn_rollback_when_destroy();
extern void test_txn_reopen_ctx();
extern void test_txn_invalid_operations();
#endif

// sql operations testing
#ifdef TEST_SQL_OPERATIONS
extern void test_sql_unrecognized_token_error();
extern void test_sql_syntax_error();
extern void test_sql_create_class();
extern void test_sql_create_class_if_not_exists();
extern void test_sql_create_class_extend();
extern void test_sql_create_invalid_class();
extern void test_sql_alter_class_name();
extern void test_sql_drop_class();
extern void test_sql_drop_class_if_exists();
extern void test_sql_drop_invalid_class();
extern void test_sql_add_property();
extern void test_sql_alter_property();
extern void test_sql_delete_property();
extern void test_sql_create_vertex();
extern void test_sql_create_edges();
extern void test_sql_select_vertex();
extern void test_sql_select_vertex_with_rid();
extern void test_sql_select_property();
extern void test_sql_select_count();
extern void test_sql_select_walk();
extern void test_sql_select_method_property();
extern void test_sql_select_alias_property();
extern void test_sql_select_vertex_condition();
extern void test_sql_select_vertex_with_multi_condition();
extern void test_sql_select_nested_condition();
extern void test_sql_select_skip_limit();
extern void test_sql_select_group_by();
extern void test_sql_update_vertex_with_rid();
extern void test_sql_update_vertex_with_condition();
extern void test_sql_delete_vertex_with_rid();
extern void test_sql_delete_vertex_with_condition();
extern void test_sql_delete_edge_with_rid();
extern void test_sql_delete_edge_with_condition();
extern void test_sql_validate_property_type();
extern void test_sql_traverse();
extern void test_sql_create_index();
extern void test_sql_create_index_unique();
extern void test_sql_drop_index();
#endif
