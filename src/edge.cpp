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

  const RecordDescriptor Edge::create(const Txn &txn,
                                      const std::string &className,
                                      const RecordDescriptor &srcVertexRecordDescriptor,
                                      const RecordDescriptor &dstVertexRecordDescriptor,
                                      const Record &record) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingSrcVertex(srcVertexRecordDescriptor)
        .isExistingDstVertex(dstVertexRecordDescriptor);

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    auto recordBlob = parser::Parser::parseRecord(record, propertyNameMapInfo);
    try {
      auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
      auto vertexBlob = parser::Parser::parseEdgeVertexSrcDst(srcVertexRecordDescriptor.rid,
                                                              dstVertexRecordDescriptor.rid);
      auto positionId = edgeDataRecord.insert(vertexBlob + recordBlob);
      auto recordDescriptor = RecordDescriptor{edgeClassInfo.id, positionId};
      txn._iGraph->addRel(recordDescriptor.rid, srcVertexRecordDescriptor.rid, dstVertexRecordDescriptor.rid);
      txn._iIndex->insert(recordDescriptor, record, propertyNameMapInfo);
      return recordDescriptor;
    } catch (const Error &error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Edge::update(const Txn &txn, const RecordDescriptor &recordDescriptor, const Record &record) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto existingRecordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    auto newRecordBlob = parser::Parser::parseRecord(record, propertyNameMapInfo);
    try {
      // insert an updated record
      auto vertexBlob = parser::Parser::parseEdgeRawDataVertexSrcDstAsBlob(existingRecordResult.data.blob());
      edgeDataRecord.insert(vertexBlob + newRecordBlob);
      // remove index if applied in existing record
      auto propertyIdMapInfo = txn._iSchema->getPropertyIdMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
      auto existingRecord = parser::Parser::parseRawData(existingRecordResult, propertyIdMapInfo, true);
      txn._iIndex->remove(recordDescriptor, existingRecord, propertyNameMapInfo);
      // add index if applied in new record
      txn._iIndex->insert(recordDescriptor, record, propertyNameMapInfo);
    } catch (const Error &error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Edge::destroy(const Txn &txn, const RecordDescriptor &recordDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto recordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
    try {
      auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
      auto propertyIdMapInfo = txn._iSchema->getPropertyIdMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
      auto srcDstVertex = parser::Parser::parseEdgeRawDataVertexSrcDst(recordResult.data.blob());
      edgeDataRecord.remove(recordDescriptor.rid.second);
      txn._iGraph->removeRelFromEdge(recordDescriptor.rid, srcDstVertex.first, srcDstVertex.second);
      // remove index if applied in the record
      auto record = parser::Parser::parseRawData(recordResult, propertyIdMapInfo, true);
      txn._iIndex->remove(recordDescriptor, record, propertyNameMapInfo);
    } catch (const Error &error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Edge::destroy(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    try {
      auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
      auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
      auto result = std::map<RecordId, std::pair<RecordId, RecordId>>{};
      std::function<void(const PositionId &, const storage_engine::lmdb::Result &)> callback =
          [&](const PositionId &positionId, const storage_engine::lmdb::Result &result) {
            auto srcDstVertex = parser::Parser::parseEdgeRawDataVertexSrcDst(result.data.blob());
            auto edgeRecordId = RecordId{edgeClassInfo.id, positionId};
            txn._iGraph->removeRelFromEdge(edgeRecordId, srcDstVertex.first, srcDstVertex.second);
          };
      edgeDataRecord.resultSetIter(callback);
      edgeDataRecord.destroy();
      txn._iIndex->drop(edgeClassInfo.id, propertyNameMapInfo);
    } catch (const Error &error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Edge::updateSrc(const Txn &txn,
                       const RecordDescriptor &recordDescriptor,
                       const RecordDescriptor &newSrcVertexRecordDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingSrcVertex(newSrcVertexRecordDescriptor);

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto recordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
    try {
      auto srcDstVertex = parser::Parser::parseEdgeRawDataVertexSrcDst(recordResult.data.blob());
      txn._iGraph->updateSrcRel(recordDescriptor.rid, newSrcVertexRecordDescriptor.rid, srcDstVertex.first,
                                srcDstVertex.second);
      auto newVertexBlob = parser::Parser::parseEdgeVertexSrcDst(newSrcVertexRecordDescriptor.rid, srcDstVertex.second);
      auto dataBlob = parser::Parser::parseEdgeRawDataAsBlob(recordResult.data.blob());
      edgeDataRecord.insert(newVertexBlob + dataBlob);
    } catch (const Error &error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Edge::updateDst(const Txn &txn,
                       const RecordDescriptor &recordDescriptor,
                       const RecordDescriptor &newDstVertexDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingSrcVertex(newDstVertexDescriptor);

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto recordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
    try {
      auto srcDstVertex = parser::Parser::parseEdgeRawDataVertexSrcDst(recordResult.data.blob());
      txn._iGraph->updateDstRel(recordDescriptor.rid, newDstVertexDescriptor.rid, srcDstVertex.first,
                                srcDstVertex.second);
      auto newVertexBlob = parser::Parser::parseEdgeVertexSrcDst(srcDstVertex.first, newDstVertexDescriptor.rid);
      auto dataBlob = parser::Parser::parseEdgeRawDataAsBlob(recordResult.data.blob());
      edgeDataRecord.insert(newVertexBlob + dataBlob);
    } catch (const Error &error) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  ResultSet Edge::get(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    return txn._iRecord->getResultSet(edgeClassInfo);
  }

  ResultSet Edge::getExtend(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    edgeClassInfoExtend = txn._iSchema->getSubClassInfos(edgeClassInfo.id, edgeClassInfoExtend);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto resultSet = txn._iRecord->getResultSet(classInfo);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSetCursor Edge::getCursor(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    return txn._iRecord->getResultSetCursor(edgeClassInfo);
  }

  ResultSetCursor Edge::getExtendCursor(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    edgeClassInfoExtend = txn._iSchema->getSubClassInfos(edgeClassInfo.id, edgeClassInfoExtend);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      resultSetExtend.addMetadata(txn._iRecord->getResultSetCursor(classNameMapInfo.second));
    }
    return resultSetExtend;
  }

  Result Edge::getSrc(const Txn &txn, const RecordDescriptor &recordDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto rawData = edgeDataRecord.getBlob(recordDescriptor.rid.second);
    auto srcDstVertex = parser::Parser::parseEdgeRawDataVertexSrcDst(rawData);
    auto srcVertexRecordDescriptor = RecordDescriptor{srcDstVertex.first};
    auto srcVertexClassInfo = txn._iSchema->getExistingClass(srcVertexRecordDescriptor.rid.first);
    return Result{srcVertexRecordDescriptor, txn._iRecord->getRecord(srcVertexClassInfo, srcVertexRecordDescriptor)};
  }

  Result Edge::getDst(const Txn &txn, const RecordDescriptor &recordDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto rawData = edgeDataRecord.getBlob(recordDescriptor.rid.second);
    auto srcDstVertex = parser::Parser::parseEdgeRawDataVertexSrcDst(rawData);
    auto dstVertexRecordDescriptor = RecordDescriptor{srcDstVertex.second};
    auto dstVertexClassInfo = txn._iSchema->getExistingClass(dstVertexRecordDescriptor.rid.first);
    return Result{dstVertexRecordDescriptor, txn._iRecord->getRecord(dstVertexClassInfo, dstVertexRecordDescriptor)};
  }

  ResultSet Edge::getSrcDst(const Txn &txn, const RecordDescriptor &recordDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto rawData = edgeDataRecord.getBlob(recordDescriptor.rid.second);
    auto srcDstVertex = parser::Parser::parseEdgeRawDataVertexSrcDst(rawData);
    auto srcVertexRecordDescriptor = RecordDescriptor{srcDstVertex.first};
    auto srcVertexClassInfo = txn._iSchema->getExistingClass(srcVertexRecordDescriptor.rid.first);
    auto dstVertexRecordDescriptor = RecordDescriptor{srcDstVertex.second};
    auto dstVertexClassInfo = txn._iSchema->getExistingClass(dstVertexRecordDescriptor.rid.first);
    auto srcVertexResult = txn._iRecord->getRecord(srcVertexClassInfo, srcVertexRecordDescriptor);
    auto dstVertexResult = txn._iRecord->getRecord(dstVertexClassInfo, dstVertexRecordDescriptor);
    return ResultSet{
        Result{srcVertexRecordDescriptor, srcVertexResult},
        Result{dstVertexRecordDescriptor, dstVertexResult}
    };
  }

  ResultSet Edge::get(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    return Compare::compareCondition(txn, edgeClassInfo, propertyNameMapInfo, condition);
  }

  ResultSet Edge::get(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    return txn._iRecord->getResultSetByCmpFunction(edgeClassInfo, condition);
  }

  ResultSet Edge::get(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    return Compare::compareMultiCondition(txn, edgeClassInfo, propertyNameMapInfo, multiCondition);
  }

  ResultSet Edge::getExtend(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    edgeClassInfoExtend = txn._iSchema->getSubClassInfos(edgeClassInfo.id, edgeClassInfoExtend);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = Compare::compareCondition(txn, classInfo, propertyNameMapInfo, condition);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSet Edge::getExtend(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    edgeClassInfoExtend = txn._iSchema->getSubClassInfos(edgeClassInfo.id, edgeClassInfoExtend);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto resultSet = txn._iRecord->getResultSetByCmpFunction(classInfo, condition);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSet Edge::getExtend(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    edgeClassInfoExtend = txn._iSchema->getSubClassInfos(edgeClassInfo.id, edgeClassInfoExtend);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = Compare::compareMultiCondition(txn, classInfo, propertyNameMapInfo, multiCondition);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSetCursor Edge::getCursor(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    auto result = Compare::compareConditionRdesc(txn, edgeClassInfo, propertyNameMapInfo, condition);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor Edge::getCursor(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto result = txn._iRecord->getRecordDescriptorByCmpFunction(edgeClassInfo, condition);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor Edge::getCursor(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    auto result = Compare::compareMultiConditionRdesc(txn, edgeClassInfo, propertyNameMapInfo, multiCondition);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor Edge::getExtendCursor(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    edgeClassInfoExtend = txn._iSchema->getSubClassInfos(edgeClassInfo.id, edgeClassInfoExtend);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = Compare::compareConditionRdesc(txn, classInfo, propertyNameMapInfo, condition);
      resultSetExtend.addMetadata(resultSet);
    }
    return resultSetExtend;
  }

  ResultSetCursor
  Edge::getExtendCursor(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    edgeClassInfoExtend = txn._iSchema->getSubClassInfos(edgeClassInfo.id, edgeClassInfoExtend);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      resultSetExtend.addMetadata(txn._iRecord->getRecordDescriptorByCmpFunction(classNameMapInfo.second, condition));
    }
    return resultSetExtend;
  }

  ResultSetCursor
  Edge::getExtendCursor(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    edgeClassInfoExtend = txn._iSchema->getSubClassInfos(edgeClassInfo.id, edgeClassInfoExtend);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = Compare::compareMultiConditionRdesc(txn, classInfo, propertyNameMapInfo, multiCondition);
      resultSetExtend.addMetadata(resultSet);
    }
    return resultSetExtend;
  }

  ResultSet Edge::getIndex(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    return Compare::compareCondition(txn, edgeClassInfo, propertyNameMapInfo, condition, true);
  }

  ResultSet Edge::getIndex(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    return Compare::compareMultiCondition(txn, edgeClassInfo, propertyNameMapInfo, multiCondition, true);
  }

  ResultSet Edge::getExtendIndex(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    edgeClassInfoExtend = txn._iSchema->getSubClassInfos(edgeClassInfo.id, edgeClassInfoExtend);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = Compare::compareCondition(txn, classInfo, propertyNameMapInfo, condition, true);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSet Edge::getExtendIndex(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    edgeClassInfoExtend = txn._iSchema->getSubClassInfos(edgeClassInfo.id, edgeClassInfoExtend);
    auto resultSetExtend = ResultSet{};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = Compare::compareMultiCondition(txn, classInfo, propertyNameMapInfo, multiCondition, true);
      resultSetExtend.insert(resultSetExtend.cend(), resultSet.cbegin(), resultSet.cend());
    }
    return resultSetExtend;
  }

  ResultSetCursor Edge::getIndexCursor(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    auto result = Compare::compareConditionRdesc(txn, edgeClassInfo, propertyNameMapInfo, condition, true);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor
  Edge::getIndexCursor(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    auto result = Compare::compareMultiConditionRdesc(txn, edgeClassInfo, propertyNameMapInfo, multiCondition, true);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSetCursor Edge::getExtendIndexCursor(const Txn &txn, const std::string &className, const Condition &condition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    edgeClassInfoExtend = txn._iSchema->getSubClassInfos(edgeClassInfo.id, edgeClassInfoExtend);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = Compare::compareConditionRdesc(txn, classInfo, propertyNameMapInfo, condition, true);
      resultSetExtend.addMetadata(resultSet);
    }
    return resultSetExtend;
  }

  ResultSetCursor
  Edge::getExtendIndexCursor(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid();

    auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
    auto edgeClassInfoExtend = std::map<std::string, schema::ClassAccessInfo>{};
    edgeClassInfoExtend = txn._iSchema->getSubClassInfos(edgeClassInfo.id, edgeClassInfoExtend);
    auto resultSetExtend = ResultSetCursor{txn};
    for (const auto &classNameMapInfo: edgeClassInfoExtend) {
      auto &classInfo = classNameMapInfo.second;
      auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto resultSet = Compare::compareMultiConditionRdesc(txn, classInfo, propertyNameMapInfo, multiCondition, true);
      resultSetExtend.addMetadata(resultSet);
    }
    return resultSetExtend;
  }

}
