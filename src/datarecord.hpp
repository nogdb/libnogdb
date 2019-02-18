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

using adapter::datarecord::DataRecord;

namespace datarecord {

    class DataRecordInterface {
    public:
        DataRecordInterface(const Transaction* txn)
            : _txn { txn }
        {
        }

        virtual ~DataRecordInterface() noexcept = default;

        Record getRecord(const schema::ClassAccessInfo& classInfo, const RecordDescriptor& recordDescriptor) const;

        Record getRecordWithBasicInfo(const schema::ClassAccessInfo& classInfo,
            const RecordDescriptor& recordDescriptor) const;

        ResultSet getResultSet(const schema::ClassAccessInfo& classInfo,
            const std::vector<RecordDescriptor>& recordDescriptors) const;

        ResultSet getResultSet(const schema::ClassAccessInfo& classInfo) const;

        ResultSetCursor getResultSetCursor(const schema::ClassAccessInfo& classInfo) const;

        ResultSet getResultSetByCondition(const schema::ClassAccessInfo& classInfo,
            const PropertyType& propertyType,
            const Condition& condition) const;

        std::vector<RecordDescriptor>
        getRecordDescriptorByCondition(const schema::ClassAccessInfo& classInfo,
            const PropertyType& propertyType,
            const Condition& condition) const;

        ResultSet getResultSetByMultiCondition(const schema::ClassAccessInfo& classInfo,
            const schema::PropertyNameMapInfo& propertyInfos,
            const MultiCondition& multiCondition) const;

        std::vector<RecordDescriptor>
        getRecordDescriptorByMultiCondition(const schema::ClassAccessInfo& classInfo,
            const schema::PropertyNameMapInfo& propertyInfos,
            const MultiCondition& multiCondition) const;

        ResultSet getResultSetByCmpFunction(const schema::ClassAccessInfo& classInfo,
            bool (*condition)(const Record& record)) const;

        std::vector<RecordDescriptor>
        getRecordDescriptorByCmpFunction(const schema::ClassAccessInfo& classInfo,
            bool (*condition)(const Record& record)) const;

    private:
        const Transaction* _txn;
    };

}

}