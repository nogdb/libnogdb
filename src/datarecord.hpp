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

#include "nogdb/nogdb_txn.h"

namespace nogdb {

    namespace datarecord {

        using adapter::datarecord::DataRecord;

        class DataRecordInterface {
        public:
            DataRecordInterface(const Txn* txn)
                : _txn{txn} {}

            virtual ~DataRecordInterface() noexcept = default;

            Record getRecord(const schema::ClassAccessInfo& classInfo, const RecordDescriptor& recordDescriptor) const {
                auto result = adapter::datarecord::DataRecord(_txn->_txnBase, recordDescriptor.rid.first, classInfo.type)
                        .getResult(recordDescriptor.rid.second);
                auto propertyInfos = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
                return parser::Parser::parseRawDataWithBasicInfo(classInfo.name, recordDescriptor.rid, result, propertyInfos, classInfo.type);
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

            ResultSet getResultSetExtend(const schema::ClassAccessInfo &classInfo) const {
                auto dataRecordInfos = prepareClassInfosExtend(classInfo);
                auto result = ResultSet{};
                for(const auto& dataRecordInfo: dataRecordInfos) {
                    auto &currentClassInfo = dataRecordInfo.second.first;
                    auto &dataRecord = dataRecordInfo.second.second;
                    auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(currentClassInfo.id, currentClassInfo.superClassId);
                    auto cursorHandler = dataRecord.getCursor();
                    for(auto keyValue = cursorHandler.getNext();
                        !keyValue.empty();
                        keyValue = cursorHandler.getNext()) {
                        auto key = keyValue.key.data.numeric<PositionId>();
                        if (key == MAX_RECORD_NUM_EM) continue;
                        auto recordId = RecordId{currentClassInfo.id, key};
                        auto record = parser::Parser::parseRawDataWithBasicInfo(
                                currentClassInfo.name, recordId, keyValue.val,
                                propertyIdMapInfo, currentClassInfo.type);
                        result.emplace_back(ResultSet{RecordDescriptor{currentClassInfo.id, key}, record});
                    }
                }
                return result;
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

            ResultSetCursor getResultSetCursorExtend(const schema::ClassAccessInfo &classInfo) const {
                auto dataRecordInfos = prepareClassInfosExtend(classInfo);
                auto result = ResultSetCursor{*_txn};
                for(const auto& dataRecordInfo: _dataRecordInfos) {
                    auto &currentClassInfo = dataRecordInfo.second.first;
                    auto &dataRecord = dataRecordInfo.second.second;
                    auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(currentClassInfo.id, currentClassInfo.superClassId);
                    auto cursorHandler = dataRecord.getCursor();
                    for(auto keyValue = cursorHandler.getNext();
                        !keyValue.empty();
                        keyValue = cursorHandler.getNext()) {
                        auto key = keyValue.key.data.numeric<PositionId>();
                        if (key == MAX_RECORD_NUM_EM) continue;
                        result.metadata.emplace_back(RecordDescriptor{currentClassInfo.id, key});
                    }
                }
                return result;
            }

        private:

            const Txn* _txn;
            std::map<std::string, std::pair<schema::ClassAccessInfo, DataRecord>> _dataRecordInfos{};

            std::map<std::string, std::pair<schema::ClassAccessInfo, DataRecord>>
            prepareClassInfosExtend(const schema::ClassAccessInfo& classInfo) const {
                auto dataRecordInfos = std::map<std::string, std::pair<schema::ClassAccessInfo, DataRecord>>{};
                dataRecordInfos.emplace(
                        classInfo.name,
                        std::make_pair(classInfo, DataRecord(_txn->_txnBase, classInfo.id, classInfo.type))
                );
                auto subClassInfos = std::map<std::string, schema::ClassAccessInfo>{};
                for(const auto& subClassInfo: _txn->_iSchema->getSubClassInfos(classInfo.id, subClassInfos)) {
                    dataRecordInfos.emplace(
                            subClassInfo.second.name,
                            std::make_pair(subClassInfo.first, DataRecord(_txn->_txnBase, subClassInfo.second.id, subClassInfo.second.type))
                    );
                }
                return dataRecordInfos;
            }

        };

    }

}

#endif