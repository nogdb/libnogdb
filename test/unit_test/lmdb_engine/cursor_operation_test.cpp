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

TEST_F(LMDBCursorOperations, put_find_get_string_string)
{
    beforeEach();
    {
        auto dbi = txn->openDBi("LMDBCursorOperations::put_find_get_string_string", false, true);
        dbi.put(std::string { "hello1" }, std::string { "world1" });
        dbi.put(std::string { "hello2" }, std::string { "world2" });
        dbi.put(std::string { "hello3" }, std::string { "world3" });
        dbi.put(std::string { "hello4" }, std::string { "world4" });
        dbi.put(std::string { "hello5" }, std::string { "world5" });

        auto cursor = txn->openCursor(dbi);

        // find exact match and get next iterations
        auto res = cursor.find(std::string { "hello3" });
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world3" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello4" });
        ASSERT_EQ(res.val.data.string(), std::string { "world4" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello5" });
        ASSERT_EQ(res.val.data.string(), std::string { "world5" });

        res = cursor.getNext();
        ASSERT_TRUE(res.empty());

        // find exact match and get previous iterations
        res = cursor.find(std::string { "hello2" });
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world2" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello1" });
        ASSERT_EQ(res.val.data.string(), std::string { "world1" });

        res = cursor.getPrev();
        ASSERT_TRUE(res.empty());

        // find range match and get next iterations
        res = cursor.find(std::string { "hello0" });
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(std::string { "hello0" });
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello1" });
        ASSERT_EQ(res.val.data.string(), std::string { "world1" });

        res = cursor.findRange(std::string { "hello1" });
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello1" });
        ASSERT_EQ(res.val.data.string(), std::string { "world1" });

        res = cursor.findRange(std::string { "hello2" });
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello2" });
        ASSERT_EQ(res.val.data.string(), std::string { "world2" });

        // find range match and get previous iterations
        res = cursor.find(std::string { "hello6" });
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(std::string { "hello6" });
        ASSERT_TRUE(res.empty());

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello5" });
        ASSERT_EQ(res.val.data.string(), std::string { "world5" });

        res = cursor.findRange(std::string { "hello5" });
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello5" });
        ASSERT_EQ(res.val.data.string(), std::string { "world5" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello4" });
        ASSERT_EQ(res.val.data.string(), std::string { "world4" });

        res = cursor.findRange(std::string { "hello4" });
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello4" });
        ASSERT_EQ(res.val.data.string(), std::string { "world4" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello3" });
        ASSERT_EQ(res.val.data.string(), std::string { "world3" });
    }
    afterEach();
}

TEST_F(LMDBCursorOperations, put_find_get_uint64_string)
{
    beforeEach();
    {
        auto dbi = txn->openDBi("LMDBCursorOperations::put_find_get_uint64_string", true, true);
        dbi.put(1ULL, std::string { "world1" });
        dbi.put(2ULL, std::string { "world2" });
        dbi.put(3ULL, std::string { "world3" });
        dbi.put(4ULL, std::string { "world4" });
        dbi.put(5ULL, std::string { "world5" });

        auto cursor = txn->openCursor(dbi);

        // find exact match and get next iterations
        auto res = cursor.find(3ULL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world3" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 4ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world4" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 5ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world5" });

        res = cursor.getNext();
        ASSERT_TRUE(res.empty());

        // find exact match and get previous iterations
        res = cursor.find(2ULL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world2" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 1ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1" });

        res = cursor.getPrev();
        ASSERT_TRUE(res.empty());

        // find range match and get next iterations
        res = cursor.find(0ULL);
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(0ULL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 1ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1" });

        res = cursor.findRange(1ULL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 1ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1" });

        res = cursor.findRange(2ULL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 2ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world2" });

        // find range match and get previous iterations
        res = cursor.find(6ULL);
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(6ULL);
        ASSERT_TRUE(res.empty());

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 5ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world5" });

        res = cursor.findRange(5ULL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 5ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world5" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 4ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world4" });

        res = cursor.findRange(4ULL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 4ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world4" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 3ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world3" });
    }
    afterEach();
}

TEST_F(LMDBCursorOperations, put_find_get_int64_string)
{
    beforeEach();
    {
        auto dbi = txn->openDBi("LMDBCursorOperations::put_find_get_int64_string", true, true);
        dbi.put(-50LL, std::string { "world1" });
        dbi.put(-40LL, std::string { "world2" });
        dbi.put(-30LL, std::string { "world3" });
        dbi.put(-20LL, std::string { "world4" });
        dbi.put(-10LL, std::string { "world5" });

        auto cursor = txn->openCursor(dbi);

        // find exact match and get next iterations
        auto res = cursor.find(-30LL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world3" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -20LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world4" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -10LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world5" });

        res = cursor.getNext();
        ASSERT_TRUE(res.empty());

        // find exact match and get previous iterations
        res = cursor.find(-40LL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world2" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -50LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1" });

        res = cursor.getPrev();
        ASSERT_TRUE(res.empty());

        // find range match and get next iterations
        res = cursor.find(-999LL);
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(-999LL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -50LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1" });

        res = cursor.findRange(-50LL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -50LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1" });

        res = cursor.findRange(-40LL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -40LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world2" });

        // find range match and get previous iterations
        res = cursor.find(-1LL);
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(-1LL);
        ASSERT_TRUE(res.empty());

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -10LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world5" });

        res = cursor.findRange(-10LL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -10ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world5" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -20ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world4" });

        res = cursor.findRange(-20LL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -20LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world4" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -30ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world3" });
    }
    afterEach();
}

TEST_F(LMDBCursorOperations, put_find_get_decimal_string)
{
    beforeEach();
    {
        auto dbi = txn->openDBi("LMDBCursorOperations::put_find_get_decimal_string", true, true);
        dbi.put(1.1, std::string { "world1" });
        dbi.put(2.2, std::string { "world2" });
        dbi.put(3.3, std::string { "world3" });
        dbi.put(4.4, std::string { "world4" });
        dbi.put(5.5, std::string { "world5" });

        auto cursor = txn->openCursor(dbi);

        // find exact match and get next iterations
        auto res = cursor.find(3.3);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world3" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 4.4);
        ASSERT_EQ(res.val.data.string(), std::string { "world4" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 5.5);
        ASSERT_EQ(res.val.data.string(), std::string { "world5" });

        res = cursor.getNext();
        ASSERT_TRUE(res.empty());

        // find exact match and get previous iterations
        res = cursor.find(2.2);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world2" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 1.1);
        ASSERT_EQ(res.val.data.string(), std::string { "world1" });

        res = cursor.getPrev();
        ASSERT_TRUE(res.empty());

        // find range match and get next iterations
        res = cursor.find(0.0);
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(0.0);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 1.1);
        ASSERT_EQ(res.val.data.string(), std::string { "world1" });

        res = cursor.findRange(1.1);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 1.1);
        ASSERT_EQ(res.val.data.string(), std::string { "world1" });

        res = cursor.findRange(2.2);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 2.2);
        ASSERT_EQ(res.val.data.string(), std::string { "world2" });

        // find range match and get previous iterations
        res = cursor.find(6.6);
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(6.6);
        ASSERT_TRUE(res.empty());

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 5.5);
        ASSERT_EQ(res.val.data.string(), std::string { "world5" });

        res = cursor.findRange(5.5);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 5.5);
        ASSERT_EQ(res.val.data.string(), std::string { "world5" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 4.4);
        ASSERT_EQ(res.val.data.string(), std::string { "world4" });

        res = cursor.findRange(4.4);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 4.4);
        ASSERT_EQ(res.val.data.string(), std::string { "world4" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 3.3);
        ASSERT_EQ(res.val.data.string(), std::string { "world3" });
    }
    afterEach();
}

TEST_F(LMDBCursorOperations, put_find_get_string_string_dup)
{
    beforeEach();
    {
        auto dbi = txn->openDBi("LMDBCursorOperations::put_find_get_string_string_dup", false, false);
        dbi.put(std::string { "hello1" }, std::string { "world1-1" });
        dbi.put(std::string { "hello1" }, std::string { "world1-2" });
        dbi.put(std::string { "hello2" }, std::string { "world2-1" });
        dbi.put(std::string { "hello2" }, std::string { "world2-2" });
        dbi.put(std::string { "hello3" }, std::string { "world3-1" });
        dbi.put(std::string { "hello3" }, std::string { "world3-2" });

        auto cursor = txn->openCursor(dbi);

        // find exact match and get next iterations
        auto res = cursor.find(std::string { "hello2" });
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello2" });
        ASSERT_EQ(res.val.data.string(), std::string { "world2-2" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello3" });
        ASSERT_EQ(res.val.data.string(), std::string { "world3-1" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello3" });
        ASSERT_EQ(res.val.data.string(), std::string { "world3-2" });

        res = cursor.getNext();
        ASSERT_TRUE(res.empty());

        // find exact match and get previous iterations
        res = cursor.find(std::string { "hello2" });
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello1" });
        ASSERT_EQ(res.val.data.string(), std::string { "world1-2" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello1" });
        ASSERT_EQ(res.val.data.string(), std::string { "world1-1" });

        res = cursor.getPrev();
        ASSERT_TRUE(res.empty());

        // find range match and get next iterations
        res = cursor.find(std::string { "hello0" });
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(std::string { "hello0" });
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello1" });
        ASSERT_EQ(res.val.data.string(), std::string { "world1-1" });

        res = cursor.findRange(std::string { "hello1" });
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello1" });
        ASSERT_EQ(res.val.data.string(), std::string { "world1-1" });

        res = cursor.findRange(std::string { "hello2" });
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello2" });
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        // find range match and get previous iterations
        res = cursor.find(std::string { "hello6" });
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(std::string { "hello6" });
        ASSERT_TRUE(res.empty());

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello3" });
        ASSERT_EQ(res.val.data.string(), std::string { "world3-2" });

        res = cursor.findRange(std::string { "hello3" });
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello3" });
        ASSERT_EQ(res.val.data.string(), std::string { "world3-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello2" });
        ASSERT_EQ(res.val.data.string(), std::string { "world2-2" });

        res = cursor.findRange(std::string { "hello2" });
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello2" });
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.string(), std::string { "hello1" });
        ASSERT_EQ(res.val.data.string(), std::string { "world1-2" });
    }
    afterEach();
}

TEST_F(LMDBCursorOperations, put_find_get_uint64_string_dup)
{
    beforeEach();
    {
        auto dbi = txn->openDBi("LMDBCursorOperations::put_find_get_uint64_string_dup", true, false);
        dbi.put(1ULL, std::string { "world1-1" });
        dbi.put(1ULL, std::string { "world1-2" });
        dbi.put(2ULL, std::string { "world2-1" });
        dbi.put(2ULL, std::string { "world2-2" });
        dbi.put(3ULL, std::string { "world3-1" });
        dbi.put(3ULL, std::string { "world3-2" });

        auto cursor = txn->openCursor(dbi);

        // find exact match and get next iterations
        auto res = cursor.find(2ULL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 2ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-2" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 3ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world3-1" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 3ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world3-2" });

        res = cursor.getNext();
        ASSERT_TRUE(res.empty());

        // find exact match and get previous iterations
        res = cursor.find(3ULL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world3-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 2ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-2" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 2ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 1ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-2" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 1ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-1" });

        res = cursor.getPrev();
        ASSERT_TRUE(res.empty());

        // find range match and get next iterations
        res = cursor.find(0ULL);
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(0ULL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 1ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-1" });

        res = cursor.findRange(1ULL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 1ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-1" });

        res = cursor.findRange(2ULL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 2ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        // find range match and get previous iterations
        res = cursor.find(6ULL);
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(6ULL);
        ASSERT_TRUE(res.empty());

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 3ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world3-2" });

        res = cursor.findRange(3ULL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 3ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world3-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 2ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-2" });

        res = cursor.findRange(2ULL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 2ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<uint64_t>(), 1ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-2" });
    }
    afterEach();
}

TEST_F(LMDBCursorOperations, put_find_get_int64_string_dup)
{
    beforeEach();
    {
        auto dbi = txn->openDBi("LMDBCursorOperations::put_find_get_int64_string_dup", true, false);
        dbi.put(-30LL, std::string { "world1-1" });
        dbi.put(-30LL, std::string { "world1-2" });
        dbi.put(-20LL, std::string { "world2-1" });
        dbi.put(-20LL, std::string { "world2-2" });
        dbi.put(-10LL, std::string { "world3-1" });
        dbi.put(-10LL, std::string { "world3-2" });

        auto cursor = txn->openCursor(dbi);

        // find exact match and get next iterations
        auto res = cursor.find(-20LL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -20LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-2" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -10LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world3-1" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -10LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world3-2" });

        res = cursor.getNext();
        ASSERT_TRUE(res.empty());

        // find exact match and get previous iterations
        res = cursor.find(-10LL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world3-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -20LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-2" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -20LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -30LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-2" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -30LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-1" });

        res = cursor.getPrev();
        ASSERT_TRUE(res.empty());

        // find range match and get next iterations
        res = cursor.find(-999LL);
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(-999LL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -30LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-1" });

        res = cursor.findRange(-30LL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -30LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-1" });

        res = cursor.findRange(-20LL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -20LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        // find range match and get previous iterations
        res = cursor.find(-1LL);
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(-1LL);
        ASSERT_TRUE(res.empty());

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -10LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world3-2" });

        res = cursor.findRange(-10LL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -10ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world3-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -20ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-2" });

        res = cursor.findRange(-20LL);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -20LL);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<int64_t>(), -30ULL);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-2" });
    }
    afterEach();
}

TEST_F(LMDBCursorOperations, put_find_get_decimal_string_dup)
{
    beforeEach();
    {
        auto dbi = txn->openDBi("LMDBCursorOperations::put_find_get_decimal_string_dup", true, false);
        dbi.put(1.1, std::string { "world1-1" });
        dbi.put(1.1, std::string { "world1-2" });
        dbi.put(2.2, std::string { "world2-1" });
        dbi.put(2.2, std::string { "world2-2" });
        dbi.put(3.3, std::string { "world3-1" });
        dbi.put(3.3, std::string { "world3-2" });

        auto cursor = txn->openCursor(dbi);

        // find exact match and get next iterations
        auto res = cursor.find(2.2);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 2.2);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-2" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 3.3);
        ASSERT_EQ(res.val.data.string(), std::string { "world3-1" });

        res = cursor.getNext();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 3.3);
        ASSERT_EQ(res.val.data.string(), std::string { "world3-2" });

        res = cursor.getNext();
        ASSERT_TRUE(res.empty());

        // find exact match and get previous iterations
        res = cursor.find(3.3);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.val.data.string(), std::string { "world3-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 2.2);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-2" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 2.2);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 1.1);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-2" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 1.1);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-1" });

        res = cursor.getPrev();
        ASSERT_TRUE(res.empty());

        // find range match and get next iterations
        res = cursor.find(0.0);
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(0.0);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 1.1);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-1" });

        res = cursor.findRange(1.1);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 1.1);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-1" });

        res = cursor.findRange(2.2);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 2.2);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        // find range match and get previous iterations
        res = cursor.find(6.6);
        ASSERT_TRUE(res.empty());

        res = cursor.findRange(6.6);
        ASSERT_TRUE(res.empty());

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 3.3);
        ASSERT_EQ(res.val.data.string(), std::string { "world3-2" });

        res = cursor.findRange(3.3);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 3.3);
        ASSERT_EQ(res.val.data.string(), std::string { "world3-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 2.2);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-2" });

        res = cursor.findRange(2.2);
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 2.2);
        ASSERT_EQ(res.val.data.string(), std::string { "world2-1" });

        res = cursor.getPrev();
        ASSERT_FALSE(res.empty());
        ASSERT_EQ(res.key.data.numeric<double>(), 1.1);
        ASSERT_EQ(res.val.data.string(), std::string { "world1-2" });
    }
    afterEach();
}