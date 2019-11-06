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

#include "compare.hpp"
#include "datarecord_adapter.hpp"

#include "nogdb/nogdb.h"

namespace nogdb {
namespace datarecord {
    using namespace adapter::datarecord;
    using namespace adapter::schema;

    struct DataRecordUtils {

        static Record getRecord(const Transaction *txn,
            const ClassAccessInfo& classInfo,
            const RecordDescriptor& recordDescriptor);

        static Record getRecordWithBasicInfo(const Transaction *txn,
            const ClassAccessInfo& classInfo,
            const RecordDescriptor& recordDescriptor);

        static ResultSet getResultSet(const Transaction *txn,
            const ClassAccessInfo& classInfo,
            const std::vector<RecordDescriptor>& recordDescriptors);

        static ResultSet getResultSet(const Transaction *txn, const ClassAccessInfo& classInfo);

        static ResultSetCursor getResultSetCursor(const Transaction *txn, const ClassAccessInfo& classInfo);

        static size_t getCountRecord(const Transaction *txn, const ClassAccessInfo& classInfo);

        static ResultSet getResultSetByCondition(const Transaction *txn,
            const ClassAccessInfo& classInfo,
            const PropertyType& propertyType,
            const Condition& condition);

        static std::vector<RecordDescriptor> getRecordDescriptorByCondition(const Transaction *txn,
            const ClassAccessInfo& classInfo,
            const PropertyType& propertyType,
            const Condition& condition);

        static size_t getCountRecordByCondition(const Transaction *txn,
            const ClassAccessInfo& classInfo,
            const PropertyType& propertyType,
            const Condition& condition);

        static ResultSet getResultSetByMultiCondition(const Transaction *txn,
            const ClassAccessInfo& classInfo,
            const PropertyNameMapInfo& propertyInfos,
            const MultiCondition& multiCondition);

        static std::vector<RecordDescriptor> getRecordDescriptorByMultiCondition(const Transaction *txn,
            const ClassAccessInfo& classInfo,
            const PropertyNameMapInfo& propertyInfos,
            const MultiCondition& multiCondition);

        static size_t getCountRecordByMultiCondition(const Transaction *txn,
            const ClassAccessInfo& classInfo,
            const PropertyNameMapInfo& propertyInfos,
            const MultiCondition& multiCondition);


        static ResultSet getResultSetByCmpFunction(const Transaction *txn,
            const ClassAccessInfo& classInfo,
            bool (*condition)(const Record& record));

        static std::vector<RecordDescriptor> getRecordDescriptorByCmpFunction(const Transaction *txn,
            const ClassAccessInfo& classInfo,
            bool (*condition)(const Record& record));

        static size_t getCountRecordByCmpFunction(const Transaction *txn,
            const ClassAccessInfo& classInfo,
            bool (*condition)(const Record& record));

    };

}
}