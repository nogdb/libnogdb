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

#include <regex>
#include <utility>

#include "compare.hpp"
#include "schema.hpp"
#include "index.hpp"
#include "datarecord.hpp"
#include "relation.hpp"

namespace nogdb {

  namespace compare {

    bool RecordCompare::compareBytesValue(const Bytes &value, PropertyType type, const Condition &condition) {
      if (condition.comp == Condition::Comparator::IN) {
        for (const auto &valueBytes: condition.valueSet) {
          if (genericCompareFunc(value, type, valueBytes, Bytes{}, Condition::Comparator::EQUAL,
                                 condition.isIgnoreCase) ^
              condition.isNegative) {
            return true;
          }
        }
      } else if (condition.comp >= Condition::Comparator::BETWEEN &&
                 condition.comp <= Condition::Comparator::BETWEEN_NO_BOUND) {
        return genericCompareFunc(value, type, condition.valueSet[0], condition.valueSet[1], condition.comp,
                                  condition.isIgnoreCase) ^
               condition.isNegative;
      } else {
        return
            genericCompareFunc(value, type, condition.valueBytes, Bytes{}, condition.comp, condition.isIgnoreCase) ^
            condition.isNegative;
      }
      return false;
    }

    bool RecordCompare::compareRecordByCondition(const Record &record, const PropertyType &propertyType,
                                                 const Condition &condition) {
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

    std::vector<RecordId>
    RecordCompare::resolveEdgeRecordIds(const Txn &txn, const RecordId &recordId, const Direction &direction) {
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

    ResultSet RecordCompare::compareCondition(const Txn &txn,
                                              const schema::ClassAccessInfo &classInfo,
                                              const schema::PropertyNameMapInfo &propertyNameMapInfo,
                                              const Condition &condition,
                                              bool searchIndexOnly) {
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

    ResultSet RecordCompare::compareMultiCondition(const Txn &txn,
                                                   const schema::ClassAccessInfo &classInfo,
                                                   const schema::PropertyNameMapInfo &propertyNameMapInfo,
                                                   const MultiCondition &conditions,
                                                   bool searchIndexOnly) {
      auto conditionProperties = schema::PropertyNameMapInfo{};
      for (const auto &conditionNode: conditions.conditions) {
        auto conditionNodePtr = conditionNode.lock();
        require(conditionNodePtr != nullptr);
        auto &condition = conditionNodePtr->getCondition();
        auto foundConditionProperty = conditionProperties.find(condition.propName);
        if (foundConditionProperty == conditionProperties.cend()) {
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

    std::vector<RecordDescriptor>
    RecordCompare::compareConditionRdesc(const Txn &txn,
                                         const schema::ClassAccessInfo &classInfo,
                                         const schema::PropertyNameMapInfo &propertyNameMapInfo,
                                         const Condition &condition,
                                         bool searchIndexOnly) {
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
          return txn._iRecord->getRecordDescriptorByCondition(classInfo, propertyInfo.type, condition);
        }
      }
      return std::vector<RecordDescriptor>{};
    }

    std::vector<RecordDescriptor>
    RecordCompare::compareMultiConditionRdesc(const Txn &txn,
                                              const schema::ClassAccessInfo &classInfo,
                                              const schema::PropertyNameMapInfo &propertyNameMapInfo,
                                              const MultiCondition &conditions,
                                              bool searchIndexOnly) {
      auto conditionProperties = schema::PropertyNameMapInfo{};
      for (const auto &conditionNode: conditions.conditions) {
        auto conditionNodePtr = conditionNode.lock();
        require(conditionNodePtr != nullptr);
        auto &condition = conditionNodePtr->getCondition();
        auto foundConditionProperty = conditionProperties.find(condition.propName);
        if (foundConditionProperty == conditionProperties.cend()) {
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
          return txn._iRecord->getRecordDescriptorByMultiCondition(classInfo, conditionProperties, conditions);
        }
      }
      return std::vector<RecordDescriptor>{};
    }

    ResultSet RecordCompare::compareEdgeCondition(const Txn &txn,
                                                  const RecordDescriptor &recordDescriptor,
                                                  const Direction &direction,
                                                  const Condition &condition) {
      auto edgeRecordIds = resolveEdgeRecordIds(txn, recordDescriptor.rid, direction);
      auto resultSet = ResultSet{};
      auto edgeInfos = std::map<ClassId, std::pair<schema::ClassAccessInfo, PropertyType>>{};
      for (const auto &edgeRecordId: edgeRecordIds) {
        auto edgeClassInfo = schema::ClassAccessInfo{};
        auto propertyType = PropertyType::UNDEFINED;

        auto foundEdgeInfo = edgeInfos.find(edgeRecordId.first);
        if (foundEdgeInfo == edgeInfos.cend()) {
          edgeClassInfo = txn._class->getInfo(edgeRecordId.first);
          auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id,
                                                                          edgeClassInfo.superClassId);
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

    ResultSet RecordCompare::compareEdgeCondition(const Txn &txn,
                                                  const RecordDescriptor &recordDescriptor,
                                                  const Direction &direction,
                                                  bool (*condition)(const Record &)) {
      auto edgeRecordIds = resolveEdgeRecordIds(txn, recordDescriptor.rid, direction);
      auto resultSet = ResultSet{};
      auto edgeInfos = std::map<ClassId, schema::ClassAccessInfo>{};
      for (const auto &edgeRecordId: edgeRecordIds) {
        auto edgeClassInfo = schema::ClassAccessInfo{};

        auto foundEdgeInfo = edgeInfos.find(edgeRecordId.first);
        if (foundEdgeInfo == edgeInfos.cend()) {
          edgeClassInfo = txn._class->getInfo(edgeRecordId.first);
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

    ResultSet RecordCompare::compareEdgeMultiCondition(const Txn &txn,
                                                       const RecordDescriptor &recordDescriptor,
                                                       const Direction &direction,
                                                       const MultiCondition &multiCondition) {
      auto edgeRecordIds = resolveEdgeRecordIds(txn, recordDescriptor.rid, direction);
      auto resultSet = ResultSet{};
      auto edgeInfos = std::map<ClassId, std::pair<schema::ClassAccessInfo, PropertyMapType>>{};
      for (const auto &edgeRecordId: edgeRecordIds) {
        auto edgeClassInfo = schema::ClassAccessInfo{};
        auto propertyTypes = PropertyMapType{};

        auto foundEdgeInfo = edgeInfos.find(edgeRecordId.first);
        if (foundEdgeInfo == edgeInfos.cend()) {
          edgeClassInfo = txn._class->getInfo(edgeRecordId.first);
          auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id,
                                                                          edgeClassInfo.superClassId);
          for (const auto &property: propertyNameMapInfo) {
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

    std::vector<RecordDescriptor>
    RecordCompare::compareEdgeConditionRdesc(const Txn &txn,
                                             const RecordDescriptor &recordDescriptor,
                                             const Direction &direction,
                                             const Condition &condition) {
      auto edgeRecordIds = resolveEdgeRecordIds(txn, recordDescriptor.rid, direction);
      auto recordDescriptors = std::vector<RecordDescriptor>{};
      auto edgeInfos = std::map<ClassId, std::pair<schema::ClassAccessInfo, PropertyType>>{};
      for (const auto &edgeRecordId: edgeRecordIds) {
        auto edgeClassInfo = schema::ClassAccessInfo{};
        auto propertyType = PropertyType::UNDEFINED;

        auto foundEdgeInfo = edgeInfos.find(edgeRecordId.first);
        if (foundEdgeInfo == edgeInfos.cend()) {
          edgeClassInfo = txn._class->getInfo(edgeRecordId.first);
          auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id,
                                                                          edgeClassInfo.superClassId);
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

    std::vector<RecordDescriptor>
    RecordCompare::compareEdgeConditionRdesc(const Txn &txn,
                                             const RecordDescriptor &recordDescriptor,
                                             const Direction &direction,
                                             bool (*condition)(const Record &)) {
      auto edgeRecordIds = resolveEdgeRecordIds(txn, recordDescriptor.rid, direction);
      auto recordDescriptors = std::vector<RecordDescriptor>{};
      auto edgeInfos = std::map<ClassId, schema::ClassAccessInfo>{};
      for (const auto &edgeRecordId: edgeRecordIds) {
        auto edgeClassInfo = schema::ClassAccessInfo{};

        auto foundEdgeInfo = edgeInfos.find(edgeRecordId.first);
        if (foundEdgeInfo == edgeInfos.cend()) {
          edgeClassInfo = txn._class->getInfo(edgeRecordId.first);
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

    std::vector<RecordDescriptor>
    RecordCompare::compareEdgeMultiConditionRdesc(const Txn &txn,
                                                  const RecordDescriptor &recordDescriptor,
                                                  const Direction &direction,
                                                  const MultiCondition &multiCondition) {
      auto edgeRecordIds = resolveEdgeRecordIds(txn, recordDescriptor.rid, direction);
      auto recordDescriptors = std::vector<RecordDescriptor>{};
      auto edgeInfos = std::map<ClassId, std::pair<schema::ClassAccessInfo, PropertyMapType>>{};
      for (const auto &edgeRecordId: edgeRecordIds) {
        auto edgeClassInfo = schema::ClassAccessInfo{};
        auto propertyTypes = PropertyMapType{};

        auto foundEdgeInfo = edgeInfos.find(edgeRecordId.first);
        if (foundEdgeInfo == edgeInfos.cend()) {
          edgeClassInfo = txn._class->getInfo(edgeRecordId.first);
          auto propertyNameMapInfo = txn._iSchema->getPropertyNameMapInfo(edgeClassInfo.id,
                                                                          edgeClassInfo.superClassId);
          for (const auto &property: propertyNameMapInfo) {
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

  }
}
