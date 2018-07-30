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

#include <vector>

#include "shared_lock.hpp"
#include "constant.hpp"
#include "lmdb_engine.hpp"
#include "graph.hpp"
#include "parser.hpp"
#include "generic.hpp"
#include "compare.hpp"
#include "index.hpp"
#include "utils.hpp"

#include "nogdb_errors.h"
#include "nogdb_compare.h"

namespace nogdb {

//*****************************************************************
//*  compare by condition and multi-condition object              *
//*****************************************************************

    std::vector<RecordDescriptor>
    Compare::getRdescCondition(const Txn &txn, const std::vector<ClassInfo> &classInfos, const Condition &condition,
                               PropertyType type) {
        auto result = std::vector<RecordDescriptor>{};
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        for (const auto &classInfo: classInfos) {
            auto cursorHandler = dsTxnHandler->openCursor(std::to_string(classInfo.id), true);
            auto keyValue = cursorHandler.getNext();
            while (!keyValue.empty()) {
                auto key = keyValue.key.data.numeric<PositionId>();
                if (key != EM_MAXRECNUM) {
                    auto rid = RecordId{classInfo.id, key};
                    auto record = Parser::parseRawDataWithBasicInfo(classInfo.name, rid, keyValue.val, classInfo.propertyInfo);
                    if (condition.comp != Condition::Comparator::IS_NULL &&
                        condition.comp != Condition::Comparator::NOT_NULL) {
                        if (record.get(condition.propName).empty()) {
                            keyValue = cursorHandler.getNext();
                            continue;
                        }
                        if (compareBytesValue(record.get(condition.propName), type, condition)) {
                            result.emplace_back(RecordDescriptor{rid});
                        }
                    } else {
                        switch (condition.comp) {
                            case Condition::Comparator::IS_NULL:
                                if (record.get(condition.propName).empty()) {
                                    result.emplace_back(RecordDescriptor{rid});
                                }
                                break;
                            case Condition::Comparator::NOT_NULL:
                                if (!record.get(condition.propName).empty()) {
                                    result.emplace_back(RecordDescriptor{rid});
                                }
                                break;
                            default:
                                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                        }
                    }
                }
                keyValue = cursorHandler.getNext();
            }
        }
        return result;
    }

    std::vector<RecordDescriptor>
    Compare::getRdescMultiCondition(const Txn &txn,
                                    const std::vector<ClassInfo> &classInfos,
                                    const MultiCondition &conditions,
                                    const PropertyMapType &types) {
        auto result = std::vector<RecordDescriptor>{};
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        for (const auto &classInfo: classInfos) {
            auto cursorHandler = dsTxnHandler->openCursor(std::to_string(classInfo.id), true);
            auto keyValue = cursorHandler.getNext();
            while (!keyValue.empty()) {
                auto key = keyValue.key.data.numeric<PositionId>();
                if (key != EM_MAXRECNUM) {
                    auto rid = RecordId{classInfo.id, key};
                    auto record = Parser::parseRawDataWithBasicInfo(classInfo.name, rid, keyValue.val, classInfo.propertyInfo);
                    if (conditions.execute(record, types)) {
                        result.emplace_back(RecordDescriptor{rid});
                    }
                }
                keyValue = cursorHandler.getNext();
            }
        }
        return result;
    }

    std::vector<RecordDescriptor>
    Compare::getRdescEdgeCondition(const Txn &txn, const RecordDescriptor &recordDescriptor,
                                   const std::vector<ClassId> &edgeClassIds,
                                   std::vector<RecordId> (Graph::*func)(const BaseTxn &txn, const RecordId &rid, const ClassId &classId),
                                   const Condition &condition, PropertyType type) {
        switch (Generic::checkIfRecordExist(txn, recordDescriptor)) {
            case RECORD_NOT_EXIST:
                throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
            case RECORD_NOT_EXIST_IN_MEMORY:
                return std::vector<RecordDescriptor>{};
            default:
                auto result = std::vector<RecordDescriptor>{};
                auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
                try {
                    auto classDescriptor = Schema::ClassDescriptorPtr{};
                    auto classPropertyInfo = ClassPropertyInfo{};
                    auto classDBHandler = storage_engine::lmdb::Dbi{};
                    auto className = std::string{};
                    auto filter = [&condition, &type](const Record &record) {
                        if (condition.comp != Condition::Comparator::IS_NULL &&
                            condition.comp != Condition::Comparator::NOT_NULL) {
                            auto recordValue = record.get(condition.propName);
                            if (recordValue.empty()) {
                                return false;
                            }
                            if (compareBytesValue(recordValue, type, condition)) {
                                return true;
                            }
                        } else {
                            switch (condition.comp) {
                                case Condition::Comparator::IS_NULL:
                                    if (record.get(condition.propName).empty()) {
                                        return true;
                                    }
                                    break;
                                case Condition::Comparator::NOT_NULL:
                                    if (!record.get(condition.propName).empty()) {
                                        return true;
                                    }
                                    break;
                                default:
                                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                            }
                        }
                        return false;
                    };
                    auto retrieve = [&](std::vector<RecordDescriptor> &result, const RecordId &edge) {
                        if (classDescriptor == nullptr || classDescriptor->id != edge.first) {
                            classDescriptor = Generic::getClassDescriptor(txn, edge.first, ClassType::UNDEFINED);
                            classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
                            classDBHandler = dsTxnHandler->openDbi(std::to_string(edge.first), true);
                            className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
                        }
                        auto keyValue = classDBHandler.get(edge.second);
                        auto record = Parser::parseRawDataWithBasicInfo(className, edge, keyValue, classPropertyInfo);
                        if (filter(record)) {
                            result.emplace_back(RecordDescriptor{edge});
                        }
                    };
                    if (edgeClassIds.empty()) {
                        for (const auto &edge: ((*txn.txnCtx.dbRelation).*func)(*txn.txnBase, recordDescriptor.rid, 0)) {
                            retrieve(result, edge);
                        }
                    } else {
                        for (const auto &edgeId: edgeClassIds) {
                            for (const auto &edge: ((*txn.txnCtx.dbRelation).*func)(*txn.txnBase, recordDescriptor.rid, edgeId)) {
                                retrieve(result, edge);
                            }
                        }
                    }
                } catch (const Error &err) {
                    if (err.code() == NOGDB_GRAPH_NOEXST_VERTEX) {
                        throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_UNKNOWN_ERR);
                    } else {
                        throw err;
                    }
                }
                return result;
        }
    }

    std::vector<RecordDescriptor>
    Compare::getRdescEdgeMultiCondition(const Txn &txn, const RecordDescriptor &recordDescriptor,
                                        const std::vector<ClassId> &edgeClassIds,
                                        std::vector<RecordId>
                                        (Graph::*func)(const BaseTxn &baseTxn, const RecordId &rid, const ClassId &classId),
                                        const MultiCondition &conditions, const PropertyMapType &types) {
        switch (Generic::checkIfRecordExist(txn, recordDescriptor)) {
            case RECORD_NOT_EXIST:
                throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
            case RECORD_NOT_EXIST_IN_MEMORY:
                return std::vector<RecordDescriptor>{};
            default:
                auto result = std::vector<RecordDescriptor>{};
                auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
                try {
                    auto classDescriptor = Schema::ClassDescriptorPtr{};
                    auto classPropertyInfo = ClassPropertyInfo{};
                    auto classDBHandler = storage_engine::lmdb::Dbi{};
                    auto className = std::string{};
                    auto retrieve = [&](std::vector<RecordDescriptor> &result, const RecordId &edge) {
                        if (classDescriptor == nullptr || classDescriptor->id != edge.first) {
                            classDescriptor = Generic::getClassDescriptor(txn, edge.first, ClassType::UNDEFINED);
                            classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
                            classDBHandler = dsTxnHandler->openDbi(std::to_string(edge.first), true);
                            className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
                        }
                        auto keyValue = classDBHandler.get(edge.second);
                        auto record = Parser::parseRawDataWithBasicInfo(className, edge, keyValue, classPropertyInfo);
                        if (conditions.execute(record, types)) {
                            result.emplace_back(RecordDescriptor{edge});
                        }
                    };
                    if (edgeClassIds.empty()) {
                        for (const auto &edge: ((*txn.txnCtx.dbRelation).*func)(*txn.txnBase, recordDescriptor.rid, 0)) {
                            retrieve(result, edge);
                        }
                    } else {
                        for (const auto &edgeId: edgeClassIds) {
                            for (const auto &edge: ((*txn.txnCtx.dbRelation).*func)(*txn.txnBase, recordDescriptor.rid, edgeId)) {
                                retrieve(result, edge);
                            }
                        }
                    }
                } catch (const Error &err) {
                    if (err.code() == NOGDB_GRAPH_NOEXST_VERTEX) {
                        throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_UNKNOWN_ERR);
                    } else {
                        throw err;
                    }
                }
                return result;
        }
    }

    std::vector<RecordDescriptor>
    Compare::compareConditionRdesc(const Txn &txn, const std::string &className, ClassType type,
                                   const Condition &condition, bool searchIndexOnly) {
        auto propertyType = PropertyType::UNDEFINED;
        auto classDescriptors = Generic::getMultipleClassDescriptor(txn, std::set<std::string>{className}, type);
        auto classInfos = Generic::getMultipleClassMapProperty(*txn.txnBase, classDescriptors);
        for (const auto &classInfo: classInfos) {
            auto propertyInfo = classInfo.propertyInfo.nameToDesc.find(condition.propName);
            if (propertyInfo != classInfo.propertyInfo.nameToDesc.cend()) {
                if (propertyType == PropertyType::UNDEFINED) {
                    propertyType = propertyInfo->second.type;
                } else {
                    if (propertyType != propertyInfo->second.type) {
                        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_CONFLICT_PROPTYPE);
                    }
                }
            }
        }
        if (propertyType == PropertyType::UNDEFINED) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
        }
        //TODO: temporary fix indexing errors
//        auto foundClassId = std::find_if(classDescriptors.cbegin(), classDescriptors.cend(),
//                                         [&txn, &className](const Schema::ClassDescriptorPtr& ptr) {
//            return BaseTxn::getCurrentVersion(*txn.txnBase, ptr->name).first == className;
//        });
//        auto &classId = (*foundClassId)->id;
//        auto foundIndex = Index::hasIndex(classId, *classInfos.cbegin(), condition);
//        if (foundIndex.second) {
//            return Index::getIndexRecord(txn, classId, foundIndex.first, condition);
//        }
//        if (searchIndexOnly) {
//            return std::vector<RecordDescriptor>{};
//        } else {
            return getRdescCondition(txn, classInfos, condition, propertyType);
//        }
    }

    std::vector<RecordDescriptor>
    Compare::compareMultiConditionRdesc(const Txn &txn, const std::string &className, ClassType type,
                                        const MultiCondition &conditions, bool searchIndexOnly) {
        // check if all conditions are valid
        auto conditionPropertyTypes = PropertyMapType{};
        for (const auto &conditionNode: conditions.conditions) {
            auto conditionNodePtr = conditionNode.lock();
            require(conditionNodePtr != nullptr);
            auto &condition = conditionNodePtr->getCondition();
            conditionPropertyTypes.emplace(condition.propName, PropertyType::UNDEFINED);
        }
        require(!conditionPropertyTypes.empty());

        auto classDescriptors = Generic::getMultipleClassDescriptor(txn, std::set<std::string>{className}, type);
        auto classInfos = Generic::getMultipleClassMapProperty(*txn.txnBase, classDescriptors);
        auto numOfUndefPropertyType = conditionPropertyTypes.size();
        for (const auto &classInfo: classInfos) {
            for (auto &property: conditionPropertyTypes) {
                auto propertyInfo = classInfo.propertyInfo.nameToDesc.find(property.first);
                if (propertyInfo != classInfo.propertyInfo.nameToDesc.cend()) {
                    if (property.second == PropertyType::UNDEFINED) {
                        property.second = propertyInfo->second.type;
                        --numOfUndefPropertyType;
                    } else {
                        if (property.second != propertyInfo->second.type) {
                            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_CONFLICT_PROPTYPE);
                        }
                    }
                }
            }
        }
        if (numOfUndefPropertyType != 0) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
        }
        //TODO: temporary fix indexing errors
//        auto foundClassId = std::find_if(classDescriptors.cbegin(), classDescriptors.cend(),
//                                         [&txn, &className](const Schema::ClassDescriptorPtr& ptr) {
//            return BaseTxn::getCurrentVersion(*txn.txnBase, ptr->name).first == className;
//        });
//        auto &classId = (*foundClassId)->id;
//        auto foundIndex = Index::hasIndex(classId, *classInfos.cbegin(), conditions);
//        if (foundIndex.second) {
//            return Index::getIndexRecord(txn, classId, foundIndex.first, conditions);
//        }
//        if (searchIndexOnly) {
//            return std::vector<RecordDescriptor>{};
//        } else {
            return getRdescMultiCondition(txn, classInfos, conditions, conditionPropertyTypes);
//        }
    }

    std::vector<RecordDescriptor>
    Compare::compareEdgeConditionRdesc(const Txn &txn, const RecordDescriptor &recordDescriptor,
                                       std::vector<RecordId>
                                       (Graph::*func1)(const BaseTxn &baseTxn, const RecordId &rid, const ClassId &classId),
                                       std::vector<ClassId>
                                       (Graph::*func2)(const BaseTxn &baseTxn, const RecordId &rid),
                                       const Condition &condition, const ClassFilter &classFilter) {
        auto classDescriptor = Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto propertyType = PropertyType::UNDEFINED;
        auto edgeClassIds = std::vector<ClassId>{};
        auto validateProperty = [&](const std::vector<ClassInfo> &classInfos, const std::string &propName) {
            auto isFoundProperty = false;
            for (const auto &classInfo: classInfos) {
                edgeClassIds.push_back(classInfo.id);
                auto propertyInfo = classInfo.propertyInfo.nameToDesc.find(propName);
                if (propertyInfo != classInfo.propertyInfo.nameToDesc.cend()) {
                    if (propertyType == PropertyType::UNDEFINED) {
                        propertyType = propertyInfo->second.type;
                        isFoundProperty = true;
                    } else {
                        if (propertyType != propertyInfo->second.type) {
                            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_CONFLICT_PROPTYPE);
                        }
                    }
                }
            }
            return isFoundProperty;
        };

        auto edgeClassDescriptors = Generic::getMultipleClassDescriptor(txn, classFilter.getClassName(), ClassType::EDGE);
        if (!edgeClassDescriptors.empty()) {
            auto edgeClassInfos = Generic::getMultipleClassMapProperty(*txn.txnBase, edgeClassDescriptors);
            if (!validateProperty(edgeClassInfos, condition.propName)) {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
            }
        } else {
            auto edgeClassIds = ((*txn.txnCtx.dbRelation).*func2)(*txn.txnBase, recordDescriptor.rid);
            auto edgeClassDescriptors = Generic::getMultipleClassDescriptor(txn, edgeClassIds, ClassType::EDGE);
            auto edgeClassInfos = Generic::getMultipleClassMapProperty(*txn.txnBase, edgeClassDescriptors);
            if (!validateProperty(edgeClassInfos, condition.propName)) {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
            }
        }
        return getRdescEdgeCondition(txn, recordDescriptor, edgeClassIds, func1, condition, propertyType);
    }

    std::vector<RecordDescriptor>
    Compare::compareEdgeMultiConditionRdesc(const Txn &txn, const RecordDescriptor &recordDescriptor,
                                            std::vector<RecordId>
                                            (Graph::*func1)(const BaseTxn &baseTxn, const RecordId &rid, const ClassId &classId),
                                            std::vector<ClassId>
                                            (Graph::*func2)(const BaseTxn &baseTxn, const RecordId &rid),
                                            const MultiCondition &conditions, const ClassFilter &classFilter) {
        // check if all conditions are valid
        auto conditionPropertyTypes = PropertyMapType{};
        for (const auto &conditionNode: conditions.conditions) {
            auto conditionNodePtr = conditionNode.lock();
            require(conditionNodePtr != nullptr);
            auto &condition = conditionNodePtr->getCondition();
            conditionPropertyTypes.emplace(condition.propName, PropertyType::UNDEFINED);
        }
        require(!conditionPropertyTypes.empty());

        auto classDescriptor = Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = std::vector<ClassId> {};
        auto validateAndResolveProperties = [&](const std::vector<ClassInfo> &classInfos,
                                                PropertyMapType &propertyTypes) {
            auto numOfUndefPropertyType = propertyTypes.size();
            for (const auto &classInfo: classInfos) {
                edgeClassIds.push_back(classInfo.id);
                for (auto &property: propertyTypes) {
                    auto propertyInfo = classInfo.propertyInfo.nameToDesc.find(property.first);
                    if (propertyInfo != classInfo.propertyInfo.nameToDesc.cend()) {
                        if (property.second == PropertyType::UNDEFINED) {
                            property.second = propertyInfo->second.type;
                            --numOfUndefPropertyType;
                        } else {
                            if (property.second != propertyInfo->second.type) {
                                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_CONFLICT_PROPTYPE);
                            }
                        }
                    }
                }
            }
            return numOfUndefPropertyType == 0;
        };

        auto edgeClassDescriptors = Generic::getMultipleClassDescriptor(txn, classFilter.getClassName(), ClassType::EDGE);
        if (!edgeClassDescriptors.empty()) {
            auto edgeClassInfos = Generic::getMultipleClassMapProperty(*txn.txnBase, edgeClassDescriptors);
            if (!validateAndResolveProperties(edgeClassInfos, conditionPropertyTypes)) {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
            }
        } else {
            auto edgeClassIds = ((*txn.txnCtx.dbRelation).*func2)(*txn.txnBase, recordDescriptor.rid);
            auto edgeClassDescriptors = Generic::getMultipleClassDescriptor(txn, edgeClassIds, ClassType::EDGE);
            auto edgeClassInfos = Generic::getMultipleClassMapProperty(*txn.txnBase, edgeClassDescriptors);
            if (!validateAndResolveProperties(edgeClassInfos, conditionPropertyTypes)) {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
            }
        }
        return getRdescEdgeMultiCondition(txn, recordDescriptor, edgeClassIds, func1, conditions, conditionPropertyTypes);
    }

//*****************************************************************
//*  compare by a conditional function                            *
//*****************************************************************

    std::vector<RecordDescriptor>
    Compare::getRdescCondition(const Txn &txn, const std::vector<ClassInfo> &classInfos,
                               bool (*condition)(const Record &record)) {
        auto result = std::vector<RecordDescriptor>{};
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        for (const auto &classInfo: classInfos) {
            auto cursorHandler = dsTxnHandler->openCursor(std::to_string(classInfo.id), true);
            auto keyValue = cursorHandler.getNext();
            while (!keyValue.empty()) {
                auto key = keyValue.key.data.numeric<PositionId>();
                if (key != EM_MAXRECNUM) {
                    auto rid = RecordId{classInfo.id, key};
                    auto record = Parser::parseRawDataWithBasicInfo(classInfo.name, rid, keyValue.val, classInfo.propertyInfo);
                    if ((*condition)(record)) {
                        result.push_back(RecordDescriptor{rid});
                    }
                }
                keyValue = cursorHandler.getNext();
            }
        }
        return result;
    }

    std::vector<RecordDescriptor>
    Compare::compareConditionRdesc(const Txn &txn, const std::string &className, ClassType type,
                                   bool (*condition)(const Record &)) {
        auto classDescriptors = Generic::getMultipleClassDescriptor(txn, std::set<std::string>{className}, type);
        auto classInfos = Generic::getMultipleClassMapProperty(*txn.txnBase, classDescriptors);
        return getRdescCondition(txn, classInfos, condition);
    }

    std::vector<RecordDescriptor>
    Compare::getRdescEdgeCondition(const Txn &txn, const RecordDescriptor &recordDescriptor,
                                   const std::vector<ClassId> &edgeClassIds,
                                   std::vector<RecordId>
                                   (Graph::*func)(const BaseTxn &baseTxn, const RecordId &rid, const ClassId &classId),
                                   bool (*condition)(const Record &record)) {
        switch (Generic::checkIfRecordExist(txn, recordDescriptor)) {
            case RECORD_NOT_EXIST:
                throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
            case RECORD_NOT_EXIST_IN_MEMORY:
                return std::vector<RecordDescriptor>{};
            default:
                auto result = std::vector<RecordDescriptor>{};
                auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
                try {
                    auto classDescriptor = Schema::ClassDescriptorPtr{};
                    auto classPropertyInfo = ClassPropertyInfo{};
                    auto classDBHandler = storage_engine::lmdb::Dbi{};
                    auto className = std::string{};
                    auto retrieve = [&](std::vector<RecordDescriptor> &result, const RecordId &edge) {
                        if (classDescriptor == nullptr || classDescriptor->id != edge.first) {
                            classDescriptor = Generic::getClassDescriptor(txn, edge.first, ClassType::UNDEFINED);
                            classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
                            classDBHandler = dsTxnHandler->openDbi(std::to_string(edge.first), true);
                            className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
                        }
                        auto keyValue = classDBHandler.get(edge.second);
                        auto record = Parser::parseRawDataWithBasicInfo(className, edge, keyValue, classPropertyInfo);
                        if ((*condition)(record)) {
                            result.emplace_back(RecordDescriptor{edge});
                        }
                    };
                    if (edgeClassIds.empty()) {
                        for (const auto &edge: ((*txn.txnCtx.dbRelation).*func)(*txn.txnBase, recordDescriptor.rid, 0)) {
                            retrieve(result, edge);
                        }
                    } else {
                        for (const auto &edgeId: edgeClassIds) {
                            for (const auto &edge: ((*txn.txnCtx.dbRelation).*func)(*txn.txnBase, recordDescriptor.rid, edgeId)) {
                                retrieve(result, edge);
                            }
                        }
                    }
                } catch (const Error &err) {
                    if (err.code() == NOGDB_GRAPH_NOEXST_VERTEX) {
                        throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_UNKNOWN_ERR);
                    } else {
                        throw err;
                    }
                }
                return result;
        }
    }

    std::vector<RecordDescriptor>
    Compare::compareEdgeConditionRdesc(const Txn &txn, const RecordDescriptor &recordDescriptor,
                                       std::vector<RecordId>
                                       (Graph::*func1)(const BaseTxn &baseTxn, const RecordId &rid, const ClassId &classId),
                                       std::vector<ClassId>
                                       (Graph::*func2)(const BaseTxn &baseTxn, const RecordId &rid),
                                       bool (*condition)(const Record &), const ClassFilter &classFilter) {
        auto classDescriptor = Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = std::vector<ClassId>{};
        auto edgeClassDescriptors = Generic::getMultipleClassDescriptor(txn, classFilter.getClassName(), ClassType::EDGE);
        if (!edgeClassDescriptors.empty()) {
            auto edgeClassInfos = Generic::getMultipleClassMapProperty(*txn.txnBase, edgeClassDescriptors);
            for (const auto &classInfo: edgeClassInfos) {
                edgeClassIds.push_back(classInfo.id);
            }
        } else {
            auto edgeClassIds = ((*txn.txnCtx.dbRelation).*func2)(*txn.txnBase, recordDescriptor.rid);
            auto edgeClassDescriptors = Generic::getMultipleClassDescriptor(txn, edgeClassIds, ClassType::EDGE);
            auto edgeClassInfos = Generic::getMultipleClassMapProperty(*txn.txnBase, edgeClassDescriptors);
            for (const auto &classInfo: edgeClassInfos) {
                edgeClassIds.push_back(classInfo.id);
            }
        }
        return getRdescEdgeCondition(txn, recordDescriptor, edgeClassIds, func1, condition);
    }

}
