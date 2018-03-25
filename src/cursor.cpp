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

#include <iterator>

#include "generic.hpp"
#include "schema.hpp"

#include "nogdb_error.h"
#include "nogdb_types.h"

namespace nogdb {

    ResultSetCursor::ResultSetCursor(Txn &txn_)
            : txn{txn_}, currentIndex{-1} {
        classPropertyInfos = new ClassPropertyCache();
    }

    ResultSetCursor::~ResultSetCursor() noexcept {
        delete classPropertyInfos;
    }

    ResultSetCursor::ResultSetCursor(const ResultSetCursor &rc) : txn{rc.txn} {
        metadata = rc.metadata;
        classPropertyInfos = new ClassPropertyCache(*(rc.classPropertyInfos));
        currentIndex = rc.currentIndex;
    }

    ResultSetCursor &ResultSetCursor::operator=(const ResultSetCursor &rc) {
        if (this != &rc) {
            //TODO: add delete classPropertyInfos?
            txn = rc.txn;
            classPropertyInfos = new ClassPropertyCache(*(rc.classPropertyInfos));
            metadata = rc.metadata;
            currentIndex = rc.currentIndex;
        }
        return *this;
    }

    ResultSetCursor::ResultSetCursor(ResultSetCursor &&rc) noexcept: txn{rc.txn} {
        txn = rc.txn;
        metadata = std::move(rc.metadata);
        currentIndex = rc.currentIndex;
        classPropertyInfos = rc.classPropertyInfos;
        rc.classPropertyInfos = nullptr;
    }

    ResultSetCursor &ResultSetCursor::operator=(ResultSetCursor &&rc) noexcept {
        if (this != &rc) {
            //TODO: add delete classPropertyInfos?
            txn = rc.txn;
            metadata = std::move(rc.metadata);
            currentIndex = rc.currentIndex;
            classPropertyInfos = rc.classPropertyInfos;
            rc.classPropertyInfos = nullptr;
        }
        return *this;
    }

    bool ResultSetCursor::hasNext() const {
        return !(metadata.empty()) && (currentIndex < static_cast<long long>(metadata.size() - 1));
    }

    bool ResultSetCursor::hasPrevious() const {
        return !(metadata.empty()) && (currentIndex > 0);
    }

    bool ResultSetCursor::hasAt(unsigned long index) const {
        return !(metadata.empty()) && (index < metadata.size() - 1);
    }

    bool ResultSetCursor::next() {
        if (!metadata.empty() && (currentIndex == -1)) {
            currentIndex = 0;
        } else if (hasNext()) {
            ++currentIndex;
        } else {
            return false;
        }
        auto cursor = metadata.begin() + currentIndex;
        result = Generic::getRecordResult(txn, resolveClassPropertyInfo(cursor->rid.first), *(cursor));
        return true;
    }

    bool ResultSetCursor::previous() {
        if (!metadata.empty() && (currentIndex >= static_cast<long long>(metadata.size()))) {
            currentIndex = static_cast<long long>(metadata.size() - 1);
        } else if (hasPrevious()) {
            --currentIndex;
        } else {
            return false;
        }
        auto cursor = metadata.begin() + currentIndex;
        result = Generic::getRecordResult(txn, resolveClassPropertyInfo(cursor->rid.first), *(cursor));
        return true;
    }

    bool ResultSetCursor::empty() const {
        return metadata.empty();
    }

    size_t ResultSetCursor::size() const {
        return metadata.size();
    }

    size_t ResultSetCursor::count() const {
        return size();
    }

    void ResultSetCursor::first() {
        if (!metadata.empty()) {
            currentIndex = 0;
            auto cursor = metadata.begin();
            auto classPropertyInfo = resolveClassPropertyInfo(cursor->rid.first);
            result = Generic::getRecordResult(txn, classPropertyInfo, *(cursor));
        }
    }

    void ResultSetCursor::last() {
        if (!metadata.empty()) {
            currentIndex = static_cast<long long>(metadata.size() - 1);
            auto cursor = metadata.end() - 1;
            auto classPropertyInfo = resolveClassPropertyInfo(cursor->rid.first);
            result = Generic::getRecordResult(txn, classPropertyInfo, *(cursor));
        }
    }

    bool ResultSetCursor::to(unsigned long index) {
        if (index >= metadata.size()) {
            return false;
        }
        currentIndex = index;
        auto cursor = metadata.begin() + currentIndex;
        auto classPropertyInfo = resolveClassPropertyInfo(cursor->rid.first);
        result = Generic::getRecordResult(txn, classPropertyInfo, *(cursor));
        return true;
    }

    const Result &ResultSetCursor::operator*() const {
        return result;
    }

    const Result *ResultSetCursor::operator->() const {
        return &(operator*());
    }

    const ClassPropertyInfo ResultSetCursor::resolveClassPropertyInfo(ClassId classId) {
        auto classPropertyInfo = ClassPropertyInfo{};
        auto findCacheClassInfo = classPropertyInfos->find(classId);
        if (findCacheClassInfo == classPropertyInfos->cend()) {
            auto classDescriptor = Generic::getClassDescriptor(txn, classId, ClassType::UNDEFINED);
            auto classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
            classPropertyInfos->emplace(classId, classPropertyInfo);
            return classPropertyInfo;
        } else {
            return findCacheClassInfo->second;
        }
    }

}