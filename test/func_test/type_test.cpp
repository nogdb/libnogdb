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
#include <cmath>

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

constexpr int int_value = -42;
constexpr unsigned int uint_value = 42;
constexpr char tinyint_value = -128;
constexpr unsigned char utinyint_value = 255;
constexpr short smallint_value = -32768;
constexpr unsigned short usmallint_value = 65535;
constexpr long long bigint_value = -424242424242;
constexpr unsigned long long ubigint_value = 424242424242;
constexpr double real_value = 42.4242;
const std::string text_value = "hello world";
const myobject blob_value = { 42, 42424242424242ULL, 42.42 };
const std::vector<int> vector_int_value = { 1, 2, 3, 4, 5 };
const std::vector<const char*> vector_c_str { "hello", "world", "this", "is ", " a ", "test" };
const std::vector<std::vector<const char*>> vv_c_str { { "hello", "world1" }, { "hello2", "world2" },
    { "data 1", " data2", "   " } };
const std::set<std::pair<int, int>> set_pii { { 2, 3 }, { 4, 5 }, { 6, 7 }, { 8, 9 } };
const std::map<int, const char*> map_p_int_c_str { { 0, "helloQWE@!#" }, { 1, "กดฟหฟหกดก่าฟหกสดว" } };
const int array_int[] = { 3, 4, 5, 6, 10 };

void test_bytes_only()
{
    nogdb::Bytes int_vb { int_value }, uint_vb { uint_value }, tinyint_vb { tinyint_value },
        utinyint_vb { utinyint_value }, smallint_vb { smallint_value }, usmallint_vb { usmallint_value },
        bigint_vb { bigint_value }, ubigint_vb { ubigint_value }, real_vb { real_value }, text_vb { text_value },
        blob_vb { blob_value }, vector_int_vb { nogdb::Bytes::toBytes(vector_int_value) },
        vector_c_str_vb { nogdb::Bytes::toBytes(vector_c_str) }, vv_c_str_vb { nogdb::Bytes::toBytes(vv_c_str) },
        set_pii_vb { nogdb::Bytes::toBytes(set_pii) }, map_p_int_c_str_vb { nogdb::Bytes::toBytes(map_p_int_c_str) };

    assert(int_vb.toInt() == int_value);
    assert(uint_vb.toIntU() == uint_value);
    assert(tinyint_vb.toTinyInt() == tinyint_value);
    assert(utinyint_vb.toTinyIntU() == utinyint_value);
    assert(smallint_vb.toSmallInt() == smallint_value);
    assert(usmallint_vb.toSmallIntU() == usmallint_value);
    assert(bigint_vb.toBigInt() == bigint_value);
    assert(ubigint_vb.toBigIntU() == ubigint_value);
    assert(real_vb.toReal() == real_value);
    assert(text_vb.toText() == text_value);
    assert(vector_int_vb.convert<std::vector<int>>() == vector_int_value);

    auto vector_c_str_check = vector_c_str_vb.convert<std::vector<std::string>>();
    assert(vector_c_str_check.size() == vector_c_str.size());
    for (size_t i = 0; i < vector_c_str.size(); ++i) {
        assert(strcmp(vector_c_str_check[i].c_str(), vector_c_str[i]) == 0);
    }

    auto vv_c_str_check = vv_c_str_vb.convert<std::vector<std::vector<std::string>>>();
    assert(vv_c_str_check.size() == 3u);
    assert(vv_c_str_check[0].size() == 2u);
    assert(vv_c_str_check[1].size() == 2u);
    assert(vv_c_str_check[2].size() == 3u);

    auto set_pii_check = set_pii_vb.convert<std::set<std::pair<int, int>>>();
    assert(set_pii_check == set_pii);

    for (const auto& it : map_p_int_c_str_vb.convert<std::map<int, std::string>>()) {
        assert(it.second == map_p_int_c_str.at(it.first));
    }

    auto tmp = myobject {};
    blob_vb.convertTo(tmp);
    assert(tmp.x == blob_value.x);
    assert(tmp.y == blob_value.y);
    assert(tmp.z == blob_value.z);
}

void test_record_with_bytes()
{
    nogdb::Record r {};
    r.set("int", int_value)
        .set("uint", uint_value)
        .set("tinyint", tinyint_value)
        .set("utinyint", utinyint_value)
        .set("smallint", smallint_value)
        .set("usmallint", usmallint_value)
        .set("bigint", bigint_value)
        .set("ubigint", ubigint_value)
        .set("real", real_value)
        .set("text", text_value)
        .set("blob", nogdb::Bytes { blob_value })
        .set("null", "")
        .set("vector_int", vector_int_value)
        .set("set_pii", set_pii)
        .set("array_int", array_int);

    assert(r.getInt("int") == int_value);
    assert(r.getIntU("uint") == uint_value);
    assert(r.getTinyInt("tinyint") == tinyint_value);
    assert(r.getTinyIntU("utinyint") == utinyint_value);
    assert(r.getSmallInt("smallint") == smallint_value);
    assert(r.getSmallIntU("usmallint") == usmallint_value);
    assert(r.getBigInt("bigint") == bigint_value);
    assert(r.getBigIntU("ubigint") == ubigint_value);
    assert(r.getReal("real") == real_value);
    assert(r.getText("text") == text_value);
    assert(r.getText("invalid") == "");
    assert(r.get("set_pii").convert<decltype(set_pii)>() == set_pii);
    for (size_t i = 0; i < 5; ++i) {
        assert(r.get("array_int").convert<int*>()[i] == array_int[i]);
    }

    auto bytes_tmp = myobject {};
    r.get("blob").convertTo(bytes_tmp);
    assert(bytes_tmp.x == blob_value.x);
    assert(bytes_tmp.y == blob_value.y);
    assert(bytes_tmp.z == blob_value.z);

    assert(r.get("int").size() == sizeof(int_value));
    assert(r.get("uint").size() == sizeof(uint_value));
    assert(r.get("tinyint").size() == sizeof(tinyint_value));
    assert(r.get("utinyint").size() == sizeof(utinyint_value));
    assert(r.get("smallint").size() == sizeof(smallint_value));
    assert(r.get("usmallint").size() == sizeof(usmallint_value));
    assert(r.get("bigint").size() == sizeof(bigint_value));
    assert(r.get("ubigint").size() == sizeof(ubigint_value));
    assert(r.get("real").size() == sizeof(real_value));
    assert(r.get("text").size() == text_value.length());
    assert(r.get("null").size() == std::string { "" }.length());
    assert(r.get("blob").size() == sizeof(blob_value));

    auto int_b_copy = r.get("int");
    assert(int_b_copy.toInt() == int_value);
    auto int_b_assign = nogdb::Bytes {};
    int_b_assign = int_b_copy;
    assert(int_b_assign.toInt() == int_value);

    auto uint_b_copy = r.get("uint");
    assert(uint_b_copy.toInt() == uint_value);
    auto uint_b_assign = nogdb::Bytes {};
    uint_b_assign = uint_b_copy;
    assert(uint_b_assign.toIntU() == uint_value);

    auto tinyint_b_copy = r.get("tinyint");
    assert(tinyint_b_copy.toTinyInt() == tinyint_value);
    auto tinyint_b_assign = nogdb::Bytes {};
    tinyint_b_assign = tinyint_b_copy;
    assert(tinyint_b_assign.toTinyInt() == tinyint_value);

    auto utinyint_b_copy = r.get("utinyint");
    assert(utinyint_b_copy.toTinyIntU() == utinyint_value);
    auto utinyint_b_assign = nogdb::Bytes {};
    utinyint_b_assign = utinyint_b_copy;
    assert(utinyint_b_assign.toTinyIntU() == utinyint_value);

    auto smallint_b_copy = r.get("smallint");
    assert(smallint_b_copy.toSmallInt() == smallint_value);
    auto smallint_b_assign = nogdb::Bytes {};
    smallint_b_assign = smallint_b_copy;
    assert(smallint_b_assign.toSmallInt() == smallint_value);

    auto usmallint_b_copy = r.get("usmallint");
    assert(usmallint_b_copy.toSmallIntU() == usmallint_value);
    auto usmallint_b_assign = nogdb::Bytes {};
    usmallint_b_assign = usmallint_b_copy;
    assert(usmallint_b_assign.toSmallIntU() == usmallint_value);

    auto bigint_b_copy = r.get("bigint");
    assert(bigint_b_copy.toBigInt() == bigint_value);
    auto bigint_b_assign = nogdb::Bytes {};
    bigint_b_assign = bigint_b_copy;
    assert(bigint_b_assign.toBigInt() == bigint_value);

    auto ubigint_b_copy = r.get("ubigint");
    assert(ubigint_b_copy.toBigIntU() == ubigint_value);
    auto ubigint_b_assign = nogdb::Bytes {};
    ubigint_b_assign = ubigint_b_copy;
    assert(ubigint_b_assign.toBigIntU() == ubigint_value);

    auto real_b_copy = r.get("real");
    assert(real_b_copy.toReal() == real_value);
    auto real_b_assign = nogdb::Bytes {};
    real_b_assign = real_b_copy;
    assert(real_b_assign.toReal() == real_value);

    auto text_b_copy = r.get("text");
    assert(text_b_copy.toText() == text_value);
    auto text_b_assign = nogdb::Bytes {};
    text_b_assign = text_b_copy;
    assert(text_b_assign.toText() == text_value);

    auto bytes_b_copy = r.get("blob");
    auto bytes_copy_tmp = myobject {};
    bytes_b_copy.convertTo(bytes_copy_tmp);
    assert(bytes_copy_tmp.x == blob_value.x);
    assert(bytes_copy_tmp.y == blob_value.y);
    assert(bytes_copy_tmp.z == blob_value.z);
    auto bytes_b_assign = nogdb::Bytes {};
    bytes_b_assign = bytes_b_copy;
    auto bytes_assign_tmp = myobject {};
    bytes_b_assign.convertTo(bytes_assign_tmp);
    assert(bytes_assign_tmp.x == blob_value.x);
    assert(bytes_assign_tmp.y == blob_value.y);
    assert(bytes_assign_tmp.z == blob_value.z);

    r.unset("int");
    assert(r.get("int").empty());
    r.clear();
    assert(r.empty());
}

void test_invalid_record_with_bytes()
{
    nogdb::Record r {};
    try {
        r.getInt("int");
    } catch (const nogdb::Error& err) {
        REQUIRE(err, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }
    try {
        r.getIntU("uint");
    } catch (const nogdb::Error& err) {
        REQUIRE(err, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }
    try {
        r.getTinyInt("tinyint");
    } catch (const nogdb::Error& err) {
        REQUIRE(err, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }
    try {
        r.getTinyIntU("utinyint");
    } catch (const nogdb::Error& err) {
        REQUIRE(err, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }
    try {
        r.getSmallInt("smallint");
    } catch (const nogdb::Error& err) {
        REQUIRE(err, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }
    try {
        r.getSmallIntU("usmallint");
    } catch (const nogdb::Error& err) {
        REQUIRE(err, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }
    try {
        r.getBigInt("bigint");
    } catch (const nogdb::Error& err) {
        REQUIRE(err, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }
    try {
        r.getBigIntU("ubigint");
    } catch (const nogdb::Error& err) {
        REQUIRE(err, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }
    try {
        r.getReal("real");
    } catch (const nogdb::Error& err) {
        REQUIRE(err, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }
    try {
        r.getText("text");
    } catch (const nogdb::Error& err) {
        REQUIRE(err, NOGDB_CTX_NOEXST_PROPERTY, "NOGDB_CTX_NOEXST_PROPERTY");
    }
}

void test_invalid_record_property_name()
{
    nogdb::Record r {};
    r.set("hello", 1).set("_hello", 2).set("@className", "not allowed").set("@recordId", "-1:-1");
    assert(r.size() == 2);
}
