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
#include "generic.hpp"

#include "nogdb.h"

namespace nogdb {

    const RecordDescriptor Edge::create(Txn &txn,
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
        try {
            auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
            auto vertexBlob = parser::Parser::parseEdgeVertexSrcDst(srcVertexRecordDescriptor.rid,
                                                                    dstVertexRecordDescriptor.rid);
            auto valueBlob = parser::Parser::parseRecord(record, propertyNameMapInfo);
            auto positionId = edgeDataRecord.insert(vertexBlob + valueBlob);
            auto recordDescriptor = RecordDescriptor{edgeClassInfo.id, positionId};
            txn._iGraph->addEdge(recordDescriptor.rid, srcVertexRecordDescriptor.rid, dstVertexRecordDescriptor.rid);
            txn._iIndex->insert(recordDescriptor, record, propertyNameMapInfo);
            return recordDescriptor;
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }
    }

    void Edge::update(Txn &txn, const RecordDescriptor &recordDescriptor, const Record &record) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto edgeClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
        auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
        auto existingRecordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
        auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
        auto propertyIdMapInfo = txn._iSchema->getPropertyIdMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
        try {
            // insert an updated record
            auto vertexBlob = parser::Parser::parseEdgeRawDataVertexSrcDstAsBlob(existingRecordResult.data.blob());
            auto newRecordBlob = parser::Parser::parseRecord(record, propertyNameMapInfo);
            edgeDataRecord.insert(vertexBlob + newRecordBlob);
            // remove index if applied in existing record
            auto existingRecord = parser::Parser::parseRawData(existingRecordResult, propertyIdMapInfo, true);
            txn._iIndex->remove(recordDescriptor, existingRecord, propertyNameMapInfo);
            // add index if applied in new record
            txn._iIndex->insert(recordDescriptor, record, propertyNameMapInfo);
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }
    }

    void Edge::destroy(Txn &txn, const RecordDescriptor &recordDescriptor) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto edgeClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
        auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
        auto recordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
        auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
        auto propertyIdMapInfo = txn._iSchema->getPropertyIdMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
        try {
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

    void Edge::destroy(Txn &txn, const std::string &className) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::EDGE);
        auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
        auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
        try {
            auto cursorHandler = edgeDataRecord.getCursor();
            for(auto keyValue = cursorHandler.getNext();
                !keyValue.empty();
                keyValue = cursorHandler.getNext()) {
                auto key = keyValue.key.data.numeric<PositionId>();
                if (key == MAX_RECORD_NUM_EM) continue;
                auto srcDstVertex = parser::Parser::parseEdgeRawDataVertexSrcDst(keyValue.val.data.blob());
                txn._iGraph->removeRelFromEdge(RecordId{edgeClassInfo.id, key}, srcDstVertex.first, srcDstVertex.second);
            }
            edgeDataRecord.destroy();
            txn._iIndex->drop(edgeClassInfo.id, propertyNameMapInfo);
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }
    }

    void Edge::updateSrc(Txn &txn,
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
            txn._iGraph->updateEdgeSrc(recordDescriptor.rid, newSrcVertexRecordDescriptor.rid, srcDstVertex.first, srcDstVertex.second);
            auto newVertexBlob = parser::Parser::parseEdgeVertexSrcDst(newSrcVertexRecordDescriptor.rid, srcDstVertex.second);
            auto dataBlob = parser::Parser::parseEdgeRawDataAsBlob(recordResult.data.blob());
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
        . isTransactionValid()
        . isExistingSrcVertex(newDstVertexDescriptor);

        auto edgeClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::EDGE);
        auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
        auto recordResult = edgeDataRecord.getResult(recordDescriptor.rid.second);
        try {
            auto srcDstVertex = parser::Parser::parseEdgeRawDataVertexSrcDst(recordResult.data.blob());
            txn._iGraph->updateEdgeDst(recordDescriptor.rid, newDstVertexDescriptor.rid, srcDstVertex.first, srcDstVertex.second);
            auto newVertexBlob = parser::Parser::parseEdgeVertexSrcDst(srcDstVertex.first, newDstVertexDescriptor.rid);
            auto dataBlob = parser::Parser::parseEdgeRawDataAsBlob(recordResult.data.blob());
            edgeDataRecord.insert(newVertexBlob + dataBlob);
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }
    }

    ResultSet Edge::get(const Txn &txn, const std::string &className) {
        auto result = ResultSet{};
        auto classDescriptors = Generic::getMultipleClassDescriptor(txn, std::set<std::string>{className},
                                                                    ClassType::EDGE);
        for (const auto &classDescriptor: classDescriptors) {
            auto classPropertyInfo = Generic::getClassMapProperty(*txn._txnBase, classDescriptor);
            auto classInfo = ClassInfo{classDescriptor->id, className, classPropertyInfo};
            auto partial = Generic::getRecordFromClassInfo(txn, classInfo);
            result.insert(result.end(), partial.cbegin(), partial.cend());
        }
        return result;
    }

    ResultSetCursor Edge::getCursor(Txn &txn, const std::string &className) {
        auto result = ResultSetCursor{txn};
        auto classDescriptors = Generic::getMultipleClassDescriptor(txn, std::set<std::string>{className},
                                                                    ClassType::EDGE);
        for (const auto &classDescriptor: classDescriptors) {
            auto classPropertyInfo = Generic::getClassMapProperty(*txn._txnBase, classDescriptor);
            auto classInfo = ClassInfo{classDescriptor->id, className, classPropertyInfo};
            auto metadata = Generic::getRdescFromClassInfo(txn, classInfo);
            result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        }
        return result;
    }

    Result Edge::getSrc(const Txn &txn, const RecordDescriptor &recordDescriptor) {
        Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::EDGE);

        auto vertex = txn._txnCtx.dbRelation->getVertexSrc(*txn._txnBase, recordDescriptor.rid);
        auto vertexRecordDescriptor = RecordDescriptor{vertex.first, vertex.second};
        return Result{vertexRecordDescriptor, DB::getRecord(txn, vertexRecordDescriptor)};
    }

    Result Edge::getDst(const Txn &txn, const RecordDescriptor &recordDescriptor) {
        Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::EDGE);

        auto vertex = txn._txnCtx.dbRelation->getVertexDst(*txn._txnBase, recordDescriptor.rid);
        auto vertexRecordDescriptor = RecordDescriptor{vertex.first, vertex.second};
        return Result{vertexRecordDescriptor, DB::getRecord(txn, vertexRecordDescriptor)};
    }

    ResultSet Edge::getSrcDst(const Txn &txn, const RecordDescriptor &recordDescriptor) {
        Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::EDGE);

        auto vertices = txn._txnCtx.dbRelation->getVertexSrcDst(*txn._txnBase, recordDescriptor.rid);
        auto srcVertexRecordDescriptor = RecordDescriptor{vertices.first.first, vertices.first.second};
        auto dstVertexRecordDescriptor = RecordDescriptor{vertices.second.first, vertices.second.second};
        return ResultSet{
                Result{srcVertexRecordDescriptor, DB::getRecord(txn, srcVertexRecordDescriptor)},
                Result{dstVertexRecordDescriptor, DB::getRecord(txn, dstVertexRecordDescriptor)}
        };
    }

    ResultSet Edge::get(const Txn &txn, const std::string &className, const Condition &condition) {
        return Compare::compareCondition(txn, className, ClassType::EDGE, condition);
    }

    ResultSet Edge::get(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
        return Compare::compareCondition(txn, className, ClassType::EDGE, condition);
    }

    ResultSet Edge::get(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
        return Compare::compareMultiCondition(txn, className, ClassType::EDGE, multiCondition);
    }

    ResultSetCursor Edge::getCursor(Txn &txn, const std::string &className, const Condition &condition) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::EDGE, condition);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Edge::getCursor(Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::EDGE, condition);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Edge::getCursor(Txn &txn, const std::string &className, const MultiCondition &exp) {
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

    ResultSetCursor Edge::getIndexCursor(Txn &txn, const std::string &className, const Condition &condition) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::EDGE, condition, true);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Edge::getIndexCursor(Txn &txn, const std::string &className, const MultiCondition &exp) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareMultiConditionRdesc(txn, className, ClassType::EDGE, exp, true);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

}
