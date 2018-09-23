/*
 *  Copyright (C) 2018, Throughwave (Thailand) Co., Ltd.
 *  <peerawich at throughwave dot co dot th>
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

#ifndef __DATARECORD_HPP_INCLUDED_
#define __DATARECORD_HPP_INCLUDED_

#include "datarecord_adapter.hpp"
#include "compare.hpp"

#include "nogdb/nogdb_compare.h"
#include "nogdb/nogdb_txn.h"

namespace nogdb {

    using adapter::datarecord::DataRecord;

    namespace datarecord {

        class DataRecordInterface {
        public:
            DataRecordInterface(const Txn* txn) : _txn{txn} {}

            virtual ~DataRecordInterface() noexcept = default;

            Record getRecord(const schema::ClassAccessInfo& classInfo, const RecordDescriptor& recordDescriptor) const {
                auto propertyInfos = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
                auto result = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type)
                        .getResult(recordDescriptor.rid.second);
                return parser::Parser::parseRawDataWithBasicInfo(classInfo.name, recordDescriptor.rid, result, propertyInfos, classInfo.type);
            }

            ResultSet getResultSet(const schema::ClassAccessInfo& classInfo, const std::vector<RecordDescriptor>& recordDescriptors) const {
                auto resultSet = ResultSet{};
                auto propertyInfos = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
                auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
                for(const auto& recordDescriptor: recordDescriptors) {
                    auto result = dataRecord.getResult(recordDescriptor.rid.second);
                    auto record = parser::Parser::parseRawDataWithBasicInfo(classInfo.name, recordDescriptor.rid, result, propertyInfos, classInfo.type);
                    resultSet.emplace_back(Result{recordDescriptor, record});
                }
                return resultSet;
            }

            ResultSet getResultSet(const schema::ClassAccessInfo &classInfo) const {
                auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
                auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
                auto resultSet = ResultSet{};
                std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
                    [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                        auto const record = parser::Parser::parseRawDataWithBasicInfo(
                                classInfo.name,
                                RecordId{classInfo.id, positionId},
                                result, propertyIdMapInfo, classInfo.type);
                        resultSet.emplace_back(Result{RecordDescriptor{classInfo.id, positionId}, record});
                    };
                dataRecord.resultSetIter(callback);
                return resultSet;
            }

            ResultSetCursor getResultSetCursor(const schema::ClassAccessInfo &classInfo) const {
                auto vertexDataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
                auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
                auto resultSetCursor = ResultSetCursor{*_txn};
                std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
                    [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                        resultSetCursor.metadata.emplace_back(RecordDescriptor{classInfo.id, positionId});
                    };
                vertexDataRecord.resultSetIter(callback);
                return resultSetCursor;
            }

            ResultSet getResultSetByCondition(const schema::ClassAccessInfo &classInfo,
                                              const PropertyType& propertyType,
                                              const Condition& condition) const {
                auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
                auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
                auto resultSet = ResultSet{};
                std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
                    [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                        auto rid = RecordId{classInfo.id, positionId};
                        auto record = parser::Parser::parseRawDataWithBasicInfo(classInfo.name, rid, result, propertyIdMapInfo, classInfo.type);
                        switch (condition.comp) {
                            case Condition::Comparator::IS_NULL:
                                if (record.get(condition.propName).empty()) {
                                    resultSet.emplace_back(Result{RecordDescriptor{rid}, record});
                                }
                                break;
                            case Condition::Comparator::NOT_NULL:
                                if (!record.get(condition.propName).empty()) {
                                    resultSet.emplace_back(Result{RecordDescriptor{rid}, record});
                                }
                                break;
                            default:
                                if (!record.get(condition.propName).empty()) {
                                    if (Compare::compareBytesValue(record.get(condition.propName), propertyType, condition)) {
                                        resultSet.emplace_back(Result{RecordDescriptor{rid}, record});
                                    }
                                }
                                break;
                        }
                    };
                dataRecord.resultSetIter(callback);
                return resultSet;
            }

            std::vector<RecordDescriptor>
            getRecordDescriptorByCondition(const schema::ClassAccessInfo &classInfo,
                                           const PropertyType &propertyType,
                                           const Condition &condition) const {
                auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
                auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
                auto recordDescriptors = std::vector<RecordDescriptor>{};
                std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
                    [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                        auto rid = RecordId{classInfo.id, positionId};
                        auto record = parser::Parser::parseRawDataWithBasicInfo(classInfo.name, rid, result, propertyIdMapInfo, classInfo.type);
                        switch (condition.comp) {
                            case Condition::Comparator::IS_NULL:
                                if (record.get(condition.propName).empty()) {
                                    recordDescriptors.emplace_back(RecordDescriptor{rid});
                                }
                                break;
                            case Condition::Comparator::NOT_NULL:
                                if (!record.get(condition.propName).empty()) {
                                    recordDescriptors.emplace_back(RecordDescriptor{rid});
                                }
                                break;
                            default:
                                if (!record.get(condition.propName).empty()) {
                                    if (Compare::compareBytesValue(record.get(condition.propName), propertyType,
                                                                   condition)) {
                                        recordDescriptors.emplace_back(RecordDescriptor{rid});
                                    }
                                }
                                break;
                        }
                    };
                dataRecord.resultSetIter(callback);
                return recordDescriptors;
            }

            ResultSet getResultSetByMultiCondition(const schema::ClassAccessInfo &classInfo,
                                                   const schema::PropertyNameMapInfo& propertyInfos,
                                                   const MultiCondition& multiCondition) const {
                auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
                auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
                auto propertyTypes = PropertyMapType{};
                for(const auto& property: propertyInfos) {
                    propertyTypes.emplace(property.first, property.second.type);
                }
                auto resultSet = ResultSet{};
                std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
                    [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                        auto rid = RecordId{classInfo.id, positionId};
                        auto record = parser::Parser::parseRawDataWithBasicInfo(classInfo.name, rid, result, propertyIdMapInfo, classInfo.type);
                        if (multiCondition.execute(record, propertyTypes)) {
                            resultSet.emplace_back(Result{RecordDescriptor{rid}, record});
                        }
                    };
                dataRecord.resultSetIter(callback);
                return resultSet;
            }

            std::vector<RecordDescriptor>
            getRecordDescriptorByMultiCondition(const schema::ClassAccessInfo &classInfo,
                                                const schema::PropertyNameMapInfo &propertyInfos,
                                                const MultiCondition &multiCondition) const {
                auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
                auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
                auto propertyTypes = PropertyMapType{};
                for(const auto& property: propertyInfos) {
                    propertyTypes.emplace(property.first, property.second.type);
                }
                auto recordDescriptors = std::vector<RecordDescriptor>{};
                std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
                    [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                        auto rid = RecordId{classInfo.id, positionId};
                        auto record = parser::Parser::parseRawDataWithBasicInfo(classInfo.name, rid, result, propertyIdMapInfo, classInfo.type);
                        if (multiCondition.execute(record, propertyTypes)) {
                            recordDescriptors.emplace_back(RecordDescriptor{rid});
                        }
                    };
                dataRecord.resultSetIter(callback);
                return recordDescriptors;
            }

            ResultSet getResultSetByCmpFunction(const schema::ClassAccessInfo &classInfo,
                                                bool (*condition)(const Record &record)) const {
                auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
                auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
                auto resultSet = ResultSet{};
                std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
                    [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                        auto rid = RecordId{classInfo.id, positionId};
                        auto record = parser::Parser::parseRawDataWithBasicInfo(classInfo.name, rid, result, propertyIdMapInfo, classInfo.type);
                        if ((*condition)(record)) {
                            resultSet.emplace_back(Result{RecordDescriptor{rid}, record});
                        }
                    };
                dataRecord.resultSetIter(callback);
                return resultSet;
            }

            std::vector<RecordDescriptor>
            getRecordDescriptorByCmpFunction(const schema::ClassAccessInfo &classInfo,
                                             bool (*condition)(const Record &record)) const {
                auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
                auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
                auto recordDescriptors = std::vector<RecordDescriptor>{};
                std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
                    [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                        auto rid = RecordId{classInfo.id, positionId};
                        auto record = parser::Parser::parseRawDataWithBasicInfo(classInfo.name, rid, result, propertyIdMapInfo, classInfo.type);
                        if ((*condition)(record)) {
                            recordDescriptors.emplace_back(RecordDescriptor{rid});
                        }
                    };
                dataRecord.resultSetIter(callback);
                return resultSet;
            }

        private:
            const Txn* _txn;

        };

    }

}

#endif