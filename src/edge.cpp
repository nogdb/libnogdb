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

#include "nogdb.h"

namespace nogdb {

    const RecordDescriptor Edge::create(const Txn &txn,
                                        const std::string &className,
                                        const RecordDescriptor &srcVertexRecordDescriptor,
                                        const RecordDescriptor &dstVertexRecordDescriptor,
                                        const Record &record) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid()
        . isExistingSrcVertex(srcVertexRecordDescriptor)
        . isExistingDstVertex(dstVertexRecordDescriptor);

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
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }
    }

    void Edge::update(const Txn &txn, const RecordDescriptor &recordDescriptor, const Record &record) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

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
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }
    }

    void Edge::destroy(const Txn &txn, const RecordDescriptor &recordDescriptor) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

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
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }
    }

    void Edge::destroy(const Txn &txn, const std::string &className) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
        try {
            auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
            auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
            auto result = std::map<RecordId, std::pair<RecordId, RecordId>>{};
            std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
                    [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto srcDstVertex = parser::Parser::parseEdgeRawDataVertexSrcDst(result.data.blob());
                auto edgeRecordId = RecordId{edgeClassInfo.id, positionId};
                txn._iGraph->removeRelFromEdge(edgeRecordId, srcDstVertex.first, srcDstVertex.second);
            };
            edgeDataRecord.resultSetIter(callback);
            edgeDataRecord.destroy();
            txn._iIndex->drop(edgeClassInfo.id, propertyNameMapInfo);
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }
    }

    void Edge::updateSrc(const Txn &txn,
                         const RecordDescriptor &recordDescriptor,
                         const RecordDescriptor &newSrcVertexRecordDescriptor) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid()
        . isExistingSrcVertex(newSrcVertexRecordDescriptor);

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
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }
    }

    void Edge::updateDst(const Txn &txn,
                         const RecordDescriptor &recordDescriptor,
                         const RecordDescriptor &newDstVertexDescriptor) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid()
        . isExistingSrcVertex(newDstVertexDescriptor);

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
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }
    }

    ResultSet Edge::get(const Txn &txn, const std::string &className) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
        auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
        auto propertyIdMapInfo = txn._iSchema->getPropertyIdMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
        auto resultSet = ResultSet{};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
                [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
            auto const record = parser::Parser::parseRawDataWithBasicInfo(
                    edgeClassInfo.name,
                    RecordId{edgeClassInfo.id, positionId},
                    result, propertyIdMapInfo, edgeClassInfo.type);
            resultSet.emplace_back(Result{RecordDescriptor{edgeClassInfo.id, positionId}, record});
        };
        edgeDataRecord.resultSetIter(callback);
        return resultSet;
    }

    ResultSet Edge::getExtend(const Txn &txn, const std::string &className) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
        return adapter::datarecord::DataRecords(&txn, edgeClassInfo).get();
    }

    ResultSetCursor Edge::getCursor(const Txn &txn, const std::string &className) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
        auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
        auto propertyIdMapInfo = txn._iSchema->getPropertyIdMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
        auto resultSetCursor = ResultSetCursor{txn};
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
                [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
            resultSetCursor.metadata.emplace_back(RecordDescriptor{edgeClassInfo.id, positionId});
        };
        edgeDataRecord.resultSetIter(callback);
        return resultSetCursor;
    }

    ResultSetCursor Edge::getExtendCursor(const Txn &txn, const std::string &className) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
        return adapter::datarecord::DataRecords(&txn, edgeClassInfo).getCursor();
    }

    Result Edge::getSrc(const Txn &txn, const RecordDescriptor &recordDescriptor) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto edgeClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
        auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
        auto rawData = edgeDataRecord.getBlob(recordDescriptor.rid.second);
        auto srcDstVertex = parser::Parser::parseEdgeRawDataVertexSrcDst(rawData);
        auto srcVertexRecordDescriptor = RecordDescriptor{srcDstVertex.first};
        return Result{srcVertexRecordDescriptor, DB::getRecord(txn, srcVertexRecordDescriptor)};
    }

    Result Edge::getDst(const Txn &txn, const RecordDescriptor &recordDescriptor) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto edgeClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
        auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
        auto rawData = edgeDataRecord.getBlob(recordDescriptor.rid.second);
        auto srcDstVertex = parser::Parser::parseEdgeRawDataVertexSrcDst(rawData);
        auto dstVertexRecordDescriptor = RecordDescriptor{srcDstVertex.second};
        return Result{dstVertexRecordDescriptor, DB::getRecord(txn, dstVertexRecordDescriptor)};
    }

    ResultSet Edge::getSrcDst(const Txn &txn, const RecordDescriptor &recordDescriptor) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto edgeClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
        auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
        auto rawData = edgeDataRecord.getBlob(recordDescriptor.rid.second);
        auto srcDstVertex = parser::Parser::parseEdgeRawDataVertexSrcDst(rawData);
        auto srcVertexRecordDescriptor = RecordDescriptor{srcDstVertex.first};
        auto dstVertexRecordDescriptor = RecordDescriptor{srcDstVertex.second};
        return ResultSet{
                Result{srcVertexRecordDescriptor, DB::getRecord(txn, srcVertexRecordDescriptor)},
                Result{dstVertexRecordDescriptor, DB::getRecord(txn, dstVertexRecordDescriptor)}
        };
    }

    //TODO: complete all functions below
    ResultSet Edge::get(const Txn &txn, const std::string &className, const Condition &condition) {
        return Compare::compareCondition(txn, className, ClassType::EDGE, condition);
    }

    ResultSet Edge::get(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
        return Compare::compareCondition(txn, className, ClassType::EDGE, condition);
    }

    ResultSet Edge::get(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
        return Compare::compareMultiCondition(txn, className, ClassType::EDGE, multiCondition);
    }

    ResultSet Edge::getExtend(const Txn &txn, const std::string &className, const Condition &condition) {
        return Compare::compareCondition(txn, className, ClassType::EDGE, condition);
    }

    ResultSet Edge::getExtend(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
        return Compare::compareCondition(txn, className, ClassType::EDGE, condition);
    }

    ResultSet Edge::getExtend(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
        return Compare::compareMultiCondition(txn, className, ClassType::EDGE, multiCondition);
    }

    ResultSetCursor Edge::getCursor(const Txn &txn, const std::string &className, const Condition &condition) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::EDGE, condition);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Edge::getCursor(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::EDGE, condition);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Edge::getCursor(const Txn &txn, const std::string &className, const MultiCondition &exp) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareMultiConditionRdesc(txn, className, ClassType::EDGE, exp);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Edge::getExtendCursor(const Txn &txn, const std::string &className, const Condition &condition) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::EDGE, condition);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Edge::getExtendCursor(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::EDGE, condition);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Edge::getExtendCursor(const Txn &txn, const std::string &className, const MultiCondition &exp) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareMultiConditionRdesc(txn, className, ClassType::EDGE, exp);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSet Edge::getIndex(const Txn &txn, const std::string &className, const Condition &condition) {
        return Compare::compareCondition(txn, className, ClassType::EDGE, condition, true);
    }

    ResultSet Edge::getIndex(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
        return Compare::compareMultiCondition(txn, className, ClassType::EDGE, multiCondition, true);
    }

    ResultSet Edge::getExtendIndex(const Txn &txn, const std::string &className, const Condition &condition) {
        return Compare::compareCondition(txn, className, ClassType::EDGE, condition, true);
    }

    ResultSet Edge::getExtendIndex(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
        return Compare::compareMultiCondition(txn, className, ClassType::EDGE, multiCondition, true);
    }

    ResultSetCursor Edge::getIndexCursor(const Txn &txn, const std::string &className, const Condition &condition) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::EDGE, condition, true);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Edge::getIndexCursor(const Txn &txn, const std::string &className, const MultiCondition &exp) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareMultiConditionRdesc(txn, className, ClassType::EDGE, exp, true);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Edge::getExtendIndexCursor(const Txn &txn, const std::string &className, const Condition &condition) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::EDGE, condition, true);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Edge::getExtendIndexCursor(const Txn &txn, const std::string &className, const MultiCondition &exp) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareMultiConditionRdesc(txn, className, ClassType::EDGE, exp, true);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

}
