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

#include <iostream> // for debugging
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <type_traits>
#include <functional>

#include "schema.hpp"
#include "lmdb_engine.hpp"
#include "schema_adapter.hpp"
#include "index_adapter.hpp"
#include "datarecord_adapter.hpp"

#include "nogdb/nogdb_types.h"
#include "nogdb/nogdb.h"

#define UNIQUE_FLAG(_unique)                        (_unique)? INDEX_TYPE_UNIQUE: INDEX_TYPE_NON_UNIQUE
#define INDEX_POSITIVE_NUMERIC_UNIQUE(_unique)      INDEX_TYPE_POSITIVE | INDEX_TYPE_NUMERIC | UNIQUE_FLAG(_unique)
#define INDEX_NEGATIVE_NUMERIC_UNIQUE(_unique)      INDEX_TYPE_NEGATIVE | INDEX_TYPE_NUMERIC | UNIQUE_FLAG(_unique)
#define INDEX_STRING_UNIQUE(_unique)                INDEX_TYPE_POSITIVE | INDEX_TYPE_STRING | UNIQUE_FLAG(_unique)

namespace nogdb {

  namespace index {

    using namespace adapter::schema;

    typedef std::map<PropertyId, IndexAccessInfo> PropertyIdMapIndex;

    typedef std::map<std::string, std::pair<PropertyAccessInfo, IndexAccessInfo>> PropertyNameMapIndex;

    class IndexInterface {
    public:

      IndexInterface(const Transaction *txn) : _txn{txn} {}

      virtual ~IndexInterface() noexcept = default;

      void initialize(const PropertyAccessInfo &propertyInfo, const IndexAccessInfo &indexInfo, const ClassType &classType);

      void drop(const PropertyAccessInfo &propertyInfo, const IndexAccessInfo &indexInfo);

      void drop(const ClassId &classId, const PropertyNameMapInfo &propertyNameMapInfo);

      void insert(const PropertyAccessInfo &propertyInfo, const IndexAccessInfo &indexInfo,
                  const PositionId &posId, const Bytes &value);

      void insert(const RecordDescriptor &recordDescriptor,
                  const Record& record,
                  const PropertyNameMapIndex& propertyNameMapIndex);

      void remove(const PropertyAccessInfo &propertyInfo, const IndexAccessInfo &indexInfo,
                  const PositionId &posId, const Bytes &value);

      void remove(const RecordDescriptor &recordDescriptor,
                  const Record& record,
                  const PropertyNameMapIndex& propertyNameMapIndex);

      PropertyNameMapIndex getIndexInfos(const RecordDescriptor &recordDescriptor,
                                         const Record& record,
                                         const PropertyNameMapInfo &propertyNameMapInfo);

      std::pair<bool, IndexAccessInfo>
      hasIndex(const ClassAccessInfo &classInfo,
               const PropertyAccessInfo &propertyInfo,
               const Condition &condition) const;

      std::pair<bool, PropertyIdMapIndex>
      hasIndex(const ClassAccessInfo &classInfo,
               const PropertyNameMapInfo &propertyInfos,
               const MultiCondition &conditions) const;

      std::vector<RecordDescriptor>
      getRecord(const PropertyAccessInfo &propertyInfo, const IndexAccessInfo &indexInfo,
                const Condition &condition, bool isNegative = false) const;

      std::vector<RecordDescriptor>
      getRecord(const PropertyNameMapInfo &propertyInfos,
                const PropertyIdMapIndex &propertyIndexInfo,
                const MultiCondition &conditions) const;

    protected:

      const std::vector<Condition::Comparator> validComparators{
          Condition::Comparator::EQUAL,
          Condition::Comparator::BETWEEN_NO_BOUND,
          Condition::Comparator::BETWEEN,
          Condition::Comparator::BETWEEN_NO_UPPER,
          Condition::Comparator::BETWEEN_NO_LOWER,
          Condition::Comparator::LESS_EQUAL,
          Condition::Comparator::LESS,
          Condition::Comparator::GREATER_EQUAL,
          Condition::Comparator::GREATER
      };

    private:

      const Transaction *_txn;

      adapter::index::IndexRecord openIndexRecordPositive(const IndexAccessInfo &indexInfo) const;

      adapter::index::IndexRecord openIndexRecordNegative(const IndexAccessInfo &indexInfo) const;

      adapter::index::IndexRecord openIndexRecordString(const IndexAccessInfo &indexInfo) const;

      template<typename T>
      void createNumeric(const PropertyAccessInfo &propertyInfo,
                         const IndexAccessInfo &indexInfo,
                         const ClassType &classType,
                         T(*valueRetrieve)(const Bytes &)) {
        auto propertyIdMapInfo = _txn->_adapter->dbProperty()->getIdMapInfo(indexInfo.classId);
        auto indexAccess = openIndexRecordPositive(indexInfo);
        auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, indexInfo.classId, classType);
        std::function<void(const PositionId &, const storage_engine::lmdb::Result &)> callback =
            [&](const PositionId &positionId, const storage_engine::lmdb::Result &result) {
              auto const record = parser::RecordParser::parseRawData(result, propertyIdMapInfo, classType == ClassType::EDGE);
              auto bytesValue = record.get(propertyInfo.name);
              if (!bytesValue.empty()) {
                auto indexRecord = Blob(sizeof(PositionId)).append(&positionId, sizeof(PositionId));
                indexAccess.create(valueRetrieve(bytesValue), indexRecord);
              }
            };
        dataRecord.resultSetIter(callback);
      }

      template<typename T>
      void createSignedNumeric(const PropertyAccessInfo &propertyInfo,
                               const IndexAccessInfo &indexInfo,
                               const ClassType &classType,
                               T(*valueRetrieve)(const Bytes &)) {
        auto propertyIdMapInfo = _txn->_adapter->dbProperty()->getIdMapInfo(indexInfo.classId);
        auto indexPositiveAccess = openIndexRecordPositive(indexInfo);
        auto indexNegativeAccess = openIndexRecordNegative(indexInfo);
        auto dataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, indexInfo.classId, classType);
        std::function<void(const PositionId &, const storage_engine::lmdb::Result &)> callback =
            [&](const PositionId &positionId, const storage_engine::lmdb::Result &result) {
              auto const record = parser::RecordParser::parseRawData(result, propertyIdMapInfo, classType == ClassType::EDGE);
              auto bytesValue = record.get(propertyInfo.name);
              if (!bytesValue.empty()) {
                auto indexRecord = Blob(sizeof(PositionId)).append(&positionId, sizeof(PositionId));
                auto value = valueRetrieve(bytesValue);
                (value >= 0) ? indexPositiveAccess.create(value, indexRecord) : indexNegativeAccess.create(value,
                                                                                                           indexRecord);
              }
            };
        dataRecord.resultSetIter(callback);
      }

      void createString(const PropertyAccessInfo &propertyInfo, const IndexAccessInfo &indexInfo,
                        const ClassType &classType);

      template<typename T>
      void insert(const IndexAccessInfo &indexInfo, PositionId positionId, const T &value) {
        auto indexAccess = openIndexRecordPositive(indexInfo);
        auto indexRecord = Blob(sizeof(PositionId)).append(&positionId, sizeof(PositionId));
        indexAccess.create(value, indexRecord);
      }

      void insert(const IndexAccessInfo &indexInfo, PositionId positionId, const std::string &value);

      template<typename T>
      void insertSignedNumeric(const IndexAccessInfo &indexInfo, PositionId positionId, const T &value) {
        auto indexRecord = Blob(sizeof(PositionId)).append(&positionId, sizeof(PositionId));
        if (value >= 0) {
          auto indexPositiveAccess = openIndexRecordPositive(indexInfo);
          indexPositiveAccess.create(value, indexRecord);
        } else {
          auto indexNegativeAccess = openIndexRecordNegative(indexInfo);
          indexNegativeAccess.create(value, indexRecord);
        }
      }

      template<typename T>
      void removeByCursorNumeric(const storage_engine::lmdb::Cursor &cursor, PositionId positionId, const T &value) {
        for (auto keyValue = cursor.find(value);
             !keyValue.empty();
             keyValue = cursor.getNext()) {
          auto key = keyValue.key.data.template numeric<T>();
          if (key == value) {
            auto valueAsPositionId = keyValue.val.data.template numeric<PositionId>();
            if (positionId == valueAsPositionId) {
              cursor.del();
              break;
            }
          } else {
            break;
          }
        }
      }

      template<typename T>
      void removeByCursor(const IndexAccessInfo &indexInfo, PositionId positionId, const T &value) {
        auto indexAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
        removeByCursorNumeric(indexAccessCursor, positionId, value);
      }

      void removeByCursor(const IndexAccessInfo &indexInfo, PositionId positionId, const std::string &value);

      template<typename T>
      void removeByCursorWithSignNumeric(const IndexAccessInfo &indexInfo, const PositionId &posId, const T &value) {
        auto indexPositiveAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
        auto indexNegativeAccessCursor = openIndexRecordNegative(indexInfo).getCursor();
        (value < 0) ? removeByCursorNumeric(indexNegativeAccessCursor, posId, value) :
        removeByCursorNumeric(indexPositiveAccessCursor, posId, value);
      }

      inline static auto cmpRecordDescriptor = [](const RecordDescriptor &lhs, const RecordDescriptor &rhs) noexcept {
        return lhs.rid < rhs.rid;
      };

      inline static void sortByRdesc(std::vector<RecordDescriptor> &recordDescriptors) {
        std::sort(recordDescriptors.begin(), recordDescriptors.end(), cmpRecordDescriptor);
      };

      inline bool isValidComparator(const Condition &condition) const {
        return std::find(validComparators.cbegin(), validComparators.cend(), condition.comp) != validComparators.cend();
      }

      std::vector<RecordDescriptor>
      getRecordFromMultiCondition(const PropertyNameMapInfo &propertyInfos,
                                  const PropertyIdMapIndex &propertyIndexInfo,
                                  const MultiCondition::CompositeNode *compositeNode,
                                  bool isParentNegative) const;

      std::vector<RecordDescriptor>
      getMultiConditionResult(const PropertyNameMapInfo &propertyInfos,
                              const PropertyIdMapIndex &propertyIndexInfo,
                              const std::shared_ptr<MultiCondition::ExprNode> &exprNode,
                              bool isNegative) const;

      std::vector<RecordDescriptor>
      getLessOrEqual(const PropertyAccessInfo &propertyInfo, const IndexAccessInfo &indexInfo, const Bytes &value) const;

      std::vector<RecordDescriptor>
      getLessThan(const PropertyAccessInfo &propertyInfo, const IndexAccessInfo &indexInfo, const Bytes &value) const;

      std::vector<RecordDescriptor>
      getEqual(const PropertyAccessInfo &propertyInfo, const IndexAccessInfo &indexInfo, const Bytes &value) const;

      std::vector<RecordDescriptor>
      getGreaterOrEqual(const PropertyAccessInfo &propertyInfo, const IndexAccessInfo &indexInfo,
                        const Bytes &value) const;

      std::vector<RecordDescriptor>
      getGreaterThan(const PropertyAccessInfo &propertyInfo, const IndexAccessInfo &indexInfo, const Bytes &value) const;

      std::vector<RecordDescriptor>
      getBetween(const PropertyAccessInfo &propertyInfo, const IndexAccessInfo &indexInfo,
                 const Bytes &lowerBound, const Bytes &upperBound, const std::pair<bool, bool> &isIncludeBound) const;

      std::vector<RecordDescriptor>
      getLessCommon(const PropertyAccessInfo &propertyInfo, const IndexAccessInfo &indexInfo, const Bytes &value, bool isEqual) const;

      std::vector<RecordDescriptor>
      getGreaterCommon(const PropertyAccessInfo &propertyInfo, const IndexAccessInfo &indexInfo, const Bytes &value, bool isEqual) const;

      template<typename T>
      std::vector<RecordDescriptor>
      getLessNumeric(const T &value, const IndexAccessInfo &indexInfo, bool includeEqual = false) const {
        if (value < 0) {
          auto indexAccessCursor = openIndexRecordNegative(indexInfo).getCursor();
          return backwardSearchIndex(indexAccessCursor, indexInfo.classId, value, false, includeEqual);
        } else {
          auto indexPositiveAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
          auto indexNegativeAccessCursor = openIndexRecordNegative(indexInfo).getCursor();
          auto positiveResult = backwardSearchIndex(indexPositiveAccessCursor, indexInfo.classId, value, true,
                                                    includeEqual);
          auto negativeResult = fullScanIndex(indexNegativeAccessCursor, indexInfo.classId);
          positiveResult.insert(positiveResult.end(), negativeResult.cbegin(), negativeResult.cend());
          return positiveResult;
        }
      };

      template<typename T>
      std::vector<RecordDescriptor>
      getEqualNumeric(const T &value, const IndexAccessInfo &indexInfo) const {
        auto result = std::vector<RecordDescriptor>{};
        auto indexAccess = (value < 0) ? openIndexRecordNegative(indexInfo) : openIndexRecordPositive(indexInfo);
        return exactMatchIndex(indexAccess.getCursor(), indexInfo.classId, value, result);
      };

      template<typename T>
      std::vector<RecordDescriptor>
      getGreaterNumeric(const T &value, const IndexAccessInfo &indexInfo, bool includeEqual = false) const {
        if (value < 0) {
          auto indexPositiveAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
          auto indexNegativeAccessCursor = openIndexRecordNegative(indexInfo).getCursor();
          auto positiveResult = fullScanIndex(indexPositiveAccessCursor, indexInfo.classId);
          auto negativeResult = forwardSearchIndex(indexNegativeAccessCursor, indexInfo.classId, value, false,
                                                   includeEqual);
          positiveResult.insert(positiveResult.end(), negativeResult.cbegin(), negativeResult.cend());
          return positiveResult;
        } else {
          auto indexAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
          return forwardSearchIndex(indexAccessCursor, indexInfo.classId, value, true, includeEqual);
        }
      };

      template<typename T>
      std::vector<RecordDescriptor>
      getBetweenNumeric(const T &lowerBound, const T &upperBound,
                        const IndexAccessInfo &indexInfo, const std::pair<bool, bool> &isIncludeBound) const {
        if (lowerBound < 0 && upperBound < 0) {
          auto indexAccessCursor = openIndexRecordNegative(indexInfo).getCursor();
          return betweenSearchIndex(indexAccessCursor, indexInfo.classId, lowerBound, upperBound, false,
                                    isIncludeBound);
        } else if (lowerBound < 0 && upperBound >= 0) {
          auto indexPositiveAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
          auto indexNegativeAccessCursor = openIndexRecordNegative(indexInfo).getCursor();
          auto positiveResult = betweenSearchIndex(indexPositiveAccessCursor, indexInfo.classId,
                                                   static_cast<T>(0), upperBound,
                                                   true, {true, isIncludeBound.second});
          auto negativeResult = betweenSearchIndex(indexNegativeAccessCursor, indexInfo.classId,
                                                   lowerBound, static_cast<T>(0),
                                                   false, {isIncludeBound.first, true});
          positiveResult.insert(positiveResult.end(), negativeResult.cbegin(), negativeResult.cend());
          return positiveResult;
        } else {
          auto indexAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
          return betweenSearchIndex(indexAccessCursor, indexInfo.classId, lowerBound, upperBound, true, isIncludeBound);
        }
      };

      template<typename T>
      static std::vector<RecordDescriptor>
      backwardSearchIndex(const storage_engine::lmdb::Cursor &cursorHandler, const ClassId &classId,
                          const T &value, bool positive, bool isInclude = false) {
        auto result = std::vector<RecordDescriptor>{};
        if (!std::is_same<T, double>::value || positive) {
          if (isInclude) {
            exactMatchIndex(cursorHandler, classId, value, result);
          }
          cursorHandler.findRange(value);
          for (auto keyValue = cursorHandler.getPrev();
               !keyValue.empty();
               keyValue = cursorHandler.getPrev()) {
            auto key = keyValue.key.data.template numeric<T>();
            auto positionId = keyValue.val.data.template numeric<PositionId>();
            result.emplace_back(RecordDescriptor{classId, positionId});
          }
        } else {
          for (auto keyValue = cursorHandler.findRange(value);
               !keyValue.empty();
               keyValue = cursorHandler.getNext()) {
            if (!isInclude) {
              auto key = keyValue.key.data.template numeric<T>();
              if (key == value) continue;
              else isInclude = true;
            }
            auto positionId = keyValue.val.data.template numeric<PositionId>();
            result.emplace_back(RecordDescriptor{classId, positionId});
          }
        }
        return result;
      };

      template<typename T>
      static std::vector<RecordDescriptor>
      exactMatchIndex(const storage_engine::lmdb::Cursor &cursorHandler,
                      const ClassId &classId,
                      const T &value,
                      std::vector<RecordDescriptor> &result) {
        for (auto keyValue = cursorHandler.find(value);
             !keyValue.empty();
             keyValue = cursorHandler.getNext()) {
          auto key = keyValue.key.data.template numeric<T>();
          if (key == value) {
            auto positionId = keyValue.val.data.template numeric<PositionId>();
            result.emplace_back(RecordDescriptor{classId, positionId});
          } else {
            break;
          }
        }
        return std::move(result);
      };

      static std::vector<RecordDescriptor>
      exactMatchIndex(const storage_engine::lmdb::Cursor &cursorHandler,
                      const ClassId &classId,
                      const std::string &value,
                      std::vector<RecordDescriptor> &result);

      static std::vector<RecordDescriptor>
      fullScanIndex(const storage_engine::lmdb::Cursor &cursorHandler, const ClassId &classId);

      template<typename T>
      static std::vector<RecordDescriptor>
      forwardSearchIndex(const storage_engine::lmdb::Cursor &cursorHandler, const ClassId &classId,
                         const T &value, bool positive, bool isInclude = false) {
        auto result = std::vector<RecordDescriptor>{};
        if (!std::is_same<T, double>::value || positive) {
          for (auto keyValue = cursorHandler.findRange(value);
               !keyValue.empty();
               keyValue = cursorHandler.getNext()) {
            if (!isInclude) {
              auto key = keyValue.key.data.template numeric<T>();
              if (key == value) continue;
              else isInclude = true;
            }
            auto positionId = keyValue.val.data.template numeric<PositionId>();
            result.emplace_back(RecordDescriptor{classId, positionId});
          }
        } else {
          if (isInclude) {
            exactMatchIndex(cursorHandler, classId, value, result);
          }
          cursorHandler.findRange(value);
          for (auto keyValue = cursorHandler.getPrev();
               !keyValue.empty();
               keyValue = cursorHandler.getPrev()) {
            auto positionId = keyValue.val.data.template numeric<PositionId>();
            result.emplace_back(RecordDescriptor{classId, positionId});
          }
        }
        return result;
      };

      static std::vector<RecordDescriptor>
      forwardSearchIndex(const storage_engine::lmdb::Cursor &cursorHandler, const ClassId &classId,
                         const std::string &value, bool isInclude = false);

      template<typename T>
      static std::vector<RecordDescriptor>
      betweenSearchIndex(const storage_engine::lmdb::Cursor &cursorHandler, const ClassId &classId,
                         const T &lower, const T &upper, bool isLowerPositive,
                         const std::pair<bool, bool> &isIncludeBound) {
        auto result = std::vector<RecordDescriptor>{};
        if (!std::is_same<T, double>::value || isLowerPositive) {
          for (auto keyValue = cursorHandler.findRange(lower);
               !keyValue.empty();
               keyValue = cursorHandler.getNext()) {
            auto key = keyValue.key.data.template numeric<T>();
            if (!isIncludeBound.first && key == lower) continue;
            else if ((!isIncludeBound.second && key == upper) || key > upper) break;
            auto positionId = keyValue.val.data.template numeric<PositionId>();
            result.emplace_back(RecordDescriptor{classId, positionId});
          }
        } else {
          if (isIncludeBound.first) {
            exactMatchIndex(cursorHandler, classId, lower, result);
          }
          cursorHandler.findRange(lower);
          for (auto keyValue = cursorHandler.getPrev();
               !keyValue.empty();
               keyValue = cursorHandler.getPrev()) {
            auto key = keyValue.key.data.numeric<T>();
            if ((!isIncludeBound.second && key == upper) || key > upper) break;
            auto positionId = keyValue.val.data.numeric<PositionId>();
            result.emplace_back(RecordDescriptor{classId, positionId});
          }
        }
        return result;
      };

      static std::vector<RecordDescriptor>
      betweenSearchIndex(const storage_engine::lmdb::Cursor &cursorHandler, const ClassId &classId,
                         const std::string &lower, const std::string &upper,
                         const std::pair<bool, bool> &isIncludeBound);

    };

  }

}