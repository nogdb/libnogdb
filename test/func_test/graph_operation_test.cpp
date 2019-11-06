/*
 *  test_graph_operations.cpp - A sub-test for testing all graph operations
 *
 *  Copyright (C) 2019, NogDB <https://nogdb.org>
 *  <nogdb at throughwave dot co dot th>
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

#include <list>
#include <set>
#include <vector>

#include "func_test.h"
#include "setup_cleanup.h"

void test_bfs_traverse_in()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, a, b, c, d, e, f, Z;
    try {
        for (const auto& res : txn.find("folders").get()) {
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

        for (const auto& res : txn.find("files").get()) {
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        for (const auto& res : txn.traverseIn(D).depth(1, 1).whereE(nogdb::GraphFilter {}.only("link")).get()) {
            auto name = res.record.get("name").toText();
            assert(name == "B");
            assert(res.record.getDepth() == 1);
        }
        for (const auto& res : txn.traverseIn(D).depth(0, 2).whereE(nogdb::GraphFilter {}.only("link")).get()) {
            auto name = res.record.get("name").toText();
            assert(name == "D" || name == "B" || name == "A");
            if (name == "D")
                assert(res.record.getDepth() == 0);
            else if (name == "B")
                assert(res.record.getDepth() == 1);
            else if (name == "A")
                assert(res.record.getDepth() == 2);
        }
        for (const auto& res : txn.traverseIn(D).depth(1, 3).whereE(nogdb::GraphFilter {}.only("link")).get()) {
            auto name = res.record.get("name").toText();
            assert(name == "B" || name == "A");
            if (name == "B")
                assert(res.record.getDepth() == 1);
            else if (name == "A")
                assert(res.record.getDepth() == 2);
        }
        for (const auto& res : txn.traverseIn(D).depth(0, 0).whereE(nogdb::GraphFilter {}.only("link")).get()) {
            auto name = res.record.get("name").toText();
            assert(name == "D");
            assert(res.record.getDepth() == 0);
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.traverseIn(H).depth(1, 10).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 3);
        assert(res[0].record.get("name").toText() == "F");
        assert(res[0].record.getDepth() == 1);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 2);
        assert(res[2].record.get("name").toText() == "A");
        assert(res[2].record.getDepth() == 3);

        res = txn.traverseIn(f).depth(1, 4).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 4);
        assert(res[0].record.get("name").toText() == "G");
        assert(res[0].record.getDepth() == 1);
        assert(res[1].record.get("name").toText() == "E");
        assert(res[1].record.getDepth() == 2);
        assert(res[2].record.get("name").toText() == "B");
        assert(res[2].record.getDepth() == 3);
        assert(res[3].record.get("name").toText() == "A");
        assert(res[3].record.getDepth() == 4);

        res = txn.traverseIn(f).depth(0, 4).get();
        ASSERT_SIZE(res, 6);

        res = txn.traverseIn(f).depth(0, 100).get();
        ASSERT_SIZE(res, 6);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto classNames = std::set<std::string> { "link", "symbolic" };
        auto res = txn.traverseIn(b)
                       .depth(0, 1)
                       .whereE(nogdb::GraphFilter {}.only(classNames.cbegin(), classNames.cend()))
                       .get();
        ASSERT_SIZE(res, 2);
        res = txn.traverseIn(b).depth(1, 2).get();
        ASSERT_SIZE(res, 2);
        res = txn.traverseIn(e).depth(1, 1).get();
        ASSERT_SIZE(res, 2);
        res = txn.traverseIn(e).depth(0, 2).get();
        ASSERT_SIZE(res, 6);
        res = txn.traverseIn(e).depth(0, 3).get();
        ASSERT_SIZE(res, 8);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.traverseIn(Z).depth(0, 1).get();
        ASSERT_SIZE(res, 1);
        res = txn.traverseIn(Z).depth(0, 100).get();
        ASSERT_SIZE(res, 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_bfs_traverse_out()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, a, b, c, d, e, f, Z;
    try {
        for (const auto& res : txn.find("folders").get()) {
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

        for (const auto& res : txn.find("files").get()) {
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.traverseOut(C).depth(1, 1).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 2);
        for (const auto& r : res) {
            auto name = r.record.get("name").toText();
            assert(name == "c" || name == "F");
        }
        res = txn.traverseOut(C).depth(0, 2).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 6);
        res = txn.traverseOut(C).depth(0, 3).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 6);
        res = txn.traverseOut(C).depth(0, 0).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.traverseOut(A).depth(0, 0).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 1);
        res = txn.traverseOut(A).depth(1, 1).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 3);
        res = txn.traverseOut(A).depth(1, 2).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 8);
        res = txn.traverseOut(A).depth(1, 3).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 12);
        res = txn.traverseOut(A).depth(1, 4).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 13);
        res = txn.traverseOut(A).depth(1, 100).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 13);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto classNames = std::vector<std::string> { "link", "symbolic" };
        auto res = txn.traverseOut(B)
                       .depth(1, 1)
                       .whereE(nogdb::GraphFilter {}.only(classNames.cbegin(), classNames.cend()))
                       .get();
        ASSERT_SIZE(res, 3);
        res = txn.traverseOut(C).depth(0, 1).get();
        ASSERT_SIZE(res, 4);

        res = txn.traverseOut(a).depth(0, 0).get();
        ASSERT_SIZE(res, 1);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.traverseOut(Z).depth(0, 1).get();
        ASSERT_SIZE(res, 1);
        res = txn.traverseOut(Z).depth(0, 100).get();
        ASSERT_SIZE(res, 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_bfs_traverse_all()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto& res : txn.find("folders").get()) {
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

        for (const auto& res : txn.find("files").get()) {
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto count = 0;
        for (const auto& res : txn.traverse(F).depth(1, 1).whereE(nogdb::GraphFilter {}.only("link")).get()) {
            auto name = res.record.get("name").toText();
            assert(name == "d" || name == "C" || name == "H" || name == "e");
            assert(res.record.getDepth() == 1);
            count++;
        }
        assert(count == 4);

        count = 0;
        for (const auto& res : txn.traverse(F).depth(0, 2).whereE(nogdb::GraphFilter {}.only("link")).get()) {
            auto name = res.record.get("name").toText();
            assert(
                name == "F" || name == "d" || name == "C" || name == "H" || name == "e" || name == "A" || name == "c");
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
        for (const auto& res : txn.traverse(F).depth(1, 3).whereE(nogdb::GraphFilter {}.only("link")).get()) {
            auto name = res.record.get("name").toText();
            assert(name == "d" || name == "C" || name == "H" || name == "e" || name == "A" || name == "c" || name == "a"
                || name == "B");
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

        auto res = txn.traverse(F).depth(0, 0).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 1);
        res = txn.traverse(F).depth(0, 100).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 14);
        res = txn.traverse(F).depth(2, 1).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 0);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto count = 0;
        for (const auto& res : txn.traverse(H).depth(1, 1).whereE(nogdb::GraphFilter {}.only("symbolic")).get()) {
            auto name = res.record.get("name").toText();
            assert(name == "C");
            count++;
        }
        assert(count == 1);

        count = 0;
        for (const auto& res : txn.traverse(H).depth(2, 2).whereE(nogdb::GraphFilter {}.only("symbolic")).get()) {
            auto name = res.record.get("name").toText();
            assert(name == "e");
            count++;
        }
        assert(count == 1);

        auto res = txn.traverse(H).depth(1, 3).whereE(nogdb::GraphFilter {}.only("symbolic")).get();
        ASSERT_SIZE(res, 2);

        res = txn.traverse(H).depth(0, 0).whereE(nogdb::GraphFilter {}.only("symbolic")).get();
        ASSERT_SIZE(res, 1);

        res = txn.traverse(H).depth(0, 100).whereE(nogdb::GraphFilter {}.only("symbolic")).get();
        ASSERT_SIZE(res, 3);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto count = 0;
        auto classNames = std::list<std::string> { "link", "symbolic" };
        for (const auto& res : txn.traverse(A)
                                   .depth(1, 1)
                                   .whereE(nogdb::GraphFilter {}.only(classNames.cbegin(), classNames.cend()))
                                   .get()) {
            auto name = res.record.get("name").toText();
            assert(name == "B" || name == "a" || name == "C" || name == "D");
            count++;
        }
        assert(count == 4);

        auto res = txn.traverse(A).depth(1, 2).get();
        ASSERT_SIZE(res, 11);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.traverse(Z).depth(0, 1).get();
        ASSERT_SIZE(res, 1);
        res = txn.traverse(Z).depth(0, 100).get();
        ASSERT_SIZE(res, 1);
        res = txn.traverse(Z).depth(0, 0).get();
        ASSERT_SIZE(res, 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_invalid_bfs_traverse_in()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto& res : txn.find("folders").get()) {
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

        for (const auto& res : txn.find("files").get()) {
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseIn(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("ling")).get();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseIn(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("link", "symbol")).get();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseIn(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("folders")).get();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseIn(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("link", "folders")).get();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = txn.traverseIn(tmp).depth(0, 0).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = txn.traverseIn(tmp).depth(0, 0).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_invalid_bfs_traverse_out()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto& res : txn.find("folders").get()) {
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

        for (const auto& res : txn.find("files").get()) {
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseOut(A).depth(0, 0).whereE(nogdb::GraphFilter {}.only("ling")).get();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseOut(A).depth(0, 0).whereE(nogdb::GraphFilter {}.only("link", "symbol")).get();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseOut(A).depth(0, 0).whereE(nogdb::GraphFilter {}.only("folders")).get();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseOut(A).depth(0, 0).whereE(nogdb::GraphFilter {}.only("link", "folders")).get();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = txn.traverseOut(tmp).depth(0, 0).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = txn.traverseOut(tmp).depth(0, 0).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_invalid_bfs_traverse_all()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto& res : txn.find("folders").get()) {
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

        for (const auto& res : txn.find("files").get()) {
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverse(A).depth(0, 0).whereE(nogdb::GraphFilter {}.only("ling")).get();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverse(A).depth(0, 0).whereE(nogdb::GraphFilter {}.only("link", "symbol")).get();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverse(A).depth(0, 0).whereE(nogdb::GraphFilter {}.only("folders")).get();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverse(A).depth(0, 0).whereE(nogdb::GraphFilter {}.only("link", "folders")).get();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = txn.traverse(tmp).depth(0, 0).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = txn.traverse(tmp).depth(0, 0).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_shortest_path()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto& res : txn.find("folders").get()) {
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

        for (const auto& res : txn.find("files").get()) {
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.shortestPath(A, f).get();
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "B");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "D");
        assert(res[2].record.getDepth() == 2);
        assert(res[3].record.get("name").toText() == "f");
        assert(res[3].record.getDepth() == 3);

        res = txn.shortestPath(A, e).get();
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "e");
        assert(res[2].record.getDepth() == 2);

        res = txn.shortestPath(D, f).get();
        assert(res[0].record.get("name").toText() == "D");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "f");
        assert(res[1].record.getDepth() == 1);

        res = txn.shortestPath(B, A).get();
        assert(res[0].record.get("name").toText() == "B");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "D");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "A");
        assert(res[2].record.getDepth() == 2);

        res = txn.shortestPath(A, e).whereE(nogdb::GraphFilter {}.only("link", "symbolic")).get();
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "e");
        assert(res[2].record.getDepth() == 2);

        res = txn.shortestPath(D, f).whereE(nogdb::GraphFilter {}.only("link", "symbolic")).get();
        assert(res[0].record.get("name").toText() == "D");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "f");
        assert(res[1].record.getDepth() == 1);

        res = txn.shortestPath(B, A).whereE(nogdb::GraphFilter {}.only("link", "symbolic")).get();
        assert(res[0].record.get("name").toText() == "B");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "D");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "A");
        assert(res[2].record.getDepth() == 2);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.shortestPath(a, a).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "a");
        assert(res[0].record.getDepth() == 0);

        res = txn.shortestPath(f, f).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "f");
        assert(res[0].record.getDepth() == 0);

        res = txn.shortestPath(B, B).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "B");
        assert(res[0].record.getDepth() == 0);

        res = txn.shortestPath(A, Z).get();
        assert(res.empty());

        res = txn.shortestPath(Z, G).get();
        assert(res.empty());

        res = txn.shortestPath(a, F).get();
        assert(res.empty());

        res = txn.shortestPath(d, A).get();
        assert(res.empty());

        res = txn.shortestPath(A, b).get();
        ASSERT_SIZE(res, 3);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "B");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "b");
        assert(res[2].record.getDepth() == 2);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.shortestPath(C, e).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 3);

        res = txn.shortestPath(B, d).get();
        ASSERT_SIZE(res, 4);
        res = txn.shortestPath(B, d).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 0);

        res = txn.shortestPath(H, C).whereE(nogdb::GraphFilter {}.only("link")).get();
        ASSERT_SIZE(res, 0);
        res = txn.shortestPath(H, C).whereE(nogdb::GraphFilter {}.only("symbolic")).get();
        ASSERT_SIZE(res, 2);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_invalid_shortest_path()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto& res : txn.find("folders").get()) {
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

        for (const auto& res : txn.find("files").get()) {
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.second = 999U;
        auto res = txn.shortestPath(tmp, B).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_SRC, "NOGDB_GRAPH_NOEXST_SRC");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = B;
        tmp.rid.second = 999U;
        auto res = txn.shortestPath(A, tmp).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_DST, "NOGDB_GRAPH_NOEXST_DST");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = txn.shortestPath(tmp, D).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto links = txn.find("link").get();
        auto tmp = links[0].descriptor;
        auto res = txn.shortestPath(A, tmp).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto links = txn.find("link").get();
        auto tmp = links[0].descriptor;
        auto res = txn.shortestPath(tmp, f).get();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }
}

void test_bfs_traverse_with_condition()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor a, b, c, d, e, f, z;
    try {
        for (const auto& res : txn.find("country").get()) {
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
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto edgeCondition = [](const nogdb::Record& record) { return (record.get("distance").toIntU() < 100U); };
        auto edgeFilter = nogdb::GraphFilter(edgeCondition);
        auto res = txn.traverseOut(a).depth(0, 1).whereE(edgeFilter).get();
        ASSERT_SIZE(res, 2);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "B");
        assert(res[1].record.getDepth() == 1);

        res = txn.traverseIn(a).depth(0, 1).whereE(edgeFilter).get();
        ASSERT_SIZE(res, 2);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "Z");
        assert(res[1].record.getDepth() == 1);

        auto vertexCondition
            = [](const nogdb::Record& record) { return (record.get("population").toBigIntU() > 1000ULL); };
        auto vertexFilter = nogdb::GraphFilter(vertexCondition);
        res = txn.traverseOut(a).depth(0, 1).whereE(edgeFilter).whereV(vertexFilter).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "A");

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto edgeCondition = [](const nogdb::Record& record) { return (record.get("distance").toIntU() > 100U); };
        auto edgeFilter = nogdb::GraphFilter(edgeCondition);
        auto res = txn.traverse(a).depth(1, 3).whereE(edgeFilter).get();
        ASSERT_SIZE(res, 3);
        assert(res[0].record.get("name").toText() == "D");
        assert(res[0].record.getDepth() == 1);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "F");
        assert(res[2].record.getDepth() == 2);

        res = txn.traverse(a).depth(2, 4).whereE(edgeFilter).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "F");
        assert(res[0].record.getDepth() == 2);

        auto vertexCondition
            = [](const nogdb::Record& record) { return (record.get("population").toBigIntU() < 4000ULL); };
        auto vertexFilter = nogdb::GraphFilter(vertexCondition);
        res = txn.traverse(a).depth(0, 4).whereV(vertexFilter).get();
        ASSERT_SIZE(res, 6);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "Z");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "B");
        assert(res[2].record.getDepth() == 1);
        assert(res[3].record.get("name").toText() == "C");
        assert(res[3].record.getDepth() == 1);
        assert(res[4].record.get("name").toText() == "E");
        assert(res[4].record.getDepth() == 2);
        assert(res[5].record.get("name").toText() == "F");
        assert(res[5].record.getDepth() == 2);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_shortest_path_with_condition()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor a, b, c, d, e, f, z;
    try {
        for (const auto& res : txn.find("country").get()) {
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
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.shortestPath(a, f)
                       .whereE(nogdb::GraphFilter(
                           [](const nogdb::Record& record) { return (record.get("distance").toIntU() <= 120U); }))
                       .whereV(nogdb::GraphFilter([](const nogdb::Record& record) {
                           return (record.get("population").toBigIntU() >= 1000ULL);
                       }))
                       .get();
        ASSERT_SIZE(res, 5);
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

        res = txn.shortestPath(a, f)
                  .whereE(nogdb::GraphFilter(
                      [](const nogdb::Record& record) { return (record.get("distance").toIntU() <= 200U); }))
                  .whereV(nogdb::GraphFilter(
                      [](const nogdb::Record& record) { return (record.get("population").toBigIntU() < 5000ULL); }))
                  .get();
        ASSERT_SIZE(res, 4);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "B");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "C");
        assert(res[2].record.getDepth() == 2);
        assert(res[3].record.get("name").toText() == "F");
        assert(res[3].record.getDepth() == 3);

        res = txn.shortestPath(a, f)
                  .whereE(nogdb::GraphFilter(
                      [](const nogdb::Record& record) { return (record.get("distance").toIntU() <= 200U); }))
                  .get();
        ASSERT_SIZE(res, 4);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "B");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "C");
        assert(res[2].record.getDepth() == 2);
        assert(res[3].record.get("name").toText() == "F");
        assert(res[3].record.getDepth() == 3);

        res = txn.shortestPath(a, f)
                  .whereE(nogdb::GraphFilter([](const nogdb::Record& record) {
                      return record.get("distance").toIntU() >= 100U && record.get("distance").toIntU() != 150U;
                  }))
                  .get();
        ASSERT_SIZE(res, 4);
        assert(res[0].record.get("name").toText() == "A");
        assert(res[0].record.getDepth() == 0);
        assert(res[1].record.get("name").toText() == "C");
        assert(res[1].record.getDepth() == 1);
        assert(res[2].record.get("name").toText() == "D");
        assert(res[2].record.getDepth() == 2);
        assert(res[3].record.get("name").toText() == "F");
        assert(res[3].record.getDepth() == 3);

        res = txn.shortestPath(a, f)
                  .whereE(nogdb::GraphFilter(
                      [](const nogdb::Record& record) { return record.get("distance").toIntU() >= 1000U; }))
                  .get();
        assert(res.empty());
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_bfs_traverse_in_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, a, b, c, d, e, f, Z;
    try {
        auto rsCursor = txn.find("folders").getCursor();
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

        rsCursor = txn.find("files").getCursor();
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = txn.traverseIn(D).depth(1, 1).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        rsCursor.next();
        auto name = rsCursor->record.get("name").toText();
        assert(name == "B");
        auto depth = rsCursor->record.getDepth();
        assert(depth == 1);

        rsCursor = txn.traverseIn(D).depth(0, 2).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "D");
        assert(rsCursor->record.getDepth() == 0);
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "B");
        assert(rsCursor->record.getDepth() == 1);
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "A");
        assert(rsCursor->record.getDepth() == 2);

        rsCursor = txn.traverseIn(D).depth(1, 3).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "B");
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "A");

        rsCursor = txn.traverseIn(D).depth(0, 0).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        rsCursor.next();
        name = rsCursor->record.get("name").toText();
        assert(name == "D");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = txn.traverseIn(H).depth(1, 10).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.size() == 3);
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "F");
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "C");
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "A");

        rsCursor = txn.traverseIn(f).depth(1, 4).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.size() == 4);
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "G");
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "E");
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "B");
        rsCursor.next();
        assert(rsCursor->record.get("name").toText() == "A");

        rsCursor = txn.traverseIn(f).depth(0, 4).getCursor();
        assert(rsCursor.size() == 6);

        rsCursor = txn.traverseIn(f).depth(0, 100).getCursor();
        assert(rsCursor.size() == 6);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto classNames = std::set<std::string> { "link", "symbolic" };
        auto rsCursor = txn.traverseIn(b)
                            .depth(0, 1)
                            .whereE(nogdb::GraphFilter {}.only(classNames.cbegin(), classNames.cend()))
                            .getCursor();
        assert(rsCursor.size() == 2);
        rsCursor = txn.traverseIn(b).depth(1, 2).getCursor();
        assert(rsCursor.size() == 2);
        rsCursor = txn.traverseIn(e).depth(1, 1).getCursor();
        assert(rsCursor.size() == 2);
        rsCursor = txn.traverseIn(e).depth(0, 2).getCursor();
        assert(rsCursor.size() == 6);
        rsCursor = txn.traverseIn(e).depth(0, 3).getCursor();
        assert(rsCursor.size() == 8);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = txn.traverseIn(Z).depth(0, 1).getCursor();
        assert(rsCursor.size() == 1);
        rsCursor = txn.traverseIn(Z).depth(0, 100).getCursor();
        assert(rsCursor.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_bfs_traverse_out_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, a, b, c, d, e, f, Z;
    try {
        auto rsCursor = txn.find("folders").getCursor();
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

        rsCursor = txn.find("files").getCursor();
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = txn.traverseOut(C).depth(1, 1).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.size() == 2);
        while (rsCursor.next()) {
            auto name = rsCursor->record.get("name").toText();
            assert(name == "c" || name == "F");
        }
        rsCursor = txn.traverseOut(C).depth(0, 2).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.size() == 6);
        rsCursor = txn.traverseOut(C).depth(0, 3).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.size() == 6);
        rsCursor = txn.traverseOut(C).depth(0, 0).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = txn.traverseOut(A).depth(0, 0).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.size() == 1);
        rsCursor = txn.traverseOut(A).depth(1, 1).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.size() == 3);
        rsCursor = txn.traverseOut(A).depth(1, 2).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.size() == 8);
        rsCursor = txn.traverseOut(A).depth(1, 3).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.size() == 12);
        rsCursor = txn.traverseOut(A).depth(1, 4).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.size() == 13);
        rsCursor = txn.traverseOut(A).depth(1, 100).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.size() == 13);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto classNames = std::vector<std::string> { "link", "symbolic" };
        auto rsCursor = txn.traverseOut(B)
                            .depth(1, 1)
                            .whereE(nogdb::GraphFilter {}.only(classNames.cbegin(), classNames.cend()))
                            .getCursor();
        assert(rsCursor.size() == 3);
        rsCursor = txn.traverseOut(C).depth(0, 1).getCursor();
        assert(rsCursor.size() == 4);
        rsCursor = txn.traverseOut(a).depth(0, 0).getCursor();
        assert(rsCursor.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = txn.traverseOut(Z).depth(0, 1).getCursor();
        assert(rsCursor.size() == 1);
        rsCursor = txn.traverseOut(Z).depth(0, 100).getCursor();
        assert(rsCursor.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_bfs_traverse_all_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto rsCursor = txn.find("folders").getCursor();
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

        rsCursor = txn.find("files").getCursor();
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
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = txn.traverse(F).depth(1, 1).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        while (rsCursor.next()) {
            auto name = rsCursor->record.get("name").toText();
            assert(name == "d" || name == "C" || name == "H" || name == "e");
        }
        assert(rsCursor.size() == 4);

        rsCursor = txn.traverse(F).depth(0, 2).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        while (rsCursor.next()) {
            auto name = rsCursor->record.get("name").toText();
            assert(
                name == "F" || name == "d" || name == "C" || name == "H" || name == "e" || name == "A" || name == "c");
        }
        assert(rsCursor.size() == 7);

        rsCursor = txn.traverse(F).depth(1, 3).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        while (rsCursor.next()) {
            auto name = rsCursor->record.get("name").toText();
            assert(name == "d" || name == "C" || name == "H" || name == "e" || name == "A" || name == "c" || name == "a"
                || name == "B");
        }
        assert(rsCursor.count() == 8);

        rsCursor = txn.traverse(F).depth(0, 0).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.size() == 1);
        rsCursor = txn.traverse(F).depth(0, 100).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.size() == 14);
        rsCursor = txn.traverse(F).depth(2, 1).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(rsCursor.empty());
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = txn.traverse(H).depth(1, 1).whereE(nogdb::GraphFilter {}.only("symbolic")).getCursor();
        assert(rsCursor.size() == 1);
        rsCursor.next();
        auto name = rsCursor->record.get("name").toText();
        assert(name == "C");

        rsCursor = txn.traverse(H).depth(2, 2).whereE(nogdb::GraphFilter {}.only("symbolic")).getCursor();
        rsCursor.next();
        name = rsCursor->record.get("name").toText();
        assert(name == "e");
        assert(rsCursor.count() == 1);

        rsCursor = txn.traverse(H).depth(1, 3).whereE(nogdb::GraphFilter {}.only("symbolic")).getCursor();
        assert(rsCursor.size() == 2);
        rsCursor = txn.traverse(H).depth(0, 0).whereE(nogdb::GraphFilter {}.only("symbolic")).getCursor();
        assert(rsCursor.size() == 1);
        rsCursor = txn.traverse(H).depth(0, 100).whereE(nogdb::GraphFilter {}.only("symbolic")).getCursor();
        assert(rsCursor.size() == 3);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto classNames = std::list<std::string> { "link", "symbolic" };
        auto rsCursor = txn.traverse(A)
                            .depth(1, 1)
                            .whereE(nogdb::GraphFilter {}.only(classNames.cbegin(), classNames.cend()))
                            .getCursor();
        while (rsCursor.next()) {
            auto name = rsCursor->record.get("name").toText();
            assert(name == "B" || name == "a" || name == "C" || name == "D");
        }
        assert(rsCursor.count() == 4);

        rsCursor = txn.traverse(A).depth(1, 2).getCursor();
        assert(rsCursor.size() == 11);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto rsCursor = txn.traverse(Z).depth(0, 1).getCursor();
        assert(rsCursor.size() == 1);
        rsCursor = txn.traverse(Z).depth(0, 100).getCursor();
        assert(rsCursor.size() == 1);
        rsCursor = txn.traverse(Z).depth(0, 0).getCursor();
        assert(rsCursor.size() == 1);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_invalid_bfs_traverse_in_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto res = txn.find("folders").getCursor();
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

        res = txn.find("files").getCursor();
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseIn(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("ling")).getCursor();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseIn(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("link", "symbol")).getCursor();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseIn(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("folders")).getCursor();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseIn(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("link", "folders")).getCursor();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = txn.traverseIn(tmp).depth(0, 0).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = txn.traverseIn(tmp).depth(0, 0).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_invalid_bfs_traverse_out_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto res = txn.find("folders").getCursor();
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

        res = txn.find("files").getCursor();
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseOut(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("ling")).getCursor();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseOut(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("link", "symbol")).getCursor();
        ASSERT_SIZE(res, 9);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseOut(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("folders")).getCursor();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverseOut(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("link", "folders")).getCursor();
        ASSERT_SIZE(res, 9);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = txn.traverseOut(tmp).depth(0, 0).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = txn.traverseOut(tmp).depth(0, 0).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_invalid_bfs_traverse_all_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto res = txn.find("folders").getCursor();
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

        res = txn.find("files").getCursor();
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverse(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("ling")).getCursor();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverse(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("link", "symbol")).getCursor();
        ASSERT_SIZE(res, 9);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverse(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("folders")).getCursor();
        ASSERT_SIZE(res, 1);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.traverse(A).depth(0, 2).whereE(nogdb::GraphFilter {}.only("link", "folders")).getCursor();
        ASSERT_SIZE(res, 9);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = txn.traverse(tmp).depth(0, 0).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.second = 9999U;
        auto res = txn.traverse(tmp).depth(0, 0).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_VERTEX, "NOGDB_GRAPH_NOEXST_VERTEX");
    }
}

void test_shortest_path_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto res = txn.find("folders").getCursor();
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

        res = txn.find("files").getCursor();
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.shortestPath(A, f).getCursor();
        cursorContains(res, std::set<std::string> { "A", "B", "D", "f" }, "name");
        ASSERT_SIZE(res, 4);
        res.first();
        assert(res->record.getDepth() == 0);
        res.next();
        assert(res->record.getDepth() == 1);
        res.next();
        assert(res->record.getDepth() == 2);
        res.next();
        assert(res->record.getDepth() == 3);

        res = txn.shortestPath(A, e).getCursor();
        cursorContains(res, std::set<std::string> { "A", "C", "e" }, "name");
        ASSERT_SIZE(res, 3);

        res = txn.shortestPath(D, f).getCursor();
        cursorContains(res, std::set<std::string> { "D", "f" }, "name");
        ASSERT_SIZE(res, 2);

        res = txn.shortestPath(B, A).getCursor();
        cursorContains(res, std::set<std::string> { "B", "D", "A" }, "name");
        ASSERT_SIZE(res, 3);

        res = txn.shortestPath(A, e).whereE(nogdb::GraphFilter {}.only("link", "symbolic")).getCursor();
        cursorContains(res, std::set<std::string> { "A", "C", "e" }, "name");
        ASSERT_SIZE(res, 3);

        res = txn.shortestPath(D, f).whereE(nogdb::GraphFilter {}.only("link", "symbolic")).getCursor();
        cursorContains(res, std::set<std::string> { "D", "f" }, "name");
        ASSERT_SIZE(res, 2);

        res = txn.shortestPath(B, A).whereE(nogdb::GraphFilter {}.only("link", "symbolic")).getCursor();
        cursorContains(res, std::set<std::string> { "B", "D", "A" }, "name");
        ASSERT_SIZE(res, 3);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.shortestPath(a, a).getCursor();
        ASSERT_SIZE(res, 1);
        res.next();
        assert(res->record.get("name").toText() == "a");
        assert(res->record.getDepth() == 0);

        res = txn.shortestPath(f, f).getCursor();
        ASSERT_SIZE(res, 1);
        res.next();
        assert(res->record.get("name").toText() == "f");

        res = txn.shortestPath(B, B).getCursor();
        ASSERT_SIZE(res, 1);
        res.next();
        assert(res->record.get("name").toText() == "B");

        res = txn.shortestPath(A, Z).getCursor();
        assert(res.empty());

        res = txn.shortestPath(Z, G).getCursor();
        assert(res.empty());

        res = txn.shortestPath(a, F).getCursor();
        assert(res.empty());

        res = txn.shortestPath(d, A).getCursor();
        assert(res.empty());

        res = txn.shortestPath(A, b).getCursor();
        cursorContains(res, std::set<std::string> { "A", "B", "b" }, "name");
        ASSERT_SIZE(res, 3);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.shortestPath(C, e).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        ASSERT_SIZE(res, 3);

        res = txn.shortestPath(B, d).getCursor();
        ASSERT_SIZE(res, 4);
        res = txn.shortestPath(B, d).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(res.count() == 0);

        res = txn.shortestPath(H, C).whereE(nogdb::GraphFilter {}.only("link")).getCursor();
        assert(res.count() == 0);
        res = txn.shortestPath(H, C).whereE(nogdb::GraphFilter {}.only("symbolic")).getCursor();
        ASSERT_SIZE(res, 2);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_invalid_shortest_path_cursor()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        auto res = txn.find("folders").getCursor();
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

        res = txn.find("files").getCursor();
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.second = 999U;
        auto res = txn.shortestPath(tmp, B).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_SRC, "NOGDB_GRAPH_NOEXST_SRC");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = B;
        tmp.rid.second = 999U;
        auto res = txn.shortestPath(A, tmp).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_GRAPH_NOEXST_DST, "NOGDB_GRAPH_NOEXST_DST");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto tmp = A;
        tmp.rid.first = -1;
        auto res = txn.shortestPath(tmp, D).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto links = txn.find("link").get();
        auto tmp = links[0].descriptor;
        auto res = txn.shortestPath(A, tmp).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto links = txn.find("link").get();
        auto tmp = links[0].descriptor;
        auto res = txn.shortestPath(tmp, f).getCursor();
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_MISMATCH_CLASSTYPE, "NOGDB_CTX_MISMATCH_CLASSTYPE");
    }
}

void test_bfs_traverse_cursor_with_condition()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor a, b, c, d, e, f, z;
    try {
        auto res = txn.find("country").getCursor();
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
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto edgeFilter
            = nogdb::GraphFilter([](const nogdb::Record& record) { return (record.get("distance").toIntU() < 100U); });
        auto res = txn.traverseOut(a).depth(0, 1).whereE(edgeFilter).getCursor();
        ASSERT_SIZE(res, 2);
        cursorContains(res, std::set<std::string> { "A", "B" }, "name");

        res = txn.traverseIn(a).depth(0, 1).whereE(edgeFilter).getCursor();
        ASSERT_SIZE(res, 2);
        cursorContains(res, std::set<std::string> { "A", "Z" }, "name");

        auto vertexFilter = nogdb::GraphFilter(
            [](const nogdb::Record& record) { return (record.get("population").toBigIntU() > 1000ULL); });
        res = txn.traverseOut(a).depth(0, 1).whereE(edgeFilter).whereV(vertexFilter).getCursor();
        ASSERT_SIZE(res, 1);
        res.next();
        assert(res->record.get("name").toText() == "A");

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto edgeFilter
            = nogdb::GraphFilter([](const nogdb::Record& record) { return (record.get("distance").toIntU() > 100U); });
        auto res = txn.traverse(a).depth(1, 3).whereE(edgeFilter).getCursor();
        ASSERT_SIZE(res, 3);
        cursorContains(res, std::set<std::string> { "C", "D", "F" }, "name");

        res = txn.traverse(a).depth(2, 4).whereE(edgeFilter).getCursor();
        ASSERT_SIZE(res, 1);
        res.first();
        assert(res->record.get("name").toText() == "F");

        auto vertexFilter = nogdb::GraphFilter(
            [](const nogdb::Record& record) { return (record.get("population").toBigIntU() < 4000ULL); });
        res = txn.traverse(a).depth(0, 4).whereV(vertexFilter).getCursor();
        ASSERT_SIZE(res, 6);
        cursorContains(res, std::set<std::string> { "A", "Z", "B", "C", "E", "F" }, "name");

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_shortest_path_cursor_with_condition()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor a, b, c, d, e, f, z;
    try {
        auto res = txn.find("country").getCursor();
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
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto edgeFilter
            = nogdb::GraphFilter([](const nogdb::Record& record) { return (record.get("distance").toIntU() <= 120U); });
        auto vertexFilter = nogdb::GraphFilter(
            [](const nogdb::Record& record) { return (record.get("population").toBigIntU() >= 1000ULL); });
        auto res = txn.shortestPath(a, f).whereE(edgeFilter).whereV(vertexFilter).getCursor();
        ASSERT_SIZE(res, 5);
        cursorContains(res, std::set<std::string> { "A", "B", "C", "D", "F" }, "name");

        edgeFilter
            = nogdb::GraphFilter([](const nogdb::Record& record) { return (record.get("distance").toIntU() <= 200U); });
        vertexFilter = nogdb::GraphFilter(
            [](const nogdb::Record& record) { return (record.get("population").toBigIntU() < 5000ULL); });
        res = txn.shortestPath(a, f).whereE(edgeFilter).whereV(vertexFilter).getCursor();
        ASSERT_SIZE(res, 4);
        cursorContains(res, std::set<std::string> { "A", "B", "C", "F" }, "name");

        edgeFilter
            = nogdb::GraphFilter([](const nogdb::Record& record) { return (record.get("distance").toIntU() <= 200U); });
        res = txn.shortestPath(a, f).whereE(edgeFilter).getCursor();
        ASSERT_SIZE(res, 4);
        cursorContains(res, std::set<std::string> { "A", "B", "C", "F" }, "name");

        edgeFilter = nogdb::GraphFilter([](const nogdb::Record& record) {
            return record.get("distance").toIntU() >= 100U && record.get("distance").toIntU() != 150U;
        });
        res = txn.shortestPath(a, f).whereE(edgeFilter).getCursor();
        ASSERT_SIZE(res, 4);
        cursorContains(res, std::set<std::string> { "A", "C", "D", "F" }, "name");

        edgeFilter
            = nogdb::GraphFilter([](const nogdb::Record& record) { return record.get("distance").toIntU() >= 1000U; });
        res = txn.shortestPath(a, f).whereE(edgeFilter).getCursor();
        assert(res.empty());
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_bfs_traverse_multi_edges_with_condition() {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        auto th = txn.addVertex("country", nogdb::Record{}.set("name", "Thailand"));
        auto la = txn.addVertex("country", nogdb::Record{}.set("name", "Laos"));
        auto cn = txn.addVertex("country", nogdb::Record{}.set("name", "China"));

        txn.addEdge("path", th, la, nogdb::Record{}.set("distance", 150U));
        txn.addEdge("path", th, la, nogdb::Record{}.set("distance", 120U));
        txn.addEdge("path", th, la, nogdb::Record{}.set("distance", 100U));
        txn.addEdge("path", la, cn, nogdb::Record{}.set("distance", 100U));
        txn.addEdge("path", la, cn, nogdb::Record{}.set("distance", 120U));
        txn.addEdge("path", la, cn, nogdb::Record{}.set("distance", 150U));

        auto traverse = txn.traverse(th).maxDepth(10).whereE(nogdb::GraphFilter(nogdb::Condition("distance").gt(140U)));
        auto res = traverse.get();
        ASSERT_SIZE(res, 3);
        ASSERT_EQ(res[0].descriptor.rid, th.rid);
        ASSERT_EQ(res[0].record.getDepth(), 0U);
        ASSERT_EQ(res[1].descriptor.rid, la.rid);
        ASSERT_EQ(res[1].record.getDepth(), 1U);
        ASSERT_EQ(res[2].descriptor.rid, cn.rid);
        ASSERT_EQ(res[2].record.getDepth(), 2U);

        auto resCursor = traverse.getCursor();
        ASSERT_EQ(resCursor.count(), size_t{3});
        resCursor.first();
        ASSERT_EQ(resCursor->descriptor.rid, th.rid);
        ASSERT_EQ(resCursor->record.getDepth(), 0U);
        resCursor.next();
        ASSERT_EQ(resCursor->descriptor.rid, la.rid);
        ASSERT_EQ(resCursor->record.getDepth(), 1U);
        resCursor.next();
        ASSERT_EQ(resCursor->descriptor.rid, cn.rid);
        ASSERT_EQ(resCursor->record.getDepth(), 2U);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.rollback();
}

void test_bfs_traverse_multi_vertices() {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto& res : txn.find("folders").get()) {
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

        for (const auto& res : txn.find("files").get()) {
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto traverse = txn.traverseOut(A).addSource(E).maxDepth(1);
        auto res = traverse.get();
        ASSERT_SIZE(res, 7);
        for(const auto& vertex: res) {
            if (vertex.descriptor == A) {
                ASSERT_EQ(vertex.record.getDepth(), 0U);
            } else if (vertex.descriptor == B) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == C) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == a) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == E) {
                ASSERT_EQ(vertex.record.getDepth(), 0U);
            } else if (vertex.descriptor == G) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == F) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else {
                ASSERT_FALSE(true);
            }
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto traverse = txn.traverseOut(A).addSource(E).maxDepth(2);
        auto res = traverse.get();
        ASSERT_SIZE(res, 14);
        for(const auto& vertex: res) {
            if (vertex.descriptor == A) {
                ASSERT_EQ(vertex.record.getDepth(), 0U);
            } else if (vertex.descriptor == B) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == C) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == a) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == E) {
                ASSERT_EQ(vertex.record.getDepth(), 0U);
            } else if (vertex.descriptor == G) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == F) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == D) {
                ASSERT_EQ(vertex.record.getDepth(), 2U);
            } else if (vertex.descriptor == b) {
                ASSERT_EQ(vertex.record.getDepth(), 2U);
            } else if (vertex.descriptor == f) {
                ASSERT_EQ(vertex.record.getDepth(), 2U);
            } else if (vertex.descriptor == d) {
                ASSERT_EQ(vertex.record.getDepth(), 2U);
            } else if (vertex.descriptor == H) {
                ASSERT_EQ(vertex.record.getDepth(), 2U);
            } else if (vertex.descriptor == e) {
                ASSERT_EQ(vertex.record.getDepth(), 2U);
            } else if (vertex.descriptor == c) {
                ASSERT_EQ(vertex.record.getDepth(), 2U);
            } else {
                ASSERT_FALSE(true);
            }
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_bfs_traverse_multi_vertices_with_condition() {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    nogdb::RecordDescriptor A, B, C, D, E, F, G, H, Z, a, b, c, d, e, f;
    try {
        for (const auto& res : txn.find("folders").get()) {
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

        for (const auto& res : txn.find("files").get()) {
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

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto traverse = txn.traverseOut(E).addSource(A).maxDepth(1).whereV(nogdb::GraphFilter().only("folders"));
        auto res = traverse.get();
        ASSERT_SIZE(res, 6);
        for(const auto& vertex: res) {
            if (vertex.descriptor == A) {
                ASSERT_EQ(vertex.record.getDepth(), 0U);
            } else if (vertex.descriptor == B) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == C) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == E) {
                ASSERT_EQ(vertex.record.getDepth(), 0U);
            } else if (vertex.descriptor == G) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == F) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else {
                ASSERT_FALSE(true);
            }
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto traverse = txn.traverseOut(E).addSource(A).maxDepth(2).whereV(nogdb::GraphFilter().only("folders"));
        auto res = traverse.get();
        ASSERT_SIZE(res, 8);
        for(const auto& vertex: res) {
            if (vertex.descriptor == A) {
                ASSERT_EQ(vertex.record.getDepth(), 0U);
            } else if (vertex.descriptor == B) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == C) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == E) {
                ASSERT_EQ(vertex.record.getDepth(), 0U);
            } else if (vertex.descriptor == G) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == F) {
                ASSERT_EQ(vertex.record.getDepth(), 1U);
            } else if (vertex.descriptor == D) {
                ASSERT_EQ(vertex.record.getDepth(), 2U);
            } else if (vertex.descriptor == H) {
                ASSERT_EQ(vertex.record.getDepth(), 2U);
            } else {
                ASSERT_FALSE(true);
            }
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}
