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
#include "graph.hpp"
#include "parser.hpp"
#include "compare.hpp"
#include "index.hpp"

#include "nogdb.h"

namespace nogdb {

    const RecordDescriptor Vertex::create(Txn &txn, const std::string &className, const Record &record) {
        // transaction validations
        Validate::isTransactionValid(txn);

        // set default version
        record.setBasicInfo(TXN_VERSION, txn.getVersionId());
        record.setBasicInfo(VERSION_PROPERTY, 1ULL);

        auto classDescriptor = Generic::getClassInfo(txn, className, ClassType::VERTEX);
        auto classInfo = ClassPropertyInfo{};
        auto indexInfos = std::map<std::string, std::tuple<PropertyType, IndexId, bool>>{};
        auto value = Parser::parseRecord(*txn._txnBase, classDescriptor, record, classInfo, indexInfos);
        auto dsTxnHandler = txn._txnBase->getDsTxnHandler();
        auto classDBHandler = dsTxnHandler->openDbi(std::to_string(classDescriptor->id), true);
        auto dsResult = classDBHandler.get(MAX_RECORD_NUM_EM);
        auto const maxRecordNum = dsResult.data.numeric<PositionId>();
        classDBHandler.put(maxRecordNum, value, true);
        classDBHandler.put(MAX_RECORD_NUM_EM, PositionId{maxRecordNum + 1});

        // add index if applied
        for (const auto &indexInfo: indexInfos) {
            auto bytesValue = record.get(indexInfo.first);
            auto const propertyType = std::get<0>(indexInfo.second);
            auto const indexId = std::get<1>(indexInfo.second);
            auto const isUnique = std::get<2>(indexInfo.second);
            Index::addIndex(*txn._txnBase, indexId, maxRecordNum, bytesValue, propertyType, isUnique);
        }
        return RecordDescriptor{classDescriptor->id, maxRecordNum};
    }

    void Vertex::update(Txn &txn, const RecordDescriptor &recordDescriptor, const Record &record) {
        // transaction validations
        Validate::isTransactionValid(txn);

        record.updateVersion(txn);

        auto classDescriptor = Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto classInfo = ClassPropertyInfo{};
        auto indexInfos = std::map<std::string, std::tuple<PropertyType, IndexId, bool>>{};
        auto value = Parser::parseRecord(*txn._txnBase, classDescriptor, record, classInfo, indexInfos);
        auto dsTxnHandler = txn._txnBase->getDsTxnHandler();
        auto classDBHandler = dsTxnHandler->openDbi(std::to_string(classDescriptor->id), true);
        auto dsResult = classDBHandler.get(recordDescriptor.rid.second);
        if (dsResult.data.empty()) {
            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
        }
        auto existingRecord = Parser::parseRawData(dsResult, classInfo);
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
            Index::deleteIndex(*txn._txnBase, indexId, recordDescriptor.rid.second, bytesValue, propertyType, isUnique);
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

    void Vertex::destroy(Txn &txn, const RecordDescriptor &recordDescriptor) {
        // transaction validations
        Validate::isTransactionValid(txn);
        auto classDescriptor = Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto classInfo = Generic::getClassMapProperty(*txn._txnBase, classDescriptor);
        auto edgeRecordDescriptors = std::vector<RecordDescriptor> {};
        try {
            for (const auto &edge: txn._txnCtx.dbRelation->getEdgeInOut(*txn._txnBase, recordDescriptor.rid)) {
                edgeRecordDescriptors.emplace_back(RecordDescriptor{edge.first, edge.second});
            }
        } catch (const Error &err) {
            if (err.code() != NOGDB_GRAPH_NOEXST_VERTEX)
                throw err;
        }
        // delete edges and relations
        for (const auto &edge: edgeRecordDescriptors) {
            Edge::destroy(txn, edge);
        }
        // delete a record in a datastore
        auto dsTxnHandler = txn._txnBase->getDsTxnHandler();
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
        txn._txnCtx.dbRelation->deleteVertex(*txn._txnBase, recordDescriptor.rid);
    }

    void Vertex::destroy(Txn &txn, const std::string &className) {
        // transaction validations
        Validate::isTransactionValid(txn);
        auto classDescriptor = Generic::getClassInfo(txn, className, ClassType::VERTEX);
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
        // remove all records in a database
        auto classDBHandler = dsTxnHandler->openDbi(std::to_string(classDescriptor->id), true);
        auto cursorHandler = dsTxnHandler->openCursor(classDBHandler);
        auto relationDBHandler = dsTxnHandler->openDbi(TB_RELATIONS);
        for(auto keyValue = cursorHandler.getNext();
            !keyValue.empty();
            keyValue = cursorHandler.getNext()) {
            auto key = keyValue.key.data.numeric<PositionId>();
            if (key != MAX_RECORD_NUM_EM) {
                auto recordDescriptor = RecordDescriptor{classDescriptor->id, key};
                recordIds.push_back(recordDescriptor.rid);
                auto edgeRecordDescriptors = std::vector<RecordDescriptor> {};
                try {
                    for (const auto &edge: txn._txnCtx.dbRelation->getEdgeInOut(*txn._txnBase, recordDescriptor.rid)) {
                        edgeRecordDescriptors.emplace_back(RecordDescriptor{edge.first, edge.second});
                    }
                } catch (const Error &err) {
                    if (err.code() != NOGDB_GRAPH_NOEXST_VERTEX) {
                        throw err;
                    }
                }
                // delete from relations
                for (const auto &edge: edgeRecordDescriptors) {
                    relationDBHandler.del(rid2str(edge.rid));
                    auto edgeClassHandler = dsTxnHandler->openDbi(std::to_string(edge.rid.first), true);
                    edgeClassHandler.del(edge.rid.second);
                }
            }
        }
        // empty a database
        classDBHandler.drop();
        // update in-memory
        for (const auto &recordId: recordIds) {
            txn._txnCtx.dbRelation->deleteVertex(*txn._txnBase, recordId);
        }
    }

    ResultSet Vertex::get(const Txn &txn, const std::string &className) {
        auto result = ResultSet{};
        auto classDescriptors = Generic::getMultipleClassDescriptor(txn, std::set<std::string>{className}, ClassType::VERTEX);
        for (const auto &classDescriptor: classDescriptors) {
            auto classPropertyInfo = Generic::getClassMapProperty(*txn._txnBase, classDescriptor);
            auto classInfo = ClassInfo{classDescriptor->id, className, classPropertyInfo};
            auto partial = Generic::getRecordFromClassInfo(txn, classInfo);
            result.insert(result.end(), partial.cbegin(), partial.cend());
        }
        return result;
    }

    ResultSetCursor Vertex::getCursor(Txn &txn, const std::string &className) {
        auto result = ResultSetCursor{txn};
        auto classDescriptors = Generic::getMultipleClassDescriptor(txn, std::set<std::string>{className}, ClassType::VERTEX);
        for (const auto &classDescriptor: classDescriptors) {
            auto classPropertyInfo = Generic::getClassMapProperty(*txn._txnBase, classDescriptor);
            auto classInfo = ClassInfo{classDescriptor->id, className, classPropertyInfo};
            auto metadata = Generic::getRdescFromClassInfo(txn, classInfo);
            result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        }
        return result;
    }

    ResultSet Vertex::getInEdge(const Txn &txn,
                                const RecordDescriptor &recordDescriptor,
                                const ClassFilter &classFilter) {
        // basic class verification
        Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Generic::getEdgeNeighbour(txn, recordDescriptor, edgeClassIds, &Graph::getEdgeIn);
    }

    ResultSet Vertex::getOutEdge(const Txn &txn,
                                 const RecordDescriptor &recordDescriptor,
                                 const ClassFilter &classFilter) {
        // basic class verification
        Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Generic::getEdgeNeighbour(txn, recordDescriptor, edgeClassIds, &Graph::getEdgeOut);
    }

    ResultSet Vertex::getAllEdge(const Txn &txn,
                                 const RecordDescriptor &recordDescriptor,
                                 const ClassFilter &classFilter) {
        // basic class verification
        Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Generic::getEdgeNeighbour(txn, recordDescriptor, edgeClassIds, &Graph::getEdgeInOut);
    }

    ResultSetCursor Vertex::getInEdgeCursor(Txn &txn,
                                            const RecordDescriptor &recordDescriptor,
                                            const ClassFilter &classFilter) {
        // basic class verification
        Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Generic::getRdescEdgeNeighbour(txn, recordDescriptor, edgeClassIds, &Graph::getEdgeIn);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Vertex::getOutEdgeCursor(Txn &txn,
                                             const RecordDescriptor &recordDescriptor,
                                             const ClassFilter &classFilter) {
        // basic class verification
        Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Generic::getRdescEdgeNeighbour(txn, recordDescriptor, edgeClassIds, &Graph::getEdgeOut);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Vertex::getAllEdgeCursor(Txn &txn,
                                             const RecordDescriptor &recordDescriptor,
                                             const ClassFilter &classFilter) {
        // basic class verification
        Generic::getClassInfo(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Generic::getRdescEdgeNeighbour(txn, recordDescriptor, edgeClassIds, &Graph::getEdgeInOut);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSet Vertex::get(const Txn &txn,
                           const std::string &className,
                           const Condition &condition) {
        return Compare::compareCondition(txn, className, ClassType::VERTEX, condition);
    }

    ResultSet Vertex::get(const Txn &txn,
                           const std::string &className,
                           bool (*condition)(const Record &)) {
        return Compare::compareCondition(txn, className, ClassType::VERTEX, condition);
    }

    ResultSet Vertex::get(const Txn &txn,
                           const std::string &className,
                           const MultiCondition &multiCondition) {
        return Compare::compareMultiCondition(txn, className, ClassType::VERTEX, multiCondition);
    }

    ResultSetCursor Vertex::getCursor(Txn &txn, const std::string &className, const Condition &condition) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::VERTEX, condition);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;

    }

    ResultSetCursor Vertex::getCursor(Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::VERTEX, condition);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Vertex::getCursor(Txn &txn, const std::string &className, const MultiCondition &exp) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareMultiConditionRdesc(txn, className, ClassType::VERTEX, exp);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSet Vertex::getInEdge(const Txn &txn,
                                 const RecordDescriptor &recordDescriptor,
                                 const Condition &condition,
                                 const ClassFilter &classFilter) {
        return Compare::compareEdgeCondition(txn,
                                             recordDescriptor,
                                             &Graph::getEdgeIn,
                                             &Graph::getEdgeClassIn,
                                             condition,
                                             classFilter);
    }

    ResultSet Vertex::getInEdge(const Txn &txn,
                                 const RecordDescriptor &recordDescriptor,
                                 const MultiCondition &multiCondition,
                                 const ClassFilter &classFilter) {
        return Compare::compareEdgeMultiCondition(txn,
                                                  recordDescriptor,
                                                  &Graph::getEdgeIn,
                                                  &Graph::getEdgeClassIn,
                                                  multiCondition,
                                                  classFilter);
    }

    ResultSet Vertex::getInEdge(const Txn &txn,
                                 const RecordDescriptor &recordDescriptor,
                                 bool (*condition)(const Record &record),
                                 const ClassFilter &classFilter) {
        return Compare::compareEdgeCondition(txn,
                                             recordDescriptor,
                                             &Graph::getEdgeIn,
                                             &Graph::getEdgeClassIn,
                                             condition,
                                             classFilter);
    }

    ResultSetCursor Vertex::getInEdgeCursor(Txn &txn,
                                             const RecordDescriptor &recordDescriptor,
                                             const Condition &condition,
                                             const ClassFilter &classFilter) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareEdgeConditionRdesc(txn,
                                                           recordDescriptor,
                                                           &Graph::getEdgeIn,
                                                           &Graph::getEdgeClassIn,
                                                           condition,
                                                           classFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Vertex::getInEdgeCursor(Txn &txn,
                                             const RecordDescriptor &recordDescriptor,
                                             const MultiCondition &multiCondition,
                                             const ClassFilter &classFilter) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareEdgeMultiConditionRdesc(txn,
                                                                recordDescriptor,
                                                                &Graph::getEdgeIn,
                                                                &Graph::getEdgeClassIn,
                                                                multiCondition,
                                                                classFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Vertex::getInEdgeCursor(Txn &txn,
                                             const RecordDescriptor &recordDescriptor,
                                             bool (*condition)(const Record &record),
                                             const ClassFilter &classFilter) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareEdgeConditionRdesc(txn,
                                                           recordDescriptor,
                                                           &Graph::getEdgeIn,
                                                           &Graph::getEdgeClassIn,
                                                           condition,
                                                           classFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSet Vertex::getOutEdge(const Txn &txn,
                                  const RecordDescriptor &recordDescriptor,
                                  const Condition &condition,
                                  const ClassFilter &classFilter) {
        return Compare::compareEdgeCondition(txn,
                                             recordDescriptor,
                                             &Graph::getEdgeOut,
                                             &Graph::getEdgeClassOut,
                                             condition,
                                             classFilter);
    }

    ResultSet Vertex::getOutEdge(const Txn &txn,
                                  const RecordDescriptor &recordDescriptor,
                                  const MultiCondition &multiCondition,
                                  const ClassFilter &classFilter) {
        return Compare::compareEdgeMultiCondition(txn,
                                                  recordDescriptor,
                                                  &Graph::getEdgeOut,
                                                  &Graph::getEdgeClassOut,
                                                  multiCondition,
                                                  classFilter);
    }

    ResultSet Vertex::getOutEdge(const Txn &txn,
                                  const RecordDescriptor &recordDescriptor,
                                  bool (*condition)(const Record &record),
                                  const ClassFilter &classFilter) {
        return Compare::compareEdgeCondition(txn,
                                             recordDescriptor,
                                             &Graph::getEdgeOut,
                                             &Graph::getEdgeClassOut,
                                             condition,
                                             classFilter);
    }

    ResultSetCursor Vertex::getOutEdgeCursor(Txn &txn,
                                              const RecordDescriptor &recordDescriptor,
                                              const Condition &condition,
                                              const ClassFilter &classFilter) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareEdgeConditionRdesc(txn,
                                                           recordDescriptor,
                                                           &Graph::getEdgeOut,
                                                           &Graph::getEdgeClassOut,
                                                           condition,
                                                           classFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;

    }

    ResultSetCursor Vertex::getOutEdgeCursor(Txn &txn,
                                              const RecordDescriptor &recordDescriptor,
                                              const MultiCondition &multiCondition,
                                              const ClassFilter &classFilter) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareEdgeMultiConditionRdesc(txn,
                                                                recordDescriptor,
                                                                &Graph::getEdgeOut,
                                                                &Graph::getEdgeClassOut,
                                                                multiCondition,
                                                                classFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;

    }

    ResultSetCursor Vertex::getOutEdgeCursor(Txn &txn,
                                              const RecordDescriptor &recordDescriptor,
                                              bool (*condition)(const Record &record),
                                              const ClassFilter &classFilter) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareEdgeConditionRdesc(txn,
                                                           recordDescriptor,
                                                           &Graph::getEdgeOut,
                                                           &Graph::getEdgeClassOut,
                                                           condition,
                                                           classFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSet Vertex::getAllEdge(const Txn &txn,
                                  const RecordDescriptor &recordDescriptor,
                                  const Condition &condition,
                                  const ClassFilter &classFilter) {
        return Compare::compareEdgeCondition(txn,
                                             recordDescriptor,
                                             &Graph::getEdgeInOut,
                                             &Graph::getEdgeClassInOut,
                                             condition,
                                             classFilter);
    }

    ResultSet Vertex::getAllEdge(const Txn &txn,
                                  const RecordDescriptor &recordDescriptor,
                                  const MultiCondition &multiCondition,
                                  const ClassFilter &classFilter) {
        return Compare::compareEdgeMultiCondition(txn,
                                                  recordDescriptor,
                                                  &Graph::getEdgeInOut,
                                                  &Graph::getEdgeClassInOut,
                                                  multiCondition,
                                                  classFilter);
    }

    ResultSet Vertex::getAllEdge(const Txn &txn,
                                  const RecordDescriptor &recordDescriptor,
                                  bool (*condition)(const Record &record),
                                  const ClassFilter &classFilter) {
        return Compare::compareEdgeCondition(txn,
                                             recordDescriptor,
                                             &Graph::getEdgeInOut,
                                             &Graph::getEdgeClassInOut,
                                             condition,
                                             classFilter);
    }

    ResultSetCursor Vertex::getAllEdgeCursor(Txn &txn,
                                              const RecordDescriptor &recordDescriptor,
                                              const Condition &condition,
                                              const ClassFilter &classFilter) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareEdgeConditionRdesc(txn,
                                                           recordDescriptor,
                                                           &Graph::getEdgeInOut,
                                                           &Graph::getEdgeClassInOut,
                                                           condition,
                                                           classFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Vertex::getAllEdgeCursor(Txn &txn,
                                              const RecordDescriptor &recordDescriptor,
                                              const MultiCondition &multiCondition,
                                              const ClassFilter &classFilter) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareEdgeMultiConditionRdesc(txn,
                                                                recordDescriptor,
                                                                &Graph::getEdgeInOut,
                                                                &Graph::getEdgeClassInOut,
                                                                multiCondition,
                                                                classFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Vertex::getAllEdgeCursor(Txn &txn,
                                              const RecordDescriptor &recordDescriptor,
                                              bool (*condition)(const Record &record),
                                              const ClassFilter &classFilter) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareEdgeConditionRdesc(txn,
                                                           recordDescriptor,
                                                           &Graph::getEdgeInOut,
                                                           &Graph::getEdgeClassInOut,
                                                           condition,
                                                           classFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSet Vertex::getIndex(const Txn &txn, const std::string &className, const Condition &condition) {
        return Compare::compareCondition(txn, className, ClassType::VERTEX, condition, true);
    }

    ResultSet Vertex::getIndex(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
        return Compare::compareMultiCondition(txn, className, ClassType::VERTEX, multiCondition, true);
    }

    ResultSetCursor Vertex::getIndexCursor(Txn &txn, const std::string &className, const Condition &condition) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::VERTEX, condition, true);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Vertex::getIndexCursor(Txn &txn, const std::string &className, const MultiCondition &exp) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareMultiConditionRdesc(txn, className, ClassType::VERTEX, exp, true);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

}
