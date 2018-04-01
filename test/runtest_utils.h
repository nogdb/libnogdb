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

#include <string>
#include "nogdb.h"

inline void init() {
    const std::string clear_dir_command = "rm -rf " + DATABASE_PATH;
    system(clear_dir_command.c_str());
}

inline void show_schema(const nogdb::Txn &txn) {
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
        }
    }
}

#endif
