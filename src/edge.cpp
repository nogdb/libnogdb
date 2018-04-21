/*
 *  Copyright (C) 2018, Throughwave (Thailand) Co., Ltd.
 *  <peerawich at throughwave dot co dot th>
 *
 *  This file is part of libnogdb, the NogDB core library in C++.
 *
 *  libnogdb is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <tuple>

#include "shared_lock.hpp"
#include "schema.hpp"
#include "constant.hpp"
#include "env_handler.hpp"
#include "datastore.hpp"
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
        // transaction validations
        Validate::isTransactionValid(txn);
        auto classDescriptor = Generic::getClassDescriptor(txn, className, ClassType::EDGE);
        auto srcVertexDescriptor = Generic::getClassDescriptor(txn, srcVertexRecordDescriptor.rid.first, ClassType::VERTEX);
        auto dstVertexDescriptor = Generic::getClassDescriptor(txn, dstVertexRecordDescriptor.rid.first, ClassType::VERTEX);
        auto classInfo = ClassPropertyInfo{};
        auto indexInfos = std::map<std::string, std::tuple<PropertyType, IndexId, bool>>{};
        auto value = Parser::parseRecord(*txn.txnBase, classDescriptor, record, classInfo, indexInfos);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            auto srcDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(srcVertexRecordDescriptor.rid.first), true);
            auto srcKeyValue = Datastore::getRecord(dsTxnHandler, srcDBHandler, srcVertexRecordDescriptor.rid.second);
            if (srcKeyValue.empty()) {
                throw Error(GRAPH_NOEXST_SRC, Error::Type::GRAPH);
            }
            auto dstDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(dstVertexRecordDescriptor.rid.first), true);
            auto dstKeyValue = Datastore::getRecord(dsTxnHandler, dstDBHandler, dstVertexRecordDescriptor.rid.second);
            if (dstKeyValue.empty()) {
                throw Error(GRAPH_NOEXST_DST, Error::Type::GRAPH);
            }
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
        const PositionId *maxRecordNum = nullptr;
        auto maxRecordNumValue = 0U;
        try {
            auto classDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(classDescriptor->id), true);
            auto keyValue = Datastore::getRecord(dsTxnHandler, classDBHandler, EM_MAXRECNUM);
            maxRecordNum = Datastore::getValueAsNumeric<PositionId>(keyValue);
            maxRecordNumValue = *maxRecordNum;
            Datastore::putRecord(dsTxnHandler, classDBHandler, maxRecordNumValue, value, true);
            Datastore::putRecord(dsTxnHandler, classDBHandler, EM_MAXRECNUM, PositionId{maxRecordNumValue + 1});

            // add index if applied
            for (const auto &indexInfo: indexInfos) {
                auto bytesValue = record.get(indexInfo.first);
                auto const propertyType = std::get<0>(indexInfo.second);
                auto const indexId = std::get<1>(indexInfo.second);
                auto const isUnique = std::get<2>(indexInfo.second);
                Index::addIndex(*txn.txnBase, indexId, maxRecordNumValue, bytesValue, propertyType, isUnique);
            }

            auto relationDBHandler = Datastore::openDbi(dsTxnHandler, TB_RELATIONS);
            auto edgeRecord = Blob((sizeof(ClassId) + sizeof(PositionId)) * 2);
            edgeRecord.append(&srcVertexRecordDescriptor.rid.first, sizeof(ClassId));
            edgeRecord.append(&srcVertexRecordDescriptor.rid.second, sizeof(PositionId));
            edgeRecord.append(&dstVertexRecordDescriptor.rid.first, sizeof(ClassId));
            edgeRecord.append(&dstVertexRecordDescriptor.rid.second, sizeof(PositionId));
            auto key = RecordId{classDescriptor->id, maxRecordNumValue};
            Datastore::putRecord(dsTxnHandler, relationDBHandler, rid2str(key), edgeRecord);

            // update in-memory relations
            txn.txnCtx.dbRelation->createEdge(*txn.txnBase, key, srcVertexRecordDescriptor.rid,
                                              dstVertexRecordDescriptor.rid);
        } catch (const Error &err) {
            throw err;
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        } catch (Graph::ErrorType &err) {
            // should not get here
            throw Error(err, Error::Type::GRAPH);
        }
        return RecordDescriptor{classDescriptor->id, maxRecordNumValue};
    }

    void Edge::update(Txn &txn, const RecordDescriptor &recordDescriptor, const Record &record) {
        // transaction validations
        Validate::isTransactionValid(txn);
        auto classDescriptor = Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::EDGE);
        auto classInfo = ClassPropertyInfo{};
        auto indexInfos = std::map<std::string, std::tuple<PropertyType, IndexId, bool>>{};
        auto value = Parser::parseRecord(*txn.txnBase, classDescriptor, record, classInfo, indexInfos);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            auto classDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(classDescriptor->id), true);
            auto keyValue = Datastore::getRecord(dsTxnHandler, classDBHandler, recordDescriptor.rid.second);
            if (keyValue.empty()) {
                throw Error(GRAPH_NOEXST_EDGE, Error::Type::GRAPH);
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
                Index::deleteIndex(*txn.txnBase, indexId, recordDescriptor.rid.second, bytesValue, propertyType,
                                   isUnique);
            }
            for (const auto &indexInfo: indexInfos) {
                auto bytesValue = record.get(indexInfo.first);
                auto const propertyType = std::get<0>(indexInfo.second);
                auto const indexId = std::get<1>(indexInfo.second);
                auto const isUnique = std::get<2>(indexInfo.second);
                Index::addIndex(*txn.txnBase, indexId, recordDescriptor.rid.second, bytesValue, propertyType, isUnique);
            }

            Datastore::putRecord(dsTxnHandler, classDBHandler, recordDescriptor.rid.second, value);
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
    }

    void Edge::destroy(Txn &txn, const RecordDescriptor &recordDescriptor) {
        // transaction validations
        Validate::isTransactionValid(txn);
        auto classDescriptor = Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::EDGE);
        auto classInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            auto relationDBHandler = Datastore::openDbi(dsTxnHandler, TB_RELATIONS);
            Datastore::deleteRecord(dsTxnHandler, relationDBHandler, rid2str(recordDescriptor.rid));

            auto classDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(classDescriptor->id), true);
            // delete index if existing
            auto keyValue = Datastore::getRecord(dsTxnHandler, classDBHandler, recordDescriptor.rid.second);
            if (!keyValue.empty()) {
                auto indexInfos = std::map<std::string, std::tuple<PropertyType, IndexId, bool>>{};
                auto record = Parser::parseRawData(keyValue, classInfo);
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
                    Index::deleteIndex(*txn.txnBase, indexId, recordDescriptor.rid.second, bytesValue, propertyType, isUnique);
                }
            }
            // delete actual record
            Datastore::deleteRecord(dsTxnHandler, classDBHandler, recordDescriptor.rid.second);
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
        // update in-memory relations
        txn.txnCtx.dbRelation->deleteEdge(*txn.txnBase, recordDescriptor.rid);
    }

    void Edge::destroy(Txn &txn, const std::string &className) {
        // transaction validations
        Validate::isTransactionValid(txn);
        auto classDescriptor = Generic::getClassDescriptor(txn, className, ClassType::EDGE);
        auto classInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
        auto recordIds = std::vector<RecordId> {};
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        // remove all index records
        try {
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
                        auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                                     TB_INDEXING_PREFIX + std::to_string(indexId),
                                                                     true, isUnique);
                        Datastore::emptyDbi(dsTxnHandler, dataIndexDBHandler);
                        break;
                    }
                    case PropertyType::TINYINT:
                    case PropertyType::SMALLINT:
                    case PropertyType::INTEGER:
                    case PropertyType::BIGINT:
                    case PropertyType::REAL: {
                        auto dataIndexDBHandlerPositive =
                                Datastore::openDbi(dsTxnHandler,
                                                   TB_INDEXING_PREFIX + std::to_string(indexId) + "_positive",
                                                   true, isUnique);
                        auto dataIndexDBHandlerNegative =
                                Datastore::openDbi(dsTxnHandler,
                                                   TB_INDEXING_PREFIX + std::to_string(indexId) + "_negative",
                                                   true, isUnique);
                        Datastore::emptyDbi(dsTxnHandler, dataIndexDBHandlerPositive);
                        Datastore::emptyDbi(dsTxnHandler, dataIndexDBHandlerNegative);
                        break;
                    }
                    case PropertyType::TEXT: {
                        auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                                     TB_INDEXING_PREFIX + std::to_string(indexId),
                                                                     false, isUnique);
                        Datastore::emptyDbi(dsTxnHandler, dataIndexDBHandler);
                        break;
                    }
                    default:
                        break;
                }
            }
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
        // remove all records in database
        try {
            auto classDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(classDescriptor->id), true);
            auto cursorHandler = Datastore::openCursor(dsTxnHandler, classDBHandler);
            auto relationDBHandler = Datastore::openDbi(dsTxnHandler, TB_RELATIONS);
            auto keyValue = Datastore::getNextCursor(cursorHandler);
            while (!keyValue.empty()) {
                auto key = Datastore::getKeyAsNumeric<PositionId>(keyValue);
                if (*key != EM_MAXRECNUM) {
                    auto recordDescriptor = RecordDescriptor{classDescriptor->id, *key};
                    recordIds.push_back(recordDescriptor.rid);
                    // delete from relations
                    Datastore::deleteRecord(dsTxnHandler, relationDBHandler, rid2str(recordDescriptor.rid));
                    // delete a record in a datastore
                    //Datastore::deleteCursor(cursorHandler);
                }
                keyValue = Datastore::getNextCursor(cursorHandler);
            }
            // empty a database
            Datastore::emptyDbi(dsTxnHandler, classDBHandler);
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
        // update in-memory relations
        for (const auto &recordId: recordIds) {
            txn.txnCtx.dbRelation->deleteEdge(*txn.txnBase, recordId);
        }
    }

    void Edge::updateSrc(Txn &txn,
                         const RecordDescriptor &recordDescriptor,
                         const RecordDescriptor &newSrcVertexRecordDescriptor) {
        // transaction validations
        Validate::isTransactionValid(txn);
        auto classDescriptor = Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::EDGE);
        auto vertexDescriptor = Generic::getClassDescriptor(txn, newSrcVertexRecordDescriptor.rid.first,
                                                            ClassType::VERTEX);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            auto classDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(classDescriptor->id), true);
            auto keyValue = Datastore::getRecord(dsTxnHandler, classDBHandler, recordDescriptor.rid.second);
            if (keyValue.empty()) {
                throw Error(GRAPH_NOEXST_EDGE, Error::Type::GRAPH);
            }
            auto srcDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(newSrcVertexRecordDescriptor.rid.first), true);
            keyValue = Datastore::getRecord(dsTxnHandler, srcDBHandler, newSrcVertexRecordDescriptor.rid.second);
            if (keyValue.empty()) {
                throw Error(GRAPH_NOEXST_SRC, Error::Type::GRAPH);
            }
            auto key = rid2str(recordDescriptor.rid);
            auto relationDBHandler = Datastore::openDbi(dsTxnHandler, TB_RELATIONS);
            keyValue = Datastore::getRecord(dsTxnHandler, relationDBHandler, key);
            auto data = Datastore::getValueAsBlob(keyValue);
            auto dstClassId = ClassId{0};
            auto dstPositionId = PositionId{0};
            auto offset = data.retrieve(&dstClassId, sizeof(ClassId) + sizeof(PositionId), sizeof(ClassId));
            data.retrieve(&dstPositionId, offset, sizeof(PositionId));
            auto edge = Blob((sizeof(ClassId) + sizeof(PositionId)) * 2);
            edge.append(&newSrcVertexRecordDescriptor.rid.first, sizeof(ClassId));
            edge.append(&newSrcVertexRecordDescriptor.rid.second, sizeof(PositionId));
            edge.append(&dstClassId, sizeof(ClassId));
            edge.append(&dstPositionId, sizeof(PositionId));
            Datastore::putRecord(dsTxnHandler, relationDBHandler, key, edge);

            // update in-memory relations
            txn.txnCtx.dbRelation->alterVertexSrc(*txn.txnBase, recordDescriptor.rid, newSrcVertexRecordDescriptor.rid);
        } catch (Graph::ErrorType &err) {
            throw Error(err, Error::Type::GRAPH);
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
    }

    void Edge::updateDst(Txn &txn,
                         const RecordDescriptor &recordDescriptor,
                         const RecordDescriptor &newDstVertexDescriptor) {
        // transaction validations
        Validate::isTransactionValid(txn);
        auto classDescriptor = Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::EDGE);
        auto vertexDescriptor = Generic::getClassDescriptor(txn, newDstVertexDescriptor.rid.first, ClassType::VERTEX);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            auto classDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(classDescriptor->id), true);
            auto keyValue = Datastore::getRecord(dsTxnHandler, classDBHandler, recordDescriptor.rid.second);
            if (keyValue.empty()) {
                throw Error(GRAPH_NOEXST_EDGE, Error::Type::GRAPH);
            }
            auto srcDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(newDstVertexDescriptor.rid.first), true);
            keyValue = Datastore::getRecord(dsTxnHandler, srcDBHandler, newDstVertexDescriptor.rid.second);
            if (keyValue.empty()) {
                throw Error(GRAPH_NOEXST_DST, Error::Type::GRAPH);
            }
            auto key = rid2str(recordDescriptor.rid);
            auto relationDBHandler = Datastore::openDbi(dsTxnHandler, TB_RELATIONS);
            keyValue = Datastore::getRecord(dsTxnHandler, relationDBHandler, key);
            auto data = Datastore::getValueAsBlob(keyValue);
            auto srcClassId = 0U;
            auto srcPositionId = 0U;
            auto offset = data.retrieve(&srcClassId, 0, sizeof(ClassId));
            data.retrieve(&srcPositionId, offset, sizeof(PositionId));
            auto edge = Blob((sizeof(ClassId) + sizeof(PositionId)) * 2);
            edge.append(&srcClassId, sizeof(ClassId));
            edge.append(&srcPositionId, sizeof(PositionId));
            edge.append(&newDstVertexDescriptor.rid.first, sizeof(ClassId));
            edge.append(&newDstVertexDescriptor.rid.second, sizeof(PositionId));
            Datastore::putRecord(dsTxnHandler, relationDBHandler, key, edge);

            // update in-memory relations
            txn.txnCtx.dbRelation->alterVertexDst(*txn.txnBase, recordDescriptor.rid, newDstVertexDescriptor.rid);
        } catch (Graph::ErrorType &err) {
            throw Error(err, Error::Type::GRAPH);
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
    }

    ResultSet Edge::get(const Txn &txn, const std::string &className) {
        auto result = ResultSet{};
        auto classDescriptors = Generic::getMultipleClassDescriptor(txn, std::set<std::string>{className},
                                                                    ClassType::EDGE);
        for (const auto &classDescriptor: classDescriptors) {
            auto classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
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
            auto classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
            auto classInfo = ClassInfo{classDescriptor->id, className, classPropertyInfo};
            auto metadata = Generic::getRdescFromClassInfo(txn, classInfo);
            result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        }
        return result;
    }

    Result Edge::getSrc(const Txn &txn, const RecordDescriptor &recordDescriptor) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::EDGE);
        try {
            auto vertex = txn.txnCtx.dbRelation->getVertexSrc(*txn.txnBase, recordDescriptor.rid);
            auto vertexRecordDescriptor = RecordDescriptor{vertex.first, vertex.second};
            return Result{vertexRecordDescriptor, Db::getRecord(txn, vertexRecordDescriptor)};
        } catch (Graph::ErrorType &err) {
            throw Error(err, Error::Type::GRAPH);
        }
    }

    Result Edge::getDst(const Txn &txn, const RecordDescriptor &recordDescriptor) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::EDGE);
        try {
            auto vertex = txn.txnCtx.dbRelation->getVertexDst(*txn.txnBase, recordDescriptor.rid);
            auto vertexRecordDescriptor = RecordDescriptor{vertex.first, vertex.second};
            return Result{vertexRecordDescriptor, Db::getRecord(txn, vertexRecordDescriptor)};
        } catch (Graph::ErrorType &err) {
            throw Error(err, Error::Type::GRAPH);
        }
    }

    ResultSet Edge::getSrcDst(const Txn &txn, const RecordDescriptor &recordDescriptor) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::EDGE);
        try {
            auto vertices = txn.txnCtx.dbRelation->getVertexSrcDst(*txn.txnBase, recordDescriptor.rid);
            auto srcVertexRecordDescriptor = RecordDescriptor{vertices.first.first, vertices.first.second};
            auto dstVertexRecordDescriptor = RecordDescriptor{vertices.second.first, vertices.second.second};
            return ResultSet{
                    Result{srcVertexRecordDescriptor, Db::getRecord(txn, srcVertexRecordDescriptor)},
                    Result{dstVertexRecordDescriptor, Db::getRecord(txn, dstVertexRecordDescriptor)}
            };
        } catch (Graph::ErrorType &err) {
            throw Error(err, Error::Type::GRAPH);
        }
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

    ResultSetCursor Edge::getCursorIndex(Txn &txn, const std::string &className, const Condition &condition) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::EDGE, condition, true);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Edge::getCursorIndex(Txn &txn, const std::string &className, const MultiCondition &exp) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareMultiConditionRdesc(txn, className, ClassType::EDGE, exp, true);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

}
