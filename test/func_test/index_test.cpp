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

void test_create_index()
{
    init_vertex_index_test();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addIndex("index_test", "index_text", true);
        txn.addIndex("index_test", "index_tinyint_u", false);
        txn.addIndex("index_test", "index_tinyint", true);
        txn.addIndex("index_test", "index_smallint_u", false);
        txn.addIndex("index_test", "index_smallint", true);
        txn.addIndex("index_test", "index_int_u", false);
        txn.addIndex("index_test", "index_int", true);
        txn.addIndex("index_test", "index_bigint_u", false);
        txn.addIndex("index_test", "index_bigint", true);
        txn.addIndex("index_test", "index_real", false);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto classDesc = txn.getClass("index_test");
        auto properties = txn.getProperties(classDesc);
        auto indexSchema = txn.getIndexes(classDesc);
        assert(indexSchema.size() == 10);
        for (const auto& property : properties) {
            if (property.name == "index_blob")
                continue;
            auto foundIndex = std::find_if(
                indexSchema.cbegin(), indexSchema.cend(), [&classDesc, &property](const nogdb::IndexDescriptor& index) {
                    return index.propertyId == property.id && index.classId == classDesc.id;
                });
            assert(foundIndex != indexSchema.cend());
        }
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_create_index_extended_class()
{

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addSubClassOf("index_test", "index_test_2");
        txn.addProperty("index_test_2", "index_text_2", nogdb::PropertyType::TEXT);
        txn.addProperty("index_test_2", "index_int_2", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addIndex("index_test_2", "index_text", true);
        txn.addIndex("index_test_2", "index_tinyint_u", false);
        txn.addIndex("index_test_2", "index_tinyint", true);
        txn.addIndex("index_test_2", "index_smallint_u", false);
        txn.addIndex("index_test_2", "index_smallint", true);
        txn.addIndex("index_test_2", "index_int_u", false);
        txn.addIndex("index_test_2", "index_int", true);
        txn.addIndex("index_test_2", "index_bigint_u", false);
        txn.addIndex("index_test_2", "index_bigint", true);
        txn.addIndex("index_test_2", "index_real", false);
        txn.addIndex("index_test_2", "index_text_2", true);
        txn.addIndex("index_test_2", "index_int_2", false);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto classDesc = txn.getClass("index_test_2");
        auto properties = txn.getProperties(classDesc);
        auto indexSchema = txn.getIndexes(classDesc);
        assert(indexSchema.size() == 12);
        for (const auto& property : properties) {
            if (property.name == "index_blob")
                continue;
            auto foundIndex = std::find_if(
                indexSchema.cbegin(), indexSchema.cend(), [&classDesc, &property](const nogdb::IndexDescriptor& index) {
                    return index.propertyId == property.id && index.classId == classDesc.id;
                });
            assert(foundIndex != indexSchema.cend());
        }
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_create_invalid_index()
{

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.addIndex("index_test", "index_blob", true);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_PROPTYPE_INDEX, "NOGDB_CTX_INVALID_PROPTYPE_INDEX");
    }

    try {
        txn.addIndex("index_test", "index_text_2", false);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }

    try {
        txn.addIndex("index_test_2", "index_text_x", false);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }

    try {
        txn.addIndex("index_test_3", "index_text", false);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    try {
        txn.addIndex("index_test", "index_text", true);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_DUPLICATE_INDEX, "NOGDB_CTX_DUPLICATE_INDEX");
    }

    try {
        txn.addIndex("index_test_2", "index_text", true);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_DUPLICATE_INDEX, "NOGDB_CTX_DUPLICATE_INDEX");
    }
}

void test_drop_index()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("index_test", "index_text");
        txn.dropIndex("index_test", "index_tinyint_u");
        txn.dropIndex("index_test", "index_tinyint");
        txn.dropIndex("index_test", "index_smallint_u");
        txn.dropIndex("index_test", "index_smallint");
        txn.dropIndex("index_test", "index_int_u");
        txn.dropIndex("index_test", "index_int");
        txn.dropIndex("index_test", "index_bigint_u");
        txn.dropIndex("index_test", "index_bigint");
        txn.dropIndex("index_test", "index_real");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto classDesc = txn.getClass("index_test");
        auto properties = txn.getProperties(classDesc);
        auto indexSchema = txn.getIndexes(classDesc);
        assert(indexSchema.size() == 0);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_drop_index_extended_class()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("index_test_2", "index_int_2");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto classDesc = txn.getClass("index_test_2");
        auto properties = txn.getProperties(classDesc);
        auto indexSchema = txn.getIndexes(classDesc);
        assert(indexSchema.size() == 11);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        txn.getIndex("index_test_2", "index_int_2");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_INDEX, "NOGDB_CTX_NOEXST_INDEX");
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("index_test_2", "index_text");
        txn.dropIndex("index_test_2", "index_tinyint_u");
        txn.dropIndex("index_test_2", "index_tinyint");
        txn.dropIndex("index_test_2", "index_smallint_u");
        txn.dropIndex("index_test_2", "index_smallint");
        txn.dropIndex("index_test_2", "index_int_u");
        txn.dropIndex("index_test_2", "index_int");
        txn.dropIndex("index_test_2", "index_bigint_u");
        txn.dropIndex("index_test_2", "index_bigint");
        txn.dropIndex("index_test_2", "index_real");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto classDesc = txn.getClass("index_test_2");
        auto index = txn.getIndex("index_test_2", "index_text_2");
        auto indexSchema = txn.getIndexes(classDesc);
        assert(indexSchema.size() == 1);
        assert(indexSchema[0].id == index.id);
        assert(indexSchema[0].classId == index.classId);
        assert(indexSchema[0].propertyId == index.propertyId);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_drop_invalid_index()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.dropIndex("index_test", "index_text_x");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }

    try {
        txn.dropIndex("index_test_2", "index_text_x");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }

    try {
        txn.dropIndex("index_test_3", "index_text");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    try {
        txn.dropIndex("index_test", "index_text");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_INDEX, "NOGDB_CTX_NOEXST_INDEX");
    }

    try {
        txn.dropIndex("index_test_2", "index_text");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_INDEX, "NOGDB_CTX_NOEXST_INDEX");
    }

    try {
        txn.dropIndex("index_test_2", "index_int_2");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_INDEX, "NOGDB_CTX_NOEXST_INDEX");
    }

    try {
        txn.dropProperty("index_test_2", "index_text_2");
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_IN_USED_PROPERTY, "NOGDB_CTX_IN_USED_PROPERTY");
    }

    try {
        txn.dropClass("index_test_2");
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_IN_USED_PROPERTY, "NOGDB_CTX_IN_USED_PROPERTY");
    }
    txn.rollback();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("index_test_2", "index_text_2");
        txn.dropClass("index_test_2");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    destroy_vertex_index_test();
}

void test_create_index_with_records()
{
    init_vertex_index_test();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "aaa")
                .set("index_tinyint_u", uint8_t { 1 })
                .set("index_tinyint", int8_t { -1 })
                .set("index_smallint_u", uint16_t { 10 })
                .set("index_smallint", int16_t { -10 })
                .set("index_int_u", uint32_t { 100 })
                .set("index_int", int32_t { -100 })
                .set("index_bigint_u", uint64_t { 1000 })
                .set("index_bigint", int64_t { -1000 })
                .set("index_real", 2.0));
        txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "ccc")
                .set("index_tinyint_u", uint8_t { 2 })
                .set("index_tinyint", int8_t { 2 })
                .set("index_smallint_u", uint16_t { 20 })
                .set("index_smallint", int16_t { 20 })
                .set("index_int_u", uint32_t { 200 })
                .set("index_int", int32_t { 200 })
                .set("index_bigint_u", uint64_t { 2000 })
                .set("index_bigint", int64_t { 2000 })
                .set("index_real", 8.4));
        txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "bbb")
                .set("index_tinyint_u", uint8_t { 0 })
                .set("index_tinyint", int8_t { 0 })
                .set("index_smallint_u", uint16_t { 0 })
                .set("index_smallint", int16_t { 0 })
                .set("index_int_u", uint32_t { 0 })
                .set("index_int", int32_t { 0 })
                .set("index_bigint_u", uint64_t { 0 })
                .set("index_bigint", int64_t { 0 })
                .set("index_real", 0.0));
        txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "zz")
                .set("index_tinyint_u", uint8_t { 1 })
                .set("index_tinyint", int8_t { -123 })
                .set("index_smallint_u", uint16_t { 10 })
                .set("index_smallint", int16_t { -123 })
                .set("index_int_u", uint32_t { 100 })
                .set("index_int", int32_t { -123123123 })
                .set("index_bigint_u", uint64_t { 1000 })
                .set("index_bigint", int64_t { -123123123 })
                .set("index_real", 2.0));
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addIndex("index_test", "index_text", true);
        txn.addIndex("index_test", "index_tinyint_u", false);
        txn.addIndex("index_test", "index_tinyint", true);
        txn.addIndex("index_test", "index_smallint_u", false);
        txn.addIndex("index_test", "index_smallint", true);
        txn.addIndex("index_test", "index_int_u", false);
        txn.addIndex("index_test", "index_int", true);
        txn.addIndex("index_test", "index_bigint_u", false);
        txn.addIndex("index_test", "index_bigint", true);
        txn.addIndex("index_test", "index_real", false);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto classDesc = txn.getClass("index_test");
        auto properties = txn.getProperties(classDesc);
        auto indexSchema = txn.getIndexes(classDesc);
        assert(indexSchema.size() == 10);
        for (const auto& property : properties) {
            if (property.name == "index_blob")
                continue;
            auto foundIndex = std::find_if(
                indexSchema.cbegin(), indexSchema.cend(), [&classDesc, &property](const nogdb::IndexDescriptor& index) {
                    return index.propertyId == property.id && index.classId == classDesc.id;
                });
            assert(foundIndex != indexSchema.cend());
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_create_index_extended_class_with_records()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addSubClassOf("index_test", "index_test_2");
        txn.addProperty("index_test_2", "index_text_2", nogdb::PropertyType::TEXT);
        txn.addProperty("index_test_2", "index_int_2", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addVertex("index_test_2",
            nogdb::Record {}
                .set("index_text", "aaa")
                .set("index_tinyint_u", uint8_t { 1 })
                .set("index_tinyint", int8_t { -1 })
                .set("index_smallint_u", uint16_t { 10 })
                .set("index_smallint", int16_t { -10 })
                .set("index_int_u", uint32_t { 100 })
                .set("index_int", int32_t { -100 })
                .set("index_bigint_u", uint64_t { 1000 })
                .set("index_bigint", int64_t { -1000 })
                .set("index_real", 2.0)
                .set("index_text_2", "AAA")
                .set("index_int_2", int32_t { -999 }));
        txn.addVertex("index_test_2",
            nogdb::Record {}
                .set("index_text", "ccc")
                .set("index_tinyint_u", uint8_t { 2 })
                .set("index_tinyint", int8_t { 2 })
                .set("index_smallint_u", uint16_t { 20 })
                .set("index_smallint", int16_t { 20 })
                .set("index_int_u", uint32_t { 200 })
                .set("index_int", int32_t { 200 })
                .set("index_bigint_u", uint64_t { 2000 })
                .set("index_bigint", int64_t { 2000 })
                .set("index_real", 8.4)
                .set("index_text_2", "ZZZ")
                .set("index_int_2", int32_t { 99999 }));
        txn.addVertex("index_test_2",
            nogdb::Record {}
                .set("index_text", "bbb")
                .set("index_tinyint_u", uint8_t { 0 })
                .set("index_tinyint", int8_t { 0 })
                .set("index_smallint_u", uint16_t { 0 })
                .set("index_smallint", int16_t { 0 })
                .set("index_int_u", uint32_t { 0 })
                .set("index_int", int32_t { 0 })
                .set("index_bigint_u", uint64_t { 0 })
                .set("index_bigint", int64_t { 0 })
                .set("index_real", 0.0)
                .set("index_text_2", ".")
                .set("index_int_2", int32_t { 0 }));
        txn.addVertex("index_test_2",
            nogdb::Record {}
                .set("index_text", "bbb")
                .set("index_tinyint_u", uint8_t { 123 })
                .set("index_tinyint", int8_t { 0 })
                .set("index_smallint_u", uint16_t { 123 })
                .set("index_smallint", int16_t { 0 })
                .set("index_int_u", uint32_t { 123123 })
                .set("index_int", int32_t { 0 })
                .set("index_bigint_u", uint64_t { 123123123 })
                .set("index_bigint", int64_t { 0 })
                .set("index_real", 123.123)
                .set("index_text_2", "helloworld")
                .set("index_int_2", int32_t { 0 }));
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addIndex("index_test_2", "index_text_2", true);
        txn.addIndex("index_test_2", "index_int_2", false);
        txn.addIndex("index_test_2", "index_text", false);
        txn.addIndex("index_test_2", "index_tinyint_u", true);
        txn.addIndex("index_test_2", "index_tinyint", false);
        txn.addIndex("index_test_2", "index_smallint_u", true);
        txn.addIndex("index_test_2", "index_smallint", false);
        txn.addIndex("index_test_2", "index_int_u", true);
        txn.addIndex("index_test_2", "index_int", false);
        txn.addIndex("index_test_2", "index_bigint_u", true);
        txn.addIndex("index_test_2", "index_bigint", false);
        txn.addIndex("index_test_2", "index_real", true);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto classDesc = txn.getClass("index_test_2");
        auto properties = txn.getProperties(classDesc);
        auto indexSchema = txn.getIndexes(classDesc);
        assert(indexSchema.size() == 12);
        for (const auto& property : properties) {
            if (property.name == "index_blob")
                continue;
            auto foundIndex = std::find_if(
                indexSchema.cbegin(), indexSchema.cend(), [&classDesc, &property](const nogdb::IndexDescriptor& index) {
                    return index.propertyId == property.id && index.classId == classDesc.id;
                });
            assert(foundIndex != indexSchema.cend());
        }
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_create_invalid_index_with_records()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addSubClassOf("index_test", "index_test_3");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addVertex("index_test_3",
            nogdb::Record {}
                .set("index_text", "aaa")
                .set("index_tinyint_u", uint8_t { 1 })
                .set("index_tinyint", int8_t { -1 })
                .set("index_smallint_u", uint16_t { 10 })
                .set("index_smallint", int16_t { -10 })
                .set("index_int_u", uint32_t { 100 })
                .set("index_int", int32_t { -100 })
                .set("index_bigint_u", uint64_t { 1000 })
                .set("index_bigint", int64_t { -1000 })
                .set("index_real", 2.0));
        txn.addVertex("index_test_3",
            nogdb::Record {}
                .set("index_text", "ccc")
                .set("index_tinyint_u", uint8_t { 2 })
                .set("index_tinyint", int8_t { 2 })
                .set("index_smallint_u", uint16_t { 20 })
                .set("index_smallint", int16_t { 20 })
                .set("index_int_u", uint32_t { 200 })
                .set("index_int", int32_t { 200 })
                .set("index_bigint_u", uint64_t { 2000 })
                .set("index_bigint", int64_t { 2000 })
                .set("index_real", 8.4));
        txn.addVertex("index_test_3",
            nogdb::Record {}
                .set("index_text", "aaa")
                .set("index_tinyint_u", uint8_t { 1 })
                .set("index_tinyint", int8_t { -1 })
                .set("index_smallint_u", uint16_t { 10 })
                .set("index_smallint", int16_t { -10 })
                .set("index_int_u", uint32_t { 100 })
                .set("index_int", int32_t { -100 })
                .set("index_bigint_u", uint64_t { 1000 })
                .set("index_bigint", int64_t { -1000 })
                .set("index_real", 2.0));
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.addIndex("index_test_3", "index_text", true);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_INDEX_CONSTRAINT, "NOGDB_CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        txn.addIndex("index_test_3", "index_tinyint_u", true);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_INDEX_CONSTRAINT, "NOGDB_CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        txn.addIndex("index_test_3", "index_tinyint", true);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_INDEX_CONSTRAINT, "NOGDB_CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        txn.addIndex("index_test_3", "index_smallint_u", true);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_INDEX_CONSTRAINT, "NOGDB_CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        txn.addIndex("index_test_3", "index_smallint", true);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_INDEX_CONSTRAINT, "NOGDB_CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        txn.addIndex("index_test_3", "index_int_u", true);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_INDEX_CONSTRAINT, "NOGDB_CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        txn.addIndex("index_test_3", "index_int", true);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_INDEX_CONSTRAINT, "NOGDB_CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        txn.addIndex("index_test_3", "index_bigint_u", true);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_INDEX_CONSTRAINT, "NOGDB_CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        txn.addIndex("index_test_3", "index_bigint", true);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_INDEX_CONSTRAINT, "NOGDB_CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        txn.addIndex("index_test_3", "index_real", true);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_INDEX_CONSTRAINT, "NOGDB_CTX_INVALID_INDEX_CONSTRAINT");
    }
    txn.rollback();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("index_test_3");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_drop_index_with_records()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("index_test", "index_text");
        txn.dropIndex("index_test", "index_tinyint_u");
        txn.dropIndex("index_test", "index_tinyint");
        txn.dropIndex("index_test", "index_smallint_u");
        txn.dropIndex("index_test", "index_smallint");
        txn.dropIndex("index_test", "index_int_u");
        txn.dropIndex("index_test", "index_int");
        txn.dropIndex("index_test", "index_bigint_u");
        txn.dropIndex("index_test", "index_bigint");
        txn.dropIndex("index_test", "index_real");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto classDesc = txn.getClass("index_test");
        auto properties = txn.getProperties(classDesc);
        auto indexSchema = txn.getIndexes(classDesc);
        assert(indexSchema.size() == 0);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_drop_index_extended_class_with_records()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("index_test_2", "index_int_2");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        txn.getIndex("index_test_2", "index_int_2");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_INDEX, "NOGDB_CTX_NOEXST_INDEX");
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("index_test_2", "index_text");
        txn.dropIndex("index_test_2", "index_tinyint_u");
        txn.dropIndex("index_test_2", "index_tinyint");
        txn.dropIndex("index_test_2", "index_smallint_u");
        txn.dropIndex("index_test_2", "index_smallint");
        txn.dropIndex("index_test_2", "index_int_u");
        txn.dropIndex("index_test_2", "index_int");
        txn.dropIndex("index_test_2", "index_bigint_u");
        txn.dropIndex("index_test_2", "index_bigint");
        txn.dropIndex("index_test_2", "index_real");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto classDesc = txn.getClass("index_test");
        auto properties = txn.getProperties(classDesc);
        auto indexSchema = txn.getIndexes(classDesc);
        assert(indexSchema.size() == 0);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_drop_invalid_index_with_records()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.dropIndex("index_test", "index_text_x");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }

    try {
        txn.dropIndex("index_test_2", "index_text_x");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }

    try {
        txn.dropIndex("index_test_3", "index_text");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    try {
        txn.dropIndex("index_test", "index_text");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_INDEX, "NOGDB_CTX_NOEXST_INDEX");
    }

    try {
        txn.dropIndex("index_test_2", "index_text");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_INDEX, "NOGDB_CTX_NOEXST_INDEX");
    }

    try {
        txn.dropIndex("index_test_2", "index_int_2");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_INDEX, "NOGDB_CTX_NOEXST_INDEX");
    }

    try {
        txn.dropProperty("index_test_2", "index_text_2");
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_IN_USED_PROPERTY, "NOGDB_CTX_IN_USED_PROPERTY");
    }

    try {
        txn.dropClass("index_test_2");
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_IN_USED_PROPERTY, "NOGDB_CTX_IN_USED_PROPERTY");
    }
    txn.rollback();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("index_test_2", "index_text_2");
        txn.dropClass("index_test_2");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    destroy_vertex_index_test();
}

void test_search_by_index_unique_condition()
{
    init_vertex_index_test();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addIndex("index_test", "index_text", true);
        txn.addIndex("index_test", "index_tinyint_u", true);
        txn.addIndex("index_test", "index_tinyint", true);
        txn.addIndex("index_test", "index_smallint_u", true);
        txn.addIndex("index_test", "index_smallint", true);
        txn.addIndex("index_test", "index_int_u", true);
        txn.addIndex("index_test", "index_int", true);
        txn.addIndex("index_test", "index_bigint_u", true);
        txn.addIndex("index_test", "index_bigint", true);
        txn.addIndex("index_test", "index_real", true);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    nogdb::RecordDescriptor rdesc1, rdesc2, rdesc3, rdesc4;
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        rdesc1 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "abcdefghijklmnopqrstuvwxyz")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() - uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::max() - int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() - uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::max() - int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() - uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::max() - int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() - uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::max() - int64_t { 1 })
                .set("index_real", 12345.6789));
        rdesc2 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "0123456789")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::min() + uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::min() + int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::min() + uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::min() + int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::min() + uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::min() + int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::min() + uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::min() + int64_t { 1 })
                .set("index_real", -12345.6789));
        rdesc3 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "__lib_c++__")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 2)
                .set("index_tinyint", int8_t { 0 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 2)
                .set("index_smallint", int16_t { 0 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 2)
                .set("index_int", int32_t { 0 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 2)
                .set("index_bigint", int64_t { 0 })
                .set("index_real", 1.001));
        rdesc4 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "Hello, World")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 4)
                .set("index_tinyint", int8_t { -2 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 4)
                .set("index_smallint", int16_t { -2 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 4)
                .set("index_int", int32_t { -2 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 4)
                .set("index_bigint", int64_t { -2 })
                .set("index_real", -0.001));
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    indexConditionTester<std::string>(ctx, "index_test", "index_text", rdesc2, "0123456789", rdesc4, "Hello, World",
        rdesc3, "__lib_c++__", rdesc1, "abcdefghijklmnopqrstuvwxyz");
    indexConditionTester<unsigned char>(ctx, "index_test", "index_tinyint_u", rdesc2,
        std::numeric_limits<uint8_t>::min() + uint8_t { 1 }, rdesc4, std::numeric_limits<uint8_t>::max() / 4, rdesc3,
        std::numeric_limits<uint8_t>::max() / 2, rdesc1, std::numeric_limits<uint8_t>::max() - uint8_t { 1 });
    indexConditionTester<int8_t>(ctx, "index_test", "index_tinyint", rdesc2,
        std::numeric_limits<int8_t>::min() + int8_t { 1 }, rdesc4, int8_t { -2 }, rdesc3, int8_t { 0 }, rdesc1,
        std::numeric_limits<int8_t>::max() - int8_t { 1 });
    indexConditionTester<unsigned short>(ctx, "index_test", "index_smallint_u", rdesc2,
        std::numeric_limits<uint16_t>::min() + uint16_t { 1 }, rdesc4, std::numeric_limits<uint16_t>::max() / 4, rdesc3,
        std::numeric_limits<uint16_t>::max() / 2, rdesc1, std::numeric_limits<uint16_t>::max() - uint16_t { 1 });
    indexConditionTester<int16_t>(ctx, "index_test", "index_smallint", rdesc2,
        std::numeric_limits<int16_t>::min() + int16_t { 1 }, rdesc4, int16_t { -2 }, rdesc3, int16_t { 0 }, rdesc1,
        std::numeric_limits<int16_t>::max() - int16_t { 1 });
    indexConditionTester(ctx, "index_test", "index_int_u", rdesc2,
        std::numeric_limits<uint32_t>::min() + uint32_t { 1 }, rdesc4, std::numeric_limits<uint32_t>::max() / 4, rdesc3,
        std::numeric_limits<uint32_t>::max() / 2, rdesc1, std::numeric_limits<uint32_t>::max() - uint32_t { 1 });
    indexConditionTester(ctx, "index_test", "index_int", rdesc2, std::numeric_limits<int32_t>::min() + int32_t { 1 },
        rdesc4, int32_t { -2 }, rdesc3, int32_t { 0 }, rdesc1, std::numeric_limits<int32_t>::max() - int32_t { 1 });
    indexConditionTester(ctx, "index_test", "index_bigint_u", rdesc2,
        std::numeric_limits<uint64_t>::min() + uint64_t { 1 }, rdesc4, std::numeric_limits<uint64_t>::max() / 4, rdesc3,
        std::numeric_limits<uint64_t>::max() / 2, rdesc1, std::numeric_limits<uint64_t>::max() - uint64_t { 1 });
    indexConditionTester(ctx, "index_test", "index_bigint", rdesc2, std::numeric_limits<int64_t>::min() + int64_t { 1 },
        rdesc4, int64_t { -2 }, rdesc3, int64_t { 0 }, rdesc1, std::numeric_limits<int64_t>::max() - int64_t { 1 });
    indexConditionTester(
        ctx, "index_test", "index_real", rdesc2, -12345.6789, rdesc4, -0.001, rdesc3, 1.001, rdesc1, 12345.6789);

    indexAdjacentConditionTester<unsigned char>(ctx, "index_test", "index_tinyint_u", rdesc2,
        std::numeric_limits<uint8_t>::min() + uint8_t { 1 }, rdesc4, std::numeric_limits<uint8_t>::max() / 4, rdesc3,
        std::numeric_limits<uint8_t>::max() / 2, rdesc1, std::numeric_limits<uint8_t>::max() - uint8_t { 1 });
    indexAdjacentConditionTester<int8_t>(ctx, "index_test", "index_tinyint", rdesc2,
        std::numeric_limits<int8_t>::min() + int8_t { 1 }, rdesc4, int8_t { -2 }, rdesc3, int8_t { 0 }, rdesc1,
        std::numeric_limits<int8_t>::max() - int8_t { 1 });
    indexAdjacentConditionTester<unsigned short>(ctx, "index_test", "index_smallint_u", rdesc2,
        std::numeric_limits<uint16_t>::min() + uint16_t { 1 }, rdesc4, std::numeric_limits<uint16_t>::max() / 4, rdesc3,
        std::numeric_limits<uint16_t>::max() / 2, rdesc1, std::numeric_limits<uint16_t>::max() - uint16_t { 1 });
    indexAdjacentConditionTester<int16_t>(ctx, "index_test", "index_smallint", rdesc2,
        std::numeric_limits<int16_t>::min() + int16_t { 1 }, rdesc4, int16_t { -2 }, rdesc3, int16_t { 0 }, rdesc1,
        std::numeric_limits<int16_t>::max() - int16_t { 1 });
    indexAdjacentConditionTester(ctx, "index_test", "index_int_u", rdesc2,
        std::numeric_limits<uint32_t>::min() + uint32_t { 1 }, rdesc4, std::numeric_limits<uint32_t>::max() / 4, rdesc3,
        std::numeric_limits<uint32_t>::max() / 2, rdesc1, std::numeric_limits<uint32_t>::max() - uint32_t { 1 });
    indexAdjacentConditionTester(ctx, "index_test", "index_int", rdesc2,
        std::numeric_limits<int32_t>::min() + int32_t { 1 }, rdesc4, int32_t { -2 }, rdesc3, int32_t { 0 }, rdesc1,
        std::numeric_limits<int32_t>::max() - int32_t { 1 });
    indexAdjacentConditionTester(ctx, "index_test", "index_bigint_u", rdesc2,
        std::numeric_limits<uint64_t>::min() + uint64_t { 1 }, rdesc4, std::numeric_limits<uint64_t>::max() / 4, rdesc3,
        std::numeric_limits<uint64_t>::max() / 2, rdesc1, std::numeric_limits<uint64_t>::max() - uint64_t { 1 });
    indexAdjacentConditionTester(ctx, "index_test", "index_bigint", rdesc2,
        std::numeric_limits<int64_t>::min() + int64_t { 1 }, rdesc4, int64_t { -2 }, rdesc3, int64_t { 0 }, rdesc1,
        std::numeric_limits<int64_t>::max() - int64_t { 1 });
    indexAdjacentConditionTester(
        ctx, "index_test", "index_real", rdesc2, -12345.6789, rdesc4, -0.001, rdesc3, 1.001, rdesc1, 12345.6789);

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("index_test", "index_text");
        txn.dropIndex("index_test", "index_tinyint_u");
        txn.dropIndex("index_test", "index_tinyint");
        txn.dropIndex("index_test", "index_smallint_u");
        txn.dropIndex("index_test", "index_smallint");
        txn.dropIndex("index_test", "index_int_u");
        txn.dropIndex("index_test", "index_int");
        txn.dropIndex("index_test", "index_bigint_u");
        txn.dropIndex("index_test", "index_bigint");
        txn.dropIndex("index_test", "index_real");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    destroy_vertex_index_test();
}

void test_search_by_index_non_unique_condition()
{
    init_vertex_index_test();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addIndex("index_test", "index_text", false);
        txn.addIndex("index_test", "index_tinyint_u", false);
        txn.addIndex("index_test", "index_tinyint", false);
        txn.addIndex("index_test", "index_smallint_u", false);
        txn.addIndex("index_test", "index_smallint", false);
        txn.addIndex("index_test", "index_int_u", false);
        txn.addIndex("index_test", "index_int", false);
        txn.addIndex("index_test", "index_bigint_u", false);
        txn.addIndex("index_test", "index_bigint", false);
        txn.addIndex("index_test", "index_real", false);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    nogdb::RecordDescriptor rdesc11, rdesc12, rdesc21, rdesc22, rdesc31, rdesc32, rdesc41, rdesc42;
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        rdesc11 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "abcdefghijklmnopqrstuvwxyz")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() - uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::max() - int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() - uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::max() - int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() - uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::max() - int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() - uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::max() - int64_t { 1 })
                .set("index_real", 12345.6789));
        rdesc21 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "0123456789")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::min() + uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::min() + int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::min() + uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::min() + int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::min() + uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::min() + int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::min() + uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::min() + int64_t { 1 })
                .set("index_real", -12345.6789));
        rdesc31 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "__lib_c++__")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 2)
                .set("index_tinyint", int8_t { 0 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 2)
                .set("index_smallint", int16_t { 0 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 2)
                .set("index_int", int32_t { 0 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 2)
                .set("index_bigint", int64_t { 0 })
                .set("index_real", 1.001));
        rdesc41 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "Hello, World")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 4)
                .set("index_tinyint", int8_t { -2 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 4)
                .set("index_smallint", int16_t { -2 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 4)
                .set("index_int", int32_t { -2 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 4)
                .set("index_bigint", int64_t { -2 })
                .set("index_real", -0.001));
        rdesc12 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "abcdefghijklmnopqrstuvwxyz")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() - uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::max() - int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() - uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::max() - int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() - uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::max() - int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() - uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::max() - int64_t { 1 })
                .set("index_real", 12345.6789));
        rdesc22 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "0123456789")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::min() + uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::min() + int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::min() + uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::min() + int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::min() + uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::min() + int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::min() + uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::min() + int64_t { 1 })
                .set("index_real", -12345.6789));
        rdesc32 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "__lib_c++__")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 2)
                .set("index_tinyint", int8_t { 0 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 2)
                .set("index_smallint", int16_t { 0 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 2)
                .set("index_int", int32_t { 0 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 2)
                .set("index_bigint", int64_t { 0 })
                .set("index_real", 1.001));
        rdesc42 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "Hello, World")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 4)
                .set("index_tinyint", int8_t { -2 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 4)
                .set("index_smallint", int16_t { -2 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 4)
                .set("index_int", int32_t { -2 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 4)
                .set("index_bigint", int64_t { -2 })
                .set("index_real", -0.001));
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    nonUniqueIndexConditionTester<std::string>(ctx, "index_test", "index_text", rdesc21, rdesc22, "0123456789", rdesc41,
        rdesc42, "Hello, World", rdesc31, rdesc32, "__lib_c++__", rdesc11, rdesc12, "abcdefghijklmnopqrstuvwxyz");
    nonUniqueIndexConditionTester<unsigned char>(ctx, "index_test", "index_tinyint_u", rdesc21, rdesc22,
        std::numeric_limits<uint8_t>::min() + uint8_t { 1 }, rdesc41, rdesc42, std::numeric_limits<uint8_t>::max() / 4,
        rdesc31, rdesc32, std::numeric_limits<uint8_t>::max() / 2, rdesc11, rdesc12,
        std::numeric_limits<uint8_t>::max() - uint8_t { 1 });
    nonUniqueIndexConditionTester<int8_t>(ctx, "index_test", "index_tinyint", rdesc21, rdesc22,
        std::numeric_limits<int8_t>::min() + int8_t { 1 }, rdesc41, rdesc42, int8_t { -2 }, rdesc31, rdesc32,
        int8_t { 0 }, rdesc11, rdesc12, std::numeric_limits<int8_t>::max() - int8_t { 1 });
    nonUniqueIndexConditionTester<unsigned short>(ctx, "index_test", "index_smallint_u", rdesc21, rdesc22,
        std::numeric_limits<uint16_t>::min() + uint16_t { 1 }, rdesc41, rdesc42,
        std::numeric_limits<uint16_t>::max() / 4, rdesc31, rdesc32, std::numeric_limits<uint16_t>::max() / 2, rdesc11,
        rdesc12, std::numeric_limits<uint16_t>::max() - uint16_t { 1 });
    nonUniqueIndexConditionTester<int16_t>(ctx, "index_test", "index_smallint", rdesc21, rdesc22,
        std::numeric_limits<int16_t>::min() + int16_t { 1 }, rdesc41, rdesc42, int16_t { -2 }, rdesc31, rdesc32,
        int16_t { 0 }, rdesc11, rdesc12, std::numeric_limits<int16_t>::max() - int16_t { 1 });
    nonUniqueIndexConditionTester(ctx, "index_test", "index_int_u", rdesc21, rdesc22,
        std::numeric_limits<uint32_t>::min() + uint32_t { 1 }, rdesc41, rdesc42,
        std::numeric_limits<uint32_t>::max() / 4, rdesc31, rdesc32, std::numeric_limits<uint32_t>::max() / 2, rdesc11,
        rdesc12, std::numeric_limits<uint32_t>::max() - uint32_t { 1 });
    nonUniqueIndexConditionTester(ctx, "index_test", "index_int", rdesc21, rdesc22,
        std::numeric_limits<int32_t>::min() + int32_t { 1 }, rdesc41, rdesc42, int32_t { -2 }, rdesc31, rdesc32,
        int32_t { 0 }, rdesc11, rdesc12, std::numeric_limits<int32_t>::max() - int32_t { 1 });
    nonUniqueIndexConditionTester(ctx, "index_test", "index_bigint_u", rdesc21, rdesc22,
        std::numeric_limits<uint64_t>::min() + uint64_t { 1 }, rdesc41, rdesc42,
        std::numeric_limits<uint64_t>::max() / 4, rdesc31, rdesc32, std::numeric_limits<uint64_t>::max() / 2, rdesc11,
        rdesc12, std::numeric_limits<uint64_t>::max() - uint64_t { 1 });
    nonUniqueIndexConditionTester(ctx, "index_test", "index_bigint", rdesc21, rdesc22,
        std::numeric_limits<int64_t>::min() + int64_t { 1 }, rdesc41, rdesc42, int64_t { -2 }, rdesc31, rdesc32,
        int64_t { 0 }, rdesc11, rdesc12, std::numeric_limits<int64_t>::max() - int64_t { 1 });
    nonUniqueIndexConditionTester(ctx, "index_test", "index_real", rdesc21, rdesc22, -12345.6789, rdesc41, rdesc42,
        -0.001, rdesc31, rdesc32, 1.001, rdesc11, rdesc12, 12345.6789);

    nonUniqueIndexAdjacentConditionTester<unsigned char>(ctx, "index_test", "index_tinyint_u", rdesc21, rdesc22,
        std::numeric_limits<uint8_t>::min() + uint8_t { 1 }, rdesc41, rdesc42, std::numeric_limits<uint8_t>::max() / 4,
        rdesc31, rdesc32, std::numeric_limits<uint8_t>::max() / 2, rdesc11, rdesc12,
        std::numeric_limits<uint8_t>::max() - uint8_t { 1 });
    nonUniqueIndexAdjacentConditionTester<int8_t>(ctx, "index_test", "index_tinyint", rdesc21, rdesc22,
        std::numeric_limits<int8_t>::min() + int8_t { 1 }, rdesc41, rdesc42, int8_t { -2 }, rdesc31, rdesc32,
        int8_t { 0 }, rdesc11, rdesc12, std::numeric_limits<int8_t>::max() - int8_t { 1 });
    nonUniqueIndexAdjacentConditionTester<unsigned short>(ctx, "index_test", "index_smallint_u", rdesc21, rdesc22,
        std::numeric_limits<uint16_t>::min() + uint16_t { 1 }, rdesc41, rdesc42,
        std::numeric_limits<uint16_t>::max() / 4, rdesc31, rdesc32, std::numeric_limits<uint16_t>::max() / 2, rdesc11,
        rdesc12, std::numeric_limits<uint16_t>::max() - uint16_t { 1 });
    nonUniqueIndexAdjacentConditionTester<int16_t>(ctx, "index_test", "index_smallint", rdesc21, rdesc22,
        std::numeric_limits<int16_t>::min() + int16_t { 1 }, rdesc41, rdesc42, int16_t { -2 }, rdesc31, rdesc32,
        int16_t { 0 }, rdesc11, rdesc12, std::numeric_limits<int16_t>::max() - int16_t { 1 });
    nonUniqueIndexAdjacentConditionTester(ctx, "index_test", "index_int_u", rdesc21, rdesc22,
        std::numeric_limits<uint32_t>::min() + uint32_t { 1 }, rdesc41, rdesc42,
        std::numeric_limits<uint32_t>::max() / 4, rdesc31, rdesc32, std::numeric_limits<uint32_t>::max() / 2, rdesc11,
        rdesc12, std::numeric_limits<uint32_t>::max() - uint32_t { 1 });
    nonUniqueIndexAdjacentConditionTester(ctx, "index_test", "index_int", rdesc21, rdesc22,
        std::numeric_limits<int32_t>::min() + int32_t { 1 }, rdesc41, rdesc42, int32_t { -2 }, rdesc31, rdesc32,
        int32_t { 0 }, rdesc11, rdesc12, std::numeric_limits<int32_t>::max() - int32_t { 1 });
    nonUniqueIndexAdjacentConditionTester(ctx, "index_test", "index_bigint_u", rdesc21, rdesc22,
        std::numeric_limits<uint64_t>::min() + uint64_t { 1 }, rdesc41, rdesc42,
        std::numeric_limits<uint64_t>::max() / 4, rdesc31, rdesc32, std::numeric_limits<uint64_t>::max() / 2, rdesc11,
        rdesc12, std::numeric_limits<uint64_t>::max() - uint64_t { 1 });
    nonUniqueIndexAdjacentConditionTester(ctx, "index_test", "index_bigint", rdesc21, rdesc22,
        std::numeric_limits<int64_t>::min() + int64_t { 1 }, rdesc41, rdesc42, int64_t { -2 }, rdesc31, rdesc32,
        int64_t { 0 }, rdesc11, rdesc12, std::numeric_limits<int64_t>::max() - int64_t { 1 });
    nonUniqueIndexAdjacentConditionTester(ctx, "index_test", "index_real", rdesc21, rdesc22, -12345.6789, rdesc41,
        rdesc42, -0.001, rdesc31, rdesc32, 1.001, rdesc11, rdesc12, 12345.6789);

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("index_test", "index_text");
        txn.dropIndex("index_test", "index_tinyint_u");
        txn.dropIndex("index_test", "index_tinyint");
        txn.dropIndex("index_test", "index_smallint_u");
        txn.dropIndex("index_test", "index_smallint");
        txn.dropIndex("index_test", "index_int_u");
        txn.dropIndex("index_test", "index_int");
        txn.dropIndex("index_test", "index_bigint_u");
        txn.dropIndex("index_test", "index_bigint");
        txn.dropIndex("index_test", "index_real");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    destroy_vertex_index_test();
}

void test_search_by_index_unique_cursor_condition()
{
    init_vertex_index_test();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addIndex("index_test", "index_text", true);
        txn.addIndex("index_test", "index_tinyint_u", true);
        txn.addIndex("index_test", "index_tinyint", true);
        txn.addIndex("index_test", "index_smallint_u", true);
        txn.addIndex("index_test", "index_smallint", true);
        txn.addIndex("index_test", "index_int_u", true);
        txn.addIndex("index_test", "index_int", true);
        txn.addIndex("index_test", "index_bigint_u", true);
        txn.addIndex("index_test", "index_bigint", true);
        txn.addIndex("index_test", "index_real", true);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    nogdb::RecordDescriptor rdesc1, rdesc2, rdesc3, rdesc4;
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        rdesc1 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "abcdefghijklmnopqrstuvwxyz")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() - uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::max() - int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() - uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::max() - int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() - uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::max() - int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() - uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::max() - int64_t { 1 })
                .set("index_real", 12345.6789));
        rdesc2 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "0123456789")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::min() + uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::min() + int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::min() + uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::min() + int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::min() + uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::min() + int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::min() + uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::min() + int64_t { 1 })
                .set("index_real", -12345.6789));
        rdesc3 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "__lib_c++__")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 2)
                .set("index_tinyint", int8_t { 0 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 2)
                .set("index_smallint", int16_t { 0 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 2)
                .set("index_int", int32_t { 0 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 2)
                .set("index_bigint", int64_t { 0 })
                .set("index_real", 1.001));
        rdesc4 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "Hello, World")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 4)
                .set("index_tinyint", int8_t { -2 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 4)
                .set("index_smallint", int16_t { -2 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 4)
                .set("index_int", int32_t { -2 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 4)
                .set("index_bigint", int64_t { -2 })
                .set("index_real", -0.001));
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    indexCursorConditionTester<std::string>(ctx, "index_test", "index_text", rdesc2, "0123456789", rdesc4,
        "Hello, World", rdesc3, "__lib_c++__", rdesc1, "abcdefghijklmnopqrstuvwxyz");
    indexCursorConditionTester<unsigned char>(ctx, "index_test", "index_tinyint_u", rdesc2,
        std::numeric_limits<uint8_t>::min() + uint8_t { 1 }, rdesc4, std::numeric_limits<uint8_t>::max() / 4, rdesc3,
        std::numeric_limits<uint8_t>::max() / 2, rdesc1, std::numeric_limits<uint8_t>::max() - uint8_t { 1 });
    indexCursorConditionTester<int8_t>(ctx, "index_test", "index_tinyint", rdesc2,
        std::numeric_limits<int8_t>::min() + int8_t { 1 }, rdesc4, int8_t { -2 }, rdesc3, int8_t { 0 }, rdesc1,
        std::numeric_limits<int8_t>::max() - int8_t { 1 });
    indexCursorConditionTester<unsigned short>(ctx, "index_test", "index_smallint_u", rdesc2,
        std::numeric_limits<uint16_t>::min() + uint16_t { 1 }, rdesc4, std::numeric_limits<uint16_t>::max() / 4, rdesc3,
        std::numeric_limits<uint16_t>::max() / 2, rdesc1, std::numeric_limits<uint16_t>::max() - uint16_t { 1 });
    indexCursorConditionTester<int16_t>(ctx, "index_test", "index_smallint", rdesc2,
        std::numeric_limits<int16_t>::min() + int16_t { 1 }, rdesc4, int16_t { -2 }, rdesc3, int16_t { 0 }, rdesc1,
        std::numeric_limits<int16_t>::max() - int16_t { 1 });
    indexCursorConditionTester(ctx, "index_test", "index_int_u", rdesc2,
        std::numeric_limits<uint32_t>::min() + uint32_t { 1 }, rdesc4, std::numeric_limits<uint32_t>::max() / 4, rdesc3,
        std::numeric_limits<uint32_t>::max() / 2, rdesc1, std::numeric_limits<uint32_t>::max() - uint32_t { 1 });
    indexCursorConditionTester(ctx, "index_test", "index_int", rdesc2,
        std::numeric_limits<int32_t>::min() + int32_t { 1 }, rdesc4, int32_t { -2 }, rdesc3, int32_t { 0 }, rdesc1,
        std::numeric_limits<int32_t>::max() - int32_t { 1 });
    indexCursorConditionTester(ctx, "index_test", "index_bigint_u", rdesc2,
        std::numeric_limits<uint64_t>::min() + uint64_t { 1 }, rdesc4, std::numeric_limits<uint64_t>::max() / 4, rdesc3,
        std::numeric_limits<uint64_t>::max() / 2, rdesc1, std::numeric_limits<uint64_t>::max() - uint64_t { 1 });
    indexCursorConditionTester(ctx, "index_test", "index_bigint", rdesc2,
        std::numeric_limits<int64_t>::min() + int64_t { 1 }, rdesc4, int64_t { -2 }, rdesc3, int64_t { 0 }, rdesc1,
        std::numeric_limits<int64_t>::max() - int64_t { 1 });
    indexCursorConditionTester(
        ctx, "index_test", "index_real", rdesc2, -12345.6789, rdesc4, -0.001, rdesc3, 1.001, rdesc1, 12345.6789);

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("index_test", "index_text");
        txn.dropIndex("index_test", "index_tinyint_u");
        txn.dropIndex("index_test", "index_tinyint");
        txn.dropIndex("index_test", "index_smallint_u");
        txn.dropIndex("index_test", "index_smallint");
        txn.dropIndex("index_test", "index_int_u");
        txn.dropIndex("index_test", "index_int");
        txn.dropIndex("index_test", "index_bigint_u");
        txn.dropIndex("index_test", "index_bigint");
        txn.dropIndex("index_test", "index_real");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    destroy_vertex_index_test();
}

void test_search_by_index_non_unique_cursor_condition()
{
    init_vertex_index_test();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addIndex("index_test", "index_text", false);
        txn.addIndex("index_test", "index_tinyint_u", false);
        txn.addIndex("index_test", "index_tinyint", false);
        txn.addIndex("index_test", "index_smallint_u", false);
        txn.addIndex("index_test", "index_smallint", false);
        txn.addIndex("index_test", "index_int_u", false);
        txn.addIndex("index_test", "index_int", false);
        txn.addIndex("index_test", "index_bigint_u", false);
        txn.addIndex("index_test", "index_bigint", false);
        txn.addIndex("index_test", "index_real", false);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    nogdb::RecordDescriptor rdesc11, rdesc12, rdesc21, rdesc22, rdesc31, rdesc32, rdesc41, rdesc42;
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        rdesc11 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "abcdefghijklmnopqrstuvwxyz")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() - uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::max() - int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() - uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::max() - int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() - uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::max() - int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() - uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::max() - int64_t { 1 })
                .set("index_real", 12345.6789));
        rdesc21 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "0123456789")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::min() + uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::min() + int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::min() + uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::min() + int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::min() + uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::min() + int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::min() + uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::min() + int64_t { 1 })
                .set("index_real", -12345.6789));
        rdesc31 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "__lib_c++__")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 2)
                .set("index_tinyint", int8_t { 0 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 2)
                .set("index_smallint", int16_t { 0 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 2)
                .set("index_int", int32_t { 0 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 2)
                .set("index_bigint", int64_t { 0 })
                .set("index_real", 1.001));
        rdesc41 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "Hello, World")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 4)
                .set("index_tinyint", int8_t { -2 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 4)
                .set("index_smallint", int16_t { -2 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 4)
                .set("index_int", int32_t { -2 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 4)
                .set("index_bigint", int64_t { -2 })
                .set("index_real", -0.001));
        rdesc12 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "abcdefghijklmnopqrstuvwxyz")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() - uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::max() - int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() - uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::max() - int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() - uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::max() - int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() - uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::max() - int64_t { 1 })
                .set("index_real", 12345.6789));
        rdesc22 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "0123456789")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::min() + uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::min() + int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::min() + uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::min() + int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::min() + uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::min() + int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::min() + uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::min() + int64_t { 1 })
                .set("index_real", -12345.6789));
        rdesc32 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "__lib_c++__")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 2)
                .set("index_tinyint", int8_t { 0 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 2)
                .set("index_smallint", int16_t { 0 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 2)
                .set("index_int", int32_t { 0 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 2)
                .set("index_bigint", int64_t { 0 })
                .set("index_real", 1.001));
        rdesc42 = txn.addVertex("index_test",
            nogdb::Record {}
                .set("index_text", "Hello, World")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 4)
                .set("index_tinyint", int8_t { -2 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 4)
                .set("index_smallint", int16_t { -2 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 4)
                .set("index_int", int32_t { -2 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 4)
                .set("index_bigint", int64_t { -2 })
                .set("index_real", -0.001));
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    nonUniqueIndexCursorConditionTester<std::string>(ctx, "index_test", "index_text", rdesc21, rdesc22, "0123456789",
        rdesc41, rdesc42, "Hello, World", rdesc31, rdesc32, "__lib_c++__", rdesc11, rdesc12,
        "abcdefghijklmnopqrstuvwxyz");
    nonUniqueIndexCursorConditionTester<unsigned char>(ctx, "index_test", "index_tinyint_u", rdesc21, rdesc22,
        std::numeric_limits<uint8_t>::min() + uint8_t { 1 }, rdesc41, rdesc42, std::numeric_limits<uint8_t>::max() / 4,
        rdesc31, rdesc32, std::numeric_limits<uint8_t>::max() / 2, rdesc11, rdesc12,
        std::numeric_limits<uint8_t>::max() - uint8_t { 1 });
    nonUniqueIndexCursorConditionTester<int8_t>(ctx, "index_test", "index_tinyint", rdesc21, rdesc22,
        std::numeric_limits<int8_t>::min() + int8_t { 1 }, rdesc41, rdesc42, int8_t { -2 }, rdesc31, rdesc32,
        int8_t { 0 }, rdesc11, rdesc12, std::numeric_limits<int8_t>::max() - int8_t { 1 });
    nonUniqueIndexCursorConditionTester<unsigned short>(ctx, "index_test", "index_smallint_u", rdesc21, rdesc22,
        std::numeric_limits<uint16_t>::min() + uint16_t { 1 }, rdesc41, rdesc42,
        std::numeric_limits<uint16_t>::max() / 4, rdesc31, rdesc32, std::numeric_limits<uint16_t>::max() / 2, rdesc11,
        rdesc12, std::numeric_limits<uint16_t>::max() - uint16_t { 1 });
    nonUniqueIndexCursorConditionTester<int16_t>(ctx, "index_test", "index_smallint", rdesc21, rdesc22,
        std::numeric_limits<int16_t>::min() + int16_t { 1 }, rdesc41, rdesc42, int16_t { -2 }, rdesc31, rdesc32,
        int16_t { 0 }, rdesc11, rdesc12, std::numeric_limits<int16_t>::max() - int16_t { 1 });
    nonUniqueIndexCursorConditionTester(ctx, "index_test", "index_int_u", rdesc21, rdesc22,
        std::numeric_limits<uint32_t>::min() + uint32_t { 1 }, rdesc41, rdesc42,
        std::numeric_limits<uint32_t>::max() / 4, rdesc31, rdesc32, std::numeric_limits<uint32_t>::max() / 2, rdesc11,
        rdesc12, std::numeric_limits<uint32_t>::max() - uint32_t { 1 });
    nonUniqueIndexCursorConditionTester(ctx, "index_test", "index_int", rdesc21, rdesc22,
        std::numeric_limits<int32_t>::min() + int32_t { 1 }, rdesc41, rdesc42, int32_t { -2 }, rdesc31, rdesc32,
        int32_t { 0 }, rdesc11, rdesc12, std::numeric_limits<int32_t>::max() - int32_t { 1 });
    nonUniqueIndexCursorConditionTester(ctx, "index_test", "index_bigint_u", rdesc21, rdesc22,
        std::numeric_limits<uint64_t>::min() + uint64_t { 1 }, rdesc41, rdesc42,
        std::numeric_limits<uint64_t>::max() / 4, rdesc31, rdesc32, std::numeric_limits<uint64_t>::max() / 2, rdesc11,
        rdesc12, std::numeric_limits<uint64_t>::max() - uint64_t { 1 });
    nonUniqueIndexCursorConditionTester(ctx, "index_test", "index_bigint", rdesc21, rdesc22,
        std::numeric_limits<int64_t>::min() + int64_t { 1 }, rdesc41, rdesc42, int64_t { -2 }, rdesc31, rdesc32,
        int64_t { 0 }, rdesc11, rdesc12, std::numeric_limits<int64_t>::max() - int64_t { 1 });
    nonUniqueIndexCursorConditionTester(ctx, "index_test", "index_real", rdesc21, rdesc22, -12345.6789, rdesc41,
        rdesc42, -0.001, rdesc31, rdesc32, 1.001, rdesc11, rdesc12, 12345.6789);

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("index_test", "index_text");
        txn.dropIndex("index_test", "index_tinyint_u");
        txn.dropIndex("index_test", "index_tinyint");
        txn.dropIndex("index_test", "index_smallint_u");
        txn.dropIndex("index_test", "index_smallint");
        txn.dropIndex("index_test", "index_int_u");
        txn.dropIndex("index_test", "index_int");
        txn.dropIndex("index_test", "index_bigint_u");
        txn.dropIndex("index_test", "index_bigint");
        txn.dropIndex("index_test", "index_real");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    destroy_vertex_index_test();
}

void test_search_by_index_extended_class_condition()
{
    init_vertex_index_test();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addSubClassOf("index_test", "index_test2");
        txn.addIndex("index_test2", "index_text", true);
        txn.addIndex("index_test2", "index_tinyint_u", false);
        txn.addIndex("index_test2", "index_tinyint", true);
        txn.addIndex("index_test2", "index_smallint_u", false);
        txn.addIndex("index_test2", "index_smallint", true);
        txn.addIndex("index_test2", "index_int_u", false);
        txn.addIndex("index_test2", "index_int", true);
        txn.addIndex("index_test2", "index_bigint_u", false);
        txn.addIndex("index_test2", "index_bigint", true);
        txn.addIndex("index_test2", "index_real", false);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    nogdb::RecordDescriptor rdesc1, rdesc2, rdesc3, rdesc4;
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        rdesc1 = txn.addVertex("index_test2",
            nogdb::Record {}
                .set("index_text", "abcdefghijklmnopqrstuvwxyz")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() - uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::max() - int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() - uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::max() - int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() - uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::max() - int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() - uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::max() - int64_t { 1 })
                .set("index_real", 12345.6789));
        rdesc2 = txn.addVertex("index_test2",
            nogdb::Record {}
                .set("index_text", "0123456789")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::min() + uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::min() + int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::min() + uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::min() + int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::min() + uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::min() + int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::min() + uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::min() + int64_t { 1 })
                .set("index_real", -12345.6789));
        rdesc3 = txn.addVertex("index_test2",
            nogdb::Record {}
                .set("index_text", "__lib_c++__")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 2)
                .set("index_tinyint", int8_t { 0 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 2)
                .set("index_smallint", int16_t { 0 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 2)
                .set("index_int", int32_t { 0 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 2)
                .set("index_bigint", int64_t { 0 })
                .set("index_real", 1.001));
        rdesc4 = txn.addVertex("index_test2",
            nogdb::Record {}
                .set("index_text", "Hello, World")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 4)
                .set("index_tinyint", int8_t { -2 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 4)
                .set("index_smallint", int16_t { -2 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 4)
                .set("index_int", int32_t { -2 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 4)
                .set("index_bigint", int64_t { -2 })
                .set("index_real", -0.001));
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    indexConditionTester<std::string>(ctx, "index_test2", "index_text", rdesc2, "0123456789", rdesc4, "Hello, World",
        rdesc3, "__lib_c++__", rdesc1, "abcdefghijklmnopqrstuvwxyz");
    indexConditionTester<unsigned char>(ctx, "index_test2", "index_tinyint_u", rdesc2,
        std::numeric_limits<uint8_t>::min() + uint8_t { 1 }, rdesc4, std::numeric_limits<uint8_t>::max() / 4, rdesc3,
        std::numeric_limits<uint8_t>::max() / 2, rdesc1, std::numeric_limits<uint8_t>::max() - uint8_t { 1 });
    indexConditionTester<int8_t>(ctx, "index_test2", "index_tinyint", rdesc2,
        std::numeric_limits<int8_t>::min() + int8_t { 1 }, rdesc4, int8_t { -2 }, rdesc3, int8_t { 0 }, rdesc1,
        std::numeric_limits<int8_t>::max() - int8_t { 1 });
    indexConditionTester<unsigned short>(ctx, "index_test2", "index_smallint_u", rdesc2,
        std::numeric_limits<uint16_t>::min() + uint16_t { 1 }, rdesc4, std::numeric_limits<uint16_t>::max() / 4, rdesc3,
        std::numeric_limits<uint16_t>::max() / 2, rdesc1, std::numeric_limits<uint16_t>::max() - uint16_t { 1 });
    indexConditionTester<int16_t>(ctx, "index_test2", "index_smallint", rdesc2,
        std::numeric_limits<int16_t>::min() + int16_t { 1 }, rdesc4, int16_t { -2 }, rdesc3, int16_t { 0 }, rdesc1,
        std::numeric_limits<int16_t>::max() - int16_t { 1 });
    indexConditionTester(ctx, "index_test2", "index_int_u", rdesc2,
        std::numeric_limits<uint32_t>::min() + uint32_t { 1 }, rdesc4, std::numeric_limits<uint32_t>::max() / 4, rdesc3,
        std::numeric_limits<uint32_t>::max() / 2, rdesc1, std::numeric_limits<uint32_t>::max() - uint32_t { 1 });
    indexConditionTester(ctx, "index_test2", "index_int", rdesc2, std::numeric_limits<int32_t>::min() + int32_t { 1 },
        rdesc4, int32_t { -2 }, rdesc3, int32_t { 0 }, rdesc1, std::numeric_limits<int32_t>::max() - int32_t { 1 });
    indexConditionTester(ctx, "index_test2", "index_bigint_u", rdesc2,
        std::numeric_limits<uint64_t>::min() + uint64_t { 1 }, rdesc4, std::numeric_limits<uint64_t>::max() / 4, rdesc3,
        std::numeric_limits<uint64_t>::max() / 2, rdesc1, std::numeric_limits<uint64_t>::max() - uint64_t { 1 });
    indexConditionTester(ctx, "index_test2", "index_bigint", rdesc2,
        std::numeric_limits<int64_t>::min() + int64_t { 1 }, rdesc4, int64_t { -2 }, rdesc3, int64_t { 0 }, rdesc1,
        std::numeric_limits<int64_t>::max() - int64_t { 1 });
    indexConditionTester(
        ctx, "index_test2", "index_real", rdesc2, -12345.6789, rdesc4, -0.001, rdesc3, 1.001, rdesc1, 12345.6789);

    emptyIndexConditionTester<std::string>(ctx, "index_test", "index_text", rdesc2, "0123456789", rdesc4,
        "Hello, World", rdesc3, "__lib_c++__", rdesc1, "abcdefghijklmnopqrstuvwxyz");
    emptyIndexConditionTester<unsigned char>(ctx, "index_test", "index_tinyint_u", rdesc2,
        std::numeric_limits<uint8_t>::min() + uint8_t { 1 }, rdesc4, std::numeric_limits<uint8_t>::max() / 4, rdesc3,
        std::numeric_limits<uint8_t>::max() / 2, rdesc1, std::numeric_limits<uint8_t>::max() - uint8_t { 1 });
    emptyIndexConditionTester<int8_t>(ctx, "index_test", "index_tinyint", rdesc2,
        std::numeric_limits<int8_t>::min() + int8_t { 1 }, rdesc4, int8_t { -2 }, rdesc3, int8_t { 0 }, rdesc1,
        std::numeric_limits<int8_t>::max() - int8_t { 1 });
    emptyIndexConditionTester<unsigned short>(ctx, "index_test", "index_smallint_u", rdesc2,
        std::numeric_limits<uint16_t>::min() + uint16_t { 1 }, rdesc4, std::numeric_limits<uint16_t>::max() / 4, rdesc3,
        std::numeric_limits<uint16_t>::max() / 2, rdesc1, std::numeric_limits<uint16_t>::max() - uint16_t { 1 });
    emptyIndexConditionTester<int16_t>(ctx, "index_test", "index_smallint", rdesc2,
        std::numeric_limits<int16_t>::min() + int16_t { 1 }, rdesc4, int16_t { -2 }, rdesc3, int16_t { 0 }, rdesc1,
        std::numeric_limits<int16_t>::max() - int16_t { 1 });
    emptyIndexConditionTester(ctx, "index_test", "index_int_u", rdesc2,
        std::numeric_limits<uint32_t>::min() + uint32_t { 1 }, rdesc4, std::numeric_limits<uint32_t>::max() / 4, rdesc3,
        std::numeric_limits<uint32_t>::max() / 2, rdesc1, std::numeric_limits<uint32_t>::max() - uint32_t { 1 });
    emptyIndexConditionTester(ctx, "index_test", "index_int", rdesc2,
        std::numeric_limits<int32_t>::min() + int32_t { 1 }, rdesc4, int32_t { -2 }, rdesc3, int32_t { 0 }, rdesc1,
        std::numeric_limits<int32_t>::max() - int32_t { 1 });
    emptyIndexConditionTester(ctx, "index_test", "index_bigint_u", rdesc2,
        std::numeric_limits<uint64_t>::min() + uint64_t { 1 }, rdesc4, std::numeric_limits<uint64_t>::max() / 4, rdesc3,
        std::numeric_limits<uint64_t>::max() / 2, rdesc1, std::numeric_limits<uint64_t>::max() - uint64_t { 1 });
    emptyIndexConditionTester(ctx, "index_test", "index_bigint", rdesc2,
        std::numeric_limits<int64_t>::min() + int64_t { 1 }, rdesc4, int64_t { -2 }, rdesc3, int64_t { 0 }, rdesc1,
        std::numeric_limits<int64_t>::max() - int64_t { 1 });
    emptyIndexConditionTester(
        ctx, "index_test", "index_real", rdesc2, -12345.6789, rdesc4, -0.001, rdesc3, 1.001, rdesc1, 12345.6789);

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("index_test2", "index_text");
        txn.dropIndex("index_test2", "index_tinyint_u");
        txn.dropIndex("index_test2", "index_tinyint");
        txn.dropIndex("index_test2", "index_smallint_u");
        txn.dropIndex("index_test2", "index_smallint");
        txn.dropIndex("index_test2", "index_int_u");
        txn.dropIndex("index_test2", "index_int");
        txn.dropIndex("index_test2", "index_bigint_u");
        txn.dropIndex("index_test2", "index_bigint");
        txn.dropIndex("index_test2", "index_real");
        txn.dropClass("index_test2");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    destroy_vertex_index_test();
}

void test_search_by_index_extended_class_cursor_condition()
{
    init_vertex_index_test();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addSubClassOf("index_test", "index_test2");
        txn.addIndex("index_test2", "index_text", false);
        txn.addIndex("index_test2", "index_tinyint_u", true);
        txn.addIndex("index_test2", "index_tinyint", false);
        txn.addIndex("index_test2", "index_smallint_u", true);
        txn.addIndex("index_test2", "index_smallint", false);
        txn.addIndex("index_test2", "index_int_u", true);
        txn.addIndex("index_test2", "index_int", false);
        txn.addIndex("index_test2", "index_bigint_u", true);
        txn.addIndex("index_test2", "index_bigint", false);
        txn.addIndex("index_test2", "index_real", true);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    nogdb::RecordDescriptor rdesc1, rdesc2, rdesc3, rdesc4;
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        rdesc1 = txn.addVertex("index_test2",
            nogdb::Record {}
                .set("index_text", "abcdefghijklmnopqrstuvwxyz")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() - uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::max() - int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() - uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::max() - int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() - uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::max() - int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() - uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::max() - int64_t { 1 })
                .set("index_real", 12345.6789));
        rdesc2 = txn.addVertex("index_test2",
            nogdb::Record {}
                .set("index_text", "0123456789")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::min() + uint8_t { 1 })
                .set("index_tinyint", std::numeric_limits<int8_t>::min() + int8_t { 1 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::min() + uint16_t { 1 })
                .set("index_smallint", std::numeric_limits<int16_t>::min() + int16_t { 1 })
                .set("index_int_u", std::numeric_limits<uint32_t>::min() + uint32_t { 1 })
                .set("index_int", std::numeric_limits<int32_t>::min() + int32_t { 1 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::min() + uint64_t { 1 })
                .set("index_bigint", std::numeric_limits<int64_t>::min() + int64_t { 1 })
                .set("index_real", -12345.6789));
        rdesc3 = txn.addVertex("index_test2",
            nogdb::Record {}
                .set("index_text", "__lib_c++__")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 2)
                .set("index_tinyint", int8_t { 0 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 2)
                .set("index_smallint", int16_t { 0 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 2)
                .set("index_int", int32_t { 0 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 2)
                .set("index_bigint", int64_t { 0 })
                .set("index_real", 1.001));
        rdesc4 = txn.addVertex("index_test2",
            nogdb::Record {}
                .set("index_text", "Hello, World")
                .set("index_tinyint_u", std::numeric_limits<uint8_t>::max() / 4)
                .set("index_tinyint", int8_t { -2 })
                .set("index_smallint_u", std::numeric_limits<uint16_t>::max() / 4)
                .set("index_smallint", int16_t { -2 })
                .set("index_int_u", std::numeric_limits<uint32_t>::max() / 4)
                .set("index_int", int32_t { -2 })
                .set("index_bigint_u", std::numeric_limits<uint64_t>::max() / 4)
                .set("index_bigint", int64_t { -2 })
                .set("index_real", -0.001));
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    indexCursorConditionTester<std::string>(ctx, "index_test2", "index_text", rdesc2, "0123456789", rdesc4,
        "Hello, World", rdesc3, "__lib_c++__", rdesc1, "abcdefghijklmnopqrstuvwxyz");
    indexCursorConditionTester<unsigned char>(ctx, "index_test2", "index_tinyint_u", rdesc2,
        std::numeric_limits<uint8_t>::min() + uint8_t { 1 }, rdesc4, std::numeric_limits<uint8_t>::max() / 4, rdesc3,
        std::numeric_limits<uint8_t>::max() / 2, rdesc1, std::numeric_limits<uint8_t>::max() - uint8_t { 1 });
    indexCursorConditionTester<int8_t>(ctx, "index_test2", "index_tinyint", rdesc2,
        std::numeric_limits<int8_t>::min() + int8_t { 1 }, rdesc4, int8_t { -2 }, rdesc3, int8_t { 0 }, rdesc1,
        std::numeric_limits<int8_t>::max() - int8_t { 1 });
    indexCursorConditionTester<unsigned short>(ctx, "index_test2", "index_smallint_u", rdesc2,
        std::numeric_limits<uint16_t>::min() + uint16_t { 1 }, rdesc4, std::numeric_limits<uint16_t>::max() / 4, rdesc3,
        std::numeric_limits<uint16_t>::max() / 2, rdesc1, std::numeric_limits<uint16_t>::max() - uint16_t { 1 });
    indexCursorConditionTester<int16_t>(ctx, "index_test2", "index_smallint", rdesc2,
        std::numeric_limits<int16_t>::min() + int16_t { 1 }, rdesc4, int16_t { -2 }, rdesc3, int16_t { 0 }, rdesc1,
        std::numeric_limits<int16_t>::max() - int16_t { 1 });
    indexCursorConditionTester(ctx, "index_test2", "index_int_u", rdesc2,
        std::numeric_limits<uint32_t>::min() + uint32_t { 1 }, rdesc4, std::numeric_limits<uint32_t>::max() / 4, rdesc3,
        std::numeric_limits<uint32_t>::max() / 2, rdesc1, std::numeric_limits<uint32_t>::max() - uint32_t { 1 });
    indexCursorConditionTester(ctx, "index_test2", "index_int", rdesc2,
        std::numeric_limits<int32_t>::min() + int32_t { 1 }, rdesc4, int32_t { -2 }, rdesc3, int32_t { 0 }, rdesc1,
        std::numeric_limits<int32_t>::max() - int32_t { 1 });
    indexCursorConditionTester(ctx, "index_test2", "index_bigint_u", rdesc2,
        std::numeric_limits<uint64_t>::min() + uint64_t { 1 }, rdesc4, std::numeric_limits<uint64_t>::max() / 4, rdesc3,
        std::numeric_limits<uint64_t>::max() / 2, rdesc1, std::numeric_limits<uint64_t>::max() - uint64_t { 1 });
    indexCursorConditionTester(ctx, "index_test2", "index_bigint", rdesc2,
        std::numeric_limits<int64_t>::min() + int64_t { 1 }, rdesc4, int64_t { -2 }, rdesc3, int64_t { 0 }, rdesc1,
        std::numeric_limits<int64_t>::max() - int64_t { 1 });
    indexCursorConditionTester(
        ctx, "index_test2", "index_real", rdesc2, -12345.6789, rdesc4, -0.001, rdesc3, 1.001, rdesc1, 12345.6789);

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("index_test2", "index_text");
        txn.dropIndex("index_test2", "index_tinyint_u");
        txn.dropIndex("index_test2", "index_tinyint");
        txn.dropIndex("index_test2", "index_smallint_u");
        txn.dropIndex("index_test2", "index_smallint");
        txn.dropIndex("index_test2", "index_int_u");
        txn.dropIndex("index_test2", "index_int");
        txn.dropIndex("index_test2", "index_bigint_u");
        txn.dropIndex("index_test2", "index_bigint");
        txn.dropIndex("index_test2", "index_real");
        txn.dropClass("index_test2");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    destroy_vertex_index_test();
}

void test_search_by_index_unique_multicondition()
{
    // TODO
}

void test_search_by_index_non_unique_multicondition()
{
    // TODO
}

void test_search_by_index_unique_cursor_multicondition()
{
    // TODO
}

void test_search_by_index_non_unique_cursor_multicondition()
{
    // TODO
}

void test_search_by_index_extended_class_multicondition()
{
    // TODO
}

void test_search_by_index_extended_class_cursor_multicondition()
{
    // TODO
}
