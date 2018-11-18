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

#include "schema.hpp"
#include "constant.hpp"
#include "lmdb_engine.hpp"
#include "parser.hpp"
#include "relation.hpp"
#include "compare.hpp"
#include "index.hpp"
#include "datarecord.hpp"

#include "nogdb.h"

namespace nogdb {

  const RecordDescriptor Vertex::create(Txn &txn, const std::string &className, const Record &record) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
    auto recordBlob = parser::RecordParser::parseRecord(record, propertyNameMapInfo);
    try {
      auto vertexDataRecord = adapter::datarecord::DataRecord(txn._txnBase, vertexClassInfo.id, ClassType::VERTEX);
      auto positionId = vertexDataRecord.insert(recordBlob);
      auto recordDescriptor = RecordDescriptor{vertexClassInfo.id, positionId};
      txn._interface.index()->insert(recordDescriptor, record, propertyNameMapInfo);
      return recordDescriptor;
    } catch (const Error &error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Vertex::update(Txn &txn, const RecordDescriptor &recordDescriptor, const Record &record) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    auto vertexDataRecord = adapter::datarecord::DataRecord(txn._txnBase, vertexClassInfo.id, ClassType::VERTEX);
    auto existingRecordResult = vertexDataRecord.getResult(recordDescriptor.rid.second);
    auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
    auto newRecordBlob = parser::RecordParser::parseRecord(record, propertyNameMapInfo);
    try {
      // insert an updated record
      vertexDataRecord.insert(newRecordBlob);
      // remove index if applied in existing record
      auto propertyIdMapInfo = txn._interface.schema()->getPropertyIdMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
      auto existingRecord = parser::RecordParser::parseRawData(existingRecordResult, propertyIdMapInfo, true);
      txn._interface.index()->remove(recordDescriptor, existingRecord, propertyNameMapInfo);
      // add index if applied in new record
      txn._interface.index()->insert(recordDescriptor, record, propertyNameMapInfo);
    } catch (const Error &error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Vertex::destroy(Txn &txn, const RecordDescriptor &recordDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    auto vertexDataRecord = adapter::datarecord::DataRecord(txn._txnBase, vertexClassInfo.id, ClassType::VERTEX);
    auto recordResult = vertexDataRecord.getResult(recordDescriptor.rid.second);
    try {
      auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
      auto propertyIdMapInfo = txn._interface.schema()->getPropertyIdMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
      vertexDataRecord.remove(recordDescriptor.rid.second);
      txn._interface.graph()->removeRelFromVertex(recordDescriptor.rid);
      // remove index if applied in the record
      auto record = parser::RecordParser::parseRawData(recordResult, propertyIdMapInfo, true);
      txn._interface.index()->remove(recordDescriptor, record, propertyNameMapInfo);
    } catch (const Error &error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Vertex::destroy(Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    try {
      auto vertexDataRecord = adapter::datarecord::DataRecord(txn._txnBase, vertexClassInfo.id, ClassType::VERTEX);
      auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
      auto result = std::map<RecordId, std::pair<RecordId, RecordId>>{};
      std::function<void(const PositionId &, const storage_engine::lmdb::Result &)> callback =
          [&](const PositionId &positionId, const storage_engine::lmdb::Result &result) {
            auto recordId = RecordId{vertexClassInfo.id, positionId};
            txn._interface.graph()->removeRelFromVertex(recordId);
          };
      vertexDataRecord.resultSetIter(callback);
      vertexDataRecord.destroy();
      txn._interface.index()->drop(vertexClassInfo.id, propertyNameMapInfo);
    } catch (const Error &error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  ResultSet Vertex::get(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    return txn._interface.record()->getResultSet(vertexClassInfo);
  }

  ResultSet Vertex::getExtend(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto vertexClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    vertexClassInfoExtend = txn._interface.schema()->getSubClassInfos(vertexClassInfo.id, vertexClassInfoExtend);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: vertexClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto resultSet = txn._interface.record()->getResultSet(classInfo);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }


  ResultSetCursor Vertex::getCursor(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    return txn._interface.record()->getResultSetCursor(vertexClassInfo);
  }

  ResultSetCursor Vertex::getExtendCursor(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto vertexClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    vertexClassInfoExtend = txn._interface.schema()->getSubClassInfos(vertexClassInfo.id, vertexClassInfoExtend);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: vertexClassInfoExtend) {
      resultSetExtend.addMetadata(txn._interface.record()->getResultSetCursor(classNameMapInfo.second));
    }
    return resultSetExtend;
  }

  ResultSet Vertex::getInEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const GraphFilter &edgeFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    auto edgeRecordIds = txn._interface.graph()->getInEdges(recordDescriptor.rid);
    auto result = ResultSet{};
    for (const auto &recordId: edgeRecordIds) {
      auto edgeRecordDescriptor = RecordDescriptor{recordId};
      auto filterResult = compare::RecordCompare::filterResult(txn, edgeRecordDescriptor, edgeFilter);
      if (filterResult.descriptor != RecordDescriptor{}) {
        result.emplace_back(filterResult);
      }
    }
    return result;
  }

  ResultSet Vertex::getOutEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const GraphFilter &edgeFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    auto edgeRecordIds = txn._interface.graph()->getOutEdges(recordDescriptor.rid);
    auto result = ResultSet{};
    for (const auto &recordId: edgeRecordIds) {
      auto edgeRecordDescriptor = RecordDescriptor{recordId};
      auto filterResult = compare::RecordCompare::filterResult(txn, edgeRecordDescriptor, edgeFilter);
      if (filterResult.descriptor != RecordDescriptor{}) {
        result.emplace_back(filterResult);
      }
    }
    return result;
  }

  ResultSet Vertex::getAllEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const GraphFilter &edgeFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    auto edgeRecordIds = std::set<RecordId>{};
    auto inEdgeRecordIds = txn._interface.graph()->getInEdges(recordDescriptor.rid);
    auto outEdgeRecordIds = txn._interface.graph()->getOutEdges(recordDescriptor.rid);
    edgeRecordIds.insert(inEdgeRecordIds.cbegin(), inEdgeRecordIds.cend());
    edgeRecordIds.insert(outEdgeRecordIds.cbegin(), outEdgeRecordIds.cend());
    auto result = ResultSet{};
    for (const auto &recordId: edgeRecordIds) {
      auto edgeRecordDescriptor = RecordDescriptor{recordId};
      auto filterResult = compare::RecordCompare::filterResult(txn, edgeRecordDescriptor, edgeFilter);
      if (filterResult.descriptor != RecordDescriptor{}) {
        result.emplace_back(filterResult);
      }
    }
    return result;
  }

  ResultSetCursor Vertex::getInEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, const GraphFilter &edgeFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    auto edgeRecordIds = txn._interface.graph()->getInEdges(recordDescriptor.rid);
    auto result = ResultSetCursor{txn};
    for (const auto &recordId: edgeRecordIds) {
      auto edgeRecordDescriptor = RecordDescriptor{recordId};
      auto filterRecord = compare::RecordCompare::filterRecord(txn, edgeRecordDescriptor, edgeFilter);
      if (filterRecord != RecordDescriptor{}) {
        result.addMetadata(filterRecord);
      }
    }
    return result;
  }

  ResultSetCursor Vertex::getOutEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, const GraphFilter &edgeFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    auto edgeRecordIds = txn._interface.graph()->getOutEdges(recordDescriptor.rid);
    auto result = ResultSetCursor{txn};
    for (const auto &recordId: edgeRecordIds) {
      auto edgeRecordDescriptor = RecordDescriptor{recordId};
      auto filterRecord = compare::RecordCompare::filterRecord(txn, edgeRecordDescriptor, edgeFilter);
      if (filterRecord != RecordDescriptor{}) {
        result.addMetadata(filterRecord);
      }
    }
    return result;
  }

  ResultSetCursor Vertex::getAllEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, const GraphFilter &edgeFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    auto edgeRecordIds = std::set<RecordId>{};
    auto inEdgeRecordIds = txn._interface.graph()->getInEdges(recordDescriptor.rid);
    auto outEdgeRecordIds = txn._interface.graph()->getOutEdges(recordDescriptor.rid);
    edgeRecordIds.insert(inEdgeRecordIds.cbegin(), inEdgeRecordIds.cend());
    edgeRecordIds.insert(outEdgeRecordIds.cbegin(), outEdgeRecordIds.cend());
    auto result = ResultSetCursor{txn};
    for (const auto &recordId: edgeRecordIds) {
      auto edgeRecordDescriptor = RecordDescriptor{recordId};
      auto filterRecord = compare::RecordCompare::filterRecord(txn, edgeRecordDescriptor, edgeFilter);
      if (filterRecord != RecordDescriptor{}) {
        result.addMetadata(filterRecord);
      }
    }
    return result;
  }

  ResultSet Vertex::get(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
    return compare::RecordCompare::compareCondition(txn, vertexClassInfo, propertyNameMapInfo, condition);
  }

  ResultSet Vertex::get(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
    return txn._interface.record()->getResultSetByCmpFunction(vertexClassInfo, condition);
  }

  ResultSet Vertex::get(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
    return compare::RecordCompare::compareMultiCondition(txn, vertexClassInfo, propertyNameMapInfo, multiCondition);
  }

  ResultSet Vertex::getExtend(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto vertexClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    vertexClassInfoExtend = txn._interface.schema()->getSubClassInfos(vertexClassInfo.id, vertexClassInfoExtend);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: vertexClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareCondition(txn, classInfo, propertyNameMapInfo, condition);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSet Vertex::getExtend(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto vertexClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    vertexClassInfoExtend = txn._interface.schema()->getSubClassInfos(vertexClassInfo.id, vertexClassInfoExtend);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: vertexClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto resultSet = txn._interface.record()->getResultSetByCmpFunction(classInfo, condition);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSet Vertex::getExtend(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto vertexClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    vertexClassInfoExtend = txn._interface.schema()->getSubClassInfos(vertexClassInfo.id, vertexClassInfoExtend);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: vertexClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareMultiCondition(txn, classInfo, propertyNameMapInfo, multiCondition);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSetCursor Vertex::getCursor(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
    auto result = compare::RecordCompare::compareConditionRdesc(txn, vertexClassInfo, propertyNameMapInfo, condition);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor Vertex::getCursor(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto result = txn._interface.record()->getRecordDescriptorByCmpFunction(vertexClassInfo, condition);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor
  Vertex::getCursor(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
    auto result = compare::RecordCompare::compareMultiConditionRdesc(txn, vertexClassInfo, propertyNameMapInfo, multiCondition);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor Vertex::getExtendCursor(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto vertexClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    vertexClassInfoExtend = txn._interface.schema()->getSubClassInfos(vertexClassInfo.id, vertexClassInfoExtend);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: vertexClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareConditionRdesc(txn, classInfo, propertyNameMapInfo, condition);
      resultSetExtend.addMetadata(resultSet);
    }
    return resultSetExtend;
  }

  ResultSetCursor
  Vertex::getExtendCursor(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto vertexClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    vertexClassInfoExtend = txn._interface.schema()->getSubClassInfos(vertexClassInfo.id, vertexClassInfoExtend);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: vertexClassInfoExtend) {
      resultSetExtend.addMetadata(txn._interface.record()->getRecordDescriptorByCmpFunction(classNameMapInfo.second, condition));
    }
    return resultSetExtend;
  }

  ResultSetCursor
  Vertex::getExtendCursor(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto vertexClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    vertexClassInfoExtend = txn._interface.schema()->getSubClassInfos(vertexClassInfo.id, vertexClassInfoExtend);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: vertexClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareMultiConditionRdesc(txn, classInfo, propertyNameMapInfo, multiCondition);
      resultSetExtend.addMetadata(resultSet);
    }
    return resultSetExtend;
  }

  ResultSet Vertex::getIndex(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
    return compare::RecordCompare::compareCondition(txn, vertexClassInfo, propertyNameMapInfo, condition, true);
  }

  ResultSet Vertex::getIndex(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
    return compare::RecordCompare::compareMultiCondition(txn, vertexClassInfo, propertyNameMapInfo, multiCondition, true);
  }

  ResultSet Vertex::getExtendIndex(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto vertexClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    vertexClassInfoExtend = txn._interface.schema()->getSubClassInfos(vertexClassInfo.id, vertexClassInfoExtend);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: vertexClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareCondition(txn, classInfo, propertyNameMapInfo, condition, true);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSet Vertex::getExtendIndex(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto vertexClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    vertexClassInfoExtend = txn._interface.schema()->getSubClassInfos(vertexClassInfo.id, vertexClassInfoExtend);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: vertexClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareMultiCondition(txn, classInfo, propertyNameMapInfo, multiCondition, true);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSetCursor Vertex::getIndexCursor(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
    auto result = compare::RecordCompare::compareConditionRdesc(txn, vertexClassInfo, propertyNameMapInfo, condition, true);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor
  Vertex::getIndexCursor(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
    auto result = compare::RecordCompare::compareMultiConditionRdesc(txn, vertexClassInfo, propertyNameMapInfo, multiCondition, true);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor
  Vertex::getExtendIndexCursor(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto vertexClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    vertexClassInfoExtend = txn._interface.schema()->getSubClassInfos(vertexClassInfo.id, vertexClassInfoExtend);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: vertexClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareConditionRdesc(txn, classInfo, propertyNameMapInfo, condition, true);
      resultSetExtend.addMetadata(resultSet);
    }
    return resultSetExtend;
  }

  ResultSetCursor
  Vertex::getExtendIndexCursor(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto vertexClassInfo = txn._interface.schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto vertexClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    vertexClassInfoExtend = txn._interface.schema()->getSubClassInfos(vertexClassInfo.id, vertexClassInfoExtend);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: vertexClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface.schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareMultiConditionRdesc(txn, classInfo, propertyNameMapInfo, multiCondition, true);
      resultSetExtend.addMetadata(resultSet);
    }
    return resultSetExtend;
  }

}
