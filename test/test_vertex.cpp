/*
 *  Copyright (C) 2018, Throughwave (Thailand) Co., Ltd.
 *  <peerawich at throughwave dot co dot th>
 *
 *  This file is part of libnogdb, the NogDB core library in C++.
 *
 *  libnogdb is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "runtest.h"
#include "test_prepare.h"
#include <climits>
#include <set>
#include <vector>

void test_create_vertex() {
    init_vertex_book();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r{};
        r.set("title", "Harry Potter");
        r.set("words", 4242424242);
        r.set("pages", 865);
        r.set("price", 49.99);
        nogdb::Vertex::create(txn, "books", r);

        r.clear();
        nogdb::Vertex::create(txn, "books", r);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_vertex_book();
}

void test_create_invalid_vertex() {
    init_vertex_book();
    init_edge_author();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r{};
        r.set("profit", 1.0);
        nogdb::Vertex::create(txn, "authors", r);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r{};
        r.set("author", "J.K. Rowling");
        nogdb::Vertex::create(txn, "books", r);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r{};
        r.set("name", "J.K. Rowling");
        nogdb::Vertex::create(txn, "persons", r);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    destroy_edge_author();
    destroy_vertex_book();
}

void test_create_vertices() {
    init_vertex_book();
    init_vertex_person();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r1{};
        r1.set("title", "Percy Jackson").set("pages", 456).set("price", 24.5);
        nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Batman VS Superman").set("pages", 800).set("words", 9999999).set("price", 36.0);
        nogdb::Vertex::create(txn, "books", r1);
        nogdb::Record r2{};
        r2.set("name", "Tom Hank").set("age", 58).set("salary", 45000);
        nogdb::Vertex::create(txn, "persons", r2);
        r2.set("name", "John Doe").set("age", 21).set("salary", 90000);
        nogdb::Vertex::create(txn, "persons", r2);
        r2.set("name", "Newt Scamander").set("age", 25).set("salary", 0).set("address", "Hogwarts");
        nogdb::Vertex::create(txn, "persons", r2);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_vertex_book();
    destroy_vertex_person();
}

void test_get_vertex() {
    init_vertex_person();
    init_vertex_book();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        auto records = std::vector<nogdb::Record>{};
        records.push_back(nogdb::Record{}
                                  .set("title", "Percy Jackson")
                                  .set("pages", 456)
                                  .set("price", 24.5)
        );
        records.push_back(nogdb::Record{}
                                  .set("title", "Batman VS Superman")
                                  .set("words", 9999999ULL)
                                  .set("price", 36.0)
        );
        for (const auto &record: records) {
            nogdb::Vertex::create(txn, "books", record);
        }
        nogdb::Vertex::create(txn, "persons", nogdb::Record{}
                .set("name", "Jim Beans")
                .set("age", 40U)
        );
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto res = nogdb::Vertex::get(txn, "books");
        assertSize(res, 2);

        assert(res[0].record.get("title").toText() == "Percy Jackson");
        assert(res[0].record.get("pages").toInt() == 456);
        assert(res[0].record.get("price").toReal() == 24.5);
        assert(res[0].record.get("words").empty());

        assert(res[1].record.get("title").toText() == "Batman VS Superman");
        assert(res[1].record.get("words").toBigIntU() == 9999999);
        assert(res[1].record.get("price").toReal() == 36.0);
        assert(res[1].record.get("pages").empty());

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto res = getVertexMultipleClass(txn, std::set<std::string>{"books", "persons"});
        assertSize(res, 3);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_vertex_book();
    destroy_vertex_person();
}

struct myobject {
    myobject() {};

    myobject(int x_, double y_, unsigned long long z_) : x{x_}, y{y_}, z{z_} {}

    int x{0};
    double y{0.0};
    unsigned long long z{0};
};

void test_get_vertex_v2() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "test", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "test", "integer", nogdb::PropertyType::INTEGER);
        nogdb::Property::add(txn, "test", "uinteger", nogdb::PropertyType::UNSIGNED_INTEGER);
        nogdb::Property::add(txn, "test", "bigint", nogdb::PropertyType::BIGINT);
        nogdb::Property::add(txn, "test", "ubigint", nogdb::PropertyType::UNSIGNED_BIGINT);
        nogdb::Property::add(txn, "test", "real", nogdb::PropertyType::REAL);
        nogdb::Property::add(txn, "test", "text", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "test", "blob", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        nogdb::Record r{};
        myobject obj = {42, 42.42, 424242};
        r.set("integer", INT_MIN)
                .set("uinteger", UINT_MAX)
                .set("bigint", LLONG_MIN)
                .set("ubigint", ULLONG_MAX)
                .set("real", 0.42)
                .set("text", "hello world")
                .set("blob", obj);
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        auto rdesc = nogdb::Vertex::create(txn, "test", r);

        auto res = nogdb::Vertex::get(txn, "test");
        assert(res[0].record.get("integer").toInt() == INT_MIN);
        assert(res[0].record.get("uinteger").toIntU() == UINT_MAX);
        assert(res[0].record.get("bigint").toBigInt() == LLONG_MIN);
        assert(res[0].record.get("ubigint").toBigIntU() == ULLONG_MAX);
        assert(res[0].record.get("real").toReal() == 0.42);
        assert(res[0].record.get("text").toText() == "hello world");
        auto obj_tmp = myobject{};
        res[0].record.get("blob").convertTo(obj_tmp);
        assert(obj_tmp.x == 42);
        assert(obj_tmp.y == 42.42);
        assert(obj_tmp.z == 424242);
        assert(res[0].record.getText("@recordId") == rid2str(rdesc.rid));
        assert(res[0].record.getText("@className") == "test");
        assert(res[0].record.getBigIntU("@version") == 1UL);
        assert(res[0].record.getIntU("@depth") == 0U);

        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "test");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_get_invalid_vertices() {
    init_vertex_person();
    init_vertex_book();
    init_edge_author();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        auto v = nogdb::Vertex::create(txn, "books",
                                       nogdb::Record{}
                                               .set("title", "Percy Jackson")
                                               .set("pages", 456).set("price", 24.5));
        nogdb::Vertex::create(txn, "persons", nogdb::Record{}.set("name", "Jack Mah"));
        nogdb::Edge::create(txn, "authors", v, v, nogdb::Record{}.set("time_used", 10U));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Vertex::get(txn, "book");
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = getVertexMultipleClass(txn, std::set<std::string>{"books", "persons", "hello"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Vertex::get(txn, "authors");
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = getVertexMultipleClass(txn, std::set<std::string>{"books", "authors"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    destroy_edge_author();
    destroy_vertex_book();
    destroy_vertex_person();
}

void test_get_vertex_cursor() {
    init_vertex_person();
    init_vertex_book();
    const auto testData = std::vector<std::string>{"Percy Jackson", "Captain America", "Batman VS Superman"};
    const auto testColumn = std::string{"title"};
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        for (const auto &data: testData) {
            nogdb::Vertex::create(txn, "books", nogdb::Record{}.set(testColumn, data));
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto res = nogdb::Vertex::getCursor(txn, "books");
        cursorTester(res, testData, testColumn);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_vertex_book();
    destroy_vertex_person();
}

void test_get_invalid_vertex_cursor() {
    init_vertex_person();
    init_vertex_book();
    init_edge_author();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        auto v = nogdb::Vertex::create(txn, "books",
                                       nogdb::Record{}
                                               .set("title", "Percy Jackson")
                                               .set("pages", 456)
                                               .set("price", 24.5));
        nogdb::Vertex::create(txn, "persons", nogdb::Record{}.set("name", "Jack Mah"));
        nogdb::Edge::create(txn, "authors", v, v, nogdb::Record{}.set("time_used", 10U));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Vertex::getCursor(txn, "book");
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Vertex::getCursor(txn, "authors");
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    destroy_edge_author();
    destroy_vertex_book();
    destroy_vertex_person();
}

void test_update_vertex() {
    init_vertex_book();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r{};
        r.set("title", "Lion King").set("price", 100.0).set("pages", 320);
        auto rdesc1 = nogdb::Vertex::create(txn, "books", r);
        r.set("title", "Tarzan").set("price", 60.0).set("pages", 360);
        auto rdesc2 = nogdb::Vertex::create(txn, "books", r);

        auto record = nogdb::Db::getRecord(txn, rdesc1);
        assert(record.get("title").toText() == "Lion King");
        assert(record.get("price").toReal() == 100);
        assert(record.get("pages").toInt() == 320);
        assert(record.getVersion() == 1ULL);
        record.set("price", 50.0).set("pages", 400).set("words", 90000ULL);
        nogdb::Vertex::update(txn, rdesc1, record);

        auto res = nogdb::Vertex::get(txn, "books");
        assert(res[0].record.get("title").toText() == "Lion King");
        assert(res[0].record.get("price").toReal() == 50);
        assert(res[0].record.get("pages").toInt() == 400);
        assert(res[0].record.get("words").toBigIntU() == 90000ULL);
        assert(res[0].record.getText("@recordId") == rid2str(rdesc1.rid));
        assert(res[0].record.getBigIntU("@version") == 1ULL);
        assert(res[0].record.getVersion() == 1ULL);

        assert(res[1].record.get("title").toText() == "Tarzan");
        assert(res[1].record.get("price").toReal() == 60);
        assert(res[1].record.get("pages").toInt() == 360);
        assert(res[1].record.getText("@recordId") == rid2str(rdesc2.rid));
        assert(res[1].record.getBigIntU("@version") == 1ULL);
        assert(res[1].record.getVersion() == 1ULL);

        nogdb::Vertex::update(txn, rdesc1, nogdb::Record{});
        res = nogdb::Vertex::get(txn, "books");
        assert(res[0].record.empty() == true);
        assert(res[0].record.getText("@className") == "books");
        assert(res[0].record.getText("@recordId") == rid2str(rdesc1.rid));
        assert(res[0].record.getVersion() == 1ULL);

        assert(res[1].record.get("title").toText() == "Tarzan");
        assert(res[1].record.get("price").toReal() == 60);
        assert(res[1].record.get("pages").toInt() == 360);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_vertex_book();
}

void test_update_vertex_version() {
    init_vertex_book();
    const int ITERATION = 10;
    try { // init
        nogdb::Txn txn = nogdb::Txn(*ctx, nogdb::Txn::Mode::READ_WRITE);
        nogdb::Record r{};
        r.set("title", "Lion King").set("price", 100.0).set("pages", 320);
        auto rdesc1 = nogdb::Vertex::create(txn, "books", r);
        r.set("title", "Tarzan").set("price", 60.0).set("pages", 360);
        auto rdesc2 = nogdb::Vertex::create(txn, "books", r);

        txn.commit();

    } catch (const nogdb::Error &ex) {
        std::cout << "Error in initialization step" << std::endl;
        std::cout << "Error: " << ex.what() << std::endl;
    }
    for (int i = 0; i < ITERATION; ++i) {
        try {
            nogdb::Txn txn = nogdb::Txn(*ctx, nogdb::Txn::Mode::READ_WRITE);
            nogdb::ResultSet res = nogdb::Vertex::get(txn, "books");
            nogdb::Result rec0 = res[0], rec1 = res[1];

            assert(rec0.record.getVersion() == 1ULL + i);
            nogdb::Vertex::update(txn, rec0.descriptor, rec0.record);

            assert(rec0.record.getVersion() == 2ULL + i);
            assert(rec1.record.getVersion() == 1ULL);

            txn.commit();

        } catch (const nogdb::Error &ex) {
            std::cout << "\n In Round " << i << std::endl;
            std::cout << "Error: " << ex.what() << std::endl;
            assert(false);
        }
    }
    destroy_vertex_book();
}
void test_update_invalid_vertex() {
    init_vertex_book();
    init_edge_author();

    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        txn.rollback();
        init_vertex_person();
        nogdb::Record r{};
        r.set("name", "H. Clinton").set("age", 55);
        txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        auto v1 = nogdb::Vertex::create(txn, "persons", r);
        txn.commit();
        destroy_vertex_person();
        txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        r.set("age", 60);
        nogdb::Vertex::update(txn, v1, r);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r1{}, r2{};
        r1.set("title", "Robin Hood").set("price", 80.0).set("pages", 300);
        auto v1 = nogdb::Vertex::create(txn, "books", r1);
        r2.set("profit", 0.0);
        auto e1 = nogdb::Edge::create(txn, "authors", v1, v1, r2);
        r2.set("profit", 42.42);
        nogdb::Vertex::update(txn, e1, r2);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r{};
        r.set("title", "The Lord").set("price", 420.0).set("pages", 810);
        auto rdesc = nogdb::Vertex::create(txn, "books", r);
        r.set("ISBN", "2343482991837");
        nogdb::Vertex::update(txn, rdesc, r);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r{};
        r.set("title", "Lion King").set("price", 100.0).set("pages", 320);
        auto rdesc1 = nogdb::Vertex::create(txn, "books", r);
        r.set("title", "Tarzan").set("price", 60.0).set("pages", 360);
        auto rdesc2 = nogdb::Vertex::create(txn, "books", r);
        nogdb::Vertex::destroy(txn, rdesc2);
        r.set("price", 50.0).set("pages", 400);
        nogdb::Vertex::update(txn, rdesc2, r);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
    destroy_edge_author();
    destroy_vertex_book();
}

void test_delete_vertex_only() {
    init_vertex_book();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r{};
        r.set("title", "Lion King").set("price", 100.0).set("pages", 320);
        auto rdesc1 = nogdb::Vertex::create(txn, "books", r);
        r.set("title", "Tarzan").set("price", 60.0).set("pages", 360);
        auto rdesc2 = nogdb::Vertex::create(txn, "books", r);
        nogdb::Vertex::destroy(txn, rdesc1);

        auto res = nogdb::Vertex::get(txn, "books");
        assertSize(res, 1);
        assert(res[0].record.get("title").toText() == "Tarzan");
        assert(res[0].record.get("price").toReal() == 60);
        assert(res[0].record.get("pages").toInt() == 360);
        nogdb::Vertex::destroy(txn, rdesc1);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_vertex_book();
}

void test_delete_invalid_vertex() {
    init_vertex_book();
    auto rdesc1 = nogdb::RecordDescriptor{};
    auto rdesc2 = nogdb::RecordDescriptor{};
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r{};
        r.set("title", "Lion King").set("price", 100.0).set("pages", 320);
        rdesc1 = nogdb::Vertex::create(txn, "books", r);
        r.set("title", "Tarzan").set("price", 60.0).set("pages", 360);
        rdesc2 = nogdb::Vertex::create(txn, "books", r);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        auto tmp = rdesc1;
        tmp.rid.first = 9999U;
        nogdb::Vertex::destroy(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    init_edge_author();
    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r{};
        r.set("time_used", 1U);
        auto e = nogdb::RecordDescriptor{};
        try {
            e = nogdb::Edge::create(txn, "authors", rdesc1, rdesc2, r);
        } catch (const nogdb::Error &ex) {
            std::cout << "\nError: " << ex.what() << std::endl;
            assert(false);
        }
        nogdb::Vertex::destroy(txn, e);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    destroy_edge_author();
    destroy_vertex_book();
}

void test_delete_all_vertices() {
    init_vertex_book();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        std::vector<nogdb::Record> records{};
        records.push_back(nogdb::Record{}.set("title", "Lion King").set("price", 100.0).set("pages", 320));
        records.push_back(nogdb::Record{}.set("title", "Tarzan").set("price", 60.0).set("pages", 360));
        records.push_back(nogdb::Record{}.set("title", "Snow White").set("price", 80.0).set("pages", 280));
        for (const auto &record: records) {
            nogdb::Vertex::create(txn, "books", record);
        }
        auto res = nogdb::Vertex::get(txn, "books");
        assertSize(res, 3);
        nogdb::Vertex::destroy(txn, "books");
        res = nogdb::Vertex::get(txn, "books");
        assertSize(res, 0);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_vertex_book();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Vertex::destroy(txn, "books");
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }
}

void test_get_edge_in() {
    init_vertex_book();
    init_vertex_person();
    init_edge_author();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r1{}, r2{}, r3{};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1_1 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        auto v1_2 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        auto v1_3 = nogdb::Vertex::create(txn, "books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2_1 = nogdb::Vertex::create(txn, "persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        auto v2_2 = nogdb::Vertex::create(txn, "persons", r2);

        r3.set("time_used", 365U);
        nogdb::Edge::create(txn, "authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        nogdb::Edge::create(txn, "authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        nogdb::Edge::create(txn, "authors", v1_3, v2_2, r3);

        auto in_edges = nogdb::Vertex::getInEdge(txn, v1_1);
        assert(in_edges.size() == 0);
        in_edges = nogdb::Vertex::getInEdge(txn, v1_2);
        assert(in_edges.size() == 0);
        in_edges = nogdb::Vertex::getInEdge(txn, v1_3);
        assert(in_edges.size() == 0);
        in_edges = nogdb::Vertex::getInEdge(txn, v2_1);
        assert(in_edges.size() == 2);
        assert(in_edges[0].record.get("time_used").toIntU() == 180U);
        assert(in_edges[1].record.get("time_used").toIntU() == 365U);
        in_edges = nogdb::Vertex::getInEdge(txn, v2_2);
        assert(in_edges.size() == 1);
        assert(in_edges[0].record.get("time_used").toIntU() == 430U);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_edge_out() {
    init_vertex_book();
    init_vertex_person();
    init_edge_author();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r1{}, r2{}, r3{};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1_1 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        auto v1_2 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        auto v1_3 = nogdb::Vertex::create(txn, "books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2_1 = nogdb::Vertex::create(txn, "persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        auto v2_2 = nogdb::Vertex::create(txn, "persons", r2);

        r3.set("time_used", 365U);
        nogdb::Edge::create(txn, "authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        nogdb::Edge::create(txn, "authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        nogdb::Edge::create(txn, "authors", v1_3, v2_2, r3);

        auto out_edges = nogdb::Vertex::getOutEdge(txn, v1_1);
        assert(out_edges.size() == 1);
        assert(out_edges[0].record.get("time_used").toIntU() == 365U);
        out_edges = nogdb::Vertex::getOutEdge(txn, v1_2);
        assert(out_edges.size() == 1);
        assert(out_edges[0].record.get("time_used").toIntU() == 180U);
        out_edges = nogdb::Vertex::getOutEdge(txn, v1_3);
        assert(out_edges.size() == 1);
        assert(out_edges[0].record.get("time_used").toIntU() == 430U);
        out_edges = nogdb::Vertex::getOutEdge(txn, v2_1);
        assert(out_edges.size() == 0);
        out_edges = nogdb::Vertex::getOutEdge(txn, v2_2);
        assert(out_edges.size() == 0);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_edge_all() {
    init_vertex_book();
    init_vertex_person();
    init_edge_author();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r1{}, r2{}, r3{};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1_1 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        auto v1_2 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        auto v1_3 = nogdb::Vertex::create(txn, "books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2_1 = nogdb::Vertex::create(txn, "persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        auto v2_2 = nogdb::Vertex::create(txn, "persons", r2);

        r3.set("time_used", 365U);
        nogdb::Edge::create(txn, "authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        nogdb::Edge::create(txn, "authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        nogdb::Edge::create(txn, "authors", v1_3, v2_2, r3);

        auto all_edges = nogdb::Vertex::getAllEdge(txn, v1_1);
        assert(all_edges.size() == 1);
        assert(all_edges[0].record.get("time_used").toIntU() == 365U);
        all_edges = nogdb::Vertex::getAllEdge(txn, v1_2);
        assert(all_edges.size() == 1);
        assert(all_edges[0].record.get("time_used").toIntU() == 180U);
        all_edges = nogdb::Vertex::getAllEdge(txn, v1_3);
        assert(all_edges.size() == 1);
        assert(all_edges[0].record.get("time_used").toIntU() == 430U);
        all_edges = nogdb::Vertex::getAllEdge(txn, v2_1);
        assert(all_edges.size() == 2);
        assert(all_edges[0].record.get("time_used").toIntU() == 365U);
        assert(all_edges[1].record.get("time_used").toIntU() == 180U);
        all_edges = nogdb::Vertex::getInEdge(txn, v2_2);
        assert(all_edges.size() == 1);
        assert(all_edges[0].record.get("time_used").toIntU() == 430U);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_invalid_edge_in() {
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    nogdb::RecordDescriptor v1_1{}, v1_2{}, v1_3{}, v2_1{}, v2_2{};
    nogdb::RecordDescriptor e1{}, e2{}, e3{};
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r1{}, r2{}, r3{};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        v1_1 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        v1_2 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        v1_3 = nogdb::Vertex::create(txn, "books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        v2_1 = nogdb::Vertex::create(txn, "persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        v2_2 = nogdb::Vertex::create(txn, "persons", r2);

        r3.set("time_used", 365U);
        e1 = nogdb::Edge::create(txn, "authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        e2 = nogdb::Edge::create(txn, "authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        e3 = nogdb::Edge::create(txn, "authors", v1_3, v2_2, r3);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = v1_1;
        tmp.rid.first = 9999U;
        auto res = nogdb::Vertex::getInEdge(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = e1;
        auto res = nogdb::Vertex::getInEdge(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = v1_1;
        tmp.rid.second = -1;
        auto res = nogdb::Vertex::getInEdge(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_invalid_edge_out() {
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    nogdb::RecordDescriptor v1_1{}, v1_2{}, v1_3{}, v2_1{}, v2_2{};
    nogdb::RecordDescriptor e1{}, e2{}, e3{};
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r1{}, r2{}, r3{};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        v1_1 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        v1_2 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        v1_3 = nogdb::Vertex::create(txn, "books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        v2_1 = nogdb::Vertex::create(txn, "persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        v2_2 = nogdb::Vertex::create(txn, "persons", r2);

        r3.set("time_used", 365U);
        e1 = nogdb::Edge::create(txn, "authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        e2 = nogdb::Edge::create(txn, "authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        e3 = nogdb::Edge::create(txn, "authors", v1_3, v2_2, r3);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = v1_1;
        tmp.rid.first = 9999U;
        auto res = nogdb::Vertex::getOutEdge(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = e1;
        auto res = nogdb::Vertex::getOutEdge(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = v1_1;
        tmp.rid.second = -1;
        auto res = nogdb::Vertex::getOutEdge(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_invalid_edge_all() {
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    nogdb::RecordDescriptor v1_1{}, v1_2{}, v1_3{}, v2_1{}, v2_2{};
    nogdb::RecordDescriptor e1{}, e2{}, e3{};
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r1{}, r2{}, r3{};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        v1_1 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        v1_2 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        v1_3 = nogdb::Vertex::create(txn, "books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        v2_1 = nogdb::Vertex::create(txn, "persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        v2_2 = nogdb::Vertex::create(txn, "persons", r2);

        r3.set("time_used", 365U);
        e1 = nogdb::Edge::create(txn, "authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        e2 = nogdb::Edge::create(txn, "authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        e3 = nogdb::Edge::create(txn, "authors", v1_3, v2_2, r3);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = v1_1;
        tmp.rid.first = 9999U;
        auto res = nogdb::Vertex::getAllEdge(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = e1;
        auto res = nogdb::Vertex::getAllEdge(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = v1_1;
        tmp.rid.second = -1;
        auto res = nogdb::Vertex::getAllEdge(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_edge_in_cursor() {
    init_vertex_book();
    init_vertex_person();
    init_edge_author();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r1{}, r2{}, r3{};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1_1 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        auto v1_2 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        auto v1_3 = nogdb::Vertex::create(txn, "books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2_1 = nogdb::Vertex::create(txn, "persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        auto v2_2 = nogdb::Vertex::create(txn, "persons", r2);

        r3.set("time_used", 365U);
        nogdb::Edge::create(txn, "authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        nogdb::Edge::create(txn, "authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        nogdb::Edge::create(txn, "authors", v1_3, v2_2, r3);

        auto in_edges = nogdb::Vertex::getInEdgeCursor(txn, v1_1);
        assert(in_edges.count() == 0);
        in_edges = nogdb::Vertex::getInEdgeCursor(txn, v1_2);
        assert(in_edges.size() == 0);
        in_edges = nogdb::Vertex::getInEdgeCursor(txn, v1_3);
        assert(in_edges.empty());
        in_edges = nogdb::Vertex::getInEdgeCursor(txn, v2_1);
        assert(in_edges.size() == 2);
        in_edges.next();
        assert(in_edges->record.get("time_used").toIntU() == 180U);
        in_edges.next();
        assert(in_edges->record.get("time_used").toIntU() == 365U);
        in_edges = nogdb::Vertex::getInEdgeCursor(txn, v2_2);
        assert(in_edges.size() == 1);
        in_edges.first();
        assert(in_edges->record.get("time_used").toIntU() == 430U);
        in_edges.last();
        assert(in_edges->record.get("time_used").toIntU() == 430U);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_edge_out_cursor() {
    init_vertex_book();
    init_vertex_person();
    init_edge_author();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r1{}, r2{}, r3{};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1_1 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        auto v1_2 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        auto v1_3 = nogdb::Vertex::create(txn, "books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2_1 = nogdb::Vertex::create(txn, "persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        auto v2_2 = nogdb::Vertex::create(txn, "persons", r2);

        r3.set("time_used", 365U);
        nogdb::Edge::create(txn, "authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        nogdb::Edge::create(txn, "authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        nogdb::Edge::create(txn, "authors", v1_3, v2_2, r3);

        auto out_edges = nogdb::Vertex::getOutEdgeCursor(txn, v1_1);
        assert(out_edges.size() == 1);
        out_edges.first();
        assert(out_edges->record.get("time_used").toIntU() == 365U);
        out_edges = nogdb::Vertex::getOutEdgeCursor(txn, v1_2);
        assert(out_edges.size() == 1);
        out_edges.next();
        assert(out_edges->record.get("time_used").toIntU() == 180U);
        out_edges = nogdb::Vertex::getOutEdgeCursor(txn, v1_3);
        assert(out_edges.size() == 1);
        out_edges.to(0);
        assert(out_edges->record.get("time_used").toIntU() == 430U);
        out_edges = nogdb::Vertex::getOutEdgeCursor(txn, v2_1);
        assert(out_edges.count() == 0);
        out_edges = nogdb::Vertex::getOutEdgeCursor(txn, v2_2);
        assert(out_edges.empty());
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_edge_all_cursor() {
    init_vertex_book();
    init_vertex_person();
    init_edge_author();
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r1{}, r2{}, r3{};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        auto v1_1 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        auto v1_2 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        auto v1_3 = nogdb::Vertex::create(txn, "books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        auto v2_1 = nogdb::Vertex::create(txn, "persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        auto v2_2 = nogdb::Vertex::create(txn, "persons", r2);

        r3.set("time_used", 365U);
        nogdb::Edge::create(txn, "authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        nogdb::Edge::create(txn, "authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        nogdb::Edge::create(txn, "authors", v1_3, v2_2, r3);

        auto all_edges = nogdb::Vertex::getAllEdgeCursor(txn, v1_1);
        assert(all_edges.size() == 1);
        all_edges.first();
        assert(all_edges->record.get("time_used").toIntU() == 365U);
        all_edges = nogdb::Vertex::getAllEdgeCursor(txn, v1_2);
        assert(all_edges.size() == 1);
        all_edges.to(0);
        assert(all_edges->record.get("time_used").toIntU() == 180U);
        all_edges = nogdb::Vertex::getAllEdgeCursor(txn, v1_3);
        assert(all_edges.size() == 1);
        all_edges.last();
        assert(all_edges->record.get("time_used").toIntU() == 430U);
        all_edges = nogdb::Vertex::getAllEdgeCursor(txn, v2_1);
        assert(all_edges.size() == 2);
        all_edges.to(0);
        assert(all_edges->record.get("time_used").toIntU() == 365U);
        all_edges.to(1);
        assert(all_edges->record.get("time_used").toIntU() == 180U);
        all_edges = nogdb::Vertex::getAllEdgeCursor(txn, v2_2);
        assert(all_edges.size() == 1);
        all_edges.next();
        assert(all_edges->record.get("time_used").toIntU() == 430U);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_invalid_edge_in_cursor() {
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    nogdb::RecordDescriptor v1_1{}, v1_2{}, v1_3{}, v2_1{}, v2_2{};
    nogdb::RecordDescriptor e1{}, e2{}, e3{};
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r1{}, r2{}, r3{};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        v1_1 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        v1_2 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        v1_3 = nogdb::Vertex::create(txn, "books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        v2_1 = nogdb::Vertex::create(txn, "persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        v2_2 = nogdb::Vertex::create(txn, "persons", r2);

        r3.set("time_used", 365U);
        e1 = nogdb::Edge::create(txn, "authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        e2 = nogdb::Edge::create(txn, "authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        e3 = nogdb::Edge::create(txn, "authors", v1_3, v2_2, r3);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = v1_1;
        tmp.rid.first = 9999U;
        auto res = nogdb::Vertex::getInEdgeCursor(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = e1;
        auto res = nogdb::Vertex::getInEdgeCursor(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = v1_1;
        tmp.rid.second = -1;
        auto res = nogdb::Vertex::getInEdgeCursor(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_invalid_edge_out_cursor() {
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    nogdb::RecordDescriptor v1_1{}, v1_2{}, v1_3{}, v2_1{}, v2_2{};
    nogdb::RecordDescriptor e1{}, e2{}, e3{};
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r1{}, r2{}, r3{};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        v1_1 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        v1_2 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        v1_3 = nogdb::Vertex::create(txn, "books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        v2_1 = nogdb::Vertex::create(txn, "persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        v2_2 = nogdb::Vertex::create(txn, "persons", r2);

        r3.set("time_used", 365U);
        e1 = nogdb::Edge::create(txn, "authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        e2 = nogdb::Edge::create(txn, "authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        e3 = nogdb::Edge::create(txn, "authors", v1_3, v2_2, r3);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = v1_1;
        tmp.rid.first = 9999U;
        auto res = nogdb::Vertex::getOutEdgeCursor(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = e1;
        auto res = nogdb::Vertex::getOutEdgeCursor(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = v1_1;
        tmp.rid.second = -1;
        auto res = nogdb::Vertex::getOutEdgeCursor(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}

void test_get_invalid_edge_all_cursor() {
    init_vertex_book();
    init_vertex_person();
    init_edge_author();

    nogdb::RecordDescriptor v1_1{}, v1_2{}, v1_3{}, v2_1{}, v2_2{};
    nogdb::RecordDescriptor e1{}, e2{}, e3{};
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record r1{}, r2{}, r3{};
        r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
        v1_1 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
        v1_2 = nogdb::Vertex::create(txn, "books", r1);
        r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
        v1_3 = nogdb::Vertex::create(txn, "books", r1);

        r2.set("name", "J.K. Rowlings").set("age", 32);
        v2_1 = nogdb::Vertex::create(txn, "persons", r2);
        r2.set("name", "David Lahm").set("age", 29);
        v2_2 = nogdb::Vertex::create(txn, "persons", r2);

        r3.set("time_used", 365U);
        e1 = nogdb::Edge::create(txn, "authors", v1_1, v2_1, r3);
        r3.set("time_used", 180U);
        e2 = nogdb::Edge::create(txn, "authors", v1_2, v2_1, r3);
        r3.set("time_used", 430U);
        e3 = nogdb::Edge::create(txn, "authors", v1_3, v2_2, r3);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = v1_1;
        tmp.rid.first = 9999U;
        auto res = nogdb::Vertex::getAllEdgeCursor(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = e1;
        auto res = nogdb::Vertex::getAllEdgeCursor(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = v1_1;
        tmp.rid.second = -1;
        auto res = nogdb::Vertex::getAllEdgeCursor(txn, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

    destroy_edge_author();
    destroy_vertex_person();
    destroy_vertex_book();
}
