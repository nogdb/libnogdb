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
#include "parser.hpp"
#include "relation.hpp"
#include "compare.hpp"
#include "index.hpp"
#include "datarecord.hpp"

#include "nogdb.h"

namespace nogdb {

    const RecordDescriptor Vertex::create(const Txn &txn, const std::string &className, const Record &record) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto vertexClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::VERTEX);
        auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
        auto recordBlob = parser::Parser::parseRecord(record, propertyNameMapInfo);
        try {
            auto vertexDataRecord = adapter::datarecord::DataRecord(txn._txnBase, vertexClassInfo.id, ClassType::VERTEX);
            auto positionId = vertexDataRecord.insert(recordBlob);
            auto recordDescriptor = RecordDescriptor{vertexClassInfo.id, positionId};
            txn._iIndex->insert(recordDescriptor, record, propertyNameMapInfo);
            return recordDescriptor;
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }
    }

    void Vertex::update(const Txn &txn, const RecordDescriptor &recordDescriptor, const Record &record) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
        auto vertexDataRecord = adapter::datarecord::DataRecord(txn._txnBase, vertexClassInfo.id, ClassType::VERTEX);
        auto existingRecordResult = vertexDataRecord.getResult(recordDescriptor.rid.second);
        auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
        auto newRecordBlob = parser::Parser::parseRecord(record, propertyNameMapInfo);
        try {
            // insert an updated record
            vertexDataRecord.insert(newRecordBlob);
            // remove index if applied in existing record
            auto propertyIdMapInfo = txn._iSchema->getPropertyIdMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
            auto existingRecord = parser::Parser::parseRawData(existingRecordResult, propertyIdMapInfo, true);
            txn._iIndex->remove(recordDescriptor, existingRecord, propertyNameMapInfo);
            // add index if applied in new record
            txn._iIndex->insert(recordDescriptor, record, propertyNameMapInfo);
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }
    }

    void Vertex::destroy(const Txn &txn, const RecordDescriptor &recordDescriptor) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
        auto vertexDataRecord = adapter::datarecord::DataRecord(txn._txnBase, vertexClassInfo.id, ClassType::VERTEX);
        auto recordResult = vertexDataRecord.getResult(recordDescriptor.rid.second);
        try {
            auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
            auto propertyIdMapInfo = txn._iSchema->getPropertyIdMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
            vertexDataRecord.remove(recordDescriptor.rid.second);
            txn._iGraph->removeRelFromVertex(recordDescriptor.rid);
            // remove index if applied in the record
            auto record = parser::Parser::parseRawData(recordResult, propertyIdMapInfo, true);
            txn._iIndex->remove(recordDescriptor, record, propertyNameMapInfo);
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }
    }

    void Vertex::destroy(const Txn &txn, const std::string &className) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto vertexClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::VERTEX);
        try {
            auto vertexDataRecord = adapter::datarecord::DataRecord(txn._txnBase, vertexClassInfo.id, ClassType::VERTEX);
            auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(vertexClassInfo.id, vertexClassInfo.superClassId);
            auto result = std::map<RecordId, std::pair<RecordId, RecordId>>{};
            std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
                    [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto recordId = RecordId{vertexClassInfo.id, positionId};
                txn._iGraph->removeRelFromVertex(recordId);
            };
            vertexDataRecord.resultSetIter(callback);
            vertexDataRecord.destroy();
            txn._iIndex->drop(vertexClassInfo.id, propertyNameMapInfo);
        } catch (const Error& error) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(error);
        }
    }

    ResultSet Vertex::get(const Txn &txn, const std::string &className) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto vertexClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::VERTEX);
        return txn._iRecord->getResultSet(vertexClassInfo);
    }

    ResultSet Vertex::getExtend(const Txn &txn, const std::string &className) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto vertexClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::VERTEX);
        return txn._iRecord->getResultSetExtend(vertexClassInfo);
    }


    ResultSetCursor Vertex::getCursor(const Txn &txn, const std::string &className) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto vertexClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::VERTEX);
        return txn._iRecord->getResultSetCursor(vertexClassInfo);
    }

    ResultSetCursor Vertex::getExtendCursor(const Txn &txn, const std::string &className) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto vertexClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::VERTEX);
        return txn._iRecord->getResultSetCursorExtend(vertexClassInfo);
    }

    ResultSet Vertex::getInEdge(const Txn &txn,
                                const RecordDescriptor &recordDescriptor,
                                const ClassFilter &classFilter) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeRecordIds = txn._iGraph->getInEdges(recordDescriptor.rid);
        auto edgeClassIds = std::set<ClassId>{};
        for(const auto& className: classFilter.getClassName()) {
            auto classId = txn._class->getId(className);
            edgeClassIds.insert(classId);
        }
        auto result = ResultSet{};
        for(const auto& recordId: edgeRecordIds) {
            auto edgeRecordDescriptor = RecordDescriptor{recordId};
            if (!edgeClassIds.empty()) {
                if (edgeClassIds.find(recordId.first) != edgeClassIds.cend()) {
                    auto edgeClassInfo = txn._iSchema->getExistingClass(edgeRecordDescriptor.rid.first);
                    auto edgeRecord = txn._iRecord->getRecord(edgeClassInfo, edgeRecordDescriptor);
                    result.emplace_back(Result{edgeRecordDescriptor, edgeRecord});
                }
            } else {
                auto edgeClassInfo = txn._iSchema->getExistingClass(edgeRecordDescriptor.rid.first);
                auto edgeRecord = txn._iRecord->getRecord(edgeClassInfo, edgeRecordDescriptor);
                result.emplace_back(Result{edgeRecordDescriptor, edgeRecord});
            }
        }
        return result;
    }

    ResultSet Vertex::getOutEdge(const Txn &txn,
                                 const RecordDescriptor &recordDescriptor,
                                 const ClassFilter &classFilter) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeRecordIds = txn._iGraph->getOutEdges(recordDescriptor.rid);
        auto edgeClassIds = std::set<ClassId>{};
        for(const auto& className: classFilter.getClassName()) {
            auto classId = txn._class->getId(className);
            edgeClassIds.insert(classId);
        }
        auto result = ResultSet{};
        for(const auto& recordId: edgeRecordIds) {
            auto edgeRecordDescriptor = RecordDescriptor{recordId};
            if (!edgeClassIds.empty()) {
                if (edgeClassIds.find(recordId.first) != edgeClassIds.cend()) {
                    auto edgeClassInfo = txn._iSchema->getExistingClass(edgeRecordDescriptor.rid.first);
                    auto edgeRecord = txn._iRecord->getRecord(edgeClassInfo, edgeRecordDescriptor);
                    result.emplace_back(Result{edgeRecordDescriptor, edgeRecord});
                }
            } else {
                auto edgeClassInfo = txn._iSchema->getExistingClass(edgeRecordDescriptor.rid.first);
                auto edgeRecord = txn._iRecord->getRecord(edgeClassInfo, edgeRecordDescriptor);
                result.emplace_back(Result{edgeRecordDescriptor, edgeRecord});
            }
        }
        return result;
    }

    ResultSet Vertex::getAllEdge(const Txn &txn,
                                 const RecordDescriptor &recordDescriptor,
                                 const ClassFilter &classFilter) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeRecordIds = std::set<RecordId>{};
        auto inEdgeRecordIds = txn._iGraph->getInEdges(recordDescriptor.rid);
        auto outEdgeRecordIds = txn._iGraph->getOutEdges(recordDescriptor.rid);
        edgeRecordIds.insert(inEdgeRecordIds.cbegin(), inEdgeRecordIds.cend());
        edgeRecordIds.insert(outEdgeRecordIds.cbegin(), outEdgeRecordIds.cend());
        auto edgeClassIds = std::set<ClassId>{};
        for(const auto& className: classFilter.getClassName()) {
            auto classId = txn._class->getId(className);
            edgeClassIds.insert(classId);
        }
        auto result = ResultSet{};
        for(const auto& recordId: edgeRecordIds) {
            auto edgeRecordDescriptor = RecordDescriptor{recordId};
            if (!edgeClassIds.empty()) {
                if (edgeClassIds.find(recordId.first) != edgeClassIds.cend()) {
                    auto edgeClassInfo = txn._iSchema->getExistingClass(edgeRecordDescriptor.rid.first);
                    auto edgeRecord = txn._iRecord->getRecord(edgeClassInfo, edgeRecordDescriptor);
                    result.emplace_back(Result{edgeRecordDescriptor, edgeRecord});
                }
            } else {
                auto edgeClassInfo = txn._iSchema->getExistingClass(edgeRecordDescriptor.rid.first);
                auto edgeRecord = txn._iRecord->getRecord(edgeClassInfo, edgeRecordDescriptor);
                result.emplace_back(Result{edgeRecordDescriptor, edgeRecord});
            }
        }
        return result;
    }

    ResultSetCursor Vertex::getInEdgeCursor(const Txn &txn,
                                            const RecordDescriptor &recordDescriptor,
                                            const ClassFilter &classFilter) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeRecordIds = txn._iGraph->getInEdges(recordDescriptor.rid);
        auto edgeClassIds = std::set<ClassId>{};
        for(const auto& className: classFilter.getClassName()) {
            auto classId = txn._class->getId(className);
            edgeClassIds.insert(classId);
        }
        auto result = ResultSetCursor{txn};
        for(const auto& recordId: edgeRecordIds) {
            auto edgeRecordDescriptor = RecordDescriptor{recordId};
            if (!edgeClassIds.empty()) {
                if (edgeClassIds.find(recordId.first) != edgeClassIds.cend()) {
                    result.metadata.emplace_back(edgeRecordDescriptor);
                }
            } else {
                result.metadata.emplace_back(edgeRecordDescriptor);
            }
        }
        return result;
    }

    ResultSetCursor Vertex::getOutEdgeCursor(const Txn &txn,
                                             const RecordDescriptor &recordDescriptor,
                                             const ClassFilter &classFilter) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeRecordIds = txn._iGraph->getOutEdges(recordDescriptor.rid);
        auto edgeClassIds = std::set<ClassId>{};
        for(const auto& className: classFilter.getClassName()) {
            auto classId = txn._class->getId(className);
            edgeClassIds.insert(classId);
        }
        auto result = ResultSetCursor{txn};
        for(const auto& recordId: edgeRecordIds) {
            auto edgeRecordDescriptor = RecordDescriptor{recordId};
            if (!edgeClassIds.empty()) {
                if (edgeClassIds.find(recordId.first) != edgeClassIds.cend()) {
                    result.metadata.emplace_back(edgeRecordDescriptor);
                }
            } else {
                result.metadata.emplace_back(edgeRecordDescriptor);
            }
        }
        return result;
    }

    ResultSetCursor Vertex::getAllEdgeCursor(const Txn &txn,
                                             const RecordDescriptor &recordDescriptor,
                                             const ClassFilter &classFilter) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeRecordIds = std::set<RecordId>{};
        auto inEdgeRecordIds = txn._iGraph->getInEdges(recordDescriptor.rid);
        auto outEdgeRecordIds = txn._iGraph->getOutEdges(recordDescriptor.rid);
        edgeRecordIds.insert(inEdgeRecordIds.cbegin(), inEdgeRecordIds.cend());
        edgeRecordIds.insert(outEdgeRecordIds.cbegin(), outEdgeRecordIds.cend());
        auto edgeClassIds = std::set<ClassId>{};
        for(const auto& className: classFilter.getClassName()) {
            auto classId = txn._class->getId(className);
            edgeClassIds.insert(classId);
        }
        auto result = ResultSetCursor{txn};
        for(const auto& recordId: edgeRecordIds) {
            auto edgeRecordDescriptor = RecordDescriptor{recordId};
            if (!edgeClassIds.empty()) {
                if (edgeClassIds.find(recordId.first) != edgeClassIds.cend()) {
                    result.metadata.emplace_back(edgeRecordDescriptor);
                }
            } else {
                result.metadata.emplace_back(edgeRecordDescriptor);
            }
        }
        return result;
    }

    ResultSet Vertex::get(const Txn &txn, const std::string &className, const Condition &condition) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto edgeClassInfo = txn._iSchema->getValidClassInfo(className, ClassType::VERTEX);
        auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
        return Compare::compareCondition(txn, edgeClassInfo, propertyNameMapInfo, condition);
    }

    //TODO: complete all functions below
    ResultSet Vertex::get(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
        return Compare::compareCondition(txn, className, ClassType::VERTEX, condition);
    }

    ResultSet Vertex::get(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
        return Compare::compareMultiCondition(txn, className, ClassType::VERTEX, multiCondition);
    }

    ResultSet Vertex::getExtend(const Txn &txn, const std::string &className, const Condition &condition) {
        return Compare::compareCondition(txn, className, ClassType::VERTEX, condition);
    }

    ResultSet Vertex::getExtend(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
        return Compare::compareCondition(txn, className, ClassType::VERTEX, condition);
    }

    ResultSet Vertex::getExtend(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
        return Compare::compareMultiCondition(txn, className, ClassType::VERTEX, multiCondition);
    }

    ResultSetCursor Vertex::getCursor(const Txn &txn, const std::string &className, const Condition &condition) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::VERTEX, condition);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;

    }

    ResultSetCursor Vertex::getCursor(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::VERTEX, condition);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Vertex::getCursor(const Txn &txn, const std::string &className, const MultiCondition &exp) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareMultiConditionRdesc(txn, className, ClassType::VERTEX, exp);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Vertex::getExtendCursor(const Txn &txn, const std::string &className, const Condition &condition) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::VERTEX, condition);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;

    }

    ResultSetCursor Vertex::getExtendCursor(const Txn &txn, const std::string &className, bool (*condition)(const Record &)) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::VERTEX, condition);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Vertex::getExtendCursor(const Txn &txn, const std::string &className, const MultiCondition &exp) {
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

    ResultSetCursor Vertex::getInEdgeCursor(const Txn &txn,
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

    ResultSetCursor Vertex::getInEdgeCursor(const Txn &txn,
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

    ResultSetCursor Vertex::getInEdgeCursor(const Txn &txn,
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

    ResultSetCursor Vertex::getOutEdgeCursor(const Txn &txn,
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

    ResultSetCursor Vertex::getOutEdgeCursor(const Txn &txn,
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

    ResultSetCursor Vertex::getOutEdgeCursor(const Txn &txn,
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

    ResultSetCursor Vertex::getAllEdgeCursor(const Txn &txn,
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

    ResultSetCursor Vertex::getAllEdgeCursor(const Txn &txn,
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

    ResultSetCursor Vertex::getAllEdgeCursor(const Txn &txn,
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

    ResultSetCursor Vertex::getIndexCursor(const Txn &txn, const std::string &className, const Condition &condition) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::VERTEX, condition, true);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Vertex::getIndexCursor(const Txn &txn, const std::string &className, const MultiCondition &exp) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareMultiConditionRdesc(txn, className, ClassType::VERTEX, exp, true);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSet Vertex::getExtendIndex(const Txn &txn, const std::string &className, const Condition &condition) {
        return Compare::compareCondition(txn, className, ClassType::VERTEX, condition, true);
    }

    ResultSet Vertex::getExtendIndex(const Txn &txn, const std::string &className, const MultiCondition &multiCondition) {
        return Compare::compareMultiCondition(txn, className, ClassType::VERTEX, multiCondition, true);
    }

    ResultSetCursor Vertex::getExtendIndexCursor(const Txn &txn, const std::string &className, const Condition &condition) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareConditionRdesc(txn, className, ClassType::VERTEX, condition, true);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Vertex::getExtendIndexCursor(const Txn &txn, const std::string &className, const MultiCondition &exp) {
        auto result = ResultSetCursor{txn};
        auto metadata = Compare::compareMultiConditionRdesc(txn, className, ClassType::VERTEX, exp, true);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

}
