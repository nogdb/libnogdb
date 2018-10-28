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
#include <climits>
#include <vector>

void test_create_edges() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  nogdb::RecordDescriptor v1_1{}, v1_2{}, v2{};
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Record r1{}, r2{};
    r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
    v1_1 = nogdb::Vertex::create(txn, "books", r1);
    r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
    v1_2 = nogdb::Vertex::create(txn, "books", r1);

    r2.set("name", "J.K. Rowlings").set("age", 32);
    v2 = nogdb::Vertex::create(txn, "persons", r2);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Record r{};
    r.set("time_used", 365U);
    nogdb::Edge::create(txn, "authors", v1_1, v2, r);
    r.set("time_used", 180U);
    nogdb::Edge::create(txn, "authors", v1_1, v2, r);

    auto v1 = nogdb::DB::getRecord(txn, v1_1);
    //assert(v1.getVersion() == 2ULL);

    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }


  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_create_invalid_edge() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  auto vb = std::vector<nogdb::RecordDescriptor>{};
  auto vp = std::vector<nogdb::RecordDescriptor>{};
  try {
    nogdb::Record r1{}, r2{};
    r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
    vb.push_back(nogdb::Vertex::create(txn, "books", r1));
    r1.set("title", "Fantastic Beasts").set("pages", 342).set("price", 21.0);
    vb.push_back(nogdb::Vertex::create(txn, "books", r1));
    r1.set("title", "Percy Jackson").set("pages", 800).set("price", 32.4);
    vb.push_back(nogdb::Vertex::create(txn, "books", r1));

    nogdb::Vertex::destroy(txn, vb[1]);
    nogdb::Vertex::destroy(txn, vb[2]);

    r2.set("name", "J.K. Rowlings").set("age", 32);
    vp.push_back(nogdb::Vertex::create(txn, "persons", r2));
    r2.set("name", "David Lahm").set("age", 29);
    vp.push_back(nogdb::Vertex::create(txn, "persons", r2));

    nogdb::Vertex::destroy(txn, vp[1]);

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Record r{};
    r.set("name", "ABC").set("age", 20);
    nogdb::Edge::create(txn, "books", vb[0], vp[0], r);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Record r{};
    r.set("profits", 50.0);
    nogdb::Edge::create(txn, "authors", vb[0], vp[0], r);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Record r{};
    r.set("name", "Nanmee");
    nogdb::Edge::create(txn, "publisher", vb[0], vp[0], r);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Record r{};
    r.set("time_used", 100U);
    nogdb::Edge::create(txn, "authors", vb[1], vp[0], r);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_SRC, "NOGDB_GRAPH_NOEXST_SRC");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Record r{};
    r.set("time_used", 100U);
    nogdb::Edge::create(txn, "authors", vb[0], vp[1], r);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_DST, "NOGDB_GRAPH_NOEXST_DST");
  }

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_get_edge() {
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

    auto res = nogdb::Edge::get(txn, "authors");
    auto count = 0;
    for (const auto &it: res) {
      auto &record = it.record;
      assert(record.getText("@className") == "authors");
      assert(record.getBigIntU("@version") == 1UL);
      if (count == 0) {
        assert(record.get("time_used").toIntU() == 365U);
        assert(record.getText("@recordId") == nogdb::rid2str(e1.rid));
      } else if (count == 1) {
        assert(record.get("time_used").toIntU() == 180U);
        assert(record.getText("@recordId") == nogdb::rid2str(e2.rid));
      } else if (count == 2) {
        assert(record.get("time_used").toIntU() == 430U);
        assert(record.getText("@recordId") == nogdb::rid2str(e3.rid));
      }
      count++;
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

void test_get_invalid_edges() {
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

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::get(txn, "author");
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::get(txn, "persons");
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_get_vertex_src() {
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

    auto res = nogdb::Edge::get(txn, "authors");
    auto count = 0;
    for (const auto &it: res) {
      //auto &record = it.record;
      if (count == 0) {
        auto src_vertex = nogdb::Edge::getSrc(txn, it.descriptor);
        assert(src_vertex.record.get("title").toText() == "Harry Potter");
      } else if (count == 1) {
        auto src_vertex = nogdb::Edge::getSrc(txn, it.descriptor);
        assert(src_vertex.record.get("title").toText() == "Fantastic Beasts");
      } else if (count == 2) {
        auto src_vertex = nogdb::Edge::getSrc(txn, it.descriptor);
        assert(src_vertex.record.get("title").toText() == "Percy Jackson");
      }
      count++;
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

void test_get_vertex_dst() {
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

    auto res = nogdb::Edge::get(txn, "authors");
    auto count = 0;
    for (const auto &it: res) {
      //auto &record = it.record;
      if (count == 0) {
        auto dst_vertex = nogdb::Edge::getDst(txn, it.descriptor);
        assert(dst_vertex.record.get("name").toText() == "J.K. Rowlings");
      } else if (count == 1) {
        auto dst_vertex = nogdb::Edge::getDst(txn, it.descriptor);
        assert(dst_vertex.record.get("name").toText() == "J.K. Rowlings");
      } else if (count == 2) {
        auto dst_vertex = nogdb::Edge::getDst(txn, it.descriptor);
        assert(dst_vertex.record.get("name").toText() == "David Lahm");
      }
      count++;
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

void test_get_vertex_all() {
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

    auto res = nogdb::Edge::get(txn, "authors");
    auto count = 0;
    for (const auto &it: res) {
      //auto &record = it.record;
      if (count == 0) {
        auto vertices = nogdb::Edge::getSrcDst(txn, it.descriptor);
        assert(vertices[0].record.get("title").toText() == "Harry Potter");
        assert(vertices[1].record.get("name").toText() == "J.K. Rowlings");
      } else if (count == 1) {
        auto vertices = nogdb::Edge::getSrcDst(txn, it.descriptor);
        assert(vertices[0].record.get("title").toText() == "Fantastic Beasts");
        assert(vertices[1].record.get("name").toText() == "J.K. Rowlings");
      } else if (count == 2) {
        auto vertices = nogdb::Edge::getSrcDst(txn, it.descriptor);
        assert(vertices[0].record.get("title").toText() == "Percy Jackson");
        assert(vertices[1].record.get("name").toText() == "David Lahm");
      }
      count++;
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

void test_get_invalid_vertex_src() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  nogdb::RecordDescriptor v1_1{}, v1_2{}, v1_3{}, v2_1{}, v2_2{};
  nogdb::RecordDescriptor e1{}, e2{}, e3{};
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
    auto tmp = e1;
    tmp.rid.first = 9999U;
    auto res = nogdb::Edge::getSrc(txn, tmp);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = v1_1;
    auto res = nogdb::Edge::getSrc(txn, tmp);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = e1;
    tmp.rid.second = -1;
    auto res = nogdb::Edge::getSrc(txn, tmp);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_EDGE, "NOGDB_GRAPH_NOEXST_EDGE");
  }

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_get_invalid_vertex_dst() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  nogdb::RecordDescriptor v1_1{}, v1_2{}, v1_3{}, v2_1{}, v2_2{};
  nogdb::RecordDescriptor e1{}, e2{}, e3{};
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
    auto tmp = e1;
    tmp.rid.first = 9999U;
    auto res = nogdb::Edge::getDst(txn, tmp);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = v1_1;
    auto res = nogdb::Edge::getDst(txn, tmp);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = e1;
    tmp.rid.second = -1;
    auto res = nogdb::Edge::getDst(txn, tmp);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_EDGE, "NOGDB_GRAPH_NOEXST_EDGE");
  }

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_get_invalid_vertex_all() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  nogdb::RecordDescriptor v1_1{}, v1_2{}, v1_3{}, v2_1{}, v2_2{};
  nogdb::RecordDescriptor e1{}, e2{}, e3{};
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
    auto tmp = e1;
    tmp.rid.first = 9999U;
    auto res = nogdb::Edge::getSrcDst(txn, tmp);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = v1_1;
    auto res = nogdb::Edge::getSrcDst(txn, tmp);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto tmp = e1;
    tmp.rid.second = -1;
    auto res = nogdb::Edge::getSrcDst(txn, tmp);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_EDGE, "NOGDB_GRAPH_NOEXST_EDGE");
  }

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_update_edge() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Record r1{}, r2{}, r3{};
    r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
    auto v1 = nogdb::Vertex::create(txn, "books", r1);
    r2.set("name", "J.K. Rowlings").set("age", 32);
    auto v2 = nogdb::Vertex::create(txn, "persons", r2);
    r3.set("time_used", 365U);
    auto e1 = nogdb::Edge::create(txn, "authors", v1, v2, r3);

    auto rec_book = nogdb::Vertex::get(txn, "books")[0].record;
    //assert(rec_book.getVersion() == 1ULL);

    auto rec_person = nogdb::Vertex::get(txn, "persons")[0].record;
    //assert(rec_person.getVersion() == 1ULL);

    auto record = nogdb::DB::getRecord(txn, e1);
    assert(record.get("time_used").toIntU() == 365U);

    r3.set("time_used", 400U);
    nogdb::Edge::update(txn, e1, r3);
    auto res = nogdb::Edge::get(txn, "authors");
    assert(res[0].record.get("time_used").toIntU() == 400U);
    assert(res[0].record.getText("@className") == "authors");
    assert(res[0].record.getText("@recordId") == nogdb::rid2str(e1.rid));
    assert(res[0].record.getBigIntU("@version") == 1ULL);
    //assert(res[0].record.getVersion() == 1ULL);

    // update 10 times
    for (size_t i = 0; i < 10; ++i) {

      res[0].record.set("time_used", 1000U);

      //assert(res[0].record.getVersion() == 1ULL);
      nogdb::Edge::update(txn, res[0].descriptor, res[0].record);
      //assert(res[0].record.getVersion() == 1ULL);
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

void test_update_invalid_edge() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  nogdb::RecordDescriptor v1{}, v2{}, e1{};
  try {
    nogdb::Record r1{}, r2{}, r3{};
    r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
    v1 = nogdb::Vertex::create(txn, "books", r1);
    r2.set("name", "J.K. Rowlings").set("age", 32);
    v2 = nogdb::Vertex::create(txn, "persons", r2);
    r3.set("time_used", 365U);
    e1 = nogdb::Edge::create(txn, "authors", v1, v2, r3);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto r3 = nogdb::Record{};
    r3.set("time_used", 400U);
    auto tmp = e1;
    tmp.rid.second = -1;
    nogdb::Edge::update(txn, tmp, r3);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_EDGE, "NOGDB_GRAPH_NOEXST_EDGE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto r3 = nogdb::Record{};
    r3.set("time_used", 400U);
    auto tmp = e1;
    tmp.rid.first = 9999U;
    nogdb::Edge::update(txn, tmp, r3);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto r3 = nogdb::Record{};
    r3.set("time_used", 400U);
    auto tmp = v1;
    nogdb::Edge::update(txn, tmp, r3);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto r3 = nogdb::Record{};
    r3.set("time_use", 400U);
    nogdb::Edge::update(txn, e1, r3);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_update_vertex_src() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  nogdb::RecordDescriptor v1{}, v2{}, e1{};
  try {
    nogdb::Record r1{}, r2{}, r3{};
    r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
    v1 = nogdb::Vertex::create(txn, "books", r1);
    r2.set("name", "J.K. Rowlings").set("age", 32);
    v2 = nogdb::Vertex::create(txn, "persons", r2);
    r3.set("time_used", 365U);
    e1 = nogdb::Edge::create(txn, "authors", v1, v2, r3);

    auto tmp1 = nogdb::Edge::getSrc(txn, e1);
    auto tmp2 = nogdb::Edge::getDst(txn, e1);
    auto tmp3 = nogdb::Vertex::getInEdge(txn, v2);
    auto tmp4 = nogdb::Vertex::getOutEdge(txn, v1);

    assert(tmp1.descriptor.rid == v1.rid);
    assert(tmp2.descriptor.rid == v2.rid);
    assert(tmp3.size() == 1);
    assert(tmp3[0].descriptor.rid == e1.rid);
    assert(tmp4.size() == 1);
    assert(tmp4[0].descriptor.rid == e1.rid);

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    nogdb::Edge::updateSrc(txn, e1, v2);

    auto tmp1 = nogdb::Edge::getSrc(txn, e1);
    auto tmp2 = nogdb::Edge::getDst(txn, e1);
    auto tmp3 = nogdb::Vertex::getInEdge(txn, v1);
    auto tmp4 = nogdb::Vertex::getOutEdge(txn, v2);
    auto tmp5 = nogdb::Vertex::getOutEdge(txn, v1);
    auto tmp6 = nogdb::Vertex::getInEdge(txn, v2);

    assert(tmp1.descriptor.rid == v2.rid);
    assert(tmp2.descriptor.rid == v2.rid);
    assert(tmp3.size() == 0);
    assert(tmp4.size() == 1);
    assert(tmp4[0].descriptor.rid == e1.rid);
    assert(tmp5.size() == 0);
    assert(tmp6.size() == 1);
    assert(tmp6[0].descriptor.rid == e1.rid);

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_update_vertex_dst() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  nogdb::RecordDescriptor v1{}, v2{}, e1{};
  try {
    nogdb::Record r1{}, r2{}, r3{};
    r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
    v1 = nogdb::Vertex::create(txn, "books", r1);
    r2.set("name", "J.K. Rowlings").set("age", 32);
    v2 = nogdb::Vertex::create(txn, "persons", r2);
    r3.set("time_used", 365U);
    e1 = nogdb::Edge::create(txn, "authors", v1, v2, r3);

    auto tmp1 = nogdb::Edge::getSrc(txn, e1);
    auto tmp2 = nogdb::Edge::getDst(txn, e1);
    auto tmp3 = nogdb::Vertex::getInEdge(txn, v2);
    auto tmp4 = nogdb::Vertex::getOutEdge(txn, v1);

    assert(tmp1.descriptor.rid == v1.rid);
    assert(tmp2.descriptor.rid == v2.rid);
    assert(tmp3.size() == 1);
    assert(tmp3[0].descriptor.rid == e1.rid);
    assert(tmp4.size() == 1);
    assert(tmp4[0].descriptor.rid == e1.rid);

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    nogdb::Edge::updateDst(txn, e1, v1);

    auto tmp1 = nogdb::Edge::getSrc(txn, e1);
    auto tmp2 = nogdb::Edge::getDst(txn, e1);
    auto tmp3 = nogdb::Vertex::getInEdge(txn, v1);
    auto tmp4 = nogdb::Vertex::getOutEdge(txn, v2);
    auto tmp5 = nogdb::Vertex::getOutEdge(txn, v1);
    auto tmp6 = nogdb::Vertex::getInEdge(txn, v2);

    assert(tmp1.descriptor.rid == v1.rid);
    assert(tmp2.descriptor.rid == v1.rid);
    assert(tmp3.size() == 1);
    assert(tmp3[0].descriptor.rid == e1.rid);
    assert(tmp4.size() == 0);
    assert(tmp5.size() == 1);
    assert(tmp5[0].descriptor.rid == e1.rid);
    assert(tmp6.size() == 0);

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}


void test_update_invalid_edge_src() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  nogdb::RecordDescriptor v1{}, v2{}, e1{};
  try {
    nogdb::Record r1{}, r2{}, r3{};
    r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
    v1 = nogdb::Vertex::create(txn, "books", r1);
    r2.set("name", "J.K. Rowlings").set("age", 32);
    v2 = nogdb::Vertex::create(txn, "persons", r2);
    r3.set("time_used", 365U);
    e1 = nogdb::Edge::create(txn, "authors", v1, v2, r3);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto tmp = e1;
    tmp.rid.second = -1;
    nogdb::Edge::updateSrc(txn, tmp, v1);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_EDGE, "NOGDB_GRAPH_NOEXST_EDGE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto tmp = e1;
    tmp.rid.first = 9999U;
    nogdb::Edge::updateSrc(txn, tmp, v1);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto tmp = v1;
    nogdb::Edge::updateSrc(txn, tmp, v1);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto tmp = v1;
    tmp.rid.second = -1;
    nogdb::Edge::updateSrc(txn, e1, tmp);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_SRC, "NOGDB_GRAPH_NOEXST_SRC");
  }

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_update_invalid_edge_dst() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  nogdb::RecordDescriptor v1{}, v2{}, e1{};
  try {
    nogdb::Record r1{}, r2{}, r3{};
    r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
    v1 = nogdb::Vertex::create(txn, "books", r1);
    r2.set("name", "J.K. Rowlings").set("age", 32);
    v2 = nogdb::Vertex::create(txn, "persons", r2);
    r3.set("time_used", 365U);
    e1 = nogdb::Edge::create(txn, "authors", v1, v2, r3);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto tmp = e1;
    tmp.rid.second = -1;
    nogdb::Edge::updateDst(txn, tmp, v1);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_EDGE, "NOGDB_GRAPH_NOEXST_EDGE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto tmp = e1;
    tmp.rid.first = 9999U;
    nogdb::Edge::updateDst(txn, tmp, v1);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto tmp = v1;
    nogdb::Edge::updateDst(txn, tmp, v1);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto tmp = v1;
    tmp.rid.second = -1;
    nogdb::Edge::updateDst(txn, e1, tmp);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_GRAPH_NOEXST_DST, "NOGDB_GRAPH_NOEXST_DST");
  }

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_delete_edge() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Record r1{}, r2{}, r3{};
    r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
    auto v1 = nogdb::Vertex::create(txn, "books", r1);
    r2.set("name", "J.K. Rowlings").set("age", 32);
    auto v2 = nogdb::Vertex::create(txn, "persons", r2);
    r3.set("time_used", 365U);
    auto e1 = nogdb::Edge::create(txn, "authors", v1, v2, r3);

    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    auto e1 = nogdb::Edge::get(txn, "authors")[0].descriptor;
    auto record = nogdb::DB::getRecord(txn, e1);
    assert(record.get("time_used").toIntU() == 365U);

    nogdb::Edge::destroy(txn, e1);
    auto res = nogdb::Edge::get(txn, "authors");
    assertSize(res, 0);
    nogdb::Edge::destroy(txn, e1);

    auto v1 = nogdb::Vertex::get(txn, "books")[0];
    auto v2 = nogdb::Vertex::get(txn, "persons")[0];

    //assert(v1.record.getVersion() == 2ULL);
    //assert(v2.record.getVersion() == 2ULL);

    txn.commit();

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_update_version() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  const int E = 5;
  nogdb::RecordDescriptor edge[E];

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Record r1{}, r2{};
    r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
    auto v1 = nogdb::Vertex::create(txn, "books", r1);
    r2.set("name", "J.K. Rowlings").set("age", 32);
    auto v2 = nogdb::Vertex::create(txn, "persons", r2);

    for (int i = 0; i < E; ++i) {
      nogdb::Record r3{};
      r3.set("time_used", 365U + i);
      edge[i] = nogdb::Edge::create(txn, "authors", v1, v2, r3);
    }

    txn.commit();

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  const int ITERATION = 10;
  for (int i = 0; i < ITERATION; ++i) {
    try {

      auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};

      for (int j = 0; j < E; ++j) {
        nogdb::Record r3 = nogdb::DB::getRecord(txn, edge[j]);
        //assert(r3.getVersion() == 1ULL + j);
        r3.set("time_used", 365U + j + E);
        //assert(r3.getVersion() == 2ULL + j);

        r3.set("time_used", 365U + j + E * i);
        //assert(r3.getVersion() == 2ULL + j);
      }

      txn.commit();

    } catch (const nogdb::Error &ex) {
      std::cout << "\nError: " << ex.what() << std::endl;
      assert(false);
    }
  }
}

void test_delete_invalid_edge() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  nogdb::RecordDescriptor v1{}, v2{};
  nogdb::RecordDescriptor e1{};
  try {
    nogdb::Record r1{}, r2{}, r3{};
    r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
    v1 = nogdb::Vertex::create(txn, "books", r1);
    r2.set("name", "J.K. Rowlings").set("age", 32);
    v2 = nogdb::Vertex::create(txn, "persons", r2);
    r3.set("time_used", 365U);
    e1 = nogdb::Edge::create(txn, "authors", v1, v2, r3);

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto tmp = e1;
    tmp.rid.first = 9999U;
    nogdb::Edge::destroy(txn, tmp);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto tmp = v1;
    nogdb::Edge::destroy(txn, tmp);
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_delete_all_edges() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  nogdb::RecordDescriptor v1{}, v2{};
  try {
    nogdb::Record r1{}, r2{}, r3{};
    r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
    v1 = nogdb::Vertex::create(txn, "books", r1);
    r2.set("name", "J.K. Rowlings").set("age", 32);
    v2 = nogdb::Vertex::create(txn, "persons", r2);
    nogdb::Edge::create(txn, "authors", v1, v2, nogdb::Record{}.set("time_used", 365U));
    nogdb::Edge::create(txn, "authors", v1, v2, nogdb::Record{}.set("time_used", 363U));
    nogdb::Edge::create(txn, "authors", v1, v2, nogdb::Record{}.set("time_used", 361U));
    nogdb::Edge::create(txn, "authors", v1, v2, nogdb::Record{}.set("time_used", 356U));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto res = nogdb::Edge::get(txn, "authors");
    assertSize(res, 4);
    res = nogdb::Vertex::getOutEdge(txn, v1);
    assertSize(res, 4);
    res = nogdb::Vertex::getInEdge(txn, v2);
    assertSize(res, 4);

    nogdb::Edge::destroy(txn, "authors");
    res = nogdb::Edge::get(txn, "authors");
    assertSize(res, 0);
    res = nogdb::Vertex::getOutEdge(txn, v1);
    assertSize(res, 0);
    res = nogdb::Vertex::getInEdge(txn, v2);
    assertSize(res, 0);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Edge::destroy(txn, "books");
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Edge::destroy(txn, "authors");
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }
}

void test_get_invalid_edge() {
  init_vertex_book();
  init_vertex_person();
  init_edge_author();

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Record r1{}, r2{}, r3{};
    r1.set("title", "Harry Potter").set("pages", 456).set("price", 24.5);
    auto v1 = nogdb::Vertex::create(txn, "books", r1);
    r2.set("name", "J.K. Rowlings").set("age", 32);
    auto v2 = nogdb::Vertex::create(txn, "persons", r2);
    r3.set("time_used", 365U);
    auto e1 = nogdb::Edge::create(txn, "authors", v1, v2, r3);
    nogdb::Edge::destroy(txn, e1);

    try {
      auto res = nogdb::DB::getRecord(txn, e1);
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

void test_get_edge_cursor() {
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

    const auto testData = std::vector<unsigned int>{365U, 180U, 430U};
    const auto testColumn = std::string{"time_used"};
    try {
      auto res = nogdb::Edge::getCursor(txn, "authors");
      cursorTester(res, testData, testColumn);
    } catch (const nogdb::Error &ex) {
      std::cout << "\nError: " << ex.what() << std::endl;
      assert(false);
    }
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}

void test_get_invalid_edge_cursor() {
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

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::getCursor(txn, "author");
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Edge::getCursor(txn, "persons");
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
  }

  destroy_edge_author();
  destroy_vertex_person();
  destroy_vertex_book();
}
