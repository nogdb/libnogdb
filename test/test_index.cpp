/*
 *  Copyright (C) 2018, Throughwave (Thailand) Co., Ltd.
 *  <peerawich at throughwave dot co dot th>
 *
 *  This file is part of libnogdb, the NogDB core library in C++.
 *
 *  libnogdb is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "runtest.h"
#include "test_exec.h"

void test_create_index() {
    init_vertex_index_test();

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Property::createIndex(txn, "index_test", "index_text", true);
        nogdb::Property::createIndex(txn, "index_test", "index_tinyint_u", false);
        nogdb::Property::createIndex(txn, "index_test", "index_tinyint", true);
        nogdb::Property::createIndex(txn, "index_test", "index_smallint_u", false);
        nogdb::Property::createIndex(txn, "index_test", "index_smallint", true);
        nogdb::Property::createIndex(txn, "index_test", "index_int_u", false);
        nogdb::Property::createIndex(txn, "index_test", "index_int", true);
        nogdb::Property::createIndex(txn, "index_test", "index_bigint_u", false);
        nogdb::Property::createIndex(txn, "index_test", "index_bigint", true);
        nogdb::Property::createIndex(txn, "index_test", "index_real", false);
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto schema = nogdb::Db::getSchema(txn, "index_test");
        for(const auto& property: schema.properties) {
            if (property.first != "index_blob") {
                assert(property.second.indexInfo.size() == 1);
            }
        }
        txn.rollback();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

}

void test_create_index_extended_class() {

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::createExtend(txn, "index_test_2", "index_test");
        nogdb::Property::add(txn, "index_test_2", "index_text_2", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "index_test_2", "index_int_2", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Property::createIndex(txn, "index_test_2", "index_text", true);
        nogdb::Property::createIndex(txn, "index_test_2", "index_tinyint_u", false);
        nogdb::Property::createIndex(txn, "index_test_2", "index_tinyint", true);
        nogdb::Property::createIndex(txn, "index_test_2", "index_smallint_u", false);
        nogdb::Property::createIndex(txn, "index_test_2", "index_smallint", true);
        nogdb::Property::createIndex(txn, "index_test_2", "index_int_u", false);
        nogdb::Property::createIndex(txn, "index_test_2", "index_int", true);
        nogdb::Property::createIndex(txn, "index_test_2", "index_bigint_u", false);
        nogdb::Property::createIndex(txn, "index_test_2", "index_bigint", true);
        nogdb::Property::createIndex(txn, "index_test_2", "index_real", false);
        nogdb::Property::createIndex(txn, "index_test_2", "index_text_2", true);
        nogdb::Property::createIndex(txn, "index_test_2", "index_int_2", false);
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto schema = nogdb::Db::getSchema(txn, "index_test");
        for(const auto& property: schema.properties) {
            if (property.first != "index_blob") {
                assert(property.second.indexInfo.size() == 2);
            }
        }
        schema = nogdb::Db::getSchema(txn, "index_test_2");
        for(const auto& property: schema.properties) {
            if (property.first != "index_blob") {
                if (property.first == "index_text_2" || property.first == "index_int_2") {
                    assert(property.second.indexInfo.size() == 1);
                } else {
                    assert(property.second.indexInfo.size() == 2);
                }
            }
        }
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

}

void test_create_invalid_index() {

    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Property::createIndex(txn, "index_test", "index_blob", true);
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_INVALID_PROPTYPE_INDEX, "CTX_INVALID_PROPTYPE_INDEX");
    }

    try {
        nogdb::Property::createIndex(txn, "index_test", "index_text_2", false);
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_PROPERTY, "CTX_NOEXST_PROPERTY");
    }

    try {
        nogdb::Property::createIndex(txn, "index_test_2", "index_text_x", false);
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_PROPERTY, "CTX_NOEXST_PROPERTY");
    }

    try {
        nogdb::Property::createIndex(txn, "index_test_3", "index_text", false);
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_CLASS, "CTX_NOEXST_CLASS");
    }

    try {
        nogdb::Property::createIndex(txn, "index_test", "index_text", true);
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_DUPLICATE_INDEX, "CTX_DUPLICATE_INDEX");
    }

    try {
        nogdb::Property::createIndex(txn, "index_test_2", "index_text", true);
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_DUPLICATE_INDEX, "CTX_DUPLICATE_INDEX");
    }

}

void test_drop_index() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Property::dropIndex(txn, "index_test", "index_text");
        nogdb::Property::dropIndex(txn, "index_test", "index_tinyint_u");
        nogdb::Property::dropIndex(txn, "index_test", "index_tinyint");
        nogdb::Property::dropIndex(txn, "index_test", "index_smallint_u");
        nogdb::Property::dropIndex(txn, "index_test", "index_smallint");
        nogdb::Property::dropIndex(txn, "index_test", "index_int_u");
        nogdb::Property::dropIndex(txn, "index_test", "index_int");
        nogdb::Property::dropIndex(txn, "index_test", "index_bigint_u");
        nogdb::Property::dropIndex(txn, "index_test", "index_bigint");
        nogdb::Property::dropIndex(txn, "index_test", "index_real");
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto schema = nogdb::Db::getSchema(txn, "index_test");
        for(const auto& property: schema.properties) {
            if (property.first != "index_blob") {
                assert(property.second.indexInfo.size() == 1);
            }
        }
        schema = nogdb::Db::getSchema(txn, "index_test_2");
        for(const auto& property: schema.properties) {
            if (property.first != "index_blob") {
                assert(property.second.indexInfo.size() == 1);
            }
        }
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_drop_index_extended_class() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Property::dropIndex(txn, "index_test_2", "index_int_2");
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto schema = nogdb::Db::getSchema(txn, "index_test");
        for(const auto& property: schema.properties) {
            if (property.first != "index_blob") {
                assert(property.second.indexInfo.size() == 1);
            }
        }
        schema = nogdb::Db::getSchema(txn, "index_test_2");
        for(const auto& property: schema.properties) {
            if (property.first != "index_blob") {
                if (property.first == "index_int_2") {
                    assert(property.second.indexInfo.size() == 0);
                } else {
                    assert(property.second.indexInfo.size() == 1);
                }
            }
        }
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Property::dropIndex(txn, "index_test_2", "index_text");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_tinyint_u");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_tinyint");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_smallint_u");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_smallint");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_int_u");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_int");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_bigint_u");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_bigint");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_real");
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        for(const auto& property: nogdb::Db::getSchema(txn, "index_test").properties) {
            assert(property.second.indexInfo.size() == 0);
        }
        for(const auto& property: nogdb::Db::getSchema(txn, "index_test_2").properties) {
            if (property.first != "index_text_2")
                assert(property.second.indexInfo.size() == 0);
        }
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_drop_invalid_index() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Property::dropIndex(txn, "index_test", "index_text_x");
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_PROPERTY, "CTX_NOEXST_PROPERTY");
    }

    try {
        nogdb::Property::dropIndex(txn, "index_test_2", "index_text_x");
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_PROPERTY, "CTX_NOEXST_PROPERTY");
    }

    try {
        nogdb::Property::dropIndex(txn, "index_test_3", "index_text");
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_CLASS, "CTX_NOEXST_CLASS");
    }

    try {
        nogdb::Property::dropIndex(txn, "index_test", "index_text");
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_INDEX, "CTX_NOEXST_INDEX");
    }

    try {
        nogdb::Property::dropIndex(txn, "index_test_2", "index_text");
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_INDEX, "CTX_NOEXST_INDEX");
    }

    try {
        nogdb::Property::dropIndex(txn, "index_test_2", "index_int_2");
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_INDEX, "CTX_NOEXST_INDEX");
    }

    try {
        nogdb::Property::remove(txn, "index_test_2", "index_text_2");
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_IN_USED_PROPERTY, "CTX_IN_USED_PROPERTY");
    }

    try {
        nogdb::Class::drop(txn, "index_test_2");
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_IN_USED_PROPERTY, "CTX_IN_USED_PROPERTY");
    }
    txn.rollback();

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Property::dropIndex(txn, "index_test_2", "index_text_2");
        nogdb::Class::drop(txn, "index_test_2");
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    destroy_vertex_index_test();
}

void test_create_index_with_records() {
    init_vertex_index_test();

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Vertex::create(txn, "index_test", nogdb::Record{}
                .set("index_text", "aaa")
                .set("index_tinyint_u", uint8_t{1})
                .set("index_tinyint", int8_t{-1})
                .set("index_smallint_u", uint16_t{10})
                .set("index_smallint", int16_t{-10})
                .set("index_int_u", uint32_t{100})
                .set("index_int", int32_t{-100})
                .set("index_bigint_u", uint64_t{1000})
                .set("index_bigint", int16_t{-1000})
                .set("index_real", 2.0)
        );
        nogdb::Vertex::create(txn, "index_test", nogdb::Record{}
                .set("index_text", "ccc")
                .set("index_tinyint_u", uint8_t{2})
                .set("index_tinyint", int8_t{2})
                .set("index_smallint_u", uint16_t{20})
                .set("index_smallint", int16_t{20})
                .set("index_int_u", uint32_t{200})
                .set("index_int", int32_t{200})
                .set("index_bigint_u", uint64_t{2000})
                .set("index_bigint", int16_t{2000})
                .set("index_real", 8.4)
        );
        nogdb::Vertex::create(txn, "index_test", nogdb::Record{}
                .set("index_text", "bbb")
                .set("index_tinyint_u", uint8_t{0})
                .set("index_tinyint", int8_t{0})
                .set("index_smallint_u", uint16_t{0})
                .set("index_smallint", int16_t{0})
                .set("index_int_u", uint32_t{0})
                .set("index_int", int32_t{0})
                .set("index_bigint_u", uint64_t{0})
                .set("index_bigint", int16_t{0})
                .set("index_real", 0.0)
        );
        nogdb::Vertex::create(txn, "index_test", nogdb::Record{}
                .set("index_text", "zz")
                .set("index_tinyint_u", uint8_t{1})
                .set("index_tinyint", int8_t{-123})
                .set("index_smallint_u", uint16_t{10})
                .set("index_smallint", int16_t{-123})
                .set("index_int_u", uint32_t{100})
                .set("index_int", int32_t{-123123123})
                .set("index_bigint_u", uint64_t{1000})
                .set("index_bigint", int64_t{-123123123})
                .set("index_real", 2.0)
        );
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Property::createIndex(txn, "index_test", "index_text", true);
        nogdb::Property::createIndex(txn, "index_test", "index_tinyint_u", false);
        nogdb::Property::createIndex(txn, "index_test", "index_tinyint", true);
        nogdb::Property::createIndex(txn, "index_test", "index_smallint_u", false);
        nogdb::Property::createIndex(txn, "index_test", "index_smallint", true);
        nogdb::Property::createIndex(txn, "index_test", "index_int_u", false);
        nogdb::Property::createIndex(txn, "index_test", "index_int", true);
        nogdb::Property::createIndex(txn, "index_test", "index_bigint_u", false);
        nogdb::Property::createIndex(txn, "index_test", "index_bigint", true);
        nogdb::Property::createIndex(txn, "index_test", "index_real", false);
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto schema = nogdb::Db::getSchema(txn, "index_test");
        for(const auto& property: schema.properties) {
            if (property.first != "index_blob") {
                assert(property.second.indexInfo.size() == 1);
            }
        }
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

}


void test_create_index_extended_class_with_records() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::createExtend(txn, "index_test_2", "index_test");
        nogdb::Property::add(txn, "index_test_2", "index_text_2", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "index_test_2", "index_int_2", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Vertex::create(txn, "index_test_2", nogdb::Record{}
                .set("index_text", "aaa")
                .set("index_tinyint_u", uint8_t{1})
                .set("index_tinyint", int8_t{-1})
                .set("index_smallint_u", uint16_t{10})
                .set("index_smallint", int16_t{-10})
                .set("index_int_u", uint32_t{100})
                .set("index_int", int32_t{-100})
                .set("index_bigint_u", uint64_t{1000})
                .set("index_bigint", int16_t{-1000})
                .set("index_real", 2.0)
                .set("index_text_2", "AAA")
                .set("index_int_2", int32_t{-999})
        );
        nogdb::Vertex::create(txn, "index_test_2", nogdb::Record{}
                .set("index_text", "ccc")
                .set("index_tinyint_u", uint8_t{2})
                .set("index_tinyint", int8_t{2})
                .set("index_smallint_u", uint16_t{20})
                .set("index_smallint", int16_t{20})
                .set("index_int_u", uint32_t{200})
                .set("index_int", int32_t{200})
                .set("index_bigint_u", uint64_t{2000})
                .set("index_bigint", int16_t{2000})
                .set("index_real", 8.4)
                .set("index_text_2", "ZZZ")
                .set("index_int_2", int32_t{99999})
        );
        nogdb::Vertex::create(txn, "index_test_2", nogdb::Record{}
                .set("index_text", "bbb")
                .set("index_tinyint_u", uint8_t{0})
                .set("index_tinyint", int8_t{0})
                .set("index_smallint_u", uint16_t{0})
                .set("index_smallint", int16_t{0})
                .set("index_int_u", uint32_t{0})
                .set("index_int", int32_t{0})
                .set("index_bigint_u", uint64_t{0})
                .set("index_bigint", int16_t{0})
                .set("index_real", 0.0)
                .set("index_text_2", ".")
                .set("index_int_2", int32_t{0})
        );
        nogdb::Vertex::create(txn, "index_test_2", nogdb::Record{}
                .set("index_text", "bbb")
                .set("index_tinyint_u", uint8_t{123})
                .set("index_tinyint", int8_t{0})
                .set("index_smallint_u", uint16_t{123})
                .set("index_smallint", int16_t{0})
                .set("index_int_u", uint32_t{123123})
                .set("index_int", int32_t{0})
                .set("index_bigint_u", uint64_t{123123123})
                .set("index_bigint", int16_t{0})
                .set("index_real", 123.123)
                .set("index_text_2", "helloworld")
                .set("index_int_2", int32_t{0})
        );
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Property::createIndex(txn, "index_test_2", "index_text_2", true);
        nogdb::Property::createIndex(txn, "index_test_2", "index_int_2", false);
        nogdb::Property::createIndex(txn, "index_test_2", "index_text", false);
        nogdb::Property::createIndex(txn, "index_test_2", "index_tinyint_u", true);
        nogdb::Property::createIndex(txn, "index_test_2", "index_tinyint", false);
        nogdb::Property::createIndex(txn, "index_test_2", "index_smallint_u", true);
        nogdb::Property::createIndex(txn, "index_test_2", "index_smallint", false);
        nogdb::Property::createIndex(txn, "index_test_2", "index_int_u", true);
        nogdb::Property::createIndex(txn, "index_test_2", "index_int", false);
        nogdb::Property::createIndex(txn, "index_test_2", "index_bigint_u", true);
        nogdb::Property::createIndex(txn, "index_test_2", "index_bigint", false);
        nogdb::Property::createIndex(txn, "index_test_2", "index_real", true);
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        for(const auto& property: nogdb::Db::getSchema(txn, "index_test").properties) {
            if (property.first != "index_blob") {
                assert(property.second.indexInfo.size() == 2);
            }
        }
        for(const auto& property: nogdb::Db::getSchema(txn, "index_test_2").properties) {
            if (property.first != "index_blob") {
                if (property.first == "index_text_2" || property.first == "index_int_2") {
                    assert(property.second.indexInfo.size() == 1);
                } else {
                    assert(property.second.indexInfo.size() == 2);
                }
            }
        }
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_create_invalid_index_with_records() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::createExtend(txn, "index_test_3", "index_test");
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Vertex::create(txn, "index_test_3", nogdb::Record{}
                .set("index_text", "aaa")
                .set("index_tinyint_u", uint8_t{1})
                .set("index_tinyint", int8_t{-1})
                .set("index_smallint_u", uint16_t{10})
                .set("index_smallint", int16_t{-10})
                .set("index_int_u", uint32_t{100})
                .set("index_int", int32_t{-100})
                .set("index_bigint_u", uint64_t{1000})
                .set("index_bigint", int16_t{-1000})
                .set("index_real", 2.0)
        );
        nogdb::Vertex::create(txn, "index_test_3", nogdb::Record{}
                .set("index_text", "ccc")
                .set("index_tinyint_u", uint8_t{2})
                .set("index_tinyint", int8_t{2})
                .set("index_smallint_u", uint16_t{20})
                .set("index_smallint", int16_t{20})
                .set("index_int_u", uint32_t{200})
                .set("index_int", int32_t{200})
                .set("index_bigint_u", uint64_t{2000})
                .set("index_bigint", int16_t{2000})
                .set("index_real", 8.4)
        );
        nogdb::Vertex::create(txn, "index_test_3", nogdb::Record{}
                .set("index_text", "aaa")
                .set("index_tinyint_u", uint8_t{1})
                .set("index_tinyint", int8_t{-1})
                .set("index_smallint_u", uint16_t{10})
                .set("index_smallint", int16_t{-10})
                .set("index_int_u", uint32_t{100})
                .set("index_int", int32_t{-100})
                .set("index_bigint_u", uint64_t{1000})
                .set("index_bigint", int16_t{-1000})
                .set("index_real", 2.0)
        );
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Property::createIndex(txn, "index_test_3", "index_text", true);
        assert(false);
    }
    catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_INVALID_INDEX_CONSTRAINT, "CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        nogdb::Property::createIndex(txn, "index_test_3", "index_tinyint_u", true);
        assert(false);
    }
    catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_INVALID_INDEX_CONSTRAINT, "CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        nogdb::Property::createIndex(txn, "index_test_3", "index_tinyint", true);
        assert(false);
    }
    catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_INVALID_INDEX_CONSTRAINT, "CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        nogdb::Property::createIndex(txn, "index_test_3", "index_smallint_u", true);
        assert(false);
    }
    catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_INVALID_INDEX_CONSTRAINT, "CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        nogdb::Property::createIndex(txn, "index_test_3", "index_smallint", true);
        assert(false);
    }
    catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_INVALID_INDEX_CONSTRAINT, "CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        nogdb::Property::createIndex(txn, "index_test_3", "index_int_u", true);
        assert(false);
    }
    catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_INVALID_INDEX_CONSTRAINT, "CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        nogdb::Property::createIndex(txn, "index_test_3", "index_int", true);
        assert(false);
    }
    catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_INVALID_INDEX_CONSTRAINT, "CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        nogdb::Property::createIndex(txn, "index_test_3", "index_bigint_u", true);
        assert(false);
    }
    catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_INVALID_INDEX_CONSTRAINT, "CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        nogdb::Property::createIndex(txn, "index_test_3", "index_bigint", true);
        assert(false);
    }
    catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_INVALID_INDEX_CONSTRAINT, "CTX_INVALID_INDEX_CONSTRAINT");
    }

    try {
        nogdb::Property::createIndex(txn, "index_test_3", "index_real", true);
        assert(false);
    }
    catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_INVALID_INDEX_CONSTRAINT, "CTX_INVALID_INDEX_CONSTRAINT");
    }
    txn.rollback();

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "index_test_3");
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

}

void test_drop_index_with_records() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Property::dropIndex(txn, "index_test", "index_text");
        nogdb::Property::dropIndex(txn, "index_test", "index_tinyint_u");
        nogdb::Property::dropIndex(txn, "index_test", "index_tinyint");
        nogdb::Property::dropIndex(txn, "index_test", "index_smallint_u");
        nogdb::Property::dropIndex(txn, "index_test", "index_smallint");
        nogdb::Property::dropIndex(txn, "index_test", "index_int_u");
        nogdb::Property::dropIndex(txn, "index_test", "index_int");
        nogdb::Property::dropIndex(txn, "index_test", "index_bigint_u");
        nogdb::Property::dropIndex(txn, "index_test", "index_bigint");
        nogdb::Property::dropIndex(txn, "index_test", "index_real");
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto schema = nogdb::Db::getSchema(txn, "index_test");
        for(const auto& property: schema.properties) {
            if (property.first != "index_blob") {
                assert(property.second.indexInfo.size() == 1);
            }
        }
        schema = nogdb::Db::getSchema(txn, "index_test_2");
        for(const auto& property: schema.properties) {
            if (property.first != "index_blob") {
                assert(property.second.indexInfo.size() == 1);
            }
        }
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_drop_index_extended_class_with_records() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Property::dropIndex(txn, "index_test_2", "index_int_2");
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto schema = nogdb::Db::getSchema(txn, "index_test");
        for(const auto& property: schema.properties) {
            if (property.first != "index_blob") {
                assert(property.second.indexInfo.size() == 1);
            }
        }
        schema = nogdb::Db::getSchema(txn, "index_test_2");
        for(const auto& property: schema.properties) {
            if (property.first != "index_blob") {
                if (property.first == "index_int_2") {
                    assert(property.second.indexInfo.size() == 0);
                } else {
                    assert(property.second.indexInfo.size() == 1);
                }
            }
        }
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Property::dropIndex(txn, "index_test_2", "index_text");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_tinyint_u");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_tinyint");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_smallint_u");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_smallint");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_int_u");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_int");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_bigint_u");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_bigint");
        nogdb::Property::dropIndex(txn, "index_test_2", "index_real");
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        for(const auto& property: nogdb::Db::getSchema(txn, "index_test").properties) {
            assert(property.second.indexInfo.size() == 0);
        }
        for(const auto& property: nogdb::Db::getSchema(txn, "index_test_2").properties) {
            if (property.first != "index_text_2")
                assert(property.second.indexInfo.size() == 0);
        }
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_drop_invalid_index_with_records() {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    try {
        nogdb::Property::dropIndex(txn, "index_test", "index_text_x");
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_PROPERTY, "CTX_NOEXST_PROPERTY");
    }

    try {
        nogdb::Property::dropIndex(txn, "index_test_2", "index_text_x");
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_PROPERTY, "CTX_NOEXST_PROPERTY");
    }

    try {
        nogdb::Property::dropIndex(txn, "index_test_3", "index_text");
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_CLASS, "CTX_NOEXST_CLASS");
    }

    try {
        nogdb::Property::dropIndex(txn, "index_test", "index_text");
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_INDEX, "CTX_NOEXST_INDEX");
    }

    try {
        nogdb::Property::dropIndex(txn, "index_test_2", "index_text");
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_INDEX, "CTX_NOEXST_INDEX");
    }

    try {
        nogdb::Property::dropIndex(txn, "index_test_2", "index_int_2");
        assert(false);
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_NOEXST_INDEX, "CTX_NOEXST_INDEX");
    }

    try {
        nogdb::Property::remove(txn, "index_test_2", "index_text_2");
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_IN_USED_PROPERTY, "CTX_IN_USED_PROPERTY");
    }

    try {
        nogdb::Class::drop(txn, "index_test_2");
    } catch(const nogdb::Error& ex) {
        REQUIRE(ex, CTX_IN_USED_PROPERTY, "CTX_IN_USED_PROPERTY");
    }
    txn.rollback();

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Property::dropIndex(txn, "index_test_2", "index_text_2");
        nogdb::Class::drop(txn, "index_test_2");
        txn.commit();
    } catch(const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    destroy_vertex_index_test();
}
