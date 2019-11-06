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
#include <algorithm>
#include <set>

struct Coordinates {
    Coordinates() {};

    Coordinates(double x_, double y_)
        : x { x_ }
        , y { y_ }
    {
    }

    double x { 0.0 };
    double y { 0.0 };
};

void init_test_find()
{
    init_vertex_mountain();
    init_vertex_location();
    init_edge_street();
    init_edge_highway();
    init_edge_railway();
}

void destroy_test_find()
{
    destroy_edge_railway();
    destroy_edge_highway();
    destroy_edge_street();
    destroy_vertex_location();
    destroy_vertex_mountain();
}

void test_create_informative_graph()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.addVertex("mountains",
            nogdb::Record {}.set("name", "Fuji").set("temperature", -10).set("height", 3000U).set("rating", 5.0));
        txn.addVertex(
            "mountains", nogdb::Record {}.set("name", "Blue Mountain").set("temperature", 5).set("rating", 4.0));
        auto place1 = txn.addVertex("locations",
            nogdb::Record {}
                .set("name", "New York Tower")
                .set("temperature", 11)
                .set("postcode", 10200U)
                .set("price", 200000LL)
                .set("population", 2000ULL)
                .set("rating", 4.5)
                .set("coordinates", nogdb::Bytes { Coordinates { 42.42, 24.24 } }));

        auto place2 = txn.addVertex("locations",
            nogdb::Record {}
                .set("name", "Dubai Building")
                .set("temperature", 37)
                .set("price", 280000LL)
                .set("population", 1800ULL)
                .set("rating", 5.0)
                .set("coordinates", nogdb::Bytes { Coordinates { 112.89, -321.45 } }));

        auto place3 = txn.addVertex("locations",
            nogdb::Record {}
                .set("name", "Empire State Building")
                .set("postcode", 10250U)
                .set("price", 220000LL)
                .set("population", 2400ULL)
                .set("rating", 4.5)
                .set("coordinates", nogdb::Bytes { Coordinates { 242.42, -424.24 } }));

        auto place4 = txn.addVertex("locations",
            nogdb::Record {}
                .set("name", "ThaiCC Tower")
                .set("temperature", 28)
                .set("postcode", 11600U)
                .set("population", 900ULL)
                .set("rating", 3.5)
                .set("coordinates", nogdb::Bytes { Coordinates { -56.4242, 236.098 } }));

        auto place5 = txn.addVertex("locations",
            nogdb::Record {}
                .set("name", "Pentagon")
                .set("temperature", 18)
                .set("postcode", 10475U)
                .set("price", 300000LL)
                .set("population", 900ULL)
                .set("coordinates", nogdb::Bytes { Coordinates { -1.00, 1.00 } }));

        txn.addEdge("street", place5, place2,
            nogdb::Record {}
                .set("name", "George Street")
                .set("temperature", 20)
                .set("capacity", 300U)
                .set("distance", 40.5)
                .set("coordinates", nogdb::Bytes { Coordinates { 0.1, -0.1 } }));

        txn.addEdge("street", place3, place1,
            nogdb::Record {}
                .set("name", "Boyd Street")
                .set("capacity", 230U)
                .set("distance", 15.0)
                .set("coordinates", nogdb::Bytes { Coordinates { -9.335, 19.028 } }));

        txn.addEdge("street", place1, place5,
            nogdb::Record {}.set("name", "Henry Road").set("capacity", 1000U).set("distance", 50.45));

        txn.addEdge("street", place4, place5,
            nogdb::Record {}.set("name", "Isaac Street").set("capacity", 400U).set("distance", 33.42));

        txn.addEdge("street", place4, place2,
            nogdb::Record {}
                .set("name", "Cowboy Road")
                .set("capacity", 120U)
                .set("distance", 12.55)
                .set("coordinates", nogdb::Bytes { Coordinates { -334.51, 70.21 } }));

        txn.addEdge("street", place1, place2,
            nogdb::Record {}
                .set("name", "Andrew Street")
                .set("temperature", 28)
                .set("capacity", 420U)
                .set("distance", 42.42)
                .set("coordinates", nogdb::Bytes { Coordinates { -90.143, -172.68 } }));

        txn.addEdge("street", place2, place1,
            nogdb::Record {}
                .set("name", "Eddy Avenue")
                .set("capacity", 780U)
                .set("distance", 56.5)
                .set("coordinates", nogdb::Bytes { Coordinates { 0.00, 45.00 } }));

        txn.addEdge("street", place2, place4,
            nogdb::Record {}.set("name", "Fisher Avenue").set("capacity", 600U).set("distance", 36.20));

        txn.addEdge("street", place5, place3,
            nogdb::Record {}
                .set("name", "Jetty Road")
                .set("temperature", 32)
                .set("capacity", 530U)
                .set("distance", 70.5));

        txn.addEdge("street", place3, place4,
            nogdb::Record {}
                .set("name", "Doodee Street")
                .set("temperature", 40)
                .set("capacity", 100U)
                .set("distance", 8.42)
                .set("coordinates", nogdb::Bytes { Coordinates { -987.65, -65.789 } }));

        txn.addEdge("highway", place4, place1,
            nogdb::Record {}
                .set("name", "The Outer Ring A")
                .set("temperature", 36)
                .set("capacity", 3000U)
                .set("distance", 2200.45)
                .set("coordinates", nogdb::Bytes { Coordinates { 891.35, -301.393 } }));

        txn.addEdge("highway", place1, place5,
            nogdb::Record {}
                .set("name", "The Outer Ring B")
                .set("capacity", 3300U)
                .set("distance", 2400.8)
                .set("coordinates", nogdb::Bytes { Coordinates { -141.28, -3.942 } }));

        txn.addEdge("highway", place5, place4,
            nogdb::Record {}
                .set("name", "The Outer Ring C")
                .set("temperature", 32)
                .set("capacity", 3800U)
                .set("distance", 2980.75));

        txn.addEdge("railway", place1, place2,
            nogdb::Record {}
                .set("name", "Andy Way")
                .set("temperature", 42)
                .set("distance", 80.5)
                .set("coordinates", nogdb::Bytes { Coordinates { 84.15, -6.42 } }));

        txn.addEdge("railway", place1, place3,
            nogdb::Record {}
                .set("name", "Bamboo Way")
                .set("temperature", 43)
                .set("distance", 120.25)
                .set("coordinates", nogdb::Bytes { Coordinates { -44.67, -16.24 } }));

        txn.addEdge("railway", place1, place3,
            nogdb::Record {}.set("name", "Catalina Way").set("temperature", 37).set("distance", 112.44));

        txn.addEdge("railway", place1, place5, nogdb::Record {}.set("name", "Dwayne Way").set("distance", 150.75));

        txn.addEdge("railway", place2, place4,
            nogdb::Record {}
                .set("name", "Eastern Way")
                .set("temperature", 48)
                .set("distance", 78.5)
                .set("coordinates", nogdb::Bytes { Coordinates { 48.92, -115.222 } }));

        txn.addEdge("railway", place4, place5, nogdb::Record {}.set("name", "Gravity Way").set("distance", 254.35));

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_vertex()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("locations").where(nogdb::Condition("name").eq("Pentagon")).get();
        ASSERT_SIZE(res, 1);
        auto tmp = Coordinates {};
        res[0].record.get("coordinates").convertTo(tmp);
        assert(tmp.x == -1.00);
        assert(tmp.y == 1.00);
        res = txn.find("locations").where(nogdb::Condition("name").eq("Tokyo Tower")).get();
        ASSERT_SIZE(res, 0);
        res = txn.find("locations").where(nogdb::Condition("temperature").eq(18)).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "Pentagon");
        res = txn.find("locations").where(nogdb::Condition("postcode").eq(11600U)).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "ThaiCC Tower");
        res = txn.find("locations").where(nogdb::Condition("price").eq(280000LL)).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "Dubai Building");
        res = txn.find("locations").where(nogdb::Condition("population").eq(900ULL)).get();
        ASSERT_SIZE(res, 2);
        assert(res[0].record.get("name").toText() == "ThaiCC Tower");
        assert(res[1].record.get("name").toText() == "Pentagon");
        res = txn.find("locations").where(nogdb::Condition("rating").eq(4.5)).get();
        ASSERT_SIZE(res, 2);
        assert(res[0].record.get("name").toText() == "New York Tower");
        assert(res[1].record.get("name").toText() == "Empire State Building");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.find("locations").where(!nogdb::Condition("name").eq("Pentagon")).get();
        ASSERT_SIZE(res, 4);
        res = txn.find("locations").where(nogdb::Condition("temperature").gt(35)).get();
        ASSERT_SIZE(res, 1);
        res = txn.find("locations").where(nogdb::Condition("rating").ge(4.5)).get();
        ASSERT_SIZE(res, 3);
        res = txn.find("locations").where(nogdb::Condition("postcode").lt(10300U)).get();
        ASSERT_SIZE(res, 2);
        txn.find("locations").where(nogdb::Condition("population").le(900ULL)).get();
        ASSERT_SIZE(res, 2);
        res = txn.find("locations").where(!nogdb::Condition("price")).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "ThaiCC Tower");
        res = txn.find("locations").where(nogdb::Condition("temperature")).get();
        ASSERT_SIZE(res, 4);
        res = txn.find("locations").where(nogdb::Condition("name").eq(100)).get();
        ASSERT_SIZE(res, 0);
        res = txn.find("locations")
                  .where(nogdb::Condition("population").eq(static_cast<unsigned long long>(2000)))
                  .get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "New York Tower");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.find("locations").where(nogdb::Condition("name").contain("tag").ignoreCase()).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "Pentagon");
        res = txn.find("locations").where(!nogdb::Condition("name").contain("u").ignoreCase()).get();
        ASSERT_SIZE(res, 3);
        res = txn.find("locations").where(nogdb::Condition("name").beginWith("thai").ignoreCase()).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "ThaiCC Tower");
        res = txn.find("locations").where(!nogdb::Condition("name").beginWith("Thai")).get();
        ASSERT_SIZE(res, 4);
        res = txn.find("locations").where(nogdb::Condition("name").endWith("TOWER").ignoreCase()).get();
        ASSERT_SIZE(res, 2);
        res = txn.find("locations").where(!nogdb::Condition("name").endWith("Building")).get();
        ASSERT_SIZE(res, 3);
        res = txn.find("locations").where(nogdb::Condition("name").gt("Pentagon")).get();
        ASSERT_SIZE(res, 1);
        res = txn.find("locations").where(nogdb::Condition("name").ge("Pentagon")).get();
        ASSERT_SIZE(res, 2);
        res = txn.find("locations").where(nogdb::Condition("name").lt("Pentagon")).get();
        ASSERT_SIZE(res, 3);
        res = txn.find("locations").where(nogdb::Condition("name").le("Pentagon")).get();
        ASSERT_SIZE(res, 4);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_invalid_vertex()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("location").where(nogdb::Condition("name")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("locations").where(nogdb::Condition("names")).get();
        ASSERT_SIZE(res, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("locations").where(nogdb::Condition("coordinates").contain("invalid")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }
}

void test_find_edge()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("street").where(nogdb::Condition("name").eq("George Street")).get();
        ASSERT_SIZE(res, 1);
        auto tmp = Coordinates {};
        res[0].record.get("coordinates").convertTo(tmp);
        assert(tmp.x == 0.1);
        assert(tmp.y == -0.1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_invalid_edge()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("streets").where(nogdb::Condition("name")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("railway").where(nogdb::Condition("names")).get();
        ASSERT_SIZE(res, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("highway").where(nogdb::Condition("coordinates").contain("invalid")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }
}

void test_find_edge_in()
{
    auto cmp = [](const nogdb::Result& name1, const nogdb::Result& name2) {
        return name1.record.get("name").toText() < name2.record.get("name").toText();
    };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Dubai Building")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto condition1 = nogdb::Condition("name").eq("George Street");
        auto filter1 = nogdb::GraphFilter(condition1).only("street");
        auto res = txn.findInEdge(vertex.descriptor).where(filter1).get();
        ASSERT_SIZE(res, 1);
        auto condition2 = nogdb::Condition("distance").gt(40.0);
        auto filter2 = nogdb::GraphFilter(condition2).only("street");
        res = txn.findInEdge(vertex.descriptor).where(filter2).get();
        ASSERT_SIZE(res, 2);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Andrew Street");
        assert(res[1].record.get("name").toText() == "George Street");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Pentagon")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto condition1 = nogdb::Condition("name").eq("Isaac Street");
        auto filter1 = nogdb::GraphFilter(condition1).only("street", "railway");
        auto res = txn.findInEdge(vertex.descriptor).where(filter1).get();
        ASSERT_SIZE(res, 1);
        auto condition2 = nogdb::Condition("distance").lt(200.0);
        auto filter2 = nogdb::GraphFilter(condition2).only("street", "railway");
        res = txn.findInEdge(vertex.descriptor).where(filter2).get();
        ASSERT_SIZE(res, 3);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Dwayne Way");
        assert(res[1].record.get("name").toText() == "Henry Road");
        assert(res[2].record.get("name").toText() == "Isaac Street");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto condition1 = nogdb::Condition("name").eq("The Outer Ring C");
        auto filter1 = nogdb::GraphFilter(condition1);
        auto res = txn.findInEdge(vertex.descriptor).where(filter1).get();
        ASSERT_SIZE(res, 1);
        auto condition2 = nogdb::Condition("distance").ge(36.2);
        auto filter2 = nogdb::GraphFilter(condition2);
        res = txn.findInEdge(vertex.descriptor).where(filter2).get();
        ASSERT_SIZE(res, 3);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Eastern Way");
        assert(res[1].record.get("name").toText() == "Fisher Avenue");
        assert(res[2].record.get("name").toText() == "The Outer Ring C");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_edge_out()
{
    auto cmp = [](const nogdb::Result& name1, const nogdb::Result& name2) {
        return name1.record.get("name").toText() < name2.record.get("name").toText();
    };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("New York Tower")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto condition1 = nogdb::Condition("name").eq("Andrew Street");
        auto filter1 = nogdb::GraphFilter(condition1).only("street");
        auto res = txn.findOutEdge(vertex.descriptor).where(filter1).get();
        ASSERT_SIZE(res, 1);
        auto condition2 = nogdb::Condition("distance").ge(100.0);
        auto filter2 = nogdb::GraphFilter(condition2).only("railway");
        res = txn.findOutEdge(vertex.descriptor).where(filter2).get();
        ASSERT_SIZE(res, 3);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Bamboo Way");
        assert(res[1].record.get("name").toText() == "Catalina Way");
        assert(res[2].record.get("name").toText() == "Dwayne Way");
        auto condition3 = nogdb::Condition("temperature").le(42);
        auto filter3 = nogdb::GraphFilter(condition3).only("railway");
        res = txn.findOutEdge(vertex.descriptor).where(filter3).get();
        ASSERT_SIZE(res, 2);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Andy Way");
        assert(res[1].record.get("name").toText() == "Catalina Way");
        auto condition4 = !nogdb::Condition("temperature");
        auto filter4 = nogdb::GraphFilter(condition4).only("railway");
        res = txn.findOutEdge(vertex.descriptor).where(filter4).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "Dwayne Way");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("New York Tower")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto condition1 = !nogdb::Condition("name").eq("Andrew Street");
        auto filter1 = nogdb::GraphFilter(condition1).only("street", "railway");
        auto res = txn.findOutEdge(vertex.descriptor).where(filter1).get();
        ASSERT_SIZE(res, 5);
        auto condition2 = !nogdb::Condition("name").contain("boo");
        auto filter2 = nogdb::GraphFilter(condition2).only("street", "railway");
        res = txn.findOutEdge(vertex.descriptor).where(filter2).get();
        ASSERT_SIZE(res, 5);
        auto condition3 = nogdb::Condition("name").contain("BOO").ignoreCase();
        auto filter3 = nogdb::GraphFilter(condition3).only("street", "railway");
        res = txn.findOutEdge(vertex.descriptor).where(filter3).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "Bamboo Way");
        auto condition4 = !nogdb::Condition("name").beginWith("a").ignoreCase();
        auto filter4 = nogdb::GraphFilter(condition4).only("street", "railway");
        res = txn.findOutEdge(vertex.descriptor).where(filter4).get();
        ASSERT_SIZE(res, 4);
        auto condition5 = nogdb::Condition("name").beginWith("A");
        auto filter5 = nogdb::GraphFilter(condition5).only("street", "railway");
        res = txn.findOutEdge(vertex.descriptor).where(filter5).get();
        ASSERT_SIZE(res, 2);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("New York Tower")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto condition1 = nogdb::Condition("name").eq("The Outer Ring B");
        auto filter1 = nogdb::GraphFilter(condition1);
        auto res = txn.findOutEdge(vertex.descriptor).where(filter1).get();
        ASSERT_SIZE(res, 1);
        auto condition2 = !nogdb::Condition("name").endWith("StrEEt").ignoreCase();
        auto filter2 = nogdb::GraphFilter(condition2);
        res = txn.findOutEdge(vertex.descriptor).where(filter2).get();
        ASSERT_SIZE(res, 6);
        auto condition3 = nogdb::Condition("name").endWith("Way");
        auto filter3 = nogdb::GraphFilter(condition3);
        res = txn.findOutEdge(vertex.descriptor).where(filter3).get();
        ASSERT_SIZE(res, 4);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Andy Way");
        assert(res[1].record.get("name").toText() == "Bamboo Way");
        assert(res[2].record.get("name").toText() == "Catalina Way");
        assert(res[3].record.get("name").toText() == "Dwayne Way");
        auto condition4 = !nogdb::Condition("coordinates").null();
        auto filter4 = nogdb::GraphFilter(condition4);
        res = txn.findOutEdge(vertex.descriptor).where(filter4).get();
        ASSERT_SIZE(res, 4);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Andrew Street");
        assert(res[1].record.get("name").toText() == "Andy Way");
        assert(res[2].record.get("name").toText() == "Bamboo Way");
        assert(res[3].record.get("name").toText() == "The Outer Ring B");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_edge_all()
{
    auto cmp = [](const nogdb::Result& name1, const nogdb::Result& name2) {
        return name1.record.get("name").toText() < name2.record.get("name").toText();
    };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Pentagon")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto cond = nogdb::Condition("name").eq("George Street");
        auto res = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { cond }.only("street")).get();
        ASSERT_SIZE(res, 1);
        cond = nogdb::Condition("distance").ge(50.0);
        res = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { cond }.only("street")).get();
        ASSERT_SIZE(res, 2);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Henry Road");
        assert(res[1].record.get("name").toText() == "Jetty Road");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto classNames = std::vector<std::string> { "street", "railway" };
        auto cond = nogdb::Condition("distance").gt(100.0);
        auto res = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { cond }.only(classNames)).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "Gravity Way");
        cond = nogdb::Condition("distance").le(100.0);
        res = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { cond }.only(classNames)).get();
        ASSERT_SIZE(res, 5);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto cond = nogdb::Condition("capacity").eq(100U);
        auto res = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { cond }).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "Doodee Street");
        cond = nogdb::Condition("name").contain("C");
        res = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { cond }).get();
        ASSERT_SIZE(res, 2);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Cowboy Road");
        assert(res[1].record.get("name").toText() == "The Outer Ring C");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_invalid_edge_in()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& vertex = vertices[0];
    try {
        auto cond = nogdb::Condition("name").eq("Andrew Street");
        auto edges = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { cond }.only("streets")).get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto cond = nogdb::Condition("name").eq("Andrew Street");
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { cond }.only(classNames)).get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto cond = nogdb::Condition("names").eq("Andrew Street");
        auto edges = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { cond }.only("street")).get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto cond = nogdb::Condition("coordinates").contain("a");
        auto edges = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { cond }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto cond = nogdb::Condition("name").eq("Andrew Street");
        txn.findInEdge(edge.descriptor).where(nogdb::GraphFilter { cond }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto cond = nogdb::Condition("name").eq("Andrew Street");
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        txn.findInEdge(tmp).where(nogdb::GraphFilter { cond }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_invalid_edge_out()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& vertex = vertices[0];
    try {
        auto edges = txn.findOutEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("streets"))
                         .get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findOutEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only(classNames))
                         .get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges = txn.findOutEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("names").eq("Andrew Street") }.only("street"))
                         .get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges = txn.findOutEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("coordinates").contain("a") }.only("street"))
                         .get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto edges = txn.findOutEdge(edge.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("street"))
                         .get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto res = txn.findOutEdge(tmp)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("street"))
                       .get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_invalid_edge_all()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& vertex = vertices[0];
    try {
        auto edges = txn.findEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("streets"))
                         .get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only(classNames))
                         .get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges = txn.findEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("names").eq("Andrew Street") }.only("street"))
                         .get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges = txn.findEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("coordinates").contain("a") }.only("street"))
                         .get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto edges = txn.findEdge(edge.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("street"))
                         .get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto res = txn.findEdge(tmp)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("street"))
                       .get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_vertex_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("name").endWith("Building").ignoreCase() or nogdb::Condition("rating").ge(4.5);
        auto res = txn.find("locations").where(expr).get();
        ASSERT_SIZE(res, 3);
        assert(res[0].record.get("name").toText() == "New York Tower");
        assert(res[1].record.get("name").toText() == "Dubai Building");
        assert(res[2].record.get("name").toText() == "Empire State Building");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto expr1 = (nogdb::Condition("temperature").gt(0) and nogdb::Condition("rating").ge(3.0));
        auto expr2 = nogdb::Condition("population").le(900ULL);
        auto res = txn.find("mountains").where(expr1).get();
        auto res2 = txn.find("locations").where(expr1 or expr2).get();
        res.insert(res.end(), res2.cbegin(), res2.cend());
        ASSERT_SIZE(res, 5);
        assert(res[0].record.get("name").toText() == "Blue Mountain");
        assert(res[1].record.get("name").toText() == "New York Tower");
        assert(res[2].record.get("name").toText() == "Dubai Building");
        assert(res[3].record.get("name").toText() == "ThaiCC Tower");
        assert(res[4].record.get("name").toText() == "Pentagon");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto cond1 = nogdb::Condition("@className").eq("locations") and nogdb::Condition("temperature").lt(12);
        auto cond2 = nogdb::Condition("@className").eq("mountains") and nogdb::Condition("temperature").gt(0);
        auto res = txn.find("locations").where(cond1 or cond2).get();
        auto res2 = txn.find("mountains").where(cond1 or cond2).get();
        res.insert(res.end(), res2.cbegin(), res2.cend());
        ASSERT_SIZE(res, 2);
        for (const auto& r : res) {
            assert(
                r.record.get("name").toText() == "New York Tower" or r.record.get("name").toText() == "Blue Mountain");
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_invalid_vertex_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("name").endWith("Building").ignoreCase() or nogdb::Condition("rating").ge(4.5);
        auto res = txn.find("location").where(expr).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("names").endWith("Building").ignoreCase() or nogdb::Condition("rating").ge(4.5);
        auto res = txn.find("locations").where(expr).get();
        ASSERT_SIZE(res, 3);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr
            = nogdb::Condition("name").endWith("Building").ignoreCase() or nogdb::Condition("rating").contain("a");
        auto res = txn.find("locations").where(expr).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }
}

void test_find_edge_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr1 = nogdb::Condition("distance").lt(50.0) and nogdb::Condition("capacity").ge(300U);
        auto expr2 = nogdb::Condition("distance").ge(50.0) and nogdb::Condition("temperature").gt(30);
        auto classNames = std::set<std::string> { "street", "highway" };
        auto res = nogdb::ResultSet {};
        for (const auto& className : classNames) {
            auto tmp = txn.find(className).where(expr1 or expr2).get();
            res.insert(res.end(), tmp.cbegin(), tmp.cend());
        }
        auto tmp = txn.find("railway").where(expr2).get();
        res.insert(res.end(), tmp.cbegin(), tmp.cend());
        ASSERT_SIZE(res, 11);
        auto elements = std::vector<std::string> { "George Street", "Isaac Street", "Andrew Street", "Fisher Avenue",
            "Jetty Road", "The Outer Ring A", "The Outer Ring C", "Andy Way", "Bamboo Way", "Catalina Way",
            "Eastern Way" };
        assert(compareText(res, "name", elements));
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto cond1 = nogdb::Condition("@className").eq("highway") and nogdb::Condition("name").endWith("B");
        auto cond2 = nogdb::Condition("@className").eq("railway") and nogdb::Condition("name").beginWith("C");
        auto classNames = std::set<std::string> { "street", "highway", "railway" };
        auto res = nogdb::ResultSet {};
        for (const auto& className : classNames) {
            auto tmp = txn.find(className).where(cond1 or cond2).get();
            res.insert(res.end(), tmp.cbegin(), tmp.cend());
        }
        ASSERT_SIZE(res, 2);
        for (const auto& r : res) {
            assert(
                r.record.get("name").toText() == "The Outer Ring B" or r.record.get("name").toText() == "Catalina Way");
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_invalid_edge_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("distance").lt(50.0) and nogdb::Condition("capacity").ge(300U);
        auto res = txn.find("streets").where(expr).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("distance").lt(50.0) and nogdb::Condition("capacityyy").ge(300U);
        auto res = txn.find("street").where(expr).get();
        ASSERT_SIZE(res, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("distance").lt(50.0) and nogdb::Condition("capacity").contain("a");
        auto res = txn.find("street").where(expr).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }
}

void test_find_edge_in_with_expression()
{
    auto cmp = [](const nogdb::Result& name1, const nogdb::Result& name2) {
        return name1.record.get("name").toText() < name2.record.get("name").toText();
    };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Dubai Building")).get();
        ASSERT_SIZE(vertices, 1);
        auto& vertex = vertices[0];
        auto expr = nogdb::Condition("distance").ge(80.0) or nogdb::Condition("capacity").gt(400U)
            or nogdb::Condition("temperature").lt(30);
        auto res = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }).get();
        ASSERT_SIZE(res, 3);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Andrew Street");
        assert(res[1].record.get("name").toText() == "Andy Way");
        assert(res[2].record.get("name").toText() == "George Street");

        res = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter {}.only("street")).get();
        ASSERT_SIZE(res, 3);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Andrew Street");
        assert(res[1].record.get("name").toText() == "Cowboy Road");
        assert(res[2].record.get("name").toText() == "George Street");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_edge_out_with_expression()
{
    auto cmp = [](const nogdb::Result& name1, const nogdb::Result& name2) {
        return name1.record.get("name").toText() < name2.record.get("name").toText();
    };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("New York Tower")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto expr = nogdb::Condition("name").contain("Road").ignoreCase()
            or (nogdb::Condition("temperature").null() and nogdb::Condition("capacity").ge(2000U))
            or (nogdb::Condition("temperature").gt(40) and nogdb::Condition("distance").lt(140.0));
        auto res = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street")).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "Henry Road");

        res = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street", "highway")).get();
        ASSERT_SIZE(res, 2);
        for (const auto& r : res) {
            assert(
                r.record.get("name").toText() == "Henry Road" || r.record.get("name").toText() == "The Outer Ring B");
        }

        res = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }).get();
        ASSERT_SIZE(res, 4);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Andy Way");
        assert(res[1].record.get("name").toText() == "Bamboo Way");
        assert(res[2].record.get("name").toText() == "Henry Road");
        assert(res[3].record.get("name").toText() == "The Outer Ring B");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_edge_all_with_expression()
{
    auto cmp = [](const nogdb::Result& name1, const nogdb::Result& name2) {
        return name1.record.get("name").toText() < name2.record.get("name").toText();
    };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Pentagon")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto expr = nogdb::Condition("temperature") and nogdb::Condition("capacity")
            and nogdb::Condition("distance").gt(40.0);
        auto res = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street")).get();
        ASSERT_SIZE(res, 2);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "George Street");
        assert(res[1].record.get("name").toText() == "Jetty Road");

        res = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }).get();
        ASSERT_SIZE(res, 3);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "George Street");
        assert(res[1].record.get("name").toText() == "Jetty Road");
        assert(res[2].record.get("name").toText() == "The Outer Ring C");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Pentagon")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto cond1
            = nogdb::Condition("@className").eq("street") and nogdb::Condition("name").contain("street").ignoreCase();
        auto cond2 = nogdb::Condition("@className").eq("highway") and nogdb::Condition("name").endWith("C");
        auto res = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { cond1 or cond2 }).get();
        ASSERT_SIZE(res, 3);
        for (const auto& r : res) {
            assert(r.record.get("name").toText() == "The Outer Ring C"
                || r.record.get("name").toText() == "Isaac Street" || r.record.get("name").toText() == "George Street");
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_invalid_edge_in_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& vertex = vertices[0];
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("streets")).get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only(classNames)).get();
        ASSERT_SIZE(edges, 3);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("names").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street")).get();
        ASSERT_SIZE(edges, 2);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("distance").contain("a");
        auto edges = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findInEdge(edge.descriptor).where(nogdb::GraphFilter { expr }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto res = txn.findInEdge(tmp).where(nogdb::GraphFilter { expr }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_invalid_edge_out_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& vertex = vertices[0];
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("streets")).get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only(classNames)).get();
        ASSERT_SIZE(edges, 3);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("names").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street")).get();
        ASSERT_SIZE(edges, 2);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("distance").contain("a");
        auto edges = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findOutEdge(edge.descriptor).where(nogdb::GraphFilter { expr }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto res = txn.findOutEdge(tmp).where(nogdb::GraphFilter { expr }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_invalid_edge_all_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& vertex = vertices[0];
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("streets")).get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only(classNames)).get();
        ASSERT_SIZE(edges, 6);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("names").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street")).get();
        ASSERT_SIZE(edges, 4);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("distance").contain("a");
        auto edges = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findEdge(edge.descriptor).where(nogdb::GraphFilter { expr }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto res = txn.findEdge(tmp).where(nogdb::GraphFilter { expr }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_vertex_condition_function()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("locations")
                       .where([](const nogdb::Record& record) {
                           return record.get("name").toText().find("Building") != std::string::npos
                               || (!record.get("rating").empty() && record.get("rating").toReal() >= 4.5);
                       })
                       .get();
        ASSERT_SIZE(res, 3);
        assert(res[0].record.get("name").toText() == "New York Tower");
        assert(res[1].record.get("name").toText() == "Dubai Building");
        assert(res[2].record.get("name").toText() == "Empire State Building");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto cmp = [](const nogdb::Record& record) {
            return ((!record.get("temperature").empty() && record.get("temperature").toInt() > 0)
                       && (!record.get("rating").empty() && record.get("rating").toReal() >= 3.0))
                || (!record.get("population").empty() && record.get("population").toBigIntU() <= 900ULL);
        };
        auto res = txn.find("mountains").where(cmp).get();
        auto res2 = txn.find("locations").where(cmp).get();
        res.insert(res.end(), res2.cbegin(), res2.cend());
        ASSERT_SIZE(res, 5);
        assert(res[0].record.get("name").toText() == "Blue Mountain");
        assert(res[1].record.get("name").toText() == "New York Tower");
        assert(res[2].record.get("name").toText() == "Dubai Building");
        assert(res[3].record.get("name").toText() == "ThaiCC Tower");
        assert(res[4].record.get("name").toText() == "Pentagon");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto cmp = [](const nogdb::Record& record) {
            if (!record.get("temperature").empty()) {
                return (record.getText("@className") == "locations" && record.getInt("temperature") < 12)
                    || (record.getText("@className") == "mountains" && record.getInt("temperature") > 0);
            } else {
                return false;
            }
        };
        auto res = txn.find("locations").where(cmp).get();
        auto res2 = txn.find("mountains").where(cmp).get();
        res.insert(res.end(), res2.cbegin(), res2.cend());
        ASSERT_SIZE(res, 2);
        for (const auto& r : res) {
            assert(
                r.record.get("name").toText() == "New York Tower" || r.record.get("name").toText() == "Blue Mountain");
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_edge_condition_function()
{
    auto test_condition_function_1 = [](const nogdb::Record& record) {
        if (record.get("name").empty())
            return false;
        return (record.get("name").toText() == "George Street");
    };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("street").where(test_condition_function_1).get();
        ASSERT_SIZE(res, 1);
        auto tmp = Coordinates {};
        res[0].record.get("coordinates").convertTo(tmp);
        assert(tmp.x == 0.1);
        assert(tmp.y == -0.1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_edge_in_condition_function()
{
    auto cmp = [](const nogdb::Result& name1, const nogdb::Result& name2) {
        return name1.record.get("name").toText() < name2.record.get("name").toText();
    };

    auto test_condition_function_4 = [](const nogdb::Record& record) {
        if (record.get("distance").empty())
            return false;
        return (record.get("distance").toReal() > 40.0);
    };

    auto test_condition_function_5 = [](const nogdb::Record& record) {
        if (record.get("distance").empty())
            return false;
        return (record.get("distance").toReal() < 200.0);
    };

    auto test_condition_function_6 = [](const nogdb::Record& record) {
        if (record.get("distance").empty())
            return false;
        return (record.get("distance").toReal() >= 36.2);
    };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Dubai Building")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto res = txn.findInEdge(vertex.descriptor)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("George Street") }.only("street"))
                       .get();
        ASSERT_SIZE(res, 1);
        res = txn.findInEdge(vertex.descriptor)
                  .where(nogdb::GraphFilter { test_condition_function_4 }.only("street"))
                  .get();
        ASSERT_SIZE(res, 2);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Andrew Street");
        assert(res[1].record.get("name").toText() == "George Street");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Pentagon")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto classNames = std::vector<std::string> { "street", "railway" };
        auto res = txn.findInEdge(vertex.descriptor)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Isaac Street") }.only(classNames))
                       .get();
        ASSERT_SIZE(res, 1);
        res = txn.findInEdge(vertex.descriptor)
                  .where(nogdb::GraphFilter { test_condition_function_5 }.only(classNames))
                  .get();
        ASSERT_SIZE(res, 3);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Dwayne Way");
        assert(res[1].record.get("name").toText() == "Henry Road");
        assert(res[2].record.get("name").toText() == "Isaac Street");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto res = txn.findInEdge(vertex.descriptor)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("The Outer Ring C") })
                       .get();
        ASSERT_SIZE(res, 1);
        res = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { test_condition_function_6 }).get();
        ASSERT_SIZE(res, 3);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Eastern Way");
        assert(res[1].record.get("name").toText() == "Fisher Avenue");
        assert(res[2].record.get("name").toText() == "The Outer Ring C");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_edge_out_condition_function()
{
    auto cmp = [](const nogdb::Result& name1, const nogdb::Result& name2) {
        return name1.record.get("name").toText() < name2.record.get("name").toText();
    };

    auto test_condition_function_7 = [](const nogdb::Record& record) {
        if (record.get("distance").empty())
            return false;
        return (record.get("distance").toReal() >= 100.0);
    };

    auto test_condition_function_8 = [](const nogdb::Record& record) {
        auto tmp = record.get("temperature");
        if (tmp.empty())
            return false;
        return (tmp.toInt() <= 42);
    };

    auto test_condition_function_9 = [](const nogdb::Record& record) { return record.get("temperature").empty(); };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("New York Tower")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto res = txn.findOutEdge(vertex.descriptor)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("street"))
                       .get();
        ASSERT_SIZE(res, 1);
        res = txn.findOutEdge(vertex.descriptor)
                  .where(nogdb::GraphFilter { test_condition_function_7 }.only("railway"))
                  .get();
        ASSERT_SIZE(res, 3);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Bamboo Way");
        assert(res[1].record.get("name").toText() == "Catalina Way");
        assert(res[2].record.get("name").toText() == "Dwayne Way");
        res = txn.findOutEdge(vertex.descriptor)
                  .where(nogdb::GraphFilter { test_condition_function_8 }.only("railway"))
                  .get();
        ASSERT_SIZE(res, 2);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Andy Way");
        assert(res[1].record.get("name").toText() == "Catalina Way");
        res = txn.findOutEdge(vertex.descriptor)
                  .where(nogdb::GraphFilter { test_condition_function_9 }.only("railway"))
                  .get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "Dwayne Way");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_find_edge_all_condition_function()
{
    auto cmp = [](const nogdb::Result& name1, const nogdb::Result& name2) {
        return name1.record.get("name").toText() < name2.record.get("name").toText();
    };

    auto test_condition_function_10 = [](const nogdb::Record& record) {
        if (record.get("distance").empty())
            return false;
        return (record.get("distance").toReal() > 100);
    };

    auto test_condition_function_11 = [](const nogdb::Record& record) {
        if (record.get("distance").empty())
            return false;
        return (record.get("distance").toReal() <= 100);
    };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Pentagon")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto res = txn.findEdge(vertex.descriptor)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("George Street") }.only("street"))
                       .get();
        ASSERT_SIZE(res, 1);
        res = txn.findEdge(vertex.descriptor)
                  .where(nogdb::GraphFilter { nogdb::Condition("distance").ge(50.0) }.only("street"))
                  .get();
        ASSERT_SIZE(res, 2);
        std::sort(res.begin(), res.end(), cmp);
        assert(res[0].record.get("name").toText() == "Henry Road");
        assert(res[1].record.get("name").toText() == "Jetty Road");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        assert(vertices.size() == 1);
        auto& vertex = vertices[0];
        auto classNames = std::vector<std::string> { "street", "railway" };
        auto res = txn.findEdge(vertex.descriptor)
                       .where(nogdb::GraphFilter { test_condition_function_10 }.only(classNames))
                       .get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "Gravity Way");
        res = txn.findEdge(vertex.descriptor)
                  .where(nogdb::GraphFilter { test_condition_function_11 }.only(classNames))
                  .get();
        ASSERT_SIZE(res, 5);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_invalid_vertex_condition_function()
{
    auto condition = [](const nogdb::Record& record) {
        return record.get("name").toText().find("Building") != std::string::npos
            || record.get("rating").toReal() >= 4.5;
    };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("location").where(condition).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }
}

void test_find_invalid_edge_condition_function()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("streets")
                       .where([](const nogdb::Record& record) {
                           return record.get("distance").toReal() < 50.0 && record.get("capacity").toIntU() >= 300U;
                       })
                       .get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }
}

void test_find_invalid_edge_in_condition_function()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    auto& vertex = vertices[0];
    auto condition = [](const nogdb::Record& record) {
        return (record.get("name").toText().find("Street") != std::string::npos) || !record.get("distance").empty();
    };

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { condition }.only("streets")).get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { condition }.only(classNames)).get();
        ASSERT_SIZE(edges, 3);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto edges = txn.findInEdge(edge.descriptor).where(nogdb::GraphFilter { condition }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto res = txn.findInEdge(tmp).where(nogdb::GraphFilter { condition }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_invalid_edge_out_condition_function()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    auto& vertex = vertices[0];
    auto condition = [](const nogdb::Record& record) {
        return (record.get("name").toText().find("Street") != std::string::npos) || !record.get("distance").empty();
    };

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { condition }.only("streets")).get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { condition }.only(classNames)).get();
        ASSERT_SIZE(edges, 3);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto edges = txn.findOutEdge(edge.descriptor).where(nogdb::GraphFilter { condition }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto res = txn.findOutEdge(tmp).where(nogdb::GraphFilter { condition }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_invalid_edge_all_condition_function()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    auto& vertex = vertices[0];
    auto condition = [](const nogdb::Record& record) {
        return (record.get("name").toText().find("Street") != std::string::npos) || !record.get("distance").empty();
    };
    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { condition }.only("streets")).get();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { condition }.only(classNames)).get();
        ASSERT_SIZE(edges, 6);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto edges = txn.findEdge(edge.descriptor).where(nogdb::GraphFilter { condition }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto res = txn.findEdge(tmp).where(nogdb::GraphFilter { condition }.only("street")).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_vertex_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("locations").where(nogdb::Condition("name").eq("Pentagon")).getCursor();
        ASSERT_SIZE(res, 1);
        auto tmp = Coordinates {};
        res.first();
        res->record.get("coordinates").convertTo(tmp);
        assert(tmp.x == -1.00);
        assert(tmp.y == 1.00);
        res = txn.find("locations").where(nogdb::Condition("name").eq("Tokyo Tower")).getCursor();
        ASSERT_SIZE(res, 0);
        res = txn.find("locations").where(nogdb::Condition("temperature").eq(18)).getCursor();
        ASSERT_SIZE(res, 1);
        res.first();
        assert(res->record.get("name").toText() == "Pentagon");
        res = txn.find("locations").where(nogdb::Condition("postcode").eq(11600U)).getCursor();
        ASSERT_SIZE(res, 1);
        res.first();
        assert(res->record.get("name").toText() == "ThaiCC Tower");
        res = txn.find("locations").where(nogdb::Condition("price").eq(280000LL)).getCursor();
        ASSERT_SIZE(res, 1);
        res.first();
        assert(res->record.get("name").toText() == "Dubai Building");
        res = txn.find("locations").where(nogdb::Condition("population").eq(900ULL)).getCursor();
        ASSERT_SIZE(res, 2);
        res.next();
        assert(res->record.get("name").toText() == "ThaiCC Tower");
        res.next();
        assert(res->record.get("name").toText() == "Pentagon");
        res = txn.find("locations").where(nogdb::Condition("rating").eq(4.5)).getCursor();
        ASSERT_SIZE(res, 2);
        res.next();
        assert(res->record.get("name").toText() == "New York Tower");
        res.next();
        assert(res->record.get("name").toText() == "Empire State Building");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.find("locations").where(!nogdb::Condition("name").eq("Pentagon")).getCursor();
        ASSERT_SIZE(res, 4);
        res = txn.find("locations").where(nogdb::Condition("temperature").gt(35)).getCursor();
        ASSERT_SIZE(res, 1);
        res = txn.find("locations").where(nogdb::Condition("rating").ge(4.5)).getCursor();
        ASSERT_SIZE(res, 3);
        res = txn.find("locations").where(nogdb::Condition("postcode").lt(10300U)).getCursor();
        ASSERT_SIZE(res, 2);
        txn.find("locations").where(nogdb::Condition("population").le(900ULL)).getCursor();
        ASSERT_SIZE(res, 2);
        res = txn.find("locations").where(nogdb::Condition("price").null()).getCursor();
        ASSERT_SIZE(res, 1);
        res.last();
        assert(res->record.get("name").toText() == "ThaiCC Tower");
        res = txn.find("locations").where(nogdb::Condition("temperature")).getCursor();
        ASSERT_SIZE(res, 4);
        res = txn.find("locations").where(nogdb::Condition("name").eq(100)).getCursor();
        ASSERT_SIZE(res, 0);
        res = txn.find("locations")
                  .where(nogdb::Condition("population").eq(static_cast<unsigned long long>(2000)))
                  .getCursor();
        ASSERT_SIZE(res, 1);
        res.last();
        assert(res->record.get("name").toText() == "New York Tower");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.find("locations").where(nogdb::Condition("name").contain("tag").ignoreCase()).getCursor();
        ASSERT_SIZE(res, 1);
        res.to(0);
        assert(res->record.get("name").toText() == "Pentagon");
        res = txn.find("locations").where(!nogdb::Condition("name").contain("u").ignoreCase()).getCursor();
        ASSERT_SIZE(res, 3);
        res = txn.find("locations").where(nogdb::Condition("name").beginWith("thai").ignoreCase()).getCursor();
        ASSERT_SIZE(res, 1);
        res.to(0);
        assert(res->record.get("name").toText() == "ThaiCC Tower");
        res = txn.find("locations").where(!nogdb::Condition("name").beginWith("Thai")).getCursor();
        ASSERT_SIZE(res, 4);
        res = txn.find("locations").where(nogdb::Condition("name").endWith("TOWER").ignoreCase()).getCursor();
        ASSERT_SIZE(res, 2);
        res = txn.find("locations").where(!nogdb::Condition("name").endWith("Building")).getCursor();
        ASSERT_SIZE(res, 3);
        res = txn.find("locations").where(nogdb::Condition("name").gt("Pentagon")).getCursor();
        ASSERT_SIZE(res, 1);
        res = txn.find("locations").where(nogdb::Condition("name").ge("Pentagon")).getCursor();
        ASSERT_SIZE(res, 2);
        res = txn.find("locations").where(nogdb::Condition("name").lt("Pentagon")).getCursor();
        ASSERT_SIZE(res, 3);
        res = txn.find("locations").where(nogdb::Condition("name").le("Pentagon")).getCursor();
        ASSERT_SIZE(res, 4);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_invalid_vertex_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("location").where(nogdb::Condition("name")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("locations").where(nogdb::Condition("names")).getCursor();
        ASSERT_SIZE(res, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("locations").where(nogdb::Condition("coordinates").contain("a")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }
}

void test_find_edge_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("street").where(nogdb::Condition("name").eq("George Street")).getCursor();
        ASSERT_SIZE(res, 1);
        auto tmp = Coordinates {};
        res.first();
        res->record.get("coordinates").convertTo(tmp);
        assert(tmp.x == 0.1);
        assert(tmp.y == -0.1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_find_invalid_edge_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("streets").where(nogdb::Condition("name")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("railway").where(nogdb::Condition("names")).getCursor();
        ASSERT_SIZE(res, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("highway").where(nogdb::Condition("coordinates").contain("a")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }
}

void test_find_vertex_cursor_condition_function()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("locations")
                       .where([](const nogdb::Record& record) {
                           return record.get("name").toText().find("Building") != std::string::npos
                               || (!record.get("rating").empty() && record.get("rating").toReal() >= 4.5);
                       })
                       .getCursor();
        ASSERT_SIZE(res, 3);
        res.next();
        assert(res->record.get("name").toText() == "New York Tower");
        res.next();
        assert(res->record.get("name").toText() == "Dubai Building");
        res.next();
        assert(res->record.get("name").toText() == "Empire State Building");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto cmp = [](const nogdb::Record& record) {
            return ((!record.get("temperature").empty() && record.get("temperature").toInt() > 0)
                       && (!record.get("rating").empty() && record.get("rating").toReal() >= 3.0))
                || (!record.get("population").empty() && record.get("population").toBigIntU() <= 900ULL);
        };
        auto res = txn.find("locations").where(cmp).getCursor();
        ASSERT_SIZE(res, 4);
        res.first();
        assert(res->record.get("name").toText() == "New York Tower");
        res.to(1);
        assert(res->record.get("name").toText() == "Dubai Building");
        res.to(2);
        assert(res->record.get("name").toText() == "ThaiCC Tower");
        res.last();
        assert(res->record.get("name").toText() == "Pentagon");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_edge_cursor_condition_function()
{
    auto test_condition_function_1 = [](const nogdb::Record& record) {
        if (record.get("name").empty())
            return false;
        return (record.get("name").toText() == "George Street");
    };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("street").where(test_condition_function_1).getCursor();
        ASSERT_SIZE(res, 1);
        auto tmp = Coordinates {};
        res.next();
        res->record.get("coordinates").convertTo(tmp);
        assert(tmp.x == 0.1);
        assert(tmp.y == -0.1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_invalid_vertex_cursor_condition_function()
{
    auto condition = [](const nogdb::Record& record) {
        return record.get("name").toText().find("Building") != std::string::npos
            || record.get("rating").toReal() >= 4.5;
    };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("location").where(condition).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }
}

void test_find_invalid_edge_cursor_condition_function()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.find("streets")
                       .where([](const nogdb::Record& record) {
                           return record.get("distance").toReal() < 50.0 && record.get("capacity").toIntU() >= 300U;
                       })
                       .getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }
}

void test_find_vertex_cursor_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("name").endWith("Building").ignoreCase() or nogdb::Condition("rating").ge(4.5);
        auto res = txn.find("locations").where(expr).getCursor();
        ASSERT_SIZE(res, 3);
        res.next();
        assert(res->record.get("name").toText() == "New York Tower");
        res.next();
        assert(res->record.get("name").toText() == "Dubai Building");
        res.next();
        assert(res->record.get("name").toText() == "Empire State Building");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto expr1 = (nogdb::Condition("temperature").gt(0) and nogdb::Condition("rating").ge(3.0));
        auto expr2 = nogdb::Condition("population").le(900ULL);
        auto res = txn.find("locations").where(expr1 or expr2).getCursor();
        ASSERT_SIZE(res, 4);
        res.first();
        assert(res->record.get("name").toText() == "New York Tower");
        res.to(1);
        assert(res->record.get("name").toText() == "Dubai Building");
        res.to(2);
        assert(res->record.get("name").toText() == "ThaiCC Tower");
        res.last();
        assert(res->record.get("name").toText() == "Pentagon");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_invalid_vertex_cursor_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("name").endWith("Building").ignoreCase() or nogdb::Condition("rating").ge(4.5);
        auto res = txn.find("location").where(expr).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("names").endWith("Building").ignoreCase() or nogdb::Condition("rating").ge(4.5);
        auto res = txn.find("locations").where(expr).getCursor();
        ASSERT_SIZE(res, 3);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr
            = nogdb::Condition("name").endWith("Building").ignoreCase() or nogdb::Condition("rating").contain("a");
        auto res = txn.find("locations").where(expr).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }
}

void test_find_edge_cursor_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr1 = nogdb::Condition("distance").lt(50.0) and nogdb::Condition("capacity").ge(300U);
        auto expr2 = nogdb::Condition("distance").ge(50.0) and nogdb::Condition("temperature").gt(30);
        auto res = txn.find("street").where(expr1 or expr2).getCursor();
        ASSERT_SIZE(res, 5);
        auto elements = std::vector<std::string> { "George Street", "Isaac Street", "Andrew Street", "Fisher Avenue",
            "Jetty Road" };
        cursorTester(res, elements, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_invalid_edge_cursor_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("distance").lt(50.0) and nogdb::Condition("capacity").ge(300U);
        auto res = txn.find("streets").where(expr).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("distance").lt(50.0) and nogdb::Condition("capacityyy").ge(300U);
        auto res = txn.find("street").where(expr).getCursor();
        ASSERT_SIZE(res, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("distance").lt(50.0) and nogdb::Condition("capacity").contain("a");
        auto res = txn.find("street").where(expr).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }
}

void test_find_edge_in_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Dubai Building")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto res = txn.findInEdge(vertex)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("George Street") }.only("street"))
                       .getCursor();
        ASSERT_SIZE(res, 1);
        res = txn.findInEdge(vertex)
                  .where(nogdb::GraphFilter { nogdb::Condition("distance").gt(40.0) }.only("street"))
                  .getCursor();
        assert(res.count() == 2);
        cursorContains(res, std::set<std::string> { "Andrew Street", "George Street" }, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Pentagon")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto classNames = std::vector<std::string> { "street", "railway" };
        auto res = txn.findInEdge(vertex)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Isaac Street") }.only(classNames))
                       .getCursor();
        ASSERT_SIZE(res, 1);
        res = txn.findInEdge(vertex)
                  .where(nogdb::GraphFilter { nogdb::Condition("distance").lt(200.0) }.only(classNames))
                  .getCursor();
        assert(res.count() == 3);
        cursorContains(res, std::set<std::string> { "Dwayne Way", "Henry Road", "Isaac Street" }, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).getCursor();
        assert(vertices.size() == 1);
        vertices.first();
        auto& vertex = vertices->descriptor;
        auto res = txn.findInEdge(vertex).where(nogdb::Condition("name").eq("The Outer Ring C")).getCursor();
        ASSERT_SIZE(res, 1);
        res = txn.findInEdge(vertex).where(nogdb::Condition("distance").ge(36.2)).getCursor();
        assert(res.count() == 3);
        cursorContains(res, std::set<std::string> { "Eastern Way", "Fisher Avenue", "The Outer Ring C" }, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_edge_out_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("New York Tower")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto res = txn.findOutEdge(vertex)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("street"))
                       .getCursor();
        ASSERT_SIZE(res, 1);
        res = txn.findOutEdge(vertex)
                  .where(nogdb::GraphFilter { nogdb::Condition("distance").ge(100.0) }.only("railway"))
                  .getCursor();
        ASSERT_SIZE(res, 3);
        cursorContains(res, std::set<std::string> { "Bamboo Way", "Catalina Way", "Dwayne Way" }, "name");
        res = txn.findOutEdge(vertex)
                  .where(nogdb::GraphFilter { nogdb::Condition("temperature").le(42) }.only("railway"))
                  .getCursor();
        ASSERT_SIZE(res, 2);
        cursorContains(res, std::set<std::string> { "Andy Way", "Catalina Way" }, "name");
        res = txn.findOutEdge(vertex)
                  .where(nogdb::GraphFilter { !nogdb::Condition("temperature") }.only("railway"))
                  .getCursor();
        ASSERT_SIZE(res, 1);
        cursorContains(res, std::set<std::string> { "Dwayne Way" }, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("New York Tower")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto classNames = std::vector<std::string> { "street", "railway" };
        auto res = txn.findOutEdge(vertex)
                       .where(nogdb::GraphFilter { !nogdb::Condition("name").eq("Andrew Street") }.only(classNames))
                       .getCursor();
        ASSERT_SIZE(res, 5);
        res = txn.findOutEdge(vertex)
                  .where(nogdb::GraphFilter { !nogdb::Condition("name").contain("boo") }.only(classNames))
                  .getCursor();
        ASSERT_SIZE(res, 5);
        res = txn.findOutEdge(vertex)
                  .where(nogdb::GraphFilter { nogdb::Condition("name").contain("BOO").ignoreCase() }.only(classNames))
                  .getCursor();
        ASSERT_SIZE(res, 1);
        cursorContains(res, std::set<std::string> { "Bamboo Way" }, "name");
        res = txn.findOutEdge(vertex)
                  .where(nogdb::GraphFilter { !nogdb::Condition("name").beginWith("a").ignoreCase() }.only(classNames))
                  .getCursor();
        ASSERT_SIZE(res, 4);
        res = txn.findOutEdge(vertex)
                  .where(nogdb::GraphFilter { nogdb::Condition("name").beginWith("A") }.only(classNames))
                  .getCursor();
        ASSERT_SIZE(res, 2);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("New York Tower")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto res = txn.findOutEdge(vertex).where(nogdb::Condition("name").eq("The Outer Ring B")).getCursor();
        ASSERT_SIZE(res, 1);
        res = txn.findOutEdge(vertex).where(!nogdb::Condition("name").endWith("StrEEt").ignoreCase()).getCursor();
        ASSERT_SIZE(res, 6);
        res = txn.findOutEdge(vertex).where(nogdb::Condition("name").endWith("Way")).getCursor();
        ASSERT_SIZE(res, 4);
        cursorContains(res, std::set<std::string> { "Andy Way", "Bamboo Way", "Catalina Way", "Dwayne Way" }, "name");
        res = txn.findOutEdge(vertex).where(!nogdb::Condition("coordinates").null()).getCursor();
        ASSERT_SIZE(res, 4);
        cursorContains(
            res, std::set<std::string> { "Andrew Street", "Andy Way", "Bamboo Way", "The Outer Ring B" }, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_edge_all_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Pentagon")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto res = txn.findEdge(vertex)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("George Street") }.only("street"))
                       .getCursor();
        ASSERT_SIZE(res, 1);
        res = txn.findEdge(vertex)
                  .where(nogdb::GraphFilter {
                      nogdb::Condition("distance").ge(50.0),
                  }
                             .only("street"))
                  .getCursor();
        ASSERT_SIZE(res, 2);
        cursorContains(res, std::set<std::string> { "Henry Road", "Jetty Road" }, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto classNames = std::vector<std::string> { "street", "railway" };
        auto res = txn.findEdge(vertex)
                       .where(nogdb::GraphFilter { nogdb::Condition("distance").gt(100.0) }.only(classNames))
                       .getCursor();
        ASSERT_SIZE(res, 1);
        res.next();
        assert(res->record.get("name").toText() == "Gravity Way");
        res = txn.findEdge(vertex)
                  .where(nogdb::GraphFilter { nogdb::Condition("distance").le(100.0) }.only(classNames))
                  .getCursor();
        ASSERT_SIZE(res, 5);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto res = txn.findEdge(vertex).where(nogdb::Condition("capacity").eq(100U)).getCursor();
        ASSERT_SIZE(res, 1);
        res.first();
        assert(res->record.get("name").toText() == "Doodee Street");
        res = txn.findEdge(vertex).where(nogdb::Condition("name").contain("C")).getCursor();
        ASSERT_SIZE(res, 2);
        cursorContains(res, std::set<std::string> { "Cowboy Road", "The Outer Ring C" }, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_invalid_edge_in_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& vertex = vertices[0];
    try {
        auto edges = txn.findInEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("streets"))
                         .getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findInEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only(classNames))
                         .getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges = txn.findInEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("names").eq("Andrew Street") }.only("street"))
                         .getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges = txn.findInEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("distance").contain("a") }.only("street"))
                         .getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        txn.findInEdge(edge.descriptor)
            .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("street"))
            .getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto res = txn.findInEdge(tmp)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("street"))
                       .getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_invalid_edge_out_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& vertex = vertices[0];
    try {
        auto edges = txn.findOutEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("streets"))
                         .getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findOutEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only(classNames))
                         .getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges = txn.findOutEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("names").eq("Andrew Street") }.only("street"))
                         .getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges = txn.findOutEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("distance").contain("a") }.only("street"))
                         .getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto edges = txn.findOutEdge(edge.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("street"))
                         .getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto res = txn.findOutEdge(tmp)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("street"))
                       .getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_invalid_edge_all_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& vertex = vertices[0];
    try {
        auto edges = txn.findEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("streets"))
                         .getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only(classNames))
                         .getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges = txn.findEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("names").eq("Andrew Street") }.only("street"))
                         .getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges = txn.findEdge(vertex.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("distance").contain("a") }.only("street"))
                         .getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto edges = txn.findEdge(edge.descriptor)
                         .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("street"))
                         .getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto res = txn.findEdge(tmp)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("street"))
                       .getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_edge_in_cursor_condition_function()
{
    auto test_condition_function_4 = [](const nogdb::Record& record) {
        if (record.get("distance").empty())
            return false;
        return (record.get("distance").toReal() > 40.0);
    };

    auto test_condition_function_5 = [](const nogdb::Record& record) {
        if (record.get("distance").empty())
            return false;
        return (record.get("distance").toReal() < 200.0);
    };

    auto test_condition_function_6 = [](const nogdb::Record& record) {
        if (record.get("distance").empty())
            return false;
        return (record.get("distance").toReal() >= 36.2);
    };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Dubai Building")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto res = txn.findInEdge(vertex)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("George Street") }.only("street"))
                       .getCursor();
        ASSERT_SIZE(res, 1);
        res = txn.findInEdge(vertex).where(nogdb::GraphFilter { test_condition_function_4 }.only("street")).getCursor();
        ASSERT_SIZE(res, 2);
        cursorContains(res, std::set<std::string> { "Andrew Street", "George Street" }, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Pentagon")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto classNames = std::vector<std::string> { "street", "railway" };
        auto res = txn.findInEdge(vertex)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Isaac Street") }.only(classNames))
                       .getCursor();
        ASSERT_SIZE(res, 1);
        res = txn.findInEdge(vertex)
                  .where(nogdb::GraphFilter { test_condition_function_5 }.only(classNames))
                  .getCursor();
        ASSERT_SIZE(res, 3);
        cursorContains(res, std::set<std::string> { "Dwayne Way", "Henry Road", "Isaac Street" }, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto res = txn.findInEdge(vertex).where(nogdb::Condition("name").eq("The Outer Ring C")).getCursor();
        ASSERT_SIZE(res, 1);
        res = txn.findInEdge(vertex).where(nogdb::GraphFilter { test_condition_function_6 }).getCursor();
        ASSERT_SIZE(res, 3);
        cursorContains(res, std::set<std::string> { "Eastern Way", "Fisher Avenue", "The Outer Ring C" }, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_edge_out_cursor_condition_function()
{
    auto test_condition_function_7 = [](const nogdb::Record& record) {
        if (record.get("distance").empty())
            return false;
        return (record.get("distance").toReal() >= 100.0);
    };

    auto test_condition_function_8 = [](const nogdb::Record& record) {
        auto tmp = record.get("temperature");
        if (tmp.empty())
            return false;
        return (tmp.toInt() <= 42);
    };

    auto test_condition_function_9 = [](const nogdb::Record& record) { return record.get("temperature").empty(); };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("New York Tower")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto res = txn.findOutEdge(vertex)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("Andrew Street") }.only("street"))
                       .getCursor();
        ASSERT_SIZE(res, 1);
        res = txn.findOutEdge(vertex)
                  .where(nogdb::GraphFilter { test_condition_function_7 }.only("railway"))
                  .getCursor();
        ASSERT_SIZE(res, 3);
        cursorContains(res, std::set<std::string> { "Bamboo Way", "Catalina Way", "Dwayne Way" }, "name");
        res = txn.findOutEdge(vertex)
                  .where(nogdb::GraphFilter { test_condition_function_8 }.only("railway"))
                  .getCursor();
        ASSERT_SIZE(res, 2);
        cursorContains(res, std::set<std::string> { "Andy Way", "Catalina Way" }, "name");
        res = txn.findOutEdge(vertex)
                  .where(nogdb::GraphFilter { test_condition_function_9 }.only("railway"))
                  .getCursor();
        ASSERT_SIZE(res, 1);
        res.first();
        assert(res->record.get("name").toText() == "Dwayne Way");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_find_edge_all_cursor_condition_function()
{
    auto test_condition_function_10 = [](const nogdb::Record& record) {
        if (record.get("distance").empty())
            return false;
        return (record.get("distance").toReal() > 100);
    };

    auto test_condition_function_11 = [](const nogdb::Record& record) {
        if (record.get("distance").empty())
            return false;
        return (record.get("distance").toReal() <= 100);
    };

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Pentagon")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto res = txn.findEdge(vertex)
                       .where(nogdb::GraphFilter { nogdb::Condition("name").eq("George Street") }.only("street"))
                       .getCursor();
        ASSERT_SIZE(res, 1);
        res = txn.findEdge(vertex)
                  .where(nogdb::GraphFilter { nogdb::Condition("distance").ge(50.0) }.only("street"))
                  .getCursor();
        ASSERT_SIZE(res, 2);
        cursorContains(res, std::set<std::string> { "Henry Road", "Jetty Road" }, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto classNames = std::vector<std::string> { "street", "railway" };
        auto res = txn.findEdge(vertex)
                       .where(nogdb::GraphFilter { test_condition_function_10 }.only(classNames))
                       .getCursor();
        ASSERT_SIZE(res, 1);
        res.next();
        assert(res->record.get("name").toText() == "Gravity Way");
        res = txn.findEdge(vertex)
                  .where(nogdb::GraphFilter { test_condition_function_11 }.only(classNames))
                  .getCursor();
        ASSERT_SIZE(res, 5);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_invalid_edge_in_cursor_condition_function()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    auto& vertex = vertices[0];
    auto condition = [](const nogdb::Record& record) {
        return (record.get("name").toText().find("Street") != std::string::npos) || !record.get("distance").empty();
    };

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges
            = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { condition }.only("streets")).getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges
            = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { condition }.only(classNames)).getCursor();
        ASSERT_SIZE(edges, 3);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto edges = txn.findInEdge(edge.descriptor).where(nogdb::GraphFilter { condition }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto res = txn.findInEdge(tmp).where(nogdb::GraphFilter { condition }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_invalid_edge_out_cursor_condition_function()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    auto& vertex = vertices[0];
    auto condition = [](const nogdb::Record& record) {
        return (record.get("name").toText().find("Street") != std::string::npos) || !record.get("distance").empty();
    };

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges
            = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { condition }.only("streets")).getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges
            = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { condition }.only(classNames)).getCursor();
        ASSERT_SIZE(edges, 3);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto edges
            = txn.findOutEdge(edge.descriptor).where(nogdb::GraphFilter { condition }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto res = txn.findOutEdge(tmp).where(nogdb::GraphFilter { condition }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_invalid_edge_all_cursor_condition_function()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    auto& vertex = vertices[0];
    auto condition = [](const nogdb::Record& record) {
        return (record.get("name").toText().find("Street") != std::string::npos) || !record.get("distance").empty();
    };
    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto edges
            = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { condition }.only("streets")).getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges
            = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { condition }.only(classNames)).getCursor();
        ASSERT_SIZE(edges, 6);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto edges = txn.findEdge(edge.descriptor).where(nogdb::GraphFilter { condition }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto res = txn.findEdge(tmp).where(nogdb::GraphFilter { condition }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_edge_in_cursor_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Dubai Building")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto expr = nogdb::Condition("distance").ge(80.0) or nogdb::Condition("capacity").gt(400U)
            or nogdb::Condition("temperature").lt(30);
        auto res = txn.findInEdge(vertex).where(expr).getCursor();
        ASSERT_SIZE(res, 3);
        cursorContains(res, std::set<std::string> { "Andrew Street", "Andy Way", "George Street" }, "name");
        res = txn.findInEdge(vertex).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        ASSERT_SIZE(res, 2);
        cursorContains(res, std::set<std::string> { "Andrew Street", "George Street" }, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_edge_out_cursor_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("New York Tower")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto expr = nogdb::Condition("name").contain("Road").ignoreCase()
            or (nogdb::Condition("temperature").null() and nogdb::Condition("capacity").ge(2000U))
            or (nogdb::Condition("temperature").gt(40) and nogdb::Condition("distance").lt(140.0));
        auto res = txn.findOutEdge(vertex).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        ASSERT_SIZE(res, 1);
        res.next();
        assert(res->record.get("name").toText() == "Henry Road");

        res = txn.findOutEdge(vertex).where(nogdb::GraphFilter { expr }.only("street", "highway")).getCursor();
        ASSERT_SIZE(res, 2);
        cursorContains(res, std::set<std::string> { "Henry Road", "The Outer Ring B" }, "name");

        res = txn.findOutEdge(vertex).where(expr).getCursor();
        ASSERT_SIZE(res, 4);
        cursorContains(
            res, std::set<std::string> { "Andy Way", "Bamboo Way", "Henry Road", "The Outer Ring B" }, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_edge_all_cursor_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Pentagon")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto expr = nogdb::Condition("temperature") and nogdb::Condition("capacity")
            and nogdb::Condition("distance").gt(40.0);
        auto res = txn.findEdge(vertex).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        ASSERT_SIZE(res, 2);
        cursorContains(res, std::set<std::string> { "George Street", "Jetty Road" }, "name");

        res = txn.findEdge(vertex).where(expr).getCursor();
        ASSERT_SIZE(res, 3);
        cursorContains(res, std::set<std::string> { "George Street", "Jetty Road", "The Outer Ring C" }, "name");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto vertices = txn.find("locations").where(nogdb::Condition("name").eq("Pentagon")).getCursor();
        assert(vertices.size() == 1);
        vertices.next();
        auto& vertex = vertices->descriptor;
        auto cond1
            = nogdb::Condition("@className").eq("street") and nogdb::Condition("name").contain("street").ignoreCase();
        auto cond2 = nogdb::Condition("@className").eq("highway") and nogdb::Condition("name").endWith("C");
        auto res = txn.findEdge(vertex).where(cond1 or cond2).getCursor();
        ASSERT_SIZE(res, 3);
        while (res.next()) {
            assert(res->record.getText("name") == "The Outer Ring C" || res->record.getText("name") == "Isaac Street"
                || res->record.getText("name") == "George Street");
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_find_invalid_edge_in_cursor_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& vertex = vertices[0];
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("streets")).getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only(classNames)).getCursor();
        ASSERT_SIZE(edges, 3);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("names").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        ASSERT_SIZE(edges, 2);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("distance").contain("a");
        auto edges = txn.findInEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findInEdge(edge.descriptor).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto res = txn.findInEdge(tmp).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_invalid_edge_out_cursor_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& vertex = vertices[0];
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("streets")).getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only(classNames)).getCursor();
        ASSERT_SIZE(edges, 3);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("names").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        ASSERT_SIZE(edges, 2);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("distance").contain("a");
        auto edges = txn.findOutEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findOutEdge(edge.descriptor).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto res = txn.findOutEdge(tmp).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_find_invalid_edge_all_cursor_with_expression()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto vertices = nogdb::ResultSet {};
    auto edges = nogdb::ResultSet {};
    try {
        vertices = txn.find("locations").where(nogdb::Condition("name").eq("ThaiCC Tower")).get();
        edges = txn.find("street").where(nogdb::Condition("name").eq("Andrew Street")).get();
        assert(vertices.size() == 1);
        assert(edges.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& vertex = vertices[0];
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("streets")).getCursor();
        ASSERT_SIZE(edges, 0);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto classNames = std::vector<std::string> { "street", "railway", "subway" };
        auto edges = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only(classNames)).getCursor();
        ASSERT_SIZE(edges, 6);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("names").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        ASSERT_SIZE(edges, 4);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto expr = nogdb::Condition("distance").contain("a");
        auto edges = txn.findEdge(vertex.descriptor).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto& edge = edges[0];
    try {
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto edges = txn.findEdge(edge.descriptor).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = vertex.descriptor;
        tmp.rid.second = -1;
        auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
        auto res = txn.findEdge(tmp).where(nogdb::GraphFilter { expr }.only("street")).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}
