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

void test_txn_commit_nothing() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txnRw{*ctx, nogdb::TxnMode::READ_WRITE};
    txnRw.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_create_only_vertex_commit() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Record r{};
    r.set("name", "Koh Chang").set("area", "212.34");
    nogdb::Vertex::create(txnRw1, "islands", r);

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    auto res = nogdb::Vertex::get(txnRw1, "islands", nogdb::Condition("name").eq("Koh Chang"));
    assert(!res.empty());
    assert(res[0].record.get("name").toText() == "Koh Chang");

    res = nogdb::Vertex::get(txnRo1, "islands", nogdb::Condition("name").eq("Koh Chang"));
    assert(res.empty());
    res = nogdb::Vertex::get(txnRo2, "islands", nogdb::Condition("name").eq("Koh Chang"));
    assert(res.empty());
    res = nogdb::Vertex::get(txnRo3, "islands", nogdb::Condition("name").eq("Koh Chang"));
    assert(res.empty());

    txnRw1.commit();

    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};

    res = nogdb::Vertex::get(txnRw2, "islands", nogdb::Condition("name").eq("Koh Chang"));
    assert(!res.empty());
    assert(res[0].record.get("name").toText() == "Koh Chang");
    res = nogdb::Vertex::get(txnRo4, "islands", nogdb::Condition("name").eq("Koh Chang"));
    assert(!res.empty());
    assert(res[0].record.get("name").toText() == "Koh Chang");

    res = nogdb::Vertex::get(txnRo1, "islands", nogdb::Condition("name").eq("Koh Chang"));
    assert(res.empty());
    res = nogdb::Vertex::get(txnRo2, "islands", nogdb::Condition("name").eq("Koh Chang"));
    assert(res.empty());
    res = nogdb::Vertex::get(txnRo3, "islands", nogdb::Condition("name").eq("Koh Chang"));
    assert(res.empty());

    txnRo1.commit();
    txnRo2.commit();
    txnRo3.commit();

    txnRo4.rollback();
    txnRw2.rollback();
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_create_only_vertex_rollback() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Record r{};
    r.set("name", "Koh Mak").set("area", "87.92");
    nogdb::Vertex::create(txnRw1, "islands", r);
    auto res = nogdb::Vertex::get(txnRw1, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(!res.empty());
    assert(res[0].record.get("name").toText() == "Koh Mak");

    txnRw1.rollback();

    nogdb::Transaction txnRo{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw00{*ctx, nogdb::TxnMode::READ_WRITE};
    res = nogdb::Vertex::get(txnRo, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(res.empty());
    res = nogdb::Vertex::get(txnRw00, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(res.empty());

    txnRo.commit();
    txnRw00.commit();

  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_rollback_when_destroy() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txnRw{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Record r{};
    r.set("name", "Koh Mak").set("area", "87.92");
    nogdb::Vertex::create(txnRw, "islands", r);
    auto res = nogdb::Vertex::get(txnRw, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(!res.empty());
    assert(res[0].record.get("name").toText() == "Koh Mak");
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    nogdb::Transaction txnRo{*ctx, nogdb::TxnMode::READ_ONLY};
    auto res = nogdb::Vertex::get(txnRo, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(res.empty());
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_delete_only_vertex_commit() {
  init_vertex_island();
  init_edge_bridge();

  auto vdesc = nogdb::RecordDescriptor{};
  try {
    nogdb::Transaction txnRw{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Record r{};
    r.set("name", "Koh Mak").set("area", "87.92");
    vdesc = nogdb::Vertex::create(txnRw, "islands", r);
    txnRw.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Vertex::destroy(txnRw1, vdesc);

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    auto res = nogdb::Vertex::get(txnRw1, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(res.empty());

    res = nogdb::Vertex::get(txnRo1, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(!res.empty());
    assert(res[0].record.get("name").toText() == "Koh Mak");
    res = nogdb::Vertex::get(txnRo2, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(!res.empty());
    assert(res[0].record.get("name").toText() == "Koh Mak");
    res = nogdb::Vertex::get(txnRo3, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(!res.empty());
    assert(res[0].record.get("name").toText() == "Koh Mak");

    txnRw1.commit();

    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};
    res = nogdb::Vertex::get(txnRo4, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(res.empty());
    res = nogdb::Vertex::get(txnRw2, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(res.empty());

    res = nogdb::Vertex::get(txnRo1, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(!res.empty());
    assert(res[0].record.get("name").toText() == "Koh Mak");
    res = nogdb::Vertex::get(txnRo2, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(!res.empty());
    assert(res[0].record.get("name").toText() == "Koh Mak");
    res = nogdb::Vertex::get(txnRo3, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(!res.empty());
    assert(res[0].record.get("name").toText() == "Koh Mak");

    txnRo1.commit();
    txnRo2.commit();
    txnRo3.commit();

    txnRo4.rollback();
    txnRw2.rollback();
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_delete_only_vertex_rollback() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txnRw0{*ctx, nogdb::TxnMode::READ_WRITE};
    auto vdesc = nogdb::Vertex::create(txnRw0, "islands",
                                       nogdb::Record{}.set("name", "Koh Mak").set("area", "87.92"));
    txnRw0.commit();

    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Vertex::destroy(txnRw1, vdesc);
    auto res = nogdb::Vertex::get(txnRw1, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(res.empty());
    txnRw1.rollback();

    nogdb::Transaction txnRo{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};
    res = nogdb::Vertex::get(txnRo, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(!res.empty());
    assert(res[0].record.get("name").toText() == "Koh Mak");
    res = nogdb::Vertex::get(txnRw2, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(!res.empty());
    assert(res[0].record.get("name").toText() == "Koh Mak");

    txnRo.commit();
    txnRw2.commit();

    nogdb::Transaction txnRw00{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Vertex::destroy(txnRw00, vdesc);
    txnRw00.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_create_only_edge_commit() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    auto vdesc1 = nogdb::Vertex::create(txnRw1, "islands",
                                        nogdb::Record{}.set("name", "Koh Kood").set("area", "145.32"));
    auto vdesc2 = nogdb::Vertex::create(txnRw1, "islands",
                                        nogdb::Record{}.set("name", "Koh Mak").set("area", "87.92"));
    nogdb::Edge::create(txnRw1, "bridge", vdesc1, vdesc2, nogdb::Record{}.set("name", "yellow"));

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    auto res = nogdb::Vertex::get(txnRw1, "islands", nogdb::Condition("name").eq("Koh Kood"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRw1, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(!res.empty());
    auto resE = nogdb::Edge::get(txnRw1, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(!resE.empty());

    resE = nogdb::Edge::get(txnRo1, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(resE.empty());
    resE = nogdb::Edge::get(txnRo2, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(resE.empty());
    resE = nogdb::Edge::get(txnRo3, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(resE.empty());

    txnRw1.commit();

    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};

    resE = nogdb::Edge::get(txnRo4, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(!resE.empty());
    resE = nogdb::Edge::get(txnRw2, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(!resE.empty());

    resE = nogdb::Edge::get(txnRo1, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(resE.empty());
    resE = nogdb::Edge::get(txnRo2, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(resE.empty());
    resE = nogdb::Edge::get(txnRo3, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(resE.empty());

    txnRo1.commit();
    txnRo2.commit();
    txnRo3.commit();

    txnRo4.rollback();
    txnRw2.rollback();

  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_create_only_edge_rollback() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    auto vdesc1 = nogdb::Vertex::create(txnRw1, "islands",
                                        nogdb::Record{}.set("name", "Koh Kood").set("area", "145.32"));
    auto vdesc2 = nogdb::Vertex::create(txnRw1, "islands",
                                        nogdb::Record{}.set("name", "Koh Mak").set("area", "87.92"));
    nogdb::Edge::create(txnRw1, "bridge", vdesc1, vdesc2, nogdb::Record{}.set("name", "yellow"));

    auto res = nogdb::Vertex::get(txnRw1, "islands", nogdb::Condition("name").eq("Koh Kood"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRw1, "islands", nogdb::Condition("name").eq("Koh Mak"));
    assert(!res.empty());
    auto resE = nogdb::Edge::get(txnRw1, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(!resE.empty());

    txnRw1.rollback();

    nogdb::Transaction txnRo{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw00{*ctx, nogdb::TxnMode::READ_WRITE};

    resE = nogdb::Edge::get(txnRo, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(resE.empty());
    resE = nogdb::Edge::get(txnRw00, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(resE.empty());

    txnRo.commit();
    txnRw00.commit();

  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_delete_only_edge_commit() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txnRw{*ctx, nogdb::TxnMode::READ_WRITE};
    auto vdesc1 = nogdb::Vertex::create(txnRw, "islands",
                                        nogdb::Record{}.set("name", "Koh Kood").set("area", "145.32"));
    auto vdesc2 = nogdb::Vertex::create(txnRw, "islands",
                                        nogdb::Record{}.set("name", "Koh Mak").set("area", "87.92"));
    nogdb::Edge::create(txnRw, "bridge", vdesc1, vdesc2, nogdb::Record{}.set("name", "yellow"));
    txnRw.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    auto edesc = nogdb::Edge::get(txnRw1, "bridge", nogdb::Condition("name").eq("yellow"));
    nogdb::Edge::destroy(txnRw1, edesc[0].descriptor);

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    auto resE = nogdb::Edge::get(txnRw1, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(resE.empty());

    resE = nogdb::Edge::get(txnRo1, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(!resE.empty());
    resE = nogdb::Edge::get(txnRo2, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(!resE.empty());
    resE = nogdb::Edge::get(txnRo3, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(!resE.empty());

    txnRw1.commit();

    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};
    resE = nogdb::Edge::get(txnRo4, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(resE.empty());
    resE = nogdb::Edge::get(txnRw2, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(resE.empty());

    resE = nogdb::Edge::get(txnRo1, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(!resE.empty());
    resE = nogdb::Edge::get(txnRo2, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(!resE.empty());
    resE = nogdb::Edge::get(txnRo3, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(!resE.empty());

    txnRo1.commit();
    txnRo2.commit();
    txnRo3.commit();

    txnRo4.rollback();
    txnRw2.rollback();
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_delete_only_edge_rollback() {
  init_vertex_island();
  init_edge_bridge();

  auto vdesc1 = nogdb::RecordDescriptor{};
  auto vdesc2 = nogdb::RecordDescriptor{};
  try {
    nogdb::Transaction txnRw{*ctx, nogdb::TxnMode::READ_WRITE};
    vdesc1 = nogdb::Vertex::create(txnRw, "islands", nogdb::Record{}.set("name", "Koh Kood").set("area", "145.32"));
    vdesc2 = nogdb::Vertex::create(txnRw, "islands", nogdb::Record{}.set("name", "Koh Mak").set("area", "87.92"));
    nogdb::Edge::create(txnRw, "bridge", vdesc1, vdesc2, nogdb::Record{}.set("name", "yellow"));
    txnRw.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    auto edesc = nogdb::Edge::get(txnRw1, "bridge", nogdb::Condition("name").eq("yellow"));
    nogdb::Edge::destroy(txnRw1, edesc[0].descriptor);
    auto resE = nogdb::Edge::get(txnRw1, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(resE.empty());

    txnRw1.rollback();

    nogdb::Transaction txnRo{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};
    resE = nogdb::Edge::get(txnRo, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(!resE.empty());
    resE = nogdb::Edge::get(txnRw2, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(!resE.empty());

    txnRo.commit();
    txnRw2.commit();

    nogdb::Transaction txnRw00{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Vertex::destroy(txnRw00, vdesc1);
    nogdb::Vertex::destroy(txnRw00, vdesc2);
    resE = nogdb::Edge::get(txnRw00, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(resE.empty());

    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};

    txnRw00.commit();

    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    resE = nogdb::Edge::get(txnRo1, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(!resE.empty());
    resE = nogdb::Edge::get(txnRo2, "bridge", nogdb::Condition("name").eq("yellow"));
    assert(resE.empty());

    txnRo1.rollback();
    txnRo2.rollback();
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_get_vertex_edge() {
  init_vertex_island();
  init_edge_bridge();
  init_edge_flight();

  try {
    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    auto v1 = nogdb::Vertex::create(txnRw1, "islands", nogdb::Record{}.set("name", "1"));
    auto v2 = nogdb::Vertex::create(txnRw1, "islands", nogdb::Record{}.set("name", "2"));
    auto v3 = nogdb::Vertex::create(txnRw1, "islands", nogdb::Record{}.set("name", "3"));
    auto e1 = nogdb::Edge::create(txnRw1, "bridge", v1, v2, nogdb::Record{}.set("name", "12"));
    auto e2 = nogdb::Edge::create(txnRw1, "flight", v1, v3, nogdb::Record{}.set("name", "13"));

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    std::vector<std::function<void(nogdb::Transaction &)>> testCases{
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchSrc(e1);
          assert(res.record.get("name").toText() == "1");
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchDst(e1);
          assert(res.record.get("name").toText() == "2");
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchSrc(e2);
          assert(res.record.get("name").toText() == "1");
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchDst(e2);
          assert(res.record.get("name").toText() == "3");
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findInEdge(v2);
          assert(res[0].record.get("name").toText() == "12");
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findInEdge(v3);
          assert(res[0].record.get("name").toText() == "13");
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findOutEdge(v1);
          ASSERT_SIZE(res, 2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findOutEdge(v1, nogdb::GraphFilter{}.only("bridge"));
          assert(res[0].record.get("name").toText() == "12");
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findOutEdge(v1, nogdb::GraphFilter{}.only("flight"));
          assert(res[0].record.get("name").toText() == "13");
        }
    };

    runTestCases(txnRw1, testCases, true);
    runTestCases(txnRo1, testCases, false);
    runTestCases(txnRo2, testCases, false);
    runTestCases(txnRo3, testCases, false);

    txnRw1.commit();

    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};

    runTestCases(txnRw2, testCases, true);
    runTestCases(txnRo4, testCases, true);

    runTestCases(txnRo1, testCases, false);
    runTestCases(txnRo2, testCases, false);
    runTestCases(txnRo3, testCases, false);
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_flight();
  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_alter_vertex_edge_commit() {
  init_vertex_island();
  init_edge_bridge();
  init_edge_flight();

  try {
    nogdb::Transaction txnRw0{*ctx, nogdb::TxnMode::READ_WRITE};

    auto v1 = nogdb::Vertex::create(txnRw0, "islands", nogdb::Record{}.set("name", "1"));
    auto v2 = nogdb::Vertex::create(txnRw0, "islands", nogdb::Record{}.set("name", "2"));
    auto v3 = nogdb::Vertex::create(txnRw0, "islands", nogdb::Record{}.set("name", "3"));
    auto e1 = nogdb::Edge::create(txnRw0, "bridge", v1, v2, nogdb::Record{}.set("name", "12"));
    auto e2 = nogdb::Edge::create(txnRw0, "flight", v1, v3, nogdb::Record{}.set("name", "13"));

    txnRw0.commit();

    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Edge::updateSrc(txnRw1, e1, v3);
    nogdb::Edge::updateDst(txnRw1, e2, v2);

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};
    std::vector<std::function<void(nogdb::Transaction &)>> oldTestCases{
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchSrc(e1);
          assert(res.descriptor == v1);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchSrc(e2);
          assert(res.descriptor == v1);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchDst(e1);
          assert(res.descriptor == v2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchDst(e2);
          assert(res.descriptor == v3);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findInEdge(v2);
          assert(res[0].descriptor == e1);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findInEdge(v3);
          assert(res[0].descriptor == e2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findOutEdge(v1);
          ASSERT_SIZE(res, 2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findOutEdge(v1, nogdb::GraphFilter{}.only("bridge"));
          assert(res[0].descriptor == e1);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findOutEdge(v1, nogdb::GraphFilter{}.only("flight"));
          assert(res[0].descriptor == e2);
        }
    };

    std::vector<std::function<void(nogdb::Transaction &)>> newTestCases{
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchSrc(e1);
          assert(res.descriptor == v3);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchSrc(e2);
          assert(res.descriptor == v1);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchDst(e1);
          assert(res.descriptor == v2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchDst(e2);
          assert(res.descriptor == v2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findInEdge(v2);
          ASSERT_SIZE(res, 2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findOutEdge(v3);
          assert(res[0].descriptor == e1);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findOutEdge(v1);
          assert(res[0].descriptor == e2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findInEdge(v2, nogdb::GraphFilter{}.only("bridge"));
          assert(res[0].descriptor == e1);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findInEdge(v2, nogdb::GraphFilter{}.only("flight"));
          assert(res[0].descriptor == e2);
        }
    };

    runTestCases(txnRw1, newTestCases, true);
    runTestCases(txnRo1, oldTestCases, true);
    runTestCases(txnRo2, oldTestCases, true);
    runTestCases(txnRo3, oldTestCases, true);

    txnRw1.commit();

    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};

    runTestCases(txnRo4, newTestCases, true);
    runTestCases(txnRw2, newTestCases, true);

    runTestCases(txnRo1, oldTestCases, true);
    runTestCases(txnRo2, oldTestCases, true);
    runTestCases(txnRo3, oldTestCases, true);
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_flight();
  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_alter_vertex_edge_rollback() {
  init_vertex_island();
  init_edge_bridge();
  init_edge_flight();

  nogdb::RecordDescriptor v1, v2, v3, e1, e2;
  try {
    nogdb::Transaction txnRw0{*ctx, nogdb::TxnMode::READ_WRITE};
    v1 = nogdb::Vertex::create(txnRw0, "islands", nogdb::Record{}.set("name", "1"));
    v2 = nogdb::Vertex::create(txnRw0, "islands", nogdb::Record{}.set("name", "2"));
    v3 = nogdb::Vertex::create(txnRw0, "islands", nogdb::Record{}.set("name", "3"));
    e1 = nogdb::Edge::create(txnRw0, "bridge", v3, v2, nogdb::Record{}.set("name", "32"));
    e2 = nogdb::Edge::create(txnRw0, "flight", v1, v2, nogdb::Record{}.set("name", "12"));
    txnRw0.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Edge::updateSrc(txnRw1, e1, v1);
    nogdb::Edge::updateDst(txnRw1, e2, v3);

    std::vector<std::function<void(nogdb::Transaction &)>> newTestCases{
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchSrc(e1);
          assert(res.descriptor == v1);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchSrc(e2);
          assert(res.descriptor == v1);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchDst(e1);
          assert(res.descriptor == v2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchDst(e2);
          assert(res.descriptor == v3);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findInEdge(v2);
          assert(res[0].descriptor == e1);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findInEdge(v3);
          assert(res[0].descriptor == e2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findOutEdge(v1);
          ASSERT_SIZE(res, 2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findOutEdge(v1, nogdb::GraphFilter{}.only("bridge"));
          assert(res[0].descriptor == e1);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findOutEdge(v1, nogdb::GraphFilter{}.only("flight"));
          assert(res[0].descriptor == e2);
        }
    };

    std::vector<std::function<void(nogdb::Transaction &)>> oldTestCases{
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchSrc(e1);
          assert(res.descriptor == v3);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchSrc(e2);
          assert(res.descriptor == v1);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchDst(e1);
          assert(res.descriptor == v2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.fetchDst(e2);
          assert(res.descriptor == v2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findInEdge(v2);
          ASSERT_SIZE(res, 2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findOutEdge(v3);
          assert(res[0].descriptor == e1);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findOutEdge(v1);
          assert(res[0].descriptor == e2);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findInEdge(v2, nogdb::GraphFilter{}.only("bridge"));
          assert(res[0].descriptor == e1);
        },
        [&](const nogdb::Transaction &txn) {
          auto res = txn.findInEdge(v2, nogdb::GraphFilter{}.only("flight"));
          assert(res[0].descriptor == e2);
        }
    };

    runTestCases(txnRw1, newTestCases, true);

    txnRw1.rollback();

    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo{*ctx, nogdb::TxnMode::READ_ONLY};
    runTestCases(txnRw2, oldTestCases, true);
    runTestCases(txnRo, oldTestCases, true);
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_flight();
  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_create_only_vertex_multiversion_commit() {
  init_vertex_island();
  init_edge_bridge();
  init_edge_flight();

  try {
    nogdb::Transaction txnRw0{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo0{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Vertex::create(txnRw0, "islands", nogdb::Record{}.set("name", "Koh Samed"));
    txnRw0.commit();

    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Vertex::create(txnRw1, "islands", nogdb::Record{}.set("name", "Koh Phe Phe"));

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    txnRw1.commit();

    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};

    auto res = nogdb::Vertex::get(txnRo0, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(res.empty());
    res = nogdb::Vertex::get(txnRo0, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(res.empty());

    res = nogdb::Vertex::get(txnRo1, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo1, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(res.empty());

    res = nogdb::Vertex::get(txnRo2, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo2, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(res.empty());

    res = nogdb::Vertex::get(txnRo3, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo3, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(res.empty());

    res = nogdb::Vertex::get(txnRw2, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRw2, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(!res.empty());

    res = nogdb::Vertex::get(txnRo4, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo4, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(!res.empty());
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_flight();
  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_create_only_vertex_multiversion_rollback() {
  init_vertex_island();
  init_edge_bridge();
  init_edge_flight();

  try {
    nogdb::Transaction txnRw{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Vertex::create(txnRw, "islands", nogdb::Record{}.set("name", "Koh Tarutao"));
    txnRw.commit();

    nogdb::Transaction txnRo0{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw0{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Vertex::create(txnRw0, "islands", nogdb::Record{}.set("name", "Koh Samed"));
    txnRw0.commit();

    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Vertex::create(txnRw1, "islands", nogdb::Record{}.set("name", "Koh Phe Phe"));

    auto res = nogdb::Vertex::get(txnRw1, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRw1, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(!res.empty());

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    txnRw1.rollback();

    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};

    res = nogdb::Vertex::get(txnRo0, "islands", nogdb::Condition("name").eq("Koh Tarutao"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo0, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(res.empty());
    res = nogdb::Vertex::get(txnRo0, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(res.empty());

    res = nogdb::Vertex::get(txnRo1, "islands", nogdb::Condition("name").eq("Koh Tarutao"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo1, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo1, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(res.empty());

    res = nogdb::Vertex::get(txnRo2, "islands", nogdb::Condition("name").eq("Koh Tarutao"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo2, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo2, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(res.empty());

    res = nogdb::Vertex::get(txnRo3, "islands", nogdb::Condition("name").eq("Koh Tarutao"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo3, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo3, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(res.empty());

    res = nogdb::Vertex::get(txnRo4, "islands", nogdb::Condition("name").eq("Koh Tarutao"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo4, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo4, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(res.empty());

    res = nogdb::Vertex::get(txnRw2, "islands", nogdb::Condition("name").eq("Koh Tarutao"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRw2, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRw2, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(res.empty());
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_flight();
  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_delete_only_vertex_multiversion_commit() {
  init_vertex_island();
  init_edge_bridge();
  init_edge_flight();

  try {
    nogdb::Transaction txnRw{*ctx, nogdb::TxnMode::READ_WRITE};
    auto v1 = nogdb::Vertex::create(txnRw, "islands", nogdb::Record{}.set("name", "Koh Samed"));
    txnRw.commit();

    nogdb::Transaction txnRw0{*ctx, nogdb::TxnMode::READ_WRITE};
    auto v2 = nogdb::Vertex::create(txnRw0, "islands", nogdb::Record{}.set("name", "Koh Phe Phe"));
    txnRw0.commit();

    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Vertex::destroy(txnRw1, v2);

    auto res = nogdb::Vertex::get(txnRw1, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRw1, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(res.empty());

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    txnRw1.commit();

    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};

    res = nogdb::Vertex::get(txnRo1, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo1, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo2, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo2, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo3, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo3, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo4, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo4, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(res.empty());
    res = nogdb::Vertex::get(txnRw2, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRw2, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(res.empty());
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_flight();
  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_delete_only_vertex_multiversion_rollback() {
  init_vertex_island();
  init_edge_bridge();
  init_edge_flight();

  try {
    nogdb::Transaction txnRw{*ctx, nogdb::TxnMode::READ_WRITE};
    auto v1 = nogdb::Vertex::create(txnRw, "islands", nogdb::Record{}.set("name", "Koh Samed"));
    txnRw.commit();

    nogdb::Transaction txnRw0{*ctx, nogdb::TxnMode::READ_WRITE};
    auto v2 = nogdb::Vertex::create(txnRw0, "islands", nogdb::Record{}.set("name", "Koh Phe Phe"));
    txnRw0.commit();

    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Vertex::destroy(txnRw1, v2);

    auto res = nogdb::Vertex::get(txnRw1, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRw1, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(res.empty());

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    txnRw1.rollback();

    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};

    res = nogdb::Vertex::get(txnRo1, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo1, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo2, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo2, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo3, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo3, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo4, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRo4, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRw2, "islands", nogdb::Condition("name").eq("Koh Samed"));
    assert(!res.empty());
    res = nogdb::Vertex::get(txnRw2, "islands", nogdb::Condition("name").eq("Koh Phe Phe"));
    assert(!res.empty());
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_flight();
  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_create_edges_multiversion_commit() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txnRw00{*ctx, nogdb::TxnMode::READ_WRITE};
    auto v1 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Samed"));
    auto v2 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Phe PHe"));
    auto v3 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Tao"));
    nogdb::Edge::create(txnRw00, "bridge", v1, v2, nogdb::Record{}.set("name", "bridge 12"));
    nogdb::Edge::create(txnRw00, "bridge", v2, v1, nogdb::Record{}.set("name", "bridge 21"));

    txnRw00.commit();

    nogdb::Transaction txnRw0{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo0{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Edge::create(txnRw0, "bridge", v2, v3, nogdb::Record{}.set("name", "bridge 23"));

    txnRw0.commit();

    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Edge::create(txnRw1, "bridge", v1, v3, nogdb::Record{}.set("name", "bridge 13"));

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    txnRw1.commit();

    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};

    std::vector<std::function<void(nogdb::Transaction &)>> testCasesV0, testCasesV1, testCasesV2;
    testCasesV0.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.find("bridge", nogdb::Condition("name").eq("bridge 23"));
      assert(res.empty());
    });
    testCasesV0.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v2);
      ASSERT_SIZE(res, 1);
      assert(res[0].record.get("name").toText() == "bridge 21");
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.find("bridge", nogdb::Condition("name").eq("bridge 13"));
      assert(res.empty());
      res = txn.find("bridge", nogdb::Condition("name").eq("bridge 23"));
      assert(!res.empty());
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v3);
      ASSERT_SIZE(res, 1);
      assert(res[0].record.get("name").toText() == "bridge 23");
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v1);
      ASSERT_SIZE(res, 1);
      assert(res[0].record.get("name").toText() == "bridge 12");
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v2);
      ASSERT_SIZE(res, 2);
    });
    testCasesV2.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.find("bridge", nogdb::Condition("name").eq("bridge 13"));
      assert(!res.empty());
      res = txn.find("bridge", nogdb::Condition("name").eq("bridge 23"));
      assert(!res.empty());
    });
    testCasesV2.push_back([&](nogdb::Transaction &txn) {
      auto e = txn.find("bridge", nogdb::Condition("name").eq("bridge 13"));
      auto res = txn.fetchSrc(e[0].descriptor);
      assert(res.descriptor == v1);
      res = txn.fetchDst(e[0].descriptor);
      assert(res.descriptor == v3);
    });
    testCasesV2.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v3);
      ASSERT_SIZE(res, 2);
    });
    testCasesV2.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v1);
      ASSERT_SIZE(res, 2);
    });
    testCasesV2.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v2);
      ASSERT_SIZE(res, 2);
    });

    runTestCases(txnRo0, testCasesV0, true);
    runTestCases(txnRo1, testCasesV1, true);
    runTestCases(txnRo2, testCasesV1, true);
    runTestCases(txnRo3, testCasesV1, true);
    runTestCases(txnRo4, testCasesV2, true);
    runTestCases(txnRw2, testCasesV2, true);

  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_create_edges_multiversion_rollback() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txnRw00{*ctx, nogdb::TxnMode::READ_WRITE};
    auto v1 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Samed"));
    auto v2 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Phe PHe"));
    auto v3 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Tao"));
    nogdb::Edge::create(txnRw00, "bridge", v1, v2, nogdb::Record{}.set("name", "bridge 12"));
    nogdb::Edge::create(txnRw00, "bridge", v2, v1, nogdb::Record{}.set("name", "bridge 21"));

    txnRw00.commit();

    nogdb::Transaction txnRw0{*ctx, nogdb::TxnMode::READ_WRITE};

    nogdb::Edge::create(txnRw0, "bridge", v2, v3, nogdb::Record{}.set("name", "bridge 23"));

    txnRw0.commit();

    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Edge::create(txnRw1, "bridge", v1, v3, nogdb::Record{}.set("name", "bridge 13"));

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    txnRw1.rollback();

    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};

    std::vector<std::function<void(nogdb::Transaction &)>> testCasesV1;
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.find("bridge", nogdb::Condition("name").eq("bridge 13"));
      assert(res.empty());
      res = txn.find("bridge", nogdb::Condition("name").eq("bridge 23"));
      assert(!res.empty());
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v3);
      ASSERT_SIZE(res, 1);
      assert(res[0].record.get("name").toText() == "bridge 23");
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v1);
      ASSERT_SIZE(res, 1);
      assert(res[0].record.get("name").toText() == "bridge 12");
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v2);
      ASSERT_SIZE(res, 2);
    });

    runTestCases(txnRo1, testCasesV1, true);
    runTestCases(txnRo2, testCasesV1, true);
    runTestCases(txnRo3, testCasesV1, true);
    runTestCases(txnRo4, testCasesV1, true);
    runTestCases(txnRw2, testCasesV1, true);

  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_delete_edges_multiversion_commit() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txnRw00{*ctx, nogdb::TxnMode::READ_WRITE};
    auto v1 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Samed"));
    auto v2 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Phe PHe"));
    auto v3 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Tao"));
    auto e1 = nogdb::Edge::create(txnRw00, "bridge", v1, v2, nogdb::Record{}.set("name", "bridge 12"));
    auto e2 = nogdb::Edge::create(txnRw00, "bridge", v2, v1, nogdb::Record{}.set("name", "bridge 21"));
    auto e3 = nogdb::Edge::create(txnRw00, "bridge", v2, v3, nogdb::Record{}.set("name", "bridge 23"));
    auto e4 = nogdb::Edge::create(txnRw00, "bridge", v1, v3, nogdb::Record{}.set("name", "bridge 13"));

    txnRw00.commit();

    nogdb::Transaction txnRw0{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo0{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Edge::destroy(txnRw0, e1);

    txnRw0.commit();

    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Vertex::destroy(txnRw1, v3);

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    txnRw1.commit();

    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};

    std::vector<std::function<void(nogdb::Transaction &)>> testCasesV0, testCasesV1, testCasesV2;
    testCasesV0.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.find("bridge", nogdb::Condition("name").eq("bridge 13"));
      assert(!res.empty());
      res = txn.find("bridge", nogdb::Condition("name").eq("bridge 23"));
      assert(!res.empty());
      res = txn.find("bridge", nogdb::Condition("name").eq("bridge 12"));
      assert(!res.empty());
    });
    testCasesV0.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v3);
      ASSERT_SIZE(res, 2);
    });
    testCasesV0.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v1);
      ASSERT_SIZE(res, 2);
    });
    testCasesV0.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v2);
      ASSERT_SIZE(res, 2);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.find("bridge", nogdb::Condition("name").eq("bridge 13"));
      assert(!res.empty());
      res = txn.find("bridge", nogdb::Condition("name").eq("bridge 23"));
      assert(!res.empty());
      res = txn.find("bridge", nogdb::Condition("name").eq("bridge 12"));
      assert(res.empty());
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v3);
      ASSERT_SIZE(res, 2);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v1);
      ASSERT_SIZE(res, 1);
      assert(res[0].descriptor == e4);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v2);
      ASSERT_SIZE(res, 0);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v2);
      ASSERT_SIZE(res, 2);
    });
    testCasesV2.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.find("bridge", nogdb::Condition("name").eq("bridge 13"));
      assert(res.empty());
      res = txn.find("bridge", nogdb::Condition("name").eq("bridge 23"));
      assert(res.empty());
      res = txn.find("bridge", nogdb::Condition("name").eq("bridge 12"));
      assert(res.empty());
      res = txn.find("bridge", nogdb::Condition("name").eq("bridge 21"));
      assert(!res.empty());
    });
    testCasesV2.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v1);
      ASSERT_SIZE(res, 0);
    });
    testCasesV2.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v2);
      ASSERT_SIZE(res, 0);
    });
    testCasesV2.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v2);
      ASSERT_SIZE(res, 1);
      assert(res[0].descriptor == e2);
    });

    runTestCases(txnRo0, testCasesV0, true);
    runTestCases(txnRo1, testCasesV1, true);
    runTestCases(txnRo2, testCasesV1, true);
    runTestCases(txnRo3, testCasesV1, true);
    runTestCases(txnRo4, testCasesV2, true);
    runTestCases(txnRw2, testCasesV2, true);
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();

}

void test_txn_delete_edges_multiversion_rollback() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txnRw00{*ctx, nogdb::TxnMode::READ_WRITE};
    auto v1 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Samed"));
    auto v2 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Phe PHe"));
    auto v3 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Tao"));
    auto e1 = nogdb::Edge::create(txnRw00, "bridge", v1, v2, nogdb::Record{}.set("name", "bridge 12"));
    auto e2 = nogdb::Edge::create(txnRw00, "bridge", v2, v1, nogdb::Record{}.set("name", "bridge 21"));
    auto e3 = nogdb::Edge::create(txnRw00, "bridge", v2, v3, nogdb::Record{}.set("name", "bridge 23"));
    auto e4 = nogdb::Edge::create(txnRw00, "bridge", v1, v3, nogdb::Record{}.set("name", "bridge 13"));

    txnRw00.commit();

    nogdb::Transaction txnRw0{*ctx, nogdb::TxnMode::READ_WRITE};

    nogdb::Edge::destroy(txnRw0, e1);

    txnRw0.commit();

    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Vertex::destroy(txnRw1, v3);

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    txnRw1.rollback();

    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};

    std::vector<std::function<void(nogdb::Transaction &)>> testCasesV1;
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.find("bridge", nogdb::Condition("name").eq("bridge 13"));
      assert(!res.empty());
      res = txn.find("bridge", nogdb::Condition("name").eq("bridge 23"));
      assert(!res.empty());
      res = txn.find("bridge", nogdb::Condition("name").eq("bridge 12"));
      assert(res.empty());
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v3);
      ASSERT_SIZE(res, 2);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v1);
      ASSERT_SIZE(res, 1);
      assert(res[0].descriptor == e4);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v2);
      ASSERT_SIZE(res, 0);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v2);
      ASSERT_SIZE(res, 2);
    });

    runTestCases(txnRo1, testCasesV1, true);
    runTestCases(txnRo2, testCasesV1, true);
    runTestCases(txnRo3, testCasesV1, true);
    runTestCases(txnRo4, testCasesV1, true);
    runTestCases(txnRw2, testCasesV1, true);
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_modify_edges_multiversion_commit() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txnRw00{*ctx, nogdb::TxnMode::READ_WRITE};
    auto v1 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Samed"));
    auto v2 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Phe PHe"));
    auto v3 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Tao"));
    auto e1 = nogdb::Edge::create(txnRw00, "bridge", v1, v2, nogdb::Record{}.set("name", "bridge 12"));

    txnRw00.commit();

    nogdb::Transaction txnRw0{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo0{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Edge::updateDst(txnRw0, e1, v3);

    txnRw0.commit();

    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Edge::updateSrc(txnRw1, e1, v2);

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    txnRw1.commit();

    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};

    std::vector<std::function<void(nogdb::Transaction &)>> testCasesV0, testCasesV1, testCasesV2;
    testCasesV0.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.fetchSrc(e1);
      assert(res.descriptor == v1);
      res = txn.fetchDst(e1);
      assert(res.descriptor == v2);
    });
    testCasesV0.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v1);
      ASSERT_SIZE(res, 1);
    });
    testCasesV0.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v2);
      ASSERT_SIZE(res, 1);
    });
    testCasesV0.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v2);
      ASSERT_SIZE(res, 0);
    });
    testCasesV0.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v3);
      ASSERT_SIZE(res, 0);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.fetchSrc(e1);
      assert(res.descriptor == v1);
      res = txn.fetchDst(e1);
      assert(res.descriptor == v3);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v1);
      ASSERT_SIZE(res, 1);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v2);
      ASSERT_SIZE(res, 0);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v2);
      ASSERT_SIZE(res, 0);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v3);
      ASSERT_SIZE(res, 1);
    });
    testCasesV2.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.fetchSrc(e1);
      assert(res.descriptor == v2);
      res = txn.fetchDst(e1);
      assert(res.descriptor == v3);
    });
    testCasesV2.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v1);
      ASSERT_SIZE(res, 0);
    });
    testCasesV2.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v2);
      ASSERT_SIZE(res, 0);
    });
    testCasesV2.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v2);
      ASSERT_SIZE(res, 1);
    });
    testCasesV2.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v3);
      ASSERT_SIZE(res, 1);
    });

    runTestCases(txnRo0, testCasesV0, true);
    runTestCases(txnRo1, testCasesV1, true);
    runTestCases(txnRo2, testCasesV1, true);
    runTestCases(txnRo3, testCasesV1, true);
    runTestCases(txnRo4, testCasesV2, true);
    runTestCases(txnRw2, testCasesV2, true);
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_modify_edges_multiversion_rollback() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txnRw00{*ctx, nogdb::TxnMode::READ_WRITE};
    auto v1 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Samed"));
    auto v2 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Phe PHe"));
    auto v3 = nogdb::Vertex::create(txnRw00, "islands", nogdb::Record{}.set("name", "Koh Tao"));
    auto e1 = nogdb::Edge::create(txnRw00, "bridge", v1, v2, nogdb::Record{}.set("name", "bridge 12"));

    txnRw00.commit();

    nogdb::Transaction txnRw0{*ctx, nogdb::TxnMode::READ_WRITE};

    nogdb::Edge::updateDst(txnRw0, e1, v3);

    txnRw0.commit();

    nogdb::Transaction txnRo1{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw1{*ctx, nogdb::TxnMode::READ_WRITE};
    nogdb::Transaction txnRo2{*ctx, nogdb::TxnMode::READ_ONLY};

    nogdb::Edge::updateSrc(txnRw1, e1, v2);

    nogdb::Transaction txnRo3{*ctx, nogdb::TxnMode::READ_ONLY};

    txnRw1.rollback();

    nogdb::Transaction txnRo4{*ctx, nogdb::TxnMode::READ_ONLY};
    nogdb::Transaction txnRw2{*ctx, nogdb::TxnMode::READ_WRITE};

    std::vector<std::function<void(nogdb::Transaction &)>> testCasesV1;
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.fetchSrc(e1);
      assert(res.descriptor == v1);
      res = txn.fetchDst(e1);
      assert(res.descriptor == v3);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v1);
      ASSERT_SIZE(res, 1);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v2);
      ASSERT_SIZE(res, 0);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findOutEdge(v2);
      ASSERT_SIZE(res, 0);
    });
    testCasesV1.push_back([&](nogdb::Transaction &txn) {
      auto res = txn.findInEdge(v3);
      ASSERT_SIZE(res, 1);
    });

    runTestCases(txnRo1, testCasesV1, true);
    runTestCases(txnRo2, testCasesV1, true);
    runTestCases(txnRo3, testCasesV1, true);
    runTestCases(txnRo4, testCasesV1, true);
    runTestCases(txnRw2, testCasesV1, true);
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}

void test_txn_reopen_ctx() {
  init_vertex_island();

  try {
    auto txn = std::unique_ptr<nogdb::Txn>(new ctx->beginTxn(nogdb::TxnMode::READ_WRITE));
    auto v1 = nogdb::Vertex::create(*txn, "islands", nogdb::Record{}.set("name", "Koh Samui"));
    auto v2 = nogdb::Vertex::create(*txn, "islands", nogdb::Record{}.set("name", "Koh Tao"));
    txn->commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  delete ctx;

  try {
    ctx = new nogdb::Context{DATABASE_PATH};
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  init_edge_bridge();

  try {
    auto txn = std::unique_ptr<nogdb::Txn>(new ctx->beginTxn(nogdb::TxnMode::READ_WRITE));
    auto v1 = nogdb::Vertex::get(*txn, "islands", nogdb::Condition("name").eq("Koh Samui"));
    auto v2 = nogdb::Vertex::get(*txn, "islands", nogdb::Condition("name").eq("Koh Tao"));
    nogdb::Edge::create(*txn, "bridge", v1[0].descriptor, v2[0].descriptor, nogdb::Record{}.set("name", "red"));
    txn->commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  delete ctx;

  try {
    ctx = new nogdb::Context{DATABASE_PATH};
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = std::unique_ptr<nogdb::Txn>(new ctx->beginTxn(nogdb::TxnMode::READ_ONLY));
    auto resE = nogdb::Edge::get(*txn, "bridge", nogdb::Condition("name").eq("red"));
    assert(!resE.empty());
    auto res = nogdb::Edge::getSrcDst(*txn, resE[0].descriptor);
    assert(!res.empty());
    assert(res[0].record.get("name").toText() == "Koh Samui");
    assert(res[1].record.get("name").toText() == "Koh Tao");
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  destroy_edge_bridge();
  destroy_vertex_island();

}

void test_txn_invalid_operations() {
  init_vertex_island();
  init_edge_bridge();

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.commit();

    txn.addVertex("islands", nogdb::Record{}.set("name", "Koh Samui"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_TXN_COMPLETED, "NOGDB_TXN_COMPLETED");
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    txn.rollback();

    txn.addVertex("islands", nogdb::Record{}.set("name", "Koh Samui"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_TXN_COMPLETED, "NOGDB_TXN_COMPLETED");
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    txn.commit();

    txn.find("islands");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_TXN_COMPLETED, "NOGDB_TXN_COMPLETED");
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    txn.rollback();

    txn.find("islands");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_TXN_COMPLETED, "NOGDB_TXN_COMPLETED");
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    txn.addVertex("islands", nogdb::Record{}.set("name", "Koh Samui"));
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_TXN_INVALID_MODE, "NOGDB_TXN_INVALID_MODE");
  }

  destroy_edge_bridge();
  destroy_vertex_island();
}
//
//void test_txn_invalid_concurrent_version() {
//    init_vertex_island();
//    init_edge_bridge();
//
//    try {
//        auto txnRo = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
//        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
//        auto v1 = txn.addVertex("islands", nogdb::Record{}.set("name", "Koh Manao"));
//        auto v2 = txn.addVertex("islands", nogdb::Record{}.set("name", "Koh Som O"));
//        auto v3 = txn.addVertex("islands", nogdb::Record{}.set("name", "Koh Satang"));
//        auto v4 = txn.addVertex("islands", nogdb::Record{}.set("name", "Koh Nang"));
//        auto e = txn.addEdge("bridge", v1, v2, nogdb::Record{}.set("name", "Grand II"));
//        txn.commit();
//
//        auto vertices = std::vector<decltype(v1)>{v1, v2, v3};
//        for(int i = 0; i < 128; ++i) {
//            txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
//            txn.updateSrc(e, vertices[i%vertices.size()]);
//            txn.commit();
//        }
//
//        txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
//        txn.updateSrc(e, v4);
//
//        txn.commit();
//        assert(false);
//    } catch (const nogdb::Error& ex) {
//        REQUIRE(ex, TXN_VERSION_NOMEM, "TXN_VERSION_NOMEM");
//    }
//
//    destroy_edge_bridge();
//    destroy_vertex_island();
//}


