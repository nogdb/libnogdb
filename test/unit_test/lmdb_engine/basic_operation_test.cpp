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

#include "lmdb_engine_test.h"
#include <numeric>
#include <utility>

using namespace nogdb::internal_data_type;

TEST_F(LMDBBasicOperations, put_get_string_string)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_string_string", false, true);
    dbi.put(std::string { "hello1" }, std::string { "world1" });
    dbi.put(std::string { "hello2" }, std::string { "world2" });
    dbi.put(std::string { "hello3" }, std::string { "world3" });

    auto res = dbi.get(std::string { "hello1" });
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.string(), "world1");

    res = dbi.get(std::string { "hello2" });
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.string(), "world2");

    res = dbi.get(std::string { "hello3" });
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.string(), "world3");

    res = dbi.get(std::string { "hello4" });
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_numeric_string)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_numeric_string", true, true);
    dbi.put(1UL, std::string { "world1" });
    dbi.put(2UL, std::string { "world2" });
    dbi.put(3UL, std::string { "world3" });
    dbi.put(0UL, std::string { "world0" });
    dbi.put(std::numeric_limits<uint64_t>::max(), std::string { "worldmax" });

    auto res = dbi.get(1UL);
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.string(), "world1");

    res = dbi.get(2UL);
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.string(), "world2");

    res = dbi.get(3UL);
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.string(), "world3");

    res = dbi.get(0UL);
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.string(), "world0");

    res = dbi.get(std::numeric_limits<uint64_t>::max());
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.string(), "worldmax");

    res = dbi.get(4UL);
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_decimal_string)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_decimal_string", true, true);
    dbi.put(1.1, std::string { "world1" });
    dbi.put(-2.2, std::string { "world2" });
    dbi.put(3.3, std::string { "world3" });
    dbi.put(std::numeric_limits<double>::min(), std::string { "worldmin" });
    dbi.put(std::numeric_limits<double>::max(), std::string { "worldmax" });

    auto res = dbi.get(1.1);
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.string(), "world1");

    res = dbi.get(-2.2);
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.string(), "world2");

    res = dbi.get(3.3);
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.string(), "world3");

    res = dbi.get(std::numeric_limits<double>::min());
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.string(), "worldmin");

    res = dbi.get(std::numeric_limits<double>::max());
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.string(), "worldmax");

    res = dbi.get(-4.4);
    ASSERT_TRUE(res.empty);

    afterEach();
}

Blob setBlob(const std::string& sampleData, const unsigned int sampleNumber)
{
    Blob blob { sampleData.length() + sizeof(sampleNumber) };
    blob.append(sampleData.c_str(), sampleData.length());
    blob.append(&sampleNumber, sizeof(sampleNumber));
    return blob;
}

std::pair<std::string, unsigned int> getBlob(const Blob& blob, size_t expectDataLength)
{
    std::string expectData {};
    unsigned int expectNumber {};
    Blob::Byte nameBytes[expectDataLength];
    size_t offset = 0;
    offset += blob.retrieve(nameBytes, offset, expectDataLength);
    expectData = std::string(reinterpret_cast<char*>(nameBytes), expectDataLength);
    blob.retrieve(&expectNumber, offset, sizeof(expectNumber));
    return std::make_pair(expectData, expectNumber);
};

TEST_F(LMDBBasicOperations, put_get_string_blob)
{
    beforeEach();

    auto blob = setBlob("world", 128);
    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_string_blob", false, true);
    dbi.put(std::string { "hello" }, blob);

    auto res = dbi.get(std::string { "hello" });
    ASSERT_FALSE(res.empty);
    auto data = res.data.blob();
    EXPECT_EQ(data.size(), 9);
    EXPECT_EQ(data.capacity(), 9);

    auto values = getBlob(data, 5);
    EXPECT_EQ(values.first, "world");
    EXPECT_EQ(values.second, 128);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_numeric_blob)
{
    beforeEach();

    auto blob = setBlob("world", 128);
    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_numeric_blob", false, true);
    dbi.put(1UL, blob);

    auto res = dbi.get(1UL);
    ASSERT_FALSE(res.empty);
    auto data = res.data.blob();
    EXPECT_EQ(data.size(), 9);
    EXPECT_EQ(data.capacity(), 9);

    auto values = getBlob(data, 5);
    EXPECT_EQ(values.first, "world");
    EXPECT_EQ(values.second, 128);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_decimal_blob)
{
    beforeEach();

    auto blob = setBlob("world", 128);
    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_decimal_blob", false, true);
    dbi.put(-123.4567, blob);

    auto res = dbi.get(-123.4567);
    ASSERT_FALSE(res.empty);
    auto data = res.data.blob();
    EXPECT_EQ(data.size(), 9);
    EXPECT_EQ(data.capacity(), 9);

    auto values = getBlob(data, 5);
    EXPECT_EQ(values.first, "world");
    EXPECT_EQ(values.second, 128);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_string_numeric)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_string_numeric", false, true);
    dbi.put(std::string { "hello1" }, 100UL);
    dbi.put(std::string { "hello2" }, 200UL);
    dbi.put(std::string { "hello3" }, 300UL);

    auto res = dbi.get(std::string { "hello1" });
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.numeric<uint32_t>(), 100UL);

    res = dbi.get(std::string { "hello2" });
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.numeric<uint32_t>(), 200UL);

    res = dbi.get(std::string { "hello3" });
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.numeric<uint32_t>(), 300UL);

    res = dbi.get(std::string { "hello4" });
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_numeric_numeric)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_numeric_numeric", true, true);
    dbi.put(1UL, 100UL);
    dbi.put(2UL, 200UL);
    dbi.put(3UL, 300UL);

    auto res = dbi.get(1UL);
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.numeric<uint32_t>(), 100UL);

    res = dbi.get(2UL);
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.numeric<uint32_t>(), 200UL);

    res = dbi.get(3UL);
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.numeric<uint32_t>(), 300UL);

    res = dbi.get(4UL);
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_decimal_numeric)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_numeric_numeric", true, true);
    dbi.put(1.1, 100UL);
    dbi.put(-2.2, 200UL);
    dbi.put(3.3, 300UL);

    auto res = dbi.get(1.1);
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.numeric<uint32_t>(), 100UL);

    res = dbi.get(-2.2);
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.numeric<uint32_t>(), 200UL);

    res = dbi.get(3.3);
    ASSERT_FALSE(res.empty);
    EXPECT_EQ(res.data.numeric<uint32_t>(), 300UL);

    res = dbi.get(-4.4);
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_string_string)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_string_string", false, true);
    dbi.put(std::string { "hello1" }, std::string { "world1" });
    dbi.put(std::string { "hello2" }, std::string { "world2" });
    dbi.put(std::string { "hello3" }, std::string { "world3" });

    auto res = dbi.get(std::string { "hello4" });
    ASSERT_TRUE(res.empty);

    dbi.del(std::string { "hello1" });
    res = dbi.get(std::string { "hello1" });
    ASSERT_TRUE(res.empty);

    dbi.del(std::string { "hello2" });
    res = dbi.get(std::string { "hello2" });
    ASSERT_TRUE(res.empty);

    dbi.del(std::string { "hello3" });
    res = dbi.get(std::string { "hello3" });
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_numeric_string)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_numeric_string", true, true);
    dbi.put(1UL, std::string { "world1" });
    dbi.put(2UL, std::string { "world2" });
    dbi.put(3UL, std::string { "world3" });
    dbi.put(0UL, std::string { "world0" });
    dbi.put(std::numeric_limits<uint64_t>::max(), std::string { "worldmax" });

    dbi.del(1UL);
    auto res = dbi.get(1UL);
    ASSERT_TRUE(res.empty);

    dbi.del(2UL);
    res = dbi.get(2UL);
    ASSERT_TRUE(res.empty);

    dbi.del(3UL);
    res = dbi.get(3UL);
    ASSERT_TRUE(res.empty);

    dbi.del(0UL);
    res = dbi.get(0UL);
    ASSERT_TRUE(res.empty);

    dbi.del(std::numeric_limits<uint64_t>::max());
    res = dbi.get(std::numeric_limits<uint64_t>::max());
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_decimal_string)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_decimal_string", true, true);
    dbi.put(1.1, std::string { "world1" });
    dbi.put(-2.2, std::string { "world2" });
    dbi.put(3.3, std::string { "world3" });
    dbi.put(std::numeric_limits<double>::min(), std::string { "worldmin" });
    dbi.put(std::numeric_limits<double>::max(), std::string { "worldmax" });

    dbi.del(1.1);
    auto res = dbi.get(1.1);
    ASSERT_TRUE(res.empty);

    dbi.del(-2.2);
    res = dbi.get(-2.2);
    ASSERT_TRUE(res.empty);

    dbi.del(3.3);
    res = dbi.get(3.3);
    ASSERT_TRUE(res.empty);

    dbi.del(std::numeric_limits<double>::min());
    res = dbi.get(std::numeric_limits<double>::min());
    ASSERT_TRUE(res.empty);

    dbi.del(std::numeric_limits<double>::max());
    res = dbi.get(std::numeric_limits<double>::max());
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_string_blob)
{
    beforeEach();

    auto blob = setBlob("world", 128);
    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_string_blob", false, true);
    dbi.put(std::string { "hello" }, blob);

    dbi.del(std::string { "hello" });
    auto res = dbi.get(std::string { "hello" });
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_numeric_blob)
{
    beforeEach();

    auto blob = setBlob("world", 128);
    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_numeric_blob", false, true);
    dbi.put(1UL, blob);

    dbi.del(1UL);
    auto res = dbi.get(1UL);
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_decimal_blob)
{
    beforeEach();

    auto blob = setBlob("world", 128);
    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_decimal_blob", false, true);
    dbi.put(-123.4567, blob);

    dbi.del(-123.4567);
    auto res = dbi.get(-123.4567);
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_string_numeric)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_string_numeric", false, true);
    dbi.put(std::string { "hello1" }, 100UL);
    dbi.put(std::string { "hello2" }, 200UL);
    dbi.put(std::string { "hello3" }, 300UL);

    dbi.del(std::string { "hello1" });
    auto res = dbi.get(std::string { "hello1" });
    ASSERT_TRUE(res.empty);

    dbi.del(std::string { "hello2" });
    res = dbi.get(std::string { "hello2" });
    ASSERT_TRUE(res.empty);

    dbi.del(std::string { "hello3" });
    res = dbi.get(std::string { "hello3" });
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_numeric_numeric)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_numeric_numeric", true, true);
    dbi.put(1UL, 100UL);
    dbi.put(2UL, 200UL);
    dbi.put(3UL, 300UL);

    dbi.del(1UL);
    auto res = dbi.get(1UL);
    ASSERT_TRUE(res.empty);

    dbi.del(2UL);
    res = dbi.get(2UL);
    ASSERT_TRUE(res.empty);

    dbi.del(3UL);
    res = dbi.get(3UL);
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_decimal_numeric)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_decimal_numeric", true, true);
    dbi.put(1.1, 100UL);
    dbi.put(-2.2, 200UL);
    dbi.put(3.3, 300UL);

    dbi.del(1.1);
    auto res = dbi.get(1.1);
    ASSERT_TRUE(res.empty);

    dbi.del(-2.2);
    res = dbi.get(-2.2);
    ASSERT_TRUE(res.empty);

    dbi.del(3.3);
    res = dbi.get(3.3);
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_string_string_dup)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_string_string_dup", false, false);
    dbi.put(std::string { "hello1" }, std::string { "world1" });
    dbi.put(std::string { "hello2" }, std::string { "world2" });
    dbi.put(std::string { "hello1" }, std::string { "world3" });
    dbi.put(std::string { "hello2" }, std::string { "world4" });

    dbi.del(std::string { "hello1" });
    auto res = dbi.get(std::string { "hello1" });
    ASSERT_TRUE(res.empty);

    dbi.del(std::string { "hello2" });
    res = dbi.get(std::string { "hello2" });
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_numeric_string_dup)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_numeric_string_dup", true, false);
    dbi.put(1UL, std::string { "world1" });
    dbi.put(2UL, std::string { "world2" });
    dbi.put(1UL, std::string { "world3" });
    dbi.put(2UL, std::string { "world4" });

    dbi.del(1UL);
    auto res = dbi.get(1UL);
    ASSERT_TRUE(res.empty);

    dbi.del(2UL);
    res = dbi.get(2UL);
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_decimal_string_dup)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_decimal_string_dup", true, false);
    dbi.put(1.1, std::string { "world1" });
    dbi.put(-2.2, std::string { "world2" });
    dbi.put(1.1, std::string { "world3" });
    dbi.put(-2.2, std::string { "world4" });

    dbi.del(1.1);
    auto res = dbi.get(1.1);
    ASSERT_TRUE(res.empty);

    dbi.del(-2.2);
    res = dbi.get(-2.2);
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_string_blob_dup)
{
    beforeEach();

    auto blob1 = setBlob("world1", 128);
    auto blob2 = setBlob("world2", 256);
    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_string_blob_dup", false, false);
    dbi.put(std::string { "hello" }, blob1);
    dbi.put(std::string { "hello" }, blob2);

    dbi.del(std::string { "hello" });
    auto res = dbi.get(std::string { "hello" });
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_numeric_blob_dup)
{
    beforeEach();

    auto blob1 = setBlob("world1", 128);
    auto blob2 = setBlob("world2", 256);
    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_numeric_blob_dup", true, false);
    dbi.put(1UL, blob1);
    dbi.put(1UL, blob2);

    dbi.del(1UL);
    auto res = dbi.get(1UL);
    ASSERT_TRUE(res.empty);
    ;

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_decimal_blob_dup)
{
    beforeEach();

    auto blob1 = setBlob("world1", 128);
    auto blob2 = setBlob("world2", 256);
    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_decimal_blob_dup", true, false);
    dbi.put(1.234, blob1);
    dbi.put(1.234, blob2);

    dbi.del(1.234);
    auto res = dbi.get(1.234);
    ASSERT_TRUE(res.empty);
    ;

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_string_numeric_dup)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_string_numeric_dup", false, false);
    dbi.put(std::string { "hello1" }, 100UL);
    dbi.put(std::string { "hello1" }, 200UL);
    dbi.put(std::string { "hello1" }, 300UL);

    dbi.del(std::string { "hello1" });
    auto res = dbi.get(std::string { "hello1" });
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_numeric_numeric_dup)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_numeric_numeric_dup", true, false);
    dbi.put(1UL, 100UL);
    dbi.put(1UL, 200UL);
    dbi.put(1UL, 300UL);

    dbi.del(1UL);
    auto res = dbi.get(1UL);
    ASSERT_TRUE(res.empty);

    afterEach();
}

TEST_F(LMDBBasicOperations, put_get_del_decimal_numeric_dup)
{
    beforeEach();

    auto dbi = txn->openDBi("LMDBBasicOperations::put_get_del_decimal_numeric_dup", true, true);
    dbi.put(-1.1, 100UL);
    dbi.put(-1.1, 200UL);
    dbi.put(-1.1, 300UL);

    dbi.del(-1.1);
    auto res = dbi.get(-1.1);
    ASSERT_TRUE(res.empty);

    afterEach();
}