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
namespace datarecord {
    using parser::RecordParser;
    using compare::RecordCompare;
    using namespace schema;

    Record DataRecordUtils::getRecord(const Transaction *txn,
        const ClassAccessInfo& classInfo,
        const RecordDescriptor& recordDescriptor)
    {
        auto propertyInfos = SchemaUtils::getPropertyIdMapInfo(txn, classInfo.id, classInfo.superClassId);
        auto result = DataRecord(txn->_txnBase, classInfo.id, classInfo.type).getResult(recordDescriptor.rid.second);
        return RecordParser::parseRawData(result, propertyInfos, classInfo.type, txn->_txnCtx->isVersionEnabled());
    }

    Record DataRecordUtils::getRecordWithBasicInfo(const Transaction *txn,
        const ClassAccessInfo& classInfo,
        const RecordDescriptor& recordDescriptor)
    {
        auto propertyInfos = SchemaUtils::getPropertyIdMapInfo(txn, classInfo.id, classInfo.superClassId);
        auto result = DataRecord(txn->_txnBase, classInfo.id, classInfo.type).getResult(recordDescriptor.rid.second);
        return RecordParser::parseRawDataWithBasicInfo(
            classInfo.name, recordDescriptor.rid, result, propertyInfos, classInfo.type,
            txn->_txnCtx->isVersionEnabled());
    }

    ResultSet DataRecordUtils::getResultSet(const Transaction *txn,
        const ClassAccessInfo& classInfo,
        const std::vector<RecordDescriptor>& recordDescriptors)
    {
        auto resultSet = ResultSet {};
        auto propertyInfos = SchemaUtils::getPropertyIdMapInfo(txn, classInfo.id, classInfo.superClassId);
        auto dataRecord = DataRecord(txn->_txnBase, classInfo.id, classInfo.type);
        for (const auto& recordDescriptor : recordDescriptors) {
            auto result = dataRecord.getResult(recordDescriptor.rid.second);
            auto record = RecordParser::parseRawDataWithBasicInfo(
                classInfo.name, recordDescriptor.rid, result, propertyInfos, classInfo.type,
                txn->_txnCtx->isVersionEnabled());
            resultSet.emplace_back(Result { recordDescriptor, record });
        }
        return resultSet;
    }

    ResultSet DataRecordUtils::getResultSet(const Transaction *txn, const ClassAccessInfo& classInfo)
    {
        auto dataRecord = DataRecord(txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = SchemaUtils::getPropertyIdMapInfo(txn, classInfo.id, classInfo.superClassId);
        auto resultSet = ResultSet {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto const record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, RecordId { classInfo.id, positionId },
                    result, propertyIdMapInfo, classInfo.type, txn->_txnCtx->isVersionEnabled());
                resultSet.emplace_back(Result { RecordDescriptor { classInfo.id, positionId }, record });
            };
        dataRecord.resultSetIter(callback);
        return resultSet;
    }

    ResultSetCursor DataRecordUtils::getResultSetCursor(const Transaction *txn, const ClassAccessInfo& classInfo)
    {
        auto vertexDataRecord = DataRecord(txn->_txnBase, classInfo.id, classInfo.type);
        auto resultSetCursor = ResultSetCursor { *txn };
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                resultSetCursor.metadata.emplace_back(RecordDescriptor { classInfo.id, positionId });
            };
        vertexDataRecord.resultSetIter(callback);
        return resultSetCursor;
    }

    size_t DataRecordUtils::getCountRecord(const Transaction *txn, const ClassAccessInfo& classInfo)
    {
        auto vertexDataRecord = DataRecord(txn->_txnBase, classInfo.id, classInfo.type);
        auto count = size_t {0};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                ++count;
            };
        vertexDataRecord.resultSetIter(callback);
        return count;
    }

    ResultSet DataRecordUtils::getResultSetByCondition(const Transaction *txn,
        const ClassAccessInfo& classInfo,
        const PropertyType& propertyType,
        const Condition& condition)
    {
        auto dataRecord = DataRecord(txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = SchemaUtils::getPropertyIdMapInfo(txn, classInfo.id, classInfo.superClassId);
        auto resultSet = ResultSet {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, txn->_txnCtx->isVersionEnabled());
                if (RecordCompare::compareRecordByCondition(record, propertyType, condition)) {
                    resultSet.emplace_back(Result { RecordDescriptor { rid }, record });
                }
            };
        dataRecord.resultSetIter(callback);
        return resultSet;
    }

    std::vector<RecordDescriptor> DataRecordUtils::getRecordDescriptorByCondition(const Transaction *txn,
        const ClassAccessInfo& classInfo,
        const PropertyType& propertyType,
        const Condition& condition)
    {
        auto dataRecord = DataRecord(txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = SchemaUtils::getPropertyIdMapInfo(txn, classInfo.id, classInfo.superClassId);
        auto recordDescriptors = std::vector<RecordDescriptor> {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, txn->_txnCtx->isVersionEnabled());
                if (RecordCompare::compareRecordByCondition(record, propertyType, condition)) {
                    recordDescriptors.emplace_back(RecordDescriptor { rid });
                }
            };
        dataRecord.resultSetIter(callback);
        return recordDescriptors;
    }

    size_t DataRecordUtils::getCountRecordByCondition(const Transaction *txn,
        const ClassAccessInfo& classInfo,
        const PropertyType& propertyType,
        const Condition& condition)
    {
        auto dataRecord = DataRecord(txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = SchemaUtils::getPropertyIdMapInfo(txn, classInfo.id, classInfo.superClassId);
        auto count = size_t {0};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, txn->_txnCtx->isVersionEnabled());
                if (RecordCompare::compareRecordByCondition(record, propertyType, condition)) {
                    ++count;
                }
            };
        dataRecord.resultSetIter(callback);
        return count;
    }

    ResultSet DataRecordUtils::getResultSetByMultiCondition(const Transaction *txn,
        const ClassAccessInfo& classInfo,
        const PropertyNameMapInfo& propertyInfos,
        const MultiCondition& multiCondition)
    {
        auto dataRecord = DataRecord(txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = SchemaUtils::getPropertyIdMapInfo(txn, classInfo.id, classInfo.superClassId);
        auto propertyTypes = PropertyMapType {};
        for (const auto& property : propertyInfos) {
            propertyTypes.emplace(property.first, property.second.type);
        }
        auto resultSet = ResultSet {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, txn->_txnCtx->isVersionEnabled());
                if (multiCondition.execute(record, propertyTypes)) {
                    resultSet.emplace_back(Result { RecordDescriptor { rid }, record });
                }
            };
        dataRecord.resultSetIter(callback);
        return resultSet;
    }

    std::vector<RecordDescriptor> DataRecordUtils::getRecordDescriptorByMultiCondition(const Transaction *txn,
        const ClassAccessInfo& classInfo,
        const PropertyNameMapInfo& propertyInfos,
        const MultiCondition& multiCondition)
    {
        auto dataRecord = DataRecord(txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = SchemaUtils::getPropertyIdMapInfo(txn, classInfo.id, classInfo.superClassId);
        auto propertyTypes = PropertyMapType {};
        for (const auto& property : propertyInfos) {
            propertyTypes.emplace(property.first, property.second.type);
        }
        auto recordDescriptors = std::vector<RecordDescriptor> {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, txn->_txnCtx->isVersionEnabled());
                if (multiCondition.execute(record, propertyTypes)) {
                    recordDescriptors.emplace_back(RecordDescriptor { rid });
                }
            };
        dataRecord.resultSetIter(callback);
        return recordDescriptors;
    }

    size_t DataRecordUtils::getCountRecordByMultiCondition(const Transaction *txn,
        const ClassAccessInfo& classInfo,
        const PropertyNameMapInfo& propertyInfos,
        const MultiCondition& multiCondition)
    {
        auto dataRecord = DataRecord(txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = SchemaUtils::getPropertyIdMapInfo(txn, classInfo.id, classInfo.superClassId);
        auto propertyTypes = PropertyMapType {};
        for (const auto& property : propertyInfos) {
            propertyTypes.emplace(property.first, property.second.type);
        }
        auto count = size_t {0};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, txn->_txnCtx->isVersionEnabled());
                if (multiCondition.execute(record, propertyTypes)) {
                    ++count;
                }
            };
        dataRecord.resultSetIter(callback);
        return count;
    }

    ResultSet DataRecordUtils::getResultSetByCmpFunction(const Transaction *txn,
        const ClassAccessInfo& classInfo,
        bool (*condition)(const Record& record))
    {
        auto dataRecord = DataRecord(txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = SchemaUtils::getPropertyIdMapInfo(txn, classInfo.id, classInfo.superClassId);
        auto resultSet = ResultSet {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, txn->_txnCtx->isVersionEnabled());
                if ((*condition)(record)) {
                    resultSet.emplace_back(Result { RecordDescriptor { rid }, record });
                }
            };
        dataRecord.resultSetIter(callback);
        return resultSet;
    }

    std::vector<RecordDescriptor> DataRecordUtils::getRecordDescriptorByCmpFunction(const Transaction *txn,
        const ClassAccessInfo& classInfo,
        bool (*condition)(const Record& record))
    {
        auto dataRecord = DataRecord(txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = SchemaUtils::getPropertyIdMapInfo(txn, classInfo.id, classInfo.superClassId);
        auto recordDescriptors = std::vector<RecordDescriptor> {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, txn->_txnCtx->isVersionEnabled());
                if ((*condition)(record)) {
                    recordDescriptors.emplace_back(RecordDescriptor { rid });
                }
            };
        dataRecord.resultSetIter(callback);
        return recordDescriptors;
    }

    size_t DataRecordUtils::getCountRecordByCmpFunction(const Transaction *txn,
        const ClassAccessInfo& classInfo,
        bool (*condition)(const Record& record))
    {
        auto dataRecord = DataRecord(txn->_txnBase, classInfo.id, classInfo.type);
        auto propertyIdMapInfo = SchemaUtils::getPropertyIdMapInfo(txn, classInfo.id, classInfo.superClassId);
        auto count = size_t {0};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto rid = RecordId { classInfo.id, positionId };
                auto record = RecordParser::parseRawDataWithBasicInfo(
                    classInfo.name, rid, result, propertyIdMapInfo, classInfo.type, txn->_txnCtx->isVersionEnabled());
                if ((*condition)(record)) {
                    ++count;
                }
            };
        dataRecord.resultSetIter(callback);
        return count;
    }

}
}