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

#include "functest.h"
#include "test_prepare.h"

void test_get_set_empty_value() {
  init_vertex_person();
  init_edge_know();
  auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
  try {
    nogdb::Record r_blank_name{};
    r_blank_name.set("name", "");
    auto rdesc1 = txn.addVertex("persons", r_blank_name);
    auto r1 = txn.fetchRecord(rdesc1);
    assert(r1.get("name").toText() == "");
    assert(r1.get("name").empty());

    auto rdesc2 = txn.addVertex("persons");
    auto r2 = txn.fetchRecord(rdesc2);
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
  auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
  auto tmp = nogdb::RecordDescriptor{};
  try {
    nogdb::Record r{};
    r.set("title", "Lion King").set("price", 100.0).set("pages", 320);
    auto rdesc1 = txn.addVertex("books", r);
    r.set("title", "Tarzan").set("price", 60.0).set("pages", 360);
    auto rdesc2 = txn.addVertex("books", r);
    tmp = rdesc2;
    txn.remove(rdesc1);

    try {
      auto res = txn.fetchRecord(rdesc1);
    } catch (const nogdb::Error &ex) {
      REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
    }
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  destroy_vertex_book();

  txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
  try {
    auto res = txn.fetchRecord(tmp);
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

  auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
  auto tmp = nogdb::RecordDescriptor{};
  try {
    nogdb::Record r{};
    r.set("title", testString1).set("price", 1.0).set("pages", 10);
    txn.addVertex("books", r);
    r.set("title", testString2).set("price", 2.0).set("pages", 20);
    txn.addVertex("books", r);
    r.set("title", testString3).set("price", 3.0).set("pages", 30);
    txn.addVertex("books", r);

    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
  try {
    auto res = txn.find("books").get();
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

    res = txn.find("books").where(nogdb::Condition("title").eq(testString1)).get();
    ASSERT_SIZE(res, 1);
    assert(res[0].record.getInt("pages") == 10);

    res = txn.find("books").where(nogdb::Condition("title").eq(testString2)).get();
    ASSERT_SIZE(res, 1);
    assert(res[0].record.getInt("pages") == 20);

    res = txn.find("books").where(nogdb::Condition("title").eq(testString3)).get();
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

  auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
  try {
    auto v1 = txn.addVertex("books", nogdb::Record{}.set("@className", "bookybooky").set("@recordId", "-1:-1"));
    auto v2 = txn.addVertex("books", nogdb::Record{});
    txn.update(v2, nogdb::Record{}.set("@className", "bookybookyss").set("@recordId", "-999:-999"));

    auto res = txn.find("books").get();
    for(const auto &r: res) {
      assert(r.record.getClassName() == "books");
      assert(r.record.getText("@className") == "books");
    }

    auto res1 = txn.find("books").where(nogdb::Condition("@className").eq("bookybooky")).get();
    ASSERT_SIZE(res1, 0);
    auto res2 = txn.find("books").where(nogdb::Condition("@className").eq("books")).get();
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
  auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
  try {
    nogdb::Record r{};
    auto v = txn.addVertex("books", r.set("title", "Intro to Linux"));
    auto res1 = txn.findInEdge(v).get();
    assert(res1.size() == 0);
    auto res2 = txn.findOutEdge(v).get();
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
  auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
  try {
    nogdb::Record r1{}, r2{}, r3{};
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
    auto e1 = txn.addEdge("authors", v1_1, v2_1, r3);
    r3.set("time_used", 180U);
    auto e2 = txn.addEdge("authors", v1_2, v2_1, r3);
    r3.set("time_used", 430U);
    auto e3 = txn.addEdge("authors", v1_3, v2_2, r3);

    txn.remove(v2_1);

    try {
      auto record = txn.fetchRecord(v2_1);
    } catch (const nogdb::Error &ex) {
      REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
    }
    auto record = txn.fetchRecord(v1_1);
    assert(!record.empty());
    record = txn.fetchRecord(v1_2);
    assert(!record.empty());
    try {
      auto record = txn.fetchRecord(e1);
    } catch (const nogdb::Error &ex) {
      REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
    }
    try {
      auto record = txn.fetchRecord(e2);
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

  auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
  try {
    nogdb::Record r1{}, r2{}, r3{};
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
    auto e1 = txn.addEdge("authors", v1_1, v2_1, r3);
    r3.set("time_used", 180U);
    auto e2 = txn.addEdge("authors", v1_2, v2_1, r3);
    r3.set("time_used", 430U);
    auto e3 = txn.addEdge("authors", v1_3, v2_2, r3);

    txn.remove(v2_1);

    try {
      auto record = txn.fetchRecord(v2_1);
    } catch (const nogdb::Error &ex) {
      REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
    }
    auto record = txn.fetchRecord(v1_1);
    assert(!record.empty());
    record = txn.fetchRecord(v1_2);
    assert(!record.empty());
    try {
      auto record = txn.fetchRecord(e1);
    } catch (const nogdb::Error &ex) {
      REQUIRE(ex, NOGDB_CTX_NOEXST_RECORD, "NOGDB_CTX_NOEXST_RECORD");
    }
    try {
      auto record = txn.fetchRecord(e2);
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
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.addClass("mytest", nogdb::ClassType::VERTEX);
    txn.addProperty("mytest", "prop1", nogdb::PropertyType::TEXT);
    txn.addProperty("mytest", "prop2", nogdb::PropertyType::INTEGER);
    txn.addProperty("mytest", "prop3", nogdb::PropertyType::REAL);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    nogdb::Record r{};
    r.set("prop1", "hello").set("prop2", 42).set("prop3", 4.2);
    auto v = txn.addVertex("mytest", r);
    auto res = txn.find("mytest").get();
    assert(res[0].record.get("prop1").toText() == "hello");
    assert(res[0].record.get("prop2").toInt() == 42);
    assert(res[0].record.get("prop3").toReal() == 4.2);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.addProperty("mytest", "prop4", nogdb::PropertyType::UNSIGNED_BIGINT);
    txn.renameProperty("mytest", "prop2", "prop22");
    txn.dropProperty("mytest", "prop3");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
  auto res = nogdb::ResultSet{};
  try {
    res = txn.find("mytest").get();
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

  txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
  try {
    auto rec = res[0].record;
    rec.set("prop3", 42.42);
    txn.update(res[0].descriptor, rec);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
  try {
    auto rec = res[0].record;
    rec.set("prop2", 4242);
    txn.update(res[0].descriptor, rec);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
  try {
    auto rec = res[0].record;
    rec.set("prop4", 424242ULL);
    txn.update(res[0].descriptor, rec);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
  try {
    res = txn.find("mytest").get();
    assert(res[0].record.get("prop1").toText() == "hello");
    assert(res[0].record.get("prop22").toInt() == 42);
    assert(res[0].record.get("prop4").toBigIntU() == 424242ULL);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.dropClass("mytest");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_alter_class_with_records() {
  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.addClass("mytest", nogdb::ClassType::VERTEX);
    txn.addProperty("mytest", "prop1", nogdb::PropertyType::TEXT);
    txn.addProperty("mytest", "prop2", nogdb::PropertyType::INTEGER);
    txn.addProperty("mytest", "prop3", nogdb::PropertyType::REAL);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    auto v = txn.addVertex("mytest", nogdb::Record{}
        .set("prop1", "hello")
        .set("prop2", 42)
        .set("prop3", 4.2)
    );
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.renameClass("mytest", "mytest01");
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find("mytest01").get();
    assert(res[0].record.get("prop1").toText() == "hello");
    assert(res[0].record.get("prop2").toInt() == 42);
    assert(res[0].record.get("prop3").toReal() == 4.2);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.dropClass("mytest01");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_drop_class_with_relations() {
  nogdb::RecordDescriptor v1, v2, v3, v4, v5;
  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.addClass("myvertex1", nogdb::ClassType::VERTEX);
    txn.addProperty("myvertex1", "prop", nogdb::PropertyType::TEXT);
    txn.addClass("myvertex2", nogdb::ClassType::VERTEX);
    txn.addProperty("myvertex2", "prop", nogdb::PropertyType::TEXT);
    txn.addClass("myedge1", nogdb::ClassType::EDGE);
    txn.addProperty("myedge1", "prop", nogdb::PropertyType::TEXT);
    txn.addClass("myedge2", nogdb::ClassType::EDGE);
    txn.addProperty("myedge2", "prop", nogdb::PropertyType::TEXT);
    txn.addClass("myedge3", nogdb::ClassType::EDGE);
    txn.addProperty("myedge3", "prop", nogdb::PropertyType::TEXT);

    v1 = txn.addVertex("myvertex1", nogdb::Record{}.set("prop", "a"));
    v2 = txn.addVertex("myvertex1", nogdb::Record{}.set("prop", "b"));
    v3 = txn.addVertex("myvertex1", nogdb::Record{}.set("prop", "c"));

    v4 = txn.addVertex("myvertex2", nogdb::Record{}.set("prop", "A"));
    v5 = txn.addVertex("myvertex2", nogdb::Record{}.set("prop", "B"));

    txn.addEdge("myedge1", v1, v2);
    txn.addEdge("myedge2", v1, v4);
    txn.addEdge("myedge3", v1, v4);
    txn.addEdge("myedge1", v2, v3);
    txn.addEdge("myedge2", v2, v5);
    txn.addEdge("myedge3", v2, v5);
    txn.addEdge("myedge2", v3, v4);
    txn.addEdge("myedge3", v3, v4);
    txn.addEdge("myedge2", v3, v5);
    txn.addEdge("myedge3", v3, v5);
    txn.addEdge("myedge2", v4, v5);

    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.dropClass("myedge3");
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.findOutEdge(v1).get();
    ASSERT_SIZE(res, 2);
    res = txn.findOutEdge(v2).get();
    ASSERT_SIZE(res, 2);
    res = txn.findOutEdge(v3).get();
    ASSERT_SIZE(res, 2);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.dropClass("myvertex1");
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.findInEdge(v4).get();
    ASSERT_SIZE(res, 0);
    res = txn.findEdge(v4).get();
    ASSERT_SIZE(res, 1);
    res = txn.findOutEdge(v5).get();
    ASSERT_SIZE(res, 0);
    res = txn.findEdge(v5).get();
    ASSERT_SIZE(res, 1);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find("myedge1").get();
    ASSERT_SIZE(res, 0);
    res = txn.find("myedge2").get();
    ASSERT_SIZE(res, 1);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
  try {
    auto res = txn.getClass("myvertex1");
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
  try {
    txn.dropClass("myedge1");
    txn.dropClass("myedge2");
    txn.dropClass("myvertex2");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_drop_and_find_extended_class() {
  nogdb::ClassDescriptor v3, v4;
  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.addClass("vertex1", nogdb::ClassType::VERTEX);
    txn.addProperty("vertex1", "prop0", nogdb::PropertyType::UNSIGNED_INTEGER);
    txn.addProperty("vertex1", "prop1", nogdb::PropertyType::UNSIGNED_INTEGER);
    txn.addSubClassOf("vertex1", "vertex2");
    txn.addProperty("vertex2", "prop2", nogdb::PropertyType::INTEGER);
    v3 = txn.addSubClassOf("vertex2", "vertex3");
    txn.addProperty("vertex3", "prop3", nogdb::PropertyType::REAL);
    v4 = txn.addSubClassOf("vertex2", "vertex4");
    txn.addProperty("vertex4", "prop3", nogdb::PropertyType::TEXT);

    txn.addVertex("vertex3",
                          nogdb::Record{}.set("prop0", 0U).set("prop1", 1U).set("prop2", 1).set("prop3", 1.1));
    txn.addVertex("vertex4",
                          nogdb::Record{}.set("prop0", 0U).set("prop1", 1U).set("prop2", 1).set("prop3", "hello"));
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.dropClass("vertex2");

    auto classDesc = txn.getClass("vertex1");
    auto count = size_t{0};
    for (const auto &cdesc: txn.getClasses()) {
      if (cdesc.base == classDesc.id) {
        ++count;
        assert(cdesc.name == "vertex3" || cdesc.name == "vertex4");
      }
    }
    assert(count == 2);
    auto res = txn.getClass("vertex3");
    assert(res.base == classDesc.id);
    res = txn.getClass("vertex4");
    assert(res.base == classDesc.id);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.findSubClassOf("vertex1").get();
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
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.findSubClassOf("vertex1").where(nogdb::Condition("prop0").eq(0U)).get();
    ASSERT_SIZE(res, 2);
    res = txn.findSubClassOf("vertex3").where(nogdb::Condition("prop0").eq(0U)).get();
    ASSERT_SIZE(res, 1);
    res = txn.findSubClassOf("vertex4").where(nogdb::Condition("prop0").eq(0U)).get();
    ASSERT_SIZE(res, 1);
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.dropProperty("vertex1", "prop0");
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
      auto res = txn.find("vertex1").where(nogdb::Condition("prop0").eq(0U)).get();
      ASSERT_SIZE(res, 0);
      txn.rollback();
    } catch (const nogdb::Error &ex) {
      std::cout << "\nError: " << ex.what() << std::endl;
      assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
      auto res = txn.find("vertex3").where(nogdb::Condition("prop0").eq(0U)).get();
      ASSERT_SIZE(res, 0);
      txn.rollback();
    } catch (const nogdb::Error &ex) {
      std::cout << "\nError: " << ex.what() << std::endl;
      assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
      auto res = txn.find("vertex4").where(nogdb::Condition("prop0").eq(0U)).get();
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
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.addClass("vertex5", nogdb::ClassType::VERTEX);
    txn.addProperty("vertex5", "prop1", nogdb::PropertyType::TEXT);
    txn.addSubClassOf("vertex5", "vertex6");

    txn.addVertex("vertex6", nogdb::Record{}.set("prop1", "hello"));
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.dropClass("vertex5");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
  try {
    txn.addVertex("vertex6", nogdb::Record{}.set("prop1", "hello"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
  try {
    auto res = txn.find("vertex6").get();
    ASSERT_SIZE(res, 1);
    assert(res[0].record.get("prop1").empty());
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
  try {
    auto res = txn.find("vertex6").where(nogdb::Condition("prop1").eq("hello")).get();
    ASSERT_SIZE(res, 0);
    txn.rollback();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.dropClass("vertex1");
    txn.dropClass("vertex3");
    txn.dropClass("vertex4");
    txn.dropClass("vertex6");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_conflict_property() {
  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.addClass("vertex1", nogdb::ClassType::VERTEX);
    txn.addProperty("vertex1", "prop1", nogdb::PropertyType::INTEGER);
    txn.addSubClassOf("vertex1", "vertex2");
    txn.addProperty("vertex2", "prop2", nogdb::PropertyType::INTEGER);
    txn.addSubClassOf("vertex1", "vertex3");
    txn.addProperty("vertex3", "prop2", nogdb::PropertyType::TEXT);
    txn.addSubClassOf("vertex1", "vertex4");
    txn.addProperty("vertex4", "prop2", nogdb::PropertyType::REAL);

    txn.addVertex("vertex2", nogdb::Record{}.set("prop2", 97));
    txn.addVertex("vertex3", nogdb::Record{}.set("prop2", "a"));
    txn.addVertex("vertex4", nogdb::Record{}.set("prop2", 97.97));
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.findSubClassOf("vertex1").where(nogdb::Condition("prop2").eq(97)).get();
    ASSERT_SIZE(res, 1);
    ASSERT_EQ(res[0].record.getInt("prop2"), 97);
    res = txn.findSubClassOf("vertex1").where(nogdb::Condition("prop2").eq("a")).get();
    ASSERT_SIZE(res, 2);
    res = txn.findSubClassOf("vertex1").where(nogdb::Condition("prop2").eq(97.97)).get();
    ASSERT_SIZE(res, 1);
    ASSERT_EQ(res[0].record.getReal("prop2"), 97.97);
    txn.rollback();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.dropClass("vertex1");
    txn.dropClass("vertex2");
    txn.dropClass("vertex3");
    txn.dropClass("vertex4");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}



