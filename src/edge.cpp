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

#include <tuple>

#include "constant.hpp"
#include "lmdb_engine.hpp"
#include "datarecord_adapter.hpp"
#include "index_adapter.hpp"
#include "parser.hpp"
#include "compare.hpp"
#include "schema.hpp"
#include "index.hpp"
#include "relation.hpp"
#include "datarecord.hpp"

#include "nogdb.h"

namespace nogdb {

  const RecordDescriptor Edge::create(Txn &txn,
                                      const std::string &className,
                                      const RecordDescriptor &srcVertexRecordDescriptor,
                                      const RecordDescriptor &dstVertexRecordDescriptor,
                                      const Record &record) {
    BEGIN_VALIDATION(&txn)
        .isTxnValid()
        .isTxnCompleted()
        .isClassNameValid(className)
        .isExistingSrcVertex(srcVertexRecordDescriptor)
        .isExistingDstVertex(dstVertexRecordDescriptor);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    auto recordBlob = parser::RecordParser::parseRecord(record, propertyNameMapInfo);
    try {
      auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
      auto vertexBlob = parser::RecordParser::parseEdgeVertexSrcDst(srcVertexRecordDescriptor.rid,
                                                                    dstVertexRecordDescriptor.rid);
      auto positionId = edgeDataRecord.insert(vertexBlob + recordBlob);
      auto recordDescriptor = RecordDescriptor{edgeClassInfo.id, positionId};
      txn._interface->graph()->addRel(recordDescriptor.rid, srcVertexRecordDescriptor.rid, dstVertexRecordDescriptor.rid);
      auto indexInfos = txn._interface->index()->getIndexInfos(recordDescriptor, record, propertyNameMapInfo);
      txn._interface->index()->insert(recordDescriptor, record, indexInfos);
      return recordDescriptor;
    } catch (const Error& error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Edge::update(Txn &txn, const RecordDescriptor &recordDescriptor, const Record &record) {
    BEGIN_VALIDATION(&txn)
        .isTxnValid()
        .isTxnCompleted();

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto existingRecordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
    auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    auto newRecordBlob = parser::RecordParser::parseRecord(record, propertyNameMapInfo);
    try {
      // insert an updated record
      auto vertexBlob = parser::RecordParser::parseEdgeRawDataVertexSrcDstAsBlob(existingRecordResult.data.blob());
      edgeDataRecord.insert(vertexBlob + newRecordBlob);
      // remove index if applied in existing record
      auto propertyIdMapInfo = txn._interface->schema()->getPropertyIdMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
      auto existingRecord = parser::RecordParser::parseRawData(existingRecordResult, propertyIdMapInfo, true);
      auto indexInfos = txn._interface->index()->getIndexInfos(recordDescriptor, record, propertyNameMapInfo);
      txn._interface->index()->remove(recordDescriptor, existingRecord, indexInfos);
      // add index if applied in new record
      txn._interface->index()->insert(recordDescriptor, record, indexInfos);
    } catch (const Error& error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Edge::destroy(Txn &txn, const RecordDescriptor &recordDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTxnValid()
        .isTxnCompleted();

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto recordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
    try {
      auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
      auto propertyIdMapInfo = txn._interface->schema()->getPropertyIdMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
      auto srcDstVertex = parser::RecordParser::parseEdgeRawDataVertexSrcDst(recordResult.data.blob());
      edgeDataRecord.remove(recordDescriptor.rid.second);
      txn._interface->graph()->removeRelFromEdge(recordDescriptor.rid, srcDstVertex.first, srcDstVertex.second);
      // remove index if applied in the record
      auto record = parser::RecordParser::parseRawData(recordResult, propertyIdMapInfo, true);
      auto indexInfos = txn._interface->index()->getIndexInfos(recordDescriptor, record, propertyNameMapInfo);
      txn._interface->index()->remove(recordDescriptor, record, indexInfos);
    } catch (const Error& error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Edge::destroy(Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTxnValid()
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    try {
      auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
      auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
      auto result = std::map<RecordId, std::pair<RecordId, RecordId>>{};
      std::function<void(const PositionId &, const storage_engine::lmdb::Result &)> callback =
          [&](const PositionId &positionId, const storage_engine::lmdb::Result &result) {
            auto srcDstVertex = parser::RecordParser::parseEdgeRawDataVertexSrcDst(result.data.blob());
            auto edgeRecordId = RecordId{edgeClassInfo.id, positionId};
            txn._interface->graph()->removeRelFromEdge(edgeRecordId, srcDstVertex.first, srcDstVertex.second);
          };
      edgeDataRecord.resultSetIter(callback);
      edgeDataRecord.destroy();
      txn._interface->index()->drop(edgeClassInfo.id, propertyNameMapInfo);
    } catch (const Error& error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Edge::updateSrc(Txn &txn,
                       const RecordDescriptor &recordDescriptor,
                       const RecordDescriptor &newSrcVertexRecordDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTxnValid()
        .isTxnCompleted()
        .isExistingSrcVertex(newSrcVertexRecordDescriptor);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto recordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
    try {
      auto srcDstVertex = parser::RecordParser::parseEdgeRawDataVertexSrcDst(recordResult.data.blob());
      txn._interface->graph()->updateSrcRel(recordDescriptor.rid, newSrcVertexRecordDescriptor.rid, srcDstVertex.first,
                                srcDstVertex.second);
      auto newVertexBlob = parser::RecordParser::parseEdgeVertexSrcDst(newSrcVertexRecordDescriptor.rid, srcDstVertex.second);
      auto dataBlob = parser::RecordParser::parseEdgeRawDataAsBlob(recordResult.data.blob());
      edgeDataRecord.insert(newVertexBlob + dataBlob);
    } catch (const Error& error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Edge::updateDst(Txn &txn,
                       const RecordDescriptor &recordDescriptor,
                       const RecordDescriptor &newDstVertexDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTxnValid()
        .isTxnCompleted()
        .isExistingSrcVertex(newDstVertexDescriptor);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto recordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
    try {
      auto srcDstVertex = parser::RecordParser::parseEdgeRawDataVertexSrcDst(recordResult.data.blob());
      txn._interface->graph()->updateDstRel(recordDescriptor.rid, newDstVertexDescriptor.rid, srcDstVertex.first,
                                srcDstVertex.second);
      auto newVertexBlob = parser::RecordParser::parseEdgeVertexSrcDst(srcDstVertex.first, newDstVertexDescriptor.rid);
      auto dataBlob = parser::RecordParser::parseEdgeRawDataAsBlob(recordResult.data.blob());
      edgeDataRecord.insert(newVertexBlob + dataBlob);
    } catch (const Error& error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  ResultSet Edge::get(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    return txn._interface->record()->getResultSet(edgeClassInfo);
  }

  ResultSet Edge::getExtend(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = txn._interface->schema()->getSubClassInfos(edgeClassInfo.id);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto resultSet = txn._interface->record()->getResultSet(classInfo);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSetCursor Edge::getCursor(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    return txn._interface->record()->getResultSetCursor(edgeClassInfo);
  }

  ResultSetCursor Edge::getExtendCursor(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = txn._interface->schema()->getSubClassInfos(edgeClassInfo.id);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      resultSetExtend.addMetadata(txn._interface->record()->getResultSetCursor(classNameMapInfo.second));
    }
    return resultSetExtend;
  }

  Result Edge::getSrc(const Txn &txn, const RecordDescriptor &recordDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted();

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto srcDstVertex = txn._interface->graph()->getSrcDstVertices(recordDescriptor.rid);
    auto srcVertexRecordDescriptor = RecordDescriptor{srcDstVertex.first};
    auto srcVertexClassInfo = txn._interface->schema()->getExistingClass(srcVertexRecordDescriptor.rid.first);
    return Result{srcVertexRecordDescriptor, txn._interface->record()->getRecord(srcVertexClassInfo, srcVertexRecordDescriptor)};
  }

  Result Edge::getDst(const Txn &txn, const RecordDescriptor &recordDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted();

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto srcDstVertex = txn._interface->graph()->getSrcDstVertices(recordDescriptor.rid);
    auto dstVertexRecordDescriptor = RecordDescriptor{srcDstVertex.second};
    auto dstVertexClassInfo = txn._interface->schema()->getExistingClass(dstVertexRecordDescriptor.rid.first);
    return Result{dstVertexRecordDescriptor, txn._interface->record()->getRecord(dstVertexClassInfo, dstVertexRecordDescriptor)};
  }

  ResultSet Edge::getSrcDst(const Txn &txn, const RecordDescriptor &recordDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted();

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto srcDstVertex = txn._interface->graph()->getSrcDstVertices(recordDescriptor.rid);
    auto srcVertexRecordDescriptor = RecordDescriptor{srcDstVertex.first};
    auto srcVertexClassInfo = txn._interface->schema()->getExistingClass(srcVertexRecordDescriptor.rid.first);
    auto dstVertexRecordDescriptor = RecordDescriptor{srcDstVertex.second};
    auto dstVertexClassInfo = txn._interface->schema()->getExistingClass(dstVertexRecordDescriptor.rid.first);
    auto srcVertexResult = txn._interface->record()->getRecord(srcVertexClassInfo, srcVertexRecordDescriptor);
    auto dstVertexResult = txn._interface->record()->getRecord(dstVertexClassInfo, dstVertexRecordDescriptor);
    return ResultSet{
        Result{srcVertexRecordDescriptor, srcVertexResult},
        Result{dstVertexRecordDescriptor, dstVertexResult}
    };
  }

  ResultSet Edge::get(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    return compare::RecordCompare::compareCondition(txn, edgeClassInfo, propertyNameMapInfo, condition);
  }

  ResultSet Edge::get(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    return txn._interface->record()->getResultSetByCmpFunction(edgeClassInfo, condition);
  }

  ResultSet Edge::get(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    return compare::RecordCompare::compareMultiCondition(txn, edgeClassInfo, propertyNameMapInfo, multiCondition);
  }

  ResultSet Edge::getExtend(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = txn._interface->schema()->getSubClassInfos(edgeClassInfo.id);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareCondition(txn, classInfo, propertyNameMapInfo, condition);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSet Edge::getExtend(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = txn._interface->schema()->getSubClassInfos(edgeClassInfo.id);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto resultSet = txn._interface->record()->getResultSetByCmpFunction(classInfo, condition);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSet Edge::getExtend(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = txn._interface->schema()->getSubClassInfos(edgeClassInfo.id);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareMultiCondition(txn, classInfo, propertyNameMapInfo, multiCondition);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSetCursor Edge::getCursor(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    auto result = compare::RecordCompare::compareConditionRdesc(txn, edgeClassInfo, propertyNameMapInfo, condition);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor Edge::getCursor(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto result = txn._interface->record()->getRecordDescriptorByCmpFunction(edgeClassInfo, condition);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor Edge::getCursor(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    auto result = compare::RecordCompare::compareMultiConditionRdesc(txn, edgeClassInfo, propertyNameMapInfo, multiCondition);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor Edge::getExtendCursor(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = txn._interface->schema()->getSubClassInfos(edgeClassInfo.id);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareConditionRdesc(txn, classInfo, propertyNameMapInfo, condition);
      resultSetExtend.addMetadata(resultSet);
    }
    return resultSetExtend;
  }

  ResultSetCursor
  Edge::getExtendCursor(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = txn._interface->schema()->getSubClassInfos(edgeClassInfo.id);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      resultSetExtend.addMetadata(txn._interface->record()->getRecordDescriptorByCmpFunction(classNameMapInfo.second, condition));
    }
    return resultSetExtend;
  }

  ResultSetCursor
  Edge::getExtendCursor(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = txn._interface->schema()->getSubClassInfos(edgeClassInfo.id);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareMultiConditionRdesc(txn, classInfo, propertyNameMapInfo, multiCondition);
      resultSetExtend.addMetadata(resultSet);
    }
    return resultSetExtend;
  }

  ResultSet Edge::getIndex(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    return compare::RecordCompare::compareCondition(txn, edgeClassInfo, propertyNameMapInfo, condition, true);
  }

  ResultSet Edge::getIndex(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    return compare::RecordCompare::compareMultiCondition(txn, edgeClassInfo, propertyNameMapInfo, multiCondition, true);
  }

  ResultSet Edge::getExtendIndex(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = txn._interface->schema()->getSubClassInfos(edgeClassInfo.id);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareCondition(txn, classInfo, propertyNameMapInfo, condition, true);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSet Edge::getExtendIndex(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = txn._interface->schema()->getSubClassInfos(edgeClassInfo.id);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareMultiCondition(txn, classInfo, propertyNameMapInfo, multiCondition, true);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSetCursor Edge::getIndexCursor(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    auto result = compare::RecordCompare::compareConditionRdesc(txn, edgeClassInfo, propertyNameMapInfo, condition, true);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor
  Edge::getIndexCursor(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    auto result = compare::RecordCompare::compareMultiConditionRdesc(txn, edgeClassInfo, propertyNameMapInfo, multiCondition, true);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor Edge::getExtendIndexCursor(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = txn._interface->schema()->getSubClassInfos(edgeClassInfo.id);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareConditionRdesc(txn, classInfo, propertyNameMapInfo, condition, true);
      resultSetExtend.addMetadata(resultSet);
    }
    return resultSetExtend;
  }

  ResultSetCursor
  Edge::getExtendIndexCursor(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto edgeClassInfo = txn._interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = txn._interface->schema()->getSubClassInfos(edgeClassInfo.id);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = compare::RecordCompare::compareMultiConditionRdesc(txn, classInfo, propertyNameMapInfo, multiCondition, true);
      resultSetExtend.addMetadata(resultSet);
    }
    return resultSetExtend;
  }

}
