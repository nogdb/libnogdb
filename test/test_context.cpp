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

void assert_dbinfo(const nogdb::DBInfo &info1, const nogdb::DBInfo &info2) {
	assert(info1.numClass == info2.numClass);
	assert(info1.numProperty == info2.numProperty);
	assert(info1.numIndex == info2.numIndex);
	assert(info1.dbPath == info2.dbPath);
	assert(info1.maxClassId == info2.maxClassId);
	assert(info1.maxPropertyId == info2.maxPropertyId);
	assert(info1.maxIndexId == info2.maxIndexId);
	assert(info1.maxDB == info2.maxDB);
}

void assert_schema(const std::vector<nogdb::ClassDescriptor>& sc1, const std::vector<nogdb::ClassDescriptor>& sc2) {
	assert(sc1.size() == sc2.size());
	auto map_compare = [](const nogdb::IndexInfo& lhs, const nogdb::IndexInfo& rhs) {
		return lhs.size() == rhs.size()
			   && std::equal(lhs.begin(), lhs.end(), rhs.begin());
	};
	for(auto it = sc1.cbegin(); it != sc1.cend(); ++it) {
		auto tmp = std::find_if(sc2.cbegin(), sc2.cend(), [&it](const nogdb::ClassDescriptor& c) {
			return (it->name == c.name) && (it->id == c.id) && (it->type == c.type);
		});
		assert(tmp != sc2.cend());
		assert(it->properties.size() == tmp->properties.size());
		for(auto inner_it = it->properties.cbegin(); inner_it != it->properties.cend(); ++inner_it) {
			assert(tmp->properties.find(inner_it->first) != tmp->properties.cend());
			assert(inner_it->second.type == tmp->properties.at(inner_it->first).type);
			map_compare(inner_it->second.indexInfo, tmp->properties.at(inner_it->first).indexInfo);
		}
		assert(it->super == tmp->super);
		for(auto inner_it = it->sub.cbegin(); inner_it != it->sub.cend(); ++inner_it) {
			assert(std::find(tmp->sub.cbegin(), tmp->sub.cend(), *inner_it) != tmp->sub.cend());
		}
	}
}

void assert_ctx(const nogdb::Context &ctx1, const nogdb::Context &ctx2) {
	auto txn1 = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
	auto txn2 = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
	auto info1 = nogdb::Db::getDbInfo(txn1);
	auto info2 = nogdb::Db::getDbInfo(txn2);
	assert_dbinfo(info1, info2);
}

void test_context() {
	std::string dbname{DATABASE_PATH};
	try {
		ctx = new nogdb::Context(dbname);
	} catch (const nogdb::Error &ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}
}

void test_ctx_copy() {
	nogdb::Context tmp1 = *ctx, tmp2;
	tmp2 = *ctx;
	assert_ctx(*ctx, tmp1);
	assert_ctx(tmp1, tmp2);
	try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		nogdb::Class::create(txn, "ctx_copy_test", nogdb::ClassType::VERTEX);
		nogdb::Property::add(txn, "ctx_copy_test", "prop1", nogdb::PropertyType::TEXT);
		nogdb::Property::add(txn, "ctx_copy_test", "prop2", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.commit();
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}
	assert_ctx(*ctx, tmp1);
	assert_ctx(tmp1, tmp2);

	try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		nogdb::Class::drop(txn, "ctx_copy_test");
        txn.commit();
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}
	assert_ctx(*ctx, tmp1);
	assert_ctx(tmp1, tmp2);
}

void test_ctx_copy_v2() {
    auto schema = std::vector<nogdb::ClassDescriptor>{};
	auto info = nogdb::DBInfo{};
	try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		nogdb::Class::create(txn, "files", nogdb::ClassType::VERTEX);
		nogdb::Property::add(txn, "files", "property", nogdb::PropertyType::TEXT);
		schema = nogdb::Db::getSchema(txn);
		info = nogdb::Db::getDbInfo(txn);
		//show_schema(ctx, txn);
		txn.commit();
	} catch (const nogdb::Error &ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	delete ctx;

	{
        // move constructor
        nogdb::Context tmp1 = nogdb::Context(DATABASE_PATH);
        try {
            auto txn = nogdb::Txn{tmp1, nogdb::Txn::Mode::READ_ONLY};
            auto schema_r = nogdb::Db::getSchema(txn);
            auto info_r = nogdb::Db::getDbInfo(txn);
			//show_schema(&tmp1, txn);
            txn.rollback();
            assert_dbinfo(info, info_r);
            assert_schema(schema, schema_r);
        } catch (const nogdb::Error &ex) {
            std::cout << "\nError: " << ex.what() << std::endl;
            assert(false);
        }

        // move assignment
        nogdb::Context tmp2;
        tmp2 = std::move(tmp1);
        try {
            auto txn = nogdb::Txn{tmp2, nogdb::Txn::Mode::READ_ONLY};
            auto schema_r = nogdb::Db::getSchema(txn);
            auto info_r = nogdb::Db::getDbInfo(txn);
            txn.rollback();
            assert_dbinfo(info, info_r);
            assert_schema(schema, schema_r);
        } catch (const nogdb::Error &ex) {
            std::cout << "\nError: " << ex.what() << std::endl;
            assert(false);
        }
    }

	try {
		ctx = new nogdb::Context(DATABASE_PATH);
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		nogdb::Class::drop(txn, "files");
        txn.commit();
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}
}

/* reopening a database with schema only */
void test_reopen_ctx() {
	auto schema = std::vector<nogdb::ClassDescriptor>{};
	auto info = nogdb::DBInfo{};
	try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		nogdb::Class::create(txn, "files", nogdb::ClassType::VERTEX);
		nogdb::Property::add(txn, "files", "property1", nogdb::PropertyType::TEXT);
		nogdb::Property::add(txn, "files", "property2", nogdb::PropertyType::UNSIGNED_INTEGER);
		nogdb::Class::create(txn, "folders", nogdb::ClassType::VERTEX);
		nogdb::Property::add(txn, "folders", "property1", nogdb::PropertyType::BLOB);
		nogdb::Property::add(txn, "folders", "property2", nogdb::PropertyType::BIGINT);
		schema = nogdb::Db::getSchema(txn);
		info = nogdb::Db::getDbInfo(txn);
		txn.commit();
	}  catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	delete ctx;

	try {
		ctx = new nogdb::Context(DATABASE_PATH);
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	auto schema_r = std::vector<nogdb::ClassDescriptor>{};
	auto info_r = nogdb::DBInfo{};
	try {
		auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
		schema_r = nogdb::Db::getSchema(txn);
		info_r = nogdb::Db::getDbInfo(txn);
		txn.rollback();
		assert_dbinfo(info, info_r);
		assert_schema(schema, schema_r);
	}  catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		nogdb::Class::drop(txn, "files");
		nogdb::Class::drop(txn, "folders");
        txn.commit();
	}  catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}
}

/* reopening a database with schema and records */
struct myobject {
	myobject(){};
	myobject(int x_, unsigned long long y_, double z_): x{x_}, y{y_}, z{z_}{}
	int x{0};
	unsigned long long y{0};
	double z{0.0};
};

void test_reopen_ctx_v2() {
	auto schema = std::vector<nogdb::ClassDescriptor>{};
	auto info = nogdb::DBInfo{};
	try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		nogdb::Class::create(txn, "test1", nogdb::ClassType::VERTEX);
		nogdb::Property::add(txn, "test1", "property1", nogdb::PropertyType::TEXT);
		nogdb::Property::add(txn, "test1", "property2", nogdb::PropertyType::UNSIGNED_INTEGER);
		nogdb::Class::create(txn, "test2", nogdb::ClassType::VERTEX);
		nogdb::Property::add(txn, "test2", "property1", nogdb::PropertyType::REAL);
		nogdb::Property::add(txn, "test2", "property2", nogdb::PropertyType::BIGINT);
		nogdb::Property::add(txn, "test2", "property3", nogdb::PropertyType::BLOB);

		nogdb::Record r;
		r.set("property1", "hello1").set("property2", 15U);
		nogdb::Vertex::create(txn, "test1", r);
		r.set("property1", 42.42).set("property2", 15LL).set("property3", nogdb::Bytes{myobject{42U, 42424242424242ULL, 42.42}});
		nogdb::Vertex::create(txn, "test2", r);
		schema = nogdb::Db::getSchema(txn);
		info = nogdb::Db::getDbInfo(txn);
		txn.commit();
	}  catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	delete ctx;

	try {
		ctx = new nogdb::Context(DATABASE_PATH);
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	auto schema_r = std::vector<nogdb::ClassDescriptor>{};
	auto info_r = nogdb::DBInfo{};
	try {
		auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		schema_r = nogdb::Db::getSchema(txn);
		info_r = nogdb::Db::getDbInfo(txn);
		assert_dbinfo(info, info_r);
		assert_schema(schema, schema_r);

		nogdb::Record r;
		r.set("property1", "hello2").set("property2", 30U);
		nogdb::Vertex::create(txn, "test1", r);

		auto res = nogdb::Vertex::get(txn, "test1");
		assert(res[0].record.get("property1").toText() == "hello1");
		assert(res[0].record.get("property2").toIntU() == 15U);
		assert(res[1].record.get("property1").toText() == "hello2");
		assert(res[1].record.get("property2").toIntU() == 30U);

		res = nogdb::Vertex::get(txn, "test2");
		assert(res[0].record.get("property1").toReal() == 42.42);
		assert(res[0].record.get("property2").toBigInt() == 15LL);
		auto tmp = myobject{};
		res[0].record.get("property3").convertTo(tmp);
		assert(tmp.x == 42);
		assert(tmp.y == 42424242424242ULL);
		assert(tmp.z == 42.42);

		txn.commit();
	}  catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		nogdb::Class::drop(txn, "test1");
		nogdb::Class::drop(txn, "test2");
        txn.commit();
	}  catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}
}

/* reopening a database with schema, records, and relations */
void test_reopen_ctx_v3() {
	auto schema = std::vector<nogdb::ClassDescriptor>{};
	auto info = nogdb::DBInfo{};
	auto tmp = nogdb::RecordDescriptor{};
	try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		nogdb::Class::create(txn, "test1", nogdb::ClassType::VERTEX);
		nogdb::Property::add(txn, "test1", "property1", nogdb::PropertyType::TEXT);
		nogdb::Property::add(txn, "test1", "property2", nogdb::PropertyType::UNSIGNED_INTEGER);
		nogdb::Class::create(txn, "test2", nogdb::ClassType::VERTEX);
		nogdb::Property::add(txn, "test2", "property1", nogdb::PropertyType::REAL);
		nogdb::Property::add(txn, "test2", "property2", nogdb::PropertyType::BIGINT);
		nogdb::Class::create(txn, "test3", nogdb::ClassType::EDGE);
		nogdb::Property::add(txn, "test3", "property1", nogdb::PropertyType::INTEGER);

		nogdb::Record r1, r2;
		r1.set("property1", "hello1").set("property2", 15U);
		auto v1 = nogdb::Vertex::create(txn, "test1", r1);
		r1.set("property1", 42.42).set("property2", 15LL);
		auto v2 = nogdb::Vertex::create(txn, "test2", r1);
		r2.set("property1", 42);
		tmp = v2;
		auto e = nogdb::Edge::create(txn, "test3", v1, v2, r2);

		schema = nogdb::Db::getSchema(txn);
		info = nogdb::Db::getDbInfo(txn);
		txn.commit();
	}  catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	delete ctx;

	try {
		ctx = new nogdb::Context(DATABASE_PATH);
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	auto schema_r = std::vector<nogdb::ClassDescriptor>{};
	auto info_r = nogdb::DBInfo{};
	try {
		auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		schema_r = nogdb::Db::getSchema(txn);
		info_r = nogdb::Db::getDbInfo(txn);
		assert_dbinfo(info, info_r);
		assert_schema(schema, schema_r);

		nogdb::Record r1, r2;
		r1.set("property1", "hello2").set("property2", 30U);
		auto v3 = nogdb::Vertex::create(txn, "test1", r1);

		r2.set("property1", 24);
		auto e = nogdb::Edge::create(txn, "test3", v3, tmp, r2);

		auto res = nogdb::Vertex::get(txn, "test1");
		assert(res[0].record.get("property1").toText() == "hello1");
		assert(res[0].record.get("property2").toIntU() == 15U);
		assert(res[1].record.get("property1").toText() == "hello2");
		assert(res[1].record.get("property2").toIntU() == 30U);

		res = nogdb::Vertex::get(txn, "test2");
		assert(res[0].record.get("property1").toReal() == 42.42);
		assert(res[0].record.get("property2").toBigInt() == 15LL);

		res = nogdb::Edge::get(txn, "test3");
		assert(res[0].record.get("property1").toInt() == 42);
		assert(res[1].record.get("property1").toInt() == 24);

		auto res2 = nogdb::Edge::getSrc(txn, res[0].descriptor);
		assert(res2.record.get("property1").toText() == "hello1");

		res = nogdb::Vertex::getInEdge(txn, tmp);
		assertSize(res, 2);
		assert(res[0].record.get("property1").toInt() == 24);
		assert(res[1].record.get("property1").toInt() == 42);

		txn.commit();
	}  catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		nogdb::Class::drop(txn, "test1");
		nogdb::Class::drop(txn, "test2");
		nogdb::Class::drop(txn, "test3");
        txn.commit();
	}  catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}
}

/* reopening a database with schema, records, relations, and renaming class/property */
void test_reopen_ctx_v4() {
	auto tmp = nogdb::RecordDescriptor{};
	auto t1 = nogdb::ClassDescriptor{};
	auto p1 = nogdb::PropertyDescriptor{};
	auto schema = std::vector<nogdb::ClassDescriptor>{};
	auto info = nogdb::DBInfo{};
	try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		t1 = nogdb::Class::create(txn, "test1", nogdb::ClassType::VERTEX);
		nogdb::Property::add(txn, "test1", "property1", nogdb::PropertyType::TEXT);
		nogdb::Property::add(txn, "test1", "property2", nogdb::PropertyType::UNSIGNED_INTEGER);
		nogdb::Class::create(txn, "test2", nogdb::ClassType::VERTEX);
		p1 = nogdb::Property::add(txn, "test2", "property1", nogdb::PropertyType::REAL);
		nogdb::Property::add(txn, "test2", "property2", nogdb::PropertyType::BIGINT);
		nogdb::Class::create(txn, "test3", nogdb::ClassType::EDGE);
		nogdb::Property::add(txn, "test3", "property1", nogdb::PropertyType::INTEGER);

		nogdb::Record r1, r2;
		r1.set("property1", "hello1").set("property2", 15U);
		auto v1 = nogdb::Vertex::create(txn, "test1", r1);
		r1.set("property1", 42.42).set("property2", 15LL);
		auto v2 = nogdb::Vertex::create(txn, "test2", r1);
		r2.set("property1", 42);
		tmp = v2;
		auto e = nogdb::Edge::create(txn, "test3", v1, v2, r2);

		schema = nogdb::Db::getSchema(txn);
		info = nogdb::Db::getDbInfo(txn);

		txn.commit();
	}  catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	delete ctx;

	auto schema_r = std::vector<nogdb::ClassDescriptor>{};
	auto info_r = nogdb::DBInfo{};
	try {
		ctx = new nogdb::Context(DATABASE_PATH);
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	try {
		auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		schema_r = nogdb::Db::getSchema(txn);
		info_r = nogdb::Db::getDbInfo(txn);
		assert_dbinfo(info, info_r);
		assert_schema(schema, schema_r);

		nogdb::Class::alter(txn, "test1", "test01");
		nogdb::Property::alter(txn, "test2", "property1", "property01");

		schema_r = nogdb::Db::getSchema(txn);
		info_r = nogdb::Db::getDbInfo(txn);
		txn.commit();
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	delete ctx;

	auto schema_rr = std::vector<nogdb::ClassDescriptor>{};
	auto info_rr = nogdb::DBInfo{};
	try {
		ctx = new nogdb::Context(DATABASE_PATH);
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	try {
		auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
		schema_rr = nogdb::Db::getSchema(txn);
		info_rr = nogdb::Db::getDbInfo(txn);
		assert_dbinfo(info_rr, info_r);
		assert_schema(schema_rr, schema_r);

		auto cdesc = nogdb::Db::getSchema(txn, "test01");
		assert(cdesc.id == t1.id);
		assert(cdesc.type == t1.type);
		assert(cdesc.properties.size() == 2);

		auto pdesc = nogdb::Db::getSchema(txn, "test2").properties.at("property01");
		assert(pdesc.id == p1.id);
		assert(pdesc.type == p1.type);

		txn.rollback();
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		nogdb::Class::drop(txn, "test01");
		nogdb::Class::drop(txn, "test2");
		nogdb::Class::drop(txn, "test3");
        txn.commit();
	}  catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}
}

/* reopening a database with schema, records, relations, and extended classes */
void test_reopen_ctx_v5() {
	auto schema = std::vector<nogdb::ClassDescriptor>{};
	auto info = nogdb::DBInfo{};
	try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		nogdb::Class::create(txn, "vertex1", nogdb::ClassType::VERTEX);
		nogdb::Property::add(txn, "vertex1", "prop1", nogdb::PropertyType::INTEGER);
		nogdb::Class::createExtend(txn, "vertex2", "vertex1");
		nogdb::Property::add(txn, "vertex2", "prop2", nogdb::PropertyType::TEXT);
		nogdb::Class::createExtend(txn, "vertex3", "vertex1");
		nogdb::Property::add(txn, "vertex3", "prop3", nogdb::PropertyType::REAL);

		nogdb::Class::create(txn, "edge1", nogdb::ClassType::EDGE);
		nogdb::Property::add(txn, "edge1", "prop1", nogdb::PropertyType::INTEGER);
		nogdb::Class::createExtend(txn, "edge2", "edge1");
		nogdb::Property::add(txn, "edge2", "prop2", nogdb::PropertyType::TEXT);
		nogdb::Class::createExtend(txn, "edge3", "edge1");
		nogdb::Property::add(txn, "edge3", "prop3", nogdb::PropertyType::REAL);

		auto v1 = nogdb::Vertex::create(txn, "vertex2", nogdb::Record{}.set("prop1", 10).set("prop2", "hello"));
		auto v2 = nogdb::Vertex::create(txn, "vertex3", nogdb::Record{}.set("prop1", 20).set("prop3", 42.41));
		nogdb::Edge::create(txn, "edge2", v1, v2, nogdb::Record{}.set("prop1", 100).set("prop2", "world"));
		nogdb::Edge::create(txn, "edge3", v2, v1, nogdb::Record{}.set("prop1", 200).set("prop3", -41.42));

		schema = nogdb::Db::getSchema(txn);
		info = nogdb::Db::getDbInfo(txn);
		txn.commit();
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	delete ctx;

	auto schema_r = std::vector<nogdb::ClassDescriptor>{};
	auto info_r = nogdb::DBInfo{};
	try {
		ctx = new nogdb::Context(DATABASE_PATH);
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	try {
		auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		schema_r = nogdb::Db::getSchema(txn);
		info_r = nogdb::Db::getDbInfo(txn);
		assert_dbinfo(info, info_r);
		assert_schema(schema, schema_r);

		auto res = nogdb::Vertex::get(txn, "vertex1");
		assertSize(res, 2);
		res = nogdb::Edge::get(txn, "edge1");
		assertSize(res, 2);

		nogdb::Class::drop(txn, "vertex1");
		nogdb::Class::drop(txn, "vertex2");
		nogdb::Class::drop(txn, "vertex3");
		nogdb::Class::drop(txn, "edge1");
		nogdb::Class::drop(txn, "edge2");
		nogdb::Class::drop(txn, "edge3");

        txn.commit();
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}
}

/* reopening a database with schema, records, extended classes, and indexing */
void test_reopen_ctx_v6() {
	auto schema = std::vector<nogdb::ClassDescriptor>{};
	auto info = nogdb::DBInfo{};
	try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		nogdb::Class::create(txn, "index_vertex1", nogdb::ClassType::VERTEX);
		nogdb::Property::add(txn, "index_vertex1", "prop1", nogdb::PropertyType::INTEGER);
		nogdb::Class::createExtend(txn, "index_vertex2", "index_vertex1");
		nogdb::Property::add(txn, "index_vertex2", "prop2", nogdb::PropertyType::TEXT);

		nogdb::Class::create(txn, "index_edge1", nogdb::ClassType::EDGE);
		nogdb::Property::add(txn, "index_edge1", "prop1", nogdb::PropertyType::UNSIGNED_INTEGER);
		nogdb::Class::createExtend(txn, "index_edge2", "index_edge1");
		nogdb::Property::add(txn, "index_edge2", "prop2", nogdb::PropertyType::REAL);

		nogdb::Property::createIndex(txn, "index_vertex1", "prop1", true);
		nogdb::Property::createIndex(txn, "index_vertex2", "prop1", false);
		nogdb::Property::createIndex(txn, "index_vertex2", "prop2", true);

		nogdb::Property::createIndex(txn, "index_edge1", "prop1", true);
		nogdb::Property::createIndex(txn, "index_edge2", "prop1", false);
		nogdb::Property::createIndex(txn, "index_edge2", "prop2", true);

		schema = nogdb::Db::getSchema(txn);
		info = nogdb::Db::getDbInfo(txn);
		for(const auto& property: nogdb::Db::getSchema(txn, "index_vertex1").properties) {
			if (property.first == "prop1")
				assert(property.second.indexInfo.size() == 2);
			else
				assert(property.second.indexInfo.size() == 1);
		}
		for(const auto& property: nogdb::Db::getSchema(txn, "index_vertex2").properties) {
			if (property.first == "prop1")
				assert(property.second.indexInfo.size() == 2);
			else
				assert(property.second.indexInfo.size() == 1);
		}
		for(const auto& property: nogdb::Db::getSchema(txn, "index_edge1").properties) {
			if (property.first == "prop1")
				assert(property.second.indexInfo.size() == 2);
			else
				assert(property.second.indexInfo.size() == 1);
		}
		for(const auto& property: nogdb::Db::getSchema(txn, "index_edge2").properties) {
			if (property.first == "prop1")
				assert(property.second.indexInfo.size() == 2);
			else
				assert(property.second.indexInfo.size() == 1);
		}
        txn.commit();
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	delete ctx;

	auto schema_r = std::vector<nogdb::ClassDescriptor>{};
	auto info_r = nogdb::DBInfo{};
	try {
		ctx = new nogdb::Context(DATABASE_PATH);
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		schema_r = nogdb::Db::getSchema(txn);
		info_r = nogdb::Db::getDbInfo(txn);
		assert_dbinfo(info, info_r);
		assert_schema(schema, schema_r);

		nogdb::Property::dropIndex(txn, "index_vertex2", "prop1");
		nogdb::Property::dropIndex(txn, "index_edge2", "prop1");

		schema = nogdb::Db::getSchema(txn);
		info = nogdb::Db::getDbInfo(txn);
		for(const auto& property: nogdb::Db::getSchema(txn, "index_vertex1").properties) {
			assert(property.second.indexInfo.size() == 1);
		}
		for(const auto& property: nogdb::Db::getSchema(txn, "index_vertex2").properties) {
			assert(property.second.indexInfo.size() == 1);
		}
		for(const auto& property: nogdb::Db::getSchema(txn, "index_edge1").properties) {
			assert(property.second.indexInfo.size() == 1);
		}
		for(const auto& property: nogdb::Db::getSchema(txn, "index_edge2").properties) {
			assert(property.second.indexInfo.size() == 1);
		}
        txn.commit();
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

	delete ctx;

	try {
		ctx = new nogdb::Context(DATABASE_PATH);
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
		schema_r = nogdb::Db::getSchema(txn);
		info_r = nogdb::Db::getDbInfo(txn);
		assert_dbinfo(info, info_r);
		assert_schema(schema, schema_r);
        txn.rollback();
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}

}

void test_locked_ctx() {
	try {
		new nogdb::Context(DATABASE_PATH);
		assert(false);
	} catch(const nogdb::Error& ex) {
		REQUIRE(ex, NOGDB_CTX_IS_LOCKED, "NOGDB_CTX_IS_LOCKED");
	}

	delete ctx;

	try {
		ctx = new nogdb::Context(DATABASE_PATH);
	} catch(const nogdb::Error& ex) {
		std::cout << "\nError: " << ex.what() << std::endl;
		assert(false);
	}
}

void test_invalid_ctx() {
	auto tmp_ctx = ctx;
	ctx = nullptr;
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
	try {
		nogdb::Class::create(txn, "invalid", nogdb::ClassType::VERTEX);
		ctx = tmp_ctx;
		assert(false);
	} catch(const nogdb::Error& ex) {
		std::cout << ex.what() << '\n';
	}
    txn.rollback();
}
