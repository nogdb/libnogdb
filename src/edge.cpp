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

#include "schema.hpp"
#include "constant.hpp"
#include "lmdb_engine.hpp"
#include "datarecord_adapter.hpp"
#include "index_adapter.hpp"
#include "graph.hpp"
#include "parser.hpp"
#include "compare.hpp"
#include "index.hpp"
#include "generic.hpp"

#include "nogdb.h"

namespace nogdb {

    const RecordDescriptor Edge::create(Txn &txn,
                                        const std::string &className,
                                        const RecordDescriptor &srcVertexRecordDescriptor,
                                        const RecordDescriptor &dstVertexRecordDescriptor,
                                        const Record &record) {
        Validate::isTransactionValid(txn);
        Validate::isExistingSrcVertex(txn, srcVertexRecordDescriptor);
        Validate::isExistingDstVertex(txn, dstVertexRecordDescriptor);
        auto edgeClassInfo = Generic::getClassInfo(txn, className, ClassType::EDGE);
        try {
            auto edgeDataRecord = adapter::datarecord::DataRecord(txn._txnBase, edgeClassInfo.id, ClassType::EDGE);
            auto propertyInfo = Generic::getPropertyNameMapInfo(txn, edgeClassInfo.id, edgeClassInfo.superClassId);
            auto vertices = parser::Parser::parseVertexSrcDst(srcVertexRecordDescriptor.rid, dstVertexRecordDescriptor.rid);
            auto value = parser::Parser::parseRecord(record, propertyInfo);
            edgeDataRecord.insert(vertices + value);

            // add index if applied
            auto indexHelper = index::IndexInterface(&txn);
            for(const auto& property: propertyInfo) {
                auto propertyName = property.first;
                auto bytes = record.get(propertyName);
                if (bytes.empty()) continue;
                auto propertyId = property.second.id;
                auto indexInfo = txn._index->getInfo(edgeClassInfo.id, propertyId);
                if (indexInfo.id != IndexId{}) {
                    indexHelper.insert();
                }
            }
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }

        // add index if applied
        for (const auto &indexInfo: indexInfos) {
            auto bytesValue = record.get(indexInfo.first);
            auto const propertyType = std::get<0>(indexInfo.second);
            auto const indexId = std::get<1>(indexInfo.second);
            auto const isUnique = std::get<2>(indexInfo.second);
            Index::addIndex(*txn._txnBase, indexId, maxRecordNum, bytesValue, propertyType, isUnique);
        }

        auto relationDBHandler = dsTxnHandler->openDbi(TB_RELATIONS);
        auto edgeRecord = Blob((sizeof(ClassId) + sizeof(PositionId)) * 2);
        edgeRecord.append(&srcVertexRecordDescriptor.rid.first, sizeof(ClassId));
        edgeRecord.append(&srcVertexRecordDescriptor.rid.second, sizeof(PositionId));
        edgeRecord.append(&dstVertexRecordDescriptor.rid.first, sizeof(ClassId));
        edgeRecord.append(&dstVertexRecordDescriptor.rid.second, sizeof(PositionId));
        auto key = RecordId{classDescriptor->id, maxRecordNum};
        relationDBHandler.put(rid2str(key), edgeRecord);

        // update in-memory relations
        txn._txnCtx.dbRelation->createEdge(*txn._txnBase, key, srcVertexRecordDescriptor.rid,
                                          dstVertexRecordDescriptor.rid);

        return RecordDescriptor{classDescriptor->id, maxRecordNum};
    }

    void Edge::update(Txn &txn, const RecordDescriptor &recordDescriptor, const Record &record) {
        // transaction validations
        Validate::isTransactionValid(txn);

        record.updateVersion(txn);

        auto classDescriptor = Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::EDGE);
        auto classInfo = ClassPropertyInfo{};
        auto indexInfos = std::map<std::string, std::tuple<PropertyType, IndexId, bool>>{};
        auto value = Parser::parseRecord(*txn._txnBase, classDescriptor, record, classInfo, indexInfos);
        auto dsTxnHandler = txn._txnBase->getDsTxnHandler();

        auto classDBHandler = dsTxnHandler->openDbi(std::to_string(classDescriptor->id), true);
        auto keyValue = classDBHandler.get(recordDescriptor.rid.second);
        if (keyValue.data.empty()) {
            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_EDGE);
        }
        auto existingRecord = Parser::parseRawData(keyValue, classInfo);
        auto existingIndexInfos = std::map<std::string, std::tuple<PropertyType, IndexId, bool>>{};
        for (const auto &property: existingRecord.getAll()) {
            // check if having any index
            auto foundProperty = classInfo.nameToDesc.find(property.first);
            if (foundProperty != classInfo.nameToDesc.cend()) {
                for (const auto &indexIter: foundProperty->second.indexInfo) {
                    if (indexIter.second.first == classDescriptor->id) {
                        existingIndexInfos.emplace(
                                property.first,
                                std::make_tuple(
                                        foundProperty->second.type,
                                        indexIter.first,
                                        indexIter.second.second
                                )
                        );
                        break;
                    }
                }
            }
        }
        for (const auto &indexInfo: existingIndexInfos) {
            auto bytesValue = existingRecord.get(indexInfo.first);
            auto const propertyType = std::get<0>(indexInfo.second);
            auto const indexId = std::get<1>(indexInfo.second);
            auto const isUnique = std::get<2>(indexInfo.second);
            Index::deleteIndex(*txn._txnBase, indexId, recordDescriptor.rid.second, bytesValue, propertyType,
                               isUnique);
        }
        for (const auto &indexInfo: indexInfos) {
            auto bytesValue = record.get(indexInfo.first);
            auto const propertyType = std::get<0>(indexInfo.second);
            auto const indexId = std::get<1>(indexInfo.second);
            auto const isUnique = std::get<2>(indexInfo.second);
            Index::addIndex(*txn._txnBase, indexId, recordDescriptor.rid.second, bytesValue, propertyType, isUnique);
        }

        classDBHandler.put(recordDescriptor.rid.second, value);
    }

    void Edge::destroy(Txn &txn, const RecordDescriptor &recordDescriptor) {
        // transaction validations
        Validate::isTransactionValid(txn);

        try {
            // update src and dst version
            ResultSet srcDst = Edge::getSrcDst(txn, recordDescriptor);
            Result src = srcDst[0], dst = srcDst[1];

            Vertex::update(txn, src.descriptor, src.record);
            Vertex::update(txn, dst.descriptor, dst.record);

        } catch (const Error &err) {
            // do nothing
        }

        auto classDescriptor = Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::EDGE);
        auto classInfo = Generic::getClassMapProperty(*txn._txnBase, classDescriptor);
        auto dsTxnHandler = txn._txnBase->getDsTxnHandler();

        auto relationDBHandler = dsTxnHandler->openDbi(TB_RELATIONS);
        relationDBHandler.del(rid2str(recordDescriptor.rid));

        auto classDBHandler = dsTxnHandler->openDbi(std::to_string(classDescriptor->id), true);
        // delete index if existing
        auto dsResult = classDBHandler.get(recordDescriptor.rid.second);
        if (!dsResult.data.empty()) {
            auto indexInfos = std::map<std::string, std::tuple<PropertyType, IndexId, bool>>{};
            auto record = Parser::parseRawData(dsResult, classInfo);
            for (const auto &property: record.getAll()) {
                // check if having any index
                auto foundProperty = classInfo.nameToDesc.find(property.first);
                if (foundProperty != classInfo.nameToDesc.cend()) {
                    for (const auto &indexIter: foundProperty->second.indexInfo) {
                        if (indexIter.second.first == classDescriptor->id) {
                            indexInfos.emplace(
                                    property.first,
                                    std::make_tuple(
                                            foundProperty->second.type,
                                            indexIter.first,
                                            indexIter.second.second
                                    )
                            );
                            break;
                        }
                    }
                }
            }
            for (const auto &indexInfo: indexInfos) {
                auto bytesValue = record.get(indexInfo.first);
                auto const propertyType = std::get<0>(indexInfo.second);
                auto const indexId = std::get<1>(indexInfo.second);
                auto const isUnique = std::get<2>(indexInfo.second);
                Index::deleteIndex(*txn._txnBase, indexId, recordDescriptor.rid.second, bytesValue, propertyType, isUnique);
            }
        }
        // delete actual record
        classDBHandler.del(recordDescriptor.rid.second);

        // update in-memory relations
        txn._txnCtx.dbRelation->deleteEdge(*txn._txnBase, recordDescriptor.rid);
    }

    void Edge::destroy(Txn &txn, const std::string &className) {
        // transaction validations
        Validate::isTransactionValid(txn);
        auto classDescriptor = Generic::getClassInfo(txn, className, ClassType::EDGE);
        auto classInfo = Generic::getClassMapProperty(*txn._txnBase, classDescriptor);
        auto recordIds = std::vector<RecordId> {};
        auto dsTxnHandler = txn._txnBase->getDsTxnHandler();

        // remove all index records
        auto indexInfos = std::vector<std::tuple<PropertyType, IndexId, bool>>{};
        for (const auto &property: classInfo.nameToDesc) {
            auto &type = property.second.type;
            auto &indexInfo = property.second.indexInfo;
            for (const auto &indexIter: indexInfo) {
                if (indexIter.second.first == classDescriptor->id) {
                    indexInfos.emplace_back(
                            std::make_tuple(
                                    type,
                                    indexIter.first,
                                    indexIter.second.second
                            )
                    );
                }
                break;
            }
        }
        for (const auto &indexInfo: indexInfos) {
            auto const propertyType = std::get<0>(indexInfo);
            auto const indexId = std::get<1>(indexInfo);
            auto const isUnique = std::get<2>(indexInfo);
            switch (propertyType) {
                case PropertyType::UNSIGNED_TINYINT:
                case PropertyType::UNSIGNED_SMALLINT:
                case PropertyType::UNSIGNED_INTEGER:
                case PropertyType::UNSIGNED_BIGINT: {
                    auto dataIndexDBHandler = dsTxnHandler->openDbi(Index::getIndexingName(indexId), true, isUnique);
                    dataIndexDBHandler.drop();
                    break;
                }
                case PropertyType::TINYINT:
                case PropertyType::SMALLINT:
                case PropertyType::INTEGER:
                case PropertyType::BIGINT:
                case PropertyType::REAL: {
                    auto dataIndexDBHandlerPositive = dsTxnHandler->openDbi(Index::getIndexingName(indexId, true), true, isUnique);
                    auto dataIndexDBHandlerNegative = dsTxnHandler->openDbi(Index::getIndexingName(indexId, false), true, isUnique);
                    dataIndexDBHandlerPositive.drop();
                    dataIndexDBHandlerNegative.drop();
                    break;
                }
                case PropertyType::TEXT: {
                    auto dataIndexDBHandler = dsTxnHandler->openDbi(Index::getIndexingName(indexId), false, isUnique);
                    dataIndexDBHandler.drop();
                    break;
                }
                default:
                    break;
            }
        }

        // remove all records in database
        auto classDBHandler = dsTxnHandler->openDbi(std::to_string(classDescriptor->id), true);
        auto cursorHandler = dsTxnHandler->openCursor(classDBHandler);
        auto relationDBHandler = dsTxnHandler->openDbi(TB_RELATIONS);
        auto keyValue = cursorHandler.getNext();
        while (!keyValue.empty()) {
            auto key = keyValue.key.data.numeric<PositionId>();
            if (key != MAX_RECORD_NUM_EM) {
                auto recordDescriptor = RecordDescriptor{classDescriptor->id, key};
                recordIds.push_back(recordDescriptor.rid);
                // delete from relations
                relationDBHandler.del(rid2str(recordDescriptor.rid));

                // update src and dst version
                try {
                    auto res = getSrcDst(txn, recordDescriptor);
                    Result src = res[0], dst = res[1];

                    Vertex::update(txn, src.descriptor, src.record);
                    Vertex::update(txn, dst.descriptor, src.record);
                } catch (const Error& err) {
                    // do nothing
                }

            }
            keyValue = cursorHandler.getNext();
        }

        // empty a database
        classDBHandler.drop();

        // update in-memory relations
        for (const auto &recordId: recordIds) {
            txn._txnCtx.dbRelation->deleteEdge(*txn._txnBase, recordId);
        }
    }

    void Edge::updateSrc(Txn &txn,
                         const RecordDescriptor &recordDescriptor,
                         const RecordDescriptor &newSrcVertexRecordDescriptor) {
        // transaction validations
        Validate::isTransactionValid(txn);

        // update source
        try {
            Result currentSrc = getSrc(txn, recordDescriptor);
            Vertex::update(txn, currentSrc.descriptor, currentSrc.record);
        } catch (const Error &err) {
            // do nothing
        }

        // update new source
        try {
            Record newSrcRecord = DB::getRecord(txn, newSrcVertexRecordDescriptor);
            Vertex::update(txn, newSrcVertexRecordDescriptor, newSrcRecord);
        } catch (const Error &err) {
            // do nothing
        }

        auto classDescriptor = Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::EDGE);
        auto vertexDescriptor = Generic::getClassInfo(txn, newSrcVertexRecordDescriptor.rid.first,
                                                      ClassType::VERTEX);
        auto dsTxnHandler = txn._txnBase->getDsTxnHandler();
        auto classDBHandler = dsTxnHandler->openDbi(std::to_string(classDescriptor->id), true);
        auto dsResult = classDBHandler.get(recordDescriptor.rid.second);
        if (dsResult.data.empty()) {
            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_EDGE);
        }
        auto srcDBHandler = dsTxnHandler->openDbi(std::to_string(newSrcVertexRecordDescriptor.rid.first), true);
        dsResult = srcDBHandler.get(newSrcVertexRecordDescriptor.rid.second);
        if (dsResult.data.empty()) {
            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_SRC);
        }
        auto key = rid2str(recordDescriptor.rid);
        auto relationDBHandler = dsTxnHandler->openDbi(TB_RELATIONS);
        dsResult = relationDBHandler.get(key);
        auto data = dsResult.data.blob();
        auto dstClassId = ClassId{0};
        auto dstPositionId = PositionId{0};
        auto offset = data.retrieve(&dstClassId, sizeof(ClassId) + sizeof(PositionId), sizeof(ClassId));
        data.retrieve(&dstPositionId, offset, sizeof(PositionId));
        auto edge = Blob((sizeof(ClassId) + sizeof(PositionId)) * 2);
        edge.append(&newSrcVertexRecordDescriptor.rid.first, sizeof(ClassId));
        edge.append(&newSrcVertexRecordDescriptor.rid.second, sizeof(PositionId));
        edge.append(&dstClassId, sizeof(ClassId));
        edge.append(&dstPositionId, sizeof(PositionId));
        relationDBHandler.put(key, edge);

        // update in-memory relations
        txn._txnCtx.dbRelation->alterVertexSrc(*txn._txnBase, recordDescriptor.rid, newSrcVertexRecordDescriptor.rid);
    }

    void Edge::updateDst(Txn &txn,
                         const RecordDescriptor &recordDescriptor,
                         const RecordDescriptor &newDstVertexDescriptor) {
        // transaction validations
        Validate::isTransactionValid(txn);
        auto classDescriptor = Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::EDGE);
        auto vertexDescriptor = Generic::getClassInfo(txn, newDstVertexDescriptor.rid.first, ClassType::VERTEX);
        auto dsTxnHandler = txn._txnBase->getDsTxnHandler();
        auto classDBHandler = dsTxnHandler->openDbi(std::to_string(classDescriptor->id), true);
        auto dsResult = classDBHandler.get(recordDescriptor.rid.second);
        if (dsResult.data.empty()) {
            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_EDGE);
        }
        auto srcDBHandler = dsTxnHandler->openDbi(std::to_string(newDstVertexDescriptor.rid.first), true);
        dsResult = srcDBHandler.get(newDstVertexDescriptor.rid.second);
        if (dsResult.data.empty()) {
            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_DST);
        }
        auto key = rid2str(recordDescriptor.rid);
        auto relationDBHandler = dsTxnHandler->openDbi(TB_RELATIONS);
        dsResult = relationDBHandler.get(key);
        auto data = dsResult.data.blob();
        auto srcClassId = 0U;
        auto srcPositionId = 0U;
        auto offset = data.retrieve(&srcClassId, 0, sizeof(ClassId));
        data.retrieve(&srcPositionId, offset, sizeof(PositionId));
        auto edge = Blob((sizeof(ClassId) + sizeof(PositionId)) * 2);
        edge.append(&srcClassId, sizeof(ClassId));
        edge.append(&srcPositionId, sizeof(PositionId));
        edge.append(&newDstVertexDescriptor.rid.first, sizeof(ClassId));
        edge.append(&newDstVertexDescriptor.rid.second, sizeof(PositionId));
        relationDBHandler.put(key, edge);

        // update in-memory relations
        txn._txnCtx.dbRelation->alterVertexDst(*txn._txnBase, recordDescriptor.rid, newDstVertexDescriptor.rid);
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
