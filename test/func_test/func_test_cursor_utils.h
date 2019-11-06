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
#include <functional>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>

#include "func_test_config.h"

#include "nogdb/nogdb.h"

inline void cursorContains(
    nogdb::ResultSetCursor& rsCursor, const std::set<std::string>& expectedResults, const std::string& testColumn)
{
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

inline void cursorTester(
    nogdb::ResultSetCursor& rsCursor, const std::vector<std::string>& expectedResults, const std::string& testColumn)
{
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
            assert(rsCursor->record.getText("@recordId") == nogdb::rid2str(rsCursor->descriptor.rid));
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
            assert(rsCursor->record.getText("@recordId") == nogdb::rid2str(rsCursor->descriptor.rid));
        }
    }
}

inline void cursorTester(
    nogdb::ResultSetCursor& rsCursor, const std::vector<unsigned int>& expectedResults, const std::string& testColumn)
{
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
            assert(rsCursor->record.getText("@recordId") == nogdb::rid2str(rsCursor->descriptor.rid));
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
            assert(rsCursor->record.getText("@recordId") == nogdb::rid2str(rsCursor->descriptor.rid));
        }
    }
}

inline bool rdescCursorCompare(const std::string& propertyName, nogdb::ResultSetCursor& res,
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
        for (res.next(); res.hasNext(); res.next()) {
            std::cout << res->descriptor.rid << " ";
        }
        std::cout << "]\x1B[0m\n";
    } else {
        auto index = 0;
        auto expectedResultSorted = expectedResult;
        std::sort(expectedResultSorted.begin(), expectedResultSorted.end(),
            [](const nogdb::RecordDescriptor& lhs, const nogdb::RecordDescriptor& rhs) { return lhs.rid < rhs.rid; });
        for (res.next(); res.hasNext(); res.next()) {
            auto cmp = res->descriptor.rid == expectedResultSorted[index].rid;
            compareRes &= cmp;
            if (!cmp) {
                std::cout << propertyName << "\n";
                std::cout << "\x1B[31m"
                          << "\n[error] Expect:\t" << expectedResultSorted[index].rid << "\n"
                          << "        Actual:\t" << res->descriptor.rid << ".\x1B[0m\n";
            }
            ++index;
        }
    }
    return compareRes;
}
