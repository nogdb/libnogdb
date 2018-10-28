/*
 *  Copyright (C) 2018, Throughwave (Thailand) Co., Ltd.
 *  <peerawich at throughwave dot co dot th>
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

#include <algorithm>
#include <set>
#include "functest.h"
#include "test_prepare.h"

struct Coordinates {
  Coordinates() {};

  Coordinates(double x_, double y_) : x{x_}, y{y_} {}

  double x{0.0};
  double y{0.0};
};

void init_test_find() {
  init_vertex_mountain();
  init_vertex_location();
  init_edge_street();
  init_edge_highway();
  init_edge_railway();
}

void destroy_test_find() {
  destroy_edge_railway();
  destroy_edge_highway();
  destroy_edge_street();
  destroy_vertex_location();
  destroy_vertex_mountain();
}

void test_create_informative_graph() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Vertex::create(txn, "mountains", nogdb::Record{}
        .set("name", "Fuji")
        .set("temperature", -10)
        .set("height", 3000U)
        .set("rating", 5.0)
    );
    nogdb::Vertex::create(txn, "mountains", nogdb::Record{}
        .set("name", "Blue Mountain")
        .set("temperature", 5)
        .set("rating", 4.0)
    );
    auto place1 = nogdb::Vertex::create(txn, "locations", nogdb::Record{}
        .set("name", "New York Tower")
        .set("temperature", 11)
        .set("postcode", 10200U)
        .set("price", 200000LL)
        .set("population", 2000ULL)
        .set("rating", 4.5)
        .set("coordinates", nogdb::Bytes{Coordinates{42.42, 24.24}})
    );

    auto place2 = nogdb::Vertex::create(txn, "locations", nogdb::Record{}
        .set("name", "Dubai Building")
        .set("temperature", 37)
        .set("price", 280000LL)
        .set("population", 1800ULL)
        .set("rating", 5.0)
        .set("coordinates", nogdb::Bytes{Coordinates{112.89, -321.45}})
    );

    auto place3 = nogdb::Vertex::create(txn, "locations", nogdb::Record{}
        .set("name", "Empire State Building")
        .set("postcode", 10250U)
        .set("price", 220000LL)
        .set("population", 2400ULL)
        .set("rating", 4.5)
        .set("coordinates", nogdb::Bytes{Coordinates{242.42, -424.24}})
    );

    auto place4 = nogdb::Vertex::create(txn, "locations", nogdb::Record{}
        .set("name", "ThaiCC Tower")
        .set("temperature", 28)
        .set("postcode", 11600U)
        .set("population", 900ULL)
        .set("rating", 3.5)
        .set("coordinates", nogdb::Bytes{Coordinates{-56.4242, 236.098}})
    );

    auto place5 = nogdb::Vertex::create(txn, "locations", nogdb::Record{}
        .set("name", "Pentagon")
        .set("temperature", 18)
        .set("postcode", 10475U)
        .set("price", 300000LL)
        .set("population", 900ULL)
        .set("coordinates", nogdb::Bytes{Coordinates{-1.00, 1.00}})
    );

    nogdb::Edge::create(txn, "street", place5, place2,
                        nogdb::Record{}.set("name", "George Street")
                            .set("temperature", 20)
                            .set("capacity", 300U)
                            .set("distance", 40.5)
                            .set("coordinates", nogdb::Bytes{Coordinates{0.1, -0.1}}));

    nogdb::Edge::create(txn, "street", place3, place1,
                        nogdb::Record{}.set("name", "Boyd Street")
                            .set("capacity", 230U)
                            .set("distance", 15.0)
                            .set("coordinates", nogdb::Bytes{Coordinates{-9.335, 19.028}}));

    nogdb::Edge::create(txn, "street", place1, place5,
                        nogdb::Record{}.set("name", "Henry Road")
                            .set("capacity", 1000U)
                            .set("distance", 50.45));

    nogdb::Edge::create(txn, "street", place4, place5,
                        nogdb::Record{}.set("name", "Isaac Street")
                            .set("capacity", 400U)
                            .set("distance", 33.42));

    nogdb::Edge::create(txn, "street", place4, place2,
                        nogdb::Record{}.set("name", "Cowboy Road")
                            .set("capacity", 120U)
                            .set("distance", 12.55)
                            .set("coordinates", nogdb::Bytes{Coordinates{-334.51, 70.21}}));

    nogdb::Edge::create(txn, "street", place1, place2,
                        nogdb::Record{}.set("name", "Andrew Street")
                            .set("temperature", 28)
                            .set("capacity", 420U)
                            .set("distance", 42.42)
                            .set("coordinates", nogdb::Bytes{Coordinates{-90.143, -172.68}}));

    nogdb::Edge::create(txn, "street", place2, place1,
                        nogdb::Record{}.set("name", "Eddy Avenue")
                            .set("capacity", 780U)
                            .set("distance", 56.5)
                            .set("coordinates", nogdb::Bytes{Coordinates{0.00, 45.00}}));

    nogdb::Edge::create(txn, "street", place2, place4,
                        nogdb::Record{}.set("name", "Fisher Avenue")
                            .set("capacity", 600U)
                            .set("distance", 36.20));

    nogdb::Edge::create(txn, "street", place5, place3,
                        nogdb::Record{}.set("name", "Jetty Road")
                            .set("temperature", 32)
                            .set("capacity", 530U)
                            .set("distance", 70.5));

    nogdb::Edge::create(txn, "street", place3, place4,
                        nogdb::Record{}.set("name", "Doodee Street")
                            .set("temperature", 40)
                            .set("capacity", 100U)
                            .set("distance", 8.42)
                            .set("coordinates", nogdb::Bytes{Coordinates{-987.65, -65.789}}));

    nogdb::Edge::create(txn, "highway", place4, place1,
                        nogdb::Record{}.set("name", "The Outer Ring A")
                            .set("temperature", 36)
                            .set("capacity", 3000U)
                            .set("distance", 2200.45)
                            .set("coordinates", nogdb::Bytes{Coordinates{891.35, -301.393}}));

    nogdb::Edge::create(txn, "highway", place1, place5,
                        nogdb::Record{}.set("name", "The Outer Ring B")
                            .set("capacity", 3300U)
                            .set("distance", 2400.8)
                            .set("coordinates", nogdb::Bytes{Coordinates{-141.28, -3.942}}));

    nogdb::Edge::create(txn, "highway", place5, place4,
                        nogdb::Record{}.set("name", "The Outer Ring C")
                            .set("temperature", 32)
                            .set("capacity", 3800U)
                            .set("distance", 2980.75));

    nogdb::Edge::create(txn, "railway", place1, place2,
                        nogdb::Record{}.set("name", "Andy Way")
                            .set("temperature", 42)
                            .set("distance", 80.5)
                            .set("coordinates", nogdb::Bytes{Coordinates{84.15, -6.42}}));

    nogdb::Edge::create(txn, "railway", place1, place3,
                        nogdb::Record{}.set("name", "Bamboo Way")
                            .set("temperature", 43)
                            .set("distance", 120.25)
                            .set("coordinates", nogdb::Bytes{Coordinates{-44.67, -16.24}}));

    nogdb::Edge::create(txn, "railway", place1, place3,
                        nogdb::Record{}.set("name", "Catalina Way")
                            .set("temperature", 37)
                            .set("distance", 112.44));

    nogdb::Edge::create(txn, "railway", place1, place5,
                        nogdb::Record{}.set("name", "Dwayne Way")
                            .set("distance", 150.75));

    nogdb::Edge::create(txn, "railway", place2, place4,
                        nogdb::Record{}.set("name", "Eastern Way")
                            .set("temperature", 48)
                            .set("distance", 78.5)
                            .set("coordinates", nogdb::Bytes{Coordinates{48.92, -115.222}}));

    nogdb::Edge::create(txn, "railway", place4, place5,
                        nogdb::Record{}.set("name", "Gravity Way")
                            .set("distance", 254.35));

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_vertex() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("Pentagon"));
    assertSize(res, 1);
    auto tmp = Coordinates{};
    res[0].record.get("coordinates").convertTo(tmp);
    assert(tmp.x == -1.00);
    assert(tmp.y == 1.00);
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("Tokyo Tower"));
    assertSize(res, 0);
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("temperature").eq(18));
    assertSize(res, 1);
    assert(res[0].record.get("name").toText() == "Pentagon");
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("postcode").eq(11600U));
    assertSize(res, 1);
    assert(res[0].record.get("name").toText() == "ThaiCC Tower");
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("price").eq(280000LL));
    assertSize(res, 1);
    assert(res[0].record.get("name").toText() == "Dubai Building");
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("population").eq(900ULL));
    assertSize(res, 2);
    assert(res[0].record.get("name").toText() == "ThaiCC Tower");
    assert(res[1].record.get("name").toText() == "Pentagon");
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("rating").eq(4.5));
    assertSize(res, 2);
    assert(res[0].record.get("name").toText() == "New York Tower");
    assert(res[1].record.get("name").toText() == "Empire State Building");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto res = nogdb::Vertex::get(txn, "locations", !nogdb::Condition("name").eq("Pentagon"));
    assertSize(res, 4);
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("temperature").gt(35));
    assertSize(res, 1);
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("rating").ge(4.5));
    assertSize(res, 3);
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("postcode").lt(10300U));
    assertSize(res, 2);
    nogdb::Vertex::get(txn, "locations", nogdb::Condition("population").le(900ULL));
    assertSize(res, 2);
    res = nogdb::Vertex::get(txn, "locations", !nogdb::Condition("price"));
    assertSize(res, 1);
    assert(res[0].record.get("name").toText() == "ThaiCC Tower");
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("temperature"));
    assertSize(res, 4);
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq(100));
    assertSize(res, 0);
    res = nogdb::Vertex::get(txn, "locations",
                             nogdb::Condition("population").eq(static_cast<unsigned long long>(2000)));
    assertSize(res, 1);
    assert(res[0].record.get("name").toText() == "New York Tower");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").contain("tag").ignoreCase());
    assertSize(res, 1);
    assert(res[0].record.get("name").toText() == "Pentagon");
    res = nogdb::Vertex::get(txn, "locations", !nogdb::Condition("name").contain("u").ignoreCase());
    assertSize(res, 3);
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").beginWith("thai").ignoreCase());
    assertSize(res, 1);
    assert(res[0].record.get("name").toText() == "ThaiCC Tower");
    res = nogdb::Vertex::get(txn, "locations", !nogdb::Condition("name").beginWith("Thai"));
    assertSize(res, 4);
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").endWith("TOWER").ignoreCase());
    assertSize(res, 2);
    res = nogdb::Vertex::get(txn, "locations", !nogdb::Condition("name").endWith("Building"));
    assertSize(res, 3);
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").gt("Pentagon"));
    assertSize(res, 1);
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").ge("Pentagon"));
    assertSize(res, 2);
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").lt("Pentagon"));
    assertSize(res, 3);
    res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").le("Pentagon"));
    assertSize(res, 4);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();

}

void test_find_invalid_vertex() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::get(txn, "location", nogdb::Condition("name"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("names"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::get(txn, "locations", nogdb::Condition("coordinates").contain("invalid"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

}

void test_find_edge() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("George Street"));
    assertSize(res, 1);
    auto tmp = Coordinates{};
    res[0].record.get("coordinates").convertTo(tmp);
    assert(tmp.x == 0.1);
    assert(tmp.y == -0.1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();

}

void test_find_invalid_edge() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::get(txn, "streets", nogdb::Condition("name"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::get(txn, "railway", nogdb::Condition("names"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::get(txn, "highway", nogdb::Condition("coordinates").contain("invalid"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }
}

void test_find_edge_in() {
  auto cmp = [](const nogdb::Result &name1, const nogdb::Result &name2) {
    return name1.record.get("name").toText() < name2.record.get("name").toText();
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("Dubai Building"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto condition1 = nogdb::Condition("name").eq("George Street");
    auto filter1 = nogdb::GraphFilter(condition1).only("street");
    auto res = nogdb::Vertex::getInEdge(txn, vertex.descriptor, filter1);
    assertSize(res, 1);
    auto condition2 = nogdb::Condition("distance").gt(40.0);
    auto filter2 = nogdb::GraphFilter(condition2).only("street");
    res = nogdb::Vertex::getInEdge(txn, vertex.descriptor, filter2);
    assertSize(res, 2);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Andrew Street");
    assert(res[1].record.get("name").toText() == "George Street");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("Pentagon"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto condition1 = nogdb::Condition("name").eq("Isaac Street");
    auto filter1 = nogdb::GraphFilter(condition1).only("street", "railway");
    auto res = nogdb::Vertex::getInEdge(txn, vertex.descriptor, filter1);
    assertSize(res, 1);
    auto condition2 = nogdb::Condition("distance").lt(200.0);
    auto filter2 = nogdb::GraphFilter(condition2).only("street", "railway");
    res = nogdb::Vertex::getInEdge(txn, vertex.descriptor, filter2);
    assertSize(res, 3);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Dwayne Way");
    assert(res[1].record.get("name").toText() == "Henry Road");
    assert(res[2].record.get("name").toText() == "Isaac Street");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto condition1 = nogdb::Condition("name").eq("The Outer Ring C");
    auto filter1 = nogdb::GraphFilter(condition1);
    auto res = nogdb::Vertex::getInEdge(txn, vertex.descriptor, filter1);
    assertSize(res, 1);
    auto condition2 = nogdb::Condition("distance").ge(36.2);
    auto filter2 = nogdb::GraphFilter(condition2);
    res = nogdb::Vertex::getInEdge(txn, vertex.descriptor, filter2);
    assertSize(res, 3);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Eastern Way");
    assert(res[1].record.get("name").toText() == "Fisher Avenue");
    assert(res[2].record.get("name").toText() == "The Outer Ring C");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_edge_out() {
  auto cmp = [](const nogdb::Result &name1, const nogdb::Result &name2) {
    return name1.record.get("name").toText() < name2.record.get("name").toText();
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("New York Tower"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto condition1 = nogdb::Condition("name").eq("Andrew Street");
    auto filter1 = nogdb::GraphFilter(condition1).only("street");
    auto res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, filter1);
    assertSize(res, 1);
    auto condition2 = nogdb::Condition("distance").ge(100.0);
    auto filter2 = nogdb::GraphFilter(condition2).only("railway");
    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, filter2);
    assertSize(res, 3);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Bamboo Way");
    assert(res[1].record.get("name").toText() == "Catalina Way");
    assert(res[2].record.get("name").toText() == "Dwayne Way");
    auto condition3 = nogdb::Condition("temperature").le(42);
    auto filter3 = nogdb::GraphFilter(condition3).only("railway");
    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, filter3);
    assertSize(res, 2);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Andy Way");
    assert(res[1].record.get("name").toText() == "Catalina Way");
    auto condition4 = !nogdb::Condition("temperature");
    auto filter4 = nogdb::GraphFilter(condition4).only("railway");
    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, filter4);
    assertSize(res, 1);
    assert(res[0].record.get("name").toText() == "Dwayne Way");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("New York Tower"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto condition1 = !nogdb::Condition("name").eq("Andrew Street");
    auto filter1 = nogdb::GraphFilter(condition1).only("street", "railway");
    auto res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, filter1);
    assertSize(res, 5);
    auto condition2 = !nogdb::Condition("name").contain("boo");
    auto filter2 = nogdb::GraphFilter(condition2).only("street", "railway");
    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, filter2);
    assertSize(res, 5);
    auto condition3 = nogdb::Condition("name").contain("BOO").ignoreCase();
    auto filter3 = nogdb::GraphFilter(condition3).only("street", "railway");
    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, filter3);
    assertSize(res, 1);
    assert(res[0].record.get("name").toText() == "Bamboo Way");
    auto condition4 = !nogdb::Condition("name").beginWith("a").ignoreCase();
    auto filter4 = nogdb::GraphFilter(condition4).only("street", "railway");
    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, filter4);
    assertSize(res, 4);
    auto condition5 = nogdb::Condition("name").beginWith("A");
    auto filter5 = nogdb::GraphFilter(condition5).only("street", "railway");
    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, filter5);
    assertSize(res, 2);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("New York Tower"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto condition1 = nogdb::Condition("name").eq("The Outer Ring B");
    auto filter1 = nogdb::GraphFilter(condition1);
    auto res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, filter1);
    assertSize(res, 1);
    auto condition2 = !nogdb::Condition("name").endWith("StrEEt").ignoreCase();
    auto filter2 = nogdb::GraphFilter(condition2);
    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, filter2);
    assertSize(res, 6);
    auto condition3 = nogdb::Condition("name").endWith("Way");
    auto filter3 = nogdb::GraphFilter(condition3);
    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, filter3);
    assertSize(res, 4);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Andy Way");
    assert(res[1].record.get("name").toText() == "Bamboo Way");
    assert(res[2].record.get("name").toText() == "Catalina Way");
    assert(res[3].record.get("name").toText() == "Dwayne Way");
    auto condition4 = !nogdb::Condition("coordinates").null();
    auto filter4 = nogdb::GraphFilter(condition4);
    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, filter4);
    assertSize(res, 4);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Andrew Street");
    assert(res[1].record.get("name").toText() == "Andy Way");
    assert(res[2].record.get("name").toText() == "Bamboo Way");
    assert(res[3].record.get("name").toText() == "The Outer Ring B");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_edge_all() {
  auto cmp = [](const nogdb::Result &name1, const nogdb::Result &name2) {
    return name1.record.get("name").toText() < name2.record.get("name").toText();
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("Pentagon"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto cond = nogdb::Condition("name").eq("George Street");
    auto res = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{cond}.only("street"));
    assertSize(res, 1);
    cond = nogdb::Condition("distance").ge(50.0);
    res = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{cond}.only("street"));
    assertSize(res, 2);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Henry Road");
    assert(res[1].record.get("name").toText() == "Jetty Road");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto classNames = std::vector<std::string>{"street", "railway"};
    auto cond = nogdb::Condition("distance").gt(100.0);
    auto res = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{cond}.only(classNames));
    assertSize(res, 1);
    assert(res[0].record.get("name").toText() == "Gravity Way");
    cond = nogdb::Condition("distance").le(100.0);
    res = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{cond}.only(classNames));
    assertSize(res, 5);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto cond = nogdb::Condition("capacity").eq(100U);
    auto res = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{cond});
    assertSize(res, 1);
    assert(res[0].record.get("name").toText() == "Doodee Street");
    cond = nogdb::Condition("name").contain("C");
    res = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{cond});
    assertSize(res, 2);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Cowboy Road");
    assert(res[1].record.get("name").toText() == "The Outer Ring C");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_invalid_edge_in() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &vertex = vertices[0];
  try {
    auto cond = nogdb::Condition("name").eq("Andrew Street");
    nogdb::Vertex::getInEdge(txn, vertex.descriptor, nogdb::GraphFilter{cond}.only("streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto cond = nogdb::Condition("name").eq("Andrew Street");
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    nogdb::Vertex::getInEdge(txn, vertex.descriptor, nogdb::GraphFilter{cond}.only(classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto cond = nogdb::Condition("names").eq("Andrew Street");
    auto edges = nogdb::Vertex::getInEdge(txn, vertex.descriptor, nogdb::GraphFilter{cond}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto cond = nogdb::Condition("coordinates").contain("a");
    auto edges = nogdb::Vertex::getInEdge(txn, vertex.descriptor, nogdb::GraphFilter{cond}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto cond = nogdb::Condition("name").eq("Andrew Street");
    nogdb::Vertex::getInEdge(txn, edge.descriptor, nogdb::GraphFilter{cond}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto cond = nogdb::Condition("name").eq("Andrew Street");
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    nogdb::Vertex::getInEdge(txn, tmp, nogdb::GraphFilter{cond}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_invalid_edge_out() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &vertex = vertices[0];
  try {
    auto edges = nogdb::Vertex::getOutEdge(txn, vertex.descriptor,
                                           nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                               "streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getOutEdge(txn, vertex.descriptor,
                                           nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                               classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getOutEdge(txn, vertex.descriptor,
                                           nogdb::GraphFilter{nogdb::Condition("names").eq("Andrew Street")}.only(
                                               "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getOutEdge(txn, vertex.descriptor,
                                           nogdb::GraphFilter{nogdb::Condition("coordinates").contain("a")}.only(
                                               "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto edges = nogdb::Vertex::getOutEdge(txn, edge.descriptor,
                                           nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                               "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto res = nogdb::Vertex::getOutEdge(txn, tmp,
                                         nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                             "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_invalid_edge_all() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &vertex = vertices[0];
  try {
    auto edges = nogdb::Vertex::getAllEdge(txn, vertex.descriptor,
                                           nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                               "streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getAllEdge(txn, vertex.descriptor,
                                           nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                               classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getAllEdge(txn, vertex.descriptor,
                                           nogdb::GraphFilter{nogdb::Condition("names").eq("Andrew Street")}.only(
                                               "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getAllEdge(txn, vertex.descriptor,
                                           nogdb::GraphFilter{nogdb::Condition("coordinates").contain("a")}.only(
                                               "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto edges = nogdb::Vertex::getAllEdge(txn, edge.descriptor,
                                           nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                               "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto res = nogdb::Vertex::getAllEdge(txn, tmp,
                                         nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                             "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_vertex_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("name").endWith("Building").ignoreCase()
                or nogdb::Condition("rating").ge(4.5);
    auto res = nogdb::Vertex::get(txn, "locations", expr);
    assertSize(res, 3);
    assert(res[0].record.get("name").toText() == "New York Tower");
    assert(res[1].record.get("name").toText() == "Dubai Building");
    assert(res[2].record.get("name").toText() == "Empire State Building");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto expr1 = (nogdb::Condition("temperature").gt(0)
                  and nogdb::Condition("rating").ge(3.0));
    auto expr2 = nogdb::Condition("population").le(900ULL);
    auto res = nogdb::Vertex::get(txn, "mountains", expr1);
    auto res2 = nogdb::Vertex::get(txn, "locations", expr1 or expr2);
    res.insert(res.end(), res2.cbegin(), res2.cend());
    assertSize(res, 5);
    assert(res[0].record.get("name").toText() == "Blue Mountain");
    assert(res[1].record.get("name").toText() == "New York Tower");
    assert(res[2].record.get("name").toText() == "Dubai Building");
    assert(res[3].record.get("name").toText() == "ThaiCC Tower");
    assert(res[4].record.get("name").toText() == "Pentagon");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto cond1 = nogdb::Condition("@className").eq("locations") and nogdb::Condition("temperature").lt(12);
    auto cond2 = nogdb::Condition("@className").eq("mountains") and nogdb::Condition("temperature").gt(0);
    auto res = nogdb::Vertex::get(txn, "locations", cond1 or cond2);
    auto res2 = nogdb::Vertex::get(txn, "mountains", cond1 or cond2);
    res.insert(res.end(), res2.cbegin(), res2.cend());
    assertSize(res, 2);
    for (const auto &r: res) {
      assert(r.record.get("name").toText() == "New York Tower" or
             r.record.get("name").toText() == "Blue Mountain");
    }
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_invalid_vertex_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("name").endWith("Building").ignoreCase()
                or nogdb::Condition("rating").ge(4.5);
    auto res = nogdb::Vertex::get(txn, "location", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("names").endWith("Building").ignoreCase()
                or nogdb::Condition("rating").ge(4.5);
    auto res = nogdb::Vertex::get(txn, "locations", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("name").endWith("Building").ignoreCase()
                or nogdb::Condition("rating").contain("a");
    auto res = nogdb::Vertex::get(txn, "locations", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("name").endWith("Building").ignoreCase()
                or nogdb::Condition("rating").ge(4.5);
    auto res = nogdb::Vertex::get(txn, "street", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }
}

void test_find_edge_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr1 = nogdb::Condition("distance").lt(50.0)
                 and nogdb::Condition("capacity").ge(300U);
    auto expr2 = nogdb::Condition("distance").ge(50.0)
                 and nogdb::Condition("temperature").gt(30);
    auto classNames = std::set<std::string>{"street", "highway"};
    auto res = nogdb::ResultSet{};
    for (const auto &className: classNames) {
      auto tmp = nogdb::Edge::get(txn, className, expr1 or expr2);
      res.insert(res.end(), tmp.cbegin(), tmp.cend());
    }
    auto tmp = nogdb::Edge::get(txn, "railway", expr2);
    res.insert(res.end(), tmp.cbegin(), tmp.cend());
    assertSize(res, 11);
    auto elements = std::vector<std::string>{"George Street",
                                             "Isaac Street",
                                             "Andrew Street",
                                             "Fisher Avenue",
                                             "Jetty Road",
                                             "The Outer Ring A",
                                             "The Outer Ring C",
                                             "Andy Way",
                                             "Bamboo Way",
                                             "Catalina Way",
                                             "Eastern Way"};
    assert(compareText(res, "name", elements));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto cond1 = nogdb::Condition("@className").eq("highway") and nogdb::Condition("name").endWith("B");
    auto cond2 = nogdb::Condition("@className").eq("railway") and nogdb::Condition("name").beginWith("C");
    auto classNames = std::set<std::string>{"street", "highway", "railway"};
    auto res = nogdb::ResultSet{};
    for (const auto &className: classNames) {
      auto tmp = nogdb::Edge::get(txn, className, cond1 or cond2);
      res.insert(res.end(), tmp.cbegin(), tmp.cend());
    }
    assertSize(res, 2);
    for (const auto &r: res) {
      assert(r.record.get("name").toText() == "The Outer Ring B" or
             r.record.get("name").toText() == "Catalina Way");
    }
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_invalid_edge_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("distance").lt(50.0)
                and nogdb::Condition("capacity").ge(300U);
    auto res = nogdb::Edge::get(txn, "streets", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("distance").lt(50.0)
                and nogdb::Condition("capacityyy").ge(300U);
    auto res = nogdb::Edge::get(txn, "street", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("distance").lt(50.0)
                and nogdb::Condition("capacity").contain("a");
    auto res = nogdb::Edge::get(txn, "street", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("distance").lt(50.0)
                and nogdb::Condition("capacity").ge(300U);
    auto res = nogdb::Edge::get(txn, "locations", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }
}

void test_find_edge_in_with_expression() {
  auto cmp = [](const nogdb::Result &name1, const nogdb::Result &name2) {
    return name1.record.get("name").toText() < name2.record.get("name").toText();
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("Dubai Building"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto expr = nogdb::Condition("distance").ge(80.0)
                or nogdb::Condition("capacity").gt(400U)
                or nogdb::Condition("temperature").lt(30);
    auto res = nogdb::Vertex::getInEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr});
    assertSize(res, 3);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Andrew Street");
    assert(res[1].record.get("name").toText() == "Andy Way");
    assert(res[2].record.get("name").toText() == "George Street");

    res = nogdb::Vertex::getInEdge(txn, vertex.descriptor, nogdb::GraphFilter{}.only("street"));
    assertSize(res, 2);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Andrew Street");
    assert(res[1].record.get("name").toText() == "George Street");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_edge_out_with_expression() {
  auto cmp = [](const nogdb::Result &name1, const nogdb::Result &name2) {
    return name1.record.get("name").toText() < name2.record.get("name").toText();
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("New York Tower"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto expr = nogdb::Condition("name").contain("Road").ignoreCase()
                or (nogdb::Condition("temperature").null()
                    and nogdb::Condition("capacity").ge(2000U))
                or (nogdb::Condition("temperature").gt(40)
                    and nogdb::Condition("distance").lt(140.0));
    auto res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assertSize(res, 1);
    assert(res[0].record.get("name").toText() == "Henry Road");

    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street", "highway"));
    assertSize(res, 2);
    for (const auto &r: res) {
      assert(r.record.get("name").toText() == "Henry Road"
             || r.record.get("name").toText() == "The Outer Ring B");
    }

    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr});
    assertSize(res, 4);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Andy Way");
    assert(res[1].record.get("name").toText() == "Bamboo Way");
    assert(res[2].record.get("name").toText() == "Henry Road");
    assert(res[3].record.get("name").toText() == "The Outer Ring B");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_edge_all_with_expression() {
  auto cmp = [](const nogdb::Result &name1, const nogdb::Result &name2) {
    return name1.record.get("name").toText() < name2.record.get("name").toText();
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("Pentagon"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto expr = nogdb::Condition("temperature")
                and nogdb::Condition("capacity")
                and nogdb::Condition("distance").gt(40.0);
    auto res = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assertSize(res, 2);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "George Street");
    assert(res[1].record.get("name").toText() == "Jetty Road");

    res = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr});
    assertSize(res, 3);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "George Street");
    assert(res[1].record.get("name").toText() == "Jetty Road");
    assert(res[2].record.get("name").toText() == "The Outer Ring C");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("Pentagon"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto cond1 =
        nogdb::Condition("@className").eq("street") and nogdb::Condition("name").contain("street").ignoreCase();
    auto cond2 = nogdb::Condition("@className").eq("highway") and nogdb::Condition("name").endWith("C");
    auto res = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{cond1 or cond2});
    assertSize(res, 3);
    for (const auto &r: res) {
      assert(r.record.get("name").toText() == "The Outer Ring C" ||
             r.record.get("name").toText() == "Isaac Street" ||
             r.record.get("name").toText() == "George Street");
    }
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_invalid_edge_in_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &vertex = vertices[0];
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getInEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getInEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only(classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("names").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getInEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("distance").contain("a");
    auto edges = nogdb::Vertex::getInEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getInEdge(txn, edge.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto res = nogdb::Vertex::getInEdge(txn, tmp, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_invalid_edge_out_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &vertex = vertices[0];
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only(classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("names").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("distance").contain("a");
    auto edges = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getOutEdge(txn, edge.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto res = nogdb::Vertex::getOutEdge(txn, tmp, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_invalid_edge_all_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &vertex = vertices[0];
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only(classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("names").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("distance").contain("a");
    auto edges = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getAllEdge(txn, edge.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto res = nogdb::Vertex::getAllEdge(txn, tmp, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_vertex_condition_function() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::get(txn, "locations", [](const nogdb::Record &record) {
      return record.get("name").toText().find("Building") != std::string::npos ||
             (!record.get("rating").empty() && record.get("rating").toReal() >= 4.5);
    });
    assertSize(res, 3);
    assert(res[0].record.get("name").toText() == "New York Tower");
    assert(res[1].record.get("name").toText() == "Dubai Building");
    assert(res[2].record.get("name").toText() == "Empire State Building");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto cmp = [](const nogdb::Record &record) {
      return ((!record.get("temperature").empty() && record.get("temperature").toInt() > 0) &&
              (!record.get("rating").empty() && record.get("rating").toReal() >= 3.0)) ||
             (!record.get("population").empty() && record.get("population").toBigIntU() <= 900ULL);
    };
    auto res = nogdb::Vertex::get(txn, "mountains", cmp);
    auto res2 = nogdb::Vertex::get(txn, "locations", cmp);
    res.insert(res.end(), res2.cbegin(), res2.cend());
    assertSize(res, 5);
    assert(res[0].record.get("name").toText() == "Blue Mountain");
    assert(res[1].record.get("name").toText() == "New York Tower");
    assert(res[2].record.get("name").toText() == "Dubai Building");
    assert(res[3].record.get("name").toText() == "ThaiCC Tower");
    assert(res[4].record.get("name").toText() == "Pentagon");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto cmp = [](const nogdb::Record &record) {
      if (!record.get("temperature").empty()) {
        return (record.getText("@className") == "locations" && record.getInt("temperature") < 12) ||
               (record.getText("@className") == "mountains" && record.getInt("temperature") > 0);
      } else {
        return false;
      }
    };
    auto res = nogdb::Vertex::get(txn, "locations", cmp);
    auto res2 = nogdb::Vertex::get(txn, "mountains", cmp);
    res.insert(res.end(), res2.cbegin(), res2.cend());
    assertSize(res, 2);
    for (const auto &r: res) {
      assert(r.record.get("name").toText() == "New York Tower" ||
             r.record.get("name").toText() == "Blue Mountain");
    }
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_edge_condition_function() {
  auto test_condition_function_1 = [](const nogdb::Record &record) {
    if (record.get("name").empty()) return false;
    return (record.get("name").toText() == "George Street");
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::get(txn, "street", test_condition_function_1);
    assertSize(res, 1);
    auto tmp = Coordinates{};
    res[0].record.get("coordinates").convertTo(tmp);
    assert(tmp.x == 0.1);
    assert(tmp.y == -0.1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_edge_in_condition_function() {
  auto cmp = [](const nogdb::Result &name1, const nogdb::Result &name2) {
    return name1.record.get("name").toText() < name2.record.get("name").toText();
  };

  auto test_condition_function_4 = [](const nogdb::Record &record) {
    if (record.get("distance").empty()) return false;
    return (record.get("distance").toReal() > 40.0);
  };

  auto test_condition_function_5 = [](const nogdb::Record &record) {
    if (record.get("distance").empty()) return false;
    return (record.get("distance").toReal() < 200.0);
  };

  auto test_condition_function_6 = [](const nogdb::Record &record) {
    if (record.get("distance").empty()) return false;
    return (record.get("distance").toReal() >= 36.2);
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("Dubai Building"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto res = nogdb::Vertex::getInEdge(txn, vertex.descriptor,
                                        nogdb::GraphFilter{nogdb::Condition("name").eq("George Street")}.only(
                                            "street"));
    assertSize(res, 1);
    res = nogdb::Vertex::getInEdge(txn, vertex.descriptor,
                                   nogdb::GraphFilter{test_condition_function_4}.only("street"));
    assertSize(res, 2);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Andrew Street");
    assert(res[1].record.get("name").toText() == "George Street");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("Pentagon"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto classNames = std::vector<std::string>{"street", "railway"};
    auto res = nogdb::Vertex::getInEdge(txn, vertex.descriptor,
                                        nogdb::GraphFilter{nogdb::Condition("name").eq("Isaac Street")}.only(
                                            classNames));
    assertSize(res, 1);
    res = nogdb::Vertex::getInEdge(txn, vertex.descriptor,
                                   nogdb::GraphFilter{test_condition_function_5}.only(classNames));
    assertSize(res, 3);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Dwayne Way");
    assert(res[1].record.get("name").toText() == "Henry Road");
    assert(res[2].record.get("name").toText() == "Isaac Street");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto res = nogdb::Vertex::getInEdge(txn, vertex.descriptor,
                                        nogdb::GraphFilter{nogdb::Condition("name").eq("The Outer Ring C")});
    assertSize(res, 1);
    res = nogdb::Vertex::getInEdge(txn, vertex.descriptor, nogdb::GraphFilter{test_condition_function_6});
    assertSize(res, 3);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Eastern Way");
    assert(res[1].record.get("name").toText() == "Fisher Avenue");
    assert(res[2].record.get("name").toText() == "The Outer Ring C");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_edge_out_condition_function() {
  auto cmp = [](const nogdb::Result &name1, const nogdb::Result &name2) {
    return name1.record.get("name").toText() < name2.record.get("name").toText();
  };

  auto test_condition_function_7 = [](const nogdb::Record &record) {
    if (record.get("distance").empty()) return false;
    return (record.get("distance").toReal() >= 100.0);
  };

  auto test_condition_function_8 = [](const nogdb::Record &record) {
    auto tmp = record.get("temperature");
    if (tmp.empty()) return false;
    return (tmp.toInt() <= 42);
  };

  auto test_condition_function_9 = [](const nogdb::Record &record) {
    return record.get("temperature").empty();
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("New York Tower"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor,
                                         nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                             "street"));
    assertSize(res, 1);
    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor,
                                    nogdb::GraphFilter{test_condition_function_7}.only("railway"));
    assertSize(res, 3);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Bamboo Way");
    assert(res[1].record.get("name").toText() == "Catalina Way");
    assert(res[2].record.get("name").toText() == "Dwayne Way");
    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor,
                                    nogdb::GraphFilter{test_condition_function_8}.only("railway"));
    assertSize(res, 2);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Andy Way");
    assert(res[1].record.get("name").toText() == "Catalina Way");
    res = nogdb::Vertex::getOutEdge(txn, vertex.descriptor,
                                    nogdb::GraphFilter{test_condition_function_9}.only("railway"));
    assertSize(res, 1);
    assert(res[0].record.get("name").toText() == "Dwayne Way");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

}

void test_find_edge_all_condition_function() {
  auto cmp = [](const nogdb::Result &name1, const nogdb::Result &name2) {
    return name1.record.get("name").toText() < name2.record.get("name").toText();
  };

  auto test_condition_function_10 = [](const nogdb::Record &record) {
    if (record.get("distance").empty()) return false;
    return (record.get("distance").toReal() > 100);
  };

  auto test_condition_function_11 = [](const nogdb::Record &record) {
    if (record.get("distance").empty()) return false;
    return (record.get("distance").toReal() <= 100);
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("Pentagon"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto res = nogdb::Vertex::getAllEdge(txn, vertex.descriptor,
                                         nogdb::GraphFilter{nogdb::Condition("name").eq("George Street")}.only(
                                             "street"));
    assertSize(res, 1);
    res = nogdb::Vertex::getAllEdge(txn, vertex.descriptor,
                                    nogdb::GraphFilter{nogdb::Condition("distance").ge(50.0)}.only("street"));
    assertSize(res, 2);
    std::sort(res.begin(), res.end(), cmp);
    assert(res[0].record.get("name").toText() == "Henry Road");
    assert(res[1].record.get("name").toText() == "Jetty Road");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    assert(vertices.size() == 1);
    auto &vertex = vertices[0];
    auto classNames = std::vector<std::string>{"street", "railway"};
    auto res = nogdb::Vertex::getAllEdge(txn, vertex.descriptor,
                                         nogdb::GraphFilter{test_condition_function_10}.only(classNames));
    assertSize(res, 1);
    assert(res[0].record.get("name").toText() == "Gravity Way");
    res = nogdb::Vertex::getAllEdge(txn, vertex.descriptor,
                                    nogdb::GraphFilter{test_condition_function_11}.only(classNames));
    assertSize(res, 5);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_invalid_vertex_condition_function() {
  auto condition = [](const nogdb::Record &record) {
    return record.get("name").toText().find("Building") != std::string::npos ||
           record.get("rating").toReal() >= 4.5;
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::get(txn, "location", condition);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::get(txn, "street", condition);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }
}

void test_find_invalid_edge_condition_function() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::get(txn, "streets", [](const nogdb::Record &record) {
      return record.get("distance").toReal() < 50.0 && record.get("capacity").toIntU() >= 300U;
    });
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::get(txn, "locations", [](const nogdb::Record &record) {
      return record.get("distance").toReal() < 50.0 && record.get("capacity").toIntU() >= 300U;
    });
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }
}

void test_find_invalid_edge_in_condition_function() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  auto &vertex = vertices[0];
  auto condition = [](const nogdb::Record &record) {
    return (record.get("name").toText().find("Street") != std::string::npos) ||
           !record.get("distance").empty();
  };

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getInEdge(txn, vertex.descriptor, nogdb::GraphFilter{condition}.only("streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getInEdge(txn, vertex.descriptor, nogdb::GraphFilter{condition}.only(classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto edges = nogdb::Vertex::getInEdge(txn, edge.descriptor, nogdb::GraphFilter{condition}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto res = nogdb::Vertex::getInEdge(txn, tmp, nogdb::GraphFilter{condition}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_invalid_edge_out_condition_function() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  auto &vertex = vertices[0];
  auto condition = [](const nogdb::Record &record) {
    return (record.get("name").toText().find("Street") != std::string::npos) ||
           !record.get("distance").empty();
  };

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, nogdb::GraphFilter{condition}.only("streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getOutEdge(txn, vertex.descriptor, nogdb::GraphFilter{condition}.only(classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto edges = nogdb::Vertex::getOutEdge(txn, edge.descriptor, nogdb::GraphFilter{condition}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto res = nogdb::Vertex::getOutEdge(txn, tmp, nogdb::GraphFilter{condition}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_invalid_edge_all_condition_function() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  auto &vertex = vertices[0];
  auto condition = [](const nogdb::Record &record) {
    return (record.get("name").toText().find("Street") != std::string::npos) ||
           !record.get("distance").empty();
  };
  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{condition}.only("streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getAllEdge(txn, vertex.descriptor, nogdb::GraphFilter{condition}.only(classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto edges = nogdb::Vertex::getAllEdge(txn, edge.descriptor, nogdb::GraphFilter{condition}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto res = nogdb::Vertex::getAllEdge(txn, tmp, nogdb::GraphFilter{condition}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_vertex_cursor() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("Pentagon"));
    assertSize(res, 1);
    auto tmp = Coordinates{};
    res.first();
    res->record.get("coordinates").convertTo(tmp);
    assert(tmp.x == -1.00);
    assert(tmp.y == 1.00);
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("Tokyo Tower"));
    assertSize(res, 0);
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("temperature").eq(18));
    assertSize(res, 1);
    res.first();
    assert(res->record.get("name").toText() == "Pentagon");
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("postcode").eq(11600U));
    assertSize(res, 1);
    res.first();
    assert(res->record.get("name").toText() == "ThaiCC Tower");
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("price").eq(280000LL));
    assertSize(res, 1);
    res.first();
    assert(res->record.get("name").toText() == "Dubai Building");
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("population").eq(900ULL));
    assertSize(res, 2);
    res.next();
    assert(res->record.get("name").toText() == "ThaiCC Tower");
    res.next();
    assert(res->record.get("name").toText() == "Pentagon");
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("rating").eq(4.5));
    assertSize(res, 2);
    res.next();
    assert(res->record.get("name").toText() == "New York Tower");
    res.next();
    assert(res->record.get("name").toText() == "Empire State Building");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto res = nogdb::Vertex::getCursor(txn, "locations", !nogdb::Condition("name").eq("Pentagon"));
    assertSize(res, 4);
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("temperature").gt(35));
    assertSize(res, 1);
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("rating").ge(4.5));
    assertSize(res, 3);
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("postcode").lt(10300U));
    assertSize(res, 2);
    nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("population").le(900ULL));
    assertSize(res, 2);
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("price").null());
    assertSize(res, 1);
    res.last();
    assert(res->record.get("name").toText() == "ThaiCC Tower");
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("temperature"));
    assertSize(res, 4);
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq(100));
    assertSize(res, 0);
    res = nogdb::Vertex::getCursor(txn, "locations",
                                   nogdb::Condition("population").eq(static_cast<unsigned long long>(2000)));
    assertSize(res, 1);
    res.last();
    assert(res->record.get("name").toText() == "New York Tower");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").contain("tag").ignoreCase());
    assertSize(res, 1);
    res.to(0);
    assert(res->record.get("name").toText() == "Pentagon");
    res = nogdb::Vertex::getCursor(txn, "locations", !nogdb::Condition("name").contain("u").ignoreCase());
    assertSize(res, 3);
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").beginWith("thai").ignoreCase());
    assertSize(res, 1);
    res.to(0);
    assert(res->record.get("name").toText() == "ThaiCC Tower");
    res = nogdb::Vertex::getCursor(txn, "locations", !nogdb::Condition("name").beginWith("Thai"));
    assertSize(res, 4);
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").endWith("TOWER").ignoreCase());
    assertSize(res, 2);
    res = nogdb::Vertex::getCursor(txn, "locations", !nogdb::Condition("name").endWith("Building"));
    assertSize(res, 3);
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").gt("Pentagon"));
    assertSize(res, 1);
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").ge("Pentagon"));
    assertSize(res, 2);
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").lt("Pentagon"));
    assertSize(res, 3);
    res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").le("Pentagon"));
    assertSize(res, 4);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_invalid_vertex_cursor() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::getCursor(txn, "location", nogdb::Condition("name"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("names"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("coordinates").contain("a"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::getCursor(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }
}

void test_find_edge_cursor() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::getCursor(txn, "street", nogdb::Condition("name").eq("George Street"));
    assertSize(res, 1);
    auto tmp = Coordinates{};
    res.first();
    res->record.get("coordinates").convertTo(tmp);
    assert(tmp.x == 0.1);
    assert(tmp.y == -0.1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();
}

void test_find_invalid_edge_cursor() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::getCursor(txn, "streets", nogdb::Condition("name"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::getCursor(txn, "railway", nogdb::Condition("names"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::getCursor(txn, "highway", nogdb::Condition("coordinates").contain("a"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::getCursor(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }
}

void test_find_vertex_cursor_condition_function() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::getCursor(txn, "locations", [](const nogdb::Record &record) {
      return record.get("name").toText().find("Building") != std::string::npos ||
             (!record.get("rating").empty() && record.get("rating").toReal() >= 4.5);
    });
    assertSize(res, 3);
    res.next();
    assert(res->record.get("name").toText() == "New York Tower");
    res.next();
    assert(res->record.get("name").toText() == "Dubai Building");
    res.next();
    assert(res->record.get("name").toText() == "Empire State Building");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto cmp = [](const nogdb::Record &record) {
      return ((!record.get("temperature").empty() && record.get("temperature").toInt() > 0) &&
              (!record.get("rating").empty() && record.get("rating").toReal() >= 3.0)) ||
             (!record.get("population").empty() && record.get("population").toBigIntU() <= 900ULL);
    };
    auto res = nogdb::Vertex::getCursor(txn, "locations", cmp);
    assertSize(res, 4);
    res.first();
    assert(res->record.get("name").toText() == "New York Tower");
    res.to(1);
    assert(res->record.get("name").toText() == "Dubai Building");
    res.to(2);
    assert(res->record.get("name").toText() == "ThaiCC Tower");
    res.last();
    assert(res->record.get("name").toText() == "Pentagon");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_edge_cursor_condition_function() {
  auto test_condition_function_1 = [](const nogdb::Record &record) {
    if (record.get("name").empty()) return false;
    return (record.get("name").toText() == "George Street");
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::getCursor(txn, "street", test_condition_function_1);
    assertSize(res, 1);
    auto tmp = Coordinates{};
    res.next();
    res->record.get("coordinates").convertTo(tmp);
    assert(tmp.x == 0.1);
    assert(tmp.y == -0.1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_invalid_vertex_cursor_condition_function() {
  auto condition = [](const nogdb::Record &record) {
    return record.get("name").toText().find("Building") != std::string::npos ||
           record.get("rating").toReal() >= 4.5;
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::getCursor(txn, "location", condition);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::getCursor(txn, "street", condition);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }
}

void test_find_invalid_edge_cursor_condition_function() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::getCursor(txn, "streets", [](const nogdb::Record &record) {
      return record.get("distance").toReal() < 50.0 && record.get("capacity").toIntU() >= 300U;
    });
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::getCursor(txn, "locations", [](const nogdb::Record &record) {
      return record.get("distance").toReal() < 50.0 && record.get("capacity").toIntU() >= 300U;
    });
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }
}

void test_find_vertex_cursor_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("name").endWith("Building").ignoreCase()
                or nogdb::Condition("rating").ge(4.5);
    auto res = nogdb::Vertex::getCursor(txn, "locations", expr);
    assertSize(res, 3);
    res.next();
    assert(res->record.get("name").toText() == "New York Tower");
    res.next();
    assert(res->record.get("name").toText() == "Dubai Building");
    res.next();
    assert(res->record.get("name").toText() == "Empire State Building");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto expr1 = (nogdb::Condition("temperature").gt(0)
                  and nogdb::Condition("rating").ge(3.0));
    auto expr2 = nogdb::Condition("population").le(900ULL);
    auto res = nogdb::Vertex::getCursor(txn, "locations", expr1 or expr2);
    assertSize(res, 4);
    res.first();
    assert(res->record.get("name").toText() == "New York Tower");
    res.to(1);
    assert(res->record.get("name").toText() == "Dubai Building");
    res.to(2);
    assert(res->record.get("name").toText() == "ThaiCC Tower");
    res.last();
    assert(res->record.get("name").toText() == "Pentagon");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_invalid_vertex_cursor_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("name").endWith("Building").ignoreCase()
                or nogdb::Condition("rating").ge(4.5);
    auto res = nogdb::Vertex::getCursor(txn, "location", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("names").endWith("Building").ignoreCase()
                or nogdb::Condition("rating").ge(4.5);
    auto res = nogdb::Vertex::getCursor(txn, "locations", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("name").endWith("Building").ignoreCase()
                or nogdb::Condition("rating").contain("a");
    auto res = nogdb::Vertex::getCursor(txn, "locations", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("name").endWith("Building").ignoreCase()
                or nogdb::Condition("rating").ge(4.5);
    auto res = nogdb::Vertex::getCursor(txn, "street", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }
}

void test_find_edge_cursor_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr1 = nogdb::Condition("distance").lt(50.0)
                 and nogdb::Condition("capacity").ge(300U);
    auto expr2 = nogdb::Condition("distance").ge(50.0)
                 and nogdb::Condition("temperature").gt(30);
    auto res = nogdb::Edge::getCursor(txn, "street", expr1 or expr2);
    assertSize(res, 5);
    auto elements = std::vector<std::string>{"George Street",
                                             "Isaac Street",
                                             "Andrew Street",
                                             "Fisher Avenue",
                                             "Jetty Road"};
    cursorTester(res, elements, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_invalid_edge_cursor_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("distance").lt(50.0)
                and nogdb::Condition("capacity").ge(300U);
    auto res = nogdb::Edge::getCursor(txn, "streets", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("distance").lt(50.0)
                and nogdb::Condition("capacityyy").ge(300U);
    auto res = nogdb::Edge::getCursor(txn, "street", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("distance").lt(50.0)
                and nogdb::Condition("capacity").contain("a");
    auto res = nogdb::Edge::getCursor(txn, "street", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("distance").lt(50.0)
                and nogdb::Condition("capacity").ge(300U);
    auto res = nogdb::Edge::getCursor(txn, "locations", expr);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }
}

void test_find_edge_in_cursor() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("Dubai Building"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto res = nogdb::Vertex::getInEdgeCursor(txn, vertex,
                                              nogdb::GraphFilter{nogdb::Condition("name").eq("George Street")}.only(
                                                  "street"));
    assertSize(res, 1);
    res = nogdb::Vertex::getInEdgeCursor(txn, vertex,
                                         nogdb::GraphFilter{nogdb::Condition("distance").gt(40.0)}.only("street"));
    assert(res.count() == 2);
    cursorContains(res, std::set<std::string>{"Andrew Street", "George Street"}, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("Pentagon"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto classNames = std::vector<std::string>{"street", "railway"};
    auto res = nogdb::Vertex::getInEdgeCursor(txn, vertex,
                                              nogdb::GraphFilter{nogdb::Condition("name").eq("Isaac Street")}.only(
                                                  classNames));
    assertSize(res, 1);
    res = nogdb::Vertex::getInEdgeCursor(txn, vertex,
                                         nogdb::GraphFilter{nogdb::Condition("distance").lt(200.0)}.only(classNames));
    assert(res.count() == 3);
    cursorContains(res, std::set<std::string>{"Dwayne Way", "Henry Road", "Isaac Street"}, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    assert(vertices.size() == 1);
    vertices.first();
    auto &vertex = vertices->descriptor;
    auto res = nogdb::Vertex::getInEdgeCursor(txn, vertex, nogdb::Condition("name").eq("The Outer Ring C"));
    assertSize(res, 1);
    res = nogdb::Vertex::getInEdgeCursor(txn, vertex, nogdb::Condition("distance").ge(36.2));
    assert(res.count() == 3);
    cursorContains(res, std::set<std::string>{"Eastern Way", "Fisher Avenue", "The Outer Ring C"}, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_edge_out_cursor() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("New York Tower"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto res = nogdb::Vertex::getOutEdgeCursor(txn, vertex,
                                               nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                                   "street"));
    assertSize(res, 1);
    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex,
                                          nogdb::GraphFilter{nogdb::Condition("distance").ge(100.0)}.only("railway"));
    assertSize(res, 3);
    cursorContains(res, std::set<std::string>{"Bamboo Way", "Catalina Way", "Dwayne Way"}, "name");
    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex,
                                          nogdb::GraphFilter{nogdb::Condition("temperature").le(42)}.only("railway"));
    assertSize(res, 2);
    cursorContains(res, std::set<std::string>{"Andy Way", "Catalina Way"}, "name");
    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex,
                                          nogdb::GraphFilter{!nogdb::Condition("temperature")}.only("railway"));
    assertSize(res, 1);
    cursorContains(res, std::set<std::string>{"Dwayne Way"}, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("New York Tower"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto classNames = std::vector<std::string>{"street", "railway"};
    auto res = nogdb::Vertex::getOutEdgeCursor(txn, vertex,
                                               nogdb::GraphFilter{!nogdb::Condition("name").eq("Andrew Street")}.only(
                                                   classNames));
    assertSize(res, 5);
    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex,
                                          nogdb::GraphFilter{!nogdb::Condition("name").contain("boo")}.only(
                                              classNames));
    assertSize(res, 5);
    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex,
                                          nogdb::GraphFilter{nogdb::Condition("name").contain("BOO").ignoreCase()}.only(
                                              classNames));
    assertSize(res, 1);
    cursorContains(res, std::set<std::string>{"Bamboo Way"}, "name");
    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex,
                                          nogdb::GraphFilter{
                                              !nogdb::Condition("name").beginWith("a").ignoreCase()}.only(classNames));
    assertSize(res, 4);
    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex,
                                          nogdb::GraphFilter{nogdb::Condition("name").beginWith("A")}.only(classNames));
    assertSize(res, 2);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("New York Tower"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto res = nogdb::Vertex::getOutEdgeCursor(txn, vertex, nogdb::Condition("name").eq("The Outer Ring B"));
    assertSize(res, 1);
    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex, !nogdb::Condition("name").endWith("StrEEt").ignoreCase());
    assertSize(res, 6);
    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex, nogdb::Condition("name").endWith("Way"));
    assertSize(res, 4);
    cursorContains(res, std::set<std::string>{"Andy Way", "Bamboo Way", "Catalina Way", "Dwayne Way"}, "name");
    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex, !nogdb::Condition("coordinates").null());
    assertSize(res, 4);
    cursorContains(res, std::set<std::string>{"Andrew Street", "Andy Way", "Bamboo Way", "The Outer Ring B"}, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_edge_all_cursor() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("Pentagon"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto res = nogdb::Vertex::getAllEdgeCursor(txn, vertex,
                                               nogdb::GraphFilter{nogdb::Condition("name").eq("George Street")}.only(
                                                   "street"));
    assertSize(res, 1);
    res = nogdb::Vertex::getAllEdgeCursor(txn, vertex,
                                          nogdb::GraphFilter{nogdb::Condition("distance").ge(50.0),}.only("street"));
    assertSize(res, 2);
    cursorContains(res, std::set<std::string>{"Henry Road", "Jetty Road"}, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto classNames = std::vector<std::string>{"street", "railway"};
    auto res = nogdb::Vertex::getAllEdgeCursor(txn, vertex,
                                               nogdb::GraphFilter{nogdb::Condition("distance").gt(100.0)}.only(
                                                   classNames));
    assertSize(res, 1);
    res.next();
    assert(res->record.get("name").toText() == "Gravity Way");
    res = nogdb::Vertex::getAllEdgeCursor(txn, vertex,
                                          nogdb::GraphFilter{nogdb::Condition("distance").le(100.0)}.only(classNames));
    assertSize(res, 5);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto res = nogdb::Vertex::getAllEdgeCursor(txn, vertex, nogdb::Condition("capacity").eq(100U));
    assertSize(res, 1);
    res.first();
    assert(res->record.get("name").toText() == "Doodee Street");
    res = nogdb::Vertex::getAllEdgeCursor(txn, vertex, nogdb::Condition("name").contain("C"));
    assertSize(res, 2);
    cursorContains(res, std::set<std::string>{"Cowboy Road", "The Outer Ring C"}, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_invalid_edge_in_cursor() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &vertex = vertices[0];
  try {
    nogdb::Vertex::getInEdgeCursor(txn, vertex.descriptor,
                                   nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only("streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    nogdb::Vertex::getInEdgeCursor(txn, vertex.descriptor,
                                   nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getInEdgeCursor(txn, vertex.descriptor,
                                                nogdb::GraphFilter{nogdb::Condition("names").eq("Andrew Street")}.only(
                                                    "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getInEdgeCursor(txn, vertex.descriptor,
                                                nogdb::GraphFilter{nogdb::Condition("distance").contain("a")}.only(
                                                    "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    nogdb::Vertex::getInEdgeCursor(txn, edge.descriptor,
                                   nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto res = nogdb::Vertex::getInEdgeCursor(txn, tmp,
                                              nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                                  "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_invalid_edge_out_cursor() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &vertex = vertices[0];
  try {
    auto edges = nogdb::Vertex::getOutEdgeCursor(txn, vertex.descriptor,
                                                 nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                                     "streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getOutEdgeCursor(txn, vertex.descriptor,
                                                 nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                                     classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getOutEdgeCursor(txn, vertex.descriptor,
                                                 nogdb::GraphFilter{nogdb::Condition("names").eq("Andrew Street")}.only(
                                                     "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getOutEdgeCursor(txn, vertex.descriptor,
                                                 nogdb::GraphFilter{nogdb::Condition("distance").contain("a")}.only(
                                                     "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto edges = nogdb::Vertex::getOutEdgeCursor(txn, edge.descriptor,
                                                 nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                                     "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto res = nogdb::Vertex::getOutEdgeCursor(txn, tmp,
                                               nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                                   "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_invalid_edge_all_cursor() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &vertex = vertices[0];
  try {
    auto edges = nogdb::Vertex::getAllEdgeCursor(txn, vertex.descriptor,
                                                 nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                                     "streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getAllEdgeCursor(txn, vertex.descriptor,
                                                 nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                                     classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getAllEdgeCursor(txn, vertex.descriptor,
                                                 nogdb::GraphFilter{nogdb::Condition("names").eq("Andrew Street")}.only(
                                                     "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getAllEdgeCursor(txn, vertex.descriptor,
                                                 nogdb::GraphFilter{nogdb::Condition("distance").contain("a")}.only(
                                                     "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto edges = nogdb::Vertex::getAllEdgeCursor(txn, edge.descriptor,
                                                 nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                                     "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto res = nogdb::Vertex::getAllEdgeCursor(txn, tmp,
                                               nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                                   "street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_edge_in_cursor_condition_function() {
  auto test_condition_function_4 = [](const nogdb::Record &record) {
    if (record.get("distance").empty()) return false;
    return (record.get("distance").toReal() > 40.0);
  };

  auto test_condition_function_5 = [](const nogdb::Record &record) {
    if (record.get("distance").empty()) return false;
    return (record.get("distance").toReal() < 200.0);
  };

  auto test_condition_function_6 = [](const nogdb::Record &record) {
    if (record.get("distance").empty()) return false;
    return (record.get("distance").toReal() >= 36.2);
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("Dubai Building"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto res = nogdb::Vertex::getInEdgeCursor(txn, vertex,
                                              nogdb::GraphFilter{nogdb::Condition("name").eq("George Street")}.only(
                                                  "street"));
    assertSize(res, 1);
    res = nogdb::Vertex::getInEdgeCursor(txn, vertex, nogdb::GraphFilter{test_condition_function_4}.only("street"));
    assertSize(res, 2);
    cursorContains(res, std::set<std::string>{"Andrew Street", "George Street"}, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("Pentagon"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto classNames = std::vector<std::string>{"street", "railway"};
    auto res = nogdb::Vertex::getInEdgeCursor(txn, vertex,
                                              nogdb::GraphFilter{nogdb::Condition("name").eq("Isaac Street")}.only(
                                                  classNames));
    assertSize(res, 1);
    res = nogdb::Vertex::getInEdgeCursor(txn, vertex,
                                         nogdb::GraphFilter{test_condition_function_5}.only(classNames));
    assertSize(res, 3);
    cursorContains(res, std::set<std::string>{"Dwayne Way", "Henry Road", "Isaac Street"}, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto res = nogdb::Vertex::getInEdgeCursor(txn, vertex, nogdb::Condition("name").eq("The Outer Ring C"));
    assertSize(res, 1);
    res = nogdb::Vertex::getInEdgeCursor(txn, vertex, nogdb::GraphFilter{test_condition_function_6});
    assertSize(res, 3);
    cursorContains(res, std::set<std::string>{"Eastern Way", "Fisher Avenue", "The Outer Ring C"}, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_edge_out_cursor_condition_function() {
  auto test_condition_function_7 = [](const nogdb::Record &record) {
    if (record.get("distance").empty()) return false;
    return (record.get("distance").toReal() >= 100.0);
  };

  auto test_condition_function_8 = [](const nogdb::Record &record) {
    auto tmp = record.get("temperature");
    if (tmp.empty()) return false;
    return (tmp.toInt() <= 42);
  };

  auto test_condition_function_9 = [](const nogdb::Record &record) {
    return record.get("temperature").empty();
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("New York Tower"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto res = nogdb::Vertex::getOutEdgeCursor(txn, vertex,
                                               nogdb::GraphFilter{nogdb::Condition("name").eq("Andrew Street")}.only(
                                                   "street"));
    assertSize(res, 1);
    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex, nogdb::GraphFilter{test_condition_function_7}.only("railway"));
    assertSize(res, 3);
    cursorContains(res, std::set<std::string>{"Bamboo Way", "Catalina Way", "Dwayne Way"}, "name");
    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex, nogdb::GraphFilter{test_condition_function_8}.only("railway"));
    assertSize(res, 2);
    cursorContains(res, std::set<std::string>{"Andy Way", "Catalina Way"}, "name");
    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex, nogdb::GraphFilter{test_condition_function_9}.only("railway"));
    assertSize(res, 1);
    res.first();
    assert(res->record.get("name").toText() == "Dwayne Way");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();
}

void test_find_edge_all_cursor_condition_function() {
  auto test_condition_function_10 = [](const nogdb::Record &record) {
    if (record.get("distance").empty()) return false;
    return (record.get("distance").toReal() > 100);
  };

  auto test_condition_function_11 = [](const nogdb::Record &record) {
    if (record.get("distance").empty()) return false;
    return (record.get("distance").toReal() <= 100);
  };

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("Pentagon"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto res = nogdb::Vertex::getAllEdgeCursor(txn, vertex,
                                               nogdb::GraphFilter{nogdb::Condition("name").eq("George Street")}.only(
                                                   "street"));
    assertSize(res, 1);
    res = nogdb::Vertex::getAllEdgeCursor(txn, vertex,
                                          nogdb::GraphFilter{nogdb::Condition("distance").ge(50.0)}.only("street"));
    assertSize(res, 2);
    cursorContains(res, std::set<std::string>{"Henry Road", "Jetty Road"}, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto classNames = std::vector<std::string>{"street", "railway"};
    auto res = nogdb::Vertex::getAllEdgeCursor(txn, vertex,
                                               nogdb::GraphFilter{test_condition_function_10}.only(classNames));
    assertSize(res, 1);
    res.next();
    assert(res->record.get("name").toText() == "Gravity Way");
    res = nogdb::Vertex::getAllEdgeCursor(txn, vertex,
                                          nogdb::GraphFilter{test_condition_function_11}.only(classNames));
    assertSize(res, 5);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_invalid_edge_in_cursor_condition_function() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  auto &vertex = vertices[0];
  auto condition = [](const nogdb::Record &record) {
    return (record.get("name").toText().find("Street") != std::string::npos) ||
           !record.get("distance").empty();
  };

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getInEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{condition}.only("streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getInEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{condition}.only(classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto edges = nogdb::Vertex::getInEdgeCursor(txn, edge.descriptor, nogdb::GraphFilter{condition}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto res = nogdb::Vertex::getInEdgeCursor(txn, tmp, nogdb::GraphFilter{condition}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_invalid_edge_out_cursor_condition_function() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  auto &vertex = vertices[0];
  auto condition = [](const nogdb::Record &record) {
    return (record.get("name").toText().find("Street") != std::string::npos) ||
           !record.get("distance").empty();
  };

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getOutEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{condition}.only("streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getOutEdgeCursor(txn, vertex.descriptor,
                                                 nogdb::GraphFilter{condition}.only(classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto edges = nogdb::Vertex::getOutEdgeCursor(txn, edge.descriptor, nogdb::GraphFilter{condition}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto res = nogdb::Vertex::getOutEdgeCursor(txn, tmp, nogdb::GraphFilter{condition}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_invalid_edge_all_cursor_condition_function() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  auto &vertex = vertices[0];
  auto condition = [](const nogdb::Record &record) {
    return (record.get("name").toText().find("Street") != std::string::npos) ||
           !record.get("distance").empty();
  };
  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto edges = nogdb::Vertex::getAllEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{condition}.only("streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getAllEdgeCursor(txn, vertex.descriptor,
                                                 nogdb::GraphFilter{condition}.only(classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto edges = nogdb::Vertex::getAllEdgeCursor(txn, edge.descriptor, nogdb::GraphFilter{condition}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto res = nogdb::Vertex::getAllEdgeCursor(txn, tmp, nogdb::GraphFilter{condition}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_edge_in_cursor_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("Dubai Building"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto expr = nogdb::Condition("distance").ge(80.0)
                or nogdb::Condition("capacity").gt(400U)
                or nogdb::Condition("temperature").lt(30);
    auto res = nogdb::Vertex::getInEdgeCursor(txn, vertex, expr);
    assertSize(res, 3);
    cursorContains(res, std::set<std::string>{"Andrew Street", "Andy Way", "George Street"}, "name");
    res = nogdb::Vertex::getInEdgeCursor(txn, vertex, nogdb::GraphFilter{expr}.only("street"));
    assertSize(res, 2);
    cursorContains(res, std::set<std::string>{"Andrew Street", "George Street"}, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_edge_out_cursor_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("New York Tower"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto expr = nogdb::Condition("name").contain("Road").ignoreCase()
                or (nogdb::Condition("temperature").null()
                    and nogdb::Condition("capacity").ge(2000U))
                or (nogdb::Condition("temperature").gt(40)
                    and nogdb::Condition("distance").lt(140.0));
    auto res = nogdb::Vertex::getOutEdgeCursor(txn, vertex, nogdb::GraphFilter{expr}.only("street"));
    assertSize(res, 1);
    res.next();
    assert(res->record.get("name").toText() == "Henry Road");

    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex, nogdb::GraphFilter{expr}.only("street", "highway"));
    assertSize(res, 2);
    cursorContains(res, std::set<std::string>{"Henry Road", "The Outer Ring B"}, "name");

    res = nogdb::Vertex::getOutEdgeCursor(txn, vertex, expr);
    assertSize(res, 4);
    cursorContains(res, std::set<std::string>{"Andy Way", "Bamboo Way", "Henry Road", "The Outer Ring B"}, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_edge_all_cursor_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("Pentagon"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto expr = nogdb::Condition("temperature")
                and nogdb::Condition("capacity")
                and nogdb::Condition("distance").gt(40.0);
    auto res = nogdb::Vertex::getAllEdgeCursor(txn, vertex, nogdb::GraphFilter{expr}.only("street"));
    assertSize(res, 2);
    cursorContains(res, std::set<std::string>{"George Street", "Jetty Road"}, "name");

    res = nogdb::Vertex::getAllEdgeCursor(txn, vertex, expr);
    assertSize(res, 3);
    cursorContains(res, std::set<std::string>{"George Street", "Jetty Road", "The Outer Ring C"}, "name");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto vertices = nogdb::Vertex::getCursor(txn, "locations", nogdb::Condition("name").eq("Pentagon"));
    assert(vertices.size() == 1);
    vertices.next();
    auto &vertex = vertices->descriptor;
    auto cond1 =
        nogdb::Condition("@className").eq("street") and nogdb::Condition("name").contain("street").ignoreCase();
    auto cond2 = nogdb::Condition("@className").eq("highway") and nogdb::Condition("name").endWith("C");
    auto res = nogdb::Vertex::getAllEdgeCursor(txn, vertex, cond1 or cond2);
    assertSize(res, 3);
    while (res.next()) {
      assert(res->record.getText("name") == "The Outer Ring C" ||
             res->record.getText("name") == "Isaac Street" ||
             res->record.getText("name") == "George Street");
    }
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_find_invalid_edge_in_cursor_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &vertex = vertices[0];
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getInEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getInEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only(classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("names").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getInEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("distance").contain("a");
    auto edges = nogdb::Vertex::getInEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getInEdgeCursor(txn, edge.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto res = nogdb::Vertex::getInEdgeCursor(txn, tmp, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_invalid_edge_out_cursor_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &vertex = vertices[0];
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getOutEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getOutEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only(classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("names").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getOutEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("distance").contain("a");
    auto edges = nogdb::Vertex::getOutEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getOutEdgeCursor(txn, edge.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto res = nogdb::Vertex::getOutEdgeCursor(txn, tmp, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}

void test_find_invalid_edge_all_cursor_with_expression() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto vertices = nogdb::ResultSet{};
  auto edges = nogdb::ResultSet{};
  try {
    vertices = nogdb::Vertex::get(txn, "locations", nogdb::Condition("name").eq("ThaiCC Tower"));
    edges = nogdb::Edge::get(txn, "street", nogdb::Condition("name").eq("Andrew Street"));
    assert(vertices.size() == 1);
    assert(edges.size() == 1);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &vertex = vertices[0];
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getAllEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("streets"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto classNames = std::vector<std::string>{"street", "railway", "subway"};
    auto edges = nogdb::Vertex::getAllEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only(classNames));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("names").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getAllEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto expr = nogdb::Condition("distance").contain("a");
    auto edges = nogdb::Vertex::getAllEdgeCursor(txn, vertex.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto &edge = edges[0];
  try {
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto edges = nogdb::Vertex::getAllEdgeCursor(txn, edge.descriptor, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = vertex.descriptor;
    tmp.rid.second = -1;
    auto expr = nogdb::Condition("name").endWith("Street").ignoreCase() or nogdb::Condition("distance");
    auto res = nogdb::Vertex::getAllEdgeCursor(txn, tmp, nogdb::GraphFilter{expr}.only("street"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
  }
}
