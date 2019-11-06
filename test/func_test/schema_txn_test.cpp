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

size_t getSizeOfSubClasses(const nogdb::Transaction& txn, const nogdb::ClassDescriptor& classDesc)
{
    auto size = size_t { 0 };
    for (const auto& cdesc : txn.getClasses()) {
        size += cdesc.base == classDesc.id;
    }
    return size;
}

bool propertyExists(const nogdb::Transaction& txn, const std::string& className, const std::string& propertyName)
{
    try {
        return txn.getProperty(className, propertyName).id != nogdb::PropertyDescriptor {}.id;
    } catch (const nogdb::Error& error) {
        return false;
    }
}

bool indexExists(const nogdb::Transaction& txn, const std::string& className, const std::string& propertyName)
{
    try {
        return txn.getIndex(className, propertyName).id != nogdb::IndexDescriptor {}.id;
    } catch (const nogdb::Error& error) {
        return false;
    }
}

void test_schema_txn_commit_simple()
{
    try {
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto cdesc = txnRw1.addClass("test_0", nogdb::ClassType::VERTEX);
        txnRw1.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        txnRo1.rollback();
        txnRo2.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_create_class_commit()
{
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto cdesc = txnRw1.addClass("test_1", nogdb::ClassType::VERTEX);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.getClass("test_1");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(res.id == cdesc.id);
        assert(res.name == cdesc.name);

        try {
            txnRo1.getClass("test_1");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo2.getClass("test_1");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo3.getClass("test_1");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRw2.getClass("test_1");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(res.id == cdesc.id);
        assert(res.name == cdesc.name);
        res = txnRo4.getClass("test_1");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(res.id == cdesc.id);
        assert(res.name == cdesc.name);

        try {
            txnRo1.getClass("test_1");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo2.getClass("test_1");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo3.getClass("test_1");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_create_class_rollback()
{
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto cdesc = txnRw1.addClass("test_2", nogdb::ClassType::VERTEX);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.getClass("test_2");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(res.id == cdesc.id);
        assert(res.name == cdesc.name);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        try {
            txnRw2.getClass("test_2");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo4.getClass("test_2");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo1.getClass("test_2");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo2.getClass("test_2");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo3.getClass("test_2");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_class_commit()
{
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropClass("test_1");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        try {
            txnRw1.getClass("test_1");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

        auto res = txnRo1.getClass("test_1");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        res = txnRo2.getClass("test_1");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        res = txnRo3.getClass("test_1");
        assert(res.id != nogdb::ClassDescriptor {}.id);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        try {
            txnRo4.getClass("test_1");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRw2.getClass("test_1");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRo1.getClass("test_1");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        res = txnRo2.getClass("test_1");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        res = txnRo3.getClass("test_1");
        assert(res.id != nogdb::ClassDescriptor {}.id);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_class_rollback()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_2", nogdb::ClassType::VERTEX);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropClass("test_2");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        try {
            txnRw1.getClass("test_2");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto res = txnRo4.getClass("test_2");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        res = txnRw2.getClass("test_2");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        res = txnRo1.getClass("test_2");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        res = txnRo2.getClass("test_2");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        res = txnRo3.getClass("test_2");
        assert(res.id != nogdb::ClassDescriptor {}.id);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_alter_class_commit()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_3", nogdb::ClassType::EDGE);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.renameClass("test_3", "test_4");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        try {
            txnRw1.getClass("test_3");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        auto res = txnRw1.getClass("test_4");
        assert(res.id != nogdb::ClassDescriptor {}.id);

        try {
            txnRo1.getClass("test_4");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRo1.getClass("test_3");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        try {
            txnRo2.getClass("test_4");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRo2.getClass("test_3");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        try {
            txnRo3.getClass("test_4");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRo3.getClass("test_3");
        assert(res.id != nogdb::ClassDescriptor {}.id);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        try {
            txnRo4.getClass("test_3");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRo4.getClass("test_4");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        try {
            txnRw2.getClass("test_3");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRw2.getClass("test_4");
        assert(res.id != nogdb::ClassDescriptor {}.id);

        try {
            txnRo1.getClass("test_4");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRo1.getClass("test_3");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        try {
            txnRo2.getClass("test_4");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRo2.getClass("test_3");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        try {
            txnRo3.getClass("test_4");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRo3.getClass("test_3");
        assert(res.id != nogdb::ClassDescriptor {}.id);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_alter_class_rollback()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_5", nogdb::ClassType::EDGE);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.renameClass("test_5", "test_6");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        try {
            txnRw1.getClass("test_5");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        auto res = txnRw1.getClass("test_6");
        assert(res.id != nogdb::ClassDescriptor {}.id);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        try {
            txnRo4.getClass("test_6");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRo4.getClass("test_5");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        try {
            txnRw2.getClass("test_6");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRw2.getClass("test_5");
        assert(res.id != nogdb::ClassDescriptor {}.id);

        try {
            txnRo1.getClass("test_6");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRo1.getClass("test_5");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        try {
            txnRo2.getClass("test_6");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRo2.getClass("test_5");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        try {
            txnRo3.getClass("test_6");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRo3.getClass("test_5");
        assert(res.id != nogdb::ClassDescriptor {}.id);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_create_class_extend_commit()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_10", nogdb::ClassType::VERTEX);
        txnRw.addProperty("test_10", "prop0", nogdb::PropertyType::INTEGER);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addSubClassOf("test_10", "test_11");
        txnRw1.addSubClassOf("test_10", "test_12");
        txnRw1.addSubClassOf("test_11", "test_13");
        txnRw1.addProperty("test_11", "prop1", nogdb::PropertyType::INTEGER);
        txnRw1.addProperty("test_12", "prop2", nogdb::PropertyType::INTEGER);
        txnRw1.addProperty("test_13", "prop3", nogdb::PropertyType::INTEGER);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res10 = txnRw1.getClass("test_10");
        assert(res10.id != nogdb::ClassDescriptor {}.id);
        assert(getSizeOfSubClasses(txnRw1, res10) == 2);
        auto res11 = txnRw1.getClass("test_11");
        assert(res11.id != nogdb::ClassDescriptor {}.id);
        assert(res11.base == res10.id);
        assert(getSizeOfSubClasses(txnRw1, res11) == 1);
        auto res12 = txnRw1.getClass("test_12");
        assert(res12.id != nogdb::ClassDescriptor {}.id);
        assert(res12.base == res10.id);
        assert(getSizeOfSubClasses(txnRw1, res12) == 0);
        auto res13 = txnRw1.getClass("test_13");
        assert(res13.id != nogdb::ClassDescriptor {}.id);
        assert(res13.base == res11.id);
        assert(getSizeOfSubClasses(txnRw1, res13) == 0);

        txnRw1.addVertex("test_10", nogdb::Record {}.set("prop0", 1));
        txnRw1.addVertex("test_11", nogdb::Record {}.set("prop0", 1).set("prop1", 1));
        txnRw1.addVertex("test_12", nogdb::Record {}.set("prop0", 1).set("prop2", 1));
        txnRw1.addVertex("test_13", nogdb::Record {}.set("prop0", 1).set("prop3", 1));

        auto res = txnRo1.getClass("test_10");
        assert(getSizeOfSubClasses(txnRo1, res) == 0);
        try {
            txnRo1.getClass("test_11");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo1.getClass("test_12");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo1.getClass("test_13");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

        res = txnRo2.getClass("test_10");
        assert(getSizeOfSubClasses(txnRo2, res) == 0);
        try {
            txnRo2.getClass("test_11");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo2.getClass("test_12");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo2.getClass("test_13");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

        res = txnRo3.getClass("test_10");
        assert(getSizeOfSubClasses(txnRo3, res) == 0);
        try {
            txnRo3.getClass("test_11");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo3.getClass("test_12");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo3.getClass("test_13");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res10 = txnRw2.getClass("test_10");
        assert(res10.id != nogdb::ClassDescriptor {}.id);
        assert(getSizeOfSubClasses(txnRw2, res10) == 2);
        res11 = txnRw2.getClass("test_11");
        assert(res11.id != nogdb::ClassDescriptor {}.id);
        assert(res11.base == res10.id);
        assert(getSizeOfSubClasses(txnRw2, res11) == 1);
        res12 = txnRw2.getClass("test_12");
        assert(res12.id != nogdb::ClassDescriptor {}.id);
        assert(res12.base == res10.id);
        assert(getSizeOfSubClasses(txnRw2, res12) == 0);
        res13 = txnRw2.getClass("test_13");
        assert(res13.id != nogdb::ClassDescriptor {}.id);
        assert(res13.base == res11.id);
        assert(getSizeOfSubClasses(txnRw2, res13) == 0);

        txnRw2.addVertex("test_10", nogdb::Record {}.set("prop0", 1));
        txnRw2.addVertex("test_11", nogdb::Record {}.set("prop0", 1).set("prop1", 1));
        txnRw2.addVertex("test_12", nogdb::Record {}.set("prop0", 1).set("prop2", 1));
        txnRw2.addVertex("test_13", nogdb::Record {}.set("prop0", 1).set("prop3", 1));

        res10 = txnRo4.getClass("test_10");
        assert(res10.id != nogdb::ClassDescriptor {}.id);
        assert(getSizeOfSubClasses(txnRo4, res10) == 2);
        res11 = txnRo4.getClass("test_11");
        assert(res11.id != nogdb::ClassDescriptor {}.id);
        assert(res11.base == res10.id);
        assert(getSizeOfSubClasses(txnRo4, res11) == 1);
        res12 = txnRo4.getClass("test_12");
        assert(res12.id != nogdb::ClassDescriptor {}.id);
        assert(res12.base == res10.id);
        assert(getSizeOfSubClasses(txnRo4, res12) == 0);
        res13 = txnRo4.getClass("test_13");
        assert(res13.id != nogdb::ClassDescriptor {}.id);
        assert(res13.base == res11.id);
        assert(getSizeOfSubClasses(txnRo4, res13) == 0);

        res10 = txnRo1.getClass("test_10");
        assert(getSizeOfSubClasses(txnRo1, res10) == 0);
        try {
            txnRo1.getClass("test_11");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo1.getClass("test_12");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo1.getClass("test_13");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

        res = txnRo2.getClass("test_10");
        assert(getSizeOfSubClasses(txnRo2, res) == 0);
        try {
            txnRo2.getClass("test_11");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo2.getClass("test_12");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo2.getClass("test_13");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

        res = txnRo3.getClass("test_10");
        assert(getSizeOfSubClasses(txnRo3, res) == 0);
        try {
            txnRo3.getClass("test_11");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo3.getClass("test_12");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo3.getClass("test_13");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_create_class_extend_rollback()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_20", nogdb::ClassType::VERTEX);
        txnRw.addProperty("test_20", "prop0", nogdb::PropertyType::INTEGER);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addSubClassOf("test_20", "test_21");
        txnRw1.addSubClassOf("test_20", "test_22");
        txnRw1.addSubClassOf("test_21", "test_23");
        txnRw1.addProperty("test_21", "prop1", nogdb::PropertyType::INTEGER);
        txnRw1.addProperty("test_22", "prop2", nogdb::PropertyType::INTEGER);
        txnRw1.addProperty("test_23", "prop3", nogdb::PropertyType::INTEGER);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.getClass("test_20");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(getSizeOfSubClasses(txnRw1, res) == 2);
        res = txnRw1.getClass("test_21");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(res.base == txnRw1.getClass("test_20").id);
        assert(getSizeOfSubClasses(txnRw1, res) == 1);
        res = txnRw1.getClass("test_22");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(res.base == txnRw1.getClass("test_20").id);
        assert(getSizeOfSubClasses(txnRw1, res) == 0);
        res = txnRw1.getClass("test_23");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(res.base == txnRw1.getClass("test_21").id);
        assert(getSizeOfSubClasses(txnRw1, res) == 0);

        txnRw1.addVertex("test_20", nogdb::Record {}.set("prop0", 1));
        txnRw1.addVertex("test_21", nogdb::Record {}.set("prop0", 1).set("prop1", 1));
        txnRw1.addVertex("test_22", nogdb::Record {}.set("prop0", 1).set("prop2", 1));
        txnRw1.addVertex("test_23", nogdb::Record {}.set("prop0", 1).set("prop3", 1));

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRw2.getClass("test_20");
        assert(getSizeOfSubClasses(txnRw2, res) == 0);
        try {
            txnRw2.getClass("test_21");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRw2.getClass("test_22");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRw2.getClass("test_23");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

        res = txnRo4.getClass("test_20");
        assert(getSizeOfSubClasses(txnRo4, res) == 0);
        try {
            txnRo4.getClass("test_21");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo4.getClass("test_22");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo4.getClass("test_23");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

        res = txnRo1.getClass("test_20");
        assert(getSizeOfSubClasses(txnRo1, res) == 0);
        try {
            txnRo1.getClass("test_21");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo1.getClass("test_22");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo1.getClass("test_23");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

        res = txnRo1.getClass("test_20");
        assert(getSizeOfSubClasses(txnRo1, res) == 0);
        try {
            txnRo1.getClass("test_21");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo1.getClass("test_22");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo1.getClass("test_23");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

        res = txnRo2.getClass("test_20");
        assert(getSizeOfSubClasses(txnRo2, res) == 0);
        try {
            txnRo2.getClass("test_21");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo2.getClass("test_22");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo2.getClass("test_23");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

        res = txnRo3.getClass("test_20");
        assert(getSizeOfSubClasses(txnRo3, res) == 0);
        try {
            txnRo3.getClass("test_21");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo3.getClass("test_22");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRo3.getClass("test_23");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_class_extend_commit()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_30", nogdb::ClassType::VERTEX);
        txnRw.addProperty("test_30", "prop0", nogdb::PropertyType::INTEGER);
        txnRw.addSubClassOf("test_30", "test_31");
        txnRw.addSubClassOf("test_30", "test_32");
        txnRw.addSubClassOf("test_31", "test_33");
        txnRw.addProperty("test_31", "prop1", nogdb::PropertyType::INTEGER);
        txnRw.addProperty("test_32", "prop2", nogdb::PropertyType::INTEGER);
        txnRw.addProperty("test_33", "prop3", nogdb::PropertyType::INTEGER);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropClass("test_31");
        txnRw1.dropClass("test_32");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        try {
            txnRw1.getClass("test_31");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRw1.getClass("test_32");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        auto res = txnRw1.getClass("test_30");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(getSizeOfSubClasses(txnRw1, res) == 1);
        res = txnRw1.getClass("test_33");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(res.base == txnRw1.getClass("test_30").id);

        res = txnRo1.getClass("test_30");
        assert(getSizeOfSubClasses(txnRo1, res) == 2);
        res = txnRo1.getClass("test_31");
        assert(getSizeOfSubClasses(txnRo1, res) == 1);
        res = txnRo1.getClass("test_32");
        assert(res.base == txnRo1.getClass("test_30").id);
        res = txnRo1.getClass("test_33");
        assert(res.base == txnRo1.getClass("test_31").id);

        res = txnRo2.getClass("test_30");
        assert(getSizeOfSubClasses(txnRo2, res) == 2);
        res = txnRo2.getClass("test_31");
        assert(getSizeOfSubClasses(txnRo2, res) == 1);
        res = txnRo2.getClass("test_32");
        assert(res.base == txnRo2.getClass("test_30").id);
        res = txnRo2.getClass("test_33");
        assert(res.base == txnRo2.getClass("test_31").id);

        res = txnRo3.getClass("test_30");
        assert(getSizeOfSubClasses(txnRo3, res) == 2);
        res = txnRo3.getClass("test_31");
        assert(getSizeOfSubClasses(txnRo3, res) == 1);
        res = txnRo3.getClass("test_32");
        assert(res.base == txnRo3.getClass("test_30").id);
        res = txnRo3.getClass("test_33");
        assert(res.base == txnRo3.getClass("test_31").id);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        try {
            txnRw2.getClass("test_31");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRw2.getClass("test_32");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        res = txnRw2.getClass("test_30");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(getSizeOfSubClasses(txnRw2, res) == 1);
        res = txnRw2.getClass("test_33");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(res.base == txnRw2.getClass("test_30").id);

        res = txnRo4.getClass("test_30");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(getSizeOfSubClasses(txnRo4, res) == 1);
        res = txnRo4.getClass("test_33");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(res.base == txnRo4.getClass("test_30").id);

        res = txnRo1.getClass("test_30");
        assert(getSizeOfSubClasses(txnRo1, res) == 2);
        res = txnRo1.getClass("test_31");
        assert(getSizeOfSubClasses(txnRo1, res) == 1);
        res = txnRo1.getClass("test_32");
        assert(res.base == txnRo1.getClass("test_30").id);
        res = txnRo1.getClass("test_33");
        assert(res.base == txnRo1.getClass("test_31").id);

        res = txnRo2.getClass("test_30");
        assert(getSizeOfSubClasses(txnRo2, res) == 2);
        res = txnRo2.getClass("test_31");
        assert(getSizeOfSubClasses(txnRo2, res) == 1);
        res = txnRo2.getClass("test_32");
        assert(res.base == txnRo2.getClass("test_30").id);
        res = txnRo2.getClass("test_33");
        assert(res.base == txnRo2.getClass("test_31").id);

        res = txnRo3.getClass("test_30");
        assert(getSizeOfSubClasses(txnRo3, res) == 2);
        res = txnRo3.getClass("test_31");
        assert(getSizeOfSubClasses(txnRo3, res) == 1);
        res = txnRo3.getClass("test_32");
        assert(res.base == txnRo3.getClass("test_30").id);
        res = txnRo3.getClass("test_33");
        assert(res.base == txnRo3.getClass("test_31").id);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_class_extend_rollback()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_40", nogdb::ClassType::VERTEX);
        txnRw.addProperty("test_40", "prop0", nogdb::PropertyType::INTEGER);
        txnRw.addSubClassOf("test_40", "test_41");
        txnRw.addSubClassOf("test_40", "test_42");
        txnRw.addSubClassOf("test_41", "test_43");
        txnRw.addProperty("test_41", "prop1", nogdb::PropertyType::INTEGER);
        txnRw.addProperty("test_42", "prop2", nogdb::PropertyType::INTEGER);
        txnRw.addProperty("test_43", "prop3", nogdb::PropertyType::INTEGER);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropClass("test_41");
        txnRw1.dropClass("test_42");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        try {
            txnRw1.getClass("test_41");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        try {
            txnRw1.getClass("test_42");
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        }
        auto res = txnRw1.getClass("test_40");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(getSizeOfSubClasses(txnRw1, res) == 1);
        res = txnRw1.getClass("test_43");
        assert(res.id != nogdb::ClassDescriptor {}.id);
        assert(res.base == txnRw1.getClass("test_40").id);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRw2.getClass("test_40");
        assert(getSizeOfSubClasses(txnRw2, res) == 2);
        res = txnRw2.getClass("test_41");
        assert(getSizeOfSubClasses(txnRw2, res) == 1);
        res = txnRw2.getClass("test_42");
        assert(res.base == txnRw2.getClass("test_40").id);
        res = txnRw2.getClass("test_43");
        assert(res.base == txnRw2.getClass("test_41").id);

        res = txnRo4.getClass("test_40");
        assert(getSizeOfSubClasses(txnRo4, res) == 2);
        res = txnRo4.getClass("test_41");
        assert(getSizeOfSubClasses(txnRo4, res) == 1);
        res = txnRo4.getClass("test_42");
        assert(res.base == txnRo4.getClass("test_40").id);
        res = txnRo4.getClass("test_43");
        assert(res.base == txnRo4.getClass("test_41").id);

        res = txnRo1.getClass("test_40");
        assert(getSizeOfSubClasses(txnRo1, res) == 2);
        res = txnRo1.getClass("test_41");
        assert(getSizeOfSubClasses(txnRo1, res) == 1);
        res = txnRo1.getClass("test_42");
        assert(res.base == txnRo1.getClass("test_40").id);
        res = txnRo1.getClass("test_43");
        assert(res.base == txnRo1.getClass("test_41").id);

        res = txnRo2.getClass("test_40");
        assert(getSizeOfSubClasses(txnRo2, res) == 2);
        res = txnRo2.getClass("test_41");
        assert(getSizeOfSubClasses(txnRo2, res) == 1);
        res = txnRo2.getClass("test_42");
        assert(res.base == txnRo2.getClass("test_40").id);
        res = txnRo2.getClass("test_43");
        assert(res.base == txnRo2.getClass("test_41").id);

        res = txnRo3.getClass("test_40");
        assert(getSizeOfSubClasses(txnRo3, res) == 2);
        res = txnRo3.getClass("test_41");
        assert(getSizeOfSubClasses(txnRo3, res) == 1);
        res = txnRo3.getClass("test_42");
        assert(res.base == txnRo3.getClass("test_40").id);
        res = txnRo3.getClass("test_43");
        assert(res.base == txnRo3.getClass("test_41").id);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_add_property_commit()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_100", nogdb::ClassType::VERTEX);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addProperty("test_100", "prop1", nogdb::PropertyType::INTEGER);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.getClass("test_100");
        assert(propertyExists(txnRw1, "test_100", "prop1"));
        txnRw1.addVertex("test_100", nogdb::Record {}.set("prop1", 1));

        res = txnRo1.getClass("test_100");
        assert(!propertyExists(txnRo1, "test_100", "prop1"));
        res = txnRo2.getClass("test_100");
        assert(!propertyExists(txnRo2, "test_100", "prop1"));
        res = txnRo3.getClass("test_100");
        assert(!propertyExists(txnRo3, "test_100", "prop1"));

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRw2.getClass("test_100");
        assert(propertyExists(txnRw2, "test_100", "prop1"));
        txnRw2.addVertex("test_100", nogdb::Record {}.set("prop1", 2));

        res = txnRo4.getClass("test_100");
        assert(propertyExists(txnRo4, "test_100", "prop1"));

        res = txnRo1.getClass("test_100");
        assert(!propertyExists(txnRo1, "test_100", "prop1"));
        res = txnRo2.getClass("test_100");
        assert(!propertyExists(txnRo2, "test_100", "prop1"));
        res = txnRo3.getClass("test_100");
        assert(!propertyExists(txnRo3, "test_100", "prop1"));

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_add_property_rollback()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_101", nogdb::ClassType::VERTEX);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addProperty("test_101", "prop1", nogdb::PropertyType::INTEGER);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.getClass("test_101");
        assert(propertyExists(txnRw1, "test_101", "prop1"));
        txnRw1.addVertex("test_101", nogdb::Record {}.set("prop1", 1));

        res = txnRo1.getClass("test_101");
        assert(!propertyExists(txnRo1, "test_101", "prop1"));
        res = txnRo2.getClass("test_101");
        assert(!propertyExists(txnRo2, "test_101", "prop1"));
        res = txnRo3.getClass("test_101");
        assert(!propertyExists(txnRo3, "test_101", "prop1"));

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRo4.getClass("test_101");
        assert(!propertyExists(txnRo4, "test_101", "prop1"));
        res = txnRw2.getClass("test_101");
        assert(!propertyExists(txnRw2, "test_101", "prop1"));
        try {
            txnRw2.addVertex("test_101", nogdb::Record {}.set("prop1", 2));
            assert(false);
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
        }

        res = txnRo1.getClass("test_101");
        assert(!propertyExists(txnRo1, "test_101", "prop1"));
        res = txnRo2.getClass("test_101");
        assert(!propertyExists(txnRo2, "test_101", "prop1"));
        res = txnRo3.getClass("test_101");
        assert(!propertyExists(txnRo3, "test_101", "prop1"));

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_property_commit()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_102", nogdb::ClassType::VERTEX);
        txnRw.addProperty("test_102", "prop1", nogdb::PropertyType::TEXT);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropProperty("test_102", "prop1");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.getClass("test_102");
        assert(!propertyExists(txnRw1, "test_102", "prop1"));
        try {
            txnRw1.addVertex("test_102", nogdb::Record {}.set("prop1", "hi"));
            assert(false);
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
        }

        res = txnRo1.getClass("test_102");
        assert(propertyExists(txnRo1, "test_102", "prop1"));
        res = txnRo2.getClass("test_102");
        assert(propertyExists(txnRo2, "test_102", "prop1"));
        res = txnRo3.getClass("test_102");
        assert(propertyExists(txnRo3, "test_102", "prop1"));

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRw2.getClass("test_102");
        assert(!propertyExists(txnRw2, "test_102", "prop1"));
        try {
            txnRw2.addVertex("test_102", nogdb::Record {}.set("prop1", "world"));
            assert(false);
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
        }

        res = txnRo4.getClass("test_102");
        assert(!propertyExists(txnRo4, "test_102", "prop1"));

        res = txnRo1.getClass("test_102");
        assert(propertyExists(txnRo1, "test_102", "prop1"));
        res = txnRo2.getClass("test_102");
        assert(propertyExists(txnRo2, "test_102", "prop1"));
        res = txnRo3.getClass("test_102");
        assert(propertyExists(txnRo3, "test_102", "prop1"));

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_property_rollback()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_103", nogdb::ClassType::VERTEX);
        txnRw.addProperty("test_103", "prop1", nogdb::PropertyType::TEXT);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropProperty("test_103", "prop1");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.getClass("test_103");
        assert(!propertyExists(txnRw1, "test_103", "prop1"));
        try {
            txnRw1.addVertex("test_103", nogdb::Record {}.set("prop1", "hi"));
            assert(false);
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
        }

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRw2.getClass("test_103");
        assert(propertyExists(txnRw2, "test_103", "prop1"));
        txnRw2.addVertex("test_103", nogdb::Record {}.set("prop1", "world"));

        res = txnRo4.getClass("test_103");
        assert(propertyExists(txnRo4, "test_103", "prop1"));

        res = txnRo1.getClass("test_103");
        assert(propertyExists(txnRo1, "test_103", "prop1"));
        res = txnRo2.getClass("test_103");
        assert(propertyExists(txnRo2, "test_103", "prop1"));
        res = txnRo3.getClass("test_103");
        assert(propertyExists(txnRo3, "test_103", "prop1"));

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_alter_property_commit()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_104", nogdb::ClassType::VERTEX);
        txnRw.addProperty("test_104", "prop1", nogdb::PropertyType::INTEGER);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.renameProperty("test_104", "prop1", "prop11");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.getClass("test_104");
        assert(!propertyExists(txnRw1, "test_104", "prop1"));
        assert(propertyExists(txnRw1, "test_104", "prop11"));
        txnRw1.addVertex("test_104", nogdb::Record {}.set("prop11", 1));
        try {
            txnRw1.addVertex("test_104", nogdb::Record {}.set("prop1", 1));
            assert(false);
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
        }

        res = txnRo1.getClass("test_104");
        assert(propertyExists(txnRo1, "test_104", "prop1"));
        assert(!propertyExists(txnRo1, "test_104", "prop11"));
        res = txnRo2.getClass("test_104");
        assert(propertyExists(txnRo2, "test_104", "prop1"));
        assert(!propertyExists(txnRo2, "test_104", "prop11"));
        res = txnRo3.getClass("test_104");
        assert(propertyExists(txnRo3, "test_104", "prop1"));
        assert(!propertyExists(txnRo3, "test_104", "prop11"));

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRw2.getClass("test_104");
        assert(!propertyExists(txnRw2, "test_104", "prop1"));
        assert(propertyExists(txnRw2, "test_104", "prop11"));
        txnRw2.addVertex("test_104", nogdb::Record {}.set("prop11", 1));
        try {
            txnRw2.addVertex("test_104", nogdb::Record {}.set("prop1", 1));
            assert(false);
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
        }

        res = txnRo4.getClass("test_104");
        assert(!propertyExists(txnRo4, "test_104", "prop1"));
        assert(propertyExists(txnRo4, "test_104", "prop11"));

        res = txnRo1.getClass("test_104");
        assert(propertyExists(txnRo1, "test_104", "prop1"));
        assert(!propertyExists(txnRo1, "test_104", "prop11"));
        res = txnRo2.getClass("test_104");
        assert(propertyExists(txnRo2, "test_104", "prop1"));
        assert(!propertyExists(txnRo2, "test_104", "prop11"));
        res = txnRo3.getClass("test_104");
        assert(propertyExists(txnRo3, "test_104", "prop1"));
        assert(!propertyExists(txnRo3, "test_104", "prop11"));

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_alter_property_rollback()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_105", nogdb::ClassType::VERTEX);
        txnRw.addProperty("test_105", "prop1", nogdb::PropertyType::INTEGER);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.renameProperty("test_105", "prop1", "prop11");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.getClass("test_105");
        assert(!propertyExists(txnRw1, "test_105", "prop1"));
        assert(propertyExists(txnRw1, "test_105", "prop11"));
        txnRw1.addVertex("test_105", nogdb::Record {}.set("prop11", 1));
        try {
            txnRw1.addVertex("test_105", nogdb::Record {}.set("prop1", 1));
            assert(false);
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
        }

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRw2.getClass("test_105");
        assert(propertyExists(txnRw2, "test_105", "prop1"));
        assert(!propertyExists(txnRw2, "test_105", "prop11"));
        txnRw2.addVertex("test_105", nogdb::Record {}.set("prop1", 1));
        try {
            txnRw2.addVertex("test_105", nogdb::Record {}.set("prop11", 1));
            assert(false);
        } catch (const nogdb::Error& ex) {
            REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
        }

        res = txnRo4.getClass("test_105");
        assert(propertyExists(txnRo4, "test_105", "prop1"));
        assert(!propertyExists(txnRo4, "test_105", "prop11"));

        res = txnRo1.getClass("test_105");
        assert(propertyExists(txnRo1, "test_105", "prop1"));
        assert(!propertyExists(txnRo1, "test_105", "prop11"));
        res = txnRo2.getClass("test_105");
        assert(propertyExists(txnRo2, "test_105", "prop1"));
        assert(!propertyExists(txnRo2, "test_105", "prop11"));
        res = txnRo3.getClass("test_105");
        assert(propertyExists(txnRo3, "test_105", "prop1"));
        assert(!propertyExists(txnRo3, "test_105", "prop11"));

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_create_index_commit()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_106", nogdb::ClassType::VERTEX);
        txnRw.addProperty("test_106", "prop1", nogdb::PropertyType::INTEGER);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addIndex("test_106", "prop1");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.getClass("test_106");
        assert(indexExists(txnRw1, "test_106", "prop1"));

        res = txnRo1.getClass("test_106");
        assert(!indexExists(txnRo1, "test_106", "prop1"));
        res = txnRo2.getClass("test_106");
        assert(!indexExists(txnRo2, "test_106", "prop1"));
        res = txnRo3.getClass("test_106");
        assert(!indexExists(txnRo3, "test_106", "prop1"));

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRw2.getClass("test_106");
        assert(indexExists(txnRw2, "test_106", "prop1"));
        res = txnRo4.getClass("test_106");
        assert(indexExists(txnRo4, "test_106", "prop1"));

        res = txnRo1.getClass("test_106");
        assert(!indexExists(txnRo1, "test_106", "prop1"));
        res = txnRo2.getClass("test_106");
        assert(!indexExists(txnRo2, "test_106", "prop1"));
        res = txnRo3.getClass("test_106");
        assert(!indexExists(txnRo3, "test_106", "prop1"));

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_create_index_rollback()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_107", nogdb::ClassType::VERTEX);
        txnRw.addProperty("test_107", "prop1", nogdb::PropertyType::INTEGER);
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addIndex("test_107", "prop1");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.getClass("test_107");
        assert(indexExists(txnRw1, "test_107", "prop1"));

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRw2.getClass("test_107");
        assert(!indexExists(txnRw2, "test_107", "prop1"));
        res = txnRo4.getClass("test_107");
        assert(!indexExists(txnRo4, "test_107", "prop1"));

        res = txnRo1.getClass("test_107");
        assert(!indexExists(txnRo1, "test_107", "prop1"));
        res = txnRo2.getClass("test_107");
        assert(!indexExists(txnRo2, "test_107", "prop1"));
        res = txnRo3.getClass("test_107");
        assert(!indexExists(txnRo3, "test_107", "prop1"));

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_index_commit()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_108", nogdb::ClassType::VERTEX);
        txnRw.addProperty("test_108", "prop1", nogdb::PropertyType::INTEGER);
        txnRw.addIndex("test_108", "prop1");
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropIndex("test_108", "prop1");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.getClass("test_108");
        assert(!indexExists(txnRw1, "test_108", "prop1"));

        res = txnRo1.getClass("test_108");
        assert(indexExists(txnRo1, "test_108", "prop1"));
        res = txnRo2.getClass("test_108");
        assert(indexExists(txnRo2, "test_108", "prop1"));
        res = txnRo3.getClass("test_108");
        assert(indexExists(txnRo3, "test_108", "prop1"));

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRw2.getClass("test_108");
        assert(!indexExists(txnRw2, "test_108", "prop1"));
        res = txnRo4.getClass("test_108");
        assert(!indexExists(txnRo4, "test_108", "prop1"));

        res = txnRo1.getClass("test_108");
        assert(indexExists(txnRo1, "test_108", "prop1"));
        res = txnRo2.getClass("test_108");
        assert(indexExists(txnRo2, "test_108", "prop1"));
        res = txnRo3.getClass("test_108");
        assert(indexExists(txnRo3, "test_108", "prop1"));

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_index_rollback()
{
    try {
        auto txnRw = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txnRw.addClass("test_109", nogdb::ClassType::VERTEX);
        txnRw.addProperty("test_109", "prop1", nogdb::PropertyType::INTEGER);
        txnRw.addIndex("test_109", "prop1");
        txnRw.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropIndex("test_109", "prop1");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res = txnRw1.getClass("test_109");
        assert(!indexExists(txnRw1, "test_109", "prop1"));

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        res = txnRw2.getClass("test_109");
        assert(indexExists(txnRw2, "test_109", "prop1"));
        res = txnRo4.getClass("test_109");
        assert(indexExists(txnRo4, "test_109", "prop1"));

        res = txnRo1.getClass("test_109");
        assert(indexExists(txnRo1, "test_109", "prop1"));
        res = txnRo2.getClass("test_109");
        assert(indexExists(txnRo2, "test_109", "prop1"));
        res = txnRo3.getClass("test_109");
        assert(indexExists(txnRo3, "test_109", "prop1"));

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_create_class_multiversion_commit()
{
    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.addClass("test_mv_1", nogdb::ClassType::VERTEX);

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addClass("test_mv_2", nogdb::ClassType::EDGE);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result0 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_1");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            try {
                txn.getClass("test_mv_2");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_2");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            auto res = txn.getClass("test_mv_1");
            assert(res.id != nogdb::ClassDescriptor {}.id);
        };

        auto verify_result2 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_1");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            res = txn.getClass("test_mv_2");
            assert(res.id != nogdb::ClassDescriptor {}.id);
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result2(txnRo4);
        verify_result2(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_create_class_multiversion_rollback()
{
    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.addClass("test_mv_3", nogdb::ClassType::VERTEX);

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addClass("test_mv_4", nogdb::ClassType::EDGE);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result0 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_3");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            try {
                txn.getClass("test_mv_4");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_4");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            auto res = txn.getClass("test_mv_3");
            assert(res.id != nogdb::ClassDescriptor {}.id);
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result1(txnRo4);
        verify_result1(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_class_multiversion_commit()
{
    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.dropClass("test_mv_2");

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropClass("test_mv_1");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result2 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_1");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            try {
                txn.getClass("test_mv_2");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_2");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            auto res = txn.getClass("test_mv_1");
            assert(res.id != nogdb::ClassDescriptor {}.id);
        };

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_1");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            res = txn.getClass("test_mv_2");
            assert(res.id != nogdb::ClassDescriptor {}.id);
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result2(txnRo4);
        verify_result2(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_class_multiversion_rollback()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_4", nogdb::ClassType::EDGE);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.dropClass("test_mv_3");

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropClass("test_mv_4");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result1 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_3");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            res = txn.getClass("test_mv_4");
            assert(res.id != nogdb::ClassDescriptor {}.id);
        };

        auto verify_result0 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_3");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            auto res = txn.getClass("test_mv_4");
            assert(res.id != nogdb::ClassDescriptor {}.id);
        };

        verify_result1(txnRo0);
        verify_result0(txnRo1);
        verify_result0(txnRo2);
        verify_result0(txnRo3);
        verify_result0(txnRo4);
        verify_result0(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_alter_class_multiversion_commit()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_5", nogdb::ClassType::VERTEX);
        txn.addClass("test_mv_6", nogdb::ClassType::EDGE);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.renameClass("test_mv_5", "test_mv_55");

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.renameClass("test_mv_6", "test_mv_66");
        txnRw1.renameClass("test_mv_55", "test_mv_555");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result0 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_55");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            try {
                txn.getClass("test_mv_66");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            try {
                txn.getClass("test_mv_555");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_66");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            try {
                txn.getClass("test_mv_555");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            auto res = txn.getClass("test_mv_55");
            assert(res.id != nogdb::ClassDescriptor {}.id);
        };

        auto verify_result2 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_555");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            res = txn.getClass("test_mv_66");
            assert(res.id != nogdb::ClassDescriptor {}.id);
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result2(txnRo4);
        verify_result2(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_alter_class_multiversion_rollback()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_7", nogdb::ClassType::VERTEX);
        txn.addClass("test_mv_8", nogdb::ClassType::EDGE);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.renameClass("test_mv_7", "test_mv_77");

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.renameClass("test_mv_8", "test_mv_88");
        txnRw1.renameClass("test_mv_77", "test_mv_777");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result1 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_77");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            try {
                txn.getClass("test_mv_88");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            try {
                txn.getClass("test_mv_777");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
        };

        auto verify_result0 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_88");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            try {
                txn.getClass("test_mv_777");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            auto res = txn.getClass("test_mv_77");
            assert(res.id != nogdb::ClassDescriptor {}.id);
        };

        verify_result1(txnRo0);
        verify_result0(txnRo1);
        verify_result0(txnRo2);
        verify_result0(txnRo3);
        verify_result0(txnRo4);
        verify_result0(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_create_class_extend_multiversion_commit()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_100", nogdb::ClassType::VERTEX);
        txn.addProperty("test_mv_100", "prop100", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.addSubClassOf("test_mv_100", "test_mv_101");
        txnRw0.addProperty("test_mv_101", "prop101", nogdb::PropertyType::INTEGER);
        txnRw0.addSubClassOf("test_mv_100", "test_mv_102");
        txnRw0.addProperty("test_mv_102", "prop102", nogdb::PropertyType::INTEGER);

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addSubClassOf("test_mv_101", "test_mv_103");
        txnRw1.addProperty("test_mv_103", "prop103", nogdb::PropertyType::INTEGER);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_100");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(getSizeOfSubClasses(txn, res) == 0);
            try {
                txn.getClass("test_mv_101");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            try {
                txn.getClass("test_mv_102");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            try {
                txn.getClass("test_mv_103");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_103");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            auto res = txn.getClass("test_mv_101");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(res.base == txn.getClass("test_mv_100").id);
            res = txn.getClass("test_mv_102");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(res.base == txn.getClass("test_mv_100").id);
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                txn.addVertex("test_mv_101", nogdb::Record {}.set("prop100", 1).set("prop101", 1));
                txn.addVertex("test_mv_102", nogdb::Record {}.set("prop100", 1).set("prop102", 1));
            }
        };

        auto verify_result2 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_100");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(getSizeOfSubClasses(txn, res) == 2);
            res = txn.getClass("test_mv_101");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(res.base == txn.getClass("test_mv_100").id);
            assert(getSizeOfSubClasses(txn, res) == 1);
            res = txn.getClass("test_mv_102");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(res.base == txn.getClass("test_mv_100").id);
            res = txn.getClass("test_mv_103");
            assert(res.base == txn.getClass("test_mv_101").id);
            assert(res.id != nogdb::ClassDescriptor {}.id);
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                txn.addVertex("test_mv_101", nogdb::Record {}.set("prop100", 1).set("prop101", 1));
                txn.addVertex("test_mv_102", nogdb::Record {}.set("prop100", 1).set("prop102", 1));
                txn.addVertex("test_mv_103", nogdb::Record {}.set("prop100", 1).set("prop101", 1).set("prop103", 1));
            }
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result2(txnRo4);
        verify_result2(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_create_class_extend_multiversion_rollback()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_200", nogdb::ClassType::VERTEX);
        txn.addProperty("test_mv_200", "prop200", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.addSubClassOf("test_mv_200", "test_mv_201");
        txnRw0.addProperty("test_mv_201", "prop201", nogdb::PropertyType::INTEGER);
        txnRw0.addSubClassOf("test_mv_200", "test_mv_202");
        txnRw0.addProperty("test_mv_202", "prop202", nogdb::PropertyType::INTEGER);

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addSubClassOf("test_mv_201", "test_mv_203");
        txnRw1.addProperty("test_mv_203", "prop203", nogdb::PropertyType::INTEGER);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_200");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(getSizeOfSubClasses(txn, res) == 0);
            try {
                txn.getClass("test_mv_201");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            try {
                txn.getClass("test_mv_202");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            try {
                txn.getClass("test_mv_203");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_203");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            auto res = txn.getClass("test_mv_201");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(res.base == txn.getClass("test_mv_200").id);
            res = txn.getClass("test_mv_202");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(res.base == txn.getClass("test_mv_200").id);
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                txn.addVertex("test_mv_201", nogdb::Record {}.set("prop200", 1).set("prop201", 1));
                txn.addVertex("test_mv_202", nogdb::Record {}.set("prop200", 1).set("prop202", 1));
            }
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result1(txnRo4);
        verify_result1(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_class_extend_multiversion_commit()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_300", nogdb::ClassType::VERTEX);
        txn.addProperty("test_mv_300", "prop300", nogdb::PropertyType::INTEGER);
        txn.addSubClassOf("test_mv_300", "test_mv_301");
        txn.addProperty("test_mv_301", "prop301", nogdb::PropertyType::INTEGER);
        txn.addSubClassOf("test_mv_300", "test_mv_302");
        txn.addProperty("test_mv_302", "prop302", nogdb::PropertyType::INTEGER);
        txn.addSubClassOf("test_mv_301", "test_mv_303");
        txn.addProperty("test_mv_303", "prop303", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.dropClass("test_mv_301");

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropClass("test_mv_302");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result2 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_300");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(getSizeOfSubClasses(txn, res) == 1);
            res = txn.getClass("test_mv_303");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(res.base == txn.getClass("test_mv_300").id);
            try {
                txn.getClass("test_mv_301");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            try {
                txn.getClass("test_mv_302");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_301");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            auto res = txn.getClass("test_mv_300");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(getSizeOfSubClasses(txn, res) == 2);
            res = txn.getClass("test_mv_302");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            res = txn.getClass("test_mv_303");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(res.base == txn.getClass("test_mv_300").id);
        };

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_300");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(getSizeOfSubClasses(txn, res) == 2);
            res = txn.getClass("test_mv_301");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(res.base == txn.getClass("test_mv_300").id);
            assert(getSizeOfSubClasses(txn, res) == 1);
            res = txn.getClass("test_mv_302");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(res.base == txn.getClass("test_mv_300").id);
            res = txn.getClass("test_mv_303");
            assert(res.base == txn.getClass("test_mv_301").id);
            assert(res.id != nogdb::ClassDescriptor {}.id);
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result2(txnRo4);
        verify_result2(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_class_extend_multiversion_rollback()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_400", nogdb::ClassType::VERTEX);
        txn.addProperty("test_mv_400", "prop400", nogdb::PropertyType::INTEGER);
        txn.addSubClassOf("test_mv_400", "test_mv_401");
        txn.addProperty("test_mv_401", "prop401", nogdb::PropertyType::INTEGER);
        txn.addSubClassOf("test_mv_400", "test_mv_402");
        txn.addProperty("test_mv_402", "prop402", nogdb::PropertyType::INTEGER);
        txn.addSubClassOf("test_mv_401", "test_mv_403");
        txn.addProperty("test_mv_403", "prop403", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.dropClass("test_mv_401");

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropClass("test_mv_402");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result1 = [](nogdb::Transaction& txn) {
            try {
                txn.getClass("test_mv_401");
                assert(false);
            } catch (const nogdb::Error& ex) {
                REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
            }
            auto res = txn.getClass("test_mv_400");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(getSizeOfSubClasses(txn, res) == 2);
            res = txn.getClass("test_mv_402");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            res = txn.getClass("test_mv_403");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(res.base == txn.getClass("test_mv_400").id);
        };

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_400");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(getSizeOfSubClasses(txn, res) == 2);
            res = txn.getClass("test_mv_401");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(res.base == txn.getClass("test_mv_400").id);
            assert(getSizeOfSubClasses(txn, res) == 1);
            res = txn.getClass("test_mv_402");
            assert(res.id != nogdb::ClassDescriptor {}.id);
            assert(res.base == txn.getClass("test_mv_400").id);
            res = txn.getClass("test_mv_403");
            assert(res.base == txn.getClass("test_mv_401").id);
            assert(res.id != nogdb::ClassDescriptor {}.id);
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result1(txnRo4);
        verify_result1(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_add_property_multiversion_commit()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_10", nogdb::ClassType::VERTEX);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.addProperty("test_mv_10", "prop1", nogdb::PropertyType::INTEGER);

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addProperty("test_mv_10", "prop2", nogdb::PropertyType::INTEGER);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_10");
            assert(!propertyExists(txn, "test_mv_10", "prop1"));
            assert(!propertyExists(txn, "test_mv_10", "prop2"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                try {
                    txn.addVertex("test_mv_10", nogdb::Record {}.set("prop1", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
                try {
                    txn.addVertex("test_mv_10", nogdb::Record {}.set("prop2", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
            }
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_10");
            assert(propertyExists(txn, "test_mv_10", "prop1"));
            assert(!propertyExists(txn, "test_mv_10", "prop2"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                txn.addVertex("test_mv_10", nogdb::Record {}.set("prop1", 1));
                try {
                    txn.addVertex("test_mv_10", nogdb::Record {}.set("prop2", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
            }
        };

        auto verify_result2 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_10");
            assert(propertyExists(txn, "test_mv_10", "prop1"));
            assert(propertyExists(txn, "test_mv_10", "prop2"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                txn.addVertex("test_mv_10", nogdb::Record {}.set("prop1", 1));
                txn.addVertex("test_mv_10", nogdb::Record {}.set("prop2", 1));
            }
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result2(txnRo4);
        verify_result2(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_add_property_multiversion_rollback()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_20", nogdb::ClassType::VERTEX);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.addProperty("test_mv_20", "prop1", nogdb::PropertyType::INTEGER);

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addProperty("test_mv_20", "prop2", nogdb::PropertyType::INTEGER);

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_20");
            assert(!propertyExists(txn, "test_mv_20", "prop1"));
            assert(!propertyExists(txn, "test_mv_20", "prop2"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                try {
                    txn.addVertex("test_mv_20", nogdb::Record {}.set("prop1", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
                try {
                    txn.addVertex("test_mv_20", nogdb::Record {}.set("prop2", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
            }
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_20");
            assert(propertyExists(txn, "test_mv_20", "prop1"));
            assert(!propertyExists(txn, "test_mv_20", "prop2"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                txn.addVertex("test_mv_20", nogdb::Record {}.set("prop1", 1));
                try {
                    txn.addVertex("test_mv_20", nogdb::Record {}.set("prop2", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
            }
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result1(txnRo4);
        verify_result1(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_property_multiversion_commit()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_30", nogdb::ClassType::VERTEX);
        txn.addProperty("test_mv_30", "prop1", nogdb::PropertyType::INTEGER);
        txn.addProperty("test_mv_30", "prop2", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.dropProperty("test_mv_30", "prop2");

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropProperty("test_mv_30", "prop1");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result2 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_30");
            assert(!propertyExists(txn, "test_mv_30", "prop1"));
            assert(!propertyExists(txn, "test_mv_30", "prop2"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                try {
                    txn.addVertex("test_mv_30", nogdb::Record {}.set("prop1", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
                try {
                    txn.addVertex("test_mv_30", nogdb::Record {}.set("prop2", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
            }
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_30");
            assert(propertyExists(txn, "test_mv_30", "prop1"));
            assert(!propertyExists(txn, "test_mv_30", "prop2"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                txn.addVertex("test_mv_30", nogdb::Record {}.set("prop1", 1));
                try {
                    txn.addVertex("test_mv_30", nogdb::Record {}.set("prop2", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
            }
        };

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_30");
            assert(propertyExists(txn, "test_mv_30", "prop1"));
            assert(propertyExists(txn, "test_mv_30", "prop2"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                txn.addVertex("test_mv_30", nogdb::Record {}.set("prop1", 1));
                txn.addVertex("test_mv_30", nogdb::Record {}.set("prop2", 1));
            }
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result2(txnRo4);
        verify_result2(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_property_multiversion_rollback()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_40", nogdb::ClassType::VERTEX);
        txn.addProperty("test_mv_40", "prop1", nogdb::PropertyType::INTEGER);
        txn.addProperty("test_mv_40", "prop2", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.dropProperty("test_mv_40", "prop2");

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropProperty("test_mv_40", "prop1");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result1 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_40");
            assert(propertyExists(txn, "test_mv_40", "prop1"));
            assert(!propertyExists(txn, "test_mv_40", "prop2"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                txn.addVertex("test_mv_40", nogdb::Record {}.set("prop1", 1));
                try {
                    txn.addVertex("test_mv_40", nogdb::Record {}.set("prop2", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
            }
        };

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_40");
            assert(propertyExists(txn, "test_mv_40", "prop1"));
            assert(propertyExists(txn, "test_mv_40", "prop2"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                txn.addVertex("test_mv_40", nogdb::Record {}.set("prop1", 1));
                txn.addVertex("test_mv_40", nogdb::Record {}.set("prop2", 1));
            }
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result1(txnRo4);
        verify_result1(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_alter_property_multiversion_commit()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_50", nogdb::ClassType::VERTEX);
        txn.addProperty("test_mv_50", "prop1", nogdb::PropertyType::INTEGER);
        txn.addProperty("test_mv_50", "prop2", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.renameProperty("test_mv_50", "prop1", "prop11");

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.renameProperty("test_mv_50", "prop2", "prop22");
        txnRw1.renameProperty("test_mv_50", "prop11", "prop111");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_50");
            assert(!propertyExists(txn, "test_mv_50", "prop11"));
            assert(!propertyExists(txn, "test_mv_50", "prop22"));
            assert(!propertyExists(txn, "test_mv_50", "prop111"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                try {
                    txn.addVertex("test_mv_50", nogdb::Record {}.set("prop11", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
                try {
                    txn.addVertex("test_mv_50", nogdb::Record {}.set("prop22", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
                try {
                    txn.addVertex("test_mv_50", nogdb::Record {}.set("prop111", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
            }
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_50");
            assert(propertyExists(txn, "test_mv_50", "prop11"));
            assert(!propertyExists(txn, "test_mv_50", "prop22"));
            assert(!propertyExists(txn, "test_mv_50", "prop111"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                txn.addVertex("test_mv_50", nogdb::Record {}.set("prop11", 1));
                try {
                    txn.addVertex("test_mv_50", nogdb::Record {}.set("prop22", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
                try {
                    txn.addVertex("test_mv_50", nogdb::Record {}.set("prop111", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
            }
        };

        auto verify_result2 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_50");
            assert(!propertyExists(txn, "test_mv_50", "prop11"));
            assert(propertyExists(txn, "test_mv_50", "prop22"));
            assert(propertyExists(txn, "test_mv_50", "prop111"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                txn.addVertex("test_mv_50", nogdb::Record {}.set("prop22", 1).set("prop111", 1));
            }
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result2(txnRo4);
        verify_result2(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_alter_property_multiversion_rollback()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_60", nogdb::ClassType::VERTEX);
        txn.addProperty("test_mv_60", "prop1", nogdb::PropertyType::INTEGER);
        txn.addProperty("test_mv_60", "prop2", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.renameProperty("test_mv_60", "prop1", "prop11");

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.renameProperty("test_mv_60", "prop2", "prop22");
        txnRw1.renameProperty("test_mv_60", "prop11", "prop111");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_60");
            assert(!propertyExists(txn, "test_mv_60", "prop11"));
            assert(!propertyExists(txn, "test_mv_60", "prop22"));
            assert(!propertyExists(txn, "test_mv_60", "prop111"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                try {
                    txn.addVertex("test_mv_60", nogdb::Record {}.set("prop11", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
                try {
                    txn.addVertex("test_mv_60", nogdb::Record {}.set("prop22", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
                try {
                    txn.addVertex("test_mv_60", nogdb::Record {}.set("prop111", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
            }
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_60");
            assert(propertyExists(txn, "test_mv_60", "prop11"));
            assert(!propertyExists(txn, "test_mv_60", "prop22"));
            assert(!propertyExists(txn, "test_mv_60", "prop111"));
            if (txn.getTxnMode() == nogdb::TxnMode::READ_WRITE) {
                txn.addVertex("test_mv_60", nogdb::Record {}.set("prop11", 1));
                try {
                    txn.addVertex("test_mv_60", nogdb::Record {}.set("prop22", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
                try {
                    txn.addVertex("test_mv_60", nogdb::Record {}.set("prop111", 1));
                    assert(false);
                } catch (const nogdb::Error& ex) {
                    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
                }
            }
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result1(txnRo4);
        verify_result1(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_create_index_multiversion_commit()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_70", nogdb::ClassType::VERTEX);
        txn.addProperty("test_mv_70", "prop1", nogdb::PropertyType::INTEGER);
        txn.addProperty("test_mv_70", "prop2", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.addIndex("test_mv_70", "prop1");

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addIndex("test_mv_70", "prop2");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_70");
            assert(!indexExists(txn, "test_mv_70", "prop1"));
            assert(!indexExists(txn, "test_mv_70", "prop2"));
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_70");
            assert(indexExists(txn, "test_mv_70", "prop1"));
            assert(!indexExists(txn, "test_mv_70", "prop2"));
        };

        auto verify_result2 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_70");
            assert(indexExists(txn, "test_mv_70", "prop1"));
            assert(indexExists(txn, "test_mv_70", "prop2"));
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result2(txnRo4);
        verify_result2(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_create_index_multiversion_rollback()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_80", nogdb::ClassType::VERTEX);
        txn.addProperty("test_mv_80", "prop1", nogdb::PropertyType::INTEGER);
        txn.addProperty("test_mv_80", "prop2", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.addIndex("test_mv_80", "prop1");

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.addIndex("test_mv_80", "prop2");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_80");
            assert(!indexExists(txn, "test_mv_80", "prop1"));
            assert(!indexExists(txn, "test_mv_80", "prop2"));
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_80");
            assert(indexExists(txn, "test_mv_80", "prop1"));
            assert(!indexExists(txn, "test_mv_80", "prop2"));
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result1(txnRo4);
        verify_result1(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_index_multiversion_commit()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_90", nogdb::ClassType::VERTEX);
        txn.addProperty("test_mv_90", "prop1", nogdb::PropertyType::INTEGER);
        txn.addProperty("test_mv_90", "prop2", nogdb::PropertyType::INTEGER);
        txn.addIndex("test_mv_90", "prop1");
        txn.addIndex("test_mv_90", "prop2");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.dropIndex("test_mv_90", "prop1");

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropIndex("test_mv_90", "prop2");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.commit();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_90");
            assert(indexExists(txn, "test_mv_90", "prop1"));
            assert(indexExists(txn, "test_mv_90", "prop2"));
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_90");
            assert(!indexExists(txn, "test_mv_90", "prop1"));
            assert(indexExists(txn, "test_mv_90", "prop2"));
        };

        auto verify_result2 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_90");
            assert(!indexExists(txn, "test_mv_90", "prop1"));
            assert(!indexExists(txn, "test_mv_90", "prop2"));
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result2(txnRo4);
        verify_result2(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}

void test_schema_txn_drop_index_multiversion_rollback()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test_mv_91", nogdb::ClassType::VERTEX);
        txn.addProperty("test_mv_91", "prop1", nogdb::PropertyType::INTEGER);
        txn.addProperty("test_mv_91", "prop2", nogdb::PropertyType::INTEGER);
        txn.addIndex("test_mv_91", "prop1");
        txn.addIndex("test_mv_91", "prop2");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txnRw0 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo0 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw0.dropIndex("test_mv_91", "prop1");

        txnRw0.commit();

        auto txnRo1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw1 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txnRo2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.dropIndex("test_mv_91", "prop2");

        auto txnRo3 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);

        txnRw1.rollback();

        auto txnRo4 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txnRw2 = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);

        auto verify_result0 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_91");
            assert(indexExists(txn, "test_mv_91", "prop1"));
            assert(indexExists(txn, "test_mv_91", "prop2"));
        };

        auto verify_result1 = [](nogdb::Transaction& txn) {
            auto res = txn.getClass("test_mv_91");
            assert(!indexExists(txn, "test_mv_91", "prop1"));
            assert(indexExists(txn, "test_mv_91", "prop2"));
        };

        verify_result0(txnRo0);
        verify_result1(txnRo1);
        verify_result1(txnRo2);
        verify_result1(txnRo3);
        verify_result1(txnRo4);
        verify_result1(txnRw2);

    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    } catch (const nogdb::FatalError& err) {
        std::cout << "Fatal Error: " << err.what() << std::endl;
        assert(false);
    }
}
