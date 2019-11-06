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
#include <set>

void init_test_graph()
{
    init_vertex_teachers();
    init_vertex_students();
    init_vertex_departments();
    init_vertex_subjects();
    init_edge_teach();
    init_edge_enrol();
    init_edge_know();
    init_edge_workfor();
    init_edge_belongto();
    init_vertex_folders();
    init_vertex_files();
    init_edge_link();
    init_edge_symbolic();
    init_vertex_country();
    init_edge_path();
}

void destroy_test_graph()
{
    destroy_edge_symbolic();
    destroy_edge_link();
    destroy_vertex_files();
    destroy_vertex_folders();
    destroy_edge_belongto();
    destroy_edge_workfor();
    destroy_edge_know();
    destroy_edge_enrol();
    destroy_edge_teach();
    destroy_vertex_subjects();
    destroy_vertex_departments();
    destroy_vertex_students();
    destroy_vertex_teachers();
    destroy_edge_path();
    destroy_vertex_country();
}

void test_create_complex_graph()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        nogdb::Record rt {};
        rt.set("name", "John").set("age", 52U).set("salary", 51000U).set("level", "Dr.");
        auto john = txn.addVertex("teachers", rt);
        rt.set("name", "Jim").set("age", 55U).set("salary", 46000U).set("level", "Asso.Prof.");
        auto jim = txn.addVertex("teachers", rt);
        rt.set("name", "Wei").set("age", 32U).set("salary", 65000U).set("level", "Prof.");
        auto wei = txn.addVertex("teachers", rt);

        nogdb::Record rs {};
        rs.set("name", "Peter").set("age", 42U).set("grade", 2.89);
        auto peter = txn.addVertex("students", rs);
        rs.set("name", "David").set("age", 40U).set("grade", 3.3);
        auto david = txn.addVertex("students", rs);
        rs.set("name", "Ying").set("age", 21U).set("grade", 3.01);
        auto ying = txn.addVertex("students", rs);
        rs.set("name", "Andy").set("age", 30U).set("grade", 3.43);
        auto andy = txn.addVertex("students", rs);
        rs.set("name", "Wong").set("age", 29U).set("grade", 3.78);
        auto wong = txn.addVertex("students", rs);
        rs.set("name", "Jessie").set("age", 27U).set("grade", 2.56);
        auto jessie = txn.addVertex("students", rs);

        nogdb::Record rd {};
        rd.set("name", "Computing");
        auto comp = txn.addVertex("departments", rd);
        rd.set("name", "Business");
        auto bus = txn.addVertex("departments", rd);

        nogdb::Record rsb {};
        rsb.set("name", "Programming");
        auto prog = txn.addVertex("subjects", rsb);
        rsb.set("name", "Database");
        auto db = txn.addVertex("subjects", rsb);
        rsb.set("name", "Networking");
        auto network = txn.addVertex("subjects", rsb);
        rsb.set("name", "Marketing");
        auto market = txn.addVertex("subjects", rsb);
        rsb.set("name", "Intro to Finance");
        auto fin = txn.addVertex("subjects", rsb);

        nogdb::Record rtch {};
        rtch.set("semester", "2016s1");
        txn.addEdge("teach", john, market, rtch);
        txn.addEdge("teach", jim, fin, rtch);
        txn.addEdge("teach", wei, db, rtch);
        rtch.set("semester", "2016s2");
        txn.addEdge("teach", john, fin, rtch);
        txn.addEdge("teach", jim, network, rtch);
        txn.addEdge("teach", wei, prog, rtch);

        nogdb::Record rb {};
        rb.set("null", "0");
        txn.addEdge("belongto", prog, comp, rb);
        txn.addEdge("belongto", db, comp, rb);
        txn.addEdge("belongto", network, comp, rb);
        txn.addEdge("belongto", market, bus, rb);
        txn.addEdge("belongto", fin, bus, rb);

        nogdb::Record rw {};
        rw.set("position", "officer");
        txn.addEdge("workfor", jim, comp, rw);
        txn.addEdge("workfor", jim, bus, rw);
        rw.set("position", "dean");
        txn.addEdge("workfor", john, bus, rw);
        txn.addEdge("workfor", wei, comp, rw);

        nogdb::Record rk {};
        rk.set("relationship", "friend");
        txn.addEdge("know", john, jim, rk);
        txn.addEdge("know", jim, john, rk);
        txn.addEdge("know", john, wei, rk);
        rk.set("relationship", "colleague");
        txn.addEdge("know", wei, john, rk);
        txn.addEdge("know", jim, wei, rk);
        txn.addEdge("know", wei, jim, rk);

        nogdb::Record re {};
        re.set("semester", "2016s1");
        txn.addEdge("enrol", jessie, market, re);
        txn.addEdge("enrol", ying, fin, re);
        txn.addEdge("enrol", peter, fin, re);
        txn.addEdge("enrol", david, fin, re);
        txn.addEdge("enrol", andy, db, re);
        txn.addEdge("enrol", wong, db, re);
        re.set("semester", "2016s2");
        txn.addEdge("enrol", jessie, fin, re);
        txn.addEdge("enrol", ying, prog, re);
        txn.addEdge("enrol", peter, prog, re);
        txn.addEdge("enrol", david, prog, re);
        txn.addEdge("enrol", andy, network, re);
        txn.addEdge("enrol", wong, network, re);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        nogdb::Record rf {}, rl {};
        auto A = txn.addVertex("folders", rf.set("name", "A"));
        auto B = txn.addVertex("folders", rf.set("name", "B"));
        auto C = txn.addVertex("folders", rf.set("name", "C"));
        auto D = txn.addVertex("folders", rf.set("name", "D"));
        auto E = txn.addVertex("folders", rf.set("name", "E"));
        auto F = txn.addVertex("folders", rf.set("name", "F"));
        auto G = txn.addVertex("folders", rf.set("name", "G"));
        auto H = txn.addVertex("folders", rf.set("name", "H"));
        auto Z = txn.addVertex("folders", rf.set("name", "Z"));

        auto a = txn.addVertex("files", rf.set("name", "a"));
        auto b = txn.addVertex("files", rf.set("name", "b"));
        auto c = txn.addVertex("files", rf.set("name", "c"));
        auto d = txn.addVertex("files", rf.set("name", "d"));
        auto e = txn.addVertex("files", rf.set("name", "e"));
        auto f = txn.addVertex("files", rf.set("name", "f"));

        //        rl.set("null", "0");
        txn.addEdge("link", A, B, rl);
        txn.addEdge("link", A, a, rl);
        txn.addEdge("link", A, C, rl);
        txn.addEdge("link", B, D, rl);
        txn.addEdge("link", B, E, rl);
        txn.addEdge("link", B, b, rl);
        txn.addEdge("link", C, c, rl);
        txn.addEdge("link", C, F, rl);
        txn.addEdge("link", E, G, rl);
        txn.addEdge("link", F, d, rl);
        txn.addEdge("link", F, H, rl);
        txn.addEdge("link", F, e, rl);
        txn.addEdge("link", G, f, rl);
        txn.addEdge("symbolic", B, b, rl);
        txn.addEdge("symbolic", C, e, rl);
        txn.addEdge("symbolic", D, A, rl);
        txn.addEdge("symbolic", D, f, rl);
        txn.addEdge("symbolic", E, F, rl);
        txn.addEdge("symbolic", H, C, rl);
        txn.addEdge("symbolic", a, a, rl);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto a = txn.addVertex("country", nogdb::Record {}.set("name", "A").set("population", 400ULL));

        auto b = txn.addVertex("country", nogdb::Record {}.set("name", "B").set("population", 1000ULL));

        auto c = txn.addVertex("country", nogdb::Record {}.set("name", "C").set("population", 2000ULL));

        auto d = txn.addVertex("country", nogdb::Record {}.set("name", "D").set("population", 5000ULL));

        auto e = txn.addVertex("country", nogdb::Record {}.set("name", "E").set("population", 500ULL));

        auto f = txn.addVertex("country", nogdb::Record {}.set("name", "F").set("population", 1500ULL));

        auto z = txn.addVertex("country", nogdb::Record {}.set("name", "Z").set("population", 500ULL));

        txn.addEdge("path", z, a, nogdb::Record {}.set("distance", 40U));
        txn.addEdge("path", a, b, nogdb::Record {}.set("distance", 50U));
        txn.addEdge("path", a, c, nogdb::Record {}.set("distance", 400U));
        txn.addEdge("path", b, e, nogdb::Record {}.set("distance", 250U));
        txn.addEdge("path", b, c, nogdb::Record {}.set("distance", 80U));
        txn.addEdge("path", c, d, nogdb::Record {}.set("distance", 100U));
        txn.addEdge("path", c, f, nogdb::Record {}.set("distance", 150U));
        txn.addEdge("path", d, a, nogdb::Record {}.set("distance", 300U));
        txn.addEdge("path", d, f, nogdb::Record {}.set("distance", 120U));
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_get_edge_in_more()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto edges = txn.findInEdge(teacher).where(nogdb::GraphFilter {}.only("workfor")).get();
            assert(edges.size() == 0);
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto get_class_name = [&](const nogdb::RecordDescriptor& rdesc) {
            auto class_desc = txn.getClass(rdesc.rid.first);
            return class_desc.name;
        };
        for (const auto& res : txn.find("subjects").get()) {
            auto subject = res.descriptor;
            auto edges = txn.findInEdge(subject).where(nogdb::GraphFilter {}.only("teach")).get();
            if (res.record.get("name").toText() == "Intro to Finance") {
                assert(edges.size() == 2);
            } else {
                assert(edges.size() == 1);
            }
            edges = txn.findInEdge(subject).where(nogdb::GraphFilter {}.only("enrol")).get();
            assert(edges.size() != 0);
            bool tmp1 = false, tmp2 = false;
            for (const auto& edge : txn.findInEdge(subject).get()) {
                assert(get_class_name(edge.descriptor) == "teach" || get_class_name(edge.descriptor) == "enrol");
                if (get_class_name(edge.descriptor) == "teach")
                    tmp1 |= true;
                if (get_class_name(edge.descriptor) == "enrol")
                    tmp2 |= true;
            }
            assert(tmp1 && tmp2);
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach");
            auto edges = txn.findInEdge(teacher).where(clsset).get();
            assert(edges.size() == 0);
        }

        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach");
            auto edges = txn.findInEdge(teacher).where(clsset).get();
            assert(edges.size() == 0);
        }

        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach");
            auto edges = txn.findInEdge(teacher).where(clsset).get();
            assert(edges.size() == 0);
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach", "know");
            auto edges = txn.findInEdge(teacher).where(clsset).get();
            assert(edges.size() == 2);
        }

        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto edges = txn.findInEdge(teacher).get();
            assert(edges.size() == 2);
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_get_edge_out_more()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto edges = txn.findOutEdge(teacher).where(nogdb::GraphFilter {}.only("workfor")).get();
            assert(edges.size() == 1 || edges.size() == 2);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges[0].record.get("position").toText() == "officer");
                assert(edges[1].record.get("position").toText() == "officer");
            } else if (res.record.get("name").toText() == "John") {
                assert(edges[0].record.get("position").toText() == "dean");
            } else if (res.record.get("name").toText() == "Wei") {
                assert(edges[0].record.get("position").toText() == "dean");
            } else {
                assert(false);
            }
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto& res : txn.find("subjects").get()) {
            auto subject = res.descriptor;
            auto edges = txn.findOutEdge(subject).where(nogdb::GraphFilter {}.only("belongto")).get();
            assert(edges.size() == 1);
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach");
            auto edges = txn.findOutEdge(teacher).where(clsset).get();
            assert(edges.size() == 3 || edges.size() == 4);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 4);
            } else {
                assert(edges.size() == 3);
            }
        }

        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach");
            auto edges = txn.findOutEdge(teacher).where(clsset).get();
            assert(edges.size() == 3 || edges.size() == 4);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 4);
            } else {
                assert(edges.size() == 3);
            }
        }

        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach");
            auto edges = txn.findOutEdge(teacher).where(clsset).get();
            assert(edges.size() == 3 || edges.size() == 4);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 4);
            } else {
                assert(edges.size() == 3);
            }
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach", "know");
            auto edges = txn.findOutEdge(teacher).where(clsset).get();
            assert(edges.size() == 5 || edges.size() == 6);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 6);
            } else {
                assert(edges.size() == 5);
            }
        }

        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto edges = txn.findOutEdge(teacher).get();
            assert(edges.size() == 5 || edges.size() == 6);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 6);
            } else {
                assert(edges.size() == 5);
            }
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_get_edge_all_more()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto edges = txn.findEdge(teacher).where(nogdb::GraphFilter {}.only("workfor")).get();
            assert(edges.size() == 1 || edges.size() == 2);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges[0].record.get("position").toText() == "officer");
                assert(edges[1].record.get("position").toText() == "officer");
            } else if (res.record.get("name").toText() == "John") {
                assert(edges[0].record.get("position").toText() == "dean");
            } else if (res.record.get("name").toText() == "Wei") {
                assert(edges[0].record.get("position").toText() == "dean");
            } else {
                assert(false);
            }
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach");
            auto edges = txn.findEdge(teacher).where(clsset).get();
            assert(edges.size() == 3 || edges.size() == 4);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 4);
            } else {
                assert(edges.size() == 3);
            }
        }

        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach");
            auto edges = txn.findEdge(teacher).where(clsset).get();
            assert(edges.size() == 3 || edges.size() == 4);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 4);
            } else {
                assert(edges.size() == 3);
            }
        }

        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach");
            auto edges = txn.findEdge(teacher).where(clsset).get();
            assert(edges.size() == 3 || edges.size() == 4);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 4);
            } else {
                assert(edges.size() == 3);
            }
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach", "know");
            auto edges = txn.findEdge(teacher).where(clsset).get();
            assert(edges.size() == 7 || edges.size() == 8);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 8);
            } else {
                assert(edges.size() == 7);
            }
        }

        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto edges = txn.findEdge(teacher).get();
            assert(edges.size() == 7 || edges.size() == 8);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 8);
            } else {
                assert(edges.size() == 7);
            }
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto& res : txn.find("subjects").get()) {
            auto subject = res.descriptor;
            auto edges = txn.findEdge(subject).get();
            if (res.record.get("name").toText() == "Intro to Finance") {
                assert(edges.size() == 7);
            } else if (res.record.get("name").toText() == "Marketing") {
                assert(edges.size() == 3);
            } else if (res.record.get("name").toText() == "Programming") {
                assert(edges.size() == 5);
            } else if (res.record.get("name").toText() == "Database") {
                assert(edges.size() == 4);
            } else if (res.record.get("name").toText() == "Networking") {
                assert(edges.size() == 4);
            } else {
                assert(false);
            }
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_get_invalid_edge_in_more()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("students").get()) {
            auto student = res.descriptor;
            auto edges = txn.findInEdge(student).where(nogdb::GraphFilter {}.only("attack")).get();
            ASSERT_SIZE(edges, 0);
        }
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach", "knew");
            auto edges = txn.findInEdge(teacher).where(clsset).get();
            ASSERT_SIZE(edges, 0);
        }
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("teach", "students");
            auto edges = txn.findInEdge(teacher).where(clsset).get();
            ASSERT_SIZE(edges, 0);
        }
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            teacher.rid.second = 9999U;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach", "know");
            auto edges = txn.findInEdge(teacher).where(clsset).get();
        }
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_get_invalid_edge_out_more()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("students").get()) {
            auto student = res.descriptor;
            auto edges = txn.findOutEdge(student).where(nogdb::GraphFilter {}.only("attack")).get();
            ASSERT_SIZE(edges, 0);
        }
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach", "knew");
            auto edges = txn.findOutEdge(teacher).where(clsset).get();
            // ASSERT_SIZE(edges, 0);
        }
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("teach", "students");
            auto edges = txn.findOutEdge(teacher).where(clsset).get();
            // ASSERT_SIZE(edges, 0);
        }
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            teacher.rid.second = 9999U;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach", "know");
            auto edges = txn.findOutEdge(teacher).where(clsset).get();
        }
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_get_invalid_edge_all_more()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("students").get()) {
            auto student = res.descriptor;
            auto edges = txn.findEdge(student).where(nogdb::GraphFilter {}.only("attack")).get();
            ASSERT_SIZE(edges, 0);
        }
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach", "knew");
            auto edges = txn.findEdge(teacher).where(clsset).get();
            // ASSERT_SIZE(edges, 0);
        }
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::GraphFilter {}.only("teach", "students");
            auto edges = txn.findEdge(teacher).where(clsset).get();
            // ASSERT_SIZE(edges, 0);
        }
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        for (const auto& res : txn.find("teachers").get()) {
            auto teacher = res.descriptor;
            teacher.rid.second = 9999U;
            auto clsset = nogdb::GraphFilter {}.only("workfor", "teach", "know");
            auto edges = txn.findEdge(teacher).where(clsset).get();
        }
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}
