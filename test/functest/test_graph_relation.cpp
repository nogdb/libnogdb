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
#include <set>

void init_test_graph() {
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

void destroy_test_graph() {
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


void test_create_complex_graph() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Record rt{};
        rt.set("name", "John").set("age", 52U).set("salary", 51000U).set("level", "Dr.");
        auto john = nogdb::Vertex::create(txn, "teachers", rt);
        rt.set("name", "Jim").set("age", 55U).set("salary", 46000U).set("level", "Asso.Prof.");
        auto jim = nogdb::Vertex::create(txn, "teachers", rt);
        rt.set("name", "Wei").set("age", 32U).set("salary", 65000U).set("level", "Prof.");
        auto wei = nogdb::Vertex::create(txn, "teachers", rt);

        nogdb::Record rs{};
        rs.set("name", "Peter").set("age", 42U).set("grade", 2.89);
        auto peter = nogdb::Vertex::create(txn, "students", rs);
        rs.set("name", "David").set("age", 40U).set("grade", 3.3);
        auto david = nogdb::Vertex::create(txn, "students", rs);
        rs.set("name", "Ying").set("age", 21U).set("grade", 3.01);
        auto ying = nogdb::Vertex::create(txn, "students", rs);
        rs.set("name", "Andy").set("age", 30U).set("grade", 3.43);
        auto andy = nogdb::Vertex::create(txn, "students", rs);
        rs.set("name", "Wong").set("age", 29U).set("grade", 3.78);
        auto wong = nogdb::Vertex::create(txn, "students", rs);
        rs.set("name", "Jessie").set("age", 27U).set("grade", 2.56);
        auto jessie = nogdb::Vertex::create(txn, "students", rs);

        nogdb::Record rd{};
        rd.set("name", "Computing");
        auto comp = nogdb::Vertex::create(txn, "departments", rd);
        rd.set("name", "Business");
        auto bus = nogdb::Vertex::create(txn, "departments", rd);

        nogdb::Record rsb{};
        rsb.set("name", "Programming");
        auto prog = nogdb::Vertex::create(txn, "subjects", rsb);
        rsb.set("name", "Database");
        auto db = nogdb::Vertex::create(txn, "subjects", rsb);
        rsb.set("name", "Networking");
        auto network = nogdb::Vertex::create(txn, "subjects", rsb);
        rsb.set("name", "Marketing");
        auto market = nogdb::Vertex::create(txn, "subjects", rsb);
        rsb.set("name", "Intro to Finance");
        auto fin = nogdb::Vertex::create(txn, "subjects", rsb);

        nogdb::Record rtch{};
        rtch.set("semester", "2016s1");
        nogdb::Edge::create(txn, "teach", john, market, rtch);
        nogdb::Edge::create(txn, "teach", jim, fin, rtch);
        nogdb::Edge::create(txn, "teach", wei, db, rtch);
        rtch.set("semester", "2016s2");
        nogdb::Edge::create(txn, "teach", john, fin, rtch);
        nogdb::Edge::create(txn, "teach", jim, network, rtch);
        nogdb::Edge::create(txn, "teach", wei, prog, rtch);

        nogdb::Record rb{};
        rb.set("null", "0");
        nogdb::Edge::create(txn, "belongto", prog, comp, rb);
        nogdb::Edge::create(txn, "belongto", db, comp, rb);
        nogdb::Edge::create(txn, "belongto", network, comp, rb);
        nogdb::Edge::create(txn, "belongto", market, bus, rb);
        nogdb::Edge::create(txn, "belongto", fin, bus, rb);

        nogdb::Record rw{};
        rw.set("position", "officer");
        nogdb::Edge::create(txn, "workfor", jim, comp, rw);
        nogdb::Edge::create(txn, "workfor", jim, bus, rw);
        rw.set("position", "dean");
        nogdb::Edge::create(txn, "workfor", john, bus, rw);
        nogdb::Edge::create(txn, "workfor", wei, comp, rw);

        nogdb::Record rk{};
        rk.set("relationship", "friend");
        nogdb::Edge::create(txn, "know", john, jim, rk);
        nogdb::Edge::create(txn, "know", jim, john, rk);
        nogdb::Edge::create(txn, "know", john, wei, rk);
        rk.set("relationship", "colleague");
        nogdb::Edge::create(txn, "know", wei, john, rk);
        nogdb::Edge::create(txn, "know", jim, wei, rk);
        nogdb::Edge::create(txn, "know", wei, jim, rk);

        nogdb::Record re{};
        re.set("semester", "2016s1");
        nogdb::Edge::create(txn, "enrol", jessie, market, re);
        nogdb::Edge::create(txn, "enrol", ying, fin, re);
        nogdb::Edge::create(txn, "enrol", peter, fin, re);
        nogdb::Edge::create(txn, "enrol", david, fin, re);
        nogdb::Edge::create(txn, "enrol", andy, db, re);
        nogdb::Edge::create(txn, "enrol", wong, db, re);
        re.set("semester", "2016s2");
        nogdb::Edge::create(txn, "enrol", jessie, fin, re);
        nogdb::Edge::create(txn, "enrol", ying, prog, re);
        nogdb::Edge::create(txn, "enrol", peter, prog, re);
        nogdb::Edge::create(txn, "enrol", david, prog, re);
        nogdb::Edge::create(txn, "enrol", andy, network, re);
        nogdb::Edge::create(txn, "enrol", wong, network, re);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        nogdb::Record rf{}, rl{};
        auto A = nogdb::Vertex::create(txn, "folders", rf.set("name", "A"));
        auto B = nogdb::Vertex::create(txn, "folders", rf.set("name", "B"));
        auto C = nogdb::Vertex::create(txn, "folders", rf.set("name", "C"));
        auto D = nogdb::Vertex::create(txn, "folders", rf.set("name", "D"));
        auto E = nogdb::Vertex::create(txn, "folders", rf.set("name", "E"));
        auto F = nogdb::Vertex::create(txn, "folders", rf.set("name", "F"));
        auto G = nogdb::Vertex::create(txn, "folders", rf.set("name", "G"));
        auto H = nogdb::Vertex::create(txn, "folders", rf.set("name", "H"));
        auto Z = nogdb::Vertex::create(txn, "folders", rf.set("name", "Z"));

        auto a = nogdb::Vertex::create(txn, "files", rf.set("name", "a"));
        auto b = nogdb::Vertex::create(txn, "files", rf.set("name", "b"));
        auto c = nogdb::Vertex::create(txn, "files", rf.set("name", "c"));
        auto d = nogdb::Vertex::create(txn, "files", rf.set("name", "d"));
        auto e = nogdb::Vertex::create(txn, "files", rf.set("name", "e"));
        auto f = nogdb::Vertex::create(txn, "files", rf.set("name", "f"));

//        rl.set("null", "0");
        nogdb::Edge::create(txn, "link", A, B, rl);
        nogdb::Edge::create(txn, "link", A, a, rl);
        nogdb::Edge::create(txn, "link", A, C, rl);
        nogdb::Edge::create(txn, "link", B, D, rl);
        nogdb::Edge::create(txn, "link", B, E, rl);
        nogdb::Edge::create(txn, "link", B, b, rl);
        nogdb::Edge::create(txn, "link", C, c, rl);
        nogdb::Edge::create(txn, "link", C, F, rl);
        nogdb::Edge::create(txn, "link", E, G, rl);
        nogdb::Edge::create(txn, "link", F, d, rl);
        nogdb::Edge::create(txn, "link", F, H, rl);
        nogdb::Edge::create(txn, "link", F, e, rl);
        nogdb::Edge::create(txn, "link", G, f, rl);
        nogdb::Edge::create(txn, "symbolic", B, b, rl);
        nogdb::Edge::create(txn, "symbolic", C, e, rl);
        nogdb::Edge::create(txn, "symbolic", D, A, rl);
        nogdb::Edge::create(txn, "symbolic", D, f, rl);
        nogdb::Edge::create(txn, "symbolic", E, F, rl);
        nogdb::Edge::create(txn, "symbolic", H, C, rl);
        nogdb::Edge::create(txn, "symbolic", a, a, rl);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto a = nogdb::Vertex::create(txn, "country", nogdb::Record{}
                .set("name", "A")
                .set("population", 400ULL)
        );

        auto b = nogdb::Vertex::create(txn, "country", nogdb::Record{}
                .set("name", "B")
                .set("population", 1000ULL)
        );

        auto c = nogdb::Vertex::create(txn, "country", nogdb::Record{}
                .set("name", "C")
                .set("population", 2000ULL)
        );

        auto d = nogdb::Vertex::create(txn, "country", nogdb::Record{}
                .set("name", "D")
                .set("population", 5000ULL)
        );

        auto e = nogdb::Vertex::create(txn, "country", nogdb::Record{}
                .set("name", "E")
                .set("population", 500ULL)
        );

        auto f = nogdb::Vertex::create(txn, "country", nogdb::Record{}
                .set("name", "F")
                .set("population", 1500ULL)
        );

        auto z = nogdb::Vertex::create(txn, "country", nogdb::Record{}
                .set("name", "Z")
                .set("population", 500ULL)
        );

        nogdb::Edge::create(txn, "path", z, a, nogdb::Record{}.set("distance", 40U));
        nogdb::Edge::create(txn, "path", a, b, nogdb::Record{}.set("distance", 50U));
        nogdb::Edge::create(txn, "path", a, c, nogdb::Record{}.set("distance", 400U));
        nogdb::Edge::create(txn, "path", b, e, nogdb::Record{}.set("distance", 250U));
        nogdb::Edge::create(txn, "path", b, c, nogdb::Record{}.set("distance", 80U));
        nogdb::Edge::create(txn, "path", c, d, nogdb::Record{}.set("distance", 100U));
        nogdb::Edge::create(txn, "path", c, f, nogdb::Record{}.set("distance", 150U));
        nogdb::Edge::create(txn, "path", d, a, nogdb::Record{}.set("distance", 300U));
        nogdb::Edge::create(txn, "path", d, f, nogdb::Record{}.set("distance", 120U));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_get_edge_in_more() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto edges = nogdb::Vertex::getInEdge(txn, teacher, nogdb::ClassFilter{"workfor"});
            assert(edges.size() == 0);
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto get_class_name = [&](const nogdb::RecordDescriptor &rdesc) {
            auto class_desc = nogdb::DB::getSchema(txn, rdesc.rid.first);
            return class_desc.name;
        };
        for (const auto &res: nogdb::Vertex::get(txn, "subjects")) {
            auto subject = res.descriptor;
            auto edges = nogdb::Vertex::getInEdge(txn, subject, nogdb::ClassFilter{"teach"});
            if (res.record.get("name").toText() == "Intro to Finance") {
                assert(edges.size() == 2);
            } else {
                assert(edges.size() == 1);
            }
            edges = nogdb::Vertex::getInEdge(txn, subject, nogdb::ClassFilter{"enrol"});
            assert(edges.size() != 0);
            bool tmp1 = false, tmp2 = false;
            for (const auto &edge: nogdb::Vertex::getInEdge(txn, subject)) {
                assert(get_class_name(edge.descriptor) == "teach" ||
                       get_class_name(edge.descriptor) == "enrol");
                if (get_class_name(edge.descriptor) == "teach") tmp1 |= true;
                if (get_class_name(edge.descriptor) == "enrol") tmp2 |= true;
            }
            assert(tmp1 && tmp2);
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach"};
            auto edges = nogdb::Vertex::getInEdge(txn, teacher, clsset);
            assert(edges.size() == 0);
        }

        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach"};
            auto edges = nogdb::Vertex::getInEdge(txn, teacher, clsset);
            assert(edges.size() == 0);
        }

        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach"};
            auto edges = nogdb::Vertex::getInEdge(txn, teacher, clsset);
            assert(edges.size() == 0);
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach", "know"};
            auto edges = nogdb::Vertex::getInEdge(txn, teacher, clsset);
            assert(edges.size() == 2);
        }

        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{};
            auto edges = nogdb::Vertex::getInEdge(txn, teacher, clsset);
            assert(edges.size() == 2);
        }

        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto edges = nogdb::Vertex::getInEdge(txn, teacher);
            assert(edges.size() == 2);
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_get_edge_out_more() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto edges = nogdb::Vertex::getOutEdge(txn, teacher, nogdb::ClassFilter{"workfor"});
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
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto &res: nogdb::Vertex::get(txn, "subjects")) {
            auto subject = res.descriptor;
            auto edges = nogdb::Vertex::getOutEdge(txn, subject, nogdb::ClassFilter{"belongto"});
            assert(edges.size() == 1);
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach"};
            auto edges = nogdb::Vertex::getOutEdge(txn, teacher, clsset);
            assert(edges.size() == 3 || edges.size() == 4);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 4);
            } else {
                assert(edges.size() == 3);
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach"};
            auto edges = nogdb::Vertex::getOutEdge(txn, teacher, clsset);
            assert(edges.size() == 3 || edges.size() == 4);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 4);
            } else {
                assert(edges.size() == 3);
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach"};
            auto edges = nogdb::Vertex::getOutEdge(txn, teacher, clsset);
            assert(edges.size() == 3 || edges.size() == 4);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 4);
            } else {
                assert(edges.size() == 3);
            }
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach", "know"};
            auto edges = nogdb::Vertex::getOutEdge(txn, teacher, clsset);
            assert(edges.size() == 5 || edges.size() == 6);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 6);
            } else {
                assert(edges.size() == 5);
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{};
            auto edges = nogdb::Vertex::getOutEdge(txn, teacher, clsset);
            assert(edges.size() == 5 || edges.size() == 6);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 6);
            } else {
                assert(edges.size() == 5);
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto edges = nogdb::Vertex::getOutEdge(txn, teacher);
            assert(edges.size() == 5 || edges.size() == 6);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 6);
            } else {
                assert(edges.size() == 5);
            }
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_get_edge_all_more() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto edges = nogdb::Vertex::getAllEdge(txn, teacher, nogdb::ClassFilter{"workfor"});
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
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach"};
            auto edges = nogdb::Vertex::getAllEdge(txn, teacher, clsset);
            assert(edges.size() == 3 || edges.size() == 4);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 4);
            } else {
                assert(edges.size() == 3);
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach"};
            auto edges = nogdb::Vertex::getAllEdge(txn, teacher, clsset);
            assert(edges.size() == 3 || edges.size() == 4);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 4);
            } else {
                assert(edges.size() == 3);
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach"};
            auto edges = nogdb::Vertex::getAllEdge(txn, teacher, clsset);
            assert(edges.size() == 3 || edges.size() == 4);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 4);
            } else {
                assert(edges.size() == 3);
            }
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach", "know"};
            auto edges = nogdb::Vertex::getAllEdge(txn, teacher, clsset);
            assert(edges.size() == 7 || edges.size() == 8);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 8);
            } else {
                assert(edges.size() == 7);
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{};
            auto edges = nogdb::Vertex::getAllEdge(txn, teacher, clsset);
            assert(edges.size() == 7 || edges.size() == 8);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 8);
            } else {
                assert(edges.size() == 7);
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto edges = nogdb::Vertex::getAllEdge(txn, teacher);
            assert(edges.size() == 7 || edges.size() == 8);
            if (res.record.get("name").toText() == "Jim") {
                assert(edges.size() == 8);
            } else {
                assert(edges.size() == 7);
            }
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto &res: nogdb::Vertex::get(txn, "subjects")) {
            auto subject = res.descriptor;
            auto edges = nogdb::Vertex::getAllEdge(txn, subject);
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
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_get_invalid_edge_in_more() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "students")) {
            auto student = res.descriptor;
            auto edges = nogdb::Vertex::getInEdge(txn, student, nogdb::ClassFilter{"attack"});
            assert(false);
        }
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach", "knew"};
            auto edges = nogdb::Vertex::getInEdge(txn, teacher, clsset);
        }
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"teach", "students"};
            auto edges = nogdb::Vertex::getInEdge(txn, teacher, clsset);
        }
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            teacher.rid.second = 9999U;
            auto clsset = nogdb::ClassFilter{"workfor", "teach", "know"};
            auto edges = nogdb::Vertex::getInEdge(txn, teacher, clsset);
        }
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

}

void test_get_invalid_edge_out_more() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "students")) {
            auto student = res.descriptor;
            auto edges = nogdb::Vertex::getOutEdge(txn, student, nogdb::ClassFilter{"attack"});
            assert(false);
        }
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach", "knew"};
            auto edges = nogdb::Vertex::getOutEdge(txn, teacher, clsset);
        }
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"teach", "students"};
            auto edges = nogdb::Vertex::getOutEdge(txn, teacher, clsset);
        }
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            teacher.rid.second = 9999U;
            auto clsset = nogdb::ClassFilter{"workfor", "teach", "know"};
            auto edges = nogdb::Vertex::getOutEdge(txn, teacher, clsset);
        }
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

}

void test_get_invalid_edge_all_more() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "students")) {
            auto student = res.descriptor;
            auto edges = nogdb::Vertex::getAllEdge(txn, student, nogdb::ClassFilter{"attack"});
            assert(false);
        }
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"workfor", "teach", "knew"};
            auto edges = nogdb::Vertex::getAllEdge(txn, teacher, clsset);
        }
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            auto clsset = nogdb::ClassFilter{"teach", "students"};
            auto edges = nogdb::Vertex::getAllEdge(txn, teacher, clsset);
        }
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "teachers")) {
            auto teacher = res.descriptor;
            teacher.rid.second = 9999U;
            auto clsset = nogdb::ClassFilter{"workfor", "teach", "know"};
            auto edges = nogdb::Vertex::getAllEdge(txn, teacher, clsset);
        }
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}
