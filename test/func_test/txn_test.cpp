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

void test_txn_commit_nothing()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_create_only_vertex_commit()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        nogdb::Record r {};
        r.set("name", "Koh Chang").set("area", "212.34");
        txnRw1.addVertex("islands", r);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.find("islands").where(nogdb::Condition("name").eq("Koh Chang")).get();
        assert(!res.empty());
        assert(res[0].record.get("name").toText() == "Koh Chang");

        res = txnRo1.find("islands").where(nogdb::Condition("name").eq("Koh Chang")).get();
        assert(res.empty());
        res = txnRo2.find("islands").where(nogdb::Condition("name").eq("Koh Chang")).get();
        assert(res.empty());
        res = txnRo3.find("islands").where(nogdb::Condition("name").eq("Koh Chang")).get();
        assert(res.empty());

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRw2.find("islands").where(nogdb::Condition("name").eq("Koh Chang")).get();
        assert(!res.empty());
        assert(res[0].record.get("name").toText() == "Koh Chang");
        res = txnRo4.find("islands").where(nogdb::Condition("name").eq("Koh Chang")).get();
        assert(!res.empty());
        assert(res[0].record.get("name").toText() == "Koh Chang");

        res = txnRo1.find("islands").where(nogdb::Condition("name").eq("Koh Chang")).get();
        assert(res.empty());
        res = txnRo2.find("islands").where(nogdb::Condition("name").eq("Koh Chang")).get();
        assert(res.empty());
        res = txnRo3.find("islands").where(nogdb::Condition("name").eq("Koh Chang")).get();
        assert(res.empty());

        txnRo1.commit();
        txnRo2.commit();
        txnRo3.commit();

        txnRo4.rollback();
        txnRw2.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_create_only_vertex_rollback()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        nogdb::Record r {};
        r.set("name", "Koh Mak").set("area", "87.92");
        txnRw1.addVertex("islands", r);
        auto res = txnRw1.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(!res.empty());
        assert(res[0].record.get("name").toText() == "Koh Mak");

        txnRw1.rollback();

        auto txnRo = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw00 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        res = txnRo.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(res.empty());
        res = txnRw00.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(res.empty());

        txnRo.commit();
        txnRw00.commit();

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_rollback_when_destroy()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        nogdb::Record r {};
        r.set("name", "Koh Mak").set("area", "87.92");
        txnRw.addVertex("islands", r);
        auto res = txnRw.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(!res.empty());
        assert(res[0].record.get("name").toText() == "Koh Mak");
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRo = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto res = txnRo.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(res.empty());
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_delete_only_vertex_commit()
{
    init_vertex_island();
    init_edge_bridge();

    auto vdesc = nogdb::RecordDescriptor {};
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        nogdb::Record r {};
        r.set("name", "Koh Mak").set("area", "87.92");
        vdesc = txnRw.addVertex("islands", r);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.remove(vdesc);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(res.empty());

        res = txnRo1.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(!res.empty());
        assert(res[0].record.get("name").toText() == "Koh Mak");
        res = txnRo2.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(!res.empty());
        assert(res[0].record.get("name").toText() == "Koh Mak");
        res = txnRo3.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(!res.empty());
        assert(res[0].record.get("name").toText() == "Koh Mak");

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        res = txnRo4.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(res.empty());
        res = txnRw2.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(res.empty());

        res = txnRo1.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(!res.empty());
        assert(res[0].record.get("name").toText() == "Koh Mak");
        res = txnRo2.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(!res.empty());
        assert(res[0].record.get("name").toText() == "Koh Mak");
        res = txnRo3.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(!res.empty());
        assert(res[0].record.get("name").toText() == "Koh Mak");

        txnRo1.commit();
        txnRo2.commit();
        txnRo3.commit();

        txnRo4.rollback();
        txnRw2.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_delete_only_vertex_rollback()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto vdesc = txnRw0.addVertex("islands", nogdb::Record {}.set("name", "Koh Mak").set("area", "87.92"));
        txnRw0.commit();

        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw1.remove(vdesc);
        auto res = txnRw1.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(res.empty());
        txnRw1.rollback();

        auto txnRo = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        res = txnRo.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(!res.empty());
        assert(res[0].record.get("name").toText() == "Koh Mak");
        res = txnRw2.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(!res.empty());
        assert(res[0].record.get("name").toText() == "Koh Mak");

        txnRo.commit();
        txnRw2.commit();

        auto txnRw00 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw00.remove(vdesc);
        txnRw00.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_create_only_edge_commit()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto vdesc1 = txnRw1.addVertex("islands", nogdb::Record {}.set("name", "Koh Kood").set("area", "145.32"));
        auto vdesc2 = txnRw1.addVertex("islands", nogdb::Record {}.set("name", "Koh Mak").set("area", "87.92"));
        txnRw1.addEdge("bridge", vdesc1, vdesc2, nogdb::Record {}.set("name", "yellow"));

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.find("islands").where(nogdb::Condition("name").eq("Koh Kood")).get();
        assert(!res.empty());
        res = txnRw1.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(!res.empty());
        auto resE = txnRw1.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(!resE.empty());

        resE = txnRo1.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(resE.empty());
        resE = txnRo2.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(resE.empty());
        resE = txnRo3.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(resE.empty());

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        resE = txnRo4.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(!resE.empty());
        resE = txnRw2.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(!resE.empty());

        resE = txnRo1.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(resE.empty());
        resE = txnRo2.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(resE.empty());
        resE = txnRo3.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(resE.empty());

        txnRo1.commit();
        txnRo2.commit();
        txnRo3.commit();

        txnRo4.rollback();
        txnRw2.rollback();

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_create_only_edge_rollback()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto vdesc1 = txnRw1.addVertex("islands", nogdb::Record {}.set("name", "Koh Kood").set("area", "145.32"));
        auto vdesc2 = txnRw1.addVertex("islands", nogdb::Record {}.set("name", "Koh Mak").set("area", "87.92"));
        txnRw1.addEdge("bridge", vdesc1, vdesc2, nogdb::Record {}.set("name", "yellow"));

        auto res = txnRw1.find("islands").where(nogdb::Condition("name").eq("Koh Kood")).get();
        assert(!res.empty());
        res = txnRw1.find("islands").where(nogdb::Condition("name").eq("Koh Mak")).get();
        assert(!res.empty());
        auto resE = txnRw1.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(!resE.empty());

        txnRw1.rollback();

        auto txnRo = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw00 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        resE = txnRo.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(resE.empty());
        resE = txnRw00.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(resE.empty());

        txnRo.commit();
        txnRw00.commit();

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_delete_only_edge_commit()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto vdesc1 = txnRw.addVertex("islands", nogdb::Record {}.set("name", "Koh Kood").set("area", "145.32"));
        auto vdesc2 = txnRw.addVertex("islands", nogdb::Record {}.set("name", "Koh Mak").set("area", "87.92"));
        txnRw.addEdge("bridge", vdesc1, vdesc2, nogdb::Record {}.set("name", "yellow"));
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto edesc = txnRw1.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        txnRw1.remove(edesc[0].descriptor);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto resE = txnRw1.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(resE.empty());

        resE = txnRo1.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(!resE.empty());
        resE = txnRo2.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(!resE.empty());
        resE = txnRo3.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(!resE.empty());

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        resE = txnRo4.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(resE.empty());
        resE = txnRw2.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(resE.empty());

        resE = txnRo1.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(!resE.empty());
        resE = txnRo2.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(!resE.empty());
        resE = txnRo3.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(!resE.empty());

        txnRo1.commit();
        txnRo2.commit();
        txnRo3.commit();

        txnRo4.rollback();
        txnRw2.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_delete_only_edge_rollback()
{
    init_vertex_island();
    init_edge_bridge();

    auto vdesc1 = nogdb::RecordDescriptor {};
    auto vdesc2 = nogdb::RecordDescriptor {};
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        vdesc1 = txnRw.addVertex("islands", nogdb::Record {}.set("name", "Koh Kood").set("area", "145.32"));
        vdesc2 = txnRw.addVertex("islands", nogdb::Record {}.set("name", "Koh Mak").set("area", "87.92"));
        txnRw.addEdge("bridge", vdesc1, vdesc2, nogdb::Record {}.set("name", "yellow"));
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto edesc = txnRw1.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        txnRw1.remove(edesc[0].descriptor);
        auto resE = txnRw1.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(resE.empty());

        txnRw1.rollback();

        auto txnRo = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        resE = txnRo.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(!resE.empty());
        resE = txnRw2.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(!resE.empty());

        txnRo.commit();
        txnRw2.commit();

        auto txnRw00 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw00.remove(vdesc1);
        txnRw00.remove(vdesc2);
        resE = txnRw00.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(resE.empty());

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw00.commit();

        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        resE = txnRo1.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(!resE.empty());
        resE = txnRo2.find("bridge").where(nogdb::Condition("name").eq("yellow")).get();
        assert(resE.empty());

        txnRo1.rollback();
        txnRo2.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_get_vertex_edge()
{
    init_vertex_island();
    init_edge_bridge();
    init_edge_flight();

    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto v1 = txnRw1.addVertex("islands", nogdb::Record {}.set("name", "1"));
        auto v2 = txnRw1.addVertex("islands", nogdb::Record {}.set("name", "2"));
        auto v3 = txnRw1.addVertex("islands", nogdb::Record {}.set("name", "3"));
        auto e1 = txnRw1.addEdge("bridge", v1, v2, nogdb::Record {}.set("name", "12"));
        auto e2 = txnRw1.addEdge("flight", v1, v3, nogdb::Record {}.set("name", "13"));

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        std::vector<std::function<void(nogdb::Transaction&)>> testCases { [&](const nogdb::Transaction& txn) {
                                                                             auto res = txn.fetchSrc(e1);
                                                                             assert(res.record.get("name").toText()
                                                                                 == "1");
                                                                         },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchDst(e1);
                assert(res.record.get("name").toText() == "2");
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchSrc(e2);
                assert(res.record.get("name").toText() == "1");
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchDst(e2);
                assert(res.record.get("name").toText() == "3");
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findInEdge(v2).get();
                assert(res[0].record.get("name").toText() == "12");
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findInEdge(v3).get();
                assert(res[0].record.get("name").toText() == "13");
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findOutEdge(v1).get();
                ASSERT_SIZE(res, 2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findOutEdge(v1).where(nogdb::GraphFilter {}.only("bridge")).get();
                assert(res[0].record.get("name").toText() == "12");
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findOutEdge(v1).where(nogdb::GraphFilter {}.only("flight")).get();
                assert(res[0].record.get("name").toText() == "13");
            } };

        runTestCases(txnRw1, testCases, true);
        runTestCases(txnRo1, testCases, false);
        runTestCases(txnRo2, testCases, false);
        runTestCases(txnRo3, testCases, false);

        txnRw1.commit();

        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        runTestCases(txnRw2, testCases, true);
        runTestCases(txnRo4, testCases, true);

        runTestCases(txnRo1, testCases, false);
        runTestCases(txnRo2, testCases, false);
        runTestCases(txnRo3, testCases, false);
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_flight();
    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_alter_vertex_edge_commit()
{
    init_vertex_island();
    init_edge_bridge();
    init_edge_flight();

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto v1 = txnRw0.addVertex("islands", nogdb::Record {}.set("name", "1"));
        auto v2 = txnRw0.addVertex("islands", nogdb::Record {}.set("name", "2"));
        auto v3 = txnRw0.addVertex("islands", nogdb::Record {}.set("name", "3"));
        auto e1 = txnRw0.addEdge("bridge", v1, v2, nogdb::Record {}.set("name", "12"));
        auto e2 = txnRw0.addEdge("flight", v1, v3, nogdb::Record {}.set("name", "13"));

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.updateSrc(e1, v3);
        txnRw1.updateDst(e2, v2);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        std::vector<std::function<void(nogdb::Transaction&)>> oldTestCases { [&](const nogdb::Transaction& txn) {
                                                                                auto res = txn.fetchSrc(e1);
                                                                                assert(res.descriptor == v1);
                                                                            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchSrc(e2);
                assert(res.descriptor == v1);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchDst(e1);
                assert(res.descriptor == v2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchDst(e2);
                assert(res.descriptor == v3);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findInEdge(v2).get();
                assert(res[0].descriptor == e1);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findInEdge(v3).get();
                assert(res[0].descriptor == e2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findOutEdge(v1).get();
                ASSERT_SIZE(res, 2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findOutEdge(v1).where(nogdb::GraphFilter {}.only("bridge")).get();
                assert(res[0].descriptor == e1);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findOutEdge(v1).where(nogdb::GraphFilter {}.only("flight")).get();
                assert(res[0].descriptor == e2);
            } };

        std::vector<std::function<void(nogdb::Transaction&)>> newTestCases { [&](const nogdb::Transaction& txn) {
                                                                                auto res = txn.fetchSrc(e1);
                                                                                assert(res.descriptor == v3);
                                                                            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchSrc(e2);
                assert(res.descriptor == v1);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchDst(e1);
                assert(res.descriptor == v2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchDst(e2);
                assert(res.descriptor == v2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findInEdge(v2).get();
                ASSERT_SIZE(res, 2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findOutEdge(v3).get();
                assert(res[0].descriptor == e1);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findOutEdge(v1).get();
                assert(res[0].descriptor == e2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findInEdge(v2).where(nogdb::GraphFilter {}.only("bridge")).get();
                assert(res[0].descriptor == e1);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findInEdge(v2).where(nogdb::GraphFilter {}.only("flight")).get();
                assert(res[0].descriptor == e2);
            } };

        runTestCases(txnRw1, newTestCases, true);
        runTestCases(txnRo1, oldTestCases, true);
        runTestCases(txnRo2, oldTestCases, true);
        runTestCases(txnRo3, oldTestCases, true);

        txnRw1.commit();

        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        runTestCases(txnRo4, newTestCases, true);
        runTestCases(txnRw2, newTestCases, true);

        runTestCases(txnRo1, oldTestCases, true);
        runTestCases(txnRo2, oldTestCases, true);
        runTestCases(txnRo3, oldTestCases, true);
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_flight();
    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_alter_vertex_edge_rollback()
{
    init_vertex_island();
    init_edge_bridge();
    init_edge_flight();

    nogdb::RecordDescriptor v1, v2, v3, e1, e2;
    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        v1 = txnRw0.addVertex("islands", nogdb::Record {}.set("name", "1"));
        v2 = txnRw0.addVertex("islands", nogdb::Record {}.set("name", "2"));
        v3 = txnRw0.addVertex("islands", nogdb::Record {}.set("name", "3"));
        e1 = txnRw0.addEdge("bridge", v3, v2, nogdb::Record {}.set("name", "32"));
        e2 = txnRw0.addEdge("flight", v1, v2, nogdb::Record {}.set("name", "12"));
        txnRw0.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw1.updateSrc(e1, v1);
        txnRw1.updateDst(e2, v3);

        std::vector<std::function<void(nogdb::Transaction&)>> newTestCases { [&](const nogdb::Transaction& txn) {
                                                                                auto res = txn.fetchSrc(e1);
                                                                                assert(res.descriptor == v1);
                                                                            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchSrc(e2);
                assert(res.descriptor == v1);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchDst(e1);
                assert(res.descriptor == v2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchDst(e2);
                assert(res.descriptor == v3);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findInEdge(v2).get();
                assert(res[0].descriptor == e1);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findInEdge(v3).get();
                assert(res[0].descriptor == e2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findOutEdge(v1).get();
                ASSERT_SIZE(res, 2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findOutEdge(v1).where(nogdb::GraphFilter {}.only("bridge")).get();
                assert(res[0].descriptor == e1);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findOutEdge(v1).where(nogdb::GraphFilter {}.only("flight")).get();
                assert(res[0].descriptor == e2);
            } };

        std::vector<std::function<void(nogdb::Transaction&)>> oldTestCases { [&](const nogdb::Transaction& txn) {
                                                                                auto res = txn.fetchSrc(e1);
                                                                                assert(res.descriptor == v3);
                                                                            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchSrc(e2);
                assert(res.descriptor == v1);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchDst(e1);
                assert(res.descriptor == v2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.fetchDst(e2);
                assert(res.descriptor == v2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findInEdge(v2).get();
                ASSERT_SIZE(res, 2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findOutEdge(v3).get();
                assert(res[0].descriptor == e1);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findOutEdge(v1).get();
                assert(res[0].descriptor == e2);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findInEdge(v2).where(nogdb::GraphFilter {}.only("bridge")).get();
                assert(res[0].descriptor == e1);
            },
            [&](const nogdb::Transaction& txn) {
                auto res = txn.findInEdge(v2).where(nogdb::GraphFilter {}.only("flight")).get();
                assert(res[0].descriptor == e2);
            } };

        runTestCases(txnRw1, newTestCases, true);

        txnRw1.rollback();

        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        runTestCases(txnRw2, oldTestCases, true);
        runTestCases(txnRo, oldTestCases, true);
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_flight();
    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_create_only_vertex_multiversion_commit()
{
    init_vertex_island();
    init_edge_bridge();
    init_edge_flight();

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.addVertex("islands", nogdb::Record {}.set("name", "Koh Samed"));
        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addVertex("islands", nogdb::Record {}.set("name", "Koh Phe Phe"));

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto res = txnRo0.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(res.empty());
        res = txnRo0.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(res.empty());

        res = txnRo1.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo1.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(res.empty());

        res = txnRo2.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo2.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(res.empty());

        res = txnRo3.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo3.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(res.empty());

        res = txnRw2.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRw2.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(!res.empty());

        res = txnRo4.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo4.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(!res.empty());
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_flight();
    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_create_only_vertex_multiversion_rollback()
{
    init_vertex_island();
    init_edge_bridge();
    init_edge_flight();

    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addVertex("islands", nogdb::Record {}.set("name", "Koh Tarutao"));
        txnRw.commit();

        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw0.addVertex("islands", nogdb::Record {}.set("name", "Koh Samed"));
        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addVertex("islands", nogdb::Record {}.set("name", "Koh Phe Phe"));

        auto res = txnRw1.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRw1.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(!res.empty());

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRo0.find("islands").where(nogdb::Condition("name").eq("Koh Tarutao")).get();
        assert(!res.empty());
        res = txnRo0.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(res.empty());
        res = txnRo0.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(res.empty());

        res = txnRo1.find("islands").where(nogdb::Condition("name").eq("Koh Tarutao")).get();
        assert(!res.empty());
        res = txnRo1.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo1.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(res.empty());

        res = txnRo2.find("islands").where(nogdb::Condition("name").eq("Koh Tarutao")).get();
        assert(!res.empty());
        res = txnRo2.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo2.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(res.empty());

        res = txnRo3.find("islands").where(nogdb::Condition("name").eq("Koh Tarutao")).get();
        assert(!res.empty());
        res = txnRo3.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo3.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(res.empty());

        res = txnRo4.find("islands").where(nogdb::Condition("name").eq("Koh Tarutao")).get();
        assert(!res.empty());
        res = txnRo4.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo4.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(res.empty());

        res = txnRw2.find("islands").where(nogdb::Condition("name").eq("Koh Tarutao")).get();
        assert(!res.empty());
        res = txnRw2.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRw2.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(res.empty());
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_flight();
    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_delete_only_vertex_multiversion_commit()
{
    init_vertex_island();
    init_edge_bridge();
    init_edge_flight();

    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto v1 = txnRw.addVertex("islands", nogdb::Record {}.set("name", "Koh Samed"));
        txnRw.commit();

        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto v2 = txnRw0.addVertex("islands", nogdb::Record {}.set("name", "Koh Phe Phe"));
        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.remove(v2);

        auto res = txnRw1.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRw1.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(res.empty());

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRo1.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo1.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(!res.empty());
        res = txnRo2.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo2.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(!res.empty());
        res = txnRo3.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo3.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(!res.empty());
        res = txnRo4.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo4.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(res.empty());
        res = txnRw2.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRw2.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(res.empty());
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_flight();
    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_delete_only_vertex_multiversion_rollback()
{
    init_vertex_island();
    init_edge_bridge();
    init_edge_flight();

    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto v1 = txnRw.addVertex("islands", nogdb::Record {}.set("name", "Koh Samed"));
        txnRw.commit();

        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto v2 = txnRw0.addVertex("islands", nogdb::Record {}.set("name", "Koh Phe Phe"));
        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.remove(v2);

        auto res = txnRw1.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRw1.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(res.empty());

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRo1.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo1.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(!res.empty());
        res = txnRo2.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo2.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(!res.empty());
        res = txnRo3.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo3.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(!res.empty());
        res = txnRo4.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRo4.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(!res.empty());
        res = txnRw2.find("islands").where(nogdb::Condition("name").eq("Koh Samed")).get();
        assert(!res.empty());
        res = txnRw2.find("islands").where(nogdb::Condition("name").eq("Koh Phe Phe")).get();
        assert(!res.empty());
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_flight();
    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_create_edges_multiversion_commit()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txnRw00 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto v1 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Samed"));
        auto v2 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Phe PHe"));
        auto v3 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Tao"));
        txnRw00.addEdge("bridge", v1, v2, nogdb::Record {}.set("name", "bridge 12"));
        txnRw00.addEdge("bridge", v2, v1, nogdb::Record {}.set("name", "bridge 21"));

        txnRw00.commit();

        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.addEdge("bridge", v2, v3, nogdb::Record {}.set("name", "bridge 23"));

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addEdge("bridge", v1, v3, nogdb::Record {}.set("name", "bridge 13"));

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        std::vector<std::function<void(nogdb::Transaction&)>> testCasesV0, testCasesV1, testCasesV2;
        testCasesV0.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 23")).get();
            assert(res.empty());
        });
        testCasesV0.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v2).get();
            ASSERT_SIZE(res, 1);
            assert(res[0].record.get("name").toText() == "bridge 21");
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 13")).get();
            assert(res.empty());
            res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 23")).get();
            assert(!res.empty());
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v3).get();
            ASSERT_SIZE(res, 1);
            assert(res[0].record.get("name").toText() == "bridge 23");
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v1).get();
            ASSERT_SIZE(res, 1);
            assert(res[0].record.get("name").toText() == "bridge 12");
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v2).get();
            ASSERT_SIZE(res, 2);
        });
        testCasesV2.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 13")).get();
            assert(!res.empty());
            res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 23")).get();
            assert(!res.empty());
        });
        testCasesV2.push_back([&](nogdb::Transaction& txn) {
            auto e = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 13")).get();
            auto res = txn.fetchSrc(e[0].descriptor);
            assert(res.descriptor == v1);
            res = txn.fetchDst(e[0].descriptor);
            assert(res.descriptor == v3);
        });
        testCasesV2.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v3).get();
            ASSERT_SIZE(res, 2);
        });
        testCasesV2.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v1).get();
            ASSERT_SIZE(res, 2);
        });
        testCasesV2.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v2).get();
            ASSERT_SIZE(res, 2);
        });

        runTestCases(txnRo0, testCasesV0, true);
        runTestCases(txnRo1, testCasesV1, true);
        runTestCases(txnRo2, testCasesV1, true);
        runTestCases(txnRo3, testCasesV1, true);
        runTestCases(txnRo4, testCasesV2, true);
        runTestCases(txnRw2, testCasesV2, true);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_create_edges_multiversion_rollback()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txnRw00 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto v1 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Samed"));
        auto v2 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Phe PHe"));
        auto v3 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Tao"));
        txnRw00.addEdge("bridge", v1, v2, nogdb::Record {}.set("name", "bridge 12"));
        txnRw00.addEdge("bridge", v2, v1, nogdb::Record {}.set("name", "bridge 21"));

        txnRw00.commit();

        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        txnRw0.addEdge("bridge", v2, v3, nogdb::Record {}.set("name", "bridge 23"));

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addEdge("bridge", v1, v3, nogdb::Record {}.set("name", "bridge 13"));

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        std::vector<std::function<void(nogdb::Transaction&)>> testCasesV1;
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 13")).get();
            assert(res.empty());
            res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 23")).get();
            assert(!res.empty());
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v3).get();
            ASSERT_SIZE(res, 1);
            assert(res[0].record.get("name").toText() == "bridge 23");
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v1).get();
            ASSERT_SIZE(res, 1);
            assert(res[0].record.get("name").toText() == "bridge 12");
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v2).get();
            ASSERT_SIZE(res, 2);
        });

        runTestCases(txnRo1, testCasesV1, true);
        runTestCases(txnRo2, testCasesV1, true);
        runTestCases(txnRo3, testCasesV1, true);
        runTestCases(txnRo4, testCasesV1, true);
        runTestCases(txnRw2, testCasesV1, true);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_delete_edges_multiversion_commit()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txnRw00 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto v1 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Samed"));
        auto v2 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Phe PHe"));
        auto v3 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Tao"));
        auto e1 = txnRw00.addEdge("bridge", v1, v2, nogdb::Record {}.set("name", "bridge 12"));
        auto e2 = txnRw00.addEdge("bridge", v2, v1, nogdb::Record {}.set("name", "bridge 21"));
        auto e3 = txnRw00.addEdge("bridge", v2, v3, nogdb::Record {}.set("name", "bridge 23"));
        auto e4 = txnRw00.addEdge("bridge", v1, v3, nogdb::Record {}.set("name", "bridge 13"));

        txnRw00.commit();

        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.remove(e1);

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.remove(v3);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        std::vector<std::function<void(nogdb::Transaction&)>> testCasesV0, testCasesV1, testCasesV2;
        testCasesV0.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 13")).get();
            assert(!res.empty());
            res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 23")).get();
            assert(!res.empty());
            res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 12")).get();
            assert(!res.empty());
        });
        testCasesV0.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v3).get();
            ASSERT_SIZE(res, 2);
        });
        testCasesV0.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v1).get();
            ASSERT_SIZE(res, 2);
        });
        testCasesV0.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v2).get();
            ASSERT_SIZE(res, 2);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 13")).get();
            assert(!res.empty());
            res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 23")).get();
            assert(!res.empty());
            res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 12")).get();
            assert(res.empty());
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v3).get();
            ASSERT_SIZE(res, 2);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v1).get();
            ASSERT_SIZE(res, 1);
            assert(res[0].descriptor == e4);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v2).get();
            ASSERT_SIZE(res, 0);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v2).get();
            ASSERT_SIZE(res, 2);
        });
        testCasesV2.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 13")).get();
            assert(res.empty());
            res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 23")).get();
            assert(res.empty());
            res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 12")).get();
            assert(res.empty());
            res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 21")).get();
            assert(!res.empty());
        });
        testCasesV2.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v1).get();
            ASSERT_SIZE(res, 0);
        });
        testCasesV2.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v2).get();
            ASSERT_SIZE(res, 0);
        });
        testCasesV2.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v2).get();
            ASSERT_SIZE(res, 1);
            assert(res[0].descriptor == e2);
        });

        runTestCases(txnRo0, testCasesV0, true);
        runTestCases(txnRo1, testCasesV1, true);
        runTestCases(txnRo2, testCasesV1, true);
        runTestCases(txnRo3, testCasesV1, true);
        runTestCases(txnRo4, testCasesV2, true);
        runTestCases(txnRw2, testCasesV2, true);
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_delete_edges_multiversion_rollback()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txnRw00 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto v1 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Samed"));
        auto v2 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Phe PHe"));
        auto v3 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Tao"));
        auto e1 = txnRw00.addEdge("bridge", v1, v2, nogdb::Record {}.set("name", "bridge 12"));
        auto e2 = txnRw00.addEdge("bridge", v2, v1, nogdb::Record {}.set("name", "bridge 21"));
        auto e3 = txnRw00.addEdge("bridge", v2, v3, nogdb::Record {}.set("name", "bridge 23"));
        auto e4 = txnRw00.addEdge("bridge", v1, v3, nogdb::Record {}.set("name", "bridge 13"));

        txnRw00.commit();

        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        txnRw0.remove(e1);

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.remove(v3);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        std::vector<std::function<void(nogdb::Transaction&)>> testCasesV1;
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 13")).get();
            assert(!res.empty());
            res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 23")).get();
            assert(!res.empty());
            res = txn.find("bridge").where(nogdb::Condition("name").eq("bridge 12")).get();
            assert(res.empty());
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v3).get();
            ASSERT_SIZE(res, 2);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v1).get();
            ASSERT_SIZE(res, 1);
            assert(res[0].descriptor == e4);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v2).get();
            ASSERT_SIZE(res, 0);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v2).get();
            ASSERT_SIZE(res, 2);
        });

        runTestCases(txnRo1, testCasesV1, true);
        runTestCases(txnRo2, testCasesV1, true);
        runTestCases(txnRo3, testCasesV1, true);
        runTestCases(txnRo4, testCasesV1, true);
        runTestCases(txnRw2, testCasesV1, true);
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_modify_edges_multiversion_commit()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txnRw00 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto v1 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Samed"));
        auto v2 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Phe PHe"));
        auto v3 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Tao"));
        auto e1 = txnRw00.addEdge("bridge", v1, v2, nogdb::Record {}.set("name", "bridge 12"));

        txnRw00.commit();

        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.updateDst(e1, v3);

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.updateSrc(e1, v2);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        std::vector<std::function<void(nogdb::Transaction&)>> testCasesV0, testCasesV1, testCasesV2;
        testCasesV0.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.fetchSrc(e1);
            assert(res.descriptor == v1);
            res = txn.fetchDst(e1);
            assert(res.descriptor == v2);
        });
        testCasesV0.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v1).get();
            ASSERT_SIZE(res, 1);
        });
        testCasesV0.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v2).get();
            ASSERT_SIZE(res, 1);
        });
        testCasesV0.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v2).get();
            ASSERT_SIZE(res, 0);
        });
        testCasesV0.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v3).get();
            ASSERT_SIZE(res, 0);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.fetchSrc(e1);
            assert(res.descriptor == v1);
            res = txn.fetchDst(e1);
            assert(res.descriptor == v3);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v1).get();
            ASSERT_SIZE(res, 1);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v2).get();
            ASSERT_SIZE(res, 0);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v2).get();
            ASSERT_SIZE(res, 0);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v3).get();
            ASSERT_SIZE(res, 1);
        });
        testCasesV2.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.fetchSrc(e1);
            assert(res.descriptor == v2);
            res = txn.fetchDst(e1);
            assert(res.descriptor == v3);
        });
        testCasesV2.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v1).get();
            ASSERT_SIZE(res, 0);
        });
        testCasesV2.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v2).get();
            ASSERT_SIZE(res, 0);
        });
        testCasesV2.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v2).get();
            ASSERT_SIZE(res, 1);
        });
        testCasesV2.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v3).get();
            ASSERT_SIZE(res, 1);
        });

        runTestCases(txnRo0, testCasesV0, true);
        runTestCases(txnRo1, testCasesV1, true);
        runTestCases(txnRo2, testCasesV1, true);
        runTestCases(txnRo3, testCasesV1, true);
        runTestCases(txnRo4, testCasesV2, true);
        runTestCases(txnRw2, testCasesV2, true);
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_modify_edges_multiversion_rollback()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txnRw00 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto v1 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Samed"));
        auto v2 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Phe PHe"));
        auto v3 = txnRw00.addVertex("islands", nogdb::Record {}.set("name", "Koh Tao"));
        auto e1 = txnRw00.addEdge("bridge", v1, v2, nogdb::Record {}.set("name", "bridge 12"));

        txnRw00.commit();

        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        txnRw0.updateDst(e1, v3);

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.updateSrc(e1, v2);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        std::vector<std::function<void(nogdb::Transaction&)>> testCasesV1;
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.fetchSrc(e1);
            assert(res.descriptor == v1);
            res = txn.fetchDst(e1);
            assert(res.descriptor == v3);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v1).get();
            ASSERT_SIZE(res, 1);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v2).get();
            ASSERT_SIZE(res, 0);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findOutEdge(v2).get();
            ASSERT_SIZE(res, 0);
        });
        testCasesV1.push_back([&](nogdb::Transaction& txn) {
            auto res = txn.findInEdge(v3).get();
            ASSERT_SIZE(res, 1);
        });

        runTestCases(txnRo1, testCasesV1, true);
        runTestCases(txnRo2, testCasesV1, true);
        runTestCases(txnRo3, testCasesV1, true);
        runTestCases(txnRo4, testCasesV1, true);
        runTestCases(txnRw2, testCasesV1, true);
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_reopen_ctx()
{
    init_vertex_island();

    try {
        auto txn
            = std::unique_ptr<nogdb::Transaction>(new nogdb::Transaction(ctx->beginTxn(nogdb::TxnMode::READ_WRITE)));
        auto v1 = txn->addVertex("islands", nogdb::Record {}.set("name", "Koh Samui"));
        auto v2 = txn->addVertex("islands", nogdb::Record {}.set("name", "Koh Tao"));
        txn->commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    delete ctx;

    try {
        ctx = new nogdb::Context { DATABASE_PATH };
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    init_edge_bridge();

    try {
        auto txn
            = std::unique_ptr<nogdb::Transaction>(new nogdb::Transaction(ctx->beginTxn(nogdb::TxnMode::READ_WRITE)));
        auto v1 = txn->find("islands").where(nogdb::Condition("name").eq("Koh Samui")).get();
        auto v2 = txn->find("islands").where(nogdb::Condition("name").eq("Koh Tao")).get();
        txn->addEdge("bridge", v1[0].descriptor, v2[0].descriptor, nogdb::Record {}.set("name", "red"));
        txn->commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    delete ctx;

    try {
        ctx = new nogdb::Context { DATABASE_PATH };
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn
            = std::unique_ptr<nogdb::Transaction>(new nogdb::Transaction(ctx->beginTxn(nogdb::TxnMode::READ_ONLY)));
        auto resE = txn->find("bridge").where(nogdb::Condition("name").eq("red")).get();
        assert(!resE.empty());
        auto res = txn->fetchSrcDst(resE[0].descriptor);
        assert(!res.empty());
        assert(res[0].record.get("name").toText() == "Koh Samui");
        assert(res[1].record.get("name").toText() == "Koh Tao");
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}

void test_txn_invalid_operations()
{
    init_vertex_island();
    init_edge_bridge();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.commit();

        txn.addVertex("islands", nogdb::Record {}.set("name", "Koh Samui"));
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_TXN_COMPLETED, "NOGDB_TXN_COMPLETED");
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.rollback();

        txn.addVertex("islands", nogdb::Record {}.set("name", "Koh Samui"));
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_TXN_COMPLETED, "NOGDB_TXN_COMPLETED");
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        txn.commit();

        txn.find("islands").get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_TXN_COMPLETED, "NOGDB_TXN_COMPLETED");
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        txn.rollback();

        txn.find("islands").get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_TXN_COMPLETED, "NOGDB_TXN_COMPLETED");
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        txn.addVertex("islands", nogdb::Record {}.set("name", "Koh Samui"));
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_TXN_INVALID_MODE, "NOGDB_TXN_INVALID_MODE");
    }

    destroy_edge_bridge();
    destroy_vertex_island();
}