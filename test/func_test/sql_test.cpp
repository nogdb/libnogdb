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

#include <string.h>

#include "func_test.h"
#include "func_test_utils.h"
#include "setup_cleanup.h"

using namespace std;
using namespace nogdb;

namespace nogdb {
inline string to_string(const RecordDescriptor& recD)
{
    return "#" + ::to_string(recD.rid.first) + ":" + ::to_string(recD.rid.second);
}

inline bool operator==(const Bytes& lhs, const Bytes& rhs)
{
    return lhs.size() == rhs.size() && memcmp(lhs.getRaw(), rhs.getRaw(), lhs.size()) == 0;
}

inline bool operator==(const Record& lhs, const Record& rhs) { return lhs.getAll() == rhs.getAll(); }

inline bool operator==(const Result& lhs, const Result& rhs)
{
    if (lhs.descriptor.rid.first != (ClassId)(-2)) {
        return lhs.descriptor == rhs.descriptor;
    } else {
        return lhs.record == rhs.record;
    }
}
}

void test_sql_unrecognized_token_error()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        SQL::execute(txn, "128asyuiqwerhb;");
        assert(false);
    } catch (const Error& e) {
        REQUIRE(e, NOGDB_SQL_UNRECOGNIZED_TOKEN, "NOGDB_SQL_UNRECOGNIZED_TOKEN");
    }
    txn.commit();
}

void test_sql_syntax_error()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        SQL::execute(txn, "SELECT DELETE VERTEX;");
        assert(false);
    } catch (const Error& e) {
        REQUIRE(e, NOGDB_SQL_SYNTAX_ERROR, "NOGDB_SQL_SYNTAX_ERROR");
    }
    txn.commit();
}

void test_sql_create_class()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        // create
        SQL::Result result = SQL::execute(txn, "CREATE CLASS sql_class EXTENDS VERTEX");

        // check result.
        assert(result.type() == SQL::Result::CLASS_DESCRIPTOR);
        assert(result.get<ClassDescriptor>().name == "sql_class");
        ClassDescriptor schema = txn.getClass("sql_class");
        assert(schema.name == "sql_class");
    } catch (const Error& e) {
        std::cout << "\nError: " << e.what() << std::endl;
        assert(false);
    }

    try {
        txn.dropClass("sql_class");
    } catch (...) {
    }
    txn.commit();
}

void test_sql_create_class_if_not_exists()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    // test not exists case.
    try {
        SQL::Result result = SQL::execute(txn, "CREATE CLASS sql_class IF NOT EXISTS EXTENDS VERTEX");
        assert(result.type() == SQL::Result::CLASS_DESCRIPTOR);
        assert(result.get<ClassDescriptor>().name == "sql_class");
    } catch (const Error& e) {
        std::cout << "\nError: " << e.what() << std::endl;
        assert(false);
    }

    // test exists case.
    try {
        SQL::execute(txn, "CREATE CLASS sql_class IF NOT EXISTS EXTENDS VERTEX");
        auto schema = txn.getClass("sql_class");
        assert(schema.name == "sql_class");
    } catch (const Error& e) {
        std::cout << "\nError: " << e.what() << std::endl;
        assert(false);
    }

    try {
        txn.dropClass("sql_class");
    } catch (...) {
    }
    txn.commit();
}

void test_sql_create_class_extend()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    // create super class
    try {
        txn.addClass("sql_class", ClassType::VERTEX);
        txn.addProperty("sql_class", "prop1", PropertyType::TEXT);
        txn.addProperty("sql_class", "prop2", PropertyType::UNSIGNED_INTEGER);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    // create extend
    try {
        SQL::execute(txn, "CREATE CLASS sql_class_sub EXTENDS sql_class");
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    // check result
    try {
        auto res = txn.getClass("sql_class_sub");
        assert(res.name == "sql_class_sub");
        assert(res.type == ClassType::VERTEX);
        auto properties = txn.getProperties(res);
        assert(properties.size() == 2);
        for (const auto& property : properties) {
            if (property.name == "prop1")
                assert(property.type == nogdb::PropertyType::TEXT);
            else if (property.name == "prop2")
                assert(property.type == nogdb::PropertyType::UNSIGNED_INTEGER);
            else
                assert(false);
        }
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        txn.dropClass("sql_class");
        txn.dropClass("sql_class_sub");
    } catch (...) {
    }
    txn.commit();
}

void test_sql_create_invalid_class()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        txn.addClass("sql_class", ClassType::VERTEX);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        SQL::execute(txn, "CREATE CLASS '' EXTENDS VERTEX");
        assert(false);
    } catch (const Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_CLASSNAME, "NOGDB_CTX_INVALID_CLASSNAME");
    }
    try {
        SQL::execute(txn, "CREATE CLASS sql_class EXTENDS VERTEX");
        assert(false);
    } catch (const Error& ex) {
        REQUIRE(ex, NOGDB_CTX_DUPLICATE_CLASS, "NOGDB_CTX_DUPLICATE_CLASS");
    }
    try {
        SQL::execute(txn, "DROP CLASS sql_class");
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_sql_alter_class_name()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    // create class
    try {
        txn.addClass("sql_class", ClassType::VERTEX);
        txn.addProperty("sql_class", "prop1", PropertyType::INTEGER);
        txn.addProperty("sql_class", "prop2", PropertyType::TEXT);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    // test alter NAME
    try {
        SQL::execute(txn, "ALTER CLASS sql_class NAME 'sql_class2'");
        auto res = txn.getClass("sql_class2");
        assert(res.name == "sql_class2");
        auto properties = txn.getProperties(res);
        assert(properties.size() == 2);
        for (const auto& property : properties) {
            if (property.name == "prop1")
                assert(property.type == nogdb::PropertyType::INTEGER);
            else if (property.name == "prop2")
                assert(property.type == nogdb::PropertyType::TEXT);
            else
                assert(false);
        }
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        txn.dropClass("sql_class2");
    } catch (...) {
    }
    txn.commit();
}

void test_sql_drop_class()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        txn.addClass("sql_class", ClassType::VERTEX);

        SQL::Result result = SQL::execute(txn, "DROP CLASS sql_class");
        assert(result.type() == SQL::Result::NO_RESULT);
    } catch (const Error& e) {
        std::cout << "\nError: " << e.what() << std::endl;
        assert(false);
    }

    // check result.
    try {
        auto schema = txn.getClass("sql_class");
        assert(false);
    } catch (const Error& e) {
        assert(e.code() == NOGDB_CTX_NOEXST_CLASS);
    }
    txn.commit();
}

void test_sql_drop_class_if_exists()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    // test exists case.
    try {
        txn.addClass("sql_class", ClassType::VERTEX);

        SQL::Result result = SQL::execute(txn, "DROP CLASS sql_class IF EXISTS");
        assert(result.type() == SQL::Result::NO_RESULT);
    } catch (const Error& e) {
        std::cout << "\nError: " << e.what() << std::endl;
        assert(false);
    }

    // test not exists case.
    try {
        SQL::execute(txn, "DROP CLASS test_sql IF EXISTS");
    } catch (const Error& e) {
        std::cout << "\nError: " << e.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_sql_drop_invalid_class()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        SQL::execute(txn, "DROP CLASS ''");
        assert(false);
    } catch (const Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_CLASSNAME, "NOGDB_CTX_INVALID_CLASSNAME");
    }
    try {
        SQL::execute(txn, "DROP CLASS sql_class");
        assert(false);
    } catch (const Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        assert(ex.code() == NOGDB_CTX_NOEXST_CLASS);
    }
    txn.commit();
}

void test_sql_add_property()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        txn.addClass("sql_class", ClassType::VERTEX);
        SQL::Result result1 = SQL::execute(txn, "CREATE PROPERTY sql_class.prop1 TEXT");
        SQL::Result result2 = SQL::execute(txn, "CREATE PROPERTY sql_class.prop2 UNSIGNED_INTEGER");
        assert(result1.type() == SQL::Result::PROPERTY_DESCRIPTOR);
        assert(result1.get<PropertyDescriptor>().type == PropertyType::TEXT);
        assert(result2.type() == SQL::Result::PROPERTY_DESCRIPTOR);
        assert(result2.get<PropertyDescriptor>().type == PropertyType::UNSIGNED_INTEGER);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto schema = txn.getClass("sql_class");
        assert(schema.name == "sql_class");
        auto properties = txn.getProperties(schema);
        assert(properties.size() == 2);
        for (const auto& property : properties) {
            if (property.name == "prop1")
                assert(property.type == nogdb::PropertyType::TEXT);
            else if (property.name == "prop2")
                assert(property.type == nogdb::PropertyType::UNSIGNED_INTEGER);
            else
                assert(false);
        }
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_sql_alter_property()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        txn.addClass("links", ClassType::EDGE);
        txn.addProperty("links", "type", PropertyType::TEXT);
        txn.addProperty("links", "expire", PropertyType::INTEGER);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        SQL::execute(txn, "ALTER PROPERTY links.type NAME 'comments'");
        SQL::execute(txn, "ALTER PROPERTY links.expire NAME 'expired'");
        txn.addProperty("links", "type", PropertyType::BLOB);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto schema = txn.getClass("links");
        assert(schema.name == "links");
        assert(schema.type == ClassType::EDGE);
        auto properties = txn.getProperties(schema);
        assert(properties.size() == 3);
        for (const auto& property : properties) {
            if (property.name == "type")
                assert(property.type == nogdb::PropertyType::BLOB);
            else if (property.name == "comments")
                assert(property.type == nogdb::PropertyType::TEXT);
            else if (property.name == "expired")
                assert(property.type == nogdb::PropertyType::INTEGER);
            else
                assert(false);
        }
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        txn.dropClass("links");
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_sql_delete_property()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        SQL::execute(txn, "DROP PROPERTY sql_class.prop2");

        auto schema = txn.getClass("sql_class");
        assert(schema.name == "sql_class");
        auto properties = txn.getProperties(schema);
        assert(properties.size() == 1);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        txn.dropClass("sql_class");
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_sql_create_vertex()
{
    init_vertex_book();
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        SQL::Result result = SQL::execute(
            txn, "CREATE VERTEX books SET title='Harry Potter', words=4242424242, pages=865, price=49.99");
        assert(result.type() == SQL::Result::RECORD_DESCRIPTORS);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_vertex_book();
}

void test_sql_create_edges()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    RecordDescriptor v1_1 {}, v1_2 {}, v2 {};
    try {
        v1_1 = txn.addVertex("books", Record().set("title", "Harry Potter").set("pages", 456).set("price", 24.5));
        v1_2 = txn.addVertex("books", Record().set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0));
        v2 = txn.addVertex("persons", Record().set("name", "J.K. Rowlings").set("age", 32));
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        SQL::execute(
            txn, "CREATE EDGE authors FROM " + to_string(v1_1) + " TO " + to_string(v2) + " SET time_used=365");
        SQL::execute(txn,
            "CREATE EDGE authors FROM (" + to_string(v1_1) + ", " + to_string(v1_2) + ") TO " + to_string(v2)
                + " SET time_used=180");
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_sql_select_vertex()
{
    init_vertex_person();
    init_vertex_book();

    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        auto records = std::vector<Record> {};
        records.push_back(Record {}.set("title", "Percy Jackson").set("pages", 456).set("price", 24.5));
        records.push_back(Record {}.set("title", "Batman VS Superman").set("words", 9999999ULL).set("price", 36.0));
        for (const auto& record : records) {
            txn.addVertex("books", record);
        }
        txn.addVertex("persons", Record {}.set("name", "Jim Beans").set("age", 40U));
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        SQL::Result result = SQL::execute(txn, "SELECT * FROM books");
        assert(result.type() == SQL::Result::Type::RESULT_SET);
        ResultSet res = result.get<ResultSet>();
        ASSERT_SIZE(res, 2);
        assert(res[0].record.get("title").toText() == "Percy Jackson");
        assert(res[0].record.get("pages").toInt() == 456);
        assert(res[0].record.get("price").toReal() == 24.5);
        assert(res[0].record.get("words").empty());
        assert(res[1].record.get("title").toText() == "Batman VS Superman");
        assert(res[1].record.get("words").toBigIntU() == 9999999);
        assert(res[1].record.get("price").toReal() == 36.0);
        assert(res[1].record.get("pages").empty());
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_vertex_book();
    destroy_vertex_person();
}

void test_sql_select_vertex_with_rid()
{
    init_vertex_person();
    init_vertex_book();

    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);

    RecordDescriptor rid1, rid2;
    try {
        rid1 = txn.addVertex("persons", Record().set("name", "Jim Beans").set("age", 40U));
        rid2 = txn.addVertex("books", Record().set("title", "Percy Jackson").set("pages", 456).set("price", 24.5));
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto result = SQL::execute(txn, "SELECT FROM " + to_string(rid1));
        assert(result.type() == result.RESULT_SET);
        auto res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == rid1);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto result = SQL::execute(txn, "SELECT FROM (" + to_string(rid1) + ", " + to_string(rid2) + ")");
        assert(result.type() == result.RESULT_SET);
        auto res = result.get<ResultSet>();
        ASSERT_SIZE(res, 2);
        assert((res[0].descriptor == rid1 && res[1].descriptor == rid2)
            || (res[0].descriptor == rid2 && res[1].descriptor == rid1));
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    destroy_vertex_book();
    destroy_vertex_person();
}

void test_sql_select_property()
{
    init_vertex_person();

    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);

    RecordDescriptor rdesc, rdResult;
    try {
        rdesc = txn.addVertex("persons", Record().set("name", "Jim Beans").set("age", 40U));
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    // select properties.
    try {
        SQL::Result result = SQL::execute(txn, "SELECT name, age FROM " + to_string(rdesc));
        assert(result.type() == result.RESULT_SET);
        auto res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == RecordDescriptor(-2, 0));
        assert(res[0].record.get("name").toText() == "Jim Beans");
        assert(res[0].record.get("age").toIntU() == 40U);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    // select @recordId.
    try {
        SQL::Result result = SQL::execute(txn, "SELECT @recordId FROM " + to_string(rdesc));
        assert(result.type() == result.RESULT_SET);
        auto res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == RecordDescriptor(-2, 0));
        assert(res[0].record.get("@recordId").toText() == nogdb::rid2str(rdesc.rid));
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    // select @className.
    try {
        SQL::Result result = SQL::execute(txn, "SELECT @className FROM " + to_string(rdesc));
        assert(result.type() == result.RESULT_SET);
        auto res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == RecordDescriptor(-2, 0));
        assert(res[0].record.get("@className").toText() == "persons");
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    // select @version.
    try {
        SQL::Result result = SQL::execute(txn, "SELECT @version FROM " + to_string(rdesc));
        assert(result.type() == result.RESULT_SET);
        auto res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == RecordDescriptor(-2, 0));
        assert(res[0].record.get("@version").empty() == false);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    // select non-exist property.
    try {
        SQL::Result result = SQL::execute(txn, "SELECT nonExist FROM " + to_string(rdesc));
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>().size() == 0);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    destroy_vertex_person();
}

void test_sql_select_count()
{
    init_vertex_person();

    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);

    RecordDescriptor rid1, rid2;
    try {
        txn.addVertex("persons", Record().set("name", "Jim Beans").set("age", 40U));
        txn.addVertex("persons", Record().set("name", "Jame Beans"));
        txn.addVertex("persons");
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto result = SQL::execute(txn, "SELECT count(*) FROM persons");
        assert(result.type() == result.RESULT_SET);
        auto res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == RecordDescriptor(-2, 0));
        assert(res[0].record.get("count").toBigIntU() == 3);

        result = SQL::execute(txn, "SELECT count('name'), count(age) FROM persons");
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == RecordDescriptor(-2, 0));
        assert(res[0].record.get("count").toBigIntU() == 2);
        assert(res[0].record.get("count2").toBigIntU() == 1);

        // count empty result.
        result = SQL::execute(txn, "SELECT count(*) FROM persons WHERE name='Sam'");
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == RecordDescriptor(-2, 0));
        assert(res[0].record.get("count").toBigIntU() == 0);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    destroy_vertex_person();
}

void test_sql_select_walk()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);

    txn.addClass("v", ClassType::VERTEX);
    txn.addProperty("v", "p", PropertyType::TEXT);
    txn.addClass("eA", ClassType::EDGE);
    txn.addProperty("eA", "p", PropertyType::TEXT);
    txn.addClass("eB", ClassType::EDGE);
    txn.addProperty("eB", "p", PropertyType::TEXT);

    try {
        auto v1 = txn.addVertex("v", Record().set("p", "v1"));
        auto v2 = txn.addVertex("v", Record().set("p", "v2"));
        auto v3 = txn.addVertex("v", Record().set("p", "v3"));
        auto v4 = txn.addVertex("v", Record().set("p", "v4"));
        auto v5 = txn.addVertex("v", Record().set("p", "v5"));
        auto eA13 = txn.addEdge("eA", v1, v3, Record().set("p", "e13"));
        auto eB14 = txn.addEdge("eB", v1, v4, Record().set("p", "e14"));
        auto eA23 = txn.addEdge("eA", v2, v3, Record().set("p", "e23"));
        auto eB24 = txn.addEdge("eB", v2, v4, Record().set("p", "e24"));
        auto eA35 = txn.addEdge("eA", v3, v5, Record().set("p", "e35"));

        auto result = SQL::execute(txn, "SELECT expand(outE()) FROM " + to_string(v1));
        assert(result.type() == result.RESULT_SET);
        auto res = result.get<ResultSet>();
        ASSERT_SIZE(res, 2);
        assert(res[0].descriptor == eA13);
        assert(res[1].descriptor == eB14);

        result = SQL::execute(txn, "SELECT expand(inE()) FROM " + to_string(v3));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 2);
        assert(res[0].descriptor == eA13);
        assert(res[1].descriptor == eA23);

        result = SQL::execute(txn, "SELECT expand(bothE()) FROM " + to_string(v3));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 3);
        assert(res[0].descriptor == eA13);
        assert(res[1].descriptor == eA23);
        assert(res[2].descriptor == eA35);

        result = SQL::execute(txn, "SELECT expand(outV()) FROM " + to_string(eA13));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == v1);

        result = SQL::execute(txn, "SELECT expand(inV()) FROM " + to_string(eA13));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == v3);

        result = SQL::execute(txn, "SELECT expand(bothV()) FROM " + to_string(eB24));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 2);
        assert(res[0].descriptor == v2);
        assert(res[1].descriptor == v4);

        result = SQL::execute(txn, "SELECT expand(out()) FROM " + to_string(v1));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 2);
        assert(res[0].descriptor == v3);
        assert(res[1].descriptor == v4);

        result = SQL::execute(txn, "SELECT expand(in()) FROM " + to_string(v3));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 2);
        assert(res[0].descriptor == v1);
        assert(res[1].descriptor == v2);

        result = SQL::execute(txn, "SELECT expand(both()) FROM " + to_string(v3));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 3);
        assert(res[0].descriptor == v1);
        assert(res[1].descriptor == v2);
        assert(res[2].descriptor == v5);

        result = SQL::execute(txn, "SELECT expand(out('eA')) FROM " + to_string(v1));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == v3);

        result = SQL::execute(txn, "SELECT expand(in('eA', 'eB')) FROM " + to_string(v3));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 2);
        assert(res[0].descriptor == v1);
        assert(res[1].descriptor == v2);

        result = SQL::execute(txn, "SELECT expand(in('eA').out('eB')) FROM " + to_string(v3));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 2);
        assert(res[0].descriptor == v4);
        assert(res[1].descriptor == v4);

        result = SQL::execute(txn, "SELECT expand(outE()[p='e13'].inV()) FROM " + to_string(v1));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == v3);

        // expand empty result from condition projection.
        result = SQL::execute(txn, "SELECT expand(outE()[p='e99']) FROM " + to_string(v1));
        assert(result.type() == result.RESULT_SET);
        ASSERT_SIZE(result.get<ResultSet>(), 0);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.dropClass("v");
    txn.dropClass("eA");
    txn.dropClass("eB");

    txn.commit();
}

void test_sql_select_method_property()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);

    txn.addClass("v", ClassType::VERTEX);
    txn.addProperty("v", "propV", PropertyType::TEXT);
    txn.addClass("e", ClassType::EDGE);
    txn.addProperty("e", "propE", PropertyType::TEXT);

    try {
        auto v1 = txn.addVertex("v", Record().set("propV", "v1"));
        auto v2 = txn.addVertex("v", Record().set("propV", "v2"));
        auto v3 = txn.addVertex("v", Record().set("propV", "v3"));
        auto v4 = txn.addVertex("v", Record().set("propV", "v4"));
        auto eA13 = txn.addEdge("e", v1, v3, Record().set("propE", "e1->3"));
        txn.addEdge("e", v1, v4, Record().set("propE", "e1->4"));
        txn.addEdge("e", v2, v4, Record().set("propE", "e2->4"));

        // normal method
        auto result = SQL::execute(txn, "SELECT inV().propV FROM " + to_string(eA13));
        assert(result.type() == result.RESULT_SET);
        auto res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == RecordDescriptor(-2, 0));
        assert(res[0].record.get("inV").toText() == "v3");

        // normal method with array selector
        result = SQL::execute(txn, "SELECT out()[0].propV FROM " + to_string(v1));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        assert(res[0].descriptor == RecordDescriptor(-2, 0));
        assert(res[0].record.get("out").toText() == "v3");

        // normal method with array selector and normal property
        result = SQL::execute(txn, "SELECT propV, out()[0].propV FROM " + to_string(v1));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        assert(res[0].descriptor == RecordDescriptor(-2, 0));
        assert(res[0].record.get("propV").toText() == "v1");
        assert(res[0].record.get("out").toText() == "v3");

        // normal method with out of range array selector
        result = SQL::execute(txn, "SELECT out()[2].propV FROM " + to_string(v1));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 0);

        // method with condition
        result = SQL::execute(txn, "SELECT out()[propV='v3'].propV FROM " + to_string(v1));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("out").toText() == "v3");

        // normal property, out of range array select and method with empty result from walk
        result = SQL::execute(txn,
            "SELECT propV, out('e')[2].propV, outE()[propE='e1->5'].inV().propV as out_propV FROM " + to_string(v1));
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        assert(res[0].descriptor == RecordDescriptor(-2, 0));
        assert(res[0].record.get("propV").toText() == "v1");
        assert(res[0].record.get("out").empty());
        assert(res[0].record.get("out_propV").empty());
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.dropClass("v");
    txn.dropClass("e");
    txn.commit();
}

void test_sql_select_alias_property()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);

    txn.addClass("v", ClassType::VERTEX);
    txn.addProperty("v", "propV", PropertyType::TEXT);
    txn.addClass("e", ClassType::EDGE);
    txn.addProperty("e", "propE", PropertyType::TEXT);

    try {
        auto v1 = txn.addVertex("v", Record().set("propV", "v1"));
        auto v3 = txn.addVertex("v", Record().set("propV", "v3"));
        auto eA13 = txn.addEdge("e", v1, v3, Record().set("propE", "e1->3"));

        auto result = SQL::execute(txn, "SELECT inV().propV AS my_prop FROM " + to_string(eA13));
        assert(result.type() == result.RESULT_SET);
        auto res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == RecordDescriptor(-2, 0));
        assert(res[0].record.getText("my_prop") == "v3");
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.dropClass("v");
    txn.dropClass("e");
    txn.commit();
}

struct Coordinates {
    Coordinates() {};

    Coordinates(double x_, double y_)
        : x { x_ }
        , y { y_ }
    {
    }

    double x { 0.0 };
    double y { 0.0 };

    string toHex()
    {
        string result {};
        for (size_t i = 0; i < sizeof(struct Coordinates); i++) {
            char tmp[3];
            sprintf(tmp, "%02X", ((unsigned char*)this)[i]);
            result += tmp;
        }
        return result;
    }
};

void test_sql_select_vertex_condition()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    txn.addClass("v", ClassType::VERTEX);
    txn.addProperty("v", "text", nogdb::PropertyType::TEXT);
    txn.addProperty("v", "int", nogdb::PropertyType::INTEGER);
    txn.addProperty("v", "uint", nogdb::PropertyType::UNSIGNED_INTEGER);
    txn.addProperty("v", "bigint", nogdb::PropertyType::BIGINT);
    txn.addProperty("v", "ubigint", nogdb::PropertyType::UNSIGNED_BIGINT);
    txn.addProperty("v", "real", nogdb::PropertyType::REAL);
    auto v1 = txn.addVertex("v",
        nogdb::Record {}
            .set("text", "A")
            .set("int", 11)
            .set("uint", 10200U)
            .set("bigint", 200000LL)
            .set("ubigint", 2000ULL)
            .set("real", 4.5));
    txn.addVertex("v",
        nogdb::Record {}
            .set("text", "B1Y")
            .set("int", 37)
            .set("bigint", 280000LL)
            .set("ubigint", 1800ULL)
            .set("real", 5.0));
    txn.addVertex("v",
        nogdb::Record {}
            .set("text", "B2Y")
            .set("uint", 10250U)
            .set("bigint", 220000LL)
            .set("ubigint", 2400ULL)
            .set("real", 4.5));
    txn.addVertex("v",
        nogdb::Record {}.set("text", "CX").set("int", 28).set("uint", 11600U).set("ubigint", 900ULL).set("real", 3.5));
    txn.addVertex("v",
        nogdb::Record {}
            .set("text", "DX")
            .set("int", 18)
            .set("uint", 10475U)
            .set("bigint", 300000LL)
            .set("ubigint", 900ULL));

    try {
        auto result = SQL::execute(txn, "SELECT FROM v WHERE text='A'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("text").eq("A")).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE text='Z'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("text").eq("Z")).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE int=18");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("int").eq(18)).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE uint=11600");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("uint").eq(11600)).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE bigint=280000");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("bigint").eq(280000LL)).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE ubigint=900");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("ubigint").eq(900ULL)).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE real=4.5");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("real").eq(4.5)).get());
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    // Condition special properties.
    try {
        auto result = SQL::execute(txn, "SELECT FROM v WHERE @recordId = '" + nogdb::rid2str(v1.rid) + "'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("@recordId").eq(nogdb::rid2str(v1.rid))).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE @className = 'v'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("@className").eq("v")).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE @version = 0");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("@version").eq(0ULL)).get());
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto result = SQL::execute(txn, "SELECT FROM v WHERE text != 'A'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(not Condition("text").eq("A")).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE int > 35");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("int").gt(35)).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE real >= 4.5");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("real").ge(4.5)).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE uint < 10300");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("uint").lt(10300)).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE ubigint <= 900");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("ubigint").le(900ULL)).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE bigint IS NULL");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("bigint").null()).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE int IS NOT NULL");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(not Condition("int").null()).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE text = 100");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("text").eq(100)).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE ubigint = 2000");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("ubigint").eq(2000ULL)).get());
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto result = SQL::execute(txn, "SELECT FROM v WHERE text CONTAIN 'a'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("text").contain("a").ignoreCase()).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE NOT (text CONTAIN 'b')");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(not Condition("text").contain("b").ignoreCase()).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE text BEGIN WITH 'a'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("text").beginWith("a").ignoreCase()).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE NOT text BEGIN WITH CASE 'A'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(not Condition("text").beginWith("A")).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE text END WITH 'x'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("text").endWith("x").ignoreCase()).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE NOT text END WITH CASE 'Y'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(not Condition("text").endWith("Y")).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE text > 'B2Y'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("text").gt("B2Y")).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE text >= 'B2Y'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("text").ge("B2Y")).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE text < 'B2Y'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("text").lt("B2Y")).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE text <= 'B2Y'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("text").le("B2Y")).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE text IN ['B1Y', 'A']");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>()
            == txn.find("v").where(Condition("text").in(vector<string> { "B1Y", "A" }).ignoreCase()).get());

        result = SQL::execute(txn, "SELECT FROM v WHERE text LIKE '%1%'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.find("v").where(Condition("text").like("%1%").ignoreCase()).get());
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_sql_select_vertex_with_multi_condition()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    txn.addClass("v", ClassType::VERTEX);
    txn.addProperty("v", "prop1", PropertyType::TEXT);
    txn.addProperty("v", "prop2", PropertyType::INTEGER);
    txn.addVertex("v", Record().set("prop1", "AX").set("prop2", 1));
    txn.addVertex("v", Record().set("prop1", "BX").set("prop2", 2));
    txn.addVertex("v", Record().set("prop1", "C").set("prop2", 3));
    try {
        auto result = SQL::execute(txn, "SELECT FROM v WHERE prop1 END WITH 'X' OR prop2 >= 2");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>()
            == txn.find("v").where(Condition("prop1").endWith("X").ignoreCase() or Condition("prop2").ge(2)).get());
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto result = SQL::execute(txn, "SELECT FROM v WHERE (prop1 = 'C' AND prop2 = 3) OR prop1 = 'AX'");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>()
            == txn.find("v")
                   .where((Condition("prop1").eq("C") and Condition("prop2").eq(3)) or Condition("prop1").eq("AX"))
                   .get());
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto result = SQL::execute(txn, "SELECT FROM v WHERE (prop1 = 'AX') OR (prop1 = 'C' AND prop2 = 3)");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>()
            == txn.find("v")
                   .where(Condition("prop1").eq("AX") or (Condition("prop1").eq("C") and Condition("prop2").eq(3)))
                   .get());
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto result
            = SQL::execute(txn, "SELECT FROM v WHERE (@className='v' AND prop2<2) OR (@className='x' AND prop2>0)");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>()
            == txn.find("v")
                   .where((Condition("@className").eq("v") and Condition("prop2").lt(2))
                       or (Condition("@className").eq("x") and Condition("prop2").gt(0)))
                   .get());
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_sql_select_nested_condition()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    txn.addClass("v", ClassType::VERTEX);
    txn.addProperty("v", "prop1", PropertyType::TEXT);
    txn.addProperty("v", "prop2", PropertyType::INTEGER);
    auto v1 = txn.addVertex("v", Record().set("prop1", "AX").set("prop2", 1));
    txn.addVertex("v", Record().set("prop1", "BX").set("prop2", 2));
    txn.addVertex("v", Record().set("prop1", "C").set("prop2", 3));
    try {
        auto result = SQL::execute(txn, "SELECT * FROM (SELECT FROM v) WHERE prop2=1");
        assert(result.type() == result.RESULT_SET);
        auto res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].descriptor == v1);

        result = SQL::execute(txn, "SELECT * FROM (SELECT prop1, prop2 FROM v) WHERE prop2>2");
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("prop1").toText() == "C");

        result = SQL::execute(
            txn, "SELECT * FROM (SELECT @className, prop1, prop2 FROM v) WHERE @className='v' AND prop2<2");
        assert(result.type() == result.RESULT_SET);
        res = result.get<ResultSet>();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("prop1").toText() == "AX");
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_sql_select_skip_limit()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    txn.addClass("v", ClassType::VERTEX);
    txn.addProperty("v", "prop1", PropertyType::TEXT);
    txn.addProperty("v", "prop2", PropertyType::INTEGER);
    txn.addVertex("v", Record().set("prop1", "A").set("prop2", 1));
    txn.addVertex("v", Record().set("prop1", "B").set("prop2", 2));
    txn.addVertex("v", Record().set("prop1", "C").set("prop2", 3));
    txn.addVertex("v", Record().set("prop1", "D").set("prop2", 4));
    try {
        SQL::Result result = SQL::execute(txn, "SELECT * FROM v SKIP 1 LIMIT 2");
        assert(result.type() == result.RESULT_SET);
        ResultSet baseResult = txn.find("v").get();
        baseResult.erase(baseResult.begin(), baseResult.begin() + 1);
        baseResult.resize(2);
        assert(result.get<ResultSet>() == baseResult);

        result = SQL::execute(txn, "SELECT * FROM (SELECT FROM v) WHERE prop2<3 SKIP 0 LIMIT 1");
        assert(result.type() == result.RESULT_SET);
        baseResult = txn.find("v").where(Condition("prop2").le(3)).get();
        baseResult.resize(1);
        assert(result.get<ResultSet>() == baseResult);

        result = SQL::execute(txn, "SELECT * FROM (SELECT FROM v) SKIP 100");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>().size() == 0);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_sql_select_group_by()
{
    init_vertex_book();
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        Record r {};
        r.set("title", "Lion King").set("price", 100.0);
        txn.addVertex("books", r);
        r.set("title", "Tarzan").set("price", 100.0);
        txn.addVertex("books", r);

        SQL::Result result = SQL::execute(txn, "SELECT * FROM books GROUP BY price");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>().size() == 1);
        assert(result.get<ResultSet>()[0].record.get("price") == r.get("price"));
    } catch (const Error& e) {
        cout << "\nError: " << e.what() << endl;
        assert(false);
    }
    txn.commit();
    destroy_vertex_book();
}

void test_sql_update_vertex_with_rid()
{
    init_vertex_book();
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        Record r {};
        r.set("title", "Lion King").set("price", 100.0).set("pages", 320);
        auto rdesc1 = txn.addVertex("books", r);
        r.set("title", "Tarzan").set("price", 60.0).set("pages", 360);
        txn.addVertex("books", r);

        auto record = txn.fetchRecord(rdesc1);
        assert(record.get("title").toText() == "Lion King");
        assert(record.get("price").toReal() == 100);
        assert(record.get("pages").toInt() == 320);

        auto result = SQL::execute(txn, "UPDATE " + to_string(rdesc1) + " SET price=50.0, pages=400, words=90000");
        assert(result.type() == result.RECORD_DESCRIPTORS);
        using ::operator==;
        assert(result.get<vector<RecordDescriptor>>() == vector<RecordDescriptor> { rdesc1 });
        auto res = txn.find("books").get();
        assert(res[0].record.get("title").toText() == "Lion King");
        assert(res[0].record.get("price").toReal() == 50);
        assert(res[0].record.get("pages").toInt() == 400);
        assert(res[0].record.get("words").toBigIntU() == 90000ULL);
        assert(res[1].record.get("title").toText() == "Tarzan");
        assert(res[1].record.get("price").toReal() == 60);
        assert(res[1].record.get("pages").toInt() == 360);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_vertex_book();
}

void test_sql_update_vertex_with_condition()
{
    init_vertex_book();
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        Record r {};
        r.set("title", "Lion King").set("price", 100.0).set("pages", 320);
        auto rdesc1 = txn.addVertex("books", r);
        r.set("title", "Tarzan").set("price", 60.0).set("pages", 360);
        txn.addVertex("books", r);

        auto record = txn.fetchRecord(rdesc1);
        assert(record.get("title").toText() == "Lion King");
        assert(record.get("price").toReal() == 100);
        assert(record.get("pages").toInt() == 320);

        auto result = SQL::execute(txn, "UPDATE books SET price=50.0, pages=400, words=90000 where title='Lion King'");
        assert(result.type() == result.RECORD_DESCRIPTORS);
        assert(result.get<vector<RecordDescriptor>>() == vector<RecordDescriptor> { rdesc1 });
        auto res = txn.find("books").get();
        assert(res[0].record.get("title").toText() == "Lion King");
        assert(res[0].record.get("price").toReal() == 50);
        assert(res[0].record.get("pages").toInt() == 400);
        assert(res[0].record.get("words").toBigIntU() == 90000ULL);
        assert(res[1].record.get("title").toText() == "Tarzan");
        assert(res[1].record.get("price").toReal() == 60);
        assert(res[1].record.get("pages").toInt() == 360);
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_vertex_book();
}

void test_sql_delete_vertex_with_rid()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1_1 = txn.addVertex("books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        auto v1_2 = txn.addVertex("books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2_1 = txn.addVertex("persons", r2);

        r3.set("time_used", 365U);
        auto e1 = txn.addEdge("authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        auto e2 = txn.addEdge("authors", v1_2, v2_1, r3);

        auto result = SQL::execute(txn, "DELETE VERTEX " + to_string(v2_1));
        assert(result.type() == result.RECORD_DESCRIPTORS);
        assert(result.get<vector<RecordDescriptor>>() == vector<RecordDescriptor> { v2_1 });

        try {
            auto record = txn.fetchRecord(v2_1);
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
        }
        auto record = txn.fetchRecord(v1_1);
        assert(!record.empty());
        record = txn.fetchRecord(v1_2);
        assert(!record.empty());
        try {
            auto record = txn.fetchRecord(e1);
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
        }
        try {
            auto record = txn.fetchRecord(e2);
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
        }
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_sql_delete_vertex_with_condition()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1_1 = txn.addVertex("books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        auto v1_2 = txn.addVertex("books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2_1 = txn.addVertex("persons", r2);

        r3.set("time_used", 365U);
        auto e1 = txn.addEdge("authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        auto e2 = txn.addEdge("authors", v1_2, v2_1, r3);

        auto result = SQL::execute(txn, "DELETE VERTEX persons WHERE name='J.K. Rowlings'");
        assert(result.type() == result.RECORD_DESCRIPTORS);
        assert(result.get<vector<RecordDescriptor>>() == vector<RecordDescriptor> { v2_1 });

        try {
            auto record = txn.fetchRecord(v2_1);
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
        }
        auto record = txn.fetchRecord(v1_1);
        assert(!record.empty());
        record = txn.fetchRecord(v1_2);
        assert(!record.empty());
        try {
            auto record = txn.fetchRecord(e1);
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
        }
        try {
            auto record = txn.fetchRecord(e2);
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
        }
    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_sql_delete_edge_with_rid()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1 = txn.addVertex("books", r1);
        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2 = txn.addVertex("persons", r2);
        r3.set("time_used", 365U);
        auto e1 = txn.addEdge("authors", v1, v2, r3);

        auto record = txn.fetchRecord(e1);
        assert(record.get("time_used").toIntU() == 365U);

        auto result = SQL::execute(txn, "DELETE EDGE " + to_string(e1));
        assert(result.type() == result.RECORD_DESCRIPTORS);
        assert(result.get<vector<RecordDescriptor>>() == vector<RecordDescriptor> { e1 });

        auto res = txn.find("authors").get();
        ASSERT_SIZE(res, 0);

    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_sql_delete_edge_with_condition()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    try {
        Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1 = txn.addVertex("books", r1);
        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2 = txn.addVertex("persons", r2);
        r3.set("time_used", 365U);
        auto e1 = txn.addEdge("authors", v1, v2, r3);

        auto record = txn.fetchRecord(e1);
        assert(record.get("time_used").toIntU() == 365U);

        auto result = SQL::execute(txn,
            "DELETE EDGE authors FROM (SELECT FROM books WHERE title='Harry Potter') TO (SELECT FROM persons WHERE "
            "name='J.K. Rowlings') WHERE time_used=365");
        assert(result.type() == result.RECORD_DESCRIPTORS);
        assert(result.get<vector<RecordDescriptor>>() == vector<RecordDescriptor> { e1 });

        auto res = txn.find("authors").get();
        ASSERT_SIZE(res, 0);

    } catch (const Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_sql_validate_property_type()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);

    SQL::execute(txn, "CREATE CLASS sql_valid_type IF NOT EXISTS EXTENDS VERTEX");
    SQL::execute(txn, "CREATE PROPERTY sql_valid_type.tiny IF NOT EXISTS TINYINT");
    SQL::execute(txn, "CREATE PROPERTY sql_valid_type.utiny IF NOT EXISTS UNSIGNED_TINYINT");
    SQL::execute(txn, "CREATE PROPERTY sql_valid_type.small IF NOT EXISTS SMALLINT");
    SQL::execute(txn, "CREATE PROPERTY sql_valid_type.usmall IF NOT EXISTS UNSIGNED_SMALLINT");
    SQL::execute(txn, "CREATE PROPERTY sql_valid_type.integer IF NOT EXISTS INTEGER");
    SQL::execute(txn, "CREATE PROPERTY sql_valid_type.uinteger IF NOT EXISTS UNSIGNED_INTEGER");
    SQL::execute(txn, "CREATE PROPERTY sql_valid_type.bigint IF NOT EXISTS BIGINT");
    SQL::execute(txn, "CREATE PROPERTY sql_valid_type.ubigint IF NOT EXISTS UNSIGNED_BIGINT");
    SQL::execute(txn, "CREATE PROPERTY sql_valid_type.text IF NOT EXISTS TEXT");
    SQL::execute(txn, "CREATE PROPERTY sql_valid_type.real IF NOT EXISTS REAL");
    SQL::execute(txn, "CREATE PROPERTY sql_valid_type.blob IF NOT EXISTS BLOB");

    try {
        Record props {};
        int8_t tiny = INT8_MIN;
        uint8_t utiny = UINT8_MAX;
        int16_t small = SHRT_MIN;
        uint16_t usmall = USHRT_MAX;
        int32_t integer = INT32_MIN;
        uint32_t uinteger = UINT32_MAX;
        int64_t bigint = INT64_MIN;
        uint64_t ubigint = UINT64_MAX;
        string baseText = "\"hello\" world'!\t\\";
        string text = "\"hello\" world\\'!\t\\\\";
        double real = 0.42;
        Coordinates blob(0.421, 0.842);

        props.set("tiny", tiny);
        props.set("utiny", utiny);
        props.set("small", small);
        props.set("usmall", usmall);
        props.set("integer", integer);
        props.set("uinteger", uinteger);
        props.set("bigint", bigint);
        props.set("ubigint", ubigint);
        props.set("text", baseText);
        props.set("real", real);
        props.set("blob", blob);
        txn.addVertex("sql_valid_type", props);

        string sqlCreate = (string("CREATE VERTEX sql_valid_type ") + "SET tiny=" + to_string(tiny)
            + ", utiny=" + to_string(utiny) + ", small=" + to_string(small) + ", usmall=" + to_string(usmall)
            + ", integer=" + to_string(integer) + ", uinteger=" + to_string(uinteger) + ", bigint=" + to_string(bigint)
            + ", ubigint=" + to_string(ubigint) + ", text='" + text + "'" + ", real=" + to_string(real) + ", blob=X'"
            + blob.toHex() + "'");
        SQL::execute(txn, sqlCreate);

        auto res = txn.find("sql_valid_type").get();
        ASSERT_SIZE(res, 2);

        res = txn.find("sql_valid_type")
                  .where(Condition("tiny").eq(tiny) && Condition("utiny").eq(utiny) && Condition("small").eq(small)
                      && Condition("usmall").eq(usmall) && Condition("integer").eq(integer)
                      && Condition("uinteger").eq(uinteger) && Condition("bigint").eq(bigint)
                      && Condition("ubigint").eq(ubigint) && Condition("text").eq(baseText)
                      && Condition("real").eq(real) && Condition("blob").eq(blob))
                  .get();
        ASSERT_SIZE(res, 2);

        string sqlSelect = (string("SELECT * FROM sql_valid_type ") + "WHERE tiny=" + to_string(tiny)
            + " AND utiny=" + to_string(utiny) + " AND small=" + to_string(small) + " AND usmall=" + to_string(usmall)
            + " AND integer=" + to_string(integer) + " AND uinteger=" + to_string(uinteger)
            + " AND bigint=" + to_string(bigint) + " AND ubigint=" + to_string(ubigint) + " AND text='" + text + "'"
            + " AND real=" + to_string(real) + " AND blob=X'" + blob.toHex() + "'");
        auto result = SQL::execute(txn, sqlSelect);
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>().size() == 2);
        assert(res == result.get<ResultSet>());
    } catch (const Error& e) {
        cout << "\nError: " << e.what() << endl;
        assert(false);
    }

    SQL::execute(txn, "DROP CLASS sql_valid_type IF EXISTS");
    txn.commit();
}

void test_sql_traverse()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    txn.addClass("V", ClassType::VERTEX);
    txn.addProperty("V", "p", PropertyType::TEXT);
    txn.addClass("EL", ClassType::EDGE);
    txn.addClass("ER", ClassType::EDGE);

    try {
        auto v1 = txn.addVertex("V", Record().set("p", "v1"));
        auto v21 = txn.addVertex("V", Record().set("p", "v21"));
        auto v22 = txn.addVertex("V", Record().set("p", "v22"));
        auto v31 = txn.addVertex("V", Record().set("p", "v31"));
        auto v32 = txn.addVertex("V", Record().set("p", "v32"));
        auto v33 = txn.addVertex("V", Record().set("p", "v33"));
        txn.addEdge("EL", v1, v21);
        txn.addEdge("ER", v1, v22);
        txn.addEdge("EL", v21, v31);
        txn.addEdge("ER", v21, v32);
        txn.addEdge("EL", v22, v33);

        SQL::Result result = SQL::execute(txn, "TRAVERSE all() FROM " + to_string(v21));
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.traverse(v21).depth(0, UINT_MAX).get());

        result = SQL::execute(txn, "TRAVERSE all() FROM " + to_string(v21) + ", " + to_string(v22));
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.traverse(v21).addSource(v22).depth(0, UINT_MAX).get());

        result = SQL::execute(txn, "TRAVERSE out() FROM " + to_string(v1));
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.traverseOut(v1).depth(0, UINT_MAX).get());

        result = SQL::execute(txn, "TRAVERSE out() FROM " + to_string(v22) + ", " + to_string(v31) + ", " + to_string(v32));
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.traverseOut(v22).addSource(v31).addSource(v32).depth(0, UINT_MAX).get());

        result = SQL::execute(txn, "TRAVERSE in() FROM " + to_string(v32));
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.traverseIn(v32).depth(0, UINT_MAX).get());

        result = SQL::execute(txn, "TRAVERSE out('EL') FROM " + to_string(v1));
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>()
            == txn.traverseOut(v1).depth(0, UINT_MAX).whereE(nogdb::GraphFilter {}.only("EL")).get());

        result = SQL::execute(txn, "TRAVERSE out('EL') FROM " + to_string(v21) + ", " + to_string(v22));
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>()
               == txn.traverseOut(v21).addSource(v22).depth(0, UINT_MAX)
               .whereE(nogdb::GraphFilter {}.only("EL")).get());

        result = SQL::execute(txn, "TRAVERSE in('ER') FROM " + to_string(v33) + " MINDEPTH 2");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>()
            == txn.traverseIn(v33).depth(2, UINT_MAX).whereE(nogdb::GraphFilter {}.only("ER")).get());

        result = SQL::execute(
            txn, "TRAVERSE all('EL') FROM " + to_string(v21) + " MINDEPTH 1 MAXDEPTH 1 STRATEGY BREADTH_FIRST");
        assert(result.type() == result.RESULT_SET);
        assert(result.get<ResultSet>() == txn.traverse(v21).depth(1, 1).whereE(nogdb::GraphFilter {}.only("EL")).get());

        result = SQL::execute(txn, "SELECT p FROM (TRAVERSE out() FROM " + to_string(v1) + ") WHERE p = 'v22'");
        assert(result.type() == result.RESULT_SET);
        {
            auto traverseResult = txn.traverseOut(v1).depth(0, UINT_MAX).get();
            vector<string> traverseRid(traverseResult.size());
            transform(traverseResult.cbegin(), traverseResult.cend(), traverseRid.begin(),
                [](const Result& r) { return rid2str(r.descriptor.rid); });
            auto selectResult
                = txn.find("V").where(Condition("@recordId").in(traverseRid) && Condition("p").eq("v22")).get();
            assert(result.get<ResultSet>() == selectResult);
        }
    } catch (const Error& e) {
        cout << "\nError: " << e.what() << endl;
        assert(false);
    }

    txn.dropClass("V");
    txn.dropClass("EL");
    txn.dropClass("ER");
    txn.commit();
}

void test_sql_create_index()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    txn.addClass("V", ClassType::VERTEX);
    txn.addProperty("V", "p", PropertyType::TEXT);

    try {
        SQL::Result result = SQL::execute(txn, "CREATE INDEX V.p");
        assert(result.type() == result.NO_RESULT);
        auto index = txn.getIndex("V", "p");
        assert(index.id != nogdb::IndexDescriptor {}.id);
        assert(index.unique == false);
        auto indexes = txn.getIndexes(txn.getClass("V"));
        assert(indexes.size() == 1);
    } catch (const Error& e) {
        cout << "\nError: " << e.what() << endl;
        assert(false);
    }

    txn.dropIndex("V", "p");
    txn.dropProperty("V", "p");
    txn.dropClass("V");
    txn.commit();
}

void test_sql_create_index_unique()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    txn.addClass("V", ClassType::VERTEX);
    txn.addProperty("V", "p", PropertyType::TEXT);

    try {
        SQL::Result result = SQL::execute(txn, "CREATE INDEX V.p UNIQUE");
        assert(result.type() == result.NO_RESULT);
        auto index = txn.getIndex("V", "p");
        assert(index.id != nogdb::IndexDescriptor {}.id);
        assert(index.unique == true);
        auto indexes = txn.getIndexes(txn.getClass("V"));
        assert(indexes.size() == 1);
    } catch (const Error& e) {
        cout << "\nError: " << e.what() << endl;
        assert(false);
    }

    txn.dropIndex("V", "p");
    txn.dropProperty("V", "p");
    txn.dropClass("V");
    txn.commit();
}

void test_sql_drop_index()
{
    auto txn = ctx->beginTxn(TxnMode::READ_WRITE);
    txn.addClass("V", ClassType::VERTEX);
    txn.addProperty("V", "p", PropertyType::TEXT);
    txn.addIndex("V", "p");

    try {
        SQL::Result result = SQL::execute(txn, "DROP INDEX V.p");
        assert(result.type() == result.NO_RESULT);

        try {
            auto index = txn.getIndex("V", "p");
            assert(false);
        } catch (const Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_INDEX, "NOGDB_CTX_NOEXST_INDEX");
        }

        auto indexes = txn.getIndexes(txn.getClass("V"));
        assert(indexes.size() == 0);
    } catch (const Error& e) {
        cout << "\nError: " << e.what() << endl;
        assert(false);
    }

    txn.dropClass("V");
    txn.commit();
}
