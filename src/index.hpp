/*
 *  Copyright (C) 2019, NogDB <https://nogdb.org>
 *  <nogdb at throughwave dot co dot th>
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

#include <algorithm>
#include <functional>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include "datarecord_adapter.hpp"
#include "index_adapter.hpp"
#include "lmdb_engine.hpp"
#include "schema.hpp"
#include "schema_adapter.hpp"

#include "nogdb/nogdb.h"
#include "nogdb/nogdb_types.h"

namespace nogdb {
namespace index {
    using namespace adapter::schema;
    using namespace adapter::datarecord;
    using namespace schema;
    using parser::RecordParser;

    typedef std::map<PropertyId, IndexAccessInfo> PropertyIdMapIndex;
    typedef std::map<std::string, std::pair<PropertyAccessInfo, IndexAccessInfo>> PropertyNameMapIndex;

    struct IndexUtils {

        static void initialize(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const ClassId& superClassId,
            const ClassType& classType);

        static void drop(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo);

        static void drop(const Transaction *txn,
            const ClassId& classId,
            const PropertyNameMapInfo& propertyNameMapInfo);

        static void insert(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const PositionId& posId,
            const Bytes& value);

        static void insert(const Transaction *txn,
            const RecordDescriptor& recordDescriptor,
            const Record& record,
            const PropertyNameMapIndex& propertyNameMapIndex);

        static void remove(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const PositionId& posId,
            const Bytes& value);

        static void remove(const Transaction *txn,
            const RecordDescriptor& recordDescriptor,
            const Record& record,
            const PropertyNameMapIndex& propertyNameMapIndex);

        static PropertyNameMapIndex getIndexInfos(const Transaction *txn,
            const RecordDescriptor& recordDescriptor,
            const Record& record,
            const PropertyNameMapInfo& propertyNameMapInfo);

        static std::pair<bool, IndexAccessInfo> hasIndex(const Transaction *txn,
            const ClassAccessInfo& classInfo,
            const PropertyAccessInfo& propertyInfo,
            const Condition& condition);

        static std::pair<bool, PropertyIdMapIndex> hasIndex(const Transaction *txn,
            const ClassAccessInfo& classInfo,
            const PropertyNameMapInfo& propertyInfos,
            const MultiCondition& conditions);

        static std::vector<RecordDescriptor> getRecord(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const Condition& condition,
            bool isNegative = false);

        static size_t getCountRecord(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const Condition& condition,
            bool isNegative = false);

        static std::vector<RecordDescriptor> getRecord(const Transaction *txn,
            const PropertyNameMapInfo& propertyInfos,
            const PropertyIdMapIndex& propertyIndexInfo,
            const MultiCondition& conditions);

        static size_t getCountRecord(const Transaction *txn,
            const PropertyNameMapInfo& propertyInfos,
            const PropertyIdMapIndex& propertyIndexInfo,
            const MultiCondition& conditions);

    protected:
        static const std::vector<Condition::Comparator> validComparators;

    private:

        static adapter::index::IndexRecord openIndexRecordPositive(const Transaction *txn,
            const IndexAccessInfo& indexInfo);

        static adapter::index::IndexRecord openIndexRecordNegative(const Transaction *txn,
            const IndexAccessInfo& indexInfo);

        static adapter::index::IndexRecord openIndexRecordString(const Transaction *txn,
            const IndexAccessInfo& indexInfo);

        template <typename T>
        static void createNumeric(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const ClassId& superClassId,
            const ClassType& classType,
            T (*valueRetrieve)(const Bytes&))
        {
            auto propertyIdMapInfo = SchemaUtils::getPropertyIdMapInfo(txn, indexInfo.classId, superClassId);
            require(!propertyIdMapInfo.empty());
            auto indexAccess = openIndexRecordPositive(txn, indexInfo);
            auto dataRecord = DataRecord(txn->_txnBase, indexInfo.classId, classType);
            std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
                [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                    auto const record = RecordParser::parseRawData(
                        result, propertyIdMapInfo, classType == ClassType::EDGE, txn->_txnCtx->isVersionEnabled());
                    auto bytesValue = record.get(propertyInfo.name);
                    if (!bytesValue.empty()) {
                        auto indexRecord = Blob(sizeof(PositionId)).append(&positionId, sizeof(PositionId));
                        indexAccess.create(valueRetrieve(bytesValue), indexRecord);
                    }
                };
            dataRecord.resultSetIter(callback);
        }

        template <typename T>
        static void createSignedNumeric(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const ClassId& superClassId,
            const ClassType& classType,
            T (*valueRetrieve)(const Bytes&))
        {
            auto propertyIdMapInfo = SchemaUtils::getPropertyIdMapInfo(txn, indexInfo.classId, superClassId);
            require(!propertyIdMapInfo.empty());
            auto indexPositiveAccess = openIndexRecordPositive(txn, indexInfo);
            auto indexNegativeAccess = openIndexRecordNegative(txn, indexInfo);
            auto dataRecord = DataRecord(txn->_txnBase, indexInfo.classId, classType);
            std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
                [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                    auto const record = RecordParser::parseRawData(
                        result, propertyIdMapInfo, classType == ClassType::EDGE, txn->_txnCtx->isVersionEnabled());
                    auto bytesValue = record.get(propertyInfo.name);
                    if (!bytesValue.empty()) {
                        auto indexRecord = Blob(sizeof(PositionId)).append(&positionId, sizeof(PositionId));
                        auto value = valueRetrieve(bytesValue);
                        (value >= 0) ? indexPositiveAccess.create(value, indexRecord)
                                     : indexNegativeAccess.create(value, indexRecord);
                    }
                };
            dataRecord.resultSetIter(callback);
        }

        static void createString(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const ClassId& superClassId,
            const ClassType& classType);

        template <typename T>
        static void insert(const Transaction *txn,
            const IndexAccessInfo& indexInfo,
            PositionId positionId,
            const T& value)
        {
            auto indexAccess = openIndexRecordPositive(txn, indexInfo);
            auto indexRecord = Blob(sizeof(PositionId)).append(&positionId, sizeof(PositionId));
            indexAccess.create(value, indexRecord);
        }

        static void insert(const Transaction *txn,
            const IndexAccessInfo& indexInfo,
            PositionId positionId,
            const std::string& value);

        template <typename T>
        static void insertSignedNumeric(const Transaction *txn,
            const IndexAccessInfo& indexInfo,
            PositionId positionId,
            const T& value)
        {
            auto indexRecord = Blob(sizeof(PositionId)).append(&positionId, sizeof(PositionId));
            if (value >= 0) {
                auto indexPositiveAccess = openIndexRecordPositive(txn, indexInfo);
                indexPositiveAccess.create(value, indexRecord);
            } else {
                auto indexNegativeAccess = openIndexRecordNegative(txn, indexInfo);
                indexNegativeAccess.create(value, indexRecord);
            }
        }

        template <typename T>
        static void removeByCursorNumeric(const storage_engine::lmdb::Cursor& cursor,
            PositionId positionId,
            const T& value)
        {
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

        template <typename T>
        static void removeByCursor(const Transaction *txn,
            const IndexAccessInfo& indexInfo,
            PositionId positionId,
            const T& value)
        {
            auto indexAccessCursor = openIndexRecordPositive(txn, indexInfo).getCursor();
            removeByCursorNumeric(indexAccessCursor, positionId, value);
        }

        static void removeByCursor(const Transaction *txn,
            const IndexAccessInfo& indexInfo,
            PositionId positionId,
            const std::string& value);

        template <typename T>
        static void removeByCursorWithSignNumeric(const Transaction *txn,
            const IndexAccessInfo& indexInfo,
            const PositionId& posId,
            const T& value)
        {
            auto indexPositiveAccessCursor = openIndexRecordPositive(txn, indexInfo).getCursor();
            auto indexNegativeAccessCursor = openIndexRecordNegative(txn, indexInfo).getCursor();
            (value < 0) ?
                removeByCursorNumeric(indexNegativeAccessCursor, posId, value) :
                removeByCursorNumeric(indexPositiveAccessCursor, posId, value);
        }

        inline static void sortByRdesc(std::vector<RecordDescriptor>& recordDescriptors)
        {
            std::sort(
                recordDescriptors.begin(), recordDescriptors.end(),
                [](const RecordDescriptor& lhs, const RecordDescriptor& rhs) noexcept {
                    return lhs.rid < rhs.rid;
                });
        };

        inline static bool isValidComparator(const Condition& condition)
        {
            return std::find(validComparators.cbegin(),
                validComparators.cend(),
                condition.comp) != validComparators.cend();
        }

        static std::vector<RecordDescriptor> getRecordFromMultiCondition(const Transaction *txn,
            const PropertyNameMapInfo& propertyInfos,
            const PropertyIdMapIndex& propertyIndexInfo,
            const MultiCondition::CompositeNode* compositeNode,
            bool isParentNegative);

        static std::vector<RecordDescriptor> getMultiConditionResult(const Transaction *txn,
            const PropertyNameMapInfo& propertyInfos,
            const PropertyIdMapIndex& propertyIndexInfo,
            const std::shared_ptr<MultiCondition::ExprNode>& exprNode,
            bool isNegative);

        static std::vector<RecordDescriptor> getLessOrEqual(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const Bytes& value);

        static std::vector<RecordDescriptor> getLessThan(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const Bytes& value);

        static std::vector<RecordDescriptor> getEqual(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const Bytes& value);

        static std::vector<RecordDescriptor> getGreaterOrEqual(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const Bytes& value);

        static std::vector<RecordDescriptor> getGreaterThan(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const Bytes& value);

        static std::vector<RecordDescriptor> getBetween(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const Bytes& lowerBound,
            const Bytes& upperBound,
            const std::pair<bool, bool>& isIncludeBound);

        static std::vector<RecordDescriptor> getLessCommon(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const Bytes& value,
            bool isEqual);

        static std::vector<RecordDescriptor> getGreaterCommon(const Transaction *txn,
            const PropertyAccessInfo& propertyInfo,
            const IndexAccessInfo& indexInfo,
            const Bytes& value,
            bool isEqual);

        template <typename T>
        static std::vector<RecordDescriptor> getLessNumeric(const Transaction *txn,
            const T& value,
            const IndexAccessInfo& indexInfo,
            bool includeEqual = false)
        {
            if (value < 0) {
                auto indexAccessCursor = openIndexRecordNegative(txn, indexInfo).getCursor();
                return backwardSearchIndex(indexAccessCursor, indexInfo.classId, value, false, includeEqual);
            } else {
                auto indexPositiveAccessCursor = openIndexRecordPositive(txn, indexInfo).getCursor();
                auto indexNegativeAccessCursor = openIndexRecordNegative(txn, indexInfo).getCursor();
                auto positiveResult = backwardSearchIndex(
                    indexPositiveAccessCursor, indexInfo.classId, value, true, includeEqual);
                auto negativeResult = fullScanIndex(indexNegativeAccessCursor, indexInfo.classId);
                positiveResult.insert(positiveResult.end(), negativeResult.cbegin(), negativeResult.cend());
                return positiveResult;
            }
        };

        template <typename T>
        static std::vector<RecordDescriptor> getEqualNumeric(const Transaction *txn,
            const T& value,
            const IndexAccessInfo& indexInfo)
        {
            auto result = std::vector<RecordDescriptor> {};
            auto indexAccess = (value < 0) ?
                openIndexRecordNegative(txn, indexInfo) : openIndexRecordPositive(txn, indexInfo);
            return exactMatchIndex(indexAccess.getCursor(), indexInfo.classId, value, result);
        };

        template <typename T>
        static std::vector<RecordDescriptor> getGreaterNumeric(const Transaction *txn,
            const T& value,
            const IndexAccessInfo& indexInfo,
            bool includeEqual = false)
        {
            if (value < 0) {
                auto indexPositiveAccessCursor = openIndexRecordPositive(txn, indexInfo).getCursor();
                auto indexNegativeAccessCursor = openIndexRecordNegative(txn, indexInfo).getCursor();
                auto positiveResult = fullScanIndex(indexPositiveAccessCursor, indexInfo.classId);
                auto negativeResult = forwardSearchIndex(
                    indexNegativeAccessCursor, indexInfo.classId, value, false, includeEqual);
                positiveResult.insert(positiveResult.end(), negativeResult.cbegin(), negativeResult.cend());
                return positiveResult;
            } else {
                auto indexAccessCursor = openIndexRecordPositive(txn, indexInfo).getCursor();
                return forwardSearchIndex(indexAccessCursor, indexInfo.classId, value, true, includeEqual);
            }
        };

        template <typename T>
        static std::vector<RecordDescriptor> getBetweenNumeric(const Transaction *txn,
            const T& lowerBound,
            const T& upperBound,
            const IndexAccessInfo& indexInfo,
            const std::pair<bool, bool>& isIncludeBound)
        {
            if (lowerBound < 0 && upperBound < 0) {
                auto indexAccessCursor = openIndexRecordNegative(txn, indexInfo).getCursor();
                return betweenSearchIndex(
                    indexAccessCursor, indexInfo.classId, lowerBound, upperBound, false, isIncludeBound);
            } else if (lowerBound < 0 && upperBound >= 0) {
                auto indexPositiveAccessCursor = openIndexRecordPositive(txn, indexInfo).getCursor();
                auto indexNegativeAccessCursor = openIndexRecordNegative(txn, indexInfo).getCursor();
                auto positiveResult = betweenSearchIndex(indexPositiveAccessCursor, indexInfo.classId,
                    static_cast<T>(0), upperBound,
                    true, { true, isIncludeBound.second });
                auto negativeResult = betweenSearchIndex(indexNegativeAccessCursor, indexInfo.classId,
                    lowerBound, static_cast<T>(0),
                    false, { isIncludeBound.first, true });
                positiveResult.insert(positiveResult.end(), negativeResult.cbegin(), negativeResult.cend());
                return positiveResult;
            } else {
                auto indexAccessCursor = openIndexRecordPositive(txn, indexInfo).getCursor();
                return betweenSearchIndex(indexAccessCursor,
                    indexInfo.classId,
                    lowerBound,
                    upperBound,
                    true,
                    isIncludeBound);
            }
        };

        template <typename T>
        static std::vector<RecordDescriptor> backwardSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler,
            const ClassId& classId,
            const T& value,
            bool positive,
            bool isInclude = false)
        {
            auto result = std::vector<RecordDescriptor> {};
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
                    result.emplace_back(RecordDescriptor { classId, positionId });
                }
            } else {
                for (auto keyValue = cursorHandler.findRange(value);
                     !keyValue.empty();
                     keyValue = cursorHandler.getNext()) {
                    if (!isInclude) {
                        auto key = keyValue.key.data.template numeric<T>();
                        if (key == value)
                            continue;
                        else
                            isInclude = true;
                    }
                    auto positionId = keyValue.val.data.template numeric<PositionId>();
                    result.emplace_back(RecordDescriptor { classId, positionId });
                }
            }
            return result;
        };

        template <typename T>
        static std::vector<RecordDescriptor> exactMatchIndex(const storage_engine::lmdb::Cursor& cursorHandler,
            const ClassId& classId,
            const T& value,
            std::vector<RecordDescriptor>& result)
        {
            for (auto keyValue = cursorHandler.find(value);
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto key = keyValue.key.data.template numeric<T>();
                if (key == value) {
                    auto positionId = keyValue.val.data.template numeric<PositionId>();
                    result.emplace_back(RecordDescriptor { classId, positionId });
                } else {
                    break;
                }
            }
            return std::move(result);
        };

        static std::vector<RecordDescriptor> exactMatchIndex(const storage_engine::lmdb::Cursor& cursorHandler,
            const ClassId& classId,
            const std::string& value,
            std::vector<RecordDescriptor>& result);

        static std::vector<RecordDescriptor> fullScanIndex(const storage_engine::lmdb::Cursor& cursorHandler,
            const ClassId& classId);

        template <typename T>
        static std::vector<RecordDescriptor> forwardSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler,
            const ClassId& classId,
            const T& value,
            bool positive,
            bool isInclude = false)
        {
            auto result = std::vector<RecordDescriptor> {};
            if (!std::is_same<T, double>::value || positive) {
                for (auto keyValue = cursorHandler.findRange(value);
                     !keyValue.empty();
                     keyValue = cursorHandler.getNext()) {
                    if (!isInclude) {
                        auto key = keyValue.key.data.template numeric<T>();
                        if (key == value)
                            continue;
                        else
                            isInclude = true;
                    }
                    auto positionId = keyValue.val.data.template numeric<PositionId>();
                    result.emplace_back(RecordDescriptor { classId, positionId });
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
                    result.emplace_back(RecordDescriptor { classId, positionId });
                }
            }
            return result;
        };

        static std::vector<RecordDescriptor> forwardSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler,
            const ClassId& classId,
            const std::string& value,
            bool isInclude = false);

        template <typename T>
        static std::vector<RecordDescriptor> betweenSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler,
            const ClassId& classId,
            const T& lower,
            const T& upper,
            bool isLowerPositive,
            const std::pair<bool, bool>& isIncludeBound)
        {
            auto result = std::vector<RecordDescriptor> {};
            if (!std::is_same<T, double>::value || isLowerPositive) {
                for (auto keyValue = cursorHandler.findRange(lower);
                     !keyValue.empty();
                     keyValue = cursorHandler.getNext()) {
                    auto key = keyValue.key.data.template numeric<T>();
                    if (!isIncludeBound.first && key == lower)
                        continue;
                    else if ((!isIncludeBound.second && key == upper) || key > upper)
                        break;
                    auto positionId = keyValue.val.data.template numeric<PositionId>();
                    result.emplace_back(RecordDescriptor { classId, positionId });
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
                    if ((!isIncludeBound.second && key == upper) || key > upper)
                        break;
                    auto positionId = keyValue.val.data.numeric<PositionId>();
                    result.emplace_back(RecordDescriptor { classId, positionId });
                }
            }
            return result;
        };

        static std::vector<RecordDescriptor> betweenSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler,
            const ClassId& classId,
            const std::string& lower,
            const std::string& upper,
            const std::pair<bool, bool>& isIncludeBound);
    };

}
}