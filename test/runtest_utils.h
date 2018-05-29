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

#ifndef RUNTEST_UTILS_H_
#define RUNTEST_UTILS_H_

#include <cassert>
#include <type_traits>
#include <sstream>
#include <string>
#include <functional>
#include <set>
#include "nogdb.h"

inline void init() {
    const std::string clear_dir_command = "rm -rf " + DATABASE_PATH;
    system(clear_dir_command.c_str());
}

inline void showSchema(const nogdb::Txn &txn) {
    auto info = nogdb::Db::getDbInfo(txn);
    std::cout << "db_path = " << info.dbPath << "\n"
              << "max_db = " << info.maxDB << "\n"
              << "max_db_size = " << info.maxDBSize << "\n"
              << "num_class = " << info.numClass << "\n"
              << "num_property = " << info.numProperty << "\n"
              << "max_class_id = " << info.maxClassId << "\n"
              << "max_property_id = " << info.maxPropertyId << "\n";
    auto schema = std::vector<nogdb::ClassDescriptor>{};
    try {
        schema = nogdb::Db::getSchema(txn);
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(0);
    }
    for (auto db = schema.cbegin(); db != schema.cend(); ++db) {
        std::cout << "class id = " << db->id << ", name = '" << db->name << "', "
                  << "type = '" << db->type << "'\n"
                  << "+--------------+--------------------------+--------------+\n"
                  << "| property id  |      property name       |     type     |\n"
                  << "+--------------+--------------------------+--------------+\n";
        auto properties = db->properties;
        for (auto prop = properties.begin(); prop != properties.end(); ++prop) {
            std::cout << "| "
                      << std::setfill(' ') << std::setw(COLUMN_ID_OFFSET) << std::left << prop->second.id
                      << "| "
                      << std::setfill(' ') << std::setw(COLUMN_NAME_OFFSET) << std::left << prop->first
                      << "| "
                      << std::setfill(' ') << std::setw(COLUMN_TYPE_OFFSET) << std::left << prop->second.type
                      << "|\n";
        }
        std::cout << "+--------------+--------------------------+--------------+\n";
    }
}

inline std::string rid2str(const nogdb::RecordId &rid) {
    std::stringstream ss{};
    ss << std::to_string(rid.first) << ":" << std::to_string(rid.second);
    return ss.str();
}

#define REQUIRE(_err, _exp, _msg) \
        require(_err, _exp, _msg, __FUNCTION__, __LINE__, __FILE__)

inline void require(const nogdb::Error &err,
                    const int expect,
                    const std::string &msg,
                    const std::string &funcName,
                    const int lineNumber,
                    const std::string &fileName
) {
    if (err.code() != expect) {
        std::cout << "\x1B[31m" << "\n[error] Expect:\t" << msg
                  << " to be returned in " << funcName
                  << ", file " << fileName
                  << ", line " << std::dec << lineNumber << ".\n"
                  << "        Actual:\t" << err.what()
                  << ".\x1B[0m\n";
        assert(0);
    }
}

inline void assertSize(const nogdb::ResultSet& rs, size_t expectedSize) {
    if (rs.size() != expectedSize) {
        std::cout << "\x1B[31m" << "\n[error] Expect:\t" << expectedSize << "\n"
                  << "        Actual:\t" << rs.size()
                  << ".\x1B[0m\n";
        assert(0);
    }
}

inline void assertSize(const nogdb::ResultSetCursor& rs, size_t expectedSize) {
    if (rs.size() != expectedSize) {
        std::cout << "\x1B[31m" << "\n[error] Expect:\t" << expectedSize << "\n"
                  << "        Actual:\t" << rs.size()
                  << ".\x1B[0m\n";
        assert(0);
    }
}

inline void verbose(const nogdb::ResultSet &rs) {
    std::cout << "\nSize:" << rs.size() << '\n';
    for (const auto &r: rs) {
        std::cout << r.record.get("name").toText() << '\n';
    }
}

inline bool
compareText(const nogdb::ResultSet &rss, const std::string &propName, const std::vector<std::string> &expectedRss) {
    auto result = true;
    for (const auto &rs: rss) {
        auto value = rs.record.get(propName).toText();
        result &= std::find(expectedRss.cbegin(), expectedRss.cend(), value) != expectedRss.cend();
    }
    return result;
}

inline void runTestCases(nogdb::Txn &txn,
                         const std::vector<std::function<void(nogdb::Txn &)>> &testCases,
                         bool mustPass) {
    auto counter = 0;
    for (auto &testCase: testCases) {
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

inline nogdb::ResultSet getVertexMultipleClass(nogdb::Txn &txn, const std::set<std::string> &classNames) {
    auto res = nogdb::ResultSet{};
    for (const auto &className: classNames) {
        auto tmp = nogdb::Vertex::get(txn, className);
        res.insert(res.end(), tmp.cbegin(), tmp.cend());
    }
    return res;
}

inline nogdb::ResultSet getEdgeMultipleClass(nogdb::Txn &txn, const std::set<std::string> &classNames) {
    auto res = nogdb::ResultSet{};
    for (const auto &className: classNames) {
        auto tmp = nogdb::Edge::get(txn, className);
        res.insert(res.end(), tmp.cbegin(), tmp.cend());
    }
    return res;
}

inline void cursorContains(nogdb::ResultSetCursor &rsCursor,
                           const std::set<std::string> &expectedResults,
                           const std::string &testColumn) {
    assert(rsCursor.empty() == expectedResults.empty());
    assert(rsCursor.size() == expectedResults.size());
    assert(rsCursor.count() == expectedResults.size());
    if (!expectedResults.empty()) {
        while (rsCursor.next()) {
            auto result = rsCursor->record.getText(testColumn);
            assert(expectedResults.find(result) != expectedResults.cend());
        }
    }
}

inline void cursorTester(nogdb::ResultSetCursor &rsCursor,
                         const std::vector<std::string> &expectedResults,
                         const std::string &testColumn) {
    assert(rsCursor.empty() == expectedResults.empty());
    assert(rsCursor.size() == expectedResults.size());
    assert(rsCursor.count() == expectedResults.size());
    if (!expectedResults.empty()) {
        auto count = 0UL;
        assert(rsCursor.hasNext());
        while (rsCursor.next()) {
            if (count < expectedResults.size() - 1) {
                assert(rsCursor->record.getText(testColumn) == expectedResults[count]);
                assert(rsCursor.hasNext());
            } else if (count == expectedResults.size() - 1) {
                assert(rsCursor->record.getText(testColumn) == expectedResults[count]);
                assert(!rsCursor.hasNext());
            } else {
                assert(false);
            }
            assert(rsCursor->record.getText("@recordId") == rid2str(rsCursor->descriptor.rid));
            ++count;
        }
        rsCursor.first();
        assert(!rsCursor.hasPrevious());
        assert(rsCursor->record.getText(testColumn) == *(expectedResults.cbegin()));
        assert(rsCursor.hasAt((expectedResults.size() - 1) / 2));
        assert(rsCursor.to((expectedResults.size() - 1) / 2));
        if (expectedResults.size() > 2) {
            assert(rsCursor.hasPrevious());
            assert(rsCursor.hasNext());
        }
        assert(rsCursor->record.getText(testColumn) == *(expectedResults.cbegin() + (expectedResults.size() - 1) / 2));
        rsCursor.last();
        assert(!rsCursor.hasNext());
        assert(rsCursor->record.getText(testColumn) == *(expectedResults.cend() - 1));
        count = expectedResults.size() - 1;
        while (rsCursor.previous()) {
            --count;
            if (count == 0) {
                assert(rsCursor->record.getText(testColumn) == expectedResults[count]);
                assert(!rsCursor.hasPrevious());
            } else if (count > 0) {
                assert(rsCursor->record.getText(testColumn) == expectedResults[count]);
                assert(rsCursor.hasPrevious());
            } else {
                assert(false);
            }
            assert(rsCursor->record.getText("@recordId") == rid2str(rsCursor->descriptor.rid));
        }
    }
}

inline void cursorTester(nogdb::ResultSetCursor &rsCursor,
                         const std::vector<unsigned int> &expectedResults,
                         const std::string &testColumn) {
    assert(rsCursor.empty() == expectedResults.empty());
    assert(rsCursor.size() == expectedResults.size());
    assert(rsCursor.count() == expectedResults.size());
    if (!expectedResults.empty()) {
        auto count = 0UL;
        assert(rsCursor.hasNext());
        while (rsCursor.next()) {
            if (count < expectedResults.size() - 1) {
                assert(rsCursor->record.getIntU(testColumn) == expectedResults[count]);
                assert(rsCursor.hasNext());
            } else if (count == expectedResults.size() - 1) {
                assert(rsCursor->record.getIntU(testColumn) == expectedResults[count]);
                assert(!rsCursor.hasNext());
            } else {
                assert(false);
            }
            assert(rsCursor->record.getText("@recordId") == rid2str(rsCursor->descriptor.rid));
            ++count;
        }
        rsCursor.first();
        assert(!rsCursor.hasPrevious());
        assert(rsCursor->record.getIntU(testColumn) == *(expectedResults.cbegin()));
        assert(rsCursor.hasAt((expectedResults.size() - 1) / 2));
        assert(rsCursor.to((expectedResults.size() - 1) / 2));
        if (expectedResults.size() > 2) {
            assert(rsCursor.hasPrevious());
            assert(rsCursor.hasNext());
        }
        assert(rsCursor->record.getIntU(testColumn) == *(expectedResults.cbegin() + (expectedResults.size() - 1) / 2));
        rsCursor.last();
        assert(!rsCursor.hasNext());
        assert(rsCursor->record.getIntU(testColumn) == *(expectedResults.cend() - 1));
        count = expectedResults.size() - 1;
        while (rsCursor.previous()) {
            --count;
            if (count == 0) {
                assert(rsCursor->record.getIntU(testColumn) == expectedResults[count]);
                assert(!rsCursor.hasPrevious());
            } else if (count > 0) {
                assert(rsCursor->record.getIntU(testColumn) == expectedResults[count]);
                assert(rsCursor.hasPrevious());
            } else {
                assert(false);
            }
            assert(rsCursor->record.getText("@recordId") == rid2str(rsCursor->descriptor.rid));
        }
    }
}

inline bool rdescCompare(const nogdb::ResultSet& res, const std::vector<nogdb::RecordDescriptor>& expectedResult) {
    auto compareRes = true;
    if (res.size() != expectedResult.size()) {
        compareRes = false;
        std::cout << "\x1B[31m" << "\n[error] Expect:\t" << expectedResult.size() << "\n"
                  << "        Actual:\t" << res.size()
                  << ".\x1B[0m\n";
        std::cout << "\x1B[31m" << "\n[error] Expect:\t [ ";
        for(const auto& r: expectedResult) {
            std::cout << r.rid << " ";
        }
        std::cout << "]\n";
        std::cout << "        Actual:\t [ ";
        for(const auto& r: res) {
            std::cout << r.descriptor.rid << " ";
        }
        std::cout << "]\x1B[0m\n";
    } else {
        auto index = 0;
        auto expectedResultSorted = expectedResult;
        std::sort(expectedResultSorted.begin(), expectedResultSorted.end(),
                  [](const nogdb::RecordDescriptor& lhs, const nogdb::RecordDescriptor& rhs){
            return lhs.rid < rhs.rid;
        });
        for(const auto& r: res) {
            auto cmp = r.descriptor.rid == expectedResultSorted[index].rid;
            compareRes &= cmp;
            if (!cmp) {
                std::cout << "\x1B[31m" << "\n[error] Expect:\t" << expectedResultSorted[index].rid << "\n"
                          << "        Actual:\t" << r.descriptor.rid
                          << ".\x1B[0m\n";
            }
            ++index;
        }
    }
    return compareRes;
}

inline bool rdescCursorCompare(nogdb::ResultSetCursor& res, const std::vector<nogdb::RecordDescriptor>& expectedResult) {
    auto compareRes = true;
    if (res.size() != expectedResult.size()) {
        compareRes = false;
        std::cout << "\x1B[31m" << "\n[error] Expect:\t" << expectedResult.size() << "\n"
                  << "        Actual:\t" << res.size()
                  << ".\x1B[0m\n";
        std::cout << "\x1B[31m" << "\n[error] Expect:\t [ ";
        for(const auto& r: expectedResult) {
            std::cout << r.rid << " ";
        }
        std::cout << "]\n";
        std::cout << "        Actual:\t [ ";
        for(res.next(); res.hasNext(); res.next()) {
            std::cout << res->descriptor.rid << " ";
        }
        std::cout << "]\x1B[0m\n";
    } else {
        auto index = 0;
        auto expectedResultSorted = expectedResult;
        std::sort(expectedResultSorted.begin(), expectedResultSorted.end(),
                  [](const nogdb::RecordDescriptor& lhs, const nogdb::RecordDescriptor& rhs){
                      return lhs.rid < rhs.rid;
                  });
        for(res.next(); res.hasNext(); res.next()) {
            auto cmp = res->descriptor.rid == expectedResultSorted[index].rid;
            compareRes &= cmp;
            if (!cmp) {
                std::cout << "\x1B[31m" << "\n[error] Expect:\t" << expectedResultSorted[index].rid << "\n"
                          << "        Actual:\t" << res->descriptor.rid
                          << ".\x1B[0m\n";
            }
            ++index;
        }
    }
    return compareRes;
}

template<typename T>
void indexConditionTester(nogdb::Context *ctx, const std::string& className, const std::string& propertyName,
                          const nogdb::RecordDescriptor& rdescMin, const T& min,
                          const nogdb::RecordDescriptor& rdescFirstMid, const T& firstMid,
                          const nogdb::RecordDescriptor& rdescSecondMid, const T& secondMid,
                          const nogdb::RecordDescriptor& rdescMax, const T& max) {

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).eq(min));
        assert(rdescCompare(res, {rdescMin}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).eq(firstMid));
        assert(rdescCompare(res, {rdescFirstMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).eq(secondMid));
        assert(rdescCompare(res, {rdescSecondMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).eq(max));
        assert(rdescCompare(res, {rdescMax}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).lt(min));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).lt(firstMid));
        assert(rdescCompare(res, {rdescMin}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).lt(secondMid));
        assert(rdescCompare(res, {rdescMin, rdescFirstMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).lt(max));
        assert(rdescCompare(res, {rdescMin, rdescFirstMid, rdescSecondMid}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).le(min));
        assert(rdescCompare(res, {rdescMin}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).le(firstMid));
        assert(rdescCompare(res, {rdescMin, rdescFirstMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).le(secondMid));
        assert(rdescCompare(res, {rdescMin, rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).le(max));
        assert(rdescCompare(res, {rdescMin, rdescFirstMid, rdescSecondMid, rdescMax}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).ge(min));
        assert(rdescCompare(res, {rdescMin, rdescFirstMid, rdescSecondMid, rdescMax}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).ge(firstMid));
        assert(rdescCompare(res, {rdescMax, rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).ge(secondMid));
        assert(rdescCompare(res, {rdescMax, rdescSecondMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).ge(max));
        assert(rdescCompare(res, {rdescMax}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).gt(min));
        assert(rdescCompare(res, {rdescMax, rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).gt(firstMid));
        assert(rdescCompare(res, {rdescMax, rdescSecondMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).gt(secondMid));
        assert(rdescCompare(res, {rdescMax}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).gt(max));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, max));
        assert(rdescCompare(res, {rdescMin, rdescMax, rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, max, {false, true}));
        assert(rdescCompare(res, {rdescMax, rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, max, {true, false}));
        assert(rdescCompare(res, {rdescMin, rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, max, {false, false}));
        assert(rdescCompare(res, {rdescFirstMid, rdescSecondMid}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, firstMid));
        assert(rdescCompare(res, {rdescMin, rdescFirstMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {false, true}));
        assert(rdescCompare(res, {rdescFirstMid}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {true, false}));
        assert(rdescCompare(res, {rdescMin}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {false, false}));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, secondMid));
        assert(rdescCompare(res, {rdescMin, rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {false, true}));
        assert(rdescCompare(res, {rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {true, false}));
        assert(rdescCompare(res, {rdescMin, rdescFirstMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {false, false}));
        assert(rdescCompare(res, {rdescFirstMid}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid));
        assert(rdescCompare(res, {rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {false, true}));
        assert(rdescCompare(res, {rdescSecondMid}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {true, false}));
        assert(rdescCompare(res, {rdescFirstMid}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {false, false}));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, max));
        assert(rdescCompare(res, {rdescFirstMid, rdescSecondMid, rdescMax}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {false, true}));
        assert(rdescCompare(res, {rdescSecondMid, rdescMax}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {true, false}));
        assert(rdescCompare(res, {rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {false, false}));
        assert(rdescCompare(res, {rdescSecondMid}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(secondMid, max));
        assert(rdescCompare(res, {rdescSecondMid, rdescMax}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {false, true}));
        assert(rdescCompare(res, {rdescMax}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {true, false}));
        assert(rdescCompare(res, {rdescSecondMid}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {false, false}));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

}

template<typename T>
void emptyIndexConditionTester(nogdb::Context *ctx, const std::string& className, const std::string& propertyName,
                          const nogdb::RecordDescriptor& rdescMin, const T& min,
                          const nogdb::RecordDescriptor& rdescFirstMid, const T& firstMid,
                          const nogdb::RecordDescriptor& rdescSecondMid, const T& secondMid,
                          const nogdb::RecordDescriptor& rdescMax, const T& max) {

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).eq(min));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).eq(firstMid));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).eq(secondMid));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).eq(max));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).lt(min));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).lt(firstMid));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).lt(secondMid));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).lt(max));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).le(min));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).le(firstMid));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).le(secondMid));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).le(max));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).ge(min));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).ge(firstMid));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).ge(secondMid));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).ge(max));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).gt(min));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).gt(firstMid));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).gt(secondMid));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).gt(max));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, max));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, max, {false, true}));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, max, {true, false}));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, max, {false, false}));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, firstMid));
        assert(rdescCompare(res, {}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {false, true}));
        assert(rdescCompare(res, {}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {true, false}));
        assert(rdescCompare(res, {}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {false, false}));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, secondMid));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {false, true}));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {true, false}));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {false, false}));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid));
        assert(rdescCompare(res, {}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {false, true}));
        assert(rdescCompare(res, {}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {true, false}));
        assert(rdescCompare(res, {}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {false, false}));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, max));
        assert(rdescCompare(res, {}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {false, true}));
        assert(rdescCompare(res, {}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {true, false}));
        assert(rdescCompare(res, {}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {false, false}));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(secondMid, max));
        assert(rdescCompare(res, {}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {false, true}));
        assert(rdescCompare(res, {}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {true, false}));
        assert(rdescCompare(res, {}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {false, false}));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

}

template<typename T>
void indexCursorConditionTester(nogdb::Context *ctx, const std::string& className, const std::string& propertyName,
                          const nogdb::RecordDescriptor& rdescMin, const T& min,
                          const nogdb::RecordDescriptor& rdescFirstMid, const T& firstMid,
                          const nogdb::RecordDescriptor& rdescSecondMid, const T& secondMid,
                          const nogdb::RecordDescriptor& rdescMax, const T& max) {

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).eq(min));
        assert(rdescCursorCompare(res, {rdescMin}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).eq(firstMid));
        assert(rdescCursorCompare(res, {rdescFirstMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).eq(secondMid));
        assert(rdescCursorCompare(res, {rdescSecondMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).eq(max));
        assert(rdescCursorCompare(res, {rdescMax}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).lt(min));
        assert(rdescCursorCompare(res, {}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).lt(firstMid));
        assert(rdescCursorCompare(res, {rdescMin}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).lt(secondMid));
        assert(rdescCursorCompare(res, {rdescMin, rdescFirstMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).lt(max));
        assert(rdescCursorCompare(res, {rdescMin, rdescFirstMid, rdescSecondMid}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).le(min));
        assert(rdescCursorCompare(res, {rdescMin}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).le(firstMid));
        assert(rdescCursorCompare(res, {rdescMin, rdescFirstMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).le(secondMid));
        assert(rdescCursorCompare(res, {rdescMin, rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).le(max));
        assert(rdescCursorCompare(res, {rdescMin, rdescFirstMid, rdescSecondMid, rdescMax}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).ge(min));
        assert(rdescCursorCompare(res, {rdescMin, rdescFirstMid, rdescSecondMid, rdescMax}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).ge(firstMid));
        assert(rdescCursorCompare(res, {rdescMax, rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).ge(secondMid));
        assert(rdescCursorCompare(res, {rdescMax, rdescSecondMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).ge(max));
        assert(rdescCursorCompare(res, {rdescMax}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).gt(min));
        assert(rdescCursorCompare(res, {rdescMax, rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).gt(firstMid));
        assert(rdescCursorCompare(res, {rdescMax, rdescSecondMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).gt(secondMid));
        assert(rdescCursorCompare(res, {rdescMax}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).gt(max));
        assert(rdescCursorCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, max));
        assert(rdescCursorCompare(res, {rdescMin, rdescMax, rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, max, {false, true}));
        assert(rdescCursorCompare(res, {rdescMax, rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, max, {true, false}));
        assert(rdescCursorCompare(res, {rdescMin, rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, max, {false, false}));
        assert(rdescCursorCompare(res, {rdescFirstMid, rdescSecondMid}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, firstMid));
        assert(rdescCursorCompare(res, {rdescMin, rdescFirstMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {false, true}));
        assert(rdescCursorCompare(res, {rdescFirstMid}));
        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {true, false}));
        assert(rdescCursorCompare(res, {rdescMin}));
        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {false, false}));
        assert(rdescCursorCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, secondMid));
        assert(rdescCursorCompare(res, {rdescMin, rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {false, true}));
        assert(rdescCursorCompare(res, {rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {true, false}));
        assert(rdescCursorCompare(res, {rdescMin, rdescFirstMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {false, false}));
        assert(rdescCursorCompare(res, {rdescFirstMid}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid));
        assert(rdescCursorCompare(res, {rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {false, true}));
        assert(rdescCursorCompare(res, {rdescSecondMid}));
        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {true, false}));
        assert(rdescCursorCompare(res, {rdescFirstMid}));
        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {false, false}));
        assert(rdescCursorCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, max));
        assert(rdescCursorCompare(res, {rdescFirstMid, rdescSecondMid, rdescMax}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {false, true}));
        assert(rdescCursorCompare(res, {rdescSecondMid, rdescMax}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {true, false}));
        assert(rdescCursorCompare(res, {rdescFirstMid, rdescSecondMid}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {false, false}));
        assert(rdescCursorCompare(res, {rdescSecondMid}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(secondMid, max));
        assert(rdescCursorCompare(res, {rdescSecondMid, rdescMax}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {false, true}));
        assert(rdescCursorCompare(res, {rdescMax}));
        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {true, false}));
        assert(rdescCursorCompare(res, {rdescSecondMid}));
        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {false, false}));
        assert(rdescCursorCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

}

template<typename T>
void nonUniqueIndexConditionTester(nogdb::Context *ctx, const std::string& className, const std::string& propertyName,
                          const nogdb::RecordDescriptor& rdescMin1, const nogdb::RecordDescriptor& rdescMin2, const T& min,
                          const nogdb::RecordDescriptor& rdescFirstMid1, const nogdb::RecordDescriptor& rdescFirstMid2, const T& firstMid,
                          const nogdb::RecordDescriptor& rdescSecondMid1, const nogdb::RecordDescriptor& rdescSecondMid2, const T& secondMid,
                          const nogdb::RecordDescriptor& rdescMax1, const nogdb::RecordDescriptor& rdescMax2, const T& max) {

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).eq(min));
        assert(rdescCompare(res, {rdescMin1, rdescMin2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).eq(firstMid));
        assert(rdescCompare(res, {rdescFirstMid1, rdescFirstMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).eq(secondMid));
        assert(rdescCompare(res, {rdescSecondMid1, rdescSecondMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).eq(max));
        assert(rdescCompare(res, {rdescMax1, rdescMax2}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).lt(min));
        assert(rdescCompare(res, {}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).lt(firstMid));
        assert(rdescCompare(res, {rdescMin1, rdescMin2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).lt(secondMid));
        assert(rdescCompare(res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).lt(max));
        assert(rdescCompare(res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).le(min));
        assert(rdescCompare(res, {rdescMin1, rdescMin2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).le(firstMid));
        assert(rdescCompare(res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).le(secondMid));
        assert(rdescCompare(res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).le(max));
        assert(rdescCompare(res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescMin2, rdescFirstMid2, rdescSecondMid2, rdescMax2}));

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).ge(min));
        assert(rdescCompare(res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescMin2, rdescFirstMid2, rdescSecondMid2, rdescMax2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).ge(firstMid));
        assert(rdescCompare(res, {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).ge(secondMid));
        assert(rdescCompare(res, {rdescMax1, rdescSecondMid1, rdescMax2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).ge(max));
        assert(rdescCompare(res, {rdescMax1, rdescMax2}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).gt(min));
        assert(rdescCompare(res, {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).gt(firstMid));
        assert(rdescCompare(res, {rdescMax1, rdescSecondMid1, rdescMax2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).gt(secondMid));
        assert(rdescCompare(res, {rdescMax1, rdescMax2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).gt(max));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, max));
        assert(rdescCompare(res, {rdescMin1, rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescMax2, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, max, {false, true}));
        assert(rdescCompare(res, {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, max, {true, false}));
        assert(rdescCompare(res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, max, {false, false}));
        assert(rdescCompare(res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, firstMid));
        assert(rdescCompare(res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {false, true}));
        assert(rdescCompare(res, {rdescFirstMid1, rdescFirstMid2}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {true, false}));
        assert(rdescCompare(res, {rdescMin1, rdescMin2}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {false, false}));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, secondMid));
        assert(rdescCompare(res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {false, true}));
        assert(rdescCompare(res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {true, false}));
        assert(rdescCompare(res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {false, false}));
        assert(rdescCompare(res, {rdescFirstMid1, rdescFirstMid2}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid));
        assert(rdescCompare(res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {false, true}));
        assert(rdescCompare(res, {rdescSecondMid1, rdescSecondMid2}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {true, false}));
        assert(rdescCompare(res, {rdescFirstMid1, rdescFirstMid2}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {false, false}));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, max));
        assert(rdescCompare(res, {rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescFirstMid2, rdescSecondMid2, rdescMax2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {false, true}));
        assert(rdescCompare(res, {rdescSecondMid1, rdescMax1, rdescSecondMid2, rdescMax2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {true, false}));
        assert(rdescCompare(res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {false, false}));
        assert(rdescCompare(res, {rdescSecondMid1, rdescSecondMid2}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(secondMid, max));
        assert(rdescCompare(res, {rdescSecondMid1, rdescMax1, rdescSecondMid2, rdescMax2}));

        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {false, true}));
        assert(rdescCompare(res, {rdescMax1, rdescMax2}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {true, false}));
        assert(rdescCompare(res, {rdescSecondMid1, rdescSecondMid2}));
        res = nogdb::Vertex::getIndex(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {false, false}));
        assert(rdescCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

}

template<typename T>
void nonUniqueIndexCursorConditionTester(nogdb::Context *ctx, const std::string& className, const std::string& propertyName,
                                   const nogdb::RecordDescriptor& rdescMin1, const nogdb::RecordDescriptor& rdescMin2, const T& min,
                                   const nogdb::RecordDescriptor& rdescFirstMid1, const nogdb::RecordDescriptor& rdescFirstMid2, const T& firstMid,
                                   const nogdb::RecordDescriptor& rdescSecondMid1, const nogdb::RecordDescriptor& rdescSecondMid2, const T& secondMid,
                                   const nogdb::RecordDescriptor& rdescMax1, const nogdb::RecordDescriptor& rdescMax2, const T& max) {

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).eq(min));
        assert(rdescCursorCompare(res, {rdescMin1, rdescMin2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).eq(firstMid));
        assert(rdescCursorCompare(res, {rdescFirstMid1, rdescFirstMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).eq(secondMid));
        assert(rdescCursorCompare(res, {rdescSecondMid1, rdescSecondMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).eq(max));
        assert(rdescCursorCompare(res, {rdescMax1, rdescMax2}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).lt(min));
        assert(rdescCursorCompare(res, {}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).lt(firstMid));
        assert(rdescCursorCompare(res, {rdescMin1, rdescMin2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).lt(secondMid));
        assert(rdescCursorCompare(res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).lt(max));
        assert(rdescCursorCompare(res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).le(min));
        assert(rdescCursorCompare(res, {rdescMin1, rdescMin2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).le(firstMid));
        assert(rdescCursorCompare(res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).le(secondMid));
        assert(rdescCursorCompare(res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).le(max));
        assert(rdescCursorCompare(res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescMin2, rdescFirstMid2, rdescSecondMid2, rdescMax2}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).ge(min));
        assert(rdescCursorCompare(res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescMin2, rdescFirstMid2, rdescSecondMid2, rdescMax2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).ge(firstMid));
        assert(rdescCursorCompare(res, {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).ge(secondMid));
        assert(rdescCursorCompare(res, {rdescMax1, rdescSecondMid1, rdescMax2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).ge(max));
        assert(rdescCursorCompare(res, {rdescMax1, rdescMax2}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).gt(min));
        assert(rdescCursorCompare(res, {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).gt(firstMid));
        assert(rdescCursorCompare(res, {rdescMax1, rdescSecondMid1, rdescMax2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).gt(secondMid));
        assert(rdescCursorCompare(res, {rdescMax1, rdescMax2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).gt(max));
        assert(rdescCursorCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, max));
        assert(rdescCursorCompare(res, {rdescMin1, rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescMax2, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, max, {false, true}));
        assert(rdescCursorCompare(res, {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, max, {true, false}));
        assert(rdescCursorCompare(res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, max, {false, false}));
        assert(rdescCursorCompare(res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));

    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, firstMid));
        assert(rdescCursorCompare(res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {false, true}));
        assert(rdescCursorCompare(res, {rdescFirstMid1, rdescFirstMid2}));
        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {true, false}));
        assert(rdescCursorCompare(res, {rdescMin1, rdescMin2}));
        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, firstMid, {false, false}));
        assert(rdescCursorCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, secondMid));
        assert(rdescCursorCompare(res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {false, true}));
        assert(rdescCursorCompare(res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {true, false}));
        assert(rdescCursorCompare(res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(min, secondMid, {false, false}));
        assert(rdescCursorCompare(res, {rdescFirstMid1, rdescFirstMid2}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid));
        assert(rdescCursorCompare(res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {false, true}));
        assert(rdescCursorCompare(res, {rdescSecondMid1, rdescSecondMid2}));
        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {true, false}));
        assert(rdescCursorCompare(res, {rdescFirstMid1, rdescFirstMid2}));
        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, secondMid, {false, false}));
        assert(rdescCursorCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, max));
        assert(rdescCursorCompare(res, {rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescFirstMid2, rdescSecondMid2, rdescMax2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {false, true}));
        assert(rdescCursorCompare(res, {rdescSecondMid1, rdescMax1, rdescSecondMid2, rdescMax2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {true, false}));
        assert(rdescCursorCompare(res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(firstMid, max, {false, false}));
        assert(rdescCursorCompare(res, {rdescSecondMid1, rdescSecondMid2}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_ONLY};
        auto res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(secondMid, max));
        assert(rdescCursorCompare(res, {rdescSecondMid1, rdescMax1, rdescSecondMid2, rdescMax2}));

        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {false, true}));
        assert(rdescCursorCompare(res, {rdescMax1, rdescMax2}));
        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {true, false}));
        assert(rdescCursorCompare(res, {rdescSecondMid1, rdescSecondMid2}));
        res = nogdb::Vertex::getIndexCursor(txn, className, nogdb::Condition(propertyName).between(secondMid, max, {false, false}));
        assert(rdescCursorCompare(res, {}));
    } catch (const nogdb::Error &ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

}

#endif
