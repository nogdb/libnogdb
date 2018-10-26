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

void test_create_class() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "files", nogdb::ClassType::VERTEX);
    auto schema = nogdb::DB::getClass(txn, "files");
    assert(schema.name == "files");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_create_class_with_properties() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "files2", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "files2", "prop1", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "files2", "prop2", nogdb::PropertyType::INTEGER);
    nogdb::Property::add(txn, "files2", "prop3", nogdb::PropertyType::UNSIGNED_BIGINT);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_drop_class() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "files");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "files2");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_alter_class() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "files", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "files", "prop1", nogdb::PropertyType::INTEGER);
    nogdb::Property::add(txn, "files", "prop2", nogdb::PropertyType::TEXT);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    auto cdesc = nogdb::DB::getClass(txn, "files");
    assert(cdesc.name == "files");
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::alter(txn, "files", "file");
    txn.commit();

    txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    cdesc = nogdb::DB::getClass(txn, "file");
    assert(cdesc.name == "file");
    auto properties = nogdb::DB::getProperties(txn, cdesc);
    assert(properties.size() == 2);
    for(const auto& property: properties) {
      if (property.name == "prop1") assert(property.type == nogdb::PropertyType::INTEGER);
      else if (property.name == "prop2") assert(property.type == nogdb::PropertyType::TEXT);
      else assert(false);
    }
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "file");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_alter_invalid_class() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "files", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "files", "prop1", nogdb::PropertyType::INTEGER);
    nogdb::Property::add(txn, "files", "prop2", nogdb::PropertyType::TEXT);
    nogdb::Class::create(txn, "folders", nogdb::ClassType::VERTEX);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Class::alter(txn, "files", "");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_INVALID_CLASSNAME, "NOGDB_CTX_INVALID_CLASSNAME");
  }

  try {
    nogdb::Class::alter(txn, "", "file");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  try {
    nogdb::Class::alter(txn, "file", "filess");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }

  try {
    nogdb::Class::alter(txn, "files", "files");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_DUPLICATE_CLASS, "NOGDB_CTX_DUPLICATE_CLASS");
  }

  try {
    nogdb::Class::alter(txn, "files", "folders");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_DUPLICATE_CLASS, "NOGDB_CTX_DUPLICATE_CLASS");
  }
  txn.commit();

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "files");
    nogdb::Class::drop(txn, "folders");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_create_invalid_class() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "files", nogdb::ClassType::VERTEX);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Class::create(txn, "", nogdb::ClassType::VERTEX);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_INVALID_CLASSNAME, "NOGDB_CTX_INVALID_CLASSNAME");
  }
  try {
    nogdb::Class::create(txn, "files", nogdb::ClassType::VERTEX);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_DUPLICATE_CLASS, "NOGDB_CTX_DUPLICATE_CLASS");
  }
  try {
    nogdb::Class::create(txn, "files", nogdb::ClassType::UNDEFINED);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_INVALID_CLASSTYPE, "NOGDB_CTX_INVALID_CLASSTYPE");
  }
  txn.commit();

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "files");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_create_invalid_class_with_properties() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "files2", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "files2", "prop1", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "files2", "prop2", nogdb::PropertyType::INTEGER);
    nogdb::Property::add(txn, "files2", "prop3", nogdb::PropertyType::UNDEFINED);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_INVALID_PROPTYPE, "NOGDB_CTX_INVALID_PROPTYPE");
  }
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "files2", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "files2", "prop1", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "files2", "", nogdb::PropertyType::INTEGER);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_INVALID_PROPERTYNAME, "NOGDB_CTX_INVALID_PROPERTYNAME");
  }
}

void test_drop_invalid_class() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Class::drop(txn, "");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }
  try {
    nogdb::Class::drop(txn, "file");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
    assert(ex.code() == NOGDB_CTX_NOEXST_CLASS);
  }
  try {
    nogdb::Class::drop(txn, "files");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }
  try {
    nogdb::Class::drop(txn, "files2");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }
}

void test_add_property() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "files", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "files", "filename", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "files", "filesize", nogdb::PropertyType::UNSIGNED_INTEGER);
    nogdb::Property::add(txn, "files", "ctime", nogdb::PropertyType::UNSIGNED_INTEGER);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    auto schema = nogdb::DB::getClass(txn, "files");
    assert(schema.name == "files");
    auto properties = nogdb::DB::getProperties(txn, schema);
    assert(properties.size() == 3);
    for(const auto& property: properties) {
      if (property.name == "filename") assert(property.type == nogdb::PropertyType::TEXT);
      else if (property.name == "filesize") assert(property.type == nogdb::PropertyType::UNSIGNED_INTEGER);
      else if (property.name == "ctime") assert(property.type == nogdb::PropertyType::UNSIGNED_INTEGER);
      else assert(false);
    }
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_delete_property() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Property::remove(txn, "files", "ctime");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "files");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_add_invalid_property() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "files", nogdb::ClassType::VERTEX);
    nogdb::Property::add(txn, "files", "filename", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "files", "filesize", nogdb::PropertyType::UNSIGNED_INTEGER);
    nogdb::Property::add(txn, "files", "ctime", nogdb::PropertyType::UNSIGNED_INTEGER);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Property::add(txn, "files", "", nogdb::PropertyType::INTEGER);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_INVALID_PROPERTYNAME, "NOGDB_CTX_INVALID_PROPERTYNAME");
  }
  try {
    nogdb::Property::add(txn, "", "extension", nogdb::PropertyType::INTEGER);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }
  try {
    nogdb::Property::add(txn, "file", "extension", nogdb::PropertyType::TEXT);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }
  try {
    nogdb::Property::add(txn, "links", "type", nogdb::PropertyType::UNDEFINED);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_INVALID_PROPTYPE, "NOGDB_CTX_INVALID_PROPTYPE");
  }
  try {
    nogdb::Property::add(txn, "files", "filename", nogdb::PropertyType::TEXT);
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_DUPLICATE_PROPERTY, "NOGDB_CTX_DUPLICATE_PROPERTY");
  }
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    auto schema = nogdb::DB::getClass(txn, "files");
    assert(schema.name == "files");
    auto properties = nogdb::DB::getProperties(txn, schema);
    assert(properties.size() == 3);
    for(const auto& property: properties) {
      if (property.name == "filename") assert(property.type == nogdb::PropertyType::TEXT);
      else if (property.name == "filesize") assert(property.type == nogdb::PropertyType::UNSIGNED_INTEGER);
      else if (property.name == "ctime") assert(property.type == nogdb::PropertyType::UNSIGNED_INTEGER);
      else assert(false);
    }
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_delete_invalid_property() {
  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Property::remove(txn, "files", "ctimes");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    assert(ex.code() == NOGDB_CTX_NOEXST_PROPERTY);
  }
  try {
    nogdb::Property::remove(txn, "files", "");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }
  try {
    nogdb::Property::remove(txn, "file", "ctime");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }
  try {
    nogdb::Property::remove(txn, "files", "ctime");
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  try {
    nogdb::Property::remove(txn, "files", "ctime");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }
  txn.commit();

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "files");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_alter_property() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "links", nogdb::ClassType::EDGE);
    nogdb::Property::add(txn, "links", "type", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "links", "expire", nogdb::PropertyType::INTEGER);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Property::alter(txn, "links", "type", "comments");
    nogdb::Property::alter(txn, "links", "expire", "expired");
    nogdb::Property::add(txn, "links", "type", nogdb::PropertyType::BLOB);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
    auto schema = nogdb::DB::getClass(txn, "links");
    assert(schema.name == "links");
    auto properties = nogdb::DB::getProperties(txn, schema);
    assert(properties.size() == 3);
    for(const auto& property: properties) {
      if (property.name == "type") assert(property.type == nogdb::PropertyType::BLOB);
      else if (property.name == "comments") assert(property.type == nogdb::PropertyType::TEXT);
      else if (property.name == "expired") assert(property.type == nogdb::PropertyType::INTEGER);
      else assert(false);
    }
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "links");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

void test_alter_invalid_property() {
  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::create(txn, "links", nogdb::ClassType::EDGE);
    nogdb::Property::add(txn, "links", "type", nogdb::PropertyType::TEXT);
    nogdb::Property::add(txn, "links", "expire", nogdb::PropertyType::INTEGER);
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
  try {
    nogdb::Property::alter(txn, "link", "type", "");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_INVALID_PROPERTYNAME, "NOGDB_CTX_INVALID_PROPERTYNAME");
  }
  try {
    nogdb::Property::alter(txn, "", "type", "types");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }
  try {
    nogdb::Property::alter(txn, "links", "", "types");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }
  try {
    nogdb::Property::alter(txn, "link", "type", "comments");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_CLASS, "NOGDB_CTX_NOEXST_CLASS");
  }
  try {
    nogdb::Property::alter(txn, "links", "types", "comments");
    assert(false);
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
  }
  try {
    nogdb::Property::alter(txn, "links", "type", "expire");
  } catch (const nogdb::Error &ex) {
    REQUIRE(ex, NOGDB_CTX_DUPLICATE_PROPERTY, "NOGDB_CTX_DUPLICATE_PROPERTY");
  }
  txn.commit();

  try {
    auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
    nogdb::Class::drop(txn, "links");
    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}
