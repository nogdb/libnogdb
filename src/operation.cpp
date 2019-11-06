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

#include "algorithm.hpp"
#include "compare.hpp"
#include "constant.hpp"
#include "datarecord.hpp"
#include "datarecord_adapter.hpp"
#include "index.hpp"
#include "index_adapter.hpp"
#include "lmdb_engine.hpp"
#include "parser.hpp"
#include "relation.hpp"
#include "schema.hpp"
#include "schema_adapter.hpp"

#include "nogdb/nogdb.h"

namespace nogdb {
using namespace adapter::datarecord;
using namespace adapter::schema;
using namespace schema;
using namespace datarecord;
using namespace index;
using compare::RecordCompare;
using parser::RecordParser;

const RecordDescriptor Transaction::addVertex(const std::string& className, const Record& record)
{
    BEGIN_VALIDATION(this)
        .isTxnValid()
        .isTxnCompleted()
        .isClassNameValid(className);

    auto vertexClassInfo = SchemaUtils::getValidClassInfo(this, className, ClassType::VERTEX);
    auto propertyNameMapInfo = SchemaUtils::getPropertyNameMapInfo(
        this, vertexClassInfo.id, vertexClassInfo.superClassId);
    auto recordBlob = RecordParser::parseRecord(record, propertyNameMapInfo);
    try {
        auto vertexDataRecord = DataRecord(_txnBase, vertexClassInfo.id, ClassType::VERTEX);
        auto positionId = PositionId { 0 };
        if (_txnCtx->isVersionEnabled()) {
            auto newRecordBlob = RecordParser::parseVertexRecordWithVersion(recordBlob, VersionId { 1 });
            positionId = vertexDataRecord.insert(newRecordBlob);
            _updatedRecords.insert(RecordId { vertexClassInfo.id, positionId });
        } else {
            positionId = vertexDataRecord.insert(recordBlob);
        }
        auto recordDescriptor = RecordDescriptor { vertexClassInfo.id, positionId };
        auto indexInfos = IndexUtils::getIndexInfos(this, recordDescriptor, record, propertyNameMapInfo);
        IndexUtils::insert(this, recordDescriptor, record, indexInfos);
        return recordDescriptor;
    } catch (const Error& error) {
        rollback();
        throw NOGDB_FATAL_ERROR(error);
    }
}

const RecordDescriptor Transaction::addEdge(const std::string& className,
    const RecordDescriptor& srcVertexRecordDescriptor,
    const RecordDescriptor& dstVertexRecordDescriptor,
    const Record& record)
{
    BEGIN_VALIDATION(this)
        .isTxnValid()
        .isTxnCompleted()
        .isClassNameValid(className)
        .isExistingSrcVertex(srcVertexRecordDescriptor)
        .isExistingDstVertex(dstVertexRecordDescriptor);

    auto edgeClassInfo = SchemaUtils::getValidClassInfo(this, className, ClassType::EDGE);
    auto propertyNameMapInfo = SchemaUtils::getPropertyNameMapInfo(
        this, edgeClassInfo.id, edgeClassInfo.superClassId);
    auto recordBlob = RecordParser::parseRecord(record, propertyNameMapInfo);
    try {
        auto edgeDataRecord = DataRecord(_txnBase, edgeClassInfo.id, ClassType::EDGE);
        auto vertexBlob = RecordParser::parseEdgeVertexSrcDst(
            srcVertexRecordDescriptor.rid, dstVertexRecordDescriptor.rid);
        auto positionId = PositionId { 0 };
        if (_txnCtx->isVersionEnabled()) {
            auto newRecordBlob = RecordParser::parseEdgeRecordWithVersion(vertexBlob, recordBlob, VersionId { 1 });
            positionId = edgeDataRecord.insert(newRecordBlob);
            _updatedRecords.insert(RecordId { edgeClassInfo.id, positionId });
        } else {
            positionId = edgeDataRecord.insert(vertexBlob + recordBlob);
        }
        auto recordDescriptor = RecordDescriptor { edgeClassInfo.id, positionId };
        _graph->addRel(recordDescriptor.rid, srcVertexRecordDescriptor.rid, dstVertexRecordDescriptor.rid);
        auto indexInfos = IndexUtils::getIndexInfos(this, recordDescriptor, record, propertyNameMapInfo);
        IndexUtils::insert(this, recordDescriptor, record, indexInfos);
        return recordDescriptor;
    } catch (const Error& error) {
        rollback();
        throw NOGDB_FATAL_ERROR(error);
    }
}

void Transaction::update(const RecordDescriptor& recordDescriptor, const Record& record)
{
    BEGIN_VALIDATION(this)
        .isTxnValid()
        .isTxnCompleted();

    auto classInfo = SchemaUtils::getExistingClass(this, recordDescriptor.rid.first);
    auto dataRecord = DataRecord(_txnBase, classInfo.id, classInfo.type);
    auto recordResult = dataRecord.getResult(recordDescriptor.rid.second);
    auto propertyNameMapInfo = SchemaUtils::getPropertyNameMapInfo(this, classInfo.id, classInfo.superClassId);
    auto newRecordBlob = RecordParser::parseRecord(record, propertyNameMapInfo);
    try {
        auto propertyIdMapInfo = SchemaUtils::getPropertyIdMapInfo(this, classInfo.id, classInfo.superClassId);
        auto existingRecord = RecordParser::parseRawData(
            recordResult, propertyIdMapInfo, classInfo.type == ClassType::EDGE, _txnCtx->isVersionEnabled());

        // insert an updated record
        auto updateRecordBlob = Blob {};
        if (_txnCtx->isVersionEnabled()) {
            if (_updatedRecords.find(recordDescriptor.rid) == _updatedRecords.cend()) {
                auto versionId = RecordParser::parseRawDataVersionId(recordResult);
                if (classInfo.type == ClassType::EDGE) {
                    auto vertexBlob = RecordParser::parseEdgeRawDataVertexSrcDstAsBlob(
                        recordResult, _txnCtx->isVersionEnabled());
                    updateRecordBlob = RecordParser::parseEdgeRecordWithVersion(
                        vertexBlob, newRecordBlob, versionId + 1);
                } else {
                    updateRecordBlob = RecordParser::parseVertexRecordWithVersion(newRecordBlob, versionId + 1);
                }
                _updatedRecords.insert(recordDescriptor.rid);
            } else {
                updateRecordBlob = RecordParser::parseOnlyUpdateRecord(
                    recordResult, newRecordBlob, classInfo.type == ClassType::EDGE, true);
            }
        } else {
            updateRecordBlob = RecordParser::parseOnlyUpdateRecord(
                recordResult, newRecordBlob, classInfo.type == ClassType::EDGE, false);
        }
        dataRecord.update(recordDescriptor.rid.second, updateRecordBlob);

        // remove index if applied in existing record
        auto indexInfos = IndexUtils::getIndexInfos(this, recordDescriptor, record, propertyNameMapInfo);
        IndexUtils::remove(this, recordDescriptor, existingRecord, indexInfos);
        // add index if applied in new record
        IndexUtils::insert(this, recordDescriptor, record, indexInfos);
    } catch (const Error& error) {
        rollback();
        throw NOGDB_FATAL_ERROR(error);
    }
}

void Transaction::updateSrc(const RecordDescriptor& recordDescriptor,
    const RecordDescriptor& newSrcVertexRecordDescriptor)
{
    BEGIN_VALIDATION(this)
        .isTxnValid()
        .isTxnCompleted()
        .isExistingSrcVertex(newSrcVertexRecordDescriptor);

    auto edgeClassInfo = SchemaUtils::getValidClassInfo(this, recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = DataRecord(_txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto recordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
    try {
        auto srcDstVertex = RecordParser::parseEdgeRawDataVertexSrcDst(recordResult, _txnCtx->isVersionEnabled());
        _graph->updateSrcRel(
            recordDescriptor.rid, newSrcVertexRecordDescriptor.rid, srcDstVertex.first, srcDstVertex.second);
        auto newVertexBlob = RecordParser::parseEdgeVertexSrcDst(
            newSrcVertexRecordDescriptor.rid, srcDstVertex.second);
        auto updateEdgeRecordBlob = RecordParser::parseOnlyUpdateSrcVertex(
            recordResult, newSrcVertexRecordDescriptor.rid, _txnCtx->isVersionEnabled());
        if (_txnCtx->isVersionEnabled()) {
            // update version of old src vertex
            if (_updatedRecords.find(srcDstVertex.first) == _updatedRecords.cend()) {
                auto oldSrcVertexDataRecord = DataRecord(_txnBase, srcDstVertex.first.first, ClassType::VERTEX);
                auto oldSrcVertexRecordResult = oldSrcVertexDataRecord.getResult(srcDstVertex.first.second);
                auto versionId = RecordParser::parseRawDataVersionId(oldSrcVertexRecordResult);
                auto updateRecordBlob = RecordParser::parseOnlyUpdateVersion(oldSrcVertexRecordResult, versionId + 1);
                oldSrcVertexDataRecord.update(srcDstVertex.first.second, updateRecordBlob);
                _updatedRecords.insert(srcDstVertex.first);
            }
            // update version of new src vertex
            if (_updatedRecords.find(newSrcVertexRecordDescriptor.rid) == _updatedRecords.cend()) {
                auto newSrcVertexDataRecord = DataRecord(
                    _txnBase, newSrcVertexRecordDescriptor.rid.first, ClassType::VERTEX);
                auto newSrcVertexRecordResult = newSrcVertexDataRecord.getResult(newSrcVertexRecordDescriptor.rid.second);
                auto versionId = RecordParser::parseRawDataVersionId(newSrcVertexRecordResult);
                auto updateRecordBlob = RecordParser::parseOnlyUpdateVersion(newSrcVertexRecordResult, versionId + 1);
                newSrcVertexDataRecord.update(newSrcVertexRecordDescriptor.rid.second, updateRecordBlob);
                _updatedRecords.insert(newSrcVertexRecordDescriptor.rid);
            }
            // update version of edge
            if (_updatedRecords.find(recordDescriptor.rid) == _updatedRecords.cend()) {
                auto edgeVersionId = RecordParser::parseRawDataVersionId(recordResult);
                RecordParser::parseOnlyUpdateVersion(updateEdgeRecordBlob, edgeVersionId + 1);
                _updatedRecords.insert(recordDescriptor.rid);
            }
        }
        edgeDataRecord.update(recordDescriptor.rid.second, updateEdgeRecordBlob);
    } catch (const Error& error) {
        rollback();
        throw NOGDB_FATAL_ERROR(error);
    }
}

void Transaction::updateDst(const RecordDescriptor& recordDescriptor,
    const RecordDescriptor& newDstVertexRecordDescriptor)
{
    BEGIN_VALIDATION(this)
        .isTxnValid()
        .isTxnCompleted()
        .isExistingDstVertex(newDstVertexRecordDescriptor);

    auto edgeClassInfo = SchemaUtils::getValidClassInfo(this, recordDescriptor.rid.first, ClassType::EDGE);
    auto edgeDataRecord = DataRecord(_txnBase, edgeClassInfo.id, ClassType::EDGE);
    auto recordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
    try {
        auto srcDstVertex = RecordParser::parseEdgeRawDataVertexSrcDst(recordResult, _txnCtx->isVersionEnabled());
        _graph->updateDstRel(
            recordDescriptor.rid, newDstVertexRecordDescriptor.rid, srcDstVertex.first, srcDstVertex.second);
        auto newVertexBlob = RecordParser::parseEdgeVertexSrcDst(
            srcDstVertex.first, newDstVertexRecordDescriptor.rid);
        auto updateEdgeRecordBlob = RecordParser::parseOnlyUpdateDstVertex(
            recordResult, newDstVertexRecordDescriptor.rid, _txnCtx->isVersionEnabled());
        if (_txnCtx->isVersionEnabled()) {
            // update version of old dst vertex
            if (_updatedRecords.find(srcDstVertex.second) == _updatedRecords.cend()) {
                auto oldDstVertexDataRecord = DataRecord(_txnBase, srcDstVertex.second.first, ClassType::VERTEX);
                auto oldDstVertexRecordResult = oldDstVertexDataRecord.getResult(srcDstVertex.second.second);
                auto versionId = RecordParser::parseRawDataVersionId(oldDstVertexRecordResult);
                auto updateRecordBlob = RecordParser::parseOnlyUpdateVersion(oldDstVertexRecordResult, versionId + 1);
                oldDstVertexDataRecord.update(srcDstVertex.second.second, updateRecordBlob);
                _updatedRecords.insert(srcDstVertex.second);
            }
            // update version of new dst vertex
            if (_updatedRecords.find(newDstVertexRecordDescriptor.rid) == _updatedRecords.cend()) {
                auto newDstVertexDataRecord = DataRecord(
                    _txnBase, newDstVertexRecordDescriptor.rid.first, ClassType::VERTEX);
                auto newDstVertexRecordResult = newDstVertexDataRecord.getResult(newDstVertexRecordDescriptor.rid.second);
                auto versionId = RecordParser::parseRawDataVersionId(newDstVertexRecordResult);
                auto updateRecordBlob = RecordParser::parseOnlyUpdateVersion(newDstVertexRecordResult, versionId + 1);
                newDstVertexDataRecord.update(newDstVertexRecordDescriptor.rid.second, updateRecordBlob);
                _updatedRecords.insert(newDstVertexRecordDescriptor.rid);
            }
            // update version of edge
            if (_updatedRecords.find(recordDescriptor.rid) == _updatedRecords.cend()) {
                auto edgeVersionId = RecordParser::parseRawDataVersionId(recordResult);
                RecordParser::parseOnlyUpdateVersion(updateEdgeRecordBlob, edgeVersionId + 1);
                _updatedRecords.insert(recordDescriptor.rid);
            }
        }
        edgeDataRecord.update(recordDescriptor.rid.second, updateEdgeRecordBlob);
    } catch (const Error& error) {
        rollback();
        throw NOGDB_FATAL_ERROR(error);
    }
}

void Transaction::remove(const RecordDescriptor& recordDescriptor)
{
    BEGIN_VALIDATION(this)
        .isTxnValid()
        .isTxnCompleted();

    auto classInfo = SchemaUtils::getExistingClass(this, recordDescriptor.rid.first);
    auto dataRecord = DataRecord(_txnBase, classInfo.id, classInfo.type);
    auto recordResult = dataRecord.getResult(recordDescriptor.rid.second);
    try {
        auto propertyNameMapInfo = SchemaUtils::getPropertyNameMapInfo(this, classInfo.id, classInfo.superClassId);
        auto propertyIdMapInfo = SchemaUtils::getPropertyIdMapInfo(this, classInfo.id, classInfo.superClassId);
        auto record = RecordParser::parseRawData(
            recordResult, propertyIdMapInfo, classInfo.type == ClassType::EDGE, _txnCtx->isVersionEnabled());

        if (classInfo.type == ClassType::EDGE) {
            auto srcDstVertex = RecordParser::parseEdgeRawDataVertexSrcDst(
                recordResult, _txnCtx->isVersionEnabled());
            _graph->removeRelFromEdge(recordDescriptor.rid, srcDstVertex.first, srcDstVertex.second);
            if (_txnCtx->isVersionEnabled()) {
                // update version of src vertex
                if (_updatedRecords.find(srcDstVertex.first) == _updatedRecords.cend()) {
                    auto srcVertexDataRecord = DataRecord(
                        _txnBase, srcDstVertex.first.first, ClassType::VERTEX);
                    auto srcVertexRecordResult = srcVertexDataRecord.getResult(srcDstVertex.first.second);
                    auto versionId = RecordParser::parseRawDataVersionId(srcVertexRecordResult);
                    auto updateRecordBlob = RecordParser::parseOnlyUpdateVersion(srcVertexRecordResult, versionId + 1);
                    srcVertexDataRecord.update(srcDstVertex.first.second, updateRecordBlob);
                    _updatedRecords.insert(srcDstVertex.first);
                }
                // update version of dst vertex
                if (_updatedRecords.find(srcDstVertex.second) == _updatedRecords.cend()) {
                    auto dstVertexDataRecord = DataRecord(
                        _txnBase, srcDstVertex.second.first, ClassType::VERTEX);
                    auto dstVertexRecordResult = dstVertexDataRecord.getResult(srcDstVertex.second.second);
                    auto versionId = RecordParser::parseRawDataVersionId(dstVertexRecordResult);
                    auto updateRecordBlob = RecordParser::parseOnlyUpdateVersion(dstVertexRecordResult, versionId + 1);
                    dstVertexDataRecord.update(srcDstVertex.second.second, updateRecordBlob);
                    _updatedRecords.insert(srcDstVertex.second);
                }
            }
        } else {
            auto neighbours = _graph->removeRelFromVertex(recordDescriptor.rid);
            if (_txnCtx->isVersionEnabled()) {
                for (const auto& neighbour : neighbours) {
                    if (_updatedRecords.find(neighbour) == _updatedRecords.cend()) {
                        auto neighbourDataRecord = DataRecord(_txnBase, neighbour.first, ClassType::VERTEX);
                        auto neighbourRecordResult = neighbourDataRecord.getResult(neighbour.second);
                        auto versionId = RecordParser::parseRawDataVersionId(neighbourRecordResult);
                        auto updateRecordBlob = RecordParser::parseOnlyUpdateVersion(neighbourRecordResult, versionId + 1);
                        neighbourDataRecord.update(neighbour.second, updateRecordBlob);
                        _updatedRecords.insert(neighbour);
                    }
                }
            }
        }
        dataRecord.remove(recordDescriptor.rid.second);

        // remove index if applied in the record
        auto indexInfos = IndexUtils::getIndexInfos(this, recordDescriptor, record, propertyNameMapInfo);
        IndexUtils::remove(this, recordDescriptor, record, indexInfos);
    } catch (const Error& error) {
        rollback();
        throw NOGDB_FATAL_ERROR(error);
    }
}

void Transaction::removeAll(const std::string& className)
{
    BEGIN_VALIDATION(this)
        .isTxnValid()
        .isTxnCompleted()
        .isClassNameValid(className);

    auto classInfo = SchemaUtils::getExistingClass(this, className);
    try {
        auto dataRecord = DataRecord(_txnBase, classInfo.id, ClassType::VERTEX);
        auto propertyNameMapInfo = SchemaUtils::getPropertyNameMapInfo(this, classInfo.id, classInfo.superClassId);
        auto result = std::map<RecordId, std::pair<RecordId, RecordId>> {};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto recordId = RecordId { classInfo.id, positionId };
                if (classInfo.type == ClassType::EDGE) {
                    auto srcDstVertex = RecordParser::parseEdgeRawDataVertexSrcDst(
                        result, _txnCtx->isVersionEnabled());
                    _graph->removeRelFromEdge(recordId, srcDstVertex.first, srcDstVertex.second);
                    if (_txnCtx->isVersionEnabled()) {
                        // update version of src vertex
                        if (_updatedRecords.find(srcDstVertex.first) == _updatedRecords.cend()) {
                            auto srcVertexDataRecord = DataRecord(
                                _txnBase, srcDstVertex.first.first, ClassType::VERTEX);
                            auto srcVertexRecordResult = srcVertexDataRecord.getResult(srcDstVertex.first.second);
                            auto versionId = RecordParser::parseRawDataVersionId(srcVertexRecordResult);
                            auto updateRecordBlob = RecordParser::parseOnlyUpdateVersion(
                                srcVertexRecordResult, versionId + 1);
                            srcVertexDataRecord.update(srcDstVertex.first.second, updateRecordBlob);
                            _updatedRecords.insert(srcDstVertex.first);
                        }
                        // update version of dst vertex
                        if (_updatedRecords.find(srcDstVertex.second) == _updatedRecords.cend()) {
                            auto dstVertexDataRecord = DataRecord(
                                _txnBase, srcDstVertex.second.first, ClassType::VERTEX);
                            auto dstVertexRecordResult = dstVertexDataRecord.getResult(srcDstVertex.second.second);
                            auto versionId = RecordParser::parseRawDataVersionId(dstVertexRecordResult);
                            auto updateRecordBlob = RecordParser::parseOnlyUpdateVersion(
                                dstVertexRecordResult, versionId + 1);
                            dstVertexDataRecord.update(srcDstVertex.second.second, updateRecordBlob);
                            _updatedRecords.insert(srcDstVertex.second);
                        }
                    }
                } else {
                    auto neighbours = _graph->removeRelFromVertex(recordId);
                    if (_txnCtx->isVersionEnabled()) {
                        for (const auto& neighbour : neighbours) {
                            if (_updatedRecords.find(neighbour) == _updatedRecords.cend()) {
                                auto neighbourDataRecord = DataRecord(
                                    _txnBase, neighbour.first, ClassType::VERTEX);
                                auto neighbourRecordResult = neighbourDataRecord.getResult(neighbour.second);
                                auto versionId = RecordParser::parseRawDataVersionId(neighbourRecordResult);
                                auto updateRecordBlob = RecordParser::parseOnlyUpdateVersion(
                                    neighbourRecordResult, versionId + 1);
                                neighbourDataRecord.update(neighbour.second, updateRecordBlob);
                                _updatedRecords.insert(neighbour);
                            }
                        }
                    }
                }
            };
        dataRecord.resultSetIter(callback);
        dataRecord.destroy();

        // drop indexes
        IndexUtils::drop(this, classInfo.id, propertyNameMapInfo);
    } catch (const Error& error) {
        rollback();
        throw NOGDB_FATAL_ERROR(error);
    }
}

Result Transaction::fetchSrc(const RecordDescriptor& recordDescriptor) const
{
    BEGIN_VALIDATION(this)
        .isTxnCompleted();

    auto edgeClassInfo = SchemaUtils::getValidClassInfo(this, recordDescriptor.rid.first, ClassType::EDGE);
    auto srcDstVertex = _graph->getSrcDstVertices(recordDescriptor.rid);
    auto srcVertexRecordDescriptor = RecordDescriptor { srcDstVertex.first };
    auto srcVertexClassInfo = SchemaUtils::getExistingClass(this, srcVertexRecordDescriptor.rid.first);
    return Result {
        srcVertexRecordDescriptor,
        DataRecordUtils::getRecordWithBasicInfo(this, srcVertexClassInfo, srcVertexRecordDescriptor)
    };
}

Result Transaction::fetchDst(const RecordDescriptor& recordDescriptor) const
{
    BEGIN_VALIDATION(this)
        .isTxnCompleted();

    auto edgeClassInfo = SchemaUtils::getValidClassInfo(this, recordDescriptor.rid.first, ClassType::EDGE);
    auto srcDstVertex = _graph->getSrcDstVertices(recordDescriptor.rid);
    auto dstVertexRecordDescriptor = RecordDescriptor { srcDstVertex.second };
    auto dstVertexClassInfo = SchemaUtils::getExistingClass(this, dstVertexRecordDescriptor.rid.first);
    return Result {
        dstVertexRecordDescriptor,
        DataRecordUtils::getRecordWithBasicInfo(this, dstVertexClassInfo, dstVertexRecordDescriptor)
    };
}

ResultSet Transaction::fetchSrcDst(const RecordDescriptor& recordDescriptor) const
{
    BEGIN_VALIDATION(this)
        .isTxnCompleted();

    auto edgeClassInfo = SchemaUtils::getValidClassInfo(this, recordDescriptor.rid.first, ClassType::EDGE);
    auto srcDstVertex = _graph->getSrcDstVertices(recordDescriptor.rid);
    auto srcVertexRecordDescriptor = RecordDescriptor { srcDstVertex.first };
    auto srcVertexClassInfo = SchemaUtils::getExistingClass(this, srcVertexRecordDescriptor.rid.first);
    auto dstVertexRecordDescriptor = RecordDescriptor { srcDstVertex.second };
    auto dstVertexClassInfo = SchemaUtils::getExistingClass(this, dstVertexRecordDescriptor.rid.first);
    auto srcVertexResult = DataRecordUtils::getRecordWithBasicInfo(
        this, srcVertexClassInfo, srcVertexRecordDescriptor);
    auto dstVertexResult = DataRecordUtils::getRecordWithBasicInfo(
        this, dstVertexClassInfo, dstVertexRecordDescriptor);
    return ResultSet {
        Result { srcVertexRecordDescriptor, srcVertexResult },
        Result { dstVertexRecordDescriptor, dstVertexResult }
    };
}

FindOperationBuilder Transaction::find(const std::string& className) const
{
    return FindOperationBuilder(this, className, false);
}

FindOperationBuilder Transaction::findSubClassOf(const std::string& className) const
{
    return FindOperationBuilder(this, className, true);
}

FindEdgeOperationBuilder Transaction::findInEdge(const RecordDescriptor& recordDescriptor) const
{
    return FindEdgeOperationBuilder(this, recordDescriptor, OperationBuilder::EdgeDirection::IN);
}

FindEdgeOperationBuilder Transaction::findOutEdge(const RecordDescriptor& recordDescriptor) const
{
    return FindEdgeOperationBuilder(this, recordDescriptor, OperationBuilder::EdgeDirection::OUT);
}

FindEdgeOperationBuilder Transaction::findEdge(const RecordDescriptor& recordDescriptor) const
{
    return FindEdgeOperationBuilder(this, recordDescriptor, OperationBuilder::EdgeDirection::UNDIRECTED);
}

TraverseOperationBuilder Transaction::traverseIn(const RecordDescriptor& recordDescriptor) const
{
    return TraverseOperationBuilder(this, recordDescriptor, OperationBuilder::EdgeDirection::IN);
}

TraverseOperationBuilder Transaction::traverseOut(const RecordDescriptor& recordDescriptor) const
{
    return TraverseOperationBuilder(this, recordDescriptor, OperationBuilder::EdgeDirection::OUT);
}

TraverseOperationBuilder Transaction::traverse(const RecordDescriptor& recordDescriptor) const
{
    return TraverseOperationBuilder(this, recordDescriptor, OperationBuilder::EdgeDirection::UNDIRECTED);
}

ShortestPathOperationBuilder Transaction::shortestPath(const RecordDescriptor& srcVertexRecordDescriptor,
    const RecordDescriptor& dstVertexRecordDescriptor) const
{
    return ShortestPathOperationBuilder(this, srcVertexRecordDescriptor, dstVertexRecordDescriptor);
}

ResultSet FindOperationBuilder::get() const
{
    BEGIN_VALIDATION(_txn)
        .isTxnCompleted()
        .isClassNameValid(_className);

    auto classInfo = SchemaUtils::getExistingClass(_txn, _className);
    auto classInfoExtend = (_includeSubClassOf) ?
        SchemaUtils::getSubClassInfos(_txn, classInfo.id) : std::map<std::string, ClassAccessInfo> {};
    switch (_conditionType) {
    case ConditionType::CONDITION: {
        auto propertyNameMapInfo = SchemaUtils::getPropertyNameMapInfo(_txn, classInfo.id, classInfo.superClassId);
        auto resultSet = RecordCompare::compareCondition(*_txn, classInfo, propertyNameMapInfo, *_condition, _indexed);
        for (const auto& classNameMapInfo : classInfoExtend) {
            auto& currentClassInfo = classNameMapInfo.second;
            auto currentPropertyInfo =
                SchemaUtils::getPropertyNameMapInfo(_txn, currentClassInfo.id, currentClassInfo.superClassId);
            auto resultSetExtend = RecordCompare::compareCondition(
                *_txn, currentClassInfo, currentPropertyInfo, *_condition, _indexed);
            resultSet.insert(resultSet.cend(), resultSetExtend.cbegin(), resultSetExtend.cend());
        }
        return resultSet;
    }
    case ConditionType::MULTI_CONDITION: {
        auto propertyNameMapInfo = SchemaUtils::getPropertyNameMapInfo(_txn, classInfo.id, classInfo.superClassId);
        auto resultSet = RecordCompare::compareMultiCondition(
            *_txn, classInfo, propertyNameMapInfo, *_multiCondition, _indexed);
        for (const auto& classNameMapInfo : classInfoExtend) {
            auto& currentClassInfo = classNameMapInfo.second;
            auto currentPropertyInfo =
                SchemaUtils::getPropertyNameMapInfo(_txn, currentClassInfo.id, currentClassInfo.superClassId);
            auto resultSetExtend = RecordCompare::compareMultiCondition(
                *_txn, currentClassInfo, currentPropertyInfo, *_multiCondition, _indexed);
            resultSet.insert(resultSet.cend(), resultSetExtend.cbegin(), resultSetExtend.cend());
        }
        return resultSet;
    }
    case ConditionType::COMPARE_FUNCTION: {
        auto propertyNameMapInfo = SchemaUtils::getPropertyNameMapInfo(_txn, classInfo.id, classInfo.superClassId);
        auto resultSet = DataRecordUtils::getResultSetByCmpFunction(_txn, classInfo, _function);
        for (const auto& classNameMapInfo : classInfoExtend) {
            auto& currentClassInfo = classNameMapInfo.second;
            auto currentPropertyInfo =
                SchemaUtils::getPropertyNameMapInfo(_txn, currentClassInfo.id, currentClassInfo.superClassId);
            auto resultSetExtend = DataRecordUtils::getResultSetByCmpFunction(_txn, classInfo, _function);
            resultSet.insert(resultSet.cend(), resultSetExtend.cbegin(), resultSetExtend.cend());
        }
        return resultSet;
    }
    default: {
        auto resultSet = DataRecordUtils::getResultSet(_txn, classInfo);
        for (const auto& classNameMapInfo : classInfoExtend) {
            auto resultSetExtend = DataRecordUtils::getResultSet(_txn, classNameMapInfo.second);
            resultSet.insert(resultSet.cend(), resultSetExtend.cbegin(), resultSetExtend.cend());
        }
        return resultSet;
    }
    }
};

ResultSetCursor FindOperationBuilder::getCursor() const
{
    BEGIN_VALIDATION(_txn)
        .isTxnCompleted()
        .isClassNameValid(_className);

    auto classInfo = SchemaUtils::getExistingClass(_txn, _className);
    auto classInfoExtend = (_includeSubClassOf) ?
        SchemaUtils::getSubClassInfos(_txn, classInfo.id) : std::map<std::string, ClassAccessInfo> {};
    switch (_conditionType) {
    case ConditionType::CONDITION: {
        auto propertyNameMapInfo = SchemaUtils::getPropertyNameMapInfo(_txn, classInfo.id, classInfo.superClassId);
        auto resultSetCursor = ResultSetCursor { *_txn };
        auto result = RecordCompare::compareConditionRdesc(
            *_txn, classInfo, propertyNameMapInfo, *_condition, _indexed);
        resultSetCursor.addMetadata(result);
        for (const auto& classNameMapInfo : classInfoExtend) {
            auto& currentClassInfo = classNameMapInfo.second;
            auto currentPropertyInfo =
                SchemaUtils::getPropertyNameMapInfo(_txn, currentClassInfo.id, currentClassInfo.superClassId);
            auto resultSetExtend = RecordCompare::compareConditionRdesc(
                *_txn, classInfo, currentPropertyInfo, *_condition, _indexed);
            resultSetCursor.addMetadata(resultSetExtend);
        }
        return resultSetCursor;
    }
    case ConditionType::MULTI_CONDITION: {
        auto propertyNameMapInfo = SchemaUtils::getPropertyNameMapInfo(_txn, classInfo.id, classInfo.superClassId);
        auto resultSetCursor = ResultSetCursor { *_txn };
        auto result = RecordCompare::compareMultiConditionRdesc(
            *_txn, classInfo, propertyNameMapInfo, *_multiCondition, _indexed);
        resultSetCursor.addMetadata(result);
        for (const auto& classNameMapInfo : classInfoExtend) {
            auto& currentClassInfo = classNameMapInfo.second;
            auto currentPropertyInfo =
                SchemaUtils::getPropertyNameMapInfo(_txn, currentClassInfo.id, currentClassInfo.superClassId);
            auto resultSetExtend = RecordCompare::compareMultiConditionRdesc(
                *_txn, classInfo, currentPropertyInfo, *_multiCondition, _indexed);
            resultSetCursor.addMetadata(resultSetExtend);
        }
        return resultSetCursor;
    }
    case ConditionType::COMPARE_FUNCTION: {
        auto propertyNameMapInfo = SchemaUtils::getPropertyNameMapInfo(_txn, classInfo.id, classInfo.superClassId);
        auto resultSetCursor = ResultSetCursor { *_txn };
        auto result = DataRecordUtils::getRecordDescriptorByCmpFunction(_txn, classInfo, _function);
        resultSetCursor.addMetadata(result);
        for (const auto& classNameMapInfo : classInfoExtend) {
            auto& currentClassInfo = classNameMapInfo.second;
            auto currentPropertyInfo =
                SchemaUtils::getPropertyNameMapInfo(_txn, currentClassInfo.id, currentClassInfo.superClassId);
            auto resultSetExtend =
                DataRecordUtils::getRecordDescriptorByCmpFunction(_txn, currentClassInfo, _function);
            resultSetCursor.addMetadata(resultSetExtend);
        }
        return resultSetCursor;
    }
    default: {
        auto resultSetCursor = DataRecordUtils::getResultSetCursor(_txn, classInfo);
        if (!_includeSubClassOf) {
            return resultSetCursor;
        } else {
            auto resultSetExtend = ResultSetCursor { *_txn };
            resultSetExtend.addMetadata(resultSetCursor);
            for (const auto& classNameMapInfo : classInfoExtend) {
                resultSetExtend.addMetadata(DataRecordUtils::getResultSetCursor(_txn, classNameMapInfo.second));
            }
            return resultSetExtend;
        }
    }
    }
}

unsigned long FindOperationBuilder::count() const
{
    BEGIN_VALIDATION(_txn)
        .isTxnCompleted()
        .isClassNameValid(_className);

    auto classInfo = SchemaUtils::getExistingClass(_txn, _className);
    auto classInfoExtend = (_includeSubClassOf) ?
        SchemaUtils::getSubClassInfos(_txn, classInfo.id) : std::map<std::string, ClassAccessInfo> {};
    switch (_conditionType) {
    case ConditionType::CONDITION: {
        auto propertyNameMapInfo = SchemaUtils::getPropertyNameMapInfo(_txn, classInfo.id, classInfo.superClassId);
        auto result = RecordCompare::compareConditionCount(*_txn, classInfo, propertyNameMapInfo, *_condition, _indexed);
        for (const auto& classNameMapInfo : classInfoExtend) {
            auto& currentClassInfo = classNameMapInfo.second;
            auto currentPropertyInfo =
                SchemaUtils::getPropertyNameMapInfo(_txn, currentClassInfo.id, currentClassInfo.superClassId);
            result += RecordCompare::compareConditionCount(*_txn, classInfo, currentPropertyInfo, *_condition, _indexed);
        }
        return result;
    }
    case ConditionType::MULTI_CONDITION: {
        auto propertyNameMapInfo = SchemaUtils::getPropertyNameMapInfo(_txn, classInfo.id, classInfo.superClassId);
        auto result = RecordCompare::compareMultiConditionCount(
            *_txn, classInfo, propertyNameMapInfo, *_multiCondition, _indexed);
        for (const auto& classNameMapInfo : classInfoExtend) {
            auto& currentClassInfo = classNameMapInfo.second;
            auto currentPropertyInfo =
                SchemaUtils::getPropertyNameMapInfo(_txn, currentClassInfo.id, currentClassInfo.superClassId);
            result += RecordCompare::compareMultiConditionCount(
                *_txn, classInfo, currentPropertyInfo, *_multiCondition, _indexed);
        }
        return result;
    }
    case ConditionType::COMPARE_FUNCTION: {
        auto propertyNameMapInfo = SchemaUtils::getPropertyNameMapInfo(_txn, classInfo.id, classInfo.superClassId);
        auto result = DataRecordUtils::getCountRecordByCmpFunction(_txn, classInfo, _function);
        for (const auto& classNameMapInfo : classInfoExtend) {
            auto& currentClassInfo = classNameMapInfo.second;
            auto currentPropertyInfo =
                SchemaUtils::getPropertyNameMapInfo(_txn, currentClassInfo.id, currentClassInfo.superClassId);
            result += DataRecordUtils::getCountRecordByCmpFunction(_txn, currentClassInfo, _function);
        }
        return result;
    }
    default: {
        auto result = DataRecordUtils::getCountRecord(_txn, classInfo);
        if (_includeSubClassOf) {
            for (const auto& classNameMapInfo : classInfoExtend) {
                result += DataRecordUtils::getCountRecord(_txn, classNameMapInfo.second);
            }
        }
        return static_cast<unsigned long>(result);
    }
    }
}

ResultSet FindEdgeOperationBuilder::get() const
{
    BEGIN_VALIDATION(_txn)
        .isTxnCompleted()
        .isExistingVertex(_rdesc);

    auto edgeRecordIds = std::vector<RecordId> {};
    switch (_direction) {
    case EdgeDirection::IN: {
        edgeRecordIds = _txn->_graph->getInEdges(_rdesc.rid);
        break;
    }
    case EdgeDirection::OUT: {
        edgeRecordIds = _txn->_graph->getOutEdges(_rdesc.rid);
        break;
    }
    default: {
        auto recordIds = std::set<RecordId> {};
        auto inEdgeRecordIds = _txn->_graph->getInEdges(_rdesc.rid);
        auto outEdgeRecordIds = _txn->_graph->getOutEdges(_rdesc.rid);
        recordIds.insert(inEdgeRecordIds.cbegin(), inEdgeRecordIds.cend());
        recordIds.insert(outEdgeRecordIds.cbegin(), outEdgeRecordIds.cend());
        edgeRecordIds.assign(recordIds.cbegin(), recordIds.cend());
        break;
    }
    }
    auto result = ResultSet {};
    auto classFilter = RecordCompare::getFilterClasses(*_txn, _filter);
    for (const auto& recordId : edgeRecordIds) {
        auto edgeRecordDescriptor = RecordDescriptor { recordId };
        auto filterResult = RecordCompare::filterResult(*_txn, edgeRecordDescriptor, _filter, classFilter);
        if (filterResult.descriptor != RecordDescriptor {}) {
            result.emplace_back(filterResult);
        }
    }
    return result;
}

ResultSetCursor FindEdgeOperationBuilder::getCursor() const
{
    BEGIN_VALIDATION(_txn)
        .isTxnCompleted()
        .isExistingVertex(_rdesc);

    auto edgeRecordIds = std::vector<RecordId> {};
    switch (_direction) {
    case EdgeDirection::IN: {
        edgeRecordIds = _txn->_graph->getInEdges(_rdesc.rid);
        break;
    }
    case EdgeDirection::OUT: {
        edgeRecordIds = _txn->_graph->getOutEdges(_rdesc.rid);
        break;
    }
    default: {
        auto recordIds = std::set<RecordId> {};
        auto inEdgeRecordIds = _txn->_graph->getInEdges(_rdesc.rid);
        auto outEdgeRecordIds = _txn->_graph->getOutEdges(_rdesc.rid);
        recordIds.insert(inEdgeRecordIds.cbegin(), inEdgeRecordIds.cend());
        recordIds.insert(outEdgeRecordIds.cbegin(), outEdgeRecordIds.cend());
        edgeRecordIds.assign(recordIds.cbegin(), recordIds.cend());
        break;
    }
    }
    auto result = ResultSetCursor { *_txn };
    auto classFilter = RecordCompare::getFilterClasses(*_txn, _filter);
    for (const auto& recordId : edgeRecordIds) {
        auto edgeRecordDescriptor = RecordDescriptor { recordId };
        auto filterRecord = RecordCompare::filterRecord(*_txn, edgeRecordDescriptor, _filter, classFilter);
        if (filterRecord != RecordDescriptor {}) {
            result.addMetadata(filterRecord);
        }
    }
    return result;
}

unsigned long FindEdgeOperationBuilder::count() const
{
    BEGIN_VALIDATION(_txn)
        .isTxnCompleted()
        .isExistingVertex(_rdesc);

    auto edgeRecordIds = std::vector<RecordId> {};
    switch (_direction) {
    case EdgeDirection::IN: {
        edgeRecordIds = _txn->_graph->getInEdges(_rdesc.rid);
        break;
    }
    case EdgeDirection::OUT: {
        edgeRecordIds = _txn->_graph->getOutEdges(_rdesc.rid);
        break;
    }
    default: {
        auto recordIds = std::set<RecordId> {};
        auto inEdgeRecordIds = _txn->_graph->getInEdges(_rdesc.rid);
        auto outEdgeRecordIds = _txn->_graph->getOutEdges(_rdesc.rid);
        recordIds.insert(inEdgeRecordIds.cbegin(), inEdgeRecordIds.cend());
        recordIds.insert(outEdgeRecordIds.cbegin(), outEdgeRecordIds.cend());
        edgeRecordIds.assign(recordIds.cbegin(), recordIds.cend());
        break;
    }
    }
    auto result = 0UL;
    auto classFilter = RecordCompare::getFilterClasses(*_txn, _filter);
    for (const auto& recordId : edgeRecordIds) {
        auto edgeRecordDescriptor = RecordDescriptor { recordId };
        auto filterRecord = RecordCompare::filterRecord(*_txn, edgeRecordDescriptor, _filter, classFilter);
        if (filterRecord != RecordDescriptor {}) {
            ++result;
        }
    }
    return result;
}

ResultSet TraverseOperationBuilder::get() const
{
    BEGIN_VALIDATION(_txn)
        .isTxnCompleted()
        .isExistingVertices(_rdescs);

    auto direction = adapter::relation::Direction::ALL;
    switch (_direction) {
    case EdgeDirection::IN:
        direction = adapter::relation::Direction::IN;
        break;
    case EdgeDirection::OUT:
        direction = adapter::relation::Direction::OUT;
        break;
    default:
        break;
    }

    return algorithm::GraphTraversal::breadthFirstSearch(
        *_txn, _rdescs, _minDepth, _maxDepth, direction, _edgeFilter, _vertexFilter);
}

ResultSetCursor TraverseOperationBuilder::getCursor() const
{
    BEGIN_VALIDATION(_txn)
        .isTxnCompleted()
        .isExistingVertices(_rdescs);

    auto direction = adapter::relation::Direction::ALL;
    switch (_direction) {
    case EdgeDirection::IN:
        direction = adapter::relation::Direction::IN;
        break;
    case EdgeDirection::OUT:
        direction = adapter::relation::Direction::OUT;
        break;
    default:
        break;
    }

    auto result = algorithm::GraphTraversal::breadthFirstSearchRdesc(
        *_txn, _rdescs, _minDepth, _maxDepth, direction, _edgeFilter, _vertexFilter);
    return std::move(ResultSetCursor { *_txn }.addMetadata(result));
}

unsigned long TraverseOperationBuilder::count() const
{
    return static_cast<unsigned long>(getCursor().count());
}

ResultSet ShortestPathOperationBuilder::get() const
{
    BEGIN_VALIDATION(_txn)
        .isTxnCompleted()
        .isExistingSrcVertex(_srcRdesc)
        .isExistingDstVertex(_dstRdesc);

    return algorithm::GraphTraversal::bfsShortestPath(
        *_txn, _srcRdesc, _dstRdesc, _edgeFilter, _vertexFilter);
}

ResultSetCursor ShortestPathOperationBuilder::getCursor() const
{
    BEGIN_VALIDATION(_txn)
        .isTxnCompleted()
        .isExistingSrcVertex(_srcRdesc)
        .isExistingDstVertex(_dstRdesc);

    auto result = algorithm::GraphTraversal::bfsShortestPathRdesc(
        *_txn, _srcRdesc, _dstRdesc, _edgeFilter, _vertexFilter);
    return std::move(ResultSetCursor { *_txn }.addMetadata(result));
}

unsigned long ShortestPathOperationBuilder::count() const
{
    return static_cast<unsigned long>(getCursor().count());
}

}
