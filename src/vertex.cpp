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

    const RecordDescriptor Vertex::create(Txn &txn, const std::string &className, const Record &record) {
        // transaction validations
        Validate::isTransactionValid(txn);
        auto classDescriptor = Generic::getClassDescriptor(txn, className, ClassType::VERTEX);
        auto classInfo = ClassPropertyInfo{};
        auto indexInfos = std::map<std::string, std::tuple<PropertyType, IndexId, bool>>{};
        auto value = Parser::parseRecord(*txn.txnBase, classDescriptor, record, classInfo, indexInfos);
        const PositionId *maxRecordNum = nullptr;
        auto maxRecordNumValue = 0U;
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
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
        } catch (const Error &err) {
            throw err;
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
        return RecordDescriptor{classDescriptor->id, maxRecordNumValue};
    }

    void Vertex::update(Txn &txn, const RecordDescriptor &recordDescriptor, const Record &record) {
        // transaction validations
        Validate::isTransactionValid(txn);
        auto classDescriptor = Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto classInfo = ClassPropertyInfo{};
        auto indexInfos = std::map<std::string, std::tuple<PropertyType, IndexId, bool>>{};
        auto value = Parser::parseRecord(*txn.txnBase, classDescriptor, record, classInfo, indexInfos);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            auto classDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(classDescriptor->id), true);
            auto keyValue = Datastore::getRecord(dsTxnHandler, classDBHandler, recordDescriptor.rid.second);
            if (keyValue.empty()) {
                throw Error(GRAPH_NOEXST_VERTEX, Error::Type::GRAPH);
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
                Index::deleteIndex(*txn.txnBase, indexId, recordDescriptor.rid.second, bytesValue, propertyType, isUnique);
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

    void Vertex::destroy(Txn &txn, const RecordDescriptor &recordDescriptor) {
        // transaction validations
        Validate::isTransactionValid(txn);
        auto classDescriptor = Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto classInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
        auto edgeRecordDescriptors = std::vector<RecordDescriptor> {};
        try {
            for (const auto &edge: txn.txnCtx.dbRelation->getEdgeInOut(*txn.txnBase, recordDescriptor.rid)) {
                edgeRecordDescriptors.emplace_back(RecordDescriptor{edge.first, edge.second});
            }
        } catch (Graph::ErrorType &err) {
            if (err != GRAPH_NOEXST_VERTEX)
                throw Error(err, Error::Type::GRAPH);
        }
        // delete edges and relations
        for (const auto &edge: edgeRecordDescriptors) {
            Edge::destroy(txn, edge);
        }
        // delete a record in a datastore
        try {
            auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
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
        txn.txnCtx.dbRelation->deleteVertex(*txn.txnBase, recordDescriptor.rid);
    }

    void Vertex::destroy(Txn &txn, const std::string &className) {
        // transaction validations
        Validate::isTransactionValid(txn);
        auto classDescriptor = Generic::getClassDescriptor(txn, className, ClassType::VERTEX);
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
                                                   TB_INDEXING_PREFIX + std::to_string(indexId) + INDEX_POSITIVE_SUFFIX,
                                                   true, isUnique);
                        auto dataIndexDBHandlerNegative =
                                Datastore::openDbi(dsTxnHandler,
                                                   TB_INDEXING_PREFIX + std::to_string(indexId) + INDEX_NEGATIVE_SUFFIX,
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
        // remove all records in a database
        try {
            auto classDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(classDescriptor->id), true);
            auto cursorHandler = Datastore::CursorHandlerWrapper(dsTxnHandler, classDBHandler);
            auto relationDBHandler = Datastore::openDbi(dsTxnHandler, TB_RELATIONS);
            auto keyValue = Datastore::getNextCursor(cursorHandler.get());
            while (!keyValue.empty()) {
                auto key = Datastore::getKeyAsNumeric<PositionId>(keyValue);
                if (*key != EM_MAXRECNUM) {
                    auto recordDescriptor = RecordDescriptor{classDescriptor->id, *key};
                    recordIds.push_back(recordDescriptor.rid);
                    auto edgeRecordDescriptors = std::vector<RecordDescriptor> {};
                    try {
                        for (const auto &edge: txn.txnCtx.dbRelation->getEdgeInOut(*txn.txnBase, recordDescriptor.rid)) {
                            edgeRecordDescriptors.emplace_back(RecordDescriptor{edge.first, edge.second});
                        }
                    } catch (Graph::ErrorType &err) {
                        if (err != GRAPH_NOEXST_VERTEX) {
                            throw Error(err, Error::Type::GRAPH);
                        }
                    }
                    // delete from relations
                    for (const auto &edge: edgeRecordDescriptors) {
                        Datastore::deleteRecord(dsTxnHandler, relationDBHandler, rid2str(edge.rid));
                        auto edgeClassHandler = Datastore::openDbi(dsTxnHandler, std::to_string(edge.rid.first), true);
                        Datastore::deleteRecord(dsTxnHandler, edgeClassHandler, edge.rid.second);
                    }
                    // delete a record in a datastore
                    //Datastore::deleteCursor(cursorHandler);
                }
                keyValue = Datastore::getNextCursor(cursorHandler.get());
            }
            // empty a database
            Datastore::emptyDbi(dsTxnHandler, classDBHandler);
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
        // update in-memory
        for (const auto &recordId: recordIds) {
            txn.txnCtx.dbRelation->deleteVertex(*txn.txnBase, recordId);
        }
    }

    ResultSet Vertex::get(const Txn &txn, const std::string &className) {
        auto result = ResultSet{};
        auto classDescriptors = Generic::getMultipleClassDescriptor(txn, std::set<std::string>{className}, ClassType::VERTEX);
        for (const auto &classDescriptor: classDescriptors) {
            auto classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
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
            auto classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
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
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Generic::getEdgeNeighbour(txn, recordDescriptor, edgeClassIds, &Graph::getEdgeIn);
    }

    ResultSet Vertex::getOutEdge(const Txn &txn,
                                 const RecordDescriptor &recordDescriptor,
                                 const ClassFilter &classFilter) {
        // basic class verification
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Generic::getEdgeNeighbour(txn, recordDescriptor, edgeClassIds, &Graph::getEdgeOut);
    }

    ResultSet Vertex::getAllEdge(const Txn &txn,
                                 const RecordDescriptor &recordDescriptor,
                                 const ClassFilter &classFilter) {
        // basic class verification
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Generic::getEdgeNeighbour(txn, recordDescriptor, edgeClassIds, &Graph::getEdgeInOut);
    }

    ResultSetCursor Vertex::getInEdgeCursor(Txn &txn,
                                            const RecordDescriptor &recordDescriptor,
                                            const ClassFilter &classFilter) {
        // basic class verification
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
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
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
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
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
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
