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

struct ClassSchema {

    ClassSchema() = default;

    ClassSchema(const nogdb::Transaction& _txn, const nogdb::ClassDescriptor& _classDescriptor)
        : classDescriptor { _classDescriptor }
        , propertyDescriptors { _txn.getProperties(_classDescriptor) }
        , indexDescriptors { _txn.getIndexes(_classDescriptor) }
    {
    }

    nogdb::ClassDescriptor classDescriptor {};
    std::vector<nogdb::PropertyDescriptor> propertyDescriptors {};
    std::vector<nogdb::IndexDescriptor> indexDescriptors {};
};

void assert_dbinfo(const nogdb::DBInfo& info1, const nogdb::DBInfo& info2)
{
    assert(info1.numClass == info2.numClass);
    assert(info1.numProperty == info2.numProperty);
    assert(info1.numIndex == info2.numIndex);
    assert(info1.dbPath == info2.dbPath);
    assert(info1.maxClassId == info2.maxClassId);
    assert(info1.maxPropertyId == info2.maxPropertyId);
    assert(info1.maxIndexId == info2.maxIndexId);
}

void assert_schema(const std::vector<ClassSchema>& sc1, const std::vector<ClassSchema>& sc2)
{
    assert(sc1.size() == sc2.size());
    for (auto it = sc1.cbegin(); it != sc1.cend(); ++it) {
        auto sc1Class = it->classDescriptor;
        // compare class
        auto tmp = std::find_if(sc2.cbegin(), sc2.cend(), [&sc1Class](const ClassSchema& c) {
            auto cdesc = c.classDescriptor;
            return (sc1Class.name == cdesc.name) && (sc1Class.id == cdesc.id) && (sc1Class.type == cdesc.type)
                && (sc1Class.base == cdesc.base);
        });
        assert(tmp != sc2.cend());
        // compare property
        auto sc1Property = it->propertyDescriptors;
        auto sc2Property = tmp->propertyDescriptors;
        assert(sc1Property.size() == sc2Property.size());
        for (auto pit = sc1Property.cbegin(); pit != sc1Property.cend(); ++pit) {
            auto ptmp
                = std::find_if(sc2Property.cbegin(), sc2Property.cend(), [&pit](const nogdb::PropertyDescriptor& p) {
                      return (pit->name == p.name) && (pit->type == p.type) && (pit->id == p.id)
                          && (pit->inherited == p.inherited);
                  });
            assert(ptmp != sc2Property.cend());
        }
        // compare index
        auto sc1Index = it->indexDescriptors;
        auto sc2Index = tmp->indexDescriptors;
        assert(sc1Index.size() == sc2Index.size());
        for (auto iit = sc1Index.cbegin(); iit != sc1Index.cend(); ++iit) {
            auto itmp = std::find_if(sc2Index.cbegin(), sc2Index.cend(), [&iit](const nogdb::IndexDescriptor& i) {
                return (iit->id == i.id) && (iit->classId == i.classId) && (iit->propertyId == i.propertyId)
                    && (iit->unique == i.unique);
            });
            assert(itmp != sc2Index.cend());
        }
    }
}

void assert_ctx(const nogdb::Context& ctx1, const nogdb::Context& ctx2)
{
    auto txn1 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto txn2 = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto info1 = txn1.getDBInfo();
    auto info2 = txn2.getDBInfo();
    assert_dbinfo(info1, info2);
}

void test_context()
{
    std::string dbname { DATABASE_PATH };
    try {
        ctx = new nogdb::Context(dbname);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_ctx_move()
{
    auto schema = std::vector<ClassSchema> {};
    auto info = nogdb::DBInfo {};
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("files", nogdb::ClassType::VERTEX);
        txn.addProperty("files", "property", nogdb::PropertyType::TEXT);
        schema.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema.emplace_back(ClassSchema { txn, cdesc });
        }
        info = txn.getDBInfo();
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    delete ctx;

    {
        // move constructor
        nogdb::Context tmp1 = nogdb::Context(DATABASE_PATH);
        try {
            auto txn = tmp1.beginTxn(nogdb::TxnMode::READ_ONLY);
            auto schema_r = std::vector<ClassSchema> {};
            for (const auto& cdesc : txn.getClasses()) {
                schema_r.emplace_back(ClassSchema { txn, cdesc });
            }
            auto info_r = txn.getDBInfo();
            txn.rollback();
            assert_dbinfo(info, info_r);
            assert_schema(schema, schema_r);
        } catch (const nogdb::Error& ex) {
            std::cout << "\nError: " << ex.what() << std::endl;
            assert(false);
        }

        // move assignment
        nogdb::Context tmp2;
        tmp2 = std::move(tmp1);
        auto schema_r = std::vector<ClassSchema> {};
        try {
            auto txn = tmp2.beginTxn(nogdb::TxnMode::READ_ONLY);
            schema_r.clear();
            for (const auto& cdesc : txn.getClasses()) {
                schema_r.emplace_back(ClassSchema { txn, cdesc });
            }
            auto info_r = txn.getDBInfo();
            txn.rollback();
            assert_dbinfo(info, info_r);
            assert_schema(schema, schema_r);
        } catch (const nogdb::Error& ex) {
            std::cout << "\nError: " << ex.what() << std::endl;
            assert(false);
        }
    }

    try {
        ctx = new nogdb::Context(DATABASE_PATH);
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("files");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

/* reopening a database with schema only */
void test_reopen_ctx()
{
    auto schema = std::vector<ClassSchema> {};
    auto info = nogdb::DBInfo {};
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("files", nogdb::ClassType::VERTEX);
        txn.addProperty("files", "property1", nogdb::PropertyType::TEXT);
        txn.addProperty("files", "property2", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addClass("folders", nogdb::ClassType::VERTEX);
        txn.addProperty("folders", "property1", nogdb::PropertyType::BLOB);
        txn.addProperty("folders", "property2", nogdb::PropertyType::BIGINT);
        schema.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema.emplace_back(ClassSchema { txn, cdesc });
        }
        info = txn.getDBInfo();
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    delete ctx;

    try {
        ctx = new nogdb::Context(DATABASE_PATH);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    auto schema_r = std::vector<ClassSchema> {};
    auto info_r = nogdb::DBInfo {};
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        schema_r.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema_r.emplace_back(ClassSchema { txn, cdesc });
        }
        info_r = txn.getDBInfo();
        txn.rollback();
        assert_dbinfo(info, info_r);
        assert_schema(schema, schema_r);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

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

/* reopening a database with schema and records */
struct myobject {
    myobject() {};

    myobject(int x_, unsigned long long y_, double z_)
        : x { x_ }
        , y { y_ }
        , z { z_ }
    {
    }

    int x { 0 };
    unsigned long long y { 0 };
    double z { 0.0 };
};

void test_reopen_ctx_v2()
{
    auto schema = std::vector<ClassSchema> {};
    auto info = nogdb::DBInfo {};
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test1", nogdb::ClassType::VERTEX);
        txn.addProperty("test1", "property1", nogdb::PropertyType::TEXT);
        txn.addProperty("test1", "property2", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addClass("test2", nogdb::ClassType::VERTEX);
        txn.addProperty("test2", "property1", nogdb::PropertyType::REAL);
        txn.addProperty("test2", "property2", nogdb::PropertyType::BIGINT);
        txn.addProperty("test2", "property3", nogdb::PropertyType::BLOB);

        nogdb::Record r;
        r.set("property1", "hello1").set("property2", 15U);
        txn.addVertex("test1", r);
        r.set("property1", 42.42)
            .set("property2", 15LL)
            .set("property3", nogdb::Bytes { myobject { 42U, 42424242424242ULL, 42.42 } });
        txn.addVertex("test2", r);
        schema.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema.emplace_back(ClassSchema { txn, cdesc });
        }
        info = txn.getDBInfo();
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    delete ctx;

    try {
        ctx = new nogdb::Context(DATABASE_PATH);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    auto schema_r = std::vector<ClassSchema> {};
    auto info_r = nogdb::DBInfo {};
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        schema_r.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema_r.emplace_back(ClassSchema { txn, cdesc });
        }
        info_r = txn.getDBInfo();
        assert_dbinfo(info, info_r);
        assert_schema(schema, schema_r);

        nogdb::Record r;
        r.set("property1", "hello2").set("property2", 30U);
        txn.addVertex("test1", r);

        auto res = txn.find("test1").get();
        assert(res[0].record.get("property1").toText() == "hello1");
        assert(res[0].record.get("property2").toIntU() == 15U);
        assert(res[1].record.get("property1").toText() == "hello2");
        assert(res[1].record.get("property2").toIntU() == 30U);

        res = txn.find("test2").get();
        assert(res[0].record.get("property1").toReal() == 42.42);
        assert(res[0].record.get("property2").toBigInt() == 15LL);
        auto tmp = myobject {};
        res[0].record.get("property3").convertTo(tmp);
        assert(tmp.x == 42);
        assert(tmp.y == 42424242424242ULL);
        assert(tmp.z == 42.42);

        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("test1");
        txn.dropClass("test2");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

/* reopening a database with schema, records, and relations */
void test_reopen_ctx_v3()
{
    auto schema = std::vector<ClassSchema> {};
    auto info = nogdb::DBInfo {};
    auto tmp = nogdb::RecordDescriptor {};
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("test1", nogdb::ClassType::VERTEX);
        txn.addProperty("test1", "property1", nogdb::PropertyType::TEXT);
        txn.addProperty("test1", "property2", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addClass("test2", nogdb::ClassType::VERTEX);
        txn.addProperty("test2", "property1", nogdb::PropertyType::REAL);
        txn.addProperty("test2", "property2", nogdb::PropertyType::BIGINT);
        txn.addClass("test3", nogdb::ClassType::EDGE);
        txn.addProperty("test3", "property1", nogdb::PropertyType::INTEGER);

        nogdb::Record r1, r2;
        r1.set("property1", "hello1").set("property2", 15U);
        auto v1 = txn.addVertex("test1", r1);
        r1.set("property1", 42.42).set("property2", 15LL);
        auto v2 = txn.addVertex("test2", r1);
        r2.set("property1", 42);
        tmp = v2;
        auto e = txn.addEdge("test3", v1, v2, r2);

        schema.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema.emplace_back(ClassSchema { txn, cdesc });
        }
        info = txn.getDBInfo();
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    delete ctx;

    try {
        ctx = new nogdb::Context(DATABASE_PATH);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    auto info_r = nogdb::DBInfo {};
    auto schema_r = std::vector<ClassSchema> {};
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        schema_r.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema_r.emplace_back(ClassSchema { txn, cdesc });
        }
        info_r = txn.getDBInfo();
        assert_dbinfo(info, info_r);
        assert_schema(schema, schema_r);

        nogdb::Record r1, r2;
        r1.set("property1", "hello2").set("property2", 30U);
        auto v3 = txn.addVertex("test1", r1);

        r2.set("property1", 24);
        auto e = txn.addEdge("test3", v3, tmp, r2);

        auto res = txn.find("test1").get();
        assert(res[0].record.get("property1").toText() == "hello1");
        assert(res[0].record.get("property2").toIntU() == 15U);
        assert(res[1].record.get("property1").toText() == "hello2");
        assert(res[1].record.get("property2").toIntU() == 30U);

        res = txn.find("test2").get();
        assert(res[0].record.get("property1").toReal() == 42.42);
        assert(res[0].record.get("property2").toBigInt() == 15LL);

        res = txn.find("test3").get();
        assert(res[0].record.get("property1").toInt() == 42);
        assert(res[1].record.get("property1").toInt() == 24);

        auto res2 = txn.fetchSrc(res[0].descriptor);
        assert(res2.record.get("property1").toText() == "hello1");

        res2 = txn.fetchDst(res[0].descriptor);
        assert(res2.record.get("property1").toReal() == 42.42);

        res = txn.findInEdge(tmp).get();
        ASSERT_SIZE(res, 2);
        assert(res[0].record.get("property1").toInt() == 42);
        assert(res[1].record.get("property1").toInt() == 24);

        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("test1");
        txn.dropClass("test2");
        txn.dropClass("test3");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

/* reopening a database with schema, records, relations, and renaming class/property */
void test_reopen_ctx_v4()
{
    auto tmp = nogdb::RecordDescriptor {};
    auto t1 = nogdb::ClassDescriptor {};
    auto p1 = nogdb::PropertyDescriptor {};
    auto schema = std::vector<ClassSchema> {};
    auto info = nogdb::DBInfo {};
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        t1 = txn.addClass("test1", nogdb::ClassType::VERTEX);
        txn.addProperty("test1", "property1", nogdb::PropertyType::TEXT);
        txn.addProperty("test1", "property2", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addClass("test2", nogdb::ClassType::VERTEX);
        p1 = txn.addProperty("test2", "property1", nogdb::PropertyType::REAL);
        txn.addProperty("test2", "property2", nogdb::PropertyType::BIGINT);
        txn.addClass("test3", nogdb::ClassType::EDGE);
        txn.addProperty("test3", "property1", nogdb::PropertyType::INTEGER);

        nogdb::Record r1, r2;
        r1.set("property1", "hello1").set("property2", 15U);
        auto v1 = txn.addVertex("test1", r1);
        r1.set("property1", 42.42).set("property2", 15LL);
        auto v2 = txn.addVertex("test2", r1);
        r2.set("property1", 42);
        tmp = v2;
        auto e = txn.addEdge("test3", v1, v2, r2);

        schema.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema.emplace_back(ClassSchema { txn, cdesc });
        }
        info = txn.getDBInfo();

        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    delete ctx;

    auto schema_r = std::vector<ClassSchema> {};
    auto info_r = nogdb::DBInfo {};
    try {
        ctx = new nogdb::Context(DATABASE_PATH);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        schema_r.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema_r.emplace_back(ClassSchema { txn, cdesc });
        }
        info_r = txn.getDBInfo();
        assert_dbinfo(info, info_r);
        assert_schema(schema, schema_r);

        txn.renameClass("test1", "test01");
        txn.renameProperty("test2", "property1", "property01");

        schema_r.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema_r.emplace_back(ClassSchema { txn, cdesc });
        }
        info_r = txn.getDBInfo();
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    delete ctx;

    auto schema_rr = std::vector<ClassSchema> {};
    auto info_rr = nogdb::DBInfo {};
    try {
        ctx = new nogdb::Context(DATABASE_PATH);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
        schema_rr.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema_rr.emplace_back(ClassSchema { txn, cdesc });
        }
        info_rr = txn.getDBInfo();
        assert_dbinfo(info_rr, info_r);
        assert_schema(schema_rr, schema_r);

        auto cdesc = txn.getClass("test01");
        assert(cdesc.id == t1.id);
        assert(cdesc.type == t1.type);
        assert(txn.getProperties(cdesc).size() == 2);

        auto pdesc = txn.getProperty("test2", "property01");
        assert(pdesc.id == p1.id);
        assert(pdesc.type == p1.type);

        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("test01");
        txn.dropClass("test2");
        txn.dropClass("test3");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

/* reopening a database with schema, records, relations, and extended classes */
void test_reopen_ctx_v5()
{
    auto schema = std::vector<ClassSchema> {};
    auto info = nogdb::DBInfo {};
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("vertex1", nogdb::ClassType::VERTEX);
        txn.addProperty("vertex1", "prop1", nogdb::PropertyType::INTEGER);
        txn.addSubClassOf("vertex1", "vertex2");
        txn.addProperty("vertex2", "prop2", nogdb::PropertyType::TEXT);
        txn.addSubClassOf("vertex1", "vertex3");
        txn.addProperty("vertex3", "prop3", nogdb::PropertyType::REAL);

        txn.addClass("edge1", nogdb::ClassType::EDGE);
        txn.addProperty("edge1", "prop1", nogdb::PropertyType::INTEGER);
        txn.addSubClassOf("edge1", "edge2");
        txn.addProperty("edge2", "prop2", nogdb::PropertyType::TEXT);
        txn.addSubClassOf("edge1", "edge3");
        txn.addProperty("edge3", "prop3", nogdb::PropertyType::REAL);

        auto v1 = txn.addVertex("vertex2", nogdb::Record {}.set("prop1", 10).set("prop2", "hello"));
        auto v2 = txn.addVertex("vertex3", nogdb::Record {}.set("prop1", 20).set("prop3", 42.41));
        txn.addEdge("edge2", v1, v2, nogdb::Record {}.set("prop1", 100).set("prop2", "world"));
        txn.addEdge("edge3", v2, v1, nogdb::Record {}.set("prop1", 200).set("prop3", -41.42));

        schema.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema.emplace_back(ClassSchema { txn, cdesc });
        }
        info = txn.getDBInfo();
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    delete ctx;

    auto schema_r = std::vector<ClassSchema> {};
    auto info_r = nogdb::DBInfo {};
    try {
        ctx = new nogdb::Context(DATABASE_PATH);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        schema_r.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema_r.emplace_back(ClassSchema { txn, cdesc });
        }
        info_r = txn.getDBInfo();
        assert_dbinfo(info, info_r);
        assert_schema(schema, schema_r);

        auto res = txn.find("vertex1").get();
        ASSERT_SIZE(res, 0);
        res = txn.findSubClassOf("vertex1").get();
        ASSERT_SIZE(res, 2);
        res = txn.find("edge1").get();
        ASSERT_SIZE(res, 0);
        res = txn.findSubClassOf("edge1").get();
        ASSERT_SIZE(res, 2);

        txn.dropClass("vertex1");
        txn.dropClass("vertex2");
        txn.dropClass("vertex3");
        txn.dropClass("edge1");
        txn.dropClass("edge2");
        txn.dropClass("edge3");

        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

/* reopening a database with schema, records, extended classes, and indexing */
void test_reopen_ctx_v6()
{
    auto schema = std::vector<ClassSchema> {};
    auto info = nogdb::DBInfo {};
    nogdb::ClassDescriptor vertex1, vertex2, edge1, edge2;
    nogdb::PropertyDescriptor propVertex1, propVertex2, propEdge1, propEdge2;
    nogdb::IndexDescriptor v_index1, v_index2, v_index3, e_index1, e_index2, e_index3;
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        vertex1 = txn.addClass("index_vertex1", nogdb::ClassType::VERTEX);
        propVertex1 = txn.addProperty("index_vertex1", "prop1", nogdb::PropertyType::INTEGER);
        vertex2 = txn.addSubClassOf("index_vertex1", "index_vertex2");
        propVertex2 = txn.addProperty("index_vertex2", "prop2", nogdb::PropertyType::TEXT);

        edge1 = txn.addClass("index_edge1", nogdb::ClassType::EDGE);
        propEdge1 = txn.addProperty("index_edge1", "prop1", nogdb::PropertyType::UNSIGNED_INTEGER);
        edge2 = txn.addSubClassOf("index_edge1", "index_edge2");
        propEdge2 = txn.addProperty("index_edge2", "prop2", nogdb::PropertyType::REAL);

        v_index1 = txn.addIndex("index_vertex1", "prop1", true);
        v_index2 = txn.addIndex("index_vertex2", "prop1", false);
        v_index3 = txn.addIndex("index_vertex2", "prop2", true);

        e_index1 = txn.addIndex("index_edge1", "prop1", true);
        e_index2 = txn.addIndex("index_edge2", "prop1", false);
        e_index3 = txn.addIndex("index_edge2", "prop2", true);

        schema.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema.emplace_back(ClassSchema { txn, cdesc });
        }
        info = txn.getDBInfo();

        assert(v_index1 == txn.getIndex(vertex1.name, propVertex1.name));
        assert(v_index2 == txn.getIndex(vertex2.name, propVertex1.name));
        assert(v_index3 == txn.getIndex(vertex2.name, propVertex2.name));
        assert(e_index1 == txn.getIndex(edge1.name, propEdge1.name));
        assert(e_index2 == txn.getIndex(edge2.name, propEdge1.name));
        assert(e_index3 == txn.getIndex(edge2.name, propEdge2.name));

        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    delete ctx;

    auto schema_r = std::vector<ClassSchema> {};
    auto info_r = nogdb::DBInfo {};
    try {
        ctx = new nogdb::Context(DATABASE_PATH);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        for (const auto& cdesc : txn.getClasses()) {
            schema_r.emplace_back(ClassSchema { txn, cdesc });
        }
        info_r = txn.getDBInfo();
        assert_dbinfo(info, info_r);
        assert_schema(schema, schema_r);

        txn.dropIndex("index_vertex2", "prop1");
        txn.dropIndex("index_edge2", "prop1");

        schema.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema.emplace_back(ClassSchema { txn, cdesc });
        }
        info = txn.getDBInfo();
        assert(txn.getIndexes(vertex1).size() == 1);
        assert(txn.getIndexes(vertex2).size() == 1);
        assert(txn.getIndexes(edge1).size() == 1);
        assert(txn.getIndexes(edge2).size() == 1);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    delete ctx;

    try {
        ctx = new nogdb::Context(DATABASE_PATH);
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        schema_r.clear();
        for (const auto& cdesc : txn.getClasses()) {
            schema_r.emplace_back(ClassSchema { txn, cdesc });
        }
        info_r = txn.getDBInfo();
        assert_dbinfo(info, info_r);
        assert_schema(schema, schema_r);
        txn.rollback();
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

// void test_locked_ctx() {
//  try {
//    new nogdb::Context(DATABASE_PATH);
//    assert(false);
//  } catch (const nogdb::Error &ex) {
//    REQUIRE(ex, NOGDB_CTX_IS_LOCKED, "NOGDB_CTX_IS_LOCKED");
//  }
//
//  delete ctx;
//
//  try {
//    ctx = new nogdb::Context(DATABASE_PATH);
//  } catch (const nogdb::Error &ex) {
//    std::cout << "\nError: " << ex.what() << std::endl;
//    assert(false);
//  }
//}

void test_invalid_ctx()
{
    auto tmp_ctx = ctx;
    ctx = nullptr;
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
    try {
        txn.addClass("invalid", nogdb::ClassType::VERTEX);
        ctx = tmp_ctx;
        assert(false);
    } catch (const nogdb::Error& ex) {
        std::cout << ex.what() << '\n';
    }
    txn.rollback();
}

void test_multiple_ctx()
{
    auto dbPath = ctx->getDBPath();

    try {
        auto ctx1 = new nogdb::Context { dbPath };
        auto ctx2 = new nogdb::Context { dbPath };

        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("multiCtx_node", nogdb::ClassType::VERTEX);
        txn.addProperty("multiCtx_node", "name", nogdb::PropertyType::TEXT);
        txn.addClass("multiCtx_edge", nogdb::ClassType::EDGE);

        auto v1 = txn.addVertex("multiCtx_node", nogdb::Record {}.set("name", "v1"));
        auto v2 = txn.addVertex("multiCtx_node", nogdb::Record {}.set("name", "v2"));
        auto e = txn.addEdge("multiCtx_edge", v1, v2);

        txn.commit();

        auto txn1 = ctx1->beginTxn(nogdb::TxnMode::READ_ONLY);
        auto txn2 = ctx2->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto res1 = txn1.find("multiCtx_node").get();
        assert(res1.size() == 2);
        auto res2 = txn2.find("multiCtx_node").get();
        assert(res2.size() == 2);
        res1 = txn1.find("multiCtx_edge").get();
        assert(res1.size() == 1);
        res2 = txn2.find("multiCtx_edge").get();
        assert(res2.size() == 1);
        res1 = txn1.findEdge(v1).get();
        assert(res1[0].descriptor == e);
        res2 = txn2.findEdge(v2).get();
        assert(res2[0].descriptor == e);
        res1 = txn1.fetchSrcDst(e);
        assert(res1[0].descriptor == v1);
        assert(res1[1].descriptor == v2);
        res2 = txn2.fetchSrcDst(e);
        assert(res2[0].descriptor == v1);
        assert(res2[1].descriptor == v2);

        txn1.rollback();
        txn2.rollback();

        delete ctx1;
        delete ctx2;

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto ctx1 = new nogdb::Context { dbPath };
        auto ctx2 = new nogdb::Context { dbPath };

        auto txn1 = ctx1->beginTxn(nogdb::TxnMode::READ_WRITE);
        auto txn2 = ctx2->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto v1 = txn1.find("multiCtx_node").where(nogdb::Condition("name").eq("v1")).get();
        assert(v1.size() == 1);
        auto v3 = txn1.addVertex("multiCtx_node", nogdb::Record {}.set("name", "v3"));
        txn1.addEdge("multiCtx_edge", v1[0].descriptor, v3);

        auto res = txn2.find("multiCtx_node").where(nogdb::Condition("name").eq("v3")).get();
        assert(res.empty());
        res = txn2.find("multiCtx_edge").get();
        assert(res.size() == 1);

        txn1.commit();
        txn2.rollback();

        txn2 = ctx2->beginTxn(nogdb::TxnMode::READ_ONLY);
        res = txn2.find("multiCtx_node").where(nogdb::Condition("name").eq("v3")).get();
        assert(!res.empty());
        assert(res[0].descriptor == v3);
        res = txn2.find("multiCtx_edge").get();
        assert(res.size() == 2);

        txn2.rollback();

        delete ctx1;
        delete ctx2;

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto ctx1 = new nogdb::Context { dbPath };
        auto ctx2 = new nogdb::Context { dbPath };

        auto txn1 = ctx1->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn1.commit();

        txn1 = ctx1->beginTxn(nogdb::TxnMode::READ_ONLY);

        auto txn2 = ctx2->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn2.commit();

        txn1.rollback();

        delete ctx1;
        delete ctx2;

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}
