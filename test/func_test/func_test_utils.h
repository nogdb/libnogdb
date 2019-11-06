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

#pragma once

#include <cassert>
#include <dirent.h>
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <sys/file.h>
#include <type_traits>
#include <unistd.h>

#include "func_test_config.h"

#include "nogdb/nogdb.h"

inline void init()
{
    // clear_dir
    DIR* theFolder = opendir(DATABASE_PATH.c_str());
    if (theFolder != NULL) {
        struct dirent* next_file;
        char filepath[256];
        while ((next_file = readdir(theFolder)) != NULL) {
            sprintf(filepath, "%s/%s", DATABASE_PATH.c_str(), next_file->d_name);
            remove(filepath);
        }
        closedir(theFolder);
        rmdir(DATABASE_PATH.c_str());
    }
    // create database
    auto ctxi = nogdb::ContextInitializer(DATABASE_PATH);
#ifdef ENABLE_TEST_RECORD_VERSION
    ctxi.enableVersion();
    std::cout << "Initializing Database Context with version...\n";
#else
    std::cout << "Initializing Database Context...\n";
#endif
    ctxi.init();
}

#define REQUIRE(_err, _exp, _msg) require(_err, _exp, _msg, __FUNCTION__, __LINE__, __FILE__)

inline void require(const nogdb::Error& err, const int expect, const std::string& msg, const std::string& funcName,
    const int lineNumber, const std::string& fileName)
{
    if (err.code() != expect) {
        std::cout << "\x1B[31m"
                  << "\n[error] Expect:\t" << msg << " to be returned in " << funcName << ", file " << fileName
                  << ", line " << std::dec << lineNumber << ".\n"
                  << "        Actual:\t" << err.what() << ".\x1B[0m\n";
        assert(0);
    }
}

#define ASSERT_SIZE(_rs, _exp) assertSize(_rs, _exp, __FUNCTION__, __LINE__, __FILE__)

inline void assertSize(const nogdb::ResultSet& rs, const size_t expectedSize, const std::string& funcName,
    const int lineNumber, const std::string& fileName)
{
    if (rs.size() != expectedSize) {
        std::cout << "\x1B[31m"
                  << "\n[error] Expect:\t" << expectedSize << " in " << funcName << ", file " << fileName << ", line "
                  << std::dec << lineNumber << ".\n"
                  << "        Actual:\t" << rs.size() << ".\x1B[0m\n";
        assert(0);
    }
}

inline void assertSize(const nogdb::ResultSetCursor& rs, const size_t expectedSize, const std::string& funcName,
    const int lineNumber, const std::string& fileName)
{
    if (rs.size() != expectedSize) {
        std::cout << "\x1B[31m"
                  << "\n[error] Expect:\t" << expectedSize << " in " << funcName << ", file " << fileName << ", line "
                  << std::dec << lineNumber << ".\n"
                  << "        Actual:\t" << rs.size() << ".\x1B[0m\n";
        assert(0);
    }
}

#define ASSERT_EQ(_val, _exp) assertEqual(_val, _exp, __FUNCTION__, __LINE__, __FILE__)

template <typename T>
inline void assertEqual(
    const T& value, const T& expected, const std::string& funcName, const int lineNumber, const std::string& fileName)
{
    if (value != expected) {
        std::cout << "\x1B[31m"
                  << "\n[error] Expect:\t" << expected << " in " << funcName << ", file " << fileName << ", line "
                  << std::dec << lineNumber << ".\n"
                  << "        Actual:\t" << value << ".\x1B[0m\n";
        assert(0);
    }
}

#define ASSERT_NE(_val, _exp) assertNotEqual(_val, _exp, __FUNCTION__, __LINE__, __FILE__)

template <typename T>
inline void assertNotEqual(
    const T& value, const T& expected, const std::string& funcName, const int lineNumber, const std::string& fileName)
{
    if (value == expected) {
        std::cout << "\x1B[31m"
                  << "\n[error] Expect:\t" << expected << " in " << funcName << ", file " << fileName << ", line "
                  << std::dec << lineNumber << ".\n"
                  << "        Actual:\t" << value << ".\x1B[0m\n";
        assert(0);
    }
}

#define ASSERT_TRUE(_val) assertTrue(_val, __FUNCTION__, __LINE__, __FILE__)

inline void assertTrue(
    const bool value, const std::string& funcName, const int lineNumber, const std::string& fileName)
{
    if (value == 0) {
        std::cout << "\x1B[31m"
                  << "\n[error] Expect:\ttrue in " << funcName << ", file " << fileName << ", line "
                  << std::dec << lineNumber << ".\n"
                  << "        Actual:\t" << value << ".\x1B[0m\n";
        assert(0);
    }
}

#define ASSERT_FALSE(_val) assertFalse(_val, __FUNCTION__, __LINE__, __FILE__)

inline void assertFalse(
    const bool value, const std::string& funcName, const int lineNumber, const std::string& fileName)
{
    if (value != 0) {
        std::cout << "\x1B[31m"
                  << "\n[error] Expect:\tfalse in " << funcName << ", file " << fileName << ", line "
                  << std::dec << lineNumber << ".\n"
                  << "        Actual:\t" << value << ".\x1B[0m\n";
        assert(0);
    }
}

inline void verbose(const nogdb::ResultSet& rs)
{
    std::cout << "\nSize:" << rs.size() << '\n';
    for (const auto& r : rs) {
        std::cout << r.record.get("name").toText() << '\n';
    }
}

inline bool compareText(
    const nogdb::ResultSet& rss, const std::string& propName, const std::vector<std::string>& expectedRss)
{
    auto result = true;
    for (const auto& rs : rss) {
        auto value = rs.record.get(propName).toText();
        result &= std::find(expectedRss.cbegin(), expectedRss.cend(), value) != expectedRss.cend();
    }
    return result;
}

inline void runTestCases(
    nogdb::Transaction& txn, const std::vector<std::function<void(nogdb::Transaction&)>>& testCases, bool mustPass)
{
    auto counter = 0;
    for (auto& testCase : testCases) {
        ++counter;
        if (mustPass) {
            try {
                testCase(txn);
            } catch (...) {
                std::cout << "[error] died at " << counter << '\n';
                assert(false);
            }
        } else {
            try {
                testCase(txn);
                std::cout << "[error] died at " << counter << '\n';
                assert(false);
            } catch (...) {
                assert(true);
            }
        }
    }
}

inline nogdb::ResultSet getVertexMultipleClass(nogdb::Transaction& txn, const std::set<std::string>& classNames)
{
    auto res = nogdb::ResultSet {};
    for (const auto& className : classNames) {
        auto tmp = txn.find(className).get();
        res.insert(res.end(), tmp.cbegin(), tmp.cend());
    }
    return res;
}

inline nogdb::ResultSet getEdgeMultipleClass(nogdb::Transaction& txn, const std::set<std::string>& classNames)
{
    auto res = nogdb::ResultSet {};
    for (const auto& className : classNames) {
        auto tmp = txn.find(className).get();
        res.insert(res.end(), tmp.cbegin(), tmp.cend());
    }
    return res;
}

inline nogdb::ResultSet getVertexMultipleClassExtend(nogdb::Transaction& txn, const std::set<std::string>& classNames)
{
    auto res = nogdb::ResultSet {};
    for (const auto& className : classNames) {
        auto tmp = txn.findSubClassOf(className).get();
        res.insert(res.end(), tmp.cbegin(), tmp.cend());
    }
    return res;
}

inline nogdb::ResultSet getEdgeMultipleClassExtend(nogdb::Transaction& txn, const std::set<std::string>& classNames)
{
    auto res = nogdb::ResultSet {};
    for (const auto& className : classNames) {
        auto tmp = txn.findSubClassOf(className).get();
        res.insert(res.end(), tmp.cbegin(), tmp.cend());
    }
    return res;
}

inline bool rdescCompare(const std::string& propertyName, const nogdb::ResultSet& res,
    const std::vector<nogdb::RecordDescriptor>& expectedResult)
{
    auto compareRes = true;
    if (res.size() != expectedResult.size()) {
        compareRes = false;
        std::cout << propertyName << "\n";
        std::cout << "\x1B[31m"
                  << "\n[error] Expect:\t" << expectedResult.size() << "\n"
                  << "        Actual:\t" << res.size() << "\x1B[0m\n";
        std::cout << "\x1B[31m"
                  << "\n[error] Expect:\t [ ";
        for (const auto& r : expectedResult) {
            std::cout << r.rid << " ";
        }
        std::cout << "]\n";
        std::cout << "        Actual:\t [ ";
        for (const auto& r : res) {
            std::cout << r.descriptor.rid << " ";
        }
        std::cout << "]\x1B[0m\n";
    } else {
        auto index = 0;
        auto expectedResultSorted = expectedResult;
        std::sort(expectedResultSorted.begin(), expectedResultSorted.end(),
            [](const nogdb::RecordDescriptor& lhs, const nogdb::RecordDescriptor& rhs) { return lhs.rid < rhs.rid; });
        for (const auto& r : res) {
            auto cmp = r.descriptor.rid == expectedResultSorted[index].rid;
            compareRes &= cmp;
            if (!cmp) {
                std::cout << propertyName << "\n";
                std::cout << "\x1B[31m"
                          << "\n[error] Expect:\t" << expectedResultSorted[index].rid << "\n"
                          << "        Actual:\t" << r.descriptor.rid << ".\x1B[0m\n";
            }
            ++index;
        }
    }
    return compareRes;
}

template<typename T, typename std::enable_if<std::is_base_of<nogdb::OperationBuilder, T>::value>::type* = nullptr>
inline bool resultSetCountCompare(T& queryBuilder) {
    auto res = queryBuilder.get();
    auto resCursor = queryBuilder.getCursor();
    auto resCount = queryBuilder.count();
    return (resCount == res.size()) && (resCount == resCursor.size());
}