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

#ifndef __DATARECORD_ADAPTER_HPP_INCLUDED_
#define __DATARECORD_ADAPTER_HPP_INCLUDED_

#include <nogdb_txn.h>
#include "storage_adapter.hpp"
#include "schema.hpp"
#include "schema_adapter.hpp"
#include "parser.hpp"

namespace nogdb {

    namespace adapter {

        namespace datarecord {

            class DataRecord : public storage_engine::adapter::LMDBKeyValAccess {
            public:
                DataRecord(const storage_engine::LMDBTxn *const txn,
                           const ClassId &classId,
                           const ClassType &classType = ClassType::UNDEFINED)
                          : _classId{classId},
                            _classType{classType},
                            LMDBKeyValAccess(txn, std::to_string(classId), true, true, true, true) {}

                virtual ~DataRecord() noexcept = default;

                DataRecord(DataRecord&& other) noexcept {
                    *this = std::move(other);
                }

                DataRecord& operator=(DataRecord&& other) noexcept {
                    if (this != &other) {
                        using std::swap;
                        swap(*this, other);
                    }
                    return *this;
                }

                void init() {
                    put(MAX_RECORD_NUM_EM, PositionId{1});
                }

                PositionId insert(const Blob &blob) {
                    auto result = get(MAX_RECORD_NUM_EM);
                    require(!result.empty);
                    auto posid = result.data.numeric<PositionId>();
                    put(posid, blob);
                    put(MAX_RECORD_NUM_EM, posid + PositionId{1});
                    return posid;
                }

                void update(const PositionId &posid, const Blob &blob) {
                    auto result = get(posid);
                    if (!result.empty) {
                        put(posid, blob);
                    } else {
                        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_RECORD);
                    }
                }

                void remove(const PositionId &posid) {
                    auto result = get(posid);
                    if (!result.empty) {
                        del(posid);
                    } else {
                        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_RECORD);
                    }
                }

                void destroy() {
                    drop(true);
                }

                Blob getBlob(const PositionId &posid) {
                    auto result = get(posid);
                    if (!result.empty) {
                        return result.data.blob();
                    } else {
                        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_RECORD);
                    }
                }

                storage_engine::lmdb::Result getResult(const PositionId &posid) {
                    auto result = get(posid);
                    if (!result.empty) {
                        return result;
                    } else {
                        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_RECORD);
                    }
                }

                storage_engine::lmdb::Cursor getCursor() const {
                    return cursor();
                }

                void resultSetIter(std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback) {
                    auto cursorHandler = getCursor();
                    for (auto keyValue = cursorHandler.getNext();
                         !keyValue.empty();
                         keyValue = cursorHandler.getNext()) {
                        auto key = keyValue.key.data.numeric<PositionId>();
                        if (key == MAX_RECORD_NUM_EM) continue;
                        callback(key, keyValue.val);
                    }
                }

            private:
                ClassId _classId{};
                ClassType _classType{ClassType::UNDEFINED};
            };

            class DataRecords {
            public:
                DataRecords(const Txn* txn, const schema::ClassAccessInfo& classInfo)
                        : _txn{txn}, _dataRecordInfos{}, _classInfo{classInfo} {
                    _dataRecordInfos.emplace(classInfo.name, {classInfo, DataRecord(txn->_txnBase, classInfo.id, classInfo.type)});
                    auto subClassInfos = std::map<std::string, schema::ClassAccessInfo>{};
                    for(const auto& subClassInfo: txn->_iSchema->getSubClassInfos(classInfo.id, subClassInfos)) {
                        _dataRecordInfos.emplace(subClassInfo.second.name, {subClassInfo.first, DataRecord(txn->_txnBase, subClassInfo.id, subClassInfo.type)});
                    }
                }

                virtual ~DataRecords() noexcept = default;

                ResultSet get() const {
                    auto result = ResultSet{};
                    for(const auto& dataRecordInfo: _dataRecordInfos) {
                        auto &classInfo = dataRecordInfo.second.first;
                        auto &dataRecord = dataRecordInfo.second.second;
                        auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
                        auto cursorHandler = dataRecord.getCursor();
                        for(auto keyValue = cursorHandler.getNext();
                            !keyValue.empty();
                            keyValue = cursorHandler.getNext()) {
                            auto key = keyValue.key.data.numeric<PositionId>();
                            if (key == MAX_RECORD_NUM_EM) continue;
                            auto recordId = RecordId{classInfo.id, key};
                            auto record = parser::Parser::parseRawDataWithBasicInfo(
                                    classInfo.name, recordId, keyValue.val,
                                    propertyIdMapInfo, classInfo.type == ClassType::EDGE);
                            result.emplace_back(ResultSet{RecordDescriptor{_classInfo.id, key}, record});
                        }
                    }
                    return result;
                }

                ResultSetCursor getCursor() const {
                    auto result = ResultSetCursor{*_txn};
                    for(const auto& dataRecordInfo: _dataRecordInfos) {
                        auto &classInfo = dataRecordInfo.second.first;
                        auto &dataRecord = dataRecordInfo.second.second;
                        auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
                        auto cursorHandler = dataRecord.getCursor();
                        for(auto keyValue = cursorHandler.getNext();
                            !keyValue.empty();
                            keyValue = cursorHandler.getNext()) {
                            auto key = keyValue.key.data.numeric<PositionId>();
                            if (key == MAX_RECORD_NUM_EM) continue;
                            result.metadata.emplace_back(RecordDescriptor{_classInfo.id, key});
                        }
                    }
                    return result;
                }

            private:
                const Txn* _txn;
                std::map<std::string, std::pair<schema::ClassAccessInfo, DataRecord>> _dataRecordInfos{};
                schema::ClassAccessInfo _classInfo{};
            };

        }
    }
}

#endif //__DATARECORD_ADAPTER_HPP_INCLUDED_
