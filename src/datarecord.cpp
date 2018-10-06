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

#include "datarecord.hpp"

namespace nogdb {

  namespace datarecord {

    Record DataRecordInterface::getRecord(const schema::ClassAccessInfo &classInfo,
                                          const RecordDescriptor &recordDescriptor) const {
      auto propertyInfos = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
      auto result = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type)
          .getResult(recordDescriptor.rid.second);
      return parser::RecordParser::parseRawDataWithBasicInfo(
          classInfo.name, recordDescriptor.rid, result, propertyInfos, classInfo.type);
    }

    ResultSet DataRecordInterface::getResultSet(const schema::ClassAccessInfo &classInfo,
                                                const std::vector<RecordDescriptor> &recordDescriptors) const {
      auto resultSet = ResultSet{};
      auto propertyInfos = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
      auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
      for (const auto &recordDescriptor: recordDescriptors) {
        auto result = dataRecord.getResult(recordDescriptor.rid.second);
        auto record = parser::RecordParser::parseRawDataWithBasicInfo(
            classInfo.name, recordDescriptor.rid, result, propertyInfos, classInfo.type);
        resultSet.emplace_back(Result{recordDescriptor, record});
      }
      return resultSet;
    }

    ResultSet DataRecordInterface::getResultSet(const schema::ClassAccessInfo &classInfo) const {
      auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
      auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = ResultSet{};
      std::function<void(const PositionId &, const storage_engine::lmdb::Result &)> callback =
          [&](const PositionId &positionId, const storage_engine::lmdb::Result &result) {
            auto const record = parser::RecordParser::parseRawDataWithBasicInfo(
                classInfo.name, RecordId{classInfo.id, positionId}, result, propertyIdMapInfo, classInfo.type);
            resultSet.emplace_back(Result{RecordDescriptor{classInfo.id, positionId}, record});
          };
      dataRecord.resultSetIter(callback);
      return resultSet;
    }

    ResultSetCursor DataRecordInterface::getResultSetCursor(const schema::ClassAccessInfo &classInfo) const {
      auto vertexDataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
      auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSetCursor = ResultSetCursor{*_txn};
      std::function<void(const PositionId &, const storage_engine::lmdb::Result &)> callback =
          [&](const PositionId &positionId, const storage_engine::lmdb::Result &result) {
            resultSetCursor.metadata.emplace_back(RecordDescriptor{classInfo.id, positionId});
          };
      vertexDataRecord.resultSetIter(callback);
      return resultSetCursor;
    }

    ResultSet
    DataRecordInterface::getResultSetByCondition(const schema::ClassAccessInfo &classInfo,
                                                 const PropertyType &propertyType,
                                                 const Condition &condition) const {
      auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
      auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = ResultSet{};
      std::function<void(const PositionId &, const storage_engine::lmdb::Result &)> callback =
          [&](const PositionId &positionId, const storage_engine::lmdb::Result &result) {
            auto rid = RecordId{classInfo.id, positionId};
            auto record = parser::RecordParser::parseRawDataWithBasicInfo(classInfo.name, rid, result,
                                                                          propertyIdMapInfo,
                                                                          classInfo.type);
            if (compare::RecordCompare::compareRecordByCondition(record, propertyType, condition)) {
              resultSet.emplace_back(Result{RecordDescriptor{rid}, record});
            }
          };
      dataRecord.resultSetIter(callback);
      return resultSet;
    }

    std::vector<RecordDescriptor>
    DataRecordInterface::getRecordDescriptorByCondition(const schema::ClassAccessInfo &classInfo,
                                                        const PropertyType &propertyType,
                                                        const Condition &condition) const {
      auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
      auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
      auto recordDescriptors = std::vector<RecordDescriptor>{};
      std::function<void(const PositionId &, const storage_engine::lmdb::Result &)> callback =
          [&](const PositionId &positionId, const storage_engine::lmdb::Result &result) {
            auto rid = RecordId{classInfo.id, positionId};
            auto record = parser::RecordParser::parseRawDataWithBasicInfo(classInfo.name, rid, result,
                                                                          propertyIdMapInfo,
                                                                          classInfo.type);
            if (compare::RecordCompare::compareRecordByCondition(record, propertyType, condition)) {
              recordDescriptors.emplace_back(RecordDescriptor{rid});
            }
          };
      dataRecord.resultSetIter(callback);
      return recordDescriptors;
    }

    ResultSet
    DataRecordInterface::getResultSetByMultiCondition(const schema::ClassAccessInfo &classInfo,
                                                      const schema::PropertyNameMapInfo &propertyInfos,
                                                      const MultiCondition &multiCondition) const {
      auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
      auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
      auto propertyTypes = PropertyMapType{};
      for (const auto &property: propertyInfos) {
        propertyTypes.emplace(property.first, property.second.type);
      }
      auto resultSet = ResultSet{};
      std::function<void(const PositionId &, const storage_engine::lmdb::Result &)> callback =
          [&](const PositionId &positionId, const storage_engine::lmdb::Result &result) {
            auto rid = RecordId{classInfo.id, positionId};
            auto record = parser::RecordParser::parseRawDataWithBasicInfo(
                classInfo.name, rid, result, propertyIdMapInfo, classInfo.type);
            if (multiCondition.execute(record, propertyTypes)) {
              resultSet.emplace_back(Result{RecordDescriptor{rid}, record});
            }
          };
      dataRecord.resultSetIter(callback);
      return resultSet;
    }

    std::vector<RecordDescriptor>
    DataRecordInterface::getRecordDescriptorByMultiCondition(const schema::ClassAccessInfo &classInfo,
                                                             const schema::PropertyNameMapInfo &propertyInfos,
                                                             const MultiCondition &multiCondition) const {
      auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
      auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
      auto propertyTypes = PropertyMapType{};
      for (const auto &property: propertyInfos) {
        propertyTypes.emplace(property.first, property.second.type);
      }
      auto recordDescriptors = std::vector<RecordDescriptor>{};
      std::function<void(const PositionId &, const storage_engine::lmdb::Result &)> callback =
          [&](const PositionId &positionId, const storage_engine::lmdb::Result &result) {
            auto rid = RecordId{classInfo.id, positionId};
            auto record = parser::RecordParser::parseRawDataWithBasicInfo(
                classInfo.name, rid, result, propertyIdMapInfo, classInfo.type);
            if (multiCondition.execute(record, propertyTypes)) {
              recordDescriptors.emplace_back(RecordDescriptor{rid});
            }
          };
      dataRecord.resultSetIter(callback);
      return recordDescriptors;
    }

    ResultSet
    DataRecordInterface::getResultSetByCmpFunction(const schema::ClassAccessInfo &classInfo,
                                                   bool (*condition)(const Record &record)) const {
      auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
      auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = ResultSet{};
      std::function<void(const PositionId &, const storage_engine::lmdb::Result &)> callback =
          [&](const PositionId &positionId, const storage_engine::lmdb::Result &result) {
            auto rid = RecordId{classInfo.id, positionId};
            auto record = parser::RecordParser::parseRawDataWithBasicInfo(
                classInfo.name, rid, result, propertyIdMapInfo, classInfo.type);
            if ((*condition)(record)) {
              resultSet.emplace_back(Result{RecordDescriptor{rid}, record});
            }
          };
      dataRecord.resultSetIter(callback);
      return resultSet;
    }

    std::vector<RecordDescriptor>
    DataRecordInterface::getRecordDescriptorByCmpFunction(const schema::ClassAccessInfo &classInfo,
                                                          bool (*condition)(const Record &record)) const {
      auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, classInfo.id, classInfo.type);
      auto propertyIdMapInfo = _txn->_iSchema->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
      auto recordDescriptors = std::vector<RecordDescriptor>{};
      std::function<void(const PositionId &, const storage_engine::lmdb::Result &)> callback =
          [&](const PositionId &positionId, const storage_engine::lmdb::Result &result) {
            auto rid = RecordId{classInfo.id, positionId};
            auto record = parser::RecordParser::parseRawDataWithBasicInfo(
                classInfo.name, rid, result, propertyIdMapInfo, classInfo.type);
            if ((*condition)(record)) {
              recordDescriptors.emplace_back(RecordDescriptor{rid});
            }
          };
      dataRecord.resultSetIter(callback);
      return recordDescriptors;
    }

  }

}