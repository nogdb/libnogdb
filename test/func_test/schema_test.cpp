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

void test_create_class()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("files", nogdb::ClassType::VERTEX);
        auto schema = txn.getClass("files");
        assert(schema.name == "files");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_create_class_with_properties()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("files2", nogdb::ClassType::VERTEX);
        txn.addProperty("files2", "prop1", nogdb::PropertyType::TEXT);
        txn.addProperty("files2", "prop2", nogdb::PropertyType::INTEGER);
        txn.addProperty("files2", "prop3", nogdb::PropertyType::UNSIGNED_BIGINT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_drop_class()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("files");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("files2");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_alter_class()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("files", nogdb::ClassType::VERTEX);
        txn.addProperty("files", "prop1", nogdb::PropertyType::INTEGER);
        txn.addProperty("files", "prop2", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto cdesc = txn.getClass("files");
        assert(cdesc.name == "files");
        txn.commit();

        txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.renameClass("files", "file");
        txn.commit();

        txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        cdesc = txn.getClass("file");
        assert(cdesc.name == "file");
        auto properties = txn.getProperties(cdesc);
        assert(properties.size() == 2);
        for (const auto& property : properties) {
            if (property.name == "prop1")
                assert(property.type == nogdb::PropertyType::INTEGER);
            else if (property.name == "prop2")
                assert(property.type == nogdb::PropertyType::TEXT);
            else
                assert(false);
        }
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("file");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_alter_invalid_class()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("files", nogdb::ClassType::VERTEX);
        txn.addProperty("files", "prop1", nogdb::PropertyType::INTEGER);
        txn.addProperty("files", "prop2", nogdb::PropertyType::TEXT);
        txn.addClass("folders", nogdb::ClassType::VERTEX);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.renameClass("files", "");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_CLASSNAME, "NOGDB_CTX_INVALID_CLASSNAME");
    }

    try {
        txn.renameClass("", "file");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_CLASSNAME, "NOGDB_CTX_INVALID_CLASSNAME");
    }

    try {
        txn.renameClass("file", "filess");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    try {
        txn.renameClass("files", "files");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_DUPLICATE_CLASS, "NOGDB_CTX_DUPLICATE_CLASS");
    }

    try {
        txn.renameClass("files", "folders");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_DUPLICATE_CLASS, "NOGDB_CTX_DUPLICATE_CLASS");
    }
    txn.commit();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("files");
        txn.dropClass("folders");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_create_invalid_class()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("files", nogdb::ClassType::VERTEX);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.addClass("", nogdb::ClassType::VERTEX);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_CLASSNAME, "NOGDB_CTX_INVALID_CLASSNAME");
    }
    try {
        txn.addClass("files", nogdb::ClassType::VERTEX);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_DUPLICATE_CLASS, "NOGDB_CTX_DUPLICATE_CLASS");
    }
    try {
        txn.addClass("files", nogdb::ClassType::UNDEFINED);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_CLASSTYPE, "NOGDB_CTX_INVALID_CLASSTYPE");
    }
    txn.commit();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("files");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_create_invalid_class_with_properties()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("files2", nogdb::ClassType::VERTEX);
        txn.addProperty("files2", "prop1", nogdb::PropertyType::TEXT);
        txn.addProperty("files2", "prop2", nogdb::PropertyType::INTEGER);
        txn.addProperty("files2", "prop3", nogdb::PropertyType::UNDEFINED);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_PROPTYPE, "NOGDB_CTX_INVALID_PROPTYPE");
    }
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("files2", nogdb::ClassType::VERTEX);
        txn.addProperty("files2", "prop1", nogdb::PropertyType::TEXT);
        txn.addProperty("files2", "", nogdb::PropertyType::INTEGER);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_PROPERTYNAME, "NOGDB_CTX_INVALID_PROPERTYNAME");
    }
}

void test_drop_invalid_class()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.dropClass("");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_CLASSNAME, "NOGDB_CTX_INVALID_CLASSNAME");
    }
    try {
        txn.dropClass("file");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
        assert(ex.code() == NOGDB_CTX_NOEXST_CLASS);
    }
    try {
        txn.dropClass("files");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }
    try {
        txn.dropClass("files2");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }
}

void test_add_property()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("files", nogdb::ClassType::VERTEX);
        txn.addProperty("files", "filename", nogdb::PropertyType::TEXT);
        txn.addProperty("files", "filesize", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addProperty("files", "ctime", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto schema = txn.getClass("files");
        assert(schema.name == "files");
        auto properties = txn.getProperties(schema);
        assert(properties.size() == 3);
        for (const auto& property : properties) {
            if (property.name == "filename")
                assert(property.type == nogdb::PropertyType::TEXT);
            else if (property.name == "filesize")
                assert(property.type == nogdb::PropertyType::UNSIGNED_INTEGER);
            else if (property.name == "ctime")
                assert(property.type == nogdb::PropertyType::UNSIGNED_INTEGER);
            else
                assert(false);
        }
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_delete_property()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropProperty("files", "ctime");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("files");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_add_invalid_property()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("files", nogdb::ClassType::VERTEX);
        txn.addProperty("files", "filename", nogdb::PropertyType::TEXT);
        txn.addProperty("files", "filesize", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addProperty("files", "ctime", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.addProperty("files", "", nogdb::PropertyType::INTEGER);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_PROPERTYNAME, "NOGDB_CTX_INVALID_PROPERTYNAME");
    }
    try {
        txn.addProperty("", "extension", nogdb::PropertyType::INTEGER);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_CLASSNAME, "NOGDB_CTX_INVALID_CLASSNAME");
    }
    try {
        txn.addProperty("file", "extension", nogdb::PropertyType::TEXT);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }
    try {
        txn.addProperty("links", "type", nogdb::PropertyType::UNDEFINED);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_PROPTYPE, "NOGDB_CTX_INVALID_PROPTYPE");
    }
    try {
        txn.addProperty("files", "filename", nogdb::PropertyType::TEXT);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_DUPLICATE_PROPERTY, "NOGDB_CTX_DUPLICATE_PROPERTY");
    }
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto schema = txn.getClass("files");
        assert(schema.name == "files");
        auto properties = txn.getProperties(schema);
        assert(properties.size() == 3);
        for (const auto& property : properties) {
            if (property.name == "filename")
                assert(property.type == nogdb::PropertyType::TEXT);
            else if (property.name == "filesize")
                assert(property.type == nogdb::PropertyType::UNSIGNED_INTEGER);
            else if (property.name == "ctime")
                assert(property.type == nogdb::PropertyType::UNSIGNED_INTEGER);
            else
                assert(false);
        }
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_delete_invalid_property()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.dropProperty("files", "ctimes");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
        assert(ex.code() == NOGDB_CTX_NOEXST_PROPERTY);
    }
    try {
        txn.dropProperty("files", "");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_PROPERTYNAME, "NOGDB_CTX_INVALID_PROPERTYNAME");
    }
    try {
        txn.dropProperty("file", "ctime");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }
    try {
        txn.dropProperty("files", "ctime");
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        txn.dropProperty("files", "ctime");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }
    txn.commit();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("files");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_alter_property()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("links", nogdb::ClassType::EDGE);
        txn.addProperty("links", "type", nogdb::PropertyType::TEXT);
        txn.addProperty("links", "expire", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.renameProperty("links", "type", "comments");
        txn.renameProperty("links", "expire", "expired");
        txn.addProperty("links", "type", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto schema = txn.getClass("links");
        assert(schema.name == "links");
        auto properties = txn.getProperties(schema);
        assert(properties.size() == 3);
        for (const auto& property : properties) {
            if (property.name == "type")
                assert(property.type == nogdb::PropertyType::BLOB);
            else if (property.name == "comments")
                assert(property.type == nogdb::PropertyType::TEXT);
            else if (property.name == "expired")
                assert(property.type == nogdb::PropertyType::INTEGER);
            else
                assert(false);
        }
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("links");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_alter_invalid_property()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("links", nogdb::ClassType::EDGE);
        txn.addProperty("links", "type", nogdb::PropertyType::TEXT);
        txn.addProperty("links", "expire", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.renameProperty("link", "type", "");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_PROPERTYNAME, "NOGDB_CTX_INVALID_PROPERTYNAME");
    }
    try {
        txn.renameProperty("", "type", "types");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_CLASSNAME, "NOGDB_CTX_INVALID_CLASSNAME");
    }
    try {
        txn.renameProperty("links", "", "types");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_PROPERTYNAME, "NOGDB_CTX_INVALID_PROPERTYNAME");
    }
    try {
        txn.renameProperty("link", "type", "comments");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }
    try {
        txn.renameProperty("links", "types", "comments");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }
    try {
        txn.renameProperty("links", "type", "expire");
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_DUPLICATE_PROPERTY, "NOGDB_CTX_DUPLICATE_PROPERTY");
    }
    txn.commit();

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("links");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}
