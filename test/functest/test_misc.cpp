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

#include "functest.h"
#include "test_prepare.h"

void test_get_set_empty_value() {
  init_vertex_person();
  init_edge_know();
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Record r_blank_name{};
    r_blank_name.set("name", "");
    auto rdesc1 = nogdb::Vertex::create(txn, "persons", r_blank_name);
    auto r1 = nogdb::DB::getRecord(txn, rdesc1);
    assert(r1.get("name").toText() == "");
    assert(r1.get("name").empty());

    auto rdesc2 = nogdb::Vertex::create(txn, "persons");
    auto r2 = nogdb::DB::getRecord(txn, rdesc2);
    assert(r2.empty());
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();
  destroy_edge_know();
  destroy_vertex_person();
}

void test_get_invalid_record() {
  init_vertex_book();
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  auto tmp = nogdb::RecordDescriptor{};
  try {
    nogdb::Record r{};
    r.set("title", "Lion King").set("price", 100.0).set("pages", 320);
    auto rdesc1 = nogdb::Vertex::create(txn, "books", r);
    r.set("title", "Tarzan").set("price", 60.0).set("pages", 360);
    auto rdesc2 = nogdb::Vertex::create(txn, "books", r);
    tmp = rdesc2;
    nogdb::Vertex::destroy(txn, rdesc1);

    try {
      auto res = nogdb::DB::getRecord(txn, rdesc1);
    } catch (const nogdb::Error &ex) {
      REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
    }
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  destroy_vertex_book();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::DB::getRecord(txn, tmp);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }
}

void test_get_set_large_record() {
  init_vertex_book();

  auto testString1 = std::string(1024, 'a');
  auto testString2 = std::string(127, 'b');
  auto testString3 = std::string(128, 'c');

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  auto tmp = nogdb::RecordDescriptor{};
  try {
    nogdb::Record r{};
    r.set("title", testString1).set("price", 1.0).set("pages", 10);
    nogdb::Vertex::create(txn, "books", r);
    r.set("title", testString2).set("price", 2.0).set("pages", 20);
    nogdb::Vertex::create(txn, "books", r);
    r.set("title", testString3).set("price", 3.0).set("pages", 30);
    nogdb::Vertex::create(txn, "books", r);

    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::get(txn, "books");
    for (auto const &r: res) {
      auto price = r.record.getReal("price");
      if (price == 1.0) {
        assert(r.record.getInt("pages") == 10);
        assert(r.record.getText("title") == testString1);
      } else if (price == 2.0) {
        assert(r.record.getInt("pages") == 20);
        assert(r.record.getText("title") == testString2);
      } else if (price == 3.0) {
        assert(r.record.getInt("pages") == 30);
        assert(r.record.getText("title") == testString3);
      } else {
        assert(false);
      }
    }

    res = nogdb::Vertex::get(txn, "books", nogdb::Condition("title").eq(testString1));
    ASSERT_SIZE(res, 1);
    assert(res[0].record.getInt("pages") == 10);

    res = nogdb::Vertex::get(txn, "books", nogdb::Condition("title").eq(testString2));
    ASSERT_SIZE(res, 1);
    assert(res[0].record.getInt("pages") == 20);

    res = nogdb::Vertex::get(txn, "books", nogdb::Condition("title").eq(testString3));
    ASSERT_SIZE(res, 1);
    assert(res[0].record.getInt("pages") == 30);

    txn.rollback();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_vertex_book();
}

void test_overwrite_basic_info() {
  init_vertex_book();

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto v1 = nogdb::Vertex::create(txn, "books", nogdb::Record{}.set("@className", "bookybooky").set("@recordId", "-1:-1"));
    auto v2 = nogdb::Vertex::create(txn, "books", nogdb::Record{});
    nogdb::Vertex::update(txn, v2, nogdb::Record{}.set("@className", "bookybookyss").set("@recordId", "-999:-999"));

    auto res = nogdb::Vertex::get(txn, "books");
    for(const auto &r: res) {
      assert(r.record.getClassName() == "books");
      assert(r.record.getText("@className") == "books");
    }

    auto res1 = nogdb::Vertex::get(txn, "books", nogdb::Condition("@className").eq("bookybooky"));
    ASSERT_SIZE(res1, 0);
    auto res2 = nogdb::Vertex::get(txn, "books", nogdb::Condition("@className").eq("books"));
    ASSERT_SIZE(res2, 2);

    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_vertex_book();
}

void test_standalone_vertex() {
  init_vertex_book();
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Record r{};
    auto v = nogdb::Vertex::create(txn, "books", r.set("title", "Intro to Linux"));
    auto res1 = nogdb::Vertex::getInEdge(txn, v);
    assert(res1.size() == 0);
    auto res2 = nogdb::Vertex::getOutEdge(txn, v);
    assert(res2.empty());
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();
  destroy_vertex_book();
}

void test_delete_vertex_with_edges() {
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
    auto e1 = nogdb::Edge::create(txn, "authors", v1_1, v2_1, r3);
    r3.set("time_used", 180U);
    auto e2 = nogdb::Edge::create(txn, "authors", v1_2, v2_1, r3);
    r3.set("time_used", 430U);
    auto e3 = nogdb::Edge::create(txn, "authors", v1_3, v2_2, r3);

    nogdb::Vertex::destroy(txn, v2_1);

    try {
      auto record = nogdb::DB::getRecord(txn, v2_1);
    } catch (const nogdb::Error &ex) {
      REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
    }
    auto record = nogdb::DB::getRecord(txn, v1_1);
    assert(!record.empty());
    record = nogdb::DB::getRecord(txn, v1_2);
    assert(!record.empty());
    try {
      auto record = nogdb::DB::getRecord(txn, e1);
    } catch (const nogdb::Error &ex) {
      REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
    }
    try {
      auto record = nogdb::DB::getRecord(txn, e2);
    } catch (const nogdb::Error &ex) {
      REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
    }
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_delete_all_vertices_with_edges() {
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
    auto e1 = nogdb::Edge::create(txn, "authors", v1_1, v2_1, r3);
    r3.set("time_used", 180U);
    auto e2 = nogdb::Edge::create(txn, "authors", v1_2, v2_1, r3);
    r3.set("time_used", 430U);
    auto e3 = nogdb::Edge::create(txn, "authors", v1_3, v2_2, r3);

    nogdb::Vertex::destroy(txn, v2_1);

    try {
      auto record = nogdb::DB::getRecord(txn, v2_1);
    } catch (const nogdb::Error &ex) {
      REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
    }
    auto record = nogdb::DB::getRecord(txn, v1_1);
    assert(!record.empty());
    record = nogdb::DB::getRecord(txn, v1_2);
    assert(!record.empty());
    try {
      auto record = nogdb::DB::getRecord(txn, e1);
    } catch (const nogdb::Error &ex) {
      REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
    }
    try {
      auto record = nogdb::DB::getRecord(txn, e2);
    } catch (const nogdb::Error &ex) {
      REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
    }
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_add_delete_prop_with_records() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "mytest", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "mytest", "prop1", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "mytest", "prop2", nogdb::PropertyType::INTEGER);
    nogdb::Property::add(txn, "mytest", "prop3", nogdb::PropertyType::REAL);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Record r{};
    r.set("prop1", "hello").set("prop2", 42).set("prop3", 4.2);
    auto v = nogdb::Vertex::create(txn, "mytest", r);
    auto res = nogdb::Vertex::get(txn, "mytest");
    assert(res[0].record.get("prop1").toText() == "hello");
    assert(res[0].record.get("prop2").toInt() == 42);
    assert(res[0].record.get("prop3").toReal() == 4.2);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Property::add(txn, "mytest", "prop4", nogdb::PropertyType::UNSIGNED_BIGINT);
    nogdb::Property::alter(txn, "mytest", "prop2", "prop22");
    nogdb::Property::remove(txn, "mytest", "prop3");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  auto res = nogdb::ResultSet{};
  try {
    res = nogdb::Vertex::get(txn, "mytest");
    assert(res[0].record.get("prop1").toText() == "hello");
    assert(res[0].record.get("prop22").toInt() == 42);
    assert(res[0].record.get("prop4").empty());
    assert(res[0].record.get("prop3").empty());
    assert(res[0].record.get("prop2").empty());
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto rec = res[0].record;
    rec.set("prop3", 42.42);
    nogdb::Vertex::update(txn, res[0].descriptor, rec);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto rec = res[0].record;
    rec.set("prop2", 4242);
    nogdb::Vertex::update(txn, res[0].descriptor, rec);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto rec = res[0].record;
    rec.set("prop4", 424242ULL);
    nogdb::Vertex::update(txn, res[0].descriptor, rec);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    res = nogdb::Vertex::get(txn, "mytest");
    assert(res[0].record.get("prop1").toText() == "hello");
    assert(res[0].record.get("prop22").toInt() == 42);
    assert(res[0].record.get("prop4").toBigIntU() == 424242ULL);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "mytest");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_alter_class_with_records() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "mytest", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "mytest", "prop1", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "mytest", "prop2", nogdb::PropertyType::INTEGER);
    nogdb::Property::add(txn, "mytest", "prop3", nogdb::PropertyType::REAL);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    auto v = nogdb::Vertex::create(txn, "mytest", nogdb::Record{}
        .set("prop1", "hello")
        .set("prop2", 42)
        .set("prop3", 4.2)
    );
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::alter(txn, "mytest", "mytest01");
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    auto res = nogdb::Vertex::get(txn, "mytest01");
    assert(res[0].record.get("prop1").toText() == "hello");
    assert(res[0].record.get("prop2").toInt() == 42);
    assert(res[0].record.get("prop3").toReal() == 4.2);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "mytest01");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_drop_class_with_relations() {
  nogdb::RecordDescriptor v1, v2, v3, v4, v5;
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "myvertex1", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "myvertex1", "prop", nogdb::PropertyType::TEXT);
    nogdb::Class::create(txn, "myvertex2", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "myvertex2", "prop", nogdb::PropertyType::TEXT);
    nogdb::Class::create(txn, "myedge1", nogdb::ClassType::EDGE);
    nogdb::Property::add(txn, "myedge1", "prop", nogdb::PropertyType::TEXT);
    nogdb::Class::create(txn, "myedge2", nogdb::ClassType::EDGE);
    nogdb::Property::add(txn, "myedge2", "prop", nogdb::PropertyType::TEXT);
    nogdb::Class::create(txn, "myedge3", nogdb::ClassType::EDGE);
    nogdb::Property::add(txn, "myedge3", "prop", nogdb::PropertyType::TEXT);

    v1 = nogdb::Vertex::create(txn, "myvertex1", nogdb::Record{}.set("prop", "a"));
    v2 = nogdb::Vertex::create(txn, "myvertex1", nogdb::Record{}.set("prop", "b"));
    v3 = nogdb::Vertex::create(txn, "myvertex1", nogdb::Record{}.set("prop", "c"));

    v4 = nogdb::Vertex::create(txn, "myvertex2", nogdb::Record{}.set("prop", "A"));
    v5 = nogdb::Vertex::create(txn, "myvertex2", nogdb::Record{}.set("prop", "B"));

    nogdb::Edge::create(txn, "myedge1", v1, v2);
    nogdb::Edge::create(txn, "myedge2", v1, v4);
    nogdb::Edge::create(txn, "myedge3", v1, v4);
    nogdb::Edge::create(txn, "myedge1", v2, v3);
    nogdb::Edge::create(txn, "myedge2", v2, v5);
    nogdb::Edge::create(txn, "myedge3", v2, v5);
    nogdb::Edge::create(txn, "myedge2", v3, v4);
    nogdb::Edge::create(txn, "myedge3", v3, v4);
    nogdb::Edge::create(txn, "myedge2", v3, v5);
    nogdb::Edge::create(txn, "myedge3", v3, v5);
    nogdb::Edge::create(txn, "myedge2", v4, v5);

    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "myedge3");
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    auto res = nogdb::Vertex::getOutEdge(txn, v1);
    ASSERT_SIZE(res, 2);
    res = nogdb::Vertex::getOutEdge(txn, v2);
    ASSERT_SIZE(res, 2);
    res = nogdb::Vertex::getOutEdge(txn, v3);
    ASSERT_SIZE(res, 2);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "myvertex1");
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    auto res = nogdb::Vertex::getInEdge(txn, v4);
    ASSERT_SIZE(res, 0);
    res = nogdb::Vertex::getAllEdge(txn, v4);
    ASSERT_SIZE(res, 1);
    res = nogdb::Vertex::getOutEdge(txn, v5);
    ASSERT_SIZE(res, 0);
    res = nogdb::Vertex::getAllEdge(txn, v5);
    ASSERT_SIZE(res, 1);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    auto res = nogdb::Edge::get(txn, "myedge1");
    ASSERT_SIZE(res, 0);
    res = nogdb::Edge::get(txn, "myedge2");
    ASSERT_SIZE(res, 1);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::DB::getClass(txn, "myvertex1");
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Class::drop(txn, "myedge1");
    nogdb::Class::drop(txn, "myedge2");
    nogdb::Class::drop(txn, "myvertex2");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_drop_and_find_extended_class() {
  nogdb::ClassDescriptor v3, v4;
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "vertex1", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "vertex1", "prop0", nogdb::PropertyType::UNSIGNED_INTEGER);
    nogdb::Property::add(txn, "vertex1", "prop1", nogdb::PropertyType::UNSIGNED_INTEGER);
    nogdb::Class::createExtend(txn, "vertex2", "vertex1");
    nogdb::Property::add(txn, "vertex2", "prop2", nogdb::PropertyType::INTEGER);
    v3 = nogdb::Class::createExtend(txn, "vertex3", "vertex2");
    nogdb::Property::add(txn, "vertex3", "prop3", nogdb::PropertyType::REAL);
    v4 = nogdb::Class::createExtend(txn, "vertex4", "vertex2");
    nogdb::Property::add(txn, "vertex4", "prop3", nogdb::PropertyType::TEXT);

    nogdb::Vertex::create(txn, "vertex3",
                          nogdb::Record{}.set("prop0", 0U).set("prop1", 1U).set("prop2", 1).set("prop3", 1.1));
    nogdb::Vertex::create(txn, "vertex4",
                          nogdb::Record{}.set("prop0", 0U).set("prop1", 1U).set("prop2", 1).set("prop3", "hello"));
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "vertex2");

    auto classDesc = nogdb::DB::getClass(txn, "vertex1");
    auto count = size_t{0};
    for (const auto &cdesc: nogdb::DB::getClasses(txn)) {
      if (cdesc.base == classDesc.id) {
        ++count;
        assert(cdesc.name == "vertex3" || cdesc.name == "vertex4");
      }
    }
    assert(count == 2);
    auto res = nogdb::DB::getClass(txn, "vertex3");
    assert(res.base == classDesc.id);
    res = nogdb::DB::getClass(txn, "vertex4");
    assert(res.base == classDesc.id);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    auto res = nogdb::Vertex::getExtend(txn, "vertex1");
    ASSERT_SIZE(res, 2);
    for (const auto &r: res) {
      assert(r.record.get("prop0").toIntU() == 0U);
      assert(r.record.get("prop1").toIntU() == 1U);
      assert(r.record.get("prop2").empty());
      if (r.descriptor.rid.first == v3.id) {
        assert(r.record.get("prop3").toReal() == 1.1);
      } else if (r.descriptor.rid.first == v4.id) {
        assert(r.record.get("prop3").toText() == "hello");
      } else {
        assert(false);
      }
    }
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    auto res = nogdb::Vertex::getExtend(txn, "vertex1", nogdb::Condition("prop0").eq(0U));
    ASSERT_SIZE(res, 2);
    res = nogdb::Vertex::getExtend(txn, "vertex3", nogdb::Condition("prop0").eq(0U));
    ASSERT_SIZE(res, 1);
    res = nogdb::Vertex::getExtend(txn, "vertex4", nogdb::Condition("prop0").eq(0U));
    ASSERT_SIZE(res, 1);
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Property::remove(txn, "vertex1", "prop0");
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
      auto res = nogdb::Vertex::get(txn, "vertex1", nogdb::Condition("prop0").eq(0U));
      ASSERT_SIZE(res, 0);
      txn.rollback();
    } catch (const nogdb::Error &ex) {
      std::cout << "\nError: " << ex.what() << std::endl;
      assert(false);
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
      auto res = nogdb::Vertex::get(txn, "vertex3", nogdb::Condition("prop0").eq(0U));
      ASSERT_SIZE(res, 0);
      txn.rollback();
    } catch (const nogdb::Error &ex) {
      std::cout << "\nError: " << ex.what() << std::endl;
      assert(false);
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
      auto res = nogdb::Vertex::get(txn, "vertex4", nogdb::Condition("prop0").eq(0U));
      ASSERT_SIZE(res, 0);
      txn.rollback();
    } catch (const nogdb::Error &ex) {
      std::cout << "\nError: " << ex.what() << std::endl;
      assert(false);
    }
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "vertex5", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "vertex5", "prop1", nogdb::PropertyType::TEXT);
    nogdb::Class::createExtend(txn, "vertex6", "vertex5");

    nogdb::Vertex::create(txn, "vertex6", nogdb::Record{}.set("prop1", "hello"));
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "vertex5");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Vertex::create(txn, "vertex6", nogdb::Record{}.set("prop1", "hello"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::get(txn, "vertex6");
    ASSERT_SIZE(res, 1);
    assert(res[0].record.get("prop1").empty());
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::get(txn, "vertex6", nogdb::Condition("prop1").eq("hello"));
    ASSERT_SIZE(res, 0);
    txn.rollback();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "vertex1");
    nogdb::Class::drop(txn, "vertex3");
    nogdb::Class::drop(txn, "vertex4");
    nogdb::Class::drop(txn, "vertex6");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_conflict_property() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "vertex1", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "vertex1", "prop1", nogdb::PropertyType::INTEGER);
    nogdb::Class::createExtend(txn, "vertex2", "vertex1");
    nogdb::Property::add(txn, "vertex2", "prop2", nogdb::PropertyType::INTEGER);
    nogdb::Class::createExtend(txn, "vertex3", "vertex1");
    nogdb::Property::add(txn, "vertex3", "prop2", nogdb::PropertyType::TEXT);
    nogdb::Class::createExtend(txn, "vertex4", "vertex1");
    nogdb::Property::add(txn, "vertex4", "prop2", nogdb::PropertyType::REAL);

    nogdb::Vertex::create(txn, "vertex2", nogdb::Record{}.set("prop2", 97));
    nogdb::Vertex::create(txn, "vertex3", nogdb::Record{}.set("prop2", "a"));
    nogdb::Vertex::create(txn, "vertex4", nogdb::Record{}.set("prop2", 97.97));
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    auto res = nogdb::Vertex::getExtend(txn, "vertex1", nogdb::Condition("prop2").eq(97));
    ASSERT_SIZE(res, 1);
    ASSERT_EQ(res[0].record.getInt("prop2"), 97);
    res = nogdb::Vertex::getExtend(txn, "vertex1", nogdb::Condition("prop2").eq("a"));
    ASSERT_SIZE(res, 2);
    res = nogdb::Vertex::getExtend(txn, "vertex1", nogdb::Condition("prop2").eq(97.97));
    ASSERT_SIZE(res, 1);
    ASSERT_EQ(res[0].record.getReal("prop2"), 97.97);
    txn.rollback();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "vertex1");
    nogdb::Class::drop(txn, "vertex2");
    nogdb::Class::drop(txn, "vertex3");
    nogdb::Class::drop(txn, "vertex4");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}



