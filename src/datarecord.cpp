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

#include "datarecord.hpp"

namespace nogdb {

using adapter::schema::ClassAccessInfo;
using adapter::schema::PropertyNameMapInfo;
using adapter::datarecord::DataRecord;
using parser::RecordParser;
using compare::RecordCompare;

namespace datarecord {

    Record DataRecordInterface::getRecord(const ClassAccessInfo& classInfo,
        const RecordDescriptor& recordDescriptor) const
    {
        auto propertyInfos = _txn->_interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
        auto result = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type).getResult(recordDescriptor.rid.second);
        return RecordParser::parseRawData(result, propertyInfos, classInfo.type, _txn->_txnCtx->isEnableVersion());
    }

    Record DataRecordInterface::getRecordWithBasicInfo(const ClassAccessInfo& classInfo,
        const RecordDescriptor& recordDescriptor) const
    {
        auto propertyInfos = _txn->_interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
        auto result = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type).getResult(recordDescriptor.rid.second);
        return RecordParser::parseRawDataWithBasicInfo(
            classInfo.name, recordDescriptor.rid, result, propertyInfos, classInfo.type, _txn->_txnCtx->isEnableVersion());
    }

    ResultSet DataRecordInterface::getResultSet(const ClassAccessInfo& classInfo,
        const std::vector<RecordDescriptor>& recordDescriptors) const
    {
        auto resultSet = ResultSet {};
        auto propertyInfos = _txn->_interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
        auto dataRecord = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
        for (const auto& recordDescriptor : recordDescriptors) {
            auto result = dataRecord.getResult(recordDescriptor.rid.second);
            auto record = RecordParser::parseRawDataWithBasicInfo(
                classInfo.name, recordDescriptor.rid, result, propertyInfos, classInfo.type, _txn->_txnCtx->isEnableVersion());
            resultSet.emplace_back(Result { recordDescriptor, record });
        }
        return resultSet;
    }

    ResultSet DataRecordInterface::getResultSet(const ClassAccessInfo& classInfo) const
    {
        auto dataRecord = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = _txn->_interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
        auto resultSet = ResultSet {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto const record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, RecordId { classInfo.id, positionId },
                    result, propertyIdMapInfo, classInfo.type, _txn->_txnCtx->isEnableVersion());
                resultSet.emplace_back(Result { RecordDescriptor { classInfo.id, positionId }, record });
            };
        dataRecord.resultSetIter(callback);
        return resultSet;
    }

    ResultSetCursor DataRecordInterface::getResultSetCursor(const ClassAccessInfo& classInfo) const
    {
        auto vertexDataRecord = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
        auto resultSetCursor = ResultSetCursor { *_txn };
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                resultSetCursor.metadata.emplace_back(RecordDescriptor { classInfo.id, positionId });
            };
        vertexDataRecord.resultSetIter(callback);
        return resultSetCursor;
    }

    size_t DataRecordInterface::getCountRecord(const ClassAccessInfo& classInfo) const
    {
        auto vertexDataRecord = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
        auto count = size_t {0};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                ++count;
            };
        vertexDataRecord.resultSetIter(callback);
        return count;
    }

    ResultSet
    DataRecordInterface::getResultSetByCondition(const ClassAccessInfo& classInfo,
        const PropertyType& propertyType,
        const Condition& condition) const
    {
        auto dataRecord = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = _txn->_interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
        auto resultSet = ResultSet {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, _txn->_txnCtx->isEnableVersion());
                if (RecordCompare::compareRecordByCondition(record, propertyType, condition)) {
                    resultSet.emplace_back(Result { RecordDescriptor { rid }, record });
                }
            };
        dataRecord.resultSetIter(callback);
        return resultSet;
    }

    std::vector<RecordDescriptor>
    DataRecordInterface::getRecordDescriptorByCondition(const ClassAccessInfo& classInfo,
        const PropertyType& propertyType,
        const Condition& condition) const
    {
        auto dataRecord = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = _txn->_interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
        auto recordDescriptors = std::vector<RecordDescriptor> {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, _txn->_txnCtx->isEnableVersion());
                if (RecordCompare::compareRecordByCondition(record, propertyType, condition)) {
                    recordDescriptors.emplace_back(RecordDescriptor { rid });
                }
            };
        dataRecord.resultSetIter(callback);
        return recordDescriptors;
    }

    size_t
    DataRecordInterface::getCountRecordByCondition(const ClassAccessInfo& classInfo,
        const PropertyType& propertyType,
        const Condition& condition) const
    {
        auto dataRecord = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = _txn->_interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
        auto count = size_t {0};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, _txn->_txnCtx->isEnableVersion());
                if (RecordCompare::compareRecordByCondition(record, propertyType, condition)) {
                    ++count;
                }
            };
        dataRecord.resultSetIter(callback);
        return count;
    }

    ResultSet
    DataRecordInterface::getResultSetByMultiCondition(const ClassAccessInfo& classInfo,
        const PropertyNameMapInfo& propertyInfos,
        const MultiCondition& multiCondition) const
    {
        auto dataRecord = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = _txn->_interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
        auto propertyTypes = PropertyMapType {};
        for (const auto& property : propertyInfos) {
            propertyTypes.emplace(property.first, property.second.type);
        }
        auto resultSet = ResultSet {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, _txn->_txnCtx->isEnableVersion());
                if (multiCondition.execute(record, propertyTypes)) {
                    resultSet.emplace_back(Result { RecordDescriptor { rid }, record });
                }
            };
        dataRecord.resultSetIter(callback);
        return resultSet;
    }

    std::vector<RecordDescriptor>
    DataRecordInterface::getRecordDescriptorByMultiCondition(const ClassAccessInfo& classInfo,
        const PropertyNameMapInfo& propertyInfos,
        const MultiCondition& multiCondition) const
    {
        auto dataRecord = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = _txn->_interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
        auto propertyTypes = PropertyMapType {};
        for (const auto& property : propertyInfos) {
            propertyTypes.emplace(property.first, property.second.type);
        }
        auto recordDescriptors = std::vector<RecordDescriptor> {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, _txn->_txnCtx->isEnableVersion());
                if (multiCondition.execute(record, propertyTypes)) {
                    recordDescriptors.emplace_back(RecordDescriptor { rid });
                }
            };
        dataRecord.resultSetIter(callback);
        return recordDescriptors;
    }

    size_t
    DataRecordInterface::getCountRecordByMultiCondition(const ClassAccessInfo& classInfo,
        const PropertyNameMapInfo& propertyInfos,
        const MultiCondition& multiCondition) const
    {
        auto dataRecord = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = _txn->_interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
        auto propertyTypes = PropertyMapType {};
        for (const auto& property : propertyInfos) {
            propertyTypes.emplace(property.first, property.second.type);
        }
        auto count = size_t {0};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, _txn->_txnCtx->isEnableVersion());
                if (multiCondition.execute(record, propertyTypes)) {
                    ++count;
                }
            };
        dataRecord.resultSetIter(callback);
        return count;
    }

    ResultSet
    DataRecordInterface::getResultSetByCmpFunction(const ClassAccessInfo& classInfo,
        bool (*condition)(const Record& record)) const
    {
        auto dataRecord = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = _txn->_interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
        auto resultSet = ResultSet {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, _txn->_txnCtx->isEnableVersion());
                if ((*condition)(record)) {
                    resultSet.emplace_back(Result { RecordDescriptor { rid }, record });
                }
            };
        dataRecord.resultSetIter(callback);
        return resultSet;
    }

    std::vector<RecordDescriptor>
    DataRecordInterface::getRecordDescriptorByCmpFunction(const ClassAccessInfo& classInfo,
        bool (*condition)(const Record& record)) const
    {
        auto dataRecord = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = _txn->_interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
        auto recordDescriptors = std::vector<RecordDescriptor> {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, _txn->_txnCtx->isEnableVersion());
                if ((*condition)(record)) {
                    recordDescriptors.emplace_back(RecordDescriptor { rid });
                }
            };
        dataRecord.resultSetIter(callback);
        return recordDescriptors;
    }

    size_t
    DataRecordInterface::getCountRecordByCmpFunction(const ClassAccessInfo& classInfo,
        bool (*condition)(const Record& record)) const
    {
        auto dataRecord = DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = _txn->_interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
        auto count = size_t {0};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, _txn->_txnCtx->isEnableVersion());
                if ((*condition)(record)) {
                    ++count;
                }
            };
        dataRecord.resultSetIter(callback);
        return count;
    }

}

}