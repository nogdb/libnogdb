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

#pragma once

#include <regex>
#include <utility>

#include "schema.hpp"
#include "index.hpp"
#include "datarecord.hpp"
#include "relation.hpp"

#include "nogdb_types.h"
#include "nogdb_compare.h"
#include "nogdb.h"

namespace nogdb {

    using namespace utils::assertion;
    using adapter::relation::Direction;

    class Compare {
    public:
        Compare() = delete;

        ~Compare() noexcept = delete;

        static bool compareBytesValue(const Bytes &value, PropertyType type, const Condition &condition) {
            if (condition.comp == Condition::Comparator::IN) {
                for (const auto &valueBytes: condition.valueSet) {
                    if (genericCompareFunc(value, type, valueBytes, Bytes{}, Condition::Comparator::EQUAL, condition.isIgnoreCase) ^
                        condition.isNegative) {
                        return true;
                    }
                }
            } else if (condition.comp >= Condition::Comparator::BETWEEN &&
                       condition.comp <= Condition::Comparator::BETWEEN_NO_BOUND) {
                return genericCompareFunc(value, type, condition.valueSet[0], condition.valueSet[1], condition.comp, condition.isIgnoreCase) ^
                       condition.isNegative;
            } else {
                return genericCompareFunc(value, type, condition.valueBytes, Bytes{}, condition.comp, condition.isIgnoreCase) ^
                       condition.isNegative;
            }
            return false;
        }

        static bool compareRecordByCondition(const Record& record, const PropertyType& propertyType, const Condition& condition) {
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
                    if (!record.get(condition.propName).empty()) {
                        if (compareBytesValue(record.get(condition.propName), propertyType, condition)) {
                            return false;
                        }
                    }
                    break;
            }
            return false;
        }

        static std::vector<Record> resolveEdgeRecordIds(const Txn &txn, const RecordId& recordId, const Direction& direction) {
            auto edgeRecordIds = std::vector<RecordId>{};
            switch (direction) {
                case Direction::IN :
                    edgeRecordIds = txn._iGraph->getInEdges(recordId);
                    break;
                case Direction::OUT :
                    edgeRecordIds = txn._iGraph->getOutEdges(recordId);
                    break;
                default:
                    edgeRecordIds = txn._iGraph->getInEdges(recordId);
                    auto outEdges = txn._iGraph->getOutEdges(recordId);
                    edgeRecordIds.insert(edgeRecordIds.cend(), outEdges.cbegin(), outEdges.cend());
                    break;
            }
            return edgeRecordIds;
        }

        static ResultSet compareCondition(const Txn &txn,
                                          const schema::ClassAccessInfo& classInfo,
                                          const schema::PropertyNameMapInfo& propertyNameMapInfo,
                                          const Condition &condition,
                                          bool searchIndexOnly = false) {
            auto foundProperty = propertyNameMapInfo.find(condition.propName);
            if (foundProperty == propertyNameMapInfo.cend()) {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
            }
            auto propertyInfo = foundProperty->second;
            auto foundIndex = txn._iIndex->hasIndex(classInfo, propertyInfo, condition);
            if (foundIndex.first) {
                auto indexedRecords = txn._iIndex->getRecord(propertyInfo, foundIndex.second, condition);
                return txn._iRecord->getResultSet(classInfo, indexedRecords);
            } else {
                if (!searchIndexOnly) {
                    return txn._iRecord->getResultSetByCondition(classInfo, propertyInfo.type, condition);
                }
            }
            return ResultSet{};
        }

        static ResultSet compareMultiCondition(const Txn &txn,
                                               const schema::ClassAccessInfo& classInfo,
                                               const schema::PropertyNameMapInfo& propertyNameMapInfo,
                                               const MultiCondition &conditions,
                                               bool searchIndexOnly = false) {
            auto conditionProperties = schema::PropertyNameMapInfo{};
            for (const auto &conditionNode: conditions.conditions) {
                auto conditionNodePtr = conditionNode.lock();
                require(conditionNodePtr != nullptr);
                auto &condition = conditionNodePtr->getCondition();
                auto foundConditionProperty = conditionProperties.find(condition.propName);
                if (foundConditionProperty == foundConditionProperty.cend()) {
                    auto foundProperty = propertyNameMapInfo.find(condition.propName);
                    if (foundProperty == propertyNameMapInfo.cend()) {
                        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
                    }
                    conditionProperties.emplace(condition.propName, foundProperty->second);
                }
            }

            auto foundIndex = txn._iIndex->hasIndex(classInfo, conditionProperties, conditions);
            if (foundIndex.first) {
                auto indexedRecords = txn._iIndex->getRecord(conditionProperties, foundIndex.second, conditions);
                return txn._iRecord->getResultSet(classInfo, indexedRecords);
            } else {
                if (!searchIndexOnly) {
                    return txn._iRecord->getResultSetByMultiCondition(classInfo, conditionProperties, conditions);
                }
            }
            return ResultSet{};
        }

        static std::vector<RecordDescriptor>
        compareConditionRdesc(const Txn &txn,
                              const schema::ClassAccessInfo& classInfo,
                              const schema::PropertyNameMapInfo& propertyNameMapInfo,
                              const Condition &condition,
                              bool searchIndexOnly = false) {
            auto foundProperty = propertyNameMapInfo.find(condition.propName);
            if (foundProperty == propertyNameMapInfo.cend()) {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
            }
            auto propertyInfo = foundProperty->second;
            auto foundIndex = txn._iIndex->hasIndex(classInfo, propertyInfo, condition);
            if (foundIndex.first) {
                return txn._iIndex->getRecord(propertyInfo, foundIndex.second, condition);
            } else {
                if (!searchIndexOnly) {
                    return txn._iRecord->getRdescSetByCondition(classInfo, propertyInfo.type, condition);
                }
            }
            return std::vector<RecordDescriptor>{};
        }

        static std::vector<RecordDescriptor>
        compareMultiConditionRdesc(const Txn &txn,
                                   const schema::ClassAccessInfo& classInfo,
                                   const schema::PropertyNameMapInfo& propertyNameMapInfo,
                                   const MultiCondition &conditions,
                                   bool searchIndexOnly = false) {
            auto conditionProperties = schema::PropertyNameMapInfo{};
            for (const auto &conditionNode: conditions.conditions) {
                auto conditionNodePtr = conditionNode.lock();
                require(conditionNodePtr != nullptr);
                auto &condition = conditionNodePtr->getCondition();
                auto foundConditionProperty = conditionProperties.find(condition.propName);
                if (foundConditionProperty == foundConditionProperty.cend()) {
                    auto foundProperty = propertyNameMapInfo.find(condition.propName);
                    if (foundProperty == propertyNameMapInfo.cend()) {
                        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
                    }
                    conditionProperties.emplace(condition.propName, foundProperty->second);
                }
            }

            auto foundIndex = txn._iIndex->hasIndex(classInfo, conditionProperties, conditions);
            if (foundIndex.first) {
                return txn._iIndex->getRecord(conditionProperties, foundIndex.second, conditions);
            } else {
                if (!searchIndexOnly) {
                    return txn._iRecord->getRdescByMultiCondition(classInfo, conditionProperties, conditions);
                }
            }
            return std::vector<RecordDescriptor>{};
        }

        static ResultSet compareEdgeCondition(const Txn &txn,
                                              const RecordDescriptor &recordDescriptor,
                                              const Direction& direction,
                                              const Condition &condition) {
            auto edgeRecordIds = resolveEdgeRecordIds(txn, recordDescriptor.rid, direction);
            auto resultSet = ResultSet{};
            auto edgeInfos = std::map<ClassId, std::pair<schema::ClassAccessInfo, PropertyType>>{};
            for(const auto& edgeRecordId: edgeRecordIds) {
                auto edgeClassInfo = schema::ClassAccessInfo{};
                auto propertyType = PropertyType::UNDEFINED;

                auto foundEdgeInfo = edgeInfos.find(edgeRecordId.first);
                if (foundEdgeInfo == edgeInfos.cend()) {
                    edgeClassInfo = txn._class->getInfo(edgeRecordId);
                    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
                    auto foundProperty = propertyNameMapInfo.find(condition.propName);
                    if (foundProperty == propertyNameMapInfo.cend()) {
                        continue;
                    }
                    propertyType = foundProperty->second.type;
                    edgeInfos.emplace(std::make_pair(edgeRecordId.first, std::make_pair(edgeClassInfo, propertyType)));
                } else {
                    edgeClassInfo = foundEdgeInfo->second.first;
                    propertyType = foundEdgeInfo->second.second;
                }

                auto edgeRecord = txn._iRecord->getRecord(edgeClassInfo, RecordDescriptor{edgeRecordId});
                if (compareRecordByCondition(edgeRecord, propertyType, condition)) {
                    resultSet.emplace_back(Result{RecordDescriptor{edgeRecordId}, edgeRecord});
                }
            }
            return resultSet;
        }

        static ResultSet compareEdgeCondition(const Txn &txn,
                                              const RecordDescriptor &recordDescriptor,
                                              const Direction& direction,
                                              bool (*condition)(const Record &)) {
            auto edgeRecordIds = resolveEdgeRecordIds(txn, recordDescriptor.rid, direction);
            auto resultSet = ResultSet{};
            auto edgeInfos = std::map<ClassId, schema::ClassAccessInfo>{};
            for(const auto& edgeRecordId: edgeRecordIds) {
                auto edgeClassInfo = schema::ClassAccessInfo{};

                auto foundEdgeInfo = edgeInfos.find(edgeRecordId.first);
                if (foundEdgeInfo == edgeInfos.cend()) {
                    edgeClassInfo = txn._class->getInfo(edgeRecordId);
                    edgeInfos.emplace(std::make_pair(edgeRecordId.first, edgeClassInfo));
                } else {
                    edgeClassInfo = foundEdgeInfo->second;
                }

                auto edgeRecord = txn._iRecord->getRecord(edgeClassInfo, RecordDescriptor{edgeRecordId});
                if (condition(edgeRecord)) {
                    resultSet.emplace_back(Result{RecordDescriptor{edgeRecordId}, edgeRecord});
                }
            }
            return resultSet;
        }

        static ResultSet compareEdgeMultiCondition(const Txn &txn,
                                                   const RecordDescriptor &recordDescriptor,
                                                   const Direction& direction,
                                                   const MultiCondition &multiCondition) {
            auto edgeRecordIds = resolveEdgeRecordIds(txn, recordDescriptor.rid, direction);
            auto resultSet = ResultSet{};
            auto edgeInfos = std::map<ClassId, std::pair<schema::ClassAccessInfo, PropertyMapType>>{};
            for(const auto& edgeRecordId: edgeRecordIds) {
                auto edgeClassInfo = schema::ClassAccessInfo{};
                auto propertyTypes = PropertyMapType{};

                auto foundEdgeInfo = edgeInfos.find(edgeRecordId.first);
                if (foundEdgeInfo == edgeInfos.cend()) {
                    edgeClassInfo = txn._class->getInfo(edgeRecordId);
                    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
                    for(const auto& property: propertyNameMapInfo) {
                        propertyTypes.emplace(property.first, property.second.type);
                    }
                    edgeInfos.emplace(std::make_pair(edgeRecordId.first, std::make_pair(edgeClassInfo, propertyTypes)));
                } else {
                    edgeClassInfo = foundEdgeInfo->second.first;
                    propertyTypes = foundEdgeInfo->second.second;
                }

                auto edgeRecord = txn._iRecord->getRecord(edgeClassInfo, RecordDescriptor{edgeRecordId});
                if (multiCondition.execute(edgeRecord, propertyTypes)) {
                    resultSet.emplace_back(Result{RecordDescriptor{edgeRecordId}, edgeRecord});
                }
            }
            return resultSet;
        }

        static std::vector<RecordDescriptor>
        compareEdgeConditionRdesc(const Txn &txn,
                                  const RecordDescriptor &recordDescriptor,
                                  const Direction& direction,
                                  const Condition &condition) {
            auto edgeRecordIds = resolveEdgeRecordIds(txn, recordDescriptor.rid, direction);
            auto recordDescriptors = std::vector<RecordDescriptor>{};
            auto edgeInfos = std::map<ClassId, std::pair<schema::ClassAccessInfo, PropertyType>>{};
            for(const auto& edgeRecordId: edgeRecordIds) {
                auto edgeClassInfo = schema::ClassAccessInfo{};
                auto propertyType = PropertyType::UNDEFINED;

                auto foundEdgeInfo = edgeInfos.find(edgeRecordId.first);
                if (foundEdgeInfo == edgeInfos.cend()) {
                    edgeClassInfo = txn._class->getInfo(edgeRecordId);
                    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
                    auto foundProperty = propertyNameMapInfo.find(condition.propName);
                    if (foundProperty == propertyNameMapInfo.cend()) {
                        continue;
                    }
                    propertyType = foundProperty->second.type;
                    edgeInfos.emplace(std::make_pair(edgeRecordId.first, std::make_pair(edgeClassInfo, propertyType)));
                } else {
                    edgeClassInfo = foundEdgeInfo->second.first;
                    propertyType = foundEdgeInfo->second.second;
                }

                auto edgeRecord = txn._iRecord->getRecord(edgeClassInfo, RecordDescriptor{edgeRecordId});
                if (compareRecordByCondition(edgeRecord, propertyType, condition)) {
                    recordDescriptors.emplace_back(RecordDescriptor{edgeRecordId});
                }
            }
            return recordDescriptors;
        }

        static ResultSet compareEdgeCondition(const Txn &txn,
                                              const RecordDescriptor &recordDescriptor,
                                              const Direction& direction,
                                              bool (*condition)(const Record &)) {
            auto edgeRecordIds = resolveEdgeRecordIds(txn, recordDescriptor.rid, direction);
            auto resultSet = ResultSet{};
            auto edgeInfos = std::map<ClassId, schema::ClassAccessInfo>{};
            for(const auto& edgeRecordId: edgeRecordIds) {
                auto edgeClassInfo = schema::ClassAccessInfo{};

                auto foundEdgeInfo = edgeInfos.find(edgeRecordId.first);
                if (foundEdgeInfo == edgeInfos.cend()) {
                    edgeClassInfo = txn._class->getInfo(edgeRecordId);
                    edgeInfos.emplace(std::make_pair(edgeRecordId.first, edgeClassInfo));
                } else {
                    edgeClassInfo = foundEdgeInfo->second;
                }

                auto edgeRecord = txn._iRecord->getRecord(edgeClassInfo, RecordDescriptor{edgeRecordId});
                if (condition(edgeRecord)) {
                    resultSet.emplace_back(Result{RecordDescriptor{edgeRecordId}, edgeRecord});
                }
            }
            return resultSet;
        }

        static std::vector<RecordDescriptor>
        compareEdgeConditionRdesc(const Txn &txn,
                                  const RecordDescriptor &recordDescriptor,
                                  const Direction& direction,
                                  bool (*condition)(const Record &)) {
            auto edgeRecordIds = resolveEdgeRecordIds(txn, recordDescriptor.rid, direction);
            auto recordDescriptors = std::vector<RecordDescriptor>{};
            auto edgeInfos = std::map<ClassId, schema::ClassAccessInfo>{};
            for(const auto& edgeRecordId: edgeRecordIds) {
                auto edgeClassInfo = schema::ClassAccessInfo{};

                auto foundEdgeInfo = edgeInfos.find(edgeRecordId.first);
                if (foundEdgeInfo == edgeInfos.cend()) {
                    edgeClassInfo = txn._class->getInfo(edgeRecordId);
                    edgeInfos.emplace(std::make_pair(edgeRecordId.first, edgeClassInfo));
                } else {
                    edgeClassInfo = foundEdgeInfo->second;
                }

                auto edgeRecord = txn._iRecord->getRecord(edgeClassInfo, RecordDescriptor{edgeRecordId});
                if (condition(edgeRecord)) {
                    recordDescriptors.emplace_back(RecordDescriptor{edgeRecordId});
                }
            }
            return recordDescriptors;
        }

        static ResultSet compareEdgeMultiCondition(const Txn &txn,
                                                   const RecordDescriptor &recordDescriptor,
                                                   const Direction& direction,
                                                   const MultiCondition &multiCondition) {
            auto edgeRecordIds = resolveEdgeRecordIds(txn, recordDescriptor.rid, direction);
            auto resultSet = ResultSet{};
            auto edgeInfos = std::map<ClassId, std::pair<schema::ClassAccessInfo, PropertyMapType>>{};
            for(const auto& edgeRecordId: edgeRecordIds) {
                auto edgeClassInfo = schema::ClassAccessInfo{};
                auto propertyTypes = PropertyMapType{};

                auto foundEdgeInfo = edgeInfos.find(edgeRecordId.first);
                if (foundEdgeInfo == edgeInfos.cend()) {
                    edgeClassInfo = txn._class->getInfo(edgeRecordId);
                    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
                    for(const auto& property: propertyNameMapInfo) {
                        propertyTypes.emplace(property.first, property.second.type);
                    }
                    edgeInfos.emplace(std::make_pair(edgeRecordId.first, std::make_pair(edgeClassInfo, propertyTypes)));
                } else {
                    edgeClassInfo = foundEdgeInfo->second.first;
                    propertyTypes = foundEdgeInfo->second.second;
                }

                auto edgeRecord = txn._iRecord->getRecord(edgeClassInfo, RecordDescriptor{edgeRecordId});
                if (multiCondition.execute(edgeRecord, propertyTypes)) {
                    resultSet.emplace_back(Result{RecordDescriptor{edgeRecordId}, edgeRecord});
                }
            }
            return resultSet;
        }

        static std::vector<RecordDescriptor>
        compareEdgeMultiConditionRdesc(const Txn &txn,
                                       const RecordDescriptor &recordDescriptor,
                                       const Direction& direction,
                                       const MultiCondition &multiCondition) {
            auto edgeRecordIds = resolveEdgeRecordIds(txn, recordDescriptor.rid, direction);
            auto recordDescriptors = std::vector<RecordDescriptor>{};
            auto edgeInfos = std::map<ClassId, std::pair<schema::ClassAccessInfo, PropertyMapType>>{};
            for(const auto& edgeRecordId: edgeRecordIds) {
                auto edgeClassInfo = schema::ClassAccessInfo{};
                auto propertyTypes = PropertyMapType{};

                auto foundEdgeInfo = edgeInfos.find(edgeRecordId.first);
                if (foundEdgeInfo == edgeInfos.cend()) {
                    edgeClassInfo = txn._class->getInfo(edgeRecordId);
                    auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id, edgeClassInfo.superClassId);
                    for(const auto& property: propertyNameMapInfo) {
                        propertyTypes.emplace(property.first, property.second.type);
                    }
                    edgeInfos.emplace(std::make_pair(edgeRecordId.first, std::make_pair(edgeClassInfo, propertyTypes)));
                } else {
                    edgeClassInfo = foundEdgeInfo->second.first;
                    propertyTypes = foundEdgeInfo->second.second;
                }

                auto edgeRecord = txn._iRecord->getRecord(edgeClassInfo, RecordDescriptor{edgeRecordId});
                if (multiCondition.execute(edgeRecord, propertyTypes)) {
                    recordDescriptors.emplace_back(RecordDescriptor{edgeRecordId});
                }
            }
            return recordDescriptors;
        }

    private:

        inline static std::string toLower(const std::string &text) {
            auto tmp = std::string{};
            std::transform(text.cbegin(), text.cend(), std::back_inserter(tmp), ::tolower);
            return tmp;
        };

        inline static bool genericCompareFunc(const Bytes &value,
                                              const PropertyType& type,
                                              const Bytes &cmpValue1,
                                              const Bytes &cmpValue2,
                                              const Condition::Comparator& cmp,
                                              bool isIgnoreCase) {
            switch (type) {
                case PropertyType::TINYINT:
                    switch (cmp) {
                        case Condition::Comparator::EQUAL:
                            return value.toTinyInt() == cmpValue1.toTinyInt();
                        case Condition::Comparator::GREATER:
                            return value.toTinyInt() > cmpValue1.toTinyInt();
                        case Condition::Comparator::GREATER_EQUAL:
                            return value.toTinyInt() >= cmpValue1.toTinyInt();
                        case Condition::Comparator::LESS:
                            return value.toTinyInt() < cmpValue1.toTinyInt();
                        case Condition::Comparator::LESS_EQUAL:
                            return value.toTinyInt() <= cmpValue1.toTinyInt();
                        case Condition::Comparator::BETWEEN:
                            return (cmpValue1.toTinyInt() <= value.toTinyInt()) &&
                                   (value.toTinyInt() <= cmpValue2.toTinyInt());
                        case Condition::Comparator::BETWEEN_NO_LOWER:
                            return (cmpValue1.toTinyInt() < value.toTinyInt()) &&
                                   (value.toTinyInt() <= cmpValue2.toTinyInt());
                        case Condition::Comparator::BETWEEN_NO_UPPER:
                            return (cmpValue1.toTinyInt() <= value.toTinyInt()) &&
                                   (value.toTinyInt() < cmpValue2.toTinyInt());
                        case Condition::Comparator::BETWEEN_NO_BOUND:
                            return (cmpValue1.toTinyInt() < value.toTinyInt()) &&
                                   (value.toTinyInt() < cmpValue2.toTinyInt());
                        default:
                            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                    }
                case PropertyType::UNSIGNED_TINYINT:
                    switch (cmp) {
                        case Condition::Comparator::EQUAL:
                            return value.toTinyIntU() == cmpValue1.toTinyIntU();
                        case Condition::Comparator::GREATER:
                            return value.toTinyIntU() > cmpValue1.toTinyIntU();
                        case Condition::Comparator::GREATER_EQUAL:
                            return value.toTinyIntU() >= cmpValue1.toTinyIntU();
                        case Condition::Comparator::LESS:
                            return value.toTinyIntU() < cmpValue1.toTinyIntU();
                        case Condition::Comparator::LESS_EQUAL:
                            return value.toTinyIntU() <= cmpValue1.toTinyIntU();
                        case Condition::Comparator::BETWEEN:
                            return (cmpValue1.toTinyIntU() <= value.toTinyIntU()) &&
                                   (value.toTinyIntU() <= cmpValue2.toTinyIntU());
                        case Condition::Comparator::BETWEEN_NO_LOWER:
                            return (cmpValue1.toTinyIntU() < value.toTinyIntU()) &&
                                   (value.toTinyIntU() <= cmpValue2.toTinyIntU());
                        case Condition::Comparator::BETWEEN_NO_UPPER:
                            return (cmpValue1.toTinyIntU() <= value.toTinyIntU()) &&
                                   (value.toTinyIntU() < cmpValue2.toTinyIntU());
                        case Condition::Comparator::BETWEEN_NO_BOUND:
                            return (cmpValue1.toTinyIntU() < value.toTinyIntU()) &&
                                   (value.toTinyIntU() < cmpValue2.toTinyIntU());
                        default:
                            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                    }
                case PropertyType::SMALLINT:
                    switch (cmp) {
                        case Condition::Comparator::EQUAL:
                            return value.toSmallInt() == cmpValue1.toSmallInt();
                        case Condition::Comparator::GREATER:
                            return value.toSmallInt() > cmpValue1.toSmallInt();
                        case Condition::Comparator::GREATER_EQUAL:
                            return value.toSmallInt() >= cmpValue1.toSmallInt();
                        case Condition::Comparator::LESS:
                            return value.toSmallInt() < cmpValue1.toSmallInt();
                        case Condition::Comparator::LESS_EQUAL:
                            return value.toSmallInt() <= cmpValue1.toSmallInt();
                        case Condition::Comparator::BETWEEN:
                            return (cmpValue1.toSmallInt() <= value.toSmallInt()) &&
                                   (value.toSmallInt() <= cmpValue2.toSmallInt());
                        case Condition::Comparator::BETWEEN_NO_LOWER:
                            return (cmpValue1.toSmallInt() < value.toSmallInt()) &&
                                   (value.toSmallInt() <= cmpValue2.toSmallInt());
                        case Condition::Comparator::BETWEEN_NO_UPPER:
                            return (cmpValue1.toSmallInt() <= value.toSmallInt()) &&
                                   (value.toSmallInt() < cmpValue2.toSmallInt());
                        case Condition::Comparator::BETWEEN_NO_BOUND:
                            return (cmpValue1.toSmallInt() < value.toSmallInt()) &&
                                   (value.toSmallInt() < cmpValue2.toSmallInt());
                        default:
                            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                    }
                case PropertyType::UNSIGNED_SMALLINT:
                    switch (cmp) {
                        case Condition::Comparator::EQUAL:
                            return value.toSmallIntU() == cmpValue1.toSmallIntU();
                        case Condition::Comparator::GREATER:
                            return value.toSmallIntU() > cmpValue1.toSmallIntU();
                        case Condition::Comparator::GREATER_EQUAL:
                            return value.toSmallIntU() >= cmpValue1.toSmallIntU();
                        case Condition::Comparator::LESS:
                            return value.toSmallIntU() < cmpValue1.toSmallIntU();
                        case Condition::Comparator::LESS_EQUAL:
                            return value.toSmallIntU() <= cmpValue1.toSmallIntU();
                        case Condition::Comparator::BETWEEN:
                            return (cmpValue1.toSmallIntU() <= value.toSmallIntU()) &&
                                   (value.toSmallIntU() <= cmpValue2.toSmallIntU());
                        case Condition::Comparator::BETWEEN_NO_LOWER:
                            return (cmpValue1.toSmallIntU() < value.toSmallIntU()) &&
                                   (value.toSmallIntU() <= cmpValue2.toSmallIntU());
                        case Condition::Comparator::BETWEEN_NO_UPPER:
                            return (cmpValue1.toSmallIntU() <= value.toSmallIntU()) &&
                                   (value.toSmallIntU() < cmpValue2.toSmallIntU());
                        case Condition::Comparator::BETWEEN_NO_BOUND:
                            return (cmpValue1.toSmallIntU() < value.toSmallIntU()) &&
                                   (value.toSmallIntU() < cmpValue2.toSmallIntU());
                        default:
                            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                    }
                case PropertyType::INTEGER:
                    switch (cmp) {
                        case Condition::Comparator::EQUAL:
                            return value.toInt() == cmpValue1.toInt();
                        case Condition::Comparator::GREATER:
                            return value.toInt() > cmpValue1.toInt();
                        case Condition::Comparator::GREATER_EQUAL:
                            return value.toInt() >= cmpValue1.toInt();
                        case Condition::Comparator::LESS:
                            return value.toInt() < cmpValue1.toInt();
                        case Condition::Comparator::LESS_EQUAL:
                            return value.toInt() <= cmpValue1.toInt();
                        case Condition::Comparator::BETWEEN:
                            return (cmpValue1.toInt() <= value.toInt()) && (value.toInt() <= cmpValue2.toInt());
                        case Condition::Comparator::BETWEEN_NO_LOWER:
                            return (cmpValue1.toInt() < value.toInt()) && (value.toInt() <= cmpValue2.toInt());
                        case Condition::Comparator::BETWEEN_NO_UPPER:
                            return (cmpValue1.toInt() <= value.toInt()) && (value.toInt() < cmpValue2.toInt());
                        case Condition::Comparator::BETWEEN_NO_BOUND:
                            return (cmpValue1.toInt() < value.toInt()) && (value.toInt() < cmpValue2.toInt());
                        default:
                            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                    }
                case PropertyType::UNSIGNED_INTEGER:
                    switch (cmp) {
                        case Condition::Comparator::EQUAL:
                            return value.toIntU() == cmpValue1.toIntU();
                        case Condition::Comparator::GREATER:
                            return value.toIntU() > cmpValue1.toIntU();
                        case Condition::Comparator::GREATER_EQUAL:
                            return value.toIntU() >= cmpValue1.toIntU();
                        case Condition::Comparator::LESS:
                            return value.toIntU() < cmpValue1.toIntU();
                        case Condition::Comparator::LESS_EQUAL:
                            return value.toIntU() <= cmpValue1.toIntU();
                        case Condition::Comparator::BETWEEN:
                            return (cmpValue1.toIntU() <= value.toIntU()) && (value.toIntU() <= cmpValue2.toIntU());
                        case Condition::Comparator::BETWEEN_NO_LOWER:
                            return (cmpValue1.toIntU() < value.toIntU()) && (value.toIntU() <= cmpValue2.toIntU());
                        case Condition::Comparator::BETWEEN_NO_UPPER:
                            return (cmpValue1.toIntU() <= value.toIntU()) && (value.toIntU() < cmpValue2.toIntU());
                        case Condition::Comparator::BETWEEN_NO_BOUND:
                            return (cmpValue1.toIntU() < value.toIntU()) && (value.toIntU() < cmpValue2.toIntU());
                        default:
                            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                    }
                case PropertyType::BIGINT:
                    switch (cmp) {
                        case Condition::Comparator::EQUAL:
                            return value.toBigInt() == cmpValue1.toBigInt();
                        case Condition::Comparator::GREATER:
                            return value.toBigInt() > cmpValue1.toBigInt();
                        case Condition::Comparator::GREATER_EQUAL:
                            return value.toBigInt() >= cmpValue1.toBigInt();
                        case Condition::Comparator::LESS:
                            return value.toBigInt() < cmpValue1.toBigInt();
                        case Condition::Comparator::LESS_EQUAL:
                            return value.toBigInt() <= cmpValue1.toBigInt();
                        case Condition::Comparator::BETWEEN:
                            return (cmpValue1.toBigInt() <= value.toBigInt()) &&
                                   (value.toBigInt() <= cmpValue2.toBigInt());
                        case Condition::Comparator::BETWEEN_NO_LOWER:
                            return (cmpValue1.toBigInt() < value.toBigInt()) &&
                                   (value.toBigInt() <= cmpValue2.toBigInt());
                        case Condition::Comparator::BETWEEN_NO_UPPER:
                            return (cmpValue1.toBigInt() <= value.toBigInt()) &&
                                   (value.toBigInt() < cmpValue2.toBigInt());
                        case Condition::Comparator::BETWEEN_NO_BOUND:
                            return (cmpValue1.toBigInt() < value.toBigInt()) &&
                                   (value.toBigInt() < cmpValue2.toBigInt());
                        default:
                            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                    }
                case PropertyType::UNSIGNED_BIGINT:
                    switch (cmp) {
                        case Condition::Comparator::EQUAL:
                            return value.toBigIntU() == cmpValue1.toBigIntU();
                        case Condition::Comparator::GREATER:
                            return value.toBigIntU() > cmpValue1.toBigIntU();
                        case Condition::Comparator::GREATER_EQUAL:
                            return value.toBigIntU() >= cmpValue1.toBigIntU();
                        case Condition::Comparator::LESS:
                            return value.toBigIntU() < cmpValue1.toBigIntU();
                        case Condition::Comparator::LESS_EQUAL:
                            return value.toBigIntU() <= cmpValue1.toBigIntU();
                        case Condition::Comparator::BETWEEN:
                            return (cmpValue1.toBigIntU() <= value.toBigIntU()) &&
                                   (value.toBigIntU() <= cmpValue2.toBigIntU());
                        case Condition::Comparator::BETWEEN_NO_LOWER:
                            return (cmpValue1.toBigIntU() < value.toBigIntU()) &&
                                   (value.toBigIntU() <= cmpValue2.toBigIntU());
                        case Condition::Comparator::BETWEEN_NO_UPPER:
                            return (cmpValue1.toBigIntU() <= value.toBigIntU()) &&
                                   (value.toBigIntU() < cmpValue2.toBigIntU());
                        case Condition::Comparator::BETWEEN_NO_BOUND:
                            return (cmpValue1.toBigIntU() < value.toBigIntU()) &&
                                   (value.toBigIntU() < cmpValue2.toBigIntU());
                        default:
                            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                    }
                case PropertyType::REAL:
                    switch (cmp) {
                        case Condition::Comparator::EQUAL:
                            return value.toReal() == cmpValue1.toReal();
                        case Condition::Comparator::GREATER:
                            return value.toReal() > cmpValue1.toReal();
                        case Condition::Comparator::GREATER_EQUAL:
                            return value.toReal() >= cmpValue1.toReal();
                        case Condition::Comparator::LESS:
                            return value.toReal() < cmpValue1.toReal();
                        case Condition::Comparator::LESS_EQUAL:
                            return value.toReal() <= cmpValue1.toReal();
                        case Condition::Comparator::BETWEEN:
                            return (cmpValue1.toReal() <= value.toReal()) && (value.toReal() <= cmpValue2.toReal());
                        case Condition::Comparator::BETWEEN_NO_LOWER:
                            return (cmpValue1.toReal() < value.toReal()) && (value.toReal() <= cmpValue2.toReal());
                        case Condition::Comparator::BETWEEN_NO_UPPER:
                            return (cmpValue1.toReal() <= value.toReal()) && (value.toReal() < cmpValue2.toReal());
                        case Condition::Comparator::BETWEEN_NO_BOUND:
                            return (cmpValue1.toReal() < value.toReal()) && (value.toReal() < cmpValue2.toReal());
                        default:
                            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                    }
                case PropertyType::TEXT: {
                    auto textValue = (isIgnoreCase) ? toLower(value.toText()) : value.toText();
                    auto textCmpValue1 = (isIgnoreCase) ? toLower(cmpValue1.toText()) : cmpValue1.toText();
                    auto textCmpValue2 = (cmpValue2.empty()) ? "" : ((isIgnoreCase) ? toLower(cmpValue2.toText())
                                                                                    : cmpValue2.toText());
                    switch (cmp) {
                        case Condition::Comparator::EQUAL:
                            return textValue == textCmpValue1;
                        case Condition::Comparator::GREATER:
                            return textValue > textCmpValue1;
                        case Condition::Comparator::GREATER_EQUAL:
                            return textValue >= textCmpValue1;
                        case Condition::Comparator::LESS:
                            return textValue < textCmpValue1;
                        case Condition::Comparator::LESS_EQUAL:
                            return textValue <= textCmpValue1;
                        case Condition::Comparator::CONTAIN:
                            return textValue.find(textCmpValue1) < strlen(textValue.c_str());
                        case Condition::Comparator::BEGIN_WITH:
                            return textValue.find(textCmpValue1) == 0;
                        case Condition::Comparator::END_WITH:
                            std::reverse(textValue.begin(), textValue.end());
                            std::reverse(textCmpValue1.begin(), textCmpValue1.end());
                            return textValue.find(textCmpValue1) == 0;
                        case Condition::Comparator::LIKE:
                            utils::string::replaceAll(textCmpValue1, "%", "(.*)");
                            utils::string::replaceAll(textCmpValue1, "_", "(.)");
                            return std::regex_match(textValue, std::regex(textCmpValue1));
                        case Condition::Comparator::REGEX: {
                            return std::regex_match(textValue, std::regex(textCmpValue1));
                        }
                        case Condition::Comparator::BETWEEN:
                            return (textCmpValue1 <= textValue) && (textValue <= textCmpValue2);
                        case Condition::Comparator::BETWEEN_NO_LOWER:
                            return (textCmpValue1 < textValue) && (textValue <= textCmpValue2);
                        case Condition::Comparator::BETWEEN_NO_UPPER:
                            return (textCmpValue1 <= textValue) && (textValue < textCmpValue2);
                        case Condition::Comparator::BETWEEN_NO_BOUND:
                            return (textCmpValue1 < textValue) && (textValue < textCmpValue2);
                        default:
                            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                    }
                }
                case PropertyType::BLOB:
                    switch (cmp) {
                        case Condition::Comparator::EQUAL:
                            return memcmp(value.getRaw(), cmpValue1.getRaw(), value.size()) == 0;
                        default:
                            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                    }
                default:
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_PROPTYPE);
            }
        }

    };
}
