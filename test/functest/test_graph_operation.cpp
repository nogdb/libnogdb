/*
 *  test_graph_operations.cpp - A sub-test for testing all graph operations
 *
 *  Copyright (C) 2018, Throughwave (Thailand) Co., Ltd.
 *  <peerawich at throughwave dot co dot th>
 *
 *  This program is free software: you can redistribute it and/or modify
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

#include <set>
#include <vector>
#include <list>

#include "functest.h"
#include "test_prepare.h"

void test_bfs_traverse_in() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, a, b, c, d, e, f, Z;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "folders")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res.descriptor;
                    break;
                case 'B':
                    B = res.descriptor;
                    break;
                case 'C':
                    C = res.descriptor;
                    break;
                case 'D':
                    D = res.descriptor;
                    break;
                case 'E':
                    E = res.descriptor;
                    break;
                case 'F':
                    F = res.descriptor;
                    break;
                case 'G':
                    G = res.descriptor;
                    break;
                case 'H':
                    H = res.descriptor;
                    break;
                case 'Z':
                    Z = res.descriptor;
                    break;
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "files")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res.descriptor;
                    break;
                case 'b':
                    b = res.descriptor;
                    break;
                case 'c':
                    c = res.descriptor;
                    break;
                case 'd':
                    d = res.descriptor;
                    break;
                case 'e':
                    e = res.descriptor;
                    break;
                case 'f':
                    f = res.descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto &res: nogdb::Traverse::inEdgeBfs(txn, D, 1, 1, nogdb::ClassFilter{"link"})) {
            auto name = res.record.get("name").toText();
            assert(name == "B");
            assert(res.record.getDepth() == 1);
        }
        for (const auto &res: nogdb::Traverse::inEdgeBfs(txn, D, 0, 2, nogdb::ClassFilter{"link"})) {
            auto name = res.record.get("name").toText();
            assert(name == "D" || name == "B" || name == "A");
            if (name == "D") assert(res.record.getDepth() == 0);
            else if (name == "B") assert(res.record.getDepth() == 1);
            else if (name == "A") assert(res.record.getDepth() == 2);
        }
        for (const auto &res: nogdb::Traverse::inEdgeBfs(txn, D, 1, 3, nogdb::ClassFilter{"link"})) {
            auto name = res.record.get("name").toText();
            assert(name == "B" || name == "A");
            if (name == "B") assert(res.record.getDepth() == 1);
            else if (name == "A") assert(res.record.getDepth() == 2);
        }
        for (const auto &res: nogdb::Traverse::inEdgeBfs(txn, D, 0, 0, nogdb::ClassFilter{"link"})) {
            auto name = res.record.get("name").toText();
            assert(name == "D");
            assert(res.record.getDepth() == 0);
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::inEdgeBfs(txn, H, 1, 10, nogdb::ClassFilter{"link"});
        assertSize(res, 3);
        assert(res[0].record.get("name").toText() == "F");
        assert(res[0].record.getDepth() == 1);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 2);
        assert(res[2].record.get("name").toText() == "A");
        assert(res[2].record.getDepth() == 3);

        res = nogdb::Traverse::inEdgeBfs(txn, f, 1, 4, nogdb::ClassFilter{"link"});
        assertSize(res, 4);
        assert(res[0].record.get("name").toText() == "G");
        assert(res[0].record.getDepth() == 1);
        assert(res[1].record.get("name").toText() == "E");
        assert(res[1].record.getDepth() == 2);
        assert(res[2].record.get("name").toText() == "B");
        assert(res[2].record.getDepth() == 3);
        assert(res[3].record.get("name").toText() == "A");
        assert(res[3].record.getDepth() == 4);

        res = nogdb::Traverse::inEdgeBfs(txn, f, 0, 4);
        assertSize(res, 6);

        res = nogdb::Traverse::inEdgeBfs(txn, f, 0, 100);
        assertSize(res, 6);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto classNames = std::set<std::string>{"link", "symbolic"};
        auto res = nogdb::Traverse::inEdgeBfs(txn, b, 0, 1, nogdb::ClassFilter{classNames.cbegin(), classNames.cend()});
        assertSize(res, 2);
        res = nogdb::Traverse::inEdgeBfs(txn, b, 1, 2);
        assertSize(res, 2);
        res = nogdb::Traverse::inEdgeBfs(txn, e, 1, 1);
        assertSize(res, 2);
        res = nogdb::Traverse::inEdgeBfs(txn, e, 0, 2);
        assertSize(res, 6);
        res = nogdb::Traverse::inEdgeBfs(txn, e, 0, 3);
        assertSize(res, 8);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::inEdgeBfs(txn, Z, 0, 1);
        assertSize(res, 1);
        res = nogdb::Traverse::inEdgeBfs(txn, Z, 0, 100);
        assertSize(res, 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_bfs_traverse_out() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, a, b, c, d, e, f, Z;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "folders")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res.descriptor;
                    break;
                case 'B':
                    B = res.descriptor;
                    break;
                case 'C':
                    C = res.descriptor;
                    break;
                case 'D':
                    D = res.descriptor;
                    break;
                case 'E':
                    E = res.descriptor;
                    break;
                case 'F':
                    F = res.descriptor;
                    break;
                case 'G':
                    G = res.descriptor;
                    break;
                case 'H':
                    H = res.descriptor;
                    break;
                case 'Z':
                    Z = res.descriptor;
                    break;
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "files")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res.descriptor;
                    break;
                case 'b':
                    b = res.descriptor;
                    break;
                case 'c':
                    c = res.descriptor;
                    break;
                case 'd':
                    d = res.descriptor;
                    break;
                case 'e':
                    e = res.descriptor;
                    break;
                case 'f':
                    f = res.descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::outEdgeBfs(txn, C, 1, 1, nogdb::ClassFilter{"link"});
        assertSize(res, 2);
        for (const auto &r: res) {
            auto name = r.record.get("name").toText();
            assert(name == "c" || name == "F");
        }
        res = nogdb::Traverse::outEdgeBfs(txn, C, 0, 2, nogdb::ClassFilter{"link"});
        assertSize(res, 6);
        res = nogdb::Traverse::outEdgeBfs(txn, C, 0, 3, nogdb::ClassFilter{"link"});
        assertSize(res, 6);
        res = nogdb::Traverse::outEdgeBfs(txn, C, 0, 0, nogdb::ClassFilter{"link"});
        assertSize(res, 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::outEdgeBfs(txn, A, 0, 0, nogdb::ClassFilter{"link"});
        assertSize(res, 1);
        res = nogdb::Traverse::outEdgeBfs(txn, A, 1, 1, nogdb::ClassFilter{"link"});
        assertSize(res, 3);
        res = nogdb::Traverse::outEdgeBfs(txn, A, 1, 2, nogdb::ClassFilter{"link"});
        assertSize(res, 8);
        res = nogdb::Traverse::outEdgeBfs(txn, A, 1, 3, nogdb::ClassFilter{"link"});
        assertSize(res, 12);
        res = nogdb::Traverse::outEdgeBfs(txn, A, 1, 4, nogdb::ClassFilter{"link"});
        assertSize(res, 13);
        res = nogdb::Traverse::outEdgeBfs(txn, A, 1, 100, nogdb::ClassFilter{"link"});
        assertSize(res, 13);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto classNames = std::vector<std::string>{"link", "symbolic"};
        auto res = nogdb::Traverse::outEdgeBfs(txn, B, 1, 1,
                                               nogdb::ClassFilter{classNames.cbegin(), classNames.cend()});
        assertSize(res, 3);
        res = nogdb::Traverse::outEdgeBfs(txn, C, 0, 1);
        assertSize(res, 4);

        res = nogdb::Traverse::outEdgeBfs(txn, a, 0, 0);
        assertSize(res, 1);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::outEdgeBfs(txn, Z, 0, 1);
        assertSize(res, 1);
        res = nogdb::Traverse::outEdgeBfs(txn, Z, 0, 100);
        assertSize(res, 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_bfs_traverse_all() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "folders")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res.descriptor;
                    break;
                case 'B':
                    B = res.descriptor;
                    break;
                case 'C':
                    C = res.descriptor;
                    break;
                case 'D':
                    D = res.descriptor;
                    break;
                case 'E':
                    E = res.descriptor;
                    break;
                case 'F':
                    F = res.descriptor;
                    break;
                case 'G':
                    G = res.descriptor;
                    break;
                case 'H':
                    H = res.descriptor;
                    break;
                case 'Z':
                    Z = res.descriptor;
                    break;
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "files")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res.descriptor;
                    break;
                case 'b':
                    b = res.descriptor;
                    break;
                case 'c':
                    c = res.descriptor;
                    break;
                case 'd':
                    d = res.descriptor;
                    break;
                case 'e':
                    e = res.descriptor;
                    break;
                case 'f':
                    f = res.descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto count = 0;
        for (const auto &res: nogdb::Traverse::allEdgeBfs(txn, F, 1, 1, nogdb::ClassFilter{"link"})) {
            auto name = res.record.get("name").toText();
            assert(name == "d" || name == "C" || name == "H" || name == "e");
            assert(res.record.getDepth() == 1);
            count++;
        }
        assert(count == 4);

        count = 0;
        for (const auto &res: nogdb::Traverse::allEdgeBfs(txn, F, 0, 2, nogdb::ClassFilter{"link"})) {
            auto name = res.record.get("name").toText();
            assert(name == "F" || name == "d" || name == "C" || name == "H" || name == "e" || name == "A" ||
                   name == "c");
            if (name == "d" || name == "C" || name == "H" || name == "e") {
                assert(res.record.getDepth() == 1);
            } else if (name == "F") {
                assert(res.record.getDepth() == 0);
            } else {
                assert(res.record.getDepth() == 2);
            }
            count++;
        }
        assert(count == 7);

        count = 0;
        for (const auto &res: nogdb::Traverse::allEdgeBfs(txn, F, 1, 3, nogdb::ClassFilter{"link"})) {
            auto name = res.record.get("name").toText();
            assert(name == "d" || name == "C" || name == "H" || name == "e" ||
                   name == "A" || name == "c" || name == "a" || name == "B");
            if (name == "d" || name == "C" || name == "H" || name == "e") {
                assert(res.record.getDepth() == 1);
            } else if (name == "A" || name == "c") {
                assert(res.record.getDepth() == 2);
            } else {
                assert(res.record.getDepth() == 3);
            }
            count++;
        }
        assert(count == 8);

        auto res = nogdb::Traverse::allEdgeBfs(txn, F, 0, 0, nogdb::ClassFilter{"link"});
        assertSize(res, 1);
        res = nogdb::Traverse::allEdgeBfs(txn, F, 0, 100, nogdb::ClassFilter{"link"});
        assertSize(res, 14);
        res = nogdb::Traverse::allEdgeBfs(txn, F, 2, 1, nogdb::ClassFilter{"link"});
        assertSize(res, 0);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto count = 0;
        for (const auto &res: nogdb::Traverse::allEdgeBfs(txn, H, 1, 1, nogdb::ClassFilter{"symbolic"})) {
            auto name = res.record.get("name").toText();
            assert(name == "C");
            count++;
        }
        assert(count == 1);

        count = 0;
        for (const auto &res: nogdb::Traverse::allEdgeBfs(txn, H, 2, 2, nogdb::ClassFilter{"symbolic"})) {
            auto name = res.record.get("name").toText();
            assert(name == "e");
            count++;
        }
        assert(count == 1);

        auto res = nogdb::Traverse::allEdgeBfs(txn, H, 1, 3, nogdb::ClassFilter{"symbolic"});
        assertSize(res, 2);

        res = nogdb::Traverse::allEdgeBfs(txn, H, 0, 0, nogdb::ClassFilter{"symbolic"});
        assertSize(res, 1);

        res = nogdb::Traverse::allEdgeBfs(txn, H, 0, 100, nogdb::ClassFilter{"symbolic"});
        assertSize(res, 3);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto count = 0;
        auto classNames = std::list<std::string>{"link", "symbolic"};
        for (const auto &res: nogdb::Traverse::allEdgeBfs(txn, A, 1, 1,
                                                          nogdb::ClassFilter{classNames.cbegin(), classNames.cend()})) {
            auto name = res.record.get("name").toText();
            assert(name == "B" || name == "a" || name == "C" || name == "D");
            count++;
        }
        assert(count == 4);

        auto res = nogdb::Traverse::allEdgeBfs(txn, A, 1, 2);
        assertSize(res, 11);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::allEdgeBfs(txn, Z, 0, 1);
        assertSize(res, 1);
        res = nogdb::Traverse::allEdgeBfs(txn, Z, 0, 100);
        assertSize(res, 1);
        res = nogdb::Traverse::allEdgeBfs(txn, Z, 0, 0);
        assertSize(res, 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_invalid_bfs_traverse_in() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "folders")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res.descriptor;
                    break;
                case 'B':
                    B = res.descriptor;
                    break;
                case 'C':
                    C = res.descriptor;
                    break;
                case 'D':
                    D = res.descriptor;
                    break;
                case 'E':
                    E = res.descriptor;
                    break;
                case 'F':
                    F = res.descriptor;
                    break;
                case 'G':
                    G = res.descriptor;
                    break;
                case 'H':
                    H = res.descriptor;
                    break;
                case 'Z':
                    Z = res.descriptor;
                    break;
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "files")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res.descriptor;
                    break;
                case 'b':
                    b = res.descriptor;
                    break;
                case 'c':
                    c = res.descriptor;
                    break;
                case 'd':
                    d = res.descriptor;
                    break;
                case 'e':
                    e = res.descriptor;
                    break;
                case 'f':
                    f = res.descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeBfs(txn, A, 0, 0, nogdb::ClassFilter{"ling"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeBfs(txn, A, 0, 0, nogdb::ClassFilter{"link", "symbol"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeBfs(txn, A, 0, 0, nogdb::ClassFilter{"folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeBfs(txn, A, 0, 0, nogdb::ClassFilter{"link", "folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = nogdb::Traverse::inEdgeBfs(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = nogdb::Traverse::inEdgeBfs(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

}

void test_invalid_bfs_traverse_out() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "folders")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res.descriptor;
                    break;
                case 'B':
                    B = res.descriptor;
                    break;
                case 'C':
                    C = res.descriptor;
                    break;
                case 'D':
                    D = res.descriptor;
                    break;
                case 'E':
                    E = res.descriptor;
                    break;
                case 'F':
                    F = res.descriptor;
                    break;
                case 'G':
                    G = res.descriptor;
                    break;
                case 'H':
                    H = res.descriptor;
                    break;
                case 'Z':
                    Z = res.descriptor;
                    break;
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "files")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res.descriptor;
                    break;
                case 'b':
                    b = res.descriptor;
                    break;
                case 'c':
                    c = res.descriptor;
                    break;
                case 'd':
                    d = res.descriptor;
                    break;
                case 'e':
                    e = res.descriptor;
                    break;
                case 'f':
                    f = res.descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeBfs(txn, A, 0, 0, nogdb::ClassFilter{"ling"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeBfs(txn, A, 0, 0, nogdb::ClassFilter{"link", "symbol"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeBfs(txn, A, 0, 0, nogdb::ClassFilter{"folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeBfs(txn, A, 0, 0, nogdb::ClassFilter{"link", "folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = nogdb::Traverse::outEdgeBfs(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = nogdb::Traverse::outEdgeBfs(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_invalid_bfs_traverse_all() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "folders")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res.descriptor;
                    break;
                case 'B':
                    B = res.descriptor;
                    break;
                case 'C':
                    C = res.descriptor;
                    break;
                case 'D':
                    D = res.descriptor;
                    break;
                case 'E':
                    E = res.descriptor;
                    break;
                case 'F':
                    F = res.descriptor;
                    break;
                case 'G':
                    G = res.descriptor;
                    break;
                case 'H':
                    H = res.descriptor;
                    break;
                case 'Z':
                    Z = res.descriptor;
                    break;
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "files")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res.descriptor;
                    break;
                case 'b':
                    b = res.descriptor;
                    break;
                case 'c':
                    c = res.descriptor;
                    break;
                case 'd':
                    d = res.descriptor;
                    break;
                case 'e':
                    e = res.descriptor;
                    break;
                case 'f':
                    f = res.descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeBfs(txn, A, 0, 0, nogdb::ClassFilter{"ling"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeBfs(txn, A, 0, 0, nogdb::ClassFilter{"link", "symbol"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeBfs(txn, A, 0, 0, nogdb::ClassFilter{"folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeBfs(txn, A, 0, 0, nogdb::ClassFilter{"link", "folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = nogdb::Traverse::allEdgeBfs(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = nogdb::Traverse::allEdgeBfs(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_dfs_traverse_in() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, a, b, c, d, e, f, Z;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "folders")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res.descriptor;
                    break;
                case 'B':
                    B = res.descriptor;
                    break;
                case 'C':
                    C = res.descriptor;
                    break;
                case 'D':
                    D = res.descriptor;
                    break;
                case 'E':
                    E = res.descriptor;
                    break;
                case 'F':
                    F = res.descriptor;
                    break;
                case 'G':
                    G = res.descriptor;
                    break;
                case 'H':
                    H = res.descriptor;
                    break;
                case 'Z':
                    Z = res.descriptor;
                    break;
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "files")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res.descriptor;
                    break;
                case 'b':
                    b = res.descriptor;
                    break;
                case 'c':
                    c = res.descriptor;
                    break;
                case 'd':
                    d = res.descriptor;
                    break;
                case 'e':
                    e = res.descriptor;
                    break;
                case 'f':
                    f = res.descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto &res: nogdb::Traverse::inEdgeDfs(txn, D, 0, 1, nogdb::ClassFilter{"link"})) {
            auto name = res.record.get("name").toText();
            assert(name == "B" || name == "D");
            if (name == "D") assert(res.record.getDepth() == 0);
            else assert(res.record.getDepth() == 1);
        }
        for (const auto &res: nogdb::Traverse::inEdgeDfs(txn, D, 1, 2, nogdb::ClassFilter{"link"})) {
            auto name = res.record.get("name").toText();
            assert(name == "B" || name == "A");
            if (name == "B") assert(res.record.getDepth() == 1);
            else assert(res.record.getDepth() == 2);
        }
        auto res = nogdb::Traverse::inEdgeDfs(txn, D, 3, 4, nogdb::ClassFilter{"link"});
        assertSize(res, 0);
        for (const auto &res: nogdb::Traverse::inEdgeDfs(txn, D, 0, 0, nogdb::ClassFilter{"link"})) {
            auto name = res.record.get("name").toText();
            assert(name == "D");
            assert(res.record.getDepth() == 0);
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::inEdgeDfs(txn, H, 1, 100, nogdb::ClassFilter{"link"});
        assertSize(res, 3);
        assert(res[0].record.get("name").toText() == "F");
        assert(res[0].record.getDepth() == 1);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 2);
        assert(res[2].record.get("name").toText() == "A");
        assert(res[2].record.getDepth() == 3);

        res = nogdb::Traverse::inEdgeDfs(txn, f, 1, 4, nogdb::ClassFilter{"link"});
        assertSize(res, 4);
        assert(res[0].record.get("name").toText() == "G");
        assert(res[0].record.getDepth() == 1);
        assert(res[1].record.get("name").toText() == "E");
        assert(res[1].record.getDepth() == 2);
        assert(res[2].record.get("name").toText() == "B");
        assert(res[2].record.getDepth() == 3);
        assert(res[3].record.get("name").toText() == "A");
        assert(res[3].record.getDepth() == 4);

        res = nogdb::Traverse::inEdgeDfs(txn, f, 0, 4);
        assertSize(res, 6);

        res = nogdb::Traverse::inEdgeDfs(txn, f, 0, 100);
        assertSize(res, 6);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::inEdgeDfs(txn, b, 1, 1, nogdb::ClassFilter{"link", "symbolic"});
        assertSize(res, 1);
        res = nogdb::Traverse::inEdgeDfs(txn, b, 1, 2);
        assertSize(res, 2);
        res = nogdb::Traverse::inEdgeDfs(txn, e, 1, 1);
        assertSize(res, 2);
        assert(res[0].record.get("name").toText() == "F");
        assert(res[0].record.getDepth() == 1);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 1);
        res = nogdb::Traverse::inEdgeDfs(txn, e, 1, 2);
        assertSize(res, 3);
        assert(res[0].record.get("name").toText() == "F");
        assert(res[0].record.getDepth() == 1);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 2);
        assert(res[2].record.get("name").toText() == "E");
        assert(res[2].record.getDepth() == 2);
        res = nogdb::Traverse::inEdgeDfs(txn, e, 0, 3);
        assertSize(res, 7);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::inEdgeDfs(txn, Z, 0, 1);
        assertSize(res, 1);
        res = nogdb::Traverse::inEdgeDfs(txn, Z, 0, 100);
        assertSize(res, 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_dfs_traverse_out() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, a, b, c, d, e, f, Z;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "folders")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res.descriptor;
                    break;
                case 'B':
                    B = res.descriptor;
                    break;
                case 'C':
                    C = res.descriptor;
                    break;
                case 'D':
                    D = res.descriptor;
                    break;
                case 'E':
                    E = res.descriptor;
                    break;
                case 'F':
                    F = res.descriptor;
                    break;
                case 'G':
                    G = res.descriptor;
                    break;
                case 'H':
                    H = res.descriptor;
                    break;
                case 'Z':
                    Z = res.descriptor;
                    break;
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "files")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res.descriptor;
                    break;
                case 'b':
                    b = res.descriptor;
                    break;
                case 'c':
                    c = res.descriptor;
                    break;
                case 'd':
                    d = res.descriptor;
                    break;
                case 'e':
                    e = res.descriptor;
                    break;
                case 'f':
                    f = res.descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::outEdgeDfs(txn, C, 0, 1, nogdb::ClassFilter{"link"});
        assertSize(res, 3);
        for (const auto &r: res) {
            auto name = r.record.get("name").toText();
            assert(name == "c" || name == "F" || name == "C");
        }
        res = nogdb::Traverse::outEdgeDfs(txn, C, 1, 2, nogdb::ClassFilter{"link"});
        assertSize(res, 5);
        assert(compareText(res, "name", std::vector<std::string>{"c", "F", "d", "H", "e"}));
        res = nogdb::Traverse::outEdgeDfs(txn, C, 0, 3, nogdb::ClassFilter{"link"});
        assertSize(res, 6);
        res = nogdb::Traverse::outEdgeDfs(txn, C, 0, 0, nogdb::ClassFilter{"link"});
        assertSize(res, 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::outEdgeDfs(txn, A, 0, 0, nogdb::ClassFilter{"link"});
        assertSize(res, 1);
        res = nogdb::Traverse::outEdgeDfs(txn, A, 1, 1, nogdb::ClassFilter{"link"});
        assertSize(res, 3);
        res = nogdb::Traverse::outEdgeDfs(txn, A, 1, 2, nogdb::ClassFilter{"link"});
        assertSize(res, 8);
        assert(compareText(res, "name", std::vector<std::string>{"B", "D", "E", "b", "a", "C", "c", "F"}));
        res = nogdb::Traverse::outEdgeDfs(txn, A, 1, 3, nogdb::ClassFilter{"link"});
        assertSize(res, 12);
        res = nogdb::Traverse::outEdgeDfs(txn, A, 1, 4, nogdb::ClassFilter{"link"});
        assertSize(res, 13);
        res = nogdb::Traverse::outEdgeDfs(txn, A, 1, 100, nogdb::ClassFilter{"link"});
        assertSize(res, 13);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::outEdgeDfs(txn, B, 1, 1, nogdb::ClassFilter{"link", "symbolic"});
        assertSize(res, 3);
        res = nogdb::Traverse::outEdgeDfs(txn, C, 1, 1);
        assertSize(res, 3);

        res = nogdb::Traverse::outEdgeDfs(txn, a, 0, 0);
        assertSize(res, 1);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::outEdgeDfs(txn, Z, 0, 1);
        assertSize(res, 1);
        res = nogdb::Traverse::outEdgeDfs(txn, Z, 0, 100);
        assertSize(res, 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_dfs_traverse_all() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "folders")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res.descriptor;
                    break;
                case 'B':
                    B = res.descriptor;
                    break;
                case 'C':
                    C = res.descriptor;
                    break;
                case 'D':
                    D = res.descriptor;
                    break;
                case 'E':
                    E = res.descriptor;
                    break;
                case 'F':
                    F = res.descriptor;
                    break;
                case 'G':
                    G = res.descriptor;
                    break;
                case 'H':
                    H = res.descriptor;
                    break;
                case 'Z':
                    Z = res.descriptor;
                    break;
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "files")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res.descriptor;
                    break;
                case 'b':
                    b = res.descriptor;
                    break;
                case 'c':
                    c = res.descriptor;
                    break;
                case 'd':
                    d = res.descriptor;
                    break;
                case 'e':
                    e = res.descriptor;
                    break;
                case 'f':
                    f = res.descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto count = 0;
        for (const auto &res: nogdb::Traverse::allEdgeDfs(txn, F, 0, 1, nogdb::ClassFilter{"link"})) {
            auto name = res.record.get("name").toText();
            assert(name == "d" || name == "C" || name == "H" || name == "e" || name == "F");
            count++;
        }
        assert(count == 5);

        count = 0;
        for (const auto &res: nogdb::Traverse::allEdgeDfs(txn, F, 1, 2, nogdb::ClassFilter{"link"})) {
            auto name = res.record.get("name").toText();
            assert(name == "d" || name == "C" || name == "H" || name == "e" || name == "A" || name == "c");
            count++;
        }
        assert(count == 6);

        count = 0;
        for (const auto &res: nogdb::Traverse::allEdgeDfs(txn, F, 1, 3, nogdb::ClassFilter{"link"})) {
            auto name = res.record.get("name").toText();
            assert(name == "d" || name == "C" || name == "H" || name == "e" ||
                   name == "A" || name == "c" || name == "a" || name == "B");
            count++;
        }
        assert(count == 8);

        auto res = nogdb::Traverse::allEdgeDfs(txn, F, 0, 0, nogdb::ClassFilter{"link"});
        assertSize(res, 1);

        res = nogdb::Traverse::allEdgeDfs(txn, F, 4, 100, nogdb::ClassFilter{"link"});
        assertSize(res, 5);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto count = 0;
        for (const auto &res: nogdb::Traverse::allEdgeDfs(txn, H, 1, 1, nogdb::ClassFilter{"symbolic"})) {
            auto name = res.record.get("name").toText();
            assert(name == "C");
            count++;
        }
        assert(count == 1);

        count = 0;
        for (const auto &res: nogdb::Traverse::allEdgeDfs(txn, H, 1, 2, nogdb::ClassFilter{"symbolic"})) {
            auto name = res.record.get("name").toText();
            assert(name == "C" || name == "e");
            count++;
        }
        assert(count == 2);

        auto res = nogdb::Traverse::allEdgeDfs(txn, H, 1, 3, nogdb::ClassFilter{"symbolic"});
        assertSize(res, 2);

        res = nogdb::Traverse::allEdgeDfs(txn, H, 0, 0, nogdb::ClassFilter{"symbolic"});
        assertSize(res, 1);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto count = 0;
        for (const auto &res: nogdb::Traverse::allEdgeDfs(txn, A, 1, 1, nogdb::ClassFilter{"link", "symbolic"})) {
            auto name = res.record.get("name").toText();
            assert(name == "B" || name == "a" || name == "C" || name == "D");
            count++;
        }
        assert(count == 4);

        auto res = nogdb::Traverse::allEdgeDfs(txn, A, 1, 2);
        assertSize(res, 10);
        assert(res[0].record.get("name").toText() == "B");
        assert(res[0].record.getDepth() == 1);
        assert(res[1].record.get("name").toText() == "D");
        assert(res[1].record.getDepth() == 2);
        assert(res[2].record.get("name").toText() == "E");
        assert(res[2].record.getDepth() == 2);
        assert(res[3].record.get("name").toText() == "b");
        assert(res[3].record.getDepth() == 2);
        assert(res[4].record.get("name").toText() == "a");
        assert(res[4].record.getDepth() == 1);
        assert(res[5].record.get("name").toText() == "C");
        assert(res[5].record.getDepth() == 1);
        assert(res[6].record.get("name").toText() == "c");
        assert(res[6].record.getDepth() == 2);
        assert(res[7].record.get("name").toText() == "F");
        assert(res[7].record.getDepth() == 2);
        assert(res[8].record.get("name").toText() == "e");
        assert(res[8].record.getDepth() == 2);
        assert(res[9].record.get("name").toText() == "H");
        assert(res[9].record.getDepth() == 2);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::allEdgeDfs(txn, Z, 0, 1);
        assertSize(res, 1);
        res = nogdb::Traverse::allEdgeDfs(txn, Z, 0, 100);
        assertSize(res, 1);
        res = nogdb::Traverse::allEdgeDfs(txn, Z, 0, 0);
        assertSize(res, 1);
        res = nogdb::Traverse::allEdgeDfs(txn, Z, 10, 0);
        assert(res.empty());
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_invalid_dfs_traverse_in() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "folders")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res.descriptor;
                    break;
                case 'B':
                    B = res.descriptor;
                    break;
                case 'C':
                    C = res.descriptor;
                    break;
                case 'D':
                    D = res.descriptor;
                    break;
                case 'E':
                    E = res.descriptor;
                    break;
                case 'F':
                    F = res.descriptor;
                    break;
                case 'G':
                    G = res.descriptor;
                    break;
                case 'H':
                    H = res.descriptor;
                    break;
                case 'Z':
                    Z = res.descriptor;
                    break;
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "files")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res.descriptor;
                    break;
                case 'b':
                    b = res.descriptor;
                    break;
                case 'c':
                    c = res.descriptor;
                    break;
                case 'd':
                    d = res.descriptor;
                    break;
                case 'e':
                    e = res.descriptor;
                    break;
                case 'f':
                    f = res.descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeDfs(txn, A, 0, 0, nogdb::ClassFilter{"ling"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeDfs(txn, A, 0, 0, nogdb::ClassFilter{"link", "symbol"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeDfs(txn, A, 0, 0, nogdb::ClassFilter{"folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeDfs(txn, A, 0, 0, nogdb::ClassFilter{"link", "folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = nogdb::Traverse::inEdgeDfs(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = nogdb::Traverse::inEdgeDfs(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_invalid_dfs_traverse_out() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "folders")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res.descriptor;
                    break;
                case 'B':
                    B = res.descriptor;
                    break;
                case 'C':
                    C = res.descriptor;
                    break;
                case 'D':
                    D = res.descriptor;
                    break;
                case 'E':
                    E = res.descriptor;
                    break;
                case 'F':
                    F = res.descriptor;
                    break;
                case 'G':
                    G = res.descriptor;
                    break;
                case 'H':
                    H = res.descriptor;
                    break;
                case 'Z':
                    Z = res.descriptor;
                    break;
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "files")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res.descriptor;
                    break;
                case 'b':
                    b = res.descriptor;
                    break;
                case 'c':
                    c = res.descriptor;
                    break;
                case 'd':
                    d = res.descriptor;
                    break;
                case 'e':
                    e = res.descriptor;
                    break;
                case 'f':
                    f = res.descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeDfs(txn, A, 0, 0, nogdb::ClassFilter{"ling"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeDfs(txn, A, 0, 0, nogdb::ClassFilter{"link", "symbol"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeDfs(txn, A, 0, 0, nogdb::ClassFilter{"folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeDfs(txn, A, 0, 0, nogdb::ClassFilter{"link", "folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = nogdb::Traverse::outEdgeDfs(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = nogdb::Traverse::outEdgeDfs(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_invalid_dfs_traverse_all() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "folders")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res.descriptor;
                    break;
                case 'B':
                    B = res.descriptor;
                    break;
                case 'C':
                    C = res.descriptor;
                    break;
                case 'D':
                    D = res.descriptor;
                    break;
                case 'E':
                    E = res.descriptor;
                    break;
                case 'F':
                    F = res.descriptor;
                    break;
                case 'G':
                    G = res.descriptor;
                    break;
                case 'H':
                    H = res.descriptor;
                    break;
                case 'Z':
                    Z = res.descriptor;
                    break;
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "files")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res.descriptor;
                    break;
                case 'b':
                    b = res.descriptor;
                    break;
                case 'c':
                    c = res.descriptor;
                    break;
                case 'd':
                    d = res.descriptor;
                    break;
                case 'e':
                    e = res.descriptor;
                    break;
                case 'f':
                    f = res.descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeDfs(txn, A, 0, 0, nogdb::ClassFilter{"ling"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeDfs(txn, A, 0, 0, nogdb::ClassFilter{"link", "symbol"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeDfs(txn, A, 0, 0, nogdb::ClassFilter{"folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeDfs(txn, A, 0, 0, nogdb::ClassFilter{"link", "folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = nogdb::Traverse::allEdgeDfs(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = nogdb::Traverse::allEdgeDfs(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_shortest_path() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "folders")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res.descriptor;
                    break;
                case 'B':
                    B = res.descriptor;
                    break;
                case 'C':
                    C = res.descriptor;
                    break;
                case 'D':
                    D = res.descriptor;
                    break;
                case 'E':
                    E = res.descriptor;
                    break;
                case 'F':
                    F = res.descriptor;
                    break;
                case 'G':
                    G = res.descriptor;
                    break;
                case 'H':
                    H = res.descriptor;
                    break;
                case 'Z':
                    Z = res.descriptor;
                    break;
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "files")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res.descriptor;
                    break;
                case 'b':
                    b = res.descriptor;
                    break;
                case 'c':
                    c = res.descriptor;
                    break;
                case 'd':
                    d = res.descriptor;
                    break;
                case 'e':
                    e = res.descriptor;
                    break;
                case 'f':
                    f = res.descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::shortestPath(txn, A, f);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "B");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "D");
        assert(res[2].record.getDepth() == 2);
        assert(res[3].record.get("name").toText() == "f");
        assert(res[3].record.getDepth() == 3);

        res = nogdb::Traverse::shortestPath(txn, A, e);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "e");
        assert(res[2].record.getDepth() == 2);

        res = nogdb::Traverse::shortestPath(txn, D, f);
        assert(res[0].record.get("name").toText() == "D");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "f");
        assert(res[1].record.getDepth() == 1);

        res = nogdb::Traverse::shortestPath(txn, B, A);
        assert(res[0].record.get("name").toText() == "B");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "D");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "A");
        assert(res[2].record.getDepth() == 2);

        res = nogdb::Traverse::shortestPath(txn, A, e, nogdb::ClassFilter{"link", "symbolic"});
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "e");
        assert(res[2].record.getDepth() == 2);

        res = nogdb::Traverse::shortestPath(txn, D, f, nogdb::ClassFilter{"link", "symbolic"});
        assert(res[0].record.get("name").toText() == "D");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "f");
        assert(res[1].record.getDepth() == 1);

        res = nogdb::Traverse::shortestPath(txn, B, A, nogdb::ClassFilter{"link", "symbolic"});
        assert(res[0].record.get("name").toText() == "B");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "D");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "A");
        assert(res[2].record.getDepth() == 2);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::shortestPath(txn, a, a);
        assertSize(res, 1);
        assert(res[0].record.get("name").toText() == "a");
        assert(res[0].record.getDepth() == 0);

        res = nogdb::Traverse::shortestPath(txn, f, f);
        assertSize(res, 1);
        assert(res[0].record.get("name").toText() == "f");
        assert(res[0].record.getDepth() == 0);

        res = nogdb::Traverse::shortestPath(txn, B, B);
        assertSize(res, 1);
        assert(res[0].record.get("name").toText() == "B");
        assert(res[0].record.getDepth() == 0);

        res = nogdb::Traverse::shortestPath(txn, A, Z);
        assert(res.empty());

        res = nogdb::Traverse::shortestPath(txn, Z, G);
        assert(res.empty());

        res = nogdb::Traverse::shortestPath(txn, a, F);
        assert(res.empty());

        res = nogdb::Traverse::shortestPath(txn, d, A);
        assert(res.empty());

        res = nogdb::Traverse::shortestPath(txn, A, b);
        assertSize(res, 3);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "B");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "b");
        assert(res[2].record.getDepth() == 2);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::shortestPath(txn, C, e, nogdb::ClassFilter{"link"});
        assertSize(res, 3);

        res = nogdb::Traverse::shortestPath(txn, B, d);
        assertSize(res, 4);
        res = nogdb::Traverse::shortestPath(txn, B, d, nogdb::ClassFilter{"link"});
        assertSize(res, 0);

        res = nogdb::Traverse::shortestPath(txn, H, C, nogdb::ClassFilter{"link"});
        assertSize(res, 0);
        res = nogdb::Traverse::shortestPath(txn, H, C, nogdb::ClassFilter{"symbolic"});
        assertSize(res, 2);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_invalid_shortest_path() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "folders")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res.descriptor;
                    break;
                case 'B':
                    B = res.descriptor;
                    break;
                case 'C':
                    C = res.descriptor;
                    break;
                case 'D':
                    D = res.descriptor;
                    break;
                case 'E':
                    E = res.descriptor;
                    break;
                case 'F':
                    F = res.descriptor;
                    break;
                case 'G':
                    G = res.descriptor;
                    break;
                case 'H':
                    H = res.descriptor;
                    break;
                case 'Z':
                    Z = res.descriptor;
                    break;
            }
        }

        for (const auto &res: nogdb::Vertex::get(txn, "files")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res.descriptor;
                    break;
                case 'b':
                    b = res.descriptor;
                    break;
                case 'c':
                    c = res.descriptor;
                    break;
                case 'd':
                    d = res.descriptor;
                    break;
                case 'e':
                    e = res.descriptor;
                    break;
                case 'f':
                    f = res.descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.second = 999U;
        auto res = nogdb::Traverse::shortestPath(txn, tmp, B);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_SRC, "NOGDB_GRAPH_NOEXST_SRC");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = B;
        tmp.rid.second = 999U;
        auto res = nogdb::Traverse::shortestPath(txn, A, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_DST, "NOGDB_GRAPH_NOEXST_DST");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = nogdb::Traverse::shortestPath(txn, tmp, D);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto links = nogdb::Edge::get(txn, "link");
        auto tmp = links[0].descriptor;
        auto res = nogdb::Traverse::shortestPath(txn, A, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto links = nogdb::Edge::get(txn, "link");
        auto tmp = links[0].descriptor;
        auto res = nogdb::Traverse::shortestPath(txn, tmp, f);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }
}

void test_bfs_traverse_with_condition() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor a, b, c, d, e, f, z;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "country")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    a = res.descriptor;
                    break;
                case 'B':
                    b = res.descriptor;
                    break;
                case 'C':
                    c = res.descriptor;
                    break;
                case 'D':
                    d = res.descriptor;
                    break;
                case 'E':
                    e = res.descriptor;
                    break;
                case 'F':
                    f = res.descriptor;
                    break;
                case 'Z':
                    z = res.descriptor;
                    break;
            }
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto edgeCondition = [](const nogdb::Record &record) {
            return (record.get("distance").toIntU() < 100U);
        };
        auto pathFilter = nogdb::PathFilter{}.setEdge(edgeCondition);
        auto res = nogdb::Traverse::outEdgeBfs(txn, a, 0, 1, pathFilter);
        assertSize(res, 2);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "B");
        assert(res[1].record.getDepth() == 1);

        res = nogdb::Traverse::inEdgeBfs(txn, a, 0, 1, pathFilter);
        assertSize(res, 2);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "Z");
        assert(res[1].record.getDepth() == 1);

        auto vertexCondition = [](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() > 1000ULL);
        };
        pathFilter.setVertex(vertexCondition);
        res = nogdb::Traverse::outEdgeBfs(txn, a, 0, 1, pathFilter);
        assertSize(res, 1);
        assert(res[0].record.get("name").toText() == "A");

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto edgeCondition = [](const nogdb::Record &record) {
            return (record.get("distance").toIntU() > 100U);
        };
        auto pathFilter = nogdb::PathFilter{}.setEdge(edgeCondition);
        auto res = nogdb::Traverse::allEdgeBfs(txn, a, 1, 3, pathFilter);
        assertSize(res, 3);
        assert(res[0].record.get("name").toText() == "C");
        assert(res[0].record.getDepth() == 1);
        assert(res[1].record.get("name").toText() == "D");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "F");
        assert(res[2].record.getDepth() == 2);

        res = nogdb::Traverse::allEdgeBfs(txn, a, 2, 4, pathFilter);
        assertSize(res, 1);
        assert(res[0].record.get("name").toText() == "F");
        assert(res[0].record.getDepth() == 2);

        auto vertexCondition = [](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() < 4000ULL);
        };
        pathFilter.setVertex(vertexCondition);
        res = nogdb::Traverse::allEdgeBfs(txn, a, 0, 4, pathFilter);
        assertSize(res, 3);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "F");
        assert(res[2].record.getDepth() == 2);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_dfs_traverse_with_condition() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor a, b, c, d, e, f, z;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "country")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    a = res.descriptor;
                    break;
                case 'B':
                    b = res.descriptor;
                    break;
                case 'C':
                    c = res.descriptor;
                    break;
                case 'D':
                    d = res.descriptor;
                    break;
                case 'E':
                    e = res.descriptor;
                    break;
                case 'F':
                    f = res.descriptor;
                    break;
                case 'Z':
                    z = res.descriptor;
                    break;
            }
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto edgeCondition = [](const nogdb::Record &record) {
            return (record.get("distance").toIntU() <= 100U);
        };
        auto pathFilter = nogdb::PathFilter{}.setEdge(edgeCondition);
        auto res = nogdb::Traverse::outEdgeDfs(txn, a, 1, 3, pathFilter);
        assertSize(res, 3);
        assert(res[0].record.get("name").toText() == "B");
        assert(res[0].record.getDepth() == 1);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 2);
        assert(res[2].record.get("name").toText() == "D");
        assert(res[2].record.getDepth() == 3);

        pathFilter.setVertex([](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() >= 1500ULL);
        }).setEdge([](const nogdb::Record &record) {
            return (record.get("distance").toIntU() >= 300U);
        });
        res = nogdb::Traverse::outEdgeDfs(txn, a, 0, 4, pathFilter);
        assertSize(res, 2);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 1);

        pathFilter.setVertex([](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() < 1500ULL);
        });
        res = nogdb::Traverse::outEdgeDfs(txn, a, 0, 4, pathFilter);
        assertSize(res, 1);
        assert(res[0].record.get("name").toText() == "A");
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto edgeCondition = [](const nogdb::Record &record) {
            return (record.get("distance").toIntU() <= 100U);
        };
        auto pathFilter = nogdb::PathFilter{}.setEdge(edgeCondition);
        auto res = nogdb::Traverse::allEdgeDfs(txn, a, 1, 3, pathFilter);
        assertSize(res, 4);
        assert(res[0].record.get("name").toText() == "Z");
        assert(res[0].record.getDepth() == 1);
        assert(res[1].record.get("name").toText() == "B");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "C");
        assert(res[2].record.getDepth() == 2);
        assert(res[3].record.get("name").toText() == "D");
        assert(res[3].record.getDepth() == 3);

        res = nogdb::Traverse::allEdgeDfs(txn, a, 2, 4, pathFilter);
        assertSize(res, 2);
        assert(res[0].record.get("name").toText() == "C");
        assert(res[0].record.getDepth() == 2);
        assert(res[1].record.get("name").toText() == "D");
        assert(res[1].record.getDepth() == 3);

        pathFilter.setVertex([](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() >= 1500ULL);
        }).setEdge([](const nogdb::Record &record) {
            return (record.get("distance").toIntU() >= 100U);
        });
        res = nogdb::Traverse::allEdgeDfs(txn, a, 0, 4, pathFilter);
        assertSize(res, 4);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "D");
        assert(res[2].record.getDepth() == 2);
        assert(res[3].record.get("name").toText() == "F");
        assert(res[3].record.getDepth() == 3);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_shortest_path_with_condition() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor a, b, c, d, e, f, z;
    try {
        for (const auto &res: nogdb::Vertex::get(txn, "country")) {
            switch (res.record.get("name").toText().c_str()[0]) {
                case 'A':
                    a = res.descriptor;
                    break;
                case 'B':
                    b = res.descriptor;
                    break;
                case 'C':
                    c = res.descriptor;
                    break;
                case 'D':
                    d = res.descriptor;
                    break;
                case 'E':
                    e = res.descriptor;
                    break;
                case 'F':
                    f = res.descriptor;
                    break;
                case 'Z':
                    z = res.descriptor;
                    break;
            }
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto pathFilter = nogdb::PathFilter{}.setVertex([](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() >= 1000ULL);
        }).setEdge([](const nogdb::Record &record) {
            return (record.get("distance").toIntU() <= 120U);
        });
        auto res = nogdb::Traverse::shortestPath(txn, a, f, pathFilter);
        assertSize(res, 5);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "B");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "C");
        assert(res[2].record.getDepth() == 2);
        assert(res[3].record.get("name").toText() == "D");
        assert(res[3].record.getDepth() == 3);
        assert(res[4].record.get("name").toText() == "F");
        assert(res[4].record.getDepth() == 4);

        pathFilter = nogdb::PathFilter{}.setVertex([](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() < 5000ULL);
        }).setEdge([](const nogdb::Record &record) {
            return (record.get("distance").toIntU() <= 200U);
        });
        res = nogdb::Traverse::shortestPath(txn, a, f, pathFilter);
        assertSize(res, 4);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "B");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "C");
        assert(res[2].record.getDepth() == 2);
        assert(res[3].record.get("name").toText() == "F");
        assert(res[3].record.getDepth() == 3);

        pathFilter = nogdb::PathFilter{}.setEdge([](const nogdb::Record &record) {
            return (record.get("distance").toIntU() <= 200U);
        });
        res = nogdb::Traverse::shortestPath(txn, a, f, pathFilter);
        assertSize(res, 4);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "B");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "C");
        assert(res[2].record.getDepth() == 2);
        assert(res[3].record.get("name").toText() == "F");
        assert(res[3].record.getDepth() == 3);

        pathFilter = nogdb::PathFilter{}.setEdge([](const nogdb::Record &record) {
            return record.get("distance").toIntU() >= 100U && record.get("distance").toIntU() != 150U;
        });
        res = nogdb::Traverse::shortestPath(txn, a, f, pathFilter);
        assertSize(res, 4);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "D");
        assert(res[2].record.getDepth() == 2);
        assert(res[3].record.get("name").toText() == "F");
        assert(res[3].record.getDepth() == 3);

        pathFilter = nogdb::PathFilter{}.setEdge([](const nogdb::Record &record) {
            return record.get("distance").toIntU() >= 1000U;
        });
        res = nogdb::Traverse::shortestPath(txn, a, f, pathFilter);
        assert(res.empty());
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_bfs_traverse_in_cursor() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, a, b, c, d, e, f, Z;
    try {
        auto rsCursor = nogdb::Vertex::getCursor(txn, "folders");
        while (rsCursor.next()) {
            switch (rsCursor->record.getText("name").c_str()[0]) {
                case 'A':
                    A = rsCursor->descriptor;
                    break;
                case 'B':
                    B = rsCursor->descriptor;
                    break;
                case 'C':
                    C = rsCursor->descriptor;
                    break;
                case 'D':
                    D = rsCursor->descriptor;
                    break;
                case 'E':
                    E = rsCursor->descriptor;
                    break;
                case 'F':
                    F = rsCursor->descriptor;
                    break;
                case 'G':
                    G = rsCursor->descriptor;
                    break;
                case 'H':
                    H = rsCursor->descriptor;
                    break;
                case 'Z':
                    Z = rsCursor->descriptor;
                    break;
            }
        }

        rsCursor = nogdb::Vertex::getCursor(txn, "files");
        while (rsCursor.next()) {
            switch (rsCursor->record.getText("name").c_str()[0]) {
                case 'a':
                    a = rsCursor->descriptor;
                    break;
                case 'b':
                    b = rsCursor->descriptor;
                    break;
                case 'c':
                    c = rsCursor->descriptor;
                    break;
                case 'd':
                    d = rsCursor->descriptor;
                    break;
                case 'e':
                    e = rsCursor->descriptor;
                    break;
                case 'f':
                    f = rsCursor->descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, D, 1, 1, nogdb::ClassFilter{"link"});
        rsCursor.next();
        auto name = rsCursor->record.get("name").toText();
        assert(name == "B");
        auto depth = rsCursor->record.getDepth();
        assert(depth == 1);

        rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, D, 0, 2, nogdb::ClassFilter{"link"});
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "D");
        assert(rsCursor->record.getDepth() == 0);
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "B");
        assert(rsCursor->record.getDepth() == 1);
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "A");
        assert(rsCursor->record.getDepth() == 2);

        rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, D, 1, 3, nogdb::ClassFilter{"link"});
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "B");
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "A");

        rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, D, 0, 0, nogdb::ClassFilter{"link"});
        rsCursor.next();
        name = rsCursor->record.get("name").toText();
        assert(name == "D");
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, H, 1, 10, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 3);
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "F");
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "C");
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "A");

        rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, f, 1, 4, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 4);
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "G");
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "E");
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "B");
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "A");

        rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, f, 0, 4);
        assert(rsCursor.size() == 6);

        rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, f, 0, 100);
        assert(rsCursor.size() == 6);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto classNames = std::set<std::string>{"link", "symbolic"};
        auto rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, b, 0, 1,
                                                         nogdb::ClassFilter{classNames.cbegin(), classNames.cend()});
        assert(rsCursor.size() == 2);
        rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, b, 1, 2);
        assert(rsCursor.size() == 2);
        rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, e, 1, 1);
        assert(rsCursor.size() == 2);
        rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, e, 0, 2);
        assert(rsCursor.size() == 6);
        rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, e, 0, 3);
        assert(rsCursor.size() == 8);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, Z, 0, 1);
        assert(rsCursor.size() == 1);
        rsCursor = nogdb::Traverse::inEdgeBfsCursor(txn, Z, 0, 100);
        assert(rsCursor.size() == 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_bfs_traverse_out_cursor() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, a, b, c, d, e, f, Z;
    try {
        auto rsCursor = nogdb::Vertex::getCursor(txn, "folders");
        while (rsCursor.next()) {
            switch (rsCursor->record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = rsCursor->descriptor;
                    break;
                case 'B':
                    B = rsCursor->descriptor;
                    break;
                case 'C':
                    C = rsCursor->descriptor;
                    break;
                case 'D':
                    D = rsCursor->descriptor;
                    break;
                case 'E':
                    E = rsCursor->descriptor;
                    break;
                case 'F':
                    F = rsCursor->descriptor;
                    break;
                case 'G':
                    G = rsCursor->descriptor;
                    break;
                case 'H':
                    H = rsCursor->descriptor;
                    break;
                case 'Z':
                    Z = rsCursor->descriptor;
                    break;
            }
        }

        rsCursor = nogdb::Vertex::getCursor(txn, "files");
        while (rsCursor.next()) {
            switch (rsCursor->record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = rsCursor->descriptor;
                    break;
                case 'b':
                    b = rsCursor->descriptor;
                    break;
                case 'c':
                    c = rsCursor->descriptor;
                    break;
                case 'd':
                    d = rsCursor->descriptor;
                    break;
                case 'e':
                    e = rsCursor->descriptor;
                    break;
                case 'f':
                    f = rsCursor->descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, C, 1, 1, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 2);
        while (rsCursor.next()) {
            auto name = rsCursor->record.get("name").toText();
            assert(name == "c" || name == "F");
        }
        rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, C, 0, 2, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 6);
        rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, C, 0, 3, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 6);
        rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, C, 0, 0, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 1);
        rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, A, 1, 1, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 3);
        rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, A, 1, 2, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 8);
        rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, A, 1, 3, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 12);
        rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, A, 1, 4, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 13);
        rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, A, 1, 100, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 13);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto classNames = std::vector<std::string>{"link", "symbolic"};
        auto rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, B, 1, 1,
                                                          nogdb::ClassFilter{classNames.cbegin(), classNames.cend()});
        assert(rsCursor.size() == 3);
        rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, C, 0, 1);
        assert(rsCursor.size() == 4);
        rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, a, 0, 0);
        assert(rsCursor.size() == 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, Z, 0, 1);
        assert(rsCursor.size() == 1);
        rsCursor = nogdb::Traverse::outEdgeBfsCursor(txn, Z, 0, 100);
        assert(rsCursor.size() == 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_bfs_traverse_all_cursor() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto rsCursor = nogdb::Vertex::getCursor(txn, "folders");
        while (rsCursor.next()) {
            switch (rsCursor->record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = rsCursor->descriptor;
                    break;
                case 'B':
                    B = rsCursor->descriptor;
                    break;
                case 'C':
                    C = rsCursor->descriptor;
                    break;
                case 'D':
                    D = rsCursor->descriptor;
                    break;
                case 'E':
                    E = rsCursor->descriptor;
                    break;
                case 'F':
                    F = rsCursor->descriptor;
                    break;
                case 'G':
                    G = rsCursor->descriptor;
                    break;
                case 'H':
                    H = rsCursor->descriptor;
                    break;
                case 'Z':
                    Z = rsCursor->descriptor;
                    break;
            }
        }

        rsCursor = nogdb::Vertex::getCursor(txn, "files");
        while (rsCursor.next()) {
            switch (rsCursor->record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = rsCursor->descriptor;
                    break;
                case 'b':
                    b = rsCursor->descriptor;
                    break;
                case 'c':
                    c = rsCursor->descriptor;
                    break;
                case 'd':
                    d = rsCursor->descriptor;
                    break;
                case 'e':
                    e = rsCursor->descriptor;
                    break;
                case 'f':
                    f = rsCursor->descriptor;
                    break;
            }
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, F, 1, 1, nogdb::ClassFilter{"link"});
        while (rsCursor.next()) {
            auto name = rsCursor->record.get("name").toText();
            assert(name == "d" || name == "C" || name == "H" || name == "e");
        }
        assert(rsCursor.size() == 4);

        rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, F, 0, 2, nogdb::ClassFilter{"link"});
        while (rsCursor.next()) {
            auto name = rsCursor->record.get("name").toText();
            assert(name == "F" || name == "d" || name == "C" || name == "H" || name == "e" || name == "A" ||
                   name == "c");
        }
        assert(rsCursor.size() == 7);

        rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, F, 1, 3, nogdb::ClassFilter{"link"});
        while (rsCursor.next()) {
            auto name = rsCursor->record.get("name").toText();
            assert(name == "d" || name == "C" || name == "H" || name == "e" ||
                   name == "A" || name == "c" || name == "a" || name == "B");
        }
        assert(rsCursor.count() == 8);

        rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, F, 0, 0, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 1);
        rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, F, 0, 100, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 14);
        rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, F, 2, 1, nogdb::ClassFilter{"link"});
        assert(rsCursor.empty());
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, H, 1, 1, nogdb::ClassFilter{"symbolic"});
        assert(rsCursor.size() == 1);
        rsCursor.next();
        auto name = rsCursor->record.get("name").toText();
        assert(name == "C");

        rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, H, 2, 2, nogdb::ClassFilter{"symbolic"});
        rsCursor.next();
        name = rsCursor->record.get("name").toText();
        assert(name == "e");
        assert(rsCursor.count() == 1);

        rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, H, 1, 3, nogdb::ClassFilter{"symbolic"});
        assert(rsCursor.size() == 2);
        rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, H, 0, 0, nogdb::ClassFilter{"symbolic"});
        assert(rsCursor.size() == 1);
        rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, H, 0, 100, nogdb::ClassFilter{"symbolic"});
        assert(rsCursor.size() == 3);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto classNames = std::list<std::string>{"link", "symbolic"};
        auto rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, A, 1, 1,
                                                          nogdb::ClassFilter{classNames.cbegin(), classNames.cend()});
        while (rsCursor.next()) {
            auto name = rsCursor->record.get("name").toText();
            assert(name == "B" || name == "a" || name == "C" || name == "D");
        }
        assert(rsCursor.count() == 4);

        rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, A, 1, 2);
        assert(rsCursor.size() == 11);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, Z, 0, 1);
        assert(rsCursor.size() == 1);
        rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, Z, 0, 100);
        assert(rsCursor.size() == 1);
        rsCursor = nogdb::Traverse::allEdgeBfsCursor(txn, Z, 0, 0);
        assert(rsCursor.size() == 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_invalid_bfs_traverse_in_cursor() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto res = nogdb::Vertex::getCursor(txn, "folders");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res->descriptor;
                    break;
                case 'B':
                    B = res->descriptor;
                    break;
                case 'C':
                    C = res->descriptor;
                    break;
                case 'D':
                    D = res->descriptor;
                    break;
                case 'E':
                    E = res->descriptor;
                    break;
                case 'F':
                    F = res->descriptor;
                    break;
                case 'G':
                    G = res->descriptor;
                    break;
                case 'H':
                    H = res->descriptor;
                    break;
                case 'Z':
                    Z = res->descriptor;
                    break;
            }
        }

        res = nogdb::Vertex::getCursor(txn, "files");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res->descriptor;
                    break;
                case 'b':
                    b = res->descriptor;
                    break;
                case 'c':
                    c = res->descriptor;
                    break;
                case 'd':
                    d = res->descriptor;
                    break;
                case 'e':
                    e = res->descriptor;
                    break;
                case 'f':
                    f = res->descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeBfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"ling"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeBfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"link", "symbol"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeBfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeBfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"link", "folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = nogdb::Traverse::inEdgeBfsCursor(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = nogdb::Traverse::inEdgeBfsCursor(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }

}

void test_invalid_bfs_traverse_out_cursor() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto res = nogdb::Vertex::getCursor(txn, "folders");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res->descriptor;
                    break;
                case 'B':
                    B = res->descriptor;
                    break;
                case 'C':
                    C = res->descriptor;
                    break;
                case 'D':
                    D = res->descriptor;
                    break;
                case 'E':
                    E = res->descriptor;
                    break;
                case 'F':
                    F = res->descriptor;
                    break;
                case 'G':
                    G = res->descriptor;
                    break;
                case 'H':
                    H = res->descriptor;
                    break;
                case 'Z':
                    Z = res->descriptor;
                    break;
            }
        }

        res = nogdb::Vertex::getCursor(txn, "files");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res->descriptor;
                    break;
                case 'b':
                    b = res->descriptor;
                    break;
                case 'c':
                    c = res->descriptor;
                    break;
                case 'd':
                    d = res->descriptor;
                    break;
                case 'e':
                    e = res->descriptor;
                    break;
                case 'f':
                    f = res->descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeBfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"ling"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeBfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"link", "symbol"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeBfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeBfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"link", "folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = nogdb::Traverse::outEdgeBfsCursor(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = nogdb::Traverse::outEdgeBfsCursor(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_invalid_bfs_traverse_all_cursor() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto res = nogdb::Vertex::getCursor(txn, "folders");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res->descriptor;
                    break;
                case 'B':
                    B = res->descriptor;
                    break;
                case 'C':
                    C = res->descriptor;
                    break;
                case 'D':
                    D = res->descriptor;
                    break;
                case 'E':
                    E = res->descriptor;
                    break;
                case 'F':
                    F = res->descriptor;
                    break;
                case 'G':
                    G = res->descriptor;
                    break;
                case 'H':
                    H = res->descriptor;
                    break;
                case 'Z':
                    Z = res->descriptor;
                    break;
            }
        }

        res = nogdb::Vertex::getCursor(txn, "files");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res->descriptor;
                    break;
                case 'b':
                    b = res->descriptor;
                    break;
                case 'c':
                    c = res->descriptor;
                    break;
                case 'd':
                    d = res->descriptor;
                    break;
                case 'e':
                    e = res->descriptor;
                    break;
                case 'f':
                    f = res->descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeBfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"ling"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeBfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"link", "symbol"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeBfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeBfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"link", "folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = nogdb::Traverse::allEdgeBfsCursor(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = nogdb::Traverse::allEdgeBfsCursor(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_dfs_traverse_in_cursor() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, a, b, c, d, e, f, Z;
    try {
        auto rsCursor = nogdb::Vertex::getCursor(txn, "folders");
        while (rsCursor.next()) {
            switch (rsCursor->record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = rsCursor->descriptor;
                    break;
                case 'B':
                    B = rsCursor->descriptor;
                    break;
                case 'C':
                    C = rsCursor->descriptor;
                    break;
                case 'D':
                    D = rsCursor->descriptor;
                    break;
                case 'E':
                    E = rsCursor->descriptor;
                    break;
                case 'F':
                    F = rsCursor->descriptor;
                    break;
                case 'G':
                    G = rsCursor->descriptor;
                    break;
                case 'H':
                    H = rsCursor->descriptor;
                    break;
                case 'Z':
                    Z = rsCursor->descriptor;
                    break;
            }
        }

        rsCursor = nogdb::Vertex::getCursor(txn, "files");
        while (rsCursor.next()) {
            switch (rsCursor->record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = rsCursor->descriptor;
                    break;
                case 'b':
                    b = rsCursor->descriptor;
                    break;
                case 'c':
                    c = rsCursor->descriptor;
                    break;
                case 'd':
                    d = rsCursor->descriptor;
                    break;
                case 'e':
                    e = rsCursor->descriptor;
                    break;
                case 'f':
                    f = rsCursor->descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = nogdb::Traverse::inEdgeDfsCursor(txn, D, 0, 1, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 2);
        while (rsCursor.next()) {
            auto name = rsCursor->record.get("name").toText();
            assert(name == "B" || name == "D");
            if (name == "B") assert(rsCursor->record.getDepth() == 1);
            else assert(rsCursor->record.getDepth() == 0);
        }

        rsCursor = nogdb::Traverse::inEdgeDfsCursor(txn, D, 1, 2, nogdb::ClassFilter{"link"});
        assert(rsCursor.size() == 2);
        while (rsCursor.next()) {
            auto name = rsCursor->record.get("name").toText();
            assert(name == "B" || name == "A");
            if (name == "B") assert(rsCursor->record.getDepth() == 1);
            else assert(rsCursor->record.getDepth() == 2);
        }

        rsCursor = nogdb::Traverse::inEdgeDfsCursor(txn, D, 3, 4, nogdb::ClassFilter{"link"});
        assert(rsCursor.empty());

        rsCursor = nogdb::Traverse::inEdgeDfsCursor(txn, D, 0, 0, nogdb::ClassFilter{"link"});
        while (rsCursor.next()) {
            auto name = rsCursor->record.get("name").toText();
            assert(name == "D");
        }
        assert(rsCursor.count() == 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::inEdgeDfsCursor(txn, H, 1, 100, nogdb::ClassFilter{"link"});
        assertSize(res, 3);
        cursorContains(res, std::set<std::string>{"F", "C", "A"}, "name");

        res = nogdb::Traverse::inEdgeDfsCursor(txn, f, 1, 4, nogdb::ClassFilter{"link"});
        assertSize(res, 4);
        cursorContains(res, std::set<std::string>{"G", "E", "B", "A"}, "name");

        res = nogdb::Traverse::inEdgeDfsCursor(txn, f, 0, 4);
        assertSize(res, 6);

        res = nogdb::Traverse::inEdgeDfsCursor(txn, f, 0, 100);
        assertSize(res, 6);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::inEdgeDfsCursor(txn, b, 1, 1, nogdb::ClassFilter{"link", "symbolic"});
        assertSize(res, 1);
        res = nogdb::Traverse::inEdgeDfsCursor(txn, b, 1, 2);
        assertSize(res, 2);
        res = nogdb::Traverse::inEdgeDfsCursor(txn, e, 1, 1);
        assertSize(res, 2);
        cursorContains(res, std::set<std::string>{"F", "C"}, "name");
        res = nogdb::Traverse::inEdgeDfsCursor(txn, e, 1, 2);
        assertSize(res, 3);
        cursorContains(res, std::set<std::string>{"F", "C", "E"}, "name");
        res = nogdb::Traverse::inEdgeDfsCursor(txn, e, 0, 3);
        assertSize(res, 7);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::inEdgeDfsCursor(txn, Z, 0, 1);
        assertSize(res, 1);
        res = nogdb::Traverse::inEdgeDfsCursor(txn, Z, 0, 100);
        assertSize(res, 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_dfs_traverse_out_cursor() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, a, b, c, d, e, f, Z;
    try {
        auto res = nogdb::Vertex::getCursor(txn, "folders");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res->descriptor;
                    break;
                case 'B':
                    B = res->descriptor;
                    break;
                case 'C':
                    C = res->descriptor;
                    break;
                case 'D':
                    D = res->descriptor;
                    break;
                case 'E':
                    E = res->descriptor;
                    break;
                case 'F':
                    F = res->descriptor;
                    break;
                case 'G':
                    G = res->descriptor;
                    break;
                case 'H':
                    H = res->descriptor;
                    break;
                case 'Z':
                    Z = res->descriptor;
                    break;
            }
        }

        res = nogdb::Vertex::getCursor(txn, "files");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res->descriptor;
                    break;
                case 'b':
                    b = res->descriptor;
                    break;
                case 'c':
                    c = res->descriptor;
                    break;
                case 'd':
                    d = res->descriptor;
                    break;
                case 'e':
                    e = res->descriptor;
                    break;
                case 'f':
                    f = res->descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::outEdgeDfsCursor(txn, C, 0, 1, nogdb::ClassFilter{"link"});
        assertSize(res, 3);
        while (res.next()) {
            auto name = res->record.get("name").toText();
            assert(name == "c" || name == "F" || name == "C");
        }
        res = nogdb::Traverse::outEdgeDfsCursor(txn, C, 1, 2, nogdb::ClassFilter{"link"});
        assertSize(res, 5);
        cursorContains(res, std::set<std::string>{"c", "F", "d", "H", "e"}, "name");
        res = nogdb::Traverse::outEdgeDfsCursor(txn, C, 0, 3, nogdb::ClassFilter{"link"});
        assertSize(res, 6);
        res = nogdb::Traverse::outEdgeDfsCursor(txn, C, 0, 0, nogdb::ClassFilter{"link"});
        assertSize(res, 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::outEdgeDfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"link"});
        assertSize(res, 1);
        res = nogdb::Traverse::outEdgeDfsCursor(txn, A, 1, 1, nogdb::ClassFilter{"link"});
        assertSize(res, 3);
        res = nogdb::Traverse::outEdgeDfsCursor(txn, A, 1, 2, nogdb::ClassFilter{"link"});
        assertSize(res, 8);
        cursorContains(res, std::set<std::string>{"B", "D", "E", "b", "a", "C", "c", "F"}, "name");
        res = nogdb::Traverse::outEdgeDfsCursor(txn, A, 1, 3, nogdb::ClassFilter{"link"});
        assertSize(res, 12);
        res = nogdb::Traverse::outEdgeDfsCursor(txn, A, 1, 4, nogdb::ClassFilter{"link"});
        assertSize(res, 13);
        res = nogdb::Traverse::outEdgeDfsCursor(txn, A, 1, 100, nogdb::ClassFilter{"link"});
        assertSize(res, 13);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::outEdgeDfsCursor(txn, B, 1, 1, nogdb::ClassFilter{"link", "symbolic"});
        assertSize(res, 3);
        res = nogdb::Traverse::outEdgeDfsCursor(txn, C, 1, 1);
        assertSize(res, 3);
        res = nogdb::Traverse::outEdgeDfsCursor(txn, a, 0, 0);
        assertSize(res, 1);

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::outEdgeDfsCursor(txn, Z, 0, 1);
        assertSize(res, 1);
        res = nogdb::Traverse::outEdgeDfsCursor(txn, Z, 0, 100);
        assertSize(res, 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_dfs_traverse_all_cursor() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto res = nogdb::Vertex::getCursor(txn, "folders");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res->descriptor;
                    break;
                case 'B':
                    B = res->descriptor;
                    break;
                case 'C':
                    C = res->descriptor;
                    break;
                case 'D':
                    D = res->descriptor;
                    break;
                case 'E':
                    E = res->descriptor;
                    break;
                case 'F':
                    F = res->descriptor;
                    break;
                case 'G':
                    G = res->descriptor;
                    break;
                case 'H':
                    H = res->descriptor;
                    break;
                case 'Z':
                    Z = res->descriptor;
                    break;
            }
        }

        res = nogdb::Vertex::getCursor(txn, "files");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res->descriptor;
                    break;
                case 'b':
                    b = res->descriptor;
                    break;
                case 'c':
                    c = res->descriptor;
                    break;
                case 'd':
                    d = res->descriptor;
                    break;
                case 'e':
                    e = res->descriptor;
                    break;
                case 'f':
                    f = res->descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::allEdgeDfsCursor(txn, F, 0, 1, nogdb::ClassFilter{"link"});
        while (res.next()) {
            auto name = res->record.get("name").toText();
            assert(name == "d" || name == "C" || name == "H" || name == "e" || name == "F");
        }
        assert(res.count() == 5);

        res = nogdb::Traverse::allEdgeDfsCursor(txn, F, 1, 2, nogdb::ClassFilter{"link"});
        while (res.next()) {
            auto name = res->record.get("name").toText();
            assert(name == "d" || name == "C" || name == "H" || name == "e" || name == "A" || name == "c");
        }
        assert(res.count() == 6);

        res = nogdb::Traverse::allEdgeDfsCursor(txn, F, 1, 3, nogdb::ClassFilter{"link"});
        while (res.next()) {
            auto name = res->record.get("name").toText();
            assert(name == "d" || name == "C" || name == "H" || name == "e" ||
                   name == "A" || name == "c" || name == "a" || name == "B");
        }
        assert(res.count() == 8);

        res = nogdb::Traverse::allEdgeDfsCursor(txn, F, 0, 0, nogdb::ClassFilter{"link"});
        assertSize(res, 1);

        res = nogdb::Traverse::allEdgeDfsCursor(txn, F, 4, 100, nogdb::ClassFilter{"link"});
        assertSize(res, 5);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::allEdgeDfsCursor(txn, H, 1, 1, nogdb::ClassFilter{"symbolic"});
        while (res.next()) {
            auto name = res->record.get("name").toText();
            assert(name == "C");
        }
        assert(res.count() == 1);

        res = nogdb::Traverse::allEdgeDfsCursor(txn, H, 1, 2, nogdb::ClassFilter{"symbolic"});
        while (res.next()) {
            auto name = res->record.get("name").toText();
            assert(name == "C" || name == "e");
        }
        assert(res.count() == 2);

        res = nogdb::Traverse::allEdgeDfsCursor(txn, H, 1, 3, nogdb::ClassFilter{"symbolic"});
        assertSize(res, 2);
        res = nogdb::Traverse::allEdgeDfsCursor(txn, H, 0, 0, nogdb::ClassFilter{"symbolic"});
        assertSize(res, 1);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::allEdgeDfsCursor(txn, A, 1, 1, nogdb::ClassFilter{"link", "symbolic"});
        while (res.next()) {
            auto name = res->record.get("name").toText();
            assert(name == "B" || name == "a" || name == "C" || name == "D");
        }
        assert(res.count() == 4);

        res = nogdb::Traverse::allEdgeDfsCursor(txn, A, 1, 2);
        assertSize(res, 10);
        cursorContains(res, std::set<std::string>{"B", "D", "E", "b", "a", "C", "c", "F", "e", "H"}, "name");
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::allEdgeDfsCursor(txn, Z, 0, 1);
        assertSize(res, 1);
        res = nogdb::Traverse::allEdgeDfsCursor(txn, Z, 0, 100);
        assertSize(res, 1);
        res = nogdb::Traverse::allEdgeDfsCursor(txn, Z, 0, 0);
        assertSize(res, 1);
        res = nogdb::Traverse::allEdgeDfsCursor(txn, Z, 10, 0);
        assert(res.empty());
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_invalid_dfs_traverse_in_cursor() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto res = nogdb::Vertex::getCursor(txn, "folders");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res->descriptor;
                    break;
                case 'B':
                    B = res->descriptor;
                    break;
                case 'C':
                    C = res->descriptor;
                    break;
                case 'D':
                    D = res->descriptor;
                    break;
                case 'E':
                    E = res->descriptor;
                    break;
                case 'F':
                    F = res->descriptor;
                    break;
                case 'G':
                    G = res->descriptor;
                    break;
                case 'H':
                    H = res->descriptor;
                    break;
                case 'Z':
                    Z = res->descriptor;
                    break;
            }
        }

        res = nogdb::Vertex::getCursor(txn, "files");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res->descriptor;
                    break;
                case 'b':
                    b = res->descriptor;
                    break;
                case 'c':
                    c = res->descriptor;
                    break;
                case 'd':
                    d = res->descriptor;
                    break;
                case 'e':
                    e = res->descriptor;
                    break;
                case 'f':
                    f = res->descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeDfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"ling"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeDfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"link", "symbol"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeDfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::inEdgeDfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"link", "folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = nogdb::Traverse::inEdgeDfsCursor(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = nogdb::Traverse::inEdgeDfsCursor(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_invalid_dfs_traverse_out_cursor() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto res = nogdb::Vertex::getCursor(txn, "folders");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res->descriptor;
                    break;
                case 'B':
                    B = res->descriptor;
                    break;
                case 'C':
                    C = res->descriptor;
                    break;
                case 'D':
                    D = res->descriptor;
                    break;
                case 'E':
                    E = res->descriptor;
                    break;
                case 'F':
                    F = res->descriptor;
                    break;
                case 'G':
                    G = res->descriptor;
                    break;
                case 'H':
                    H = res->descriptor;
                    break;
                case 'Z':
                    Z = res->descriptor;
                    break;
            }
        }

        res = nogdb::Vertex::getCursor(txn, "files");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res->descriptor;
                    break;
                case 'b':
                    b = res->descriptor;
                    break;
                case 'c':
                    c = res->descriptor;
                    break;
                case 'd':
                    d = res->descriptor;
                    break;
                case 'e':
                    e = res->descriptor;
                    break;
                case 'f':
                    f = res->descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeDfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"ling"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeDfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"link", "symbol"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeDfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::outEdgeDfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"link", "folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = nogdb::Traverse::outEdgeDfsCursor(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = nogdb::Traverse::outEdgeDfsCursor(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_invalid_dfs_traverse_all_cursor() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto res = nogdb::Vertex::getCursor(txn, "folders");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res->descriptor;
                    break;
                case 'B':
                    B = res->descriptor;
                    break;
                case 'C':
                    C = res->descriptor;
                    break;
                case 'D':
                    D = res->descriptor;
                    break;
                case 'E':
                    E = res->descriptor;
                    break;
                case 'F':
                    F = res->descriptor;
                    break;
                case 'G':
                    G = res->descriptor;
                    break;
                case 'H':
                    H = res->descriptor;
                    break;
                case 'Z':
                    Z = res->descriptor;
                    break;
            }
        }

        res = nogdb::Vertex::getCursor(txn, "files");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res->descriptor;
                    break;
                case 'b':
                    b = res->descriptor;
                    break;
                case 'c':
                    c = res->descriptor;
                    break;
                case 'd':
                    d = res->descriptor;
                    break;
                case 'e':
                    e = res->descriptor;
                    break;
                case 'f':
                    f = res->descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeDfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"ling"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeDfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"link", "symbol"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeDfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto res = nogdb::Traverse::allEdgeDfsCursor(txn, A, 0, 0, nogdb::ClassFilter{"link", "folders"});
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = nogdb::Traverse::allEdgeDfsCursor(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = nogdb::Traverse::allEdgeDfsCursor(txn, tmp, 0, 0);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_shortest_path_cursor() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto res = nogdb::Vertex::getCursor(txn, "folders");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res->descriptor;
                    break;
                case 'B':
                    B = res->descriptor;
                    break;
                case 'C':
                    C = res->descriptor;
                    break;
                case 'D':
                    D = res->descriptor;
                    break;
                case 'E':
                    E = res->descriptor;
                    break;
                case 'F':
                    F = res->descriptor;
                    break;
                case 'G':
                    G = res->descriptor;
                    break;
                case 'H':
                    H = res->descriptor;
                    break;
                case 'Z':
                    Z = res->descriptor;
                    break;
            }
        }

        res = nogdb::Vertex::getCursor(txn, "files");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res->descriptor;
                    break;
                case 'b':
                    b = res->descriptor;
                    break;
                case 'c':
                    c = res->descriptor;
                    break;
                case 'd':
                    d = res->descriptor;
                    break;
                case 'e':
                    e = res->descriptor;
                    break;
                case 'f':
                    f = res->descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::shortestPathCursor(txn, A, f);
        cursorContains(res, std::set<std::string>{"A", "B", "D", "f"}, "name");
        assertSize(res, 4);
        res.first();
        assert(res->record.getDepth() == 0); res.next();
        assert(res->record.getDepth() == 1); res.next();
        assert(res->record.getDepth() == 2); res.next();
        assert(res->record.getDepth() == 3);

        res = nogdb::Traverse::shortestPathCursor(txn, A, e);
        cursorContains(res, std::set<std::string>{"A", "C", "e"}, "name");
        assertSize(res, 3);

        res = nogdb::Traverse::shortestPathCursor(txn, D, f);
        cursorContains(res, std::set<std::string>{"D", "f"}, "name");
        assertSize(res, 2);

        res = nogdb::Traverse::shortestPathCursor(txn, B, A);
        cursorContains(res, std::set<std::string>{"B", "D", "A"}, "name");
        assertSize(res, 3);

        res = nogdb::Traverse::shortestPathCursor(txn, A, e, nogdb::ClassFilter{"link", "symbolic"});
        cursorContains(res, std::set<std::string>{"A", "C", "e"}, "name");
        assertSize(res, 3);

        res = nogdb::Traverse::shortestPathCursor(txn, D, f, nogdb::ClassFilter{"link", "symbolic"});
        cursorContains(res, std::set<std::string>{"D", "f"}, "name");
        assertSize(res, 2);

        res = nogdb::Traverse::shortestPathCursor(txn, B, A, nogdb::ClassFilter{"link", "symbolic"});
        cursorContains(res, std::set<std::string>{"B", "D", "A"}, "name");
        assertSize(res, 3);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::shortestPathCursor(txn, a, a);
        assertSize(res, 1);
        res.next();
        assert(res->record.get("name").toText() == "a");
        assert(res->record.getDepth() == 0);

        res = nogdb::Traverse::shortestPathCursor(txn, f, f);
        assertSize(res, 1);
        res.next();
        assert(res->record.get("name").toText() == "f");

        res = nogdb::Traverse::shortestPathCursor(txn, B, B);
        assertSize(res, 1);
        res.next();
        assert(res->record.get("name").toText() == "B");

        res = nogdb::Traverse::shortestPathCursor(txn, A, Z);
        assert(res.empty());

        res = nogdb::Traverse::shortestPathCursor(txn, Z, G);
        assert(res.empty());

        res = nogdb::Traverse::shortestPathCursor(txn, a, F);
        assert(res.empty());

        res = nogdb::Traverse::shortestPathCursor(txn, d, A);
        assert(res.empty());

        res = nogdb::Traverse::shortestPathCursor(txn, A, b);
        cursorContains(res, std::set<std::string>{"A", "B", "b"}, "name");
        assertSize(res, 3);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = nogdb::Traverse::shortestPathCursor(txn, C, e, nogdb::ClassFilter{"link"});
        assertSize(res, 3);

        res = nogdb::Traverse::shortestPathCursor(txn, B, d);
        assertSize(res, 4);
        res = nogdb::Traverse::shortestPathCursor(txn, B, d, nogdb::ClassFilter{"link"});
        assert(res.count() == 0);

        res = nogdb::Traverse::shortestPathCursor(txn, H, C, nogdb::ClassFilter{"link"});
        assert(res.count() == 0);
        res = nogdb::Traverse::shortestPathCursor(txn, H, C, nogdb::ClassFilter{"symbolic"});
        assertSize(res, 2);
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_invalid_shortest_path_cursor() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto res = nogdb::Vertex::getCursor(txn, "folders");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'A':
                    A = res->descriptor;
                    break;
                case 'B':
                    B = res->descriptor;
                    break;
                case 'C':
                    C = res->descriptor;
                    break;
                case 'D':
                    D = res->descriptor;
                    break;
                case 'E':
                    E = res->descriptor;
                    break;
                case 'F':
                    F = res->descriptor;
                    break;
                case 'G':
                    G = res->descriptor;
                    break;
                case 'H':
                    H = res->descriptor;
                    break;
                case 'Z':
                    Z = res->descriptor;
                    break;
            }
        }

        res = nogdb::Vertex::getCursor(txn, "files");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'a':
                    a = res->descriptor;
                    break;
                case 'b':
                    b = res->descriptor;
                    break;
                case 'c':
                    c = res->descriptor;
                    break;
                case 'd':
                    d = res->descriptor;
                    break;
                case 'e':
                    e = res->descriptor;
                    break;
                case 'f':
                    f = res->descriptor;
                    break;
            }
        }

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.second = 999U;
        auto res = nogdb::Traverse::shortestPathCursor(txn, tmp, B);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_SRC, "NOGDB_GRAPH_NOEXST_SRC");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = B;
        tmp.rid.second = 999U;
        auto res = nogdb::Traverse::shortestPathCursor(txn, A, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_DST, "NOGDB_GRAPH_NOEXST_DST");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = nogdb::Traverse::shortestPathCursor(txn, tmp, D);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto links = nogdb::Edge::get(txn, "link");
        auto tmp = links[0].descriptor;
        auto res = nogdb::Traverse::shortestPathCursor(txn, A, tmp);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    try {
        auto links = nogdb::Edge::get(txn, "link");
        auto tmp = links[0].descriptor;
        auto res = nogdb::Traverse::shortestPathCursor(txn, tmp, f);
        assert(false);
    } catch (const nogdb::Error &ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }
}

void test_bfs_traverse_cursor_with_condition() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor a, b, c, d, e, f, z;
    try {
        auto res = nogdb::Vertex::getCursor(txn, "country");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'A':
                    a = res->descriptor;
                    break;
                case 'B':
                    b = res->descriptor;
                    break;
                case 'C':
                    c = res->descriptor;
                    break;
                case 'D':
                    d = res->descriptor;
                    break;
                case 'E':
                    e = res->descriptor;
                    break;
                case 'F':
                    f = res->descriptor;
                    break;
                case 'Z':
                    z = res->descriptor;
                    break;
            }
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto edgeCondition = [](const nogdb::Record &record) {
            return (record.get("distance").toIntU() < 100U);
        };
        auto pathFilter = nogdb::PathFilter{}.setEdge(edgeCondition);
        auto res = nogdb::Traverse::outEdgeBfsCursor(txn, a, 0, 1, pathFilter);
        assertSize(res, 2);
        cursorContains(res, std::set<std::string>{"A", "B"}, "name");

        res = nogdb::Traverse::inEdgeBfsCursor(txn, a, 0, 1, pathFilter);
        assertSize(res, 2);
        cursorContains(res, std::set<std::string>{"A", "Z"}, "name");

        auto vertexCondition = [](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() > 1000ULL);
        };
        pathFilter.setVertex(vertexCondition);
        res = nogdb::Traverse::outEdgeBfsCursor(txn, a, 0, 1, pathFilter);
        assertSize(res, 1);
        res.next();
        assert(res->record.get("name").toText() == "A");

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto edgeCondition = [](const nogdb::Record &record) {
            return (record.get("distance").toIntU() > 100U);
        };
        auto pathFilter = nogdb::PathFilter{}.setEdge(edgeCondition);
        auto res = nogdb::Traverse::allEdgeBfsCursor(txn, a, 1, 3, pathFilter);
        assertSize(res, 3);
        cursorContains(res, std::set<std::string>{"C", "D", "F"}, "name");

        res = nogdb::Traverse::allEdgeBfsCursor(txn, a, 2, 4, pathFilter);
        assertSize(res, 1);
        res.first();
        assert(res->record.get("name").toText() == "F");

        auto vertexCondition = [](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() < 4000ULL);
        };
        pathFilter.setVertex(vertexCondition);
        res = nogdb::Traverse::allEdgeBfsCursor(txn, a, 0, 4, pathFilter);
        assertSize(res, 3);
        cursorContains(res, std::set<std::string>{"A", "C", "F"}, "name");

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_dfs_traverse_cursor_with_condition() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor a, b, c, d, e, f, z;
    try {
        auto res = nogdb::Vertex::getCursor(txn, "country");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'A':
                    a = res->descriptor;
                    break;
                case 'B':
                    b = res->descriptor;
                    break;
                case 'C':
                    c = res->descriptor;
                    break;
                case 'D':
                    d = res->descriptor;
                    break;
                case 'E':
                    e = res->descriptor;
                    break;
                case 'F':
                    f = res->descriptor;
                    break;
                case 'Z':
                    z = res->descriptor;
                    break;
            }
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto edgeCondition = [](const nogdb::Record &record) {
            return (record.get("distance").toIntU() <= 100U);
        };
        auto pathFilter = nogdb::PathFilter{}.setEdge(edgeCondition);
        auto res = nogdb::Traverse::outEdgeDfsCursor(txn, a, 1, 3, pathFilter);
        assertSize(res, 3);
        cursorContains(res, std::set<std::string>{"B", "C", "D"}, "name");

        pathFilter.setVertex([](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() >= 1500ULL);
        }).setEdge([](const nogdb::Record &record) {
            return (record.get("distance").toIntU() >= 300U);
        });
        res = nogdb::Traverse::outEdgeDfsCursor(txn, a, 0, 4, pathFilter);
        assertSize(res, 2);
        cursorContains(res, std::set<std::string>{"A", "C"}, "name");

        pathFilter.setVertex([](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() < 1500ULL);
        });
        res = nogdb::Traverse::outEdgeDfsCursor(txn, a, 0, 4, pathFilter);
        assertSize(res, 1);
        res.next();
        assert(res->record.get("name").toText() == "A");
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto edgeCondition = [](const nogdb::Record &record) {
            return (record.get("distance").toIntU() <= 100U);
        };
        auto pathFilter = nogdb::PathFilter{}.setEdge(edgeCondition);
        auto res = nogdb::Traverse::allEdgeDfsCursor(txn, a, 1, 3, pathFilter);
        assertSize(res, 4);
        cursorContains(res, std::set<std::string>{"Z", "B", "C", "D"}, "name");

        res = nogdb::Traverse::allEdgeDfsCursor(txn, a, 2, 4, pathFilter);
        assertSize(res, 2);
        cursorContains(res, std::set<std::string>{"C", "D"}, "name");

        pathFilter.setVertex([](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() >= 1500ULL);
        }).setEdge([](const nogdb::Record &record) {
            return (record.get("distance").toIntU() >= 100U);
        });
        res = nogdb::Traverse::allEdgeDfsCursor(txn, a, 0, 4, pathFilter);
        assertSize(res, 4);
        cursorContains(res, std::set<std::string>{"A", "C", "D", "F"}, "name");
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_shortest_path_cursor_with_condition() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor a, b, c, d, e, f, z;
    try {
        auto res = nogdb::Vertex::getCursor(txn, "country");
        while (res.next()) {
            switch (res->record.get("name").toText().c_str()[0]) {
                case 'A':
                    a = res->descriptor;
                    break;
                case 'B':
                    b = res->descriptor;
                    break;
                case 'C':
                    c = res->descriptor;
                    break;
                case 'D':
                    d = res->descriptor;
                    break;
                case 'E':
                    e = res->descriptor;
                    break;
                case 'F':
                    f = res->descriptor;
                    break;
                case 'Z':
                    z = res->descriptor;
                    break;
            }
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto pathFilter = nogdb::PathFilter{}.setVertex([](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() >= 1000ULL);
        }).setEdge([](const nogdb::Record &record) {
            return (record.get("distance").toIntU() <= 120U);
        });
        auto res = nogdb::Traverse::shortestPathCursor(txn, a, f, pathFilter);
        assertSize(res, 5);
        cursorContains(res, std::set<std::string>{"A", "B", "C", "D", "F"}, "name");
        pathFilter = nogdb::PathFilter{}.setVertex([](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() < 5000ULL);
        }).setEdge([](const nogdb::Record &record) {
            return (record.get("distance").toIntU() <= 200U);
        });
        res = nogdb::Traverse::shortestPathCursor(txn, a, f, pathFilter);
        assertSize(res, 4);
        cursorContains(res, std::set<std::string>{"A", "B", "C", "F"}, "name");

        pathFilter = nogdb::PathFilter{}.setEdge([](const nogdb::Record &record) {
            return (record.get("distance").toIntU() <= 200U);
        });
        res = nogdb::Traverse::shortestPathCursor(txn, a, f, pathFilter);
        assertSize(res, 4);
        cursorContains(res, std::set<std::string>{"A", "B", "C", "F"}, "name");

        pathFilter = nogdb::PathFilter{}.setEdge([](const nogdb::Record &record) {
            return record.get("distance").toIntU() >= 100U && record.get("distance").toIntU() != 150U;
        });
        res = nogdb::Traverse::shortestPathCursor(txn, a, f, pathFilter);
        assertSize(res, 4);
        cursorContains(res, std::set<std::string>{"A", "C", "D", "F"}, "name");

        pathFilter = nogdb::PathFilter{}.setEdge([](const nogdb::Record &record) {
            return record.get("distance").toIntU() >= 1000U;
        });
        res = nogdb::Traverse::shortestPathCursor(txn, a, f, pathFilter);
        assert(res.empty());
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

/*
void test_shortest_path_dijkstra() {

    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    nogdb::RecordDescriptor a, b, c, d, e, f, z;
    try {
        auto res = nogdb::Vertex::getCursor(txn, "country");
        while (res.next()) {
            switch (res->record.get("name").toText().at(0)) {
                case 'A':
                    a = res->descriptor;
                    break;
                case 'B':
                    b = res->descriptor;
                    break;
                case 'C':
                    c = res->descriptor;
                    break;
                case 'D':
                    d = res->descriptor;
                    break;
                case 'E':
                    e = res->descriptor;
                    break;
                case 'F':
                    f = res->descriptor;
                    break;
                case 'Z':
                    z = res->descriptor;
                    break;
            }
        }
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto pathFilter = nogdb::PathFilter{}.setVertex([](const nogdb::Record &record) {
            return (record.get("population").toBigIntU() >= 1000ULL);
        }).setEdge([](const nogdb::Record &record) {
            return (record.get("distance").toIntU() <= 150U);
        });

        // normal traversal
        auto costFunction = [](const nogdb::Txn &txn, const nogdb::RecordDescriptor &descriptor) {
            return 1;
        };

        auto res = nogdb::Traverse::shortestPath(txn, a, f, costFunction, pathFilter);

        assert(res.first == 3);
        assertSize(res.second, 4);

        // traverse by distance
        auto costFunctionDistance = [](const nogdb::Txn &txn, const nogdb::RecordDescriptor &descriptor) -> int {
            const nogdb::Record &record = nogdb::DB::getRecord(txn, descriptor);
            return record.getIntU("distance");
        };

        auto res2 = nogdb::Traverse::shortestPath(txn, a, f, costFunctionDistance, pathFilter);

        assert(res2.first == 280);
        assertSize(res2.second, 4);

        auto costFunctionDistanceOffset = [](const nogdb::Txn &txn, const nogdb::RecordDescriptor &descriptor) {
            const nogdb::Record &record = nogdb::DB::getRecord(txn, descriptor);
            return record.getIntU("distance") + 20;
        };

        auto res3 = nogdb::Traverse::shortestPath(txn, a, c, costFunctionDistanceOffset, nogdb::PathFilter());
        assert(res3.first == 170);
        assertSize(res3.second, 3);

        auto res4 = nogdb::Traverse::shortestPath(txn, a, z, costFunction, nogdb::PathFilter());
        assertSize(res4.second, 0);

        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

}
*/