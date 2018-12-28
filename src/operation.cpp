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

#include "constant.hpp"
#include "lmdb_engine.hpp"
#include "datarecord_adapter.hpp"
#include "index_adapter.hpp"
#include "schema_adapter.hpp"
#include "parser.hpp"
#include "compare.hpp"
#include "schema.hpp"
#include "index.hpp"
#include "relation.hpp"
#include "datarecord.hpp"
#include "algorithm.hpp"

#include "nogdb/nogdb.h"

namespace nogdb {

  const RecordDescriptor Transaction::addVertex(const std::string &className, const Record &record) {
    BEGIN_VALIDATION(this)
        .isTransactionValid()
        .isTransactionCompleted()
        .isClassNameValid(className);

    auto vertexClassInfo = _interface->schema()->getValidClassInfo(className, ClassType::VERTEX);
    auto propertyNameMapInfo = _interface->schema()->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
    auto recordBlob = parser::RecordParser::parseRecord(record, propertyNameMapInfo);
    try {
      auto vertexDataRecord = adapter::datarecord::DataRecord(_txnBase, vertexClassInfo.id, ClassType::VERTEX);
      auto positionId = vertexDataRecord.insert(recordBlob);
      auto recordDescriptor = RecordDescriptor{vertexClassInfo.id, positionId};
      auto indexInfos = _interface->index()->getIndexInfos(recordDescriptor, record, propertyNameMapInfo);
      _interface->index()->insert(recordDescriptor, record, indexInfos);
      return recordDescriptor;
    } catch (const Error& error) {
      rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  const RecordDescriptor Transaction::addEdge(const std::string &className,
                                              const RecordDescriptor &srcVertexRecordDescriptor,
                                              const RecordDescriptor &dstVertexRecordDescriptor,
                                              const Record &record) {
    BEGIN_VALIDATION(this)
        .isTransactionValid()
        .isTransactionCompleted()
        .isClassNameValid(className)
        .isExistingSrcVertex(srcVertexRecordDescriptor)
        .isExistingDstVertex(dstVertexRecordDescriptor);

    auto edgeClassInfo = _interface->schema()->getValidClassInfo(className, ClassType::EDGE);
    auto propertyNameMapInfo = _interface->schema()->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
    auto recordBlob = parser::RecordParser::parseRecord(record, propertyNameMapInfo);
    try {
      auto edgeDataRecord = adapter::datarecord::DataRecord(_txnBase, edgeClassInfo.id, ClassType::EDGE);
      auto vertexBlob = parser::RecordParser::parseEdgeVertexSrcDst(srcVertexRecordDescriptor.rid,
                                                                    dstVertexRecordDescriptor.rid);
      auto positionId = edgeDataRecord.insert(vertexBlob + recordBlob);
      auto recordDescriptor = RecordDescriptor{edgeClassInfo.id, positionId};
      _interface->graph()->addRel(recordDescriptor.rid, srcVertexRecordDescriptor.rid, dstVertexRecordDescriptor.rid);
      auto indexInfos = _interface->index()->getIndexInfos(recordDescriptor, record, propertyNameMapInfo);
      _interface->index()->insert(recordDescriptor, record, indexInfos);
      return recordDescriptor;
    } catch (const Error& error) {
      rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Transaction::update(const RecordDescriptor &recordDescriptor, const Record &record) {
    BEGIN_VALIDATION(this)
        .isTransactionValid()
        .isTransactionCompleted();

    auto classInfo = _interface->schema()->getExistingClass(recordDescriptor.rid.first);
    auto dataRecord = adapter::datarecord::DataRecord(_txnBase, classInfo.id, classInfo.type);
    auto recordResult = dataRecord.getResult(recordDescriptor.rid.second);
    auto propertyNameMapInfo = _interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
    auto newRecordBlob = parser::RecordParser::parseRecord(record, propertyNameMapInfo);
    try {
      auto propertyIdMapInfo = _interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
      auto existingRecord = parser::RecordParser::parseRawData(
          recordResult, propertyIdMapInfo, classInfo.type == ClassType::EDGE);

      // insert an updated record
      if (classInfo.type == ClassType::EDGE) {
        auto vertexBlob = parser::RecordParser::parseEdgeRawDataVertexSrcDstAsBlob(recordResult.data.blob());
        dataRecord.update(recordDescriptor.rid.second, vertexBlob + newRecordBlob);
      } else {
        dataRecord.update(recordDescriptor.rid.second, newRecordBlob);
      }

      // remove index if applied in existing record
      auto indexInfos = _interface->index()->getIndexInfos(recordDescriptor, record, propertyNameMapInfo);
      _interface->index()->remove(recordDescriptor, existingRecord, indexInfos);
      // add index if applied in new record
      _interface->index()->insert(recordDescriptor, record, indexInfos);
    } catch (const Error& error) {
      rollback();
      throw NOGDB_FATAL_ERROR(error);
    }

  }

  void Transaction::updateSrc(const RecordDescriptor &recordDescriptor,
                              const RecordDescriptor &newSrcVertexRecordDescriptor) {
    BEGIN_VALIDATION(this)
        .isTransactionValid()
        .isTransactionCompleted()
        .isExistingSrcVertex(newSrcVertexRecordDescriptor);

    auto edgeClassInfo = _interface->schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = adapter::datarecord::DataRecord(_txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto recordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
    try {
      auto srcDstVertex = parser::RecordParser::parseEdgeRawDataVertexSrcDst(recordResult.data.blob());
      _interface->graph()->updateSrcRel(
          recordDescriptor.rid, newSrcVertexRecordDescriptor.rid, srcDstVertex.first, srcDstVertex.second);
      auto newVertexBlob = parser::RecordParser::parseEdgeVertexSrcDst(
          newSrcVertexRecordDescriptor.rid, srcDstVertex.second);
      auto dataBlob = parser::RecordParser::parseEdgeRawDataAsBlob(recordResult.data.blob());
      edgeDataRecord.update(recordDescriptor.rid.second, newVertexBlob + dataBlob);
    } catch (const Error& error) {
      rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Transaction::updateDst(const RecordDescriptor &recordDescriptor,
                              const RecordDescriptor &newDstVertexRecordDescriptor) {
    BEGIN_VALIDATION(this)
        .isTransactionValid()
        .isTransactionCompleted()
        .isExistingDstVertex(newDstVertexRecordDescriptor);

    auto edgeClassInfo = _interface->schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = adapter::datarecord::DataRecord(_txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto recordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
    try {
      auto srcDstVertex = parser::RecordParser::parseEdgeRawDataVertexSrcDst(recordResult.data.blob());
      _interface->graph()->updateDstRel(
          recordDescriptor.rid, newDstVertexRecordDescriptor.rid, srcDstVertex.first, srcDstVertex.second);
      auto newVertexBlob = parser::RecordParser::parseEdgeVertexSrcDst(
          srcDstVertex.first, newDstVertexRecordDescriptor.rid);
      auto dataBlob = parser::RecordParser::parseEdgeRawDataAsBlob(recordResult.data.blob());
      edgeDataRecord.update(recordDescriptor.rid.second, newVertexBlob + dataBlob);
    } catch (const Error& error) {
      rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Transaction::remove(const RecordDescriptor &recordDescriptor) {
    BEGIN_VALIDATION(this)
        .isTransactionValid()
        .isTransactionCompleted();

    auto classInfo = _interface->schema()->getExistingClass(recordDescriptor.rid.first);
    auto dataRecord = adapter::datarecord::DataRecord(_txnBase, classInfo.id, classInfo.type);
    auto recordResult = dataRecord.getResult(recordDescriptor.rid.second);
    try {
      auto propertyNameMapInfo = _interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto propertyIdMapInfo = _interface->schema()->getPropertyIdMapInfo(classInfo.id, classInfo.superClassId);
      auto record = parser::RecordParser::parseRawData(
          recordResult, propertyIdMapInfo, classInfo.type == ClassType::EDGE);

      if (classInfo.type == ClassType::EDGE) {
        auto srcDstVertex = parser::RecordParser::parseEdgeRawDataVertexSrcDst(recordResult.data.blob());
        dataRecord.remove(recordDescriptor.rid.second);
        _interface->graph()->removeRelFromEdge(recordDescriptor.rid, srcDstVertex.first, srcDstVertex.second);
      } else {
        dataRecord.remove(recordDescriptor.rid.second);
        _interface->graph()->removeRelFromVertex(recordDescriptor.rid);
      }

      // remove index if applied in the record
      auto indexInfos = _interface->index()->getIndexInfos(recordDescriptor, record, propertyNameMapInfo);
      _interface->index()->remove(recordDescriptor, record, indexInfos);
    } catch (const Error& error) {
      rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  void Transaction::removeAll(const std::string &className) {
    BEGIN_VALIDATION(this)
        .isTransactionValid()
        .isTransactionCompleted()
        .isClassNameValid(className);

    auto classInfo = _interface->schema()->getExistingClass(className);
    try {
      auto dataRecord = adapter::datarecord::DataRecord(_txnBase, classInfo.id, ClassType::VERTEX);
      auto propertyNameMapInfo = _interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
      auto result = std::map<RecordId, std::pair<RecordId, RecordId>>{};
      std::function<void(const PositionId &, const storage_engine::lmdb::Result &)> callback =
          [&](const PositionId &positionId, const storage_engine::lmdb::Result &result) {
            auto recordId = RecordId{classInfo.id, positionId};
            if (classInfo.type == ClassType::EDGE) {
              auto srcDstVertex = parser::RecordParser::parseEdgeRawDataVertexSrcDst(result.data.blob());
              _interface->graph()->removeRelFromEdge(recordId, srcDstVertex.first, srcDstVertex.second);
            } else {
              _interface->graph()->removeRelFromVertex(recordId);
            }
          };
      dataRecord.resultSetIter(callback);
      dataRecord.destroy();
      _interface->index()->drop(classInfo.id, propertyNameMapInfo);
    } catch (const Error& error) {
      rollback();
      throw NOGDB_FATAL_ERROR(error);
    }
  }

  Result Transaction::fetchSrc(const RecordDescriptor &recordDescriptor) {
    BEGIN_VALIDATION(this)
        .isTransactionCompleted();

    auto edgeClassInfo = _interface->schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto srcDstVertex = _interface->graph()->getSrcDstVertices(recordDescriptor.rid);
    auto srcVertexRecordDescriptor = RecordDescriptor{srcDstVertex.first};
    auto srcVertexClassInfo = _interface->schema()->getExistingClass(srcVertexRecordDescriptor.rid.first);
    return Result{
        srcVertexRecordDescriptor,
        _interface->record()->getRecordWithBasicInfo(srcVertexClassInfo, srcVertexRecordDescriptor)
    };
  }

  Result Transaction::fetchDst(const RecordDescriptor &recordDescriptor) {
    BEGIN_VALIDATION(this)
        .isTransactionCompleted();

    auto edgeClassInfo = _interface->schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto srcDstVertex = _interface->graph()->getSrcDstVertices(recordDescriptor.rid);
    auto dstVertexRecordDescriptor = RecordDescriptor{srcDstVertex.second};
    auto dstVertexClassInfo = _interface->schema()->getExistingClass(dstVertexRecordDescriptor.rid.first);
    return Result{
        dstVertexRecordDescriptor,
        _interface->record()->getRecordWithBasicInfo(dstVertexClassInfo, dstVertexRecordDescriptor)
    };
  }

  ResultSet Transaction::fetchSrcDst(const RecordDescriptor &recordDescriptor) {
    BEGIN_VALIDATION(this)
        .isTransactionCompleted();

    auto edgeClassInfo = _interface->schema()->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
    auto srcDstVertex = _interface->graph()->getSrcDstVertices(recordDescriptor.rid);
    auto srcVertexRecordDescriptor = RecordDescriptor{srcDstVertex.first};
    auto srcVertexClassInfo = _interface->schema()->getExistingClass(srcVertexRecordDescriptor.rid.first);
    auto dstVertexRecordDescriptor = RecordDescriptor{srcDstVertex.second};
    auto dstVertexClassInfo = _interface->schema()->getExistingClass(dstVertexRecordDescriptor.rid.first);
    auto srcVertexResult = _interface->record()->getRecordWithBasicInfo(
        srcVertexClassInfo, srcVertexRecordDescriptor);
    auto dstVertexResult = _interface->record()->getRecordWithBasicInfo(
        dstVertexClassInfo, dstVertexRecordDescriptor);
    return ResultSet{
        Result{srcVertexRecordDescriptor, srcVertexResult},
        Result{dstVertexRecordDescriptor, dstVertexResult}
    };
  }

  FindOperationBuilder Transaction::find(const std::string &className) {
    return FindOperationBuilder(this, className, false);
  }

  FindOperationBuilder Transaction::findSubClassOf(const std::string &className) {
    return FindOperationBuilder(this, className, true);
  }

  FindEdgeOperationBuilder Transaction::findInEdge(const RecordDescriptor &recordDescriptor) {
    return FindEdgeOperationBuilder(this, recordDescriptor, OperationBuilder::EdgeDirection::IN);
  }

  FindEdgeOperationBuilder Transaction::findOutEdge(const RecordDescriptor &recordDescriptor) {
    return FindEdgeOperationBuilder(this, recordDescriptor, OperationBuilder::EdgeDirection::OUT);
  }

  FindEdgeOperationBuilder Transaction::findEdge(const RecordDescriptor &recordDescriptor) {
    return FindEdgeOperationBuilder(this, recordDescriptor, OperationBuilder::EdgeDirection::UNDIRECTED);
  }

  TraverseOperationBuilder Transaction::traverseIn(const RecordDescriptor &recordDescriptor) {
    return TraverseOperationBuilder(this, recordDescriptor, OperationBuilder::EdgeDirection::IN);
  }

  TraverseOperationBuilder Transaction::traverseOut(const RecordDescriptor &recordDescriptor) {
    return TraverseOperationBuilder(this, recordDescriptor, OperationBuilder::EdgeDirection::OUT);
  }

  TraverseOperationBuilder Transaction::traverse(const RecordDescriptor &recordDescriptor) {
    return TraverseOperationBuilder(this, recordDescriptor, OperationBuilder::EdgeDirection::UNDIRECTED);
  }

  ShortestPathOperationBuilder Transaction::shortestPath(const RecordDescriptor &srcVertexRecordDescriptor,
                                                         const RecordDescriptor &dstVertexRecordDescriptor) {
    return ShortestPathOperationBuilder(this, srcVertexRecordDescriptor, dstVertexRecordDescriptor);
  }

  ResultSet FindOperationBuilder::get() {
    BEGIN_VALIDATION(_txn)
        .isTransactionCompleted()
        .isClassNameValid(_className);

    auto classInfo = _txn->_interface->schema()->getExistingClass(_className);
    auto classInfoExtend = (_includeSubClassOf)?
        _txn->_interface->schema()->getSubClassInfos(classInfo.id): std::map<std::string, schema::ClassAccessInfo>{};
    switch (_conditionType) {
      case ConditionType::CONDITION: {
        auto propertyNameMapInfo =
            _txn->_interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
        auto resultSet = compare::RecordCompare::compareCondition(
            *_txn, classInfo, propertyNameMapInfo, *_condition, _indexed);
        for (const auto &classNameMapInfo: classInfoExtend) {
          auto &currentClassInfo = classNameMapInfo.second;
          auto currentPropertyInfo =
              _txn->_interface->schema()->getPropertyNameMapInfo(currentClassInfo.id, currentClassInfo.superClassId);
          auto resultSetExtend = compare::RecordCompare::compareCondition(
              *_txn, currentClassInfo, currentPropertyInfo, *_condition, _indexed);
          resultSet.insert(resultSet.cend(), resultSetExtend.cbegin(), resultSetExtend.cend());
        }
        return resultSet;
      }
      case ConditionType::MULTI_CONDITION: {
        auto propertyNameMapInfo =
            _txn->_interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
        auto resultSet = compare::RecordCompare::compareMultiCondition(
            *_txn, classInfo, propertyNameMapInfo, *_multiCondition, _indexed);
        for (const auto &classNameMapInfo: classInfoExtend) {
          auto &currentClassInfo = classNameMapInfo.second;
          auto currentPropertyInfo =
              _txn->_interface->schema()->getPropertyNameMapInfo(currentClassInfo.id, currentClassInfo.superClassId);
          auto resultSetExtend = compare::RecordCompare::compareMultiCondition(
              *_txn, currentClassInfo, currentPropertyInfo, *_multiCondition, _indexed);
          resultSet.insert(resultSet.cend(), resultSetExtend.cbegin(), resultSetExtend.cend());
        }
        return resultSet;
      }
      case ConditionType::COMPARE_FUNCTION: {
        auto propertyNameMapInfo =
            _txn->_interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
        auto resultSet = _txn->_interface->record()->getResultSetByCmpFunction(classInfo, _function);
        for (const auto &classNameMapInfo: classInfoExtend) {
          auto &currentClassInfo = classNameMapInfo.second;
          auto currentPropertyInfo =
              _txn->_interface->schema()->getPropertyNameMapInfo(currentClassInfo.id, currentClassInfo.superClassId);
          auto resultSetExtend = _txn->_interface->record()->getResultSetByCmpFunction(classInfo, _function);
          resultSet.insert(resultSet.cend(), resultSetExtend.cbegin(), resultSetExtend.cend());
        }
        return resultSet;
      }
      default: {
        auto resultSet = _txn->_interface->record()->getResultSet(classInfo);
        for (const auto &classNameMapInfo: classInfoExtend) {
          auto resultSetExtend = _txn->_interface->record()->getResultSet(classNameMapInfo.second);
          resultSet.insert(resultSet.cend(), resultSetExtend.cbegin(), resultSetExtend.cend());
        }
        return resultSet;
      }
    }
  };

  ResultSetCursor FindOperationBuilder::getCursor() {
    BEGIN_VALIDATION(_txn)
        .isTransactionCompleted()
        .isClassNameValid(_className);

    auto classInfo = _txn->_interface->schema()->getExistingClass(_className);
    auto classInfoExtend = (_includeSubClassOf)?
        _txn->_interface->schema()->getSubClassInfos(classInfo.id): std::map<std::string, schema::ClassAccessInfo>{};
    switch (_conditionType) {
      case ConditionType::CONDITION: {
        auto propertyNameMapInfo =
            _txn->_interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
        auto resultSetCursor = ResultSetCursor{*_txn};
        auto result = compare::RecordCompare::compareConditionRdesc(
            *_txn, classInfo, propertyNameMapInfo, *_condition, _indexed);
        resultSetCursor.addMetadata(result);
        for (const auto &classNameMapInfo: classInfoExtend) {
          auto &currentClassInfo = classNameMapInfo.second;
          auto currentPropertyInfo =
              _txn->_interface->schema()->getPropertyNameMapInfo(currentClassInfo.id, currentClassInfo.superClassId);
          auto resultSetExtend = compare::RecordCompare::compareConditionRdesc(
              *_txn, classInfo, currentPropertyInfo, *_condition, _indexed);
          resultSetCursor.addMetadata(resultSetExtend);
        }
        return resultSetCursor;
      }
      case ConditionType::MULTI_CONDITION: {
        auto propertyNameMapInfo =
            _txn->_interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
        auto resultSetCursor = ResultSetCursor{*_txn};
        auto result = compare::RecordCompare::compareMultiConditionRdesc(
            *_txn, classInfo, propertyNameMapInfo, *_multiCondition, _indexed);
        resultSetCursor.addMetadata(result);
        for (const auto &classNameMapInfo: classInfoExtend) {
          auto &currentClassInfo = classNameMapInfo.second;
          auto currentPropertyInfo =
              _txn->_interface->schema()->getPropertyNameMapInfo(currentClassInfo.id, currentClassInfo.superClassId);
          auto resultSetExtend = compare::RecordCompare::compareMultiConditionRdesc(
              *_txn, classInfo, currentPropertyInfo, *_multiCondition, _indexed);
          resultSetCursor.addMetadata(resultSetExtend);
        }
        return resultSetCursor;
      }
      case ConditionType::COMPARE_FUNCTION: {
        auto propertyNameMapInfo =
            _txn->_interface->schema()->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
        auto resultSetCursor = ResultSetCursor{*_txn};
        auto result = _txn->_interface->record()->getRecordDescriptorByCmpFunction(classInfo, _function);
        resultSetCursor.addMetadata(result);
        for (const auto &classNameMapInfo: classInfoExtend) {
          auto &currentClassInfo = classNameMapInfo.second;
          auto currentPropertyInfo =
              _txn->_interface->schema()->getPropertyNameMapInfo(currentClassInfo.id, currentClassInfo.superClassId);
          auto resultSetExtend =
              _txn->_interface->record()->getRecordDescriptorByCmpFunction(currentClassInfo, _function);
          resultSetCursor.addMetadata(resultSetExtend);
        }
        return resultSetCursor;
      }
      default: {
        auto resultSetCursor = _txn->_interface->record()->getResultSetCursor(classInfo);
        if (!_includeSubClassOf) {
          return resultSetCursor;
        } else {
          auto resultSetExtend = ResultSetCursor{*_txn};
          resultSetExtend.addMetadata(resultSetCursor);
          for (const auto &classNameMapInfo: classInfoExtend) {
            resultSetExtend.addMetadata(_txn->_interface->record()->getResultSetCursor(classNameMapInfo.second));
          }
          return resultSetExtend;
        }
      }
    }
  }

  ResultSet FindEdgeOperationBuilder::get() {
    BEGIN_VALIDATION(_txn)
        .isTransactionCompleted()
        .isExistingVertex(_rdesc);

    auto vertexClassInfo = _txn->_interface->schema()->getValidClassInfo(_rdesc.rid.first, ClassType::VERTEX);
    auto edgeRecordIds = std::vector<RecordId>{};
    switch (_direction) {
      case EdgeDirection::IN : {
        edgeRecordIds = _txn->_interface->graph()->getInEdges(_rdesc.rid);
        break;
      }
      case EdgeDirection::OUT : {
        edgeRecordIds = _txn->_interface->graph()->getOutEdges(_rdesc.rid);
        break;
      }
      default: {
        auto recordIds = std::set<RecordId>{};
        auto inEdgeRecordIds = _txn->_interface->graph()->getInEdges(_rdesc.rid);
        auto outEdgeRecordIds = _txn->_interface->graph()->getOutEdges(_rdesc.rid);
        recordIds.insert(inEdgeRecordIds.cbegin(), inEdgeRecordIds.cend());
        recordIds.insert(outEdgeRecordIds.cbegin(), outEdgeRecordIds.cend());
        edgeRecordIds.assign(recordIds.cbegin(), recordIds.cend());
        break;
      }
    }
    auto result = ResultSet{};
    auto classFilter = compare::RecordCompare::getFilterClasses(*_txn, _filter);
    for (const auto &recordId: edgeRecordIds) {
      auto edgeRecordDescriptor = RecordDescriptor{recordId};
      auto filterResult = compare::RecordCompare::filterResult(*_txn, edgeRecordDescriptor, _filter, classFilter);
      if (filterResult.descriptor != RecordDescriptor{}) {
        result.emplace_back(filterResult);
      }
    }
    return result;
  }

  ResultSetCursor FindEdgeOperationBuilder::getCursor() {
    BEGIN_VALIDATION(_txn)
        .isTransactionCompleted()
        .isExistingVertex(_rdesc);

    auto vertexClassInfo = _txn->_interface->schema()->getValidClassInfo(_rdesc.rid.first, ClassType::VERTEX);
    auto edgeRecordIds = std::vector<RecordId>{};
    switch (_direction) {
      case EdgeDirection::IN : {
        edgeRecordIds = _txn->_interface->graph()->getInEdges(_rdesc.rid);
        break;
      }
      case EdgeDirection::OUT : {
        edgeRecordIds = _txn->_interface->graph()->getOutEdges(_rdesc.rid);
        break;
      }
      default: {
        auto recordIds = std::set<RecordId>{};
        auto inEdgeRecordIds = _txn->_interface->graph()->getInEdges(_rdesc.rid);
        auto outEdgeRecordIds = _txn->_interface->graph()->getOutEdges(_rdesc.rid);
        recordIds.insert(inEdgeRecordIds.cbegin(), inEdgeRecordIds.cend());
        recordIds.insert(outEdgeRecordIds.cbegin(), outEdgeRecordIds.cend());
        edgeRecordIds.assign(recordIds.cbegin(), recordIds.cend());
        break;
      }
    }
    auto result = ResultSetCursor{*_txn};
    auto classFilter = compare::RecordCompare::getFilterClasses(*_txn, _filter);
    for (const auto &recordId: edgeRecordIds) {
      auto edgeRecordDescriptor = RecordDescriptor{recordId};
      auto filterRecord = compare::RecordCompare::filterRecord(*_txn, edgeRecordDescriptor, _filter, classFilter);
      if (filterRecord != RecordDescriptor{}) {
        result.addMetadata(filterRecord);
      }
    }
    return result;
  }

  ResultSet TraverseOperationBuilder::get() {
    BEGIN_VALIDATION(_txn)
        .isTransactionCompleted()
        .isExistingVertex(_rdesc);

    auto vertexClassInfo = _txn->_interface->schema()->getValidClassInfo(_rdesc.rid.first, ClassType::VERTEX);
    auto direction = adapter::relation::Direction::ALL;
    switch (_direction) {
      case EdgeDirection::IN : direction = adapter::relation::Direction::IN; break;
      case EdgeDirection::OUT : direction = adapter::relation::Direction::OUT; break;
      default: break;
    }

    return algorithm::GraphTraversal::breadthFirstSearch(
        *_txn, vertexClassInfo, _rdesc, _minDepth, _maxDepth, direction, _edgeFilter, _vertexFilter);
  }

  ResultSetCursor TraverseOperationBuilder::getCursor() {
    BEGIN_VALIDATION(_txn)
        .isTransactionCompleted()
        .isExistingVertex(_rdesc);

    auto vertexClassInfo = _txn->_interface->schema()->getValidClassInfo(_rdesc.rid.first, ClassType::VERTEX);
    auto direction = adapter::relation::Direction::ALL;
    switch (_direction) {
      case EdgeDirection::IN : direction = adapter::relation::Direction::IN; break;
      case EdgeDirection::OUT : direction = adapter::relation::Direction::OUT; break;
      default: break;
    }

    auto result = algorithm::GraphTraversal::breadthFirstSearchRdesc(
        *_txn, vertexClassInfo, _rdesc, _minDepth, _maxDepth, direction, _edgeFilter, _vertexFilter);
    return std::move(ResultSetCursor{*_txn}.addMetadata(result));
  }

  ResultSet ShortestPathOperationBuilder::get() {
    BEGIN_VALIDATION(_txn)
        .isTransactionCompleted()
        .isExistingSrcVertex(_srcRdesc)
        .isExistingDstVertex(_dstRdesc);

    auto srcVertexClassInfo = _txn->_interface->schema()->getValidClassInfo(_srcRdesc.rid.first, ClassType::VERTEX);
    auto dstVertexClassInfo = _txn->_interface->schema()->getValidClassInfo(_dstRdesc.rid.first, ClassType::VERTEX);
    return algorithm::GraphTraversal::bfsShortestPath(
        *_txn, srcVertexClassInfo, dstVertexClassInfo, _srcRdesc, _dstRdesc, _edgeFilter, _vertexFilter);
  }

  ResultSetCursor ShortestPathOperationBuilder::getCursor() {
    BEGIN_VALIDATION(_txn)
        .isTransactionCompleted()
        .isExistingSrcVertex(_srcRdesc)
        .isExistingDstVertex(_dstRdesc);

    auto srcVertexClassInfo = _txn->_interface->schema()->getValidClassInfo(_srcRdesc.rid.first, ClassType::VERTEX);
    auto dstVertexClassInfo = _txn->_interface->schema()->getValidClassInfo(_dstRdesc.rid.first, ClassType::VERTEX);
    auto result = algorithm::GraphTraversal::bfsShortestPathRdesc(
        *_txn, srcVertexClassInfo, dstVertexClassInfo, _srcRdesc, _dstRdesc, _edgeFilter, _vertexFilter);
    return std::move(ResultSetCursor{*_txn}.addMetadata(result));
  }


}
