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

void assert_class(const nogdb::Txn &txn, const std::string &className, const std::string &superClassName,
                  size_t sizeOfSubClasses, size_t sizeOfProperties) {
  auto res = nogdb::DB::getClass(txn, className);
  auto superId = (superClassName == "")? 0 : nogdb::DB::getClass(txn, superClassName).id;
  assert(res.base == superId);
  auto properties = nogdb::DB::getProperties(txn, res);
  assert(properties.size() == sizeOfProperties);
  auto subClassCount = size_t{0};
  for (const auto &classDesc: nogdb::DB::getClasses(txn)) {
    subClassCount += classDesc.base == res.id;
  }
  assert(subClassCount == sizeOfSubClasses);
}

void init_all_extended_classes() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "employees", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "employees", "name", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "employees", "age", nogdb::PropertyType::UNSIGNED_INTEGER);
    nogdb::Property::add(txn, "employees", "salary", nogdb::PropertyType::UNSIGNED_BIGINT);
    nogdb::Class::createExtend(txn, "backends", "employees");
    nogdb::Property::add(txn, "backends", "cpp_skills", nogdb::PropertyType::INTEGER);
    nogdb::Property::add(txn, "backends", "js_skills", nogdb::PropertyType::INTEGER);
    nogdb::Class::createExtend(txn, "frontends", "employees");
    nogdb::Property::add(txn, "frontends", "html_skills", nogdb::PropertyType::INTEGER);
    nogdb::Property::add(txn, "frontends", "js_skills", nogdb::PropertyType::INTEGER);
    nogdb::Class::createExtend(txn, "systems", "backends");
    nogdb::Property::add(txn, "systems", "devops_skills", nogdb::PropertyType::INTEGER);
    nogdb::Class::createExtend(txn, "infras", "backends");
    nogdb::Property::add(txn, "infras", "IT_skills", nogdb::PropertyType::INTEGER);
    nogdb::Class::createExtend(txn, "designers", "frontends");
    nogdb::Property::add(txn, "designers", "ux_skills", nogdb::PropertyType::INTEGER);
    nogdb::Class::createExtend(txn, "admins", "employees");
    nogdb::Class::create(txn, "action", nogdb::ClassType::EDGE);
    nogdb::Property::add(txn, "action", "name", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "action", "type", nogdb::PropertyType::UNSIGNED_INTEGER);
    nogdb::Class::createExtend(txn, "collaborate", "action");
    nogdb::Class::createExtend(txn, "inter", "collaborate");
    nogdb::Class::createExtend(txn, "intra", "collaborate");
    nogdb::Class::createExtend(txn, "manage", "action");
    nogdb::Property::add(txn, "manage", "priority", nogdb::PropertyType::TEXT);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void destroy_all_extended_classes() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "intra");
    nogdb::Class::drop(txn, "inter");
    nogdb::Class::drop(txn, "collaborate");
    nogdb::Class::drop(txn, "manage");
    nogdb::Class::drop(txn, "action");
    nogdb::Class::drop(txn, "systems");
    nogdb::Class::drop(txn, "infras");
    nogdb::Class::drop(txn, "backends");
    nogdb::Class::drop(txn, "designers");
    nogdb::Class::drop(txn, "frontends");
    nogdb::Class::drop(txn, "admins");
    nogdb::Class::drop(txn, "employees");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_create_class_extend() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "employees", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "employees", "name", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "employees", "age", nogdb::PropertyType::UNSIGNED_INTEGER);
    nogdb::Property::add(txn, "employees", "salary", nogdb::PropertyType::UNSIGNED_BIGINT);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::createExtend(txn, "backends", "employees");
    nogdb::Property::add(txn, "backends", "cpp_skills", nogdb::PropertyType::INTEGER);
    nogdb::Property::add(txn, "backends", "js_skills", nogdb::PropertyType::INTEGER);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::createExtend(txn, "frontends", "employees");
    nogdb::Property::add(txn, "frontends", "html_skills", nogdb::PropertyType::INTEGER);
    nogdb::Property::add(txn, "frontends", "js_skills", nogdb::PropertyType::INTEGER);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::createExtend(txn, "systems", "backends");
    nogdb::Property::add(txn, "systems", "devops_skills", nogdb::PropertyType::INTEGER);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::createExtend(txn, "infras", "backends");
    nogdb::Property::add(txn, "infras", "IT_skills", nogdb::PropertyType::INTEGER);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::createExtend(txn, "designers", "frontends");
    nogdb::Property::add(txn, "designers", "ux_skills", nogdb::PropertyType::INTEGER);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::createExtend(txn, "admins", "employees");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "action", nogdb::ClassType::EDGE);
    nogdb::Property::add(txn, "action", "name", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "action", "type", nogdb::PropertyType::UNSIGNED_INTEGER);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::createExtend(txn, "collaborate", "action");
    nogdb::Class::createExtend(txn, "inter", "collaborate");
    nogdb::Class::createExtend(txn, "intra", "collaborate");
    nogdb::Class::createExtend(txn, "manage", "action");
    nogdb::Property::add(txn, "manage", "priority", nogdb::PropertyType::TEXT);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
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
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto res = nogdb::DB::getClass(txn, "infras");
    assert(res.type == nogdb::ClassType::VERTEX);
    res = nogdb::DB::getClass(txn, "intra");
    assert(res.type == nogdb::ClassType::EDGE);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.rollback();
}

void test_create_invalid_class_extend() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Class::createExtend(txn, "senior", "backend");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  try {
    nogdb::Class::createExtend(txn, "", "backends");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_INVALID_CLASSNAME, "NOGDB_CTX_INVALID_CLASSNAME");
  }

  try {
    nogdb::Class::createExtend(txn, "designers", "backends");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_DUPLICATE_CLASS, "NOGDB_CTX_DUPLICATE_CLASS");
  }

  try {
    nogdb::Class::createExtend(txn, "something1", "backends");
    nogdb::Property::add(txn, "something1", "", nogdb::PropertyType::INTEGER);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_INVALID_PROPERTYNAME, "NOGDB_CTX_INVALID_PROPERTYNAME");
  }

  try {
    nogdb::Class::createExtend(txn, "something2", "backends");
    nogdb::Property::add(txn, "something2", "prop1", nogdb::PropertyType::UNDEFINED);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_INVALID_PROPTYPE, "NOGDB_CTX_INVALID_PROPTYPE");
  }

  try {
    nogdb::Class::createExtend(txn, "something3", "systems");
    nogdb::Property::add(txn, "something3", "prop1", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "something3", "name", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "something3", "prop3", nogdb::PropertyType::TEXT);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_DUPLICATE_PROPERTY, "NOGDB_CTX_DUPLICATE_PROPERTY");
  }
}

void test_alter_class_extend() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::alter(txn, "backends", "backbackends");
    assert_class(txn, "systems", "backbackends", 0, 6);
    assert_class(txn, "infras", "backbackends", 0, 6);
    assert_class(txn, "backbackends", "employees", 2, 5);

    nogdb::Class::alter(txn, "backbackends", "backends");
    assert_class(txn, "systems", "backends", 0, 6);
    assert_class(txn, "infras", "backends", 0, 6);
    assert_class(txn, "backends", "employees", 2, 5);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_drop_class_extend() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "designers");
    assert_class(txn, "frontends", "employees", 0, 5);

    nogdb::Class::drop(txn, "collaborate");
    assert_class(txn, "action", "", 3, 2);
    assert_class(txn, "inter", "action", 0, 2);
    assert_class(txn, "intra", "action", 0, 2);

    nogdb::Class::drop(txn, "backends");
    assert_class(txn, "employees", "", 4, 3);
    assert_class(txn, "systems", "employees", 0, 4);
    assert_class(txn, "infras", "employees", 0, 4);

    nogdb::Class::drop(txn, "action");
    assert_class(txn, "manage", "", 0, 1);
    assert_class(txn, "inter", "", 0, 0);
    assert_class(txn, "intra", "", 0, 0);

    nogdb::Class::drop(txn, "employees");
    nogdb::Class::drop(txn, "inter");
    nogdb::Class::drop(txn, "admins");
    nogdb::Class::drop(txn, "intra");
    nogdb::Class::drop(txn, "manage");
    nogdb::Class::drop(txn, "systems");
    nogdb::Class::drop(txn, "infras");
    nogdb::Class::drop(txn, "frontends");

    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_add_property_extend() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Property::add(txn, "employees", "prop1", nogdb::PropertyType::UNSIGNED_INTEGER);
    assert_class(txn, "designers", "frontends", 0, 7);
    assert_class(txn, "admins", "employees", 0, 4);

    nogdb::Property::add(txn, "collaborate", "prop1", nogdb::PropertyType::BLOB);
    assert_class(txn, "collaborate", "action", 2, 3);
    assert_class(txn, "inter", "collaborate", 0, 3);
    assert_class(txn, "intra", "collaborate", 0, 3);
    assert_class(txn, "action", "", 2, 2);

    nogdb::Property::add(txn, "systems", "prop2", nogdb::PropertyType::REAL);
    assert_class(txn, "systems", "backends", 0, 8);
    assert_class(txn, "infras", "backends", 0, 7);
    assert_class(txn, "backends", "employees", 2, 6);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_add_invalid_property_extend() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Property::add(txn, "designers", "name", nogdb::PropertyType::TEXT);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_DUPLICATE_PROPERTY, "NOGDB_CTX_DUPLICATE_PROPERTY");
  }

  try {
    nogdb::Property::add(txn, "employees", "IT_skills", nogdb::PropertyType::TEXT);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_OVERRIDE_PROPERTY, "NOGDB_CTX_OVERRIDE_PROPERTY");
  }
}

void test_delete_property_extend() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Property::remove(txn, "systems", "prop2");
    assert_class(txn, "systems", "backends", 0, 7);
    assert_class(txn, "infras", "backends", 0, 7);
    assert_class(txn, "backends", "employees", 2, 6);

    nogdb::Property::remove(txn, "collaborate", "prop1");
    assert_class(txn, "collaborate", "action", 2, 2);
    assert_class(txn, "inter", "collaborate", 0, 2);
    assert_class(txn, "intra", "collaborate", 0, 2);
    assert_class(txn, "action", "", 2, 2);

    nogdb::Property::remove(txn, "employees", "prop1");
    assert_class(txn, "designers", "frontends", 0, 6);
    assert_class(txn, "admins", "employees", 0, 3);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_delete_invalid_property_extend() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Property::remove(txn, "systems", "name");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  try {
    nogdb::Property::remove(txn, "employees", "devops_skills");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }
}

void test_alter_property_extend() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Property::alter(txn, "employees", "name", "title");
    auto classDesc = nogdb::DB::getClass(txn, "systems");
    auto properties = nogdb::DB::getProperties(txn, classDesc);
    assert(std::find_if(properties.cbegin(), properties.cend(), [](const nogdb::PropertyDescriptor &property) {
      return property.name == "name";
    }) == properties.cend());
    assert(std::find_if(properties.cbegin(), properties.cend(), [](const nogdb::PropertyDescriptor &property) {
      return property.name == "title";
    }) != properties.cend());
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Property::alter(txn, "employees", "title", "name");
    auto classDesc = nogdb::DB::getClass(txn, "infras");
    auto properties = nogdb::DB::getProperties(txn, classDesc);
    assert(std::find_if(properties.cbegin(), properties.cend(), [](const nogdb::PropertyDescriptor &property) {
      return property.name == "name";
    }) != properties.cend());
    assert(std::find_if(properties.cbegin(), properties.cend(), [](const nogdb::PropertyDescriptor &property) {
      return property.name == "title";
    }) == properties.cend());
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_alter_invalid_property_extend() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Property::alter(txn, "backends", "cpp_skills", "IT_skills");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_OVERRIDE_PROPERTY, "NOGDB_CTX_OVERRIDE_PROPERTY");
  }

  try {
    nogdb::Property::alter(txn, "backends", "cpp_skills", "age");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_DUPLICATE_PROPERTY, "NOGDB_CTX_DUPLICATE_PROPERTY");
  }
}

void test_create_vertex_edge_extend() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto v1 = nogdb::Vertex::create(txn, "infras",
                                    nogdb::Record{}.set("name", "Peter").set("js_skills", 7).set("IT_skills", 9));
    auto v2 = nogdb::Vertex::create(txn, "admins", nogdb::Record{}.set("name", "Mike").set("age", 36U));
    auto e = nogdb::Edge::create(txn, "manage", v1, v2, nogdb::Record{}.set("name", "Team Leader"));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();
}

void test_create_invalid_vertex_edge_extend() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Vertex::create(txn, "infras", nogdb::Record{}.set("name", "Pete").set("devops_skills", 4));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }

  txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Vertex::create(txn, "employees", nogdb::Record{}.set("name", "Pete").set("js_skills", 4));
    assert(false);
  } catch (const nogdb::Error &ex) {
    txn.rollback();
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }
}

void test_delete_vertex_edge_extend() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    auto res = nogdb::Edge::get(txn, "manage");
    nogdb::Edge::destroy(txn, res[0].descriptor);
    res = nogdb::Vertex::get(txn, "infras");
    nogdb::Vertex::destroy(txn, res[0].descriptor);
    res = nogdb::Vertex::get(txn, "admins");
    nogdb::Vertex::destroy(txn, res[0].descriptor);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();
}

void test_get_class_extend() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  nogdb::RecordDescriptor a{}, b{}, c{}, d{}, e{}, f{};
  try {
    a = nogdb::Vertex::create(txn, "admins", nogdb::Record{}.set("name", "Adam").set("age", 26U));
    b = nogdb::Vertex::create(txn, "backends",
                              nogdb::Record{}.set("name", "Bill").set("age", 32U).set("cpp_skills", 7));
    c = nogdb::Vertex::create(txn, "systems",
                              nogdb::Record{}.set("name", "Charon").set("age", 27U).set("js_skills", 6)
                                  .set("cpp_skills", 8).set("devops_skills", 10));
    d = nogdb::Vertex::create(txn, "designers", nogdb::Record{}.set("name", "Don").set("ux_skills", 9U));
    e = nogdb::Vertex::create(txn, "employees", nogdb::Record{}.set("name", "Eric"));
    f = nogdb::Vertex::create(txn, "frontends",
                              nogdb::Record{}.set("name", "Falcao").set("age", 34U).set("js_skills", 9));

    nogdb::Edge::create(txn, "manage", a, e, nogdb::Record{}.set("name", "helpdesk").set("priority", "medium"));
    nogdb::Edge::create(txn, "inter", b, f, nogdb::Record{}.set("name", "api creator"));
    nogdb::Edge::create(txn, "intra", b, c, nogdb::Record{}.set("name", "team member"));
    nogdb::Edge::create(txn, "inter", c, f, nogdb::Record{}.set("name", "system provider"));
    nogdb::Edge::create(txn, "manage", c, b, nogdb::Record{}.set("name", "team leader").set("priority", "high"));
    nogdb::Edge::create(txn, "intra", c, b, nogdb::Record{}.set("name", "system provider"));
    nogdb::Edge::create(txn, "collaborate", d, b, nogdb::Record{}.set("name", "ui provider"));
    nogdb::Edge::create(txn, "collaborate", d, c, nogdb::Record{}.set("name", "ui provider"));
    nogdb::Edge::create(txn, "intra", d, f, nogdb::Record{}.set("name", "wireframe creator"));
    nogdb::Edge::create(txn, "collaborate", e, a, nogdb::Record{}.set("name", "guest"));
    nogdb::Edge::create(txn, "inter", f, b, nogdb::Record{}.set("name", "ui creator"));
    nogdb::Edge::create(txn, "intra", f, d, nogdb::Record{}.set("name", "team member"));

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  try {
    auto res = nogdb::Vertex::get(txn, "employees");
    ASSERT_SIZE(res, 1);

    res = nogdb::Vertex::getExtend(txn, "employees");
    ASSERT_SIZE(res, 6);
    res = getVertexMultipleClassExtend(txn, std::set<std::string>{"admins", "backends", "frontends"});
    ASSERT_SIZE(res, 5);
    res = nogdb::Edge::getExtend(txn, "action");
    ASSERT_SIZE(res, 12);
    res = nogdb::Edge::getExtend(txn, "manage");
    ASSERT_SIZE(res, 2);
    res = getEdgeMultipleClassExtend(txn, std::set<std::string>{"collaborate", "manage"});
    ASSERT_SIZE(res, 12);
    res = nogdb::Edge::getExtend(txn, "inter");
    ASSERT_SIZE(res, 3);

    res = nogdb::Vertex::getExtend(txn, "backends");
    for (const auto &r: res) {
      if (r.record.get("name").toText() == "Bill") {
        auto edges = nogdb::Vertex::getInEdge(txn, r.descriptor, nogdb::GraphFilter{}.exclude("collaborate"));
        ASSERT_SIZE(edges, 3);
        edges = nogdb::Vertex::getAllEdge(txn, r.descriptor, nogdb::GraphFilter{}.only("inter", "manage"));
        ASSERT_SIZE(edges, 3);
      } else if (r.record.get("name").toText() == "Charon") {
        auto edges = nogdb::Vertex::getOutEdge(txn, r.descriptor, nogdb::GraphFilter{}.only("collaborate"));
        ASSERT_SIZE(edges, 2);
      }
    }

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();
}

void test_find_class_extend() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto res = nogdb::Vertex::get(txn, "systems", nogdb::Condition("age").le(30U));
    ASSERT_SIZE(res, 1);
    assert(res[0].record.get("name").toText() == "Charon");
    res = nogdb::Vertex::get(txn, "employees", nogdb::Condition("age").le(30U));
    ASSERT_SIZE(res, 2);
    assert(res[0].record.get("name").toText() == "Charon" || res[0].record.get("name").toText() == "Adam");
    assert(res[1].record.get("name").toText() == "Charon" || res[1].record.get("name").toText() == "Adam");
    res = nogdb::Vertex::get(txn, "backends", nogdb::Condition("cpp_skills").eq(8));

    res = nogdb::Edge::get(txn, "collaborate", nogdb::Condition("name").endWith("provider").ignoreCase());
    ASSERT_SIZE(res, 4);
    res = nogdb::Edge::get(txn, "action", nogdb::Condition("priority"));
    ASSERT_SIZE(res, 2);

    auto b = nogdb::Vertex::get(txn, "employees", nogdb::Condition("name").eq("Bill"));
    assert(b.size() == 1);
    res = nogdb::Vertex::getInEdge(txn, b[0].descriptor, nogdb::Condition("name").endWith("provider").ignoreCase());
    ASSERT_SIZE(res, 2);
    assert(res[0].record.get("name").toText() == "ui provider" ||
           res[0].record.get("name").toText() == "system provider");
    assert(res[1].record.get("name").toText() == "ui provider" ||
           res[1].record.get("name").toText() == "system provider");
    res = nogdb::Vertex::getInEdge(txn, b[0].descriptor,
                                   nogdb::GraphFilter{nogdb::Condition("name").endWith("provider").ignoreCase()}.only(
                                       "collaborate"));
    ASSERT_SIZE(res, 2);
    assert(res[0].record.get("name").toText() == "ui provider" ||
           res[0].record.get("name").toText() == "system provider");
    assert(res[1].record.get("name").toText() == "ui provider" ||
           res[1].record.get("name").toText() == "system provider");
    res = nogdb::Vertex::getInEdge(txn, b[0].descriptor,
                                   nogdb::GraphFilter{nogdb::Condition("type").null()}.only("inter", "manage"));
    ASSERT_SIZE(res, 2);
    assert(res[0].record.get("name").toText() == "ui creator" ||
           res[0].record.get("name").toText() == "team leader");
    assert(res[1].record.get("name").toText() == "ui creator" ||
           res[1].record.get("name").toText() == "team leader");

    auto c = nogdb::Vertex::get(txn, "employees", nogdb::Condition("name").eq("Charon"));
    assert(c.size() == 1);
    res = nogdb::Vertex::getOutEdge(txn, c[0].descriptor,
                                    nogdb::GraphFilter{nogdb::Condition("name").beginWith("team").ignoreCase()}.only(
                                        "action"));
    ASSERT_SIZE(res, 1);
    assert(res[0].record.get("name").toText() == "team leader");
    res = nogdb::Vertex::getAllEdge(txn, b[0].descriptor,
                                    nogdb::GraphFilter{nogdb::Condition("name").contain("team").ignoreCase()}.only(
                                        "collaborate"));
    ASSERT_SIZE(res, 1);
    assert(res[0].record.get("name").toText() == "team member");

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  txn.commit();
}

void test_traverse_class_extend() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto b = nogdb::Vertex::get(txn, "employees", nogdb::Condition("name").eq("Bill"));
    auto c = nogdb::Vertex::get(txn, "employees", nogdb::Condition("name").eq("Charon"));
    auto f = nogdb::Vertex::get(txn, "employees", nogdb::Condition("name").eq("Falcao"));
    auto res = nogdb::Traverse::inEdgeBfs(txn, b[0].descriptor, 1, 1);
    ASSERT_SIZE(res, 3);
    res = nogdb::Traverse::inEdgeBfs(txn, b[0].descriptor, 1, 1, nogdb::GraphFilter{}.only("collaborate"));
    ASSERT_SIZE(res, 3);
    res = nogdb::Traverse::outEdgeBfs(txn, f[0].descriptor, 1, 1, nogdb::GraphFilter{}.only("collaborate"));
    ASSERT_SIZE(res, 2);
    res = nogdb::Traverse::outEdgeBfs(txn, f[0].descriptor, 1, 2, nogdb::GraphFilter{}.only("collaborate"));
    ASSERT_SIZE(res, 3);
    res = nogdb::Traverse::allEdgeBfs(txn, c[0].descriptor, 0, 100, nogdb::GraphFilter{}.only("collaborate", "manage"));
    ASSERT_SIZE(res, 4);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();
}

void test_shortest_path_class_extend() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
  try {
    auto b = nogdb::Vertex::get(txn, "employees", nogdb::Condition("name").eq("Bill"));
    auto c = nogdb::Vertex::get(txn, "employees", nogdb::Condition("name").eq("Charon"));
    auto d = nogdb::Vertex::get(txn, "employees", nogdb::Condition("name").eq("Don"));
    auto res = nogdb::Traverse::shortestPath(txn, c[0].descriptor, d[0].descriptor);
    ASSERT_SIZE(res, 3);
    assert(res[0].record.get("name").toText() == "Charon");
    assert(res[1].record.get("name").toText() == "Falcao");
    assert(res[2].record.get("name").toText() == "Don");

    res = nogdb::Traverse::shortestPath(txn, c[0].descriptor, d[0].descriptor,
                                        nogdb::GraphFilter{}.only("collaborate"));
    ASSERT_SIZE(res, 3);
    assert(res[0].record.get("name").toText() == "Charon");
    assert(res[1].record.get("name").toText() == "Falcao");
    assert(res[2].record.get("name").toText() == "Don");

    res = nogdb::Traverse::shortestPath(txn, b[0].descriptor, d[0].descriptor,
                                        nogdb::GraphFilter{}.only("collaborate"));
    ASSERT_SIZE(res, 3);
    assert(res[0].record.get("name").toText() == "Bill");
    assert(res[1].record.get("name").toText() == "Falcao");
    assert(res[2].record.get("name").toText() == "Don");

    res = nogdb::Traverse::shortestPath(txn, b[0].descriptor, d[0].descriptor,
                                        nogdb::GraphFilter{}.only("inter", "manage"));
    ASSERT_SIZE(res, 0);
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  txn.commit();
}
