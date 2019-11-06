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

void assert_class(const nogdb::Transaction& txn, const std::string& className, const std::string& superClassName,
    size_t sizeOfSubClasses, size_t sizeOfProperties)
{
    auto res = txn.getClass(className);
    auto superId = (superClassName == "") ? 0 : txn.getClass(superClassName).id;
    assert(res.base == superId);
    auto properties = txn.getProperties(res);
    assert(properties.size() == sizeOfProperties);
    auto subClassCount = size_t { 0 };
    for (const auto& classDesc : txn.getClasses()) {
        subClassCount += classDesc.base == res.id;
    }
    assert(subClassCount == sizeOfSubClasses);
}

void init_all_extended_classes()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("employees", nogdb::ClassType::VERTEX);
        txn.addProperty("employees", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("employees", "age", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addProperty("employees", "salary", nogdb::PropertyType::UNSIGNED_BIGINT);
        txn.addSubClassOf("employees", "backends");
        txn.addProperty("backends", "cpp_skills", nogdb::PropertyType::INTEGER);
        txn.addProperty("backends", "js_skills", nogdb::PropertyType::INTEGER);
        txn.addSubClassOf("employees", "frontends");
        txn.addProperty("frontends", "html_skills", nogdb::PropertyType::INTEGER);
        txn.addProperty("frontends", "js_skills", nogdb::PropertyType::INTEGER);
        txn.addSubClassOf("backends", "systems");
        txn.addProperty("systems", "devops_skills", nogdb::PropertyType::INTEGER);
        txn.addSubClassOf("backends", "infras");
        txn.addProperty("infras", "IT_skills", nogdb::PropertyType::INTEGER);
        txn.addSubClassOf("frontends", "designers");
        txn.addProperty("designers", "ux_skills", nogdb::PropertyType::INTEGER);
        txn.addSubClassOf("employees", "admins");
        txn.addClass("action", nogdb::ClassType::EDGE);
        txn.addProperty("action", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("action", "type", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addSubClassOf("action", "collaborate");
        txn.addSubClassOf("collaborate", "inter");
        txn.addSubClassOf("collaborate", "intra");
        txn.addSubClassOf("action", "manage");
        txn.addProperty("manage", "priority", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void destroy_all_extended_classes()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("intra");
        txn.dropClass("inter");
        txn.dropClass("collaborate");
        txn.dropClass("manage");
        txn.dropClass("action");
        txn.dropClass("systems");
        txn.dropClass("infras");
        txn.dropClass("backends");
        txn.dropClass("designers");
        txn.dropClass("frontends");
        txn.dropClass("admins");
        txn.dropClass("employees");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_create_class_extend()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("employees", nogdb::ClassType::VERTEX);
        txn.addProperty("employees", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("employees", "age", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addProperty("employees", "salary", nogdb::PropertyType::UNSIGNED_BIGINT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addSubClassOf("employees", "backends");
        txn.addProperty("backends", "cpp_skills", nogdb::PropertyType::INTEGER);
        txn.addProperty("backends", "js_skills", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addSubClassOf("employees", "frontends");
        txn.addProperty("frontends", "html_skills", nogdb::PropertyType::INTEGER);
        txn.addProperty("frontends", "js_skills", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addSubClassOf("backends", "systems");
        txn.addProperty("systems", "devops_skills", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addSubClassOf("backends", "infras");
        txn.addProperty("infras", "IT_skills", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addSubClassOf("frontends", "designers");
        txn.addProperty("designers", "ux_skills", nogdb::PropertyType::INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addSubClassOf("employees", "admins");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("action", nogdb::ClassType::EDGE);
        txn.addProperty("action", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("action", "type", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addSubClassOf("action", "collaborate");
        txn.addSubClassOf("collaborate", "inter");
        txn.addSubClassOf("collaborate", "intra");
        txn.addSubClassOf("action", "manage");
        txn.addProperty("manage", "priority", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        assert_class(txn, "employees", "", 3, 3);
        assert_class(txn, "backends", "employees", 2, 5);
        assert_class(txn, "frontends", "employees", 1, 5);
        assert_class(txn, "admins", "employees", 0, 3);
        assert_class(txn, "designers", "frontends", 0, 6);
        assert_class(txn, "systems", "backends", 0, 6);
        assert_class(txn, "infras", "backends", 0, 6);
        assert_class(txn, "action", "", 2, 2);
        assert_class(txn, "collaborate", "action", 2, 2);
        assert_class(txn, "manage", "action", 0, 3);
        assert_class(txn, "inter", "collaborate", 0, 2);
        assert_class(txn, "intra", "collaborate", 0, 2);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto res = txn.getClass("infras");
        assert(res.type == nogdb::ClassType::VERTEX);
        res = txn.getClass("intra");
        assert(res.type == nogdb::ClassType::EDGE);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.rollback();
}

void test_create_invalid_class_extend()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.addSubClassOf("backend", "senior");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    }

    try {
        txn.addSubClassOf("backends", "");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_CLASSNAME, "NOGDB_CTX_INVALID_CLASSNAME");
    }

    try {
        txn.addSubClassOf("backends", "designers");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_DUPLICATE_CLASS, "NOGDB_CTX_DUPLICATE_CLASS");
    }

    try {
        txn.addSubClassOf("backends", "something1");
        txn.addProperty("something1", "", nogdb::PropertyType::INTEGER);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_PROPERTYNAME, "NOGDB_CTX_INVALID_PROPERTYNAME");
    }

    try {
        txn.addSubClassOf("backends", "something2");
        txn.addProperty("something2", "prop1", nogdb::PropertyType::UNDEFINED);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_PROPTYPE, "NOGDB_CTX_INVALID_PROPTYPE");
    }

    try {
        txn.addSubClassOf("systems", "something3");
        txn.addProperty("something3", "prop1", nogdb::PropertyType::TEXT);
        txn.addProperty("something3", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("something3", "prop3", nogdb::PropertyType::TEXT);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_DUPLICATE_PROPERTY, "NOGDB_CTX_DUPLICATE_PROPERTY");
    }
}

void test_alter_class_extend()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.renameClass("backends", "backbackends");
        assert_class(txn, "systems", "backbackends", 0, 6);
        assert_class(txn, "infras", "backbackends", 0, 6);
        assert_class(txn, "backbackends", "employees", 2, 5);

        txn.renameClass("backbackends", "backends");
        assert_class(txn, "systems", "backends", 0, 6);
        assert_class(txn, "infras", "backends", 0, 6);
        assert_class(txn, "backends", "employees", 2, 5);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_drop_class_extend()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("designers");
        assert_class(txn, "frontends", "employees", 0, 5);

        txn.dropClass("collaborate");
        assert_class(txn, "action", "", 3, 2);
        assert_class(txn, "inter", "action", 0, 2);
        assert_class(txn, "intra", "action", 0, 2);

        txn.dropClass("backends");
        assert_class(txn, "employees", "", 4, 3);
        assert_class(txn, "systems", "employees", 0, 4);
        assert_class(txn, "infras", "employees", 0, 4);

        txn.dropClass("action");
        assert_class(txn, "manage", "", 0, 1);
        assert_class(txn, "inter", "", 0, 0);
        assert_class(txn, "intra", "", 0, 0);

        txn.dropClass("employees");
        txn.dropClass("inter");
        txn.dropClass("admins");
        txn.dropClass("intra");
        txn.dropClass("manage");
        txn.dropClass("systems");
        txn.dropClass("infras");
        txn.dropClass("frontends");

        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_add_property_extend()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addProperty("employees", "prop1", nogdb::PropertyType::UNSIGNED_INTEGER);
        assert_class(txn, "designers", "frontends", 0, 7);
        assert_class(txn, "admins", "employees", 0, 4);

        txn.addProperty("collaborate", "prop1", nogdb::PropertyType::BLOB);
        assert_class(txn, "collaborate", "action", 2, 3);
        assert_class(txn, "inter", "collaborate", 0, 3);
        assert_class(txn, "intra", "collaborate", 0, 3);
        assert_class(txn, "action", "", 2, 2);

        txn.addProperty("systems", "prop2", nogdb::PropertyType::REAL);
        assert_class(txn, "systems", "backends", 0, 8);
        assert_class(txn, "infras", "backends", 0, 7);
        assert_class(txn, "backends", "employees", 2, 6);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_add_invalid_property_extend()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.addProperty("designers", "name", nogdb::PropertyType::TEXT);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_DUPLICATE_PROPERTY, "NOGDB_CTX_DUPLICATE_PROPERTY");
    }

    try {
        txn.addProperty("employees", "IT_skills", nogdb::PropertyType::TEXT);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_OVERRIDE_PROPERTY, "NOGDB_CTX_OVERRIDE_PROPERTY");
    }
}

void test_delete_property_extend()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropProperty("systems", "prop2");
        assert_class(txn, "systems", "backends", 0, 7);
        assert_class(txn, "infras", "backends", 0, 7);
        assert_class(txn, "backends", "employees", 2, 6);

        txn.dropProperty("collaborate", "prop1");
        assert_class(txn, "collaborate", "action", 2, 2);
        assert_class(txn, "inter", "collaborate", 0, 2);
        assert_class(txn, "intra", "collaborate", 0, 2);
        assert_class(txn, "action", "", 2, 2);

        txn.dropProperty("employees", "prop1");
        assert_class(txn, "designers", "frontends", 0, 6);
        assert_class(txn, "admins", "employees", 0, 3);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_delete_invalid_property_extend()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.dropProperty("systems", "name");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }

    try {
        txn.dropProperty("employees", "devops_skills");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }
}

void test_alter_property_extend()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.renameProperty("employees", "name", "title");
        auto classDesc = txn.getClass("systems");
        auto properties = txn.getProperties(classDesc);
        assert(std::find_if(properties.cbegin(), properties.cend(), [](const nogdb::PropertyDescriptor& property) {
            return property.name == "name";
        }) == properties.cend());
        assert(std::find_if(properties.cbegin(), properties.cend(), [](const nogdb::PropertyDescriptor& property) {
            return property.name == "title";
        }) != properties.cend());
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.renameProperty("employees", "title", "name");
        auto classDesc = txn.getClass("infras");
        auto properties = txn.getProperties(classDesc);
        assert(std::find_if(properties.cbegin(), properties.cend(), [](const nogdb::PropertyDescriptor& property) {
            return property.name == "name";
        }) != properties.cend());
        assert(std::find_if(properties.cbegin(), properties.cend(), [](const nogdb::PropertyDescriptor& property) {
            return property.name == "title";
        }) == properties.cend());
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_alter_invalid_property_extend()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.renameProperty("backends", "cpp_skills", "IT_skills");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_OVERRIDE_PROPERTY, "NOGDB_CTX_OVERRIDE_PROPERTY");
    }

    try {
        txn.renameProperty("backends", "cpp_skills", "age");
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_DUPLICATE_PROPERTY, "NOGDB_CTX_DUPLICATE_PROPERTY");
    }
}

void test_create_vertex_edge_extend()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        auto v1
            = txn.addVertex("infras", nogdb::Record {}.set("name", "Peter").set("js_skills", 7).set("IT_skills", 9));
        auto v2 = txn.addVertex("admins", nogdb::Record {}.set("name", "Mike").set("age", 36U));
        auto e = txn.addEdge("manage", v1, v2, nogdb::Record {}.set("name", "Team Leader"));
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_create_invalid_vertex_edge_extend()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.addVertex("infras", nogdb::Record {}.set("name", "Pete").set("devops_skills", 4));
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }

    txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.addVertex("employees", nogdb::Record {}.set("name", "Pete").set("js_skills", 4));
        assert(false);
    } catch (const nogdb::Error& ex) {
        txn.rollback();
        REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }
}

void test_delete_vertex_edge_extend()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        auto res = txn.find("manage").get();
        txn.remove(res[0].descriptor);
        res = txn.find("infras").get();
        txn.remove(res[0].descriptor);
        res = txn.find("admins").get();
        txn.remove(res[0].descriptor);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_get_class_extend()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    nogdb::RecordDescriptor a {}, b {}, c {}, d {}, e {}, f {};
    try {
        a = txn.addVertex("admins", nogdb::Record {}.set("name", "Adam").set("age", 26U));
        b = txn.addVertex("backends", nogdb::Record {}.set("name", "Bill").set("age", 32U).set("cpp_skills", 7));
        c = txn.addVertex("systems",
            nogdb::Record {}
                .set("name", "Charon")
                .set("age", 27U)
                .set("js_skills", 6)
                .set("cpp_skills", 8)
                .set("devops_skills", 10));
        d = txn.addVertex("designers", nogdb::Record {}.set("name", "Don").set("ux_skills", 9U));
        e = txn.addVertex("employees", nogdb::Record {}.set("name", "Eric"));
        f = txn.addVertex("frontends", nogdb::Record {}.set("name", "Falcao").set("age", 34U).set("js_skills", 9));

        txn.addEdge("manage", a, e, nogdb::Record {}.set("name", "helpdesk").set("priority", "medium"));
        txn.addEdge("inter", b, f, nogdb::Record {}.set("name", "api creator"));
        txn.addEdge("intra", b, c, nogdb::Record {}.set("name", "team member"));
        txn.addEdge("inter", c, f, nogdb::Record {}.set("name", "system provider"));
        txn.addEdge("manage", c, b, nogdb::Record {}.set("name", "team leader").set("priority", "high"));
        txn.addEdge("intra", c, b, nogdb::Record {}.set("name", "system provider"));
        txn.addEdge("collaborate", d, b, nogdb::Record {}.set("name", "ui provider"));
        txn.addEdge("collaborate", d, c, nogdb::Record {}.set("name", "ui provider"));
        txn.addEdge("intra", d, f, nogdb::Record {}.set("name", "wireframe creator"));
        txn.addEdge("collaborate", e, a, nogdb::Record {}.set("name", "guest"));
        txn.addEdge("inter", f, b, nogdb::Record {}.set("name", "ui creator"));
        txn.addEdge("intra", f, d, nogdb::Record {}.set("name", "team member"));

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    try {
        auto res = txn.find("employees").get();
        ASSERT_SIZE(res, 1);

        res = txn.findSubClassOf("employees").get();
        ASSERT_SIZE(res, 6);
        res = getVertexMultipleClassExtend(txn, std::set<std::string> { "admins", "backends", "frontends" });
        ASSERT_SIZE(res, 5);
        res = txn.findSubClassOf("action").get();
        ASSERT_SIZE(res, 12);
        res = txn.findSubClassOf("manage").get();
        ASSERT_SIZE(res, 2);
        res = getEdgeMultipleClassExtend(txn, std::set<std::string> { "collaborate", "manage" });
        ASSERT_SIZE(res, 12);
        res = txn.findSubClassOf("inter").get();
        ASSERT_SIZE(res, 3);

        res = txn.findSubClassOf("backends").get();
        for (const auto& r : res) {
            if (r.record.get("name").toText() == "Bill") {
                auto edges = txn.findInEdge(r.descriptor).where(nogdb::GraphFilter {}.exclude("collaborate")).get();
                ASSERT_SIZE(edges, 3);
                edges
                    = txn.findInEdge(r.descriptor).where(nogdb::GraphFilter {}.excludeSubClassOf("collaborate")).get();
                ASSERT_SIZE(edges, 1);
                edges = txn.findEdge(r.descriptor).where(nogdb::GraphFilter {}.only("inter", "manage")).get();
                ASSERT_SIZE(edges, 3);
            } else if (r.record.get("name").toText() == "Charon") {
                auto edges
                    = txn.findOutEdge(r.descriptor).where(nogdb::GraphFilter {}.onlySubClassOf("collaborate")).get();
                ASSERT_SIZE(edges, 2);
                edges = txn.findInEdge(r.descriptor).where(nogdb::GraphFilter {}.onlySubClassOf("collaborate")).get();
                ASSERT_SIZE(edges, 2);
                edges = txn.findOutEdge(r.descriptor).where(nogdb::GraphFilter {}.only("collaborate")).get();
                ASSERT_SIZE(edges, 0);
                edges = txn.findInEdge(r.descriptor).where(nogdb::GraphFilter {}.only("collaborate")).get();
                ASSERT_SIZE(edges, 1);
            }
        }

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_find_class_extend()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto res = txn.findSubClassOf("systems").where(nogdb::Condition("age").le(30U)).get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "Charon");
        res = txn.findSubClassOf("employees").where(nogdb::Condition("age").le(30U)).get();
        ASSERT_SIZE(res, 2);
        assert(res[0].record.get("name").toText() == "Charon" || res[0].record.get("name").toText() == "Adam");
        assert(res[1].record.get("name").toText() == "Charon" || res[1].record.get("name").toText() == "Adam");
        res = txn.findSubClassOf("backends").where(nogdb::Condition("cpp_skills").eq(8)).get();

        res = txn.findSubClassOf("collaborate").where(nogdb::Condition("name").endWith("provider").ignoreCase()).get();
        ASSERT_SIZE(res, 4);
        res = txn.findSubClassOf("action").where(nogdb::Condition("priority")).get();
        ASSERT_SIZE(res, 2);

        auto b = txn.findSubClassOf("employees").where(nogdb::Condition("name").eq("Bill")).get();
        assert(b.size() == 1);
        res = txn.findInEdge(b[0].descriptor).where(nogdb::Condition("name").endWith("provider").ignoreCase()).get();
        ASSERT_SIZE(res, 2);
        assert(res[0].record.get("name").toText() == "ui provider"
            || res[0].record.get("name").toText() == "system provider");
        assert(res[1].record.get("name").toText() == "ui provider"
            || res[1].record.get("name").toText() == "system provider");
        res = txn.findInEdge(b[0].descriptor)
                  .where(
                      nogdb::GraphFilter { nogdb::Condition("name").endWith("provider").ignoreCase() }.onlySubClassOf(
                          "collaborate"))
                  .get();
        ASSERT_SIZE(res, 2);
        assert(res[0].record.get("name").toText() == "ui provider"
            || res[0].record.get("name").toText() == "system provider");
        assert(res[1].record.get("name").toText() == "ui provider"
            || res[1].record.get("name").toText() == "system provider");
        res = txn.findInEdge(b[0].descriptor)
                  .where(nogdb::GraphFilter { nogdb::Condition("type").null() }.only("inter", "manage"))
                  .get();
        ASSERT_SIZE(res, 2);
        assert(
            res[0].record.get("name").toText() == "ui creator" || res[0].record.get("name").toText() == "team leader");
        assert(
            res[1].record.get("name").toText() == "ui creator" || res[1].record.get("name").toText() == "team leader");

        auto c = txn.findSubClassOf("employees").where(nogdb::Condition("name").eq("Charon")).get();
        assert(c.size() == 1);
        res = txn.findOutEdge(c[0].descriptor)
                  .where(nogdb::GraphFilter { nogdb::Condition("name").beginWith("team").ignoreCase() }.onlySubClassOf(
                      "action"))
                  .get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "team leader");
        res = txn.findEdge(b[0].descriptor)
                  .where(nogdb::GraphFilter { nogdb::Condition("name").contain("team").ignoreCase() }.onlySubClassOf(
                      "collaborate"))
                  .get();
        ASSERT_SIZE(res, 1);
        assert(res[0].record.get("name").toText() == "team member");

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    txn.commit();
}

void test_traverse_class_extend()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto b = txn.findSubClassOf("employees").where(nogdb::Condition("name").eq("Bill")).get();
        auto c = txn.findSubClassOf("employees").where(nogdb::Condition("name").eq("Charon")).get();
        auto f = txn.findSubClassOf("employees").where(nogdb::Condition("name").eq("Falcao")).get();
        auto res = txn.traverseIn(b[0].descriptor).depth(1, 1).get();
        ASSERT_SIZE(res, 3);
        res = txn.traverseIn(b[0].descriptor)
                  .depth(1, 1)
                  .whereE(nogdb::GraphFilter {}.onlySubClassOf("collaborate"))
                  .get();
        ASSERT_SIZE(res, 3);
        res = txn.traverseOut(f[0].descriptor)
                  .depth(1, 1)
                  .whereE(nogdb::GraphFilter {}.onlySubClassOf("collaborate"))
                  .get();
        ASSERT_SIZE(res, 2);
        res = txn.traverseOut(f[0].descriptor)
                  .depth(1, 2)
                  .whereE(nogdb::GraphFilter {}.onlySubClassOf("collaborate"))
                  .get();
        ASSERT_SIZE(res, 3);
        res = txn.traverse(c[0].descriptor)
                  .depth(0, 100)
                  .whereE(nogdb::GraphFilter {}.onlySubClassOf("collaborate", "manage"))
                  .get();
        ASSERT_SIZE(res, 4);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}

void test_shortest_path_class_extend()
{
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    try {
        auto b = txn.findSubClassOf("employees").where(nogdb::Condition("name").eq("Bill")).get();
        auto c = txn.findSubClassOf("employees").where(nogdb::Condition("name").eq("Charon")).get();
        auto d = txn.findSubClassOf("employees").where(nogdb::Condition("name").eq("Don")).get();
        auto res = txn.shortestPath(c[0].descriptor, d[0].descriptor).get();
        ASSERT_SIZE(res, 3);
        assert(res[0].record.get("name").toText() == "Charon");
        assert(res[1].record.get("name").toText() == "Falcao");
        assert(res[2].record.get("name").toText() == "Don");

        res = txn.shortestPath(c[0].descriptor, d[0].descriptor)
                  .whereE(nogdb::GraphFilter {}.onlySubClassOf("collaborate"))
                  .get();
        ASSERT_SIZE(res, 3);
        assert(res[0].record.get("name").toText() == "Charon");
        assert(res[1].record.get("name").toText() == "Falcao");
        assert(res[2].record.get("name").toText() == "Don");

        res = txn.shortestPath(b[0].descriptor, d[0].descriptor)
                  .whereE(nogdb::GraphFilter {}.onlySubClassOf("collaborate"))
                  .get();
        ASSERT_SIZE(res, 3);
        assert(res[0].record.get("name").toText() == "Bill");
        assert(res[1].record.get("name").toText() == "Falcao");
        assert(res[2].record.get("name").toText() == "Don");

        res = txn.shortestPath(b[0].descriptor, d[0].descriptor)
                  .whereE(nogdb::GraphFilter {}.onlySubClassOf("inter", "manage"))
                  .get();
        ASSERT_SIZE(res, 0);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
    txn.commit();
}
