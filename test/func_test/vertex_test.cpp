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
#include "setup_cleanup.h"
#include <climits>
#include <set>
#include <vector>

void test_create_vertex()
{
    init_vertex_book();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r {};
        r.set("title", "Harry Potter");
        r.set("words", 4242424242);
        r.set("pages", 865);
        r.set("price", 49.99);
        txn.addVertex("books", r);

        r.clear();
        txn.addVertex("books", r);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_vertex_book();
}

void test_create_invalid_vertex()
{
    init_vertex_book();
    init_edge_author();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r {};
        r.set("profit", 1.0);
        txn.addVertex("authors", r);
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r {};
        r.set("author", "J.K. Rowling");
        txn.addVertex("books", r);
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r {};
        r.set("name", "J.K. Rowling");
        txn.addVertex("persons", r);
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    destroy_edge_author();
    destroy_vertex_book();
}

void test_create_vertices()
{
    init_vertex_book();
    init_vertex_person();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r1 {};
        r1.set("title", "Percy Jackson").set("pages", 456).set("price", 24.5);
        txn.addVertex("books", r1);
        r1.set("title", "Batman VS Superman").set("pages", 800).set("words", 9999999).set("price", 36.0);
        txn.addVertex("books", r1);
        nogdb::Record r2 {};
        r2.set("name", "Tom Hank").set("age", 58).set("salary", 45000);
        txn.addVertex("persons", r2);
        r2.set("name", "John Doe").set("age", 21).set("salary", 90000);
        txn.addVertex("persons", r2);
        r2.set("name", "Newt Scamander").set("age", 25).set("salary", 0).set("address", "Hogwarts");
        txn.addVertex("persons", r2);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_vertex_book();
    destroy_vertex_person();
}

void test_get_vertex()
{
    init_vertex_person();
    init_vertex_book();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        auto records = std::vector<nogdb::Record> {};
        records.push_back(nogdb::Record {}.set("title", "Percy Jackson").set("pages", 456).set("price", 24.5));
        records.push_back(
            nogdb::Record {}.set("title", "Batman VS Superman").set("words", 9999999ULL).set("price", 36.0));
        for (const auto& record : records) {
            txn.addVertex("books", record);
        }
        txn.addVertex("persons", nogdb::Record {}.set("name", "Jim Beans").set("age", 40U));
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto res = txn.find("books").get();
        ASSERT_SIZE(res, 2);

        assert(res[0].record.get("title").toText() == "Percy Jackson");
        assert(res[0].record.get("pages").toInt() == 456);
        assert(res[0].record.get("price").toReal() == 24.5);
        assert(res[0].record.get("words").empty());

        assert(res[1].record.get("title").toText() == "Batman VS Superman");
        assert(res[1].record.get("words").toBigIntU() == 9999999);
        assert(res[1].record.get("price").toReal() == 36.0);
        assert(res[1].record.get("pages").empty());

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto res = getVertexMultipleClass(txn, std::set<std::string> { "books", "persons" });
        ASSERT_SIZE(res, 3);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_vertex_book();
    destroy_vertex_person();
}

struct myobject {
    myobject() {};

    myobject(int x_, double y_, unsigned long long z_)
        : x { x_ }
        , y { y_ }
        , z { z_ }
    {
    }

    int x { 0 };
    double y { 0.0 };
    unsigned long long z { 0 };
};

void test_get_vertex_v2()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test", nogdb::ClassType::VERTEX);
        txn.addProperty("test", "integer", nogdb::PropertyType::INTEGER);
        txn.addProperty("test", "uinteger", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addProperty("test", "bigint", nogdb::PropertyType::BIGINT);
        txn.addProperty("test", "ubigint", nogdb::PropertyType::UNSIGNED_BIGINT);
        txn.addProperty("test", "real", nogdb::PropertyType::REAL);
        txn.addProperty("test", "text", nogdb::PropertyType::TEXT);
        txn.addProperty("test", "blob", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        nogdb::Record r {};
        myobject obj = { 42, 42.42, 424242 };
        r.set("integer", INT_MIN)
            .set("uinteger", UINT_MAX)
            .set("bigint", LLONG_MIN)
            .set("ubigint", ULLONG_MAX)
            .set("real", 0.42)
            .set("text", "hello world")
            .set("blob", obj);
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto rdesc = txn.addVertex("test", r);

        auto res = txn.find("test").get();
        assert(res[0].record.get("integer").toInt() == INT_MIN);
        assert(res[0].record.get("uinteger").toIntU() == UINT_MAX);
        assert(res[0].record.get("bigint").toBigInt() == LLONG_MIN);
        assert(res[0].record.get("ubigint").toBigIntU() == ULLONG_MAX);
        assert(res[0].record.get("real").toReal() == 0.42);
        assert(res[0].record.get("text").toText() == "hello world");
        auto obj_tmp = myobject {};
        res[0].record.get("blob").convertTo(obj_tmp);
        assert(obj_tmp.x == 42);
        assert(obj_tmp.y == 42.42);
        assert(obj_tmp.z == 424242);
        assert(res[0].record.getText("@recordId") == nogdb::rid2str(rdesc.rid));
        assert(res[0].record.getText("@className") == "test");
        assert(res[0].record.getIntU("@depth") == 0U);

        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("test");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_get_invalid_vertices()
{
    init_vertex_person();
    init_vertex_book();
    init_edge_author();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        auto v = txn.addVertex(
            "books", nogdb::Record {}.set("title", "Percy Jackson").set("pages", 456).set("price", 24.5));
        txn.addVertex("persons", nogdb::Record {}.set("name", "Jack Mah"));
        txn.addEdge("authors", v, v, nogdb::Record {}.set("time_used", 10U));
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("book").get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = getVertexMultipleClass(txn, std::set<std::string> { "books", "persons", "hello" });
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    destroy_edge_author();
    destroy_vertex_book();
    destroy_vertex_person();
}

void test_get_vertex_cursor()
{
    init_vertex_person();
    init_vertex_book();
    const auto testData = std::vector<std::string> { "Percy Jackson", "Captain America", "Batman VS Superman" };
    const auto testColumn = std::string { "title" };
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        for (const auto& data : testData) {
            txn.addVertex("books", nogdb::Record {}.set(testColumn, data));
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto res = txn.find("books").getCursor();
        cursorTester(res, testData, testColumn);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_vertex_book();
    destroy_vertex_person();
}

void test_get_invalid_vertex_cursor()
{
    init_vertex_person();
    init_vertex_book();
    init_edge_author();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        auto v = txn.addVertex(
            "books", nogdb::Record {}.set("title", "Percy Jackson").set("pages", 456).set("price", 24.5));
        txn.addVertex("persons", nogdb::Record {}.set("name", "Jack Mah"));
        txn.addEdge("authors", v, v, nogdb::Record {}.set("time_used", 10U));
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("book").get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    destroy_edge_author();
    destroy_vertex_book();
    destroy_vertex_person();
}

void test_update_vertex()
{
    init_vertex_book();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r {};
        r.set("title", "Lion King").set("price", 100.0).set("pages", 320);
        auto rdesc1 = txn.addVertex("books", r);
        r.set("title", "Tarzan").set("price", 60.0).set("pages", 360);
        auto rdesc2 = txn.addVertex("books", r);

        auto record = txn.fetchRecord(rdesc1);
        assert(record.get("title").toText() == "Lion King");
        assert(record.get("price").toReal() == 100);
        assert(record.get("pages").toInt() == 320);
        // assert(record.getVersion() == 1ULL);
        record.set("price", 50.0).set("pages", 400).set("words", 90000ULL);
        txn.update(rdesc1, record);

        auto res = txn.find("books").get();
        assert(res[0].record.get("title").toText() == "Lion King");
        assert(res[0].record.get("price").toReal() == 50);
        assert(res[0].record.get("pages").toInt() == 400);
        assert(res[0].record.get("words").toBigIntU() == 90000ULL);
        assert(res[0].record.getText("@recordId") == nogdb::rid2str(rdesc1.rid));

        assert(res[1].record.get("title").toText() == "Tarzan");
        assert(res[1].record.get("price").toReal() == 60);
        assert(res[1].record.get("pages").toInt() == 360);
        assert(res[1].record.getText("@recordId") == nogdb::rid2str(rdesc2.rid));

        txn.update(rdesc1, nogdb::Record {});
        res = txn.find("books").get();
        assert(res[0].record.empty() == true);
        assert(res[0].record.getText("@className") == "books");
        assert(res[0].record.getText("@recordId") == nogdb::rid2str(rdesc1.rid));

        assert(res[1].record.get("title").toText() == "Tarzan");
        assert(res[1].record.get("price").toReal() == 60);
        assert(res[1].record.get("pages").toInt() == 360);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_vertex_book();
}

void test_update_invalid_vertex()
{
    init_vertex_book();
    init_edge_author();

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.rollback();
        init_vertex_person();
        nogdb::Record r {};
        r.set("name", "H. Clinton").set("age", 55);
        txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto v1 = txn.addVertex("persons", r);
        txn.commit();
        destroy_vertex_person();
        txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        r.set("age", 60);
        txn.update(v1, r);
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r {};
        r.set("title", "The Lord").set("price", 420.0).set("pages", 810);
        auto rdesc = txn.addVertex("books", r);
        r.set("ISBN", "2343482991837");
        txn.update(rdesc, r);
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r {};
        r.set("title", "Lion King").set("price", 100.0).set("pages", 320);
        auto rdesc1 = txn.addVertex("books", r);
        r.set("title", "Tarzan").set("price", 60.0).set("pages", 360);
        auto rdesc2 = txn.addVertex("books", r);
        txn.remove(rdesc2);
        r.set("price", 50.0).set("pages", 400);
        txn.update(rdesc2, r);
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
    }
    destroy_edge_author();
    destroy_vertex_book();
}

void test_delete_vertex_only()
{
    init_vertex_book();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r {};
        r.set("title", "Lion King").set("price", 100.0).set("pages", 320);
        auto rdesc1 = txn.addVertex("books", r);
        r.set("title", "Tarzan").set("price", 60.0).set("pages", 360);
        auto rdesc2 = txn.addVertex("books", r);
        txn.remove(rdesc1);

        auto res = txn.find("books").get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("title").toText() == "Tarzan");
        assert(res[0].record.get("price").toReal() == 60);
        assert(res[0].record.get("pages").toInt() == 360);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_vertex_book();
}

void test_delete_invalid_vertex()
{
    init_vertex_book();
    auto rdesc1 = nogdb::RecordDescriptor {};
    auto rdesc2 = nogdb::RecordDescriptor {};
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r {};
        r.set("title", "Lion King").set("price", 100.0).set("pages", 320);
        rdesc1 = txn.addVertex("books", r);
        r.set("title", "Tarzan").set("price", 60.0).set("pages", 360);
        rdesc2 = txn.addVertex("books", r);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        auto tmp = rdesc1;
        tmp.rid.first = 9999U;
        txn.remove(tmp);
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    destroy_vertex_book();
}

void test_delete_all_vertices()
{
    init_vertex_book();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        std::vector<nogdb::Record> records {};
        records.push_back(nogdb::Record {}.set("title", "Lion King").set("price", 100.0).set("pages", 320));
        records.push_back(nogdb::Record {}.set("title", "Tarzan").set("price", 60.0).set("pages", 360));
        records.push_back(nogdb::Record {}.set("title", "Snow White").set("price", 80.0).set("pages", 280));
        for (const auto& record : records) {
            txn.addVertex("books", record);
        }
        auto res = txn.find("books").get();
        ASSERT_SIZE(res, 3);
        txn.removeAll("books");
        res = txn.find("books").get();
        ASSERT_SIZE(res, 0);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_vertex_book();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.removeAll("books");
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }
}

void test_get_edge_in()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1_1 = txn.addVertex("books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        auto v1_2 = txn.addVertex("books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        auto v1_3 = txn.addVertex("books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2_1 = txn.addVertex("persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        auto v2_2 = txn.addVertex("persons", r2);

        r3.set("time_used", 365U);
        txn.addEdge("authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        txn.addEdge("authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        txn.addEdge("authors", v1_3, v2_2, r3);

        auto in_edges = txn.findInEdge(v1_1).get();
        assert(in_edges.size() == 0);
        in_edges = txn.findInEdge(v1_2).get();
        assert(in_edges.size() == 0);
        in_edges = txn.findInEdge(v1_3).get();
        assert(in_edges.size() == 0);
        in_edges = txn.findInEdge(v2_1).get();
        assert(in_edges.size() == 2);
        assert(in_edges[0].record.get("time_used").toIntU() == 365U);
        assert(in_edges[1].record.get("time_used").toIntU() == 180U);
        in_edges = txn.findInEdge(v2_2).get();
        assert(in_edges.size() == 1);
        assert(in_edges[0].record.get("time_used").toIntU() == 430U);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_edge_out()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1_1 = txn.addVertex("books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        auto v1_2 = txn.addVertex("books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        auto v1_3 = txn.addVertex("books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2_1 = txn.addVertex("persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        auto v2_2 = txn.addVertex("persons", r2);

        r3.set("time_used", 365U);
        txn.addEdge("authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        txn.addEdge("authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        txn.addEdge("authors", v1_3, v2_2, r3);

        auto out_edges = txn.findOutEdge(v1_1).get();
        assert(out_edges.size() == 1);
        assert(out_edges[0].record.get("time_used").toIntU() == 365U);
        out_edges = txn.findOutEdge(v1_2).get();
        assert(out_edges.size() == 1);
        assert(out_edges[0].record.get("time_used").toIntU() == 180U);
        out_edges = txn.findOutEdge(v1_3).get();
        assert(out_edges.size() == 1);
        assert(out_edges[0].record.get("time_used").toIntU() == 430U);
        out_edges = txn.findOutEdge(v2_1).get();
        assert(out_edges.size() == 0);
        out_edges = txn.findOutEdge(v2_2).get();
        assert(out_edges.size() == 0);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_edge_all()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1_1 = txn.addVertex("books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        auto v1_2 = txn.addVertex("books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        auto v1_3 = txn.addVertex("books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2_1 = txn.addVertex("persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        auto v2_2 = txn.addVertex("persons", r2);

        r3.set("time_used", 365U);
        txn.addEdge("authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        txn.addEdge("authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        txn.addEdge("authors", v1_3, v2_2, r3);

        auto all_edges = txn.findEdge(v1_1).get();
        assert(all_edges.size() == 1);
        assert(all_edges[0].record.get("time_used").toIntU() == 365U);
        all_edges = txn.findEdge(v1_2).get();
        assert(all_edges.size() == 1);
        assert(all_edges[0].record.get("time_used").toIntU() == 180U);
        all_edges = txn.findEdge(v1_3).get();
        assert(all_edges.size() == 1);
        assert(all_edges[0].record.get("time_used").toIntU() == 430U);
        all_edges = txn.findEdge(v2_1).get();
        assert(all_edges.size() == 2);
        assert(all_edges[0].record.get("time_used").toIntU() == 365U);
        assert(all_edges[1].record.get("time_used").toIntU() == 180U);
        all_edges = txn.findInEdge(v2_2).get();
        assert(all_edges.size() == 1);
        assert(all_edges[0].record.get("time_used").toIntU() == 430U);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_invalid_edge_in()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    nogdb::RecordDescriptor v1_1 {}, v1_2 {}, v1_3 {}, v2_1 {}, v2_2 {};
    nogdb::RecordDescriptor e1 {}, e2 {}, e3 {};
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        v1_1 = txn.addVertex("books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        v1_2 = txn.addVertex("books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        v1_3 = txn.addVertex("books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        v2_1 = txn.addVertex("persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        v2_2 = txn.addVertex("persons", r2);

        r3.set("time_used", 365U);
        e1 = txn.addEdge("authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        e2 = txn.addEdge("authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        e3 = txn.addEdge("authors", v1_3, v2_2, r3);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = v1_1;
        tmp.rid.first = 9999U;
        auto res = txn.findInEdge(tmp).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = e1;
        auto res = txn.findInEdge(tmp).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = v1_1;
        tmp.rid.second = -1;
        auto res = txn.findInEdge(tmp).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_invalid_edge_out()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    nogdb::RecordDescriptor v1_1 {}, v1_2 {}, v1_3 {}, v2_1 {}, v2_2 {};
    nogdb::RecordDescriptor e1 {}, e2 {}, e3 {};
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        v1_1 = txn.addVertex("books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        v1_2 = txn.addVertex("books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        v1_3 = txn.addVertex("books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        v2_1 = txn.addVertex("persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        v2_2 = txn.addVertex("persons", r2);

        r3.set("time_used", 365U);
        e1 = txn.addEdge("authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        e2 = txn.addEdge("authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        e3 = txn.addEdge("authors", v1_3, v2_2, r3);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = v1_1;
        tmp.rid.first = 9999U;
        auto res = txn.findOutEdge(tmp).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = e1;
        auto res = txn.findOutEdge(tmp).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = v1_1;
        tmp.rid.second = -1;
        auto res = txn.findOutEdge(tmp).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_invalid_edge_all()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    nogdb::RecordDescriptor v1_1 {}, v1_2 {}, v1_3 {}, v2_1 {}, v2_2 {};
    nogdb::RecordDescriptor e1 {}, e2 {}, e3 {};
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        v1_1 = txn.addVertex("books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        v1_2 = txn.addVertex("books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        v1_3 = txn.addVertex("books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        v2_1 = txn.addVertex("persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        v2_2 = txn.addVertex("persons", r2);

        r3.set("time_used", 365U);
        e1 = txn.addEdge("authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        e2 = txn.addEdge("authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        e3 = txn.addEdge("authors", v1_3, v2_2, r3);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = v1_1;
        tmp.rid.first = 9999U;
        auto res = txn.findEdge(tmp).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = e1;
        auto res = txn.findEdge(tmp).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = v1_1;
        tmp.rid.second = -1;
        auto res = txn.findEdge(tmp).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_edge_in_cursor()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1_1 = txn.addVertex("books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        auto v1_2 = txn.addVertex("books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        auto v1_3 = txn.addVertex("books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2_1 = txn.addVertex("persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        auto v2_2 = txn.addVertex("persons", r2);

        r3.set("time_used", 365U);
        txn.addEdge("authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        txn.addEdge("authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        txn.addEdge("authors", v1_3, v2_2, r3);

        auto in_edges = txn.findInEdge(v1_1).getCursor();
        assert(in_edges.count() == 0);
        in_edges = txn.findInEdge(v1_2).getCursor();
        assert(in_edges.size() == 0);
        in_edges = txn.findInEdge(v1_3).getCursor();
        assert(in_edges.empty());
        in_edges = txn.findInEdge(v2_1).getCursor();
        assert(in_edges.size() == 2);
        in_edges.next();
        assert(in_edges->record.get("time_used").toIntU() == 365U);
        in_edges.next();
        assert(in_edges->record.get("time_used").toIntU() == 180U);
        in_edges = txn.findInEdge(v2_2).getCursor();
        assert(in_edges.size() == 1);
        in_edges.first();
        assert(in_edges->record.get("time_used").toIntU() == 430U);
        in_edges.last();
        assert(in_edges->record.get("time_used").toIntU() == 430U);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_edge_out_cursor()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1_1 = txn.addVertex("books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        auto v1_2 = txn.addVertex("books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        auto v1_3 = txn.addVertex("books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2_1 = txn.addVertex("persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        auto v2_2 = txn.addVertex("persons", r2);

        r3.set("time_used", 365U);
        txn.addEdge("authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        txn.addEdge("authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        txn.addEdge("authors", v1_3, v2_2, r3);

        auto out_edges = txn.findOutEdge(v1_1).getCursor();
        assert(out_edges.size() == 1);
        out_edges.first();
        assert(out_edges->record.get("time_used").toIntU() == 365U);
        out_edges = txn.findOutEdge(v1_2).getCursor();
        assert(out_edges.size() == 1);
        out_edges.next();
        assert(out_edges->record.get("time_used").toIntU() == 180U);
        out_edges = txn.findOutEdge(v1_3).getCursor();
        assert(out_edges.size() == 1);
        out_edges.to(0);
        assert(out_edges->record.get("time_used").toIntU() == 430U);
        out_edges = txn.findOutEdge(v2_1).getCursor();
        assert(out_edges.count() == 0);
        out_edges = txn.findOutEdge(v2_2).getCursor();
        assert(out_edges.empty());
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_edge_all_cursor()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1_1 = txn.addVertex("books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        auto v1_2 = txn.addVertex("books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        auto v1_3 = txn.addVertex("books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2_1 = txn.addVertex("persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        auto v2_2 = txn.addVertex("persons", r2);

        r3.set("time_used", 365U);
        txn.addEdge("authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        txn.addEdge("authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        txn.addEdge("authors", v1_3, v2_2, r3);

        auto all_edges = txn.findEdge(v1_1).getCursor();
        assert(all_edges.size() == 1);
        all_edges.first();
        assert(all_edges->record.get("time_used").toIntU() == 365U);
        all_edges = txn.findEdge(v1_2).getCursor();
        assert(all_edges.size() == 1);
        all_edges.to(0);
        assert(all_edges->record.get("time_used").toIntU() == 180U);
        all_edges = txn.findEdge(v1_3).getCursor();
        assert(all_edges.size() == 1);
        all_edges.last();
        assert(all_edges->record.get("time_used").toIntU() == 430U);
        all_edges = txn.findEdge(v2_1).getCursor();
        assert(all_edges.size() == 2);
        all_edges.to(0);
        assert(all_edges->record.get("time_used").toIntU() == 365U);
        all_edges.to(1);
        assert(all_edges->record.get("time_used").toIntU() == 180U);
        all_edges = txn.findEdge(v2_2).getCursor();
        assert(all_edges.size() == 1);
        all_edges.next();
        assert(all_edges->record.get("time_used").toIntU() == 430U);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_invalid_edge_in_cursor()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    nogdb::RecordDescriptor v1_1 {}, v1_2 {}, v1_3 {}, v2_1 {}, v2_2 {};
    nogdb::RecordDescriptor e1 {}, e2 {}, e3 {};
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        v1_1 = txn.addVertex("books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        v1_2 = txn.addVertex("books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        v1_3 = txn.addVertex("books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        v2_1 = txn.addVertex("persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        v2_2 = txn.addVertex("persons", r2);

        r3.set("time_used", 365U);
        e1 = txn.addEdge("authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        e2 = txn.addEdge("authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        e3 = txn.addEdge("authors", v1_3, v2_2, r3);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = v1_1;
        tmp.rid.first = 9999U;
        auto res = txn.findInEdge(tmp).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = e1;
        auto res = txn.findInEdge(tmp).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = v1_1;
        tmp.rid.second = -1;
        auto res = txn.findInEdge(tmp).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_invalid_edge_out_cursor()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    nogdb::RecordDescriptor v1_1 {}, v1_2 {}, v1_3 {}, v2_1 {}, v2_2 {};
    nogdb::RecordDescriptor e1 {}, e2 {}, e3 {};
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        v1_1 = txn.addVertex("books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        v1_2 = txn.addVertex("books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        v1_3 = txn.addVertex("books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        v2_1 = txn.addVertex("persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        v2_2 = txn.addVertex("persons", r2);

        r3.set("time_used", 365U);
        e1 = txn.addEdge("authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        e2 = txn.addEdge("authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        e3 = txn.addEdge("authors", v1_3, v2_2, r3);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = v1_1;
        tmp.rid.first = 9999U;
        auto res = txn.findOutEdge(tmp).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = e1;
        auto res = txn.findOutEdge(tmp).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = v1_1;
        tmp.rid.second = -1;
        auto res = txn.findOutEdge(tmp).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_invalid_edge_all_cursor()
{
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    nogdb::RecordDescriptor v1_1 {}, v1_2 {}, v1_3 {}, v2_1 {}, v2_2 {};
    nogdb::RecordDescriptor e1 {}, e2 {}, e3 {};
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record r1 {}, r2 {}, r3 {};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        v1_1 = txn.addVertex("books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        v1_2 = txn.addVertex("books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        v1_3 = txn.addVertex("books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        v2_1 = txn.addVertex("persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        v2_2 = txn.addVertex("persons", r2);

        r3.set("time_used", 365U);
        e1 = txn.addEdge("authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        e2 = txn.addEdge("authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        e3 = txn.addEdge("authors", v1_3, v2_2, r3);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = v1_1;
        tmp.rid.first = 9999U;
        auto res = txn.findEdge(tmp).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = e1;
        auto res = txn.findEdge(tmp).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = v1_1;
        tmp.rid.second = -1;
        auto res = txn.findEdge(tmp).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}
