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

#ifndef __INDEX_HPP_INCLUDED_
#define __INDEX_HPP_INCLUDED_

#include <iostream> // for debugging
#include <vector>
#include <tuple>
#include <type_traits>

#include "schema.hpp"
#include "lmdb_engine.hpp"
#include "base_txn.hpp"

#include "nogdb_types.h"
#include "nogdb_txn.h"
#include "nogdb_compare.h"

namespace nogdb {
    struct Index {
        Index() = delete;

        ~Index() noexcept = delete;

        typedef std::tuple<IndexId, bool, PropertyType> IndexPropertyType;

        static void addIndex(BaseTxn &txn, IndexId indexId, PositionId positionId, const Bytes &bytesValue,
                             PropertyType type, bool isUnique);

        static void deleteIndex(BaseTxn &txn, IndexId indexId, PositionId positionId, const Bytes &bytesValue,
                                PropertyType type, bool isUnique);

        template<typename T>
        static void deleteIndexCursor(const storage_engine::lmdb::Cursor& cursorHandler, PositionId positionId, const T& value) {
            for (auto keyValue = cursorHandler.find(value);
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto key = keyValue.key.data.numeric();
                if (key == value) {
                    auto valueAsPositionId = keyValue.val.data.numeric<PositionId>();
                    if (positionId == valueAsPositionId) {
                        cursorHandler.del();
                        break;
                    }
                } else {
                    break;
                }
            }
        }

        inline static void
        deleteIndexCursor(const storage_engine::lmdb::Cursor& cursorHandler, PositionId positionId, const std::string &value) {
            for (auto keyValue = cursorHandler.find(value);
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto key = keyValue.key.data.string();
                if (value == key) {
                    auto valueAsPositionId = keyValue.val.data.numeric<PositionId>();
                    if (positionId == valueAsPositionId) {
                        cursorHandler.del();
                        break;
                    }
                } else {
                    break;
                }
            }
        }

        inline static std::string getIndexingName(IndexId indexId) {
            return TB_INDEXING_PREFIX + std::to_string(indexId);
        }

        inline static std::string getIndexingName(IndexId indexId, bool isPositive) {
            return getIndexingName(indexId) + ((isPositive)? INDEX_POSITIVE_SUFFIX: INDEX_NEGATIVE_SUFFIX);
        }

        static std::pair<IndexPropertyType, bool>
        hasIndex(ClassId classId, const ClassInfo &classInfo, const Condition &condition);

        static std::pair<std::map<std::string, IndexPropertyType>, bool>
        hasIndex(ClassId classId, const ClassInfo &classInfo, const MultiCondition &conditions);

        static std::vector<RecordDescriptor> getIndexRecord(const Txn &txn,
                                                            ClassId classId,
                                                            IndexPropertyType indexPropertyType,
                                                            const Condition &condition,
                                                            bool isNegative = false);

        static std::vector<RecordDescriptor> getIndexRecord(const Txn &txn, ClassId classId,
                                                            const std::map<std::string, IndexPropertyType> &indexPropertyTypes,
                                                            const MultiCondition &conditions);

        static std::vector<RecordDescriptor>
        getLessEqual(const Txn &txn, ClassId classId, const IndexPropertyType &indexPropertyType, const Bytes &value);

        static std::vector<RecordDescriptor>
        getLess(const Txn &txn, ClassId classId, const IndexPropertyType &indexPropertyType, const Bytes &value);

        static std::vector<RecordDescriptor>
        getEqual(const Txn &txn, ClassId classId, const IndexPropertyType &indexPropertyType, const Bytes &value);

        static std::vector<RecordDescriptor>
        getGreaterEqual(const Txn &txn, ClassId classId, const IndexPropertyType &indexPropertyType,
                        const Bytes &value);

        static std::vector<RecordDescriptor>
        getGreater(const Txn &txn, ClassId classId, const IndexPropertyType &indexPropertyType, const Bytes &value);

        static std::vector<RecordDescriptor> getBetween(const Txn &txn,
                                                        ClassId classId,
                                                        const IndexPropertyType &indexPropertyType,
                                                        const Bytes &lowerBound,
                                                        const Bytes &upperBound,
                                                        const std::pair<bool, bool> &isIncludeBound);

        template<typename T>
        static std::vector<RecordDescriptor>
        getLess(const Txn &txn, ClassId classId, IndexId indexId, bool isUnique, T value, bool includeEqual = false) {
            auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
            if (value < 0) {
                auto dataIndexDBHandlerNegative = dsTxnHandler->openDbi(getIndexingName(indexId, false), true, isUnique);
                auto cursorHandlerNegative = dsTxnHandler->openCursor(dataIndexDBHandlerNegative);
                return backwardSearchIndex(cursorHandlerNegative, classId, value, false, includeEqual);
            } else {
                auto dataIndexDBHandlerPositive = dsTxnHandler->openDbi(getIndexingName(indexId, true), true, isUnique);
                auto dataIndexDBHandlerNegative = dsTxnHandler->openDbi(getIndexingName(indexId, false), true, isUnique);
                auto cursorHandlerPositive = dsTxnHandler->openCursor(dataIndexDBHandlerPositive);
                auto cursorHandlerNegative = dsTxnHandler->openCursor(dataIndexDBHandlerNegative);
                auto positiveResult = backwardSearchIndex(cursorHandlerPositive, classId, value, true, includeEqual);
                auto negativeResult = fullScanIndex(cursorHandlerNegative, classId);
                positiveResult.insert(positiveResult.end(), negativeResult.cbegin(), negativeResult.cend());
                return positiveResult;
            }
        };

        template<typename T>
        static std::vector<RecordDescriptor>
        getEqual(const Txn &txn, ClassId classId, IndexId indexId, bool isUnique, T value) {
            auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
            if (value < 0) {
                auto dataIndexDBHandlerNegative = dsTxnHandler->openDbi(getIndexingName(indexId, false), true, isUnique);
                auto cursorHandlerNegative = dsTxnHandler->openCursor(dataIndexDBHandlerNegative);
                return exactMatchIndex(cursorHandlerNegative, classId, value);
            } else {
                auto dataIndexDBHandlerPositive = dsTxnHandler->openDbi(getIndexingName(indexId, true), true, isUnique);
                auto cursorHandlerPositive = dsTxnHandler->openCursor(dataIndexDBHandlerPositive);
                return exactMatchIndex(cursorHandlerPositive, classId, value);
            }
        };

        template<typename T>
        static std::vector<RecordDescriptor>
        getGreater(const Txn &txn, ClassId classId, IndexId indexId, bool isUnique, T value,
                   bool includeEqual = false) {
            auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
            if (value < 0) {
                auto dataIndexDBHandlerPositive = dsTxnHandler->openDbi(getIndexingName(indexId, true), true, isUnique);
                auto dataIndexDBHandlerNegative = dsTxnHandler->openDbi(getIndexingName(indexId, false), true, isUnique);
                auto cursorHandlerPositive = dsTxnHandler->openCursor(dataIndexDBHandlerPositive);
                auto cursorHandlerNegative = dsTxnHandler->openCursor(dataIndexDBHandlerNegative);
                auto positiveResult = fullScanIndex(cursorHandlerPositive, classId);
                auto negativeResult = forwardSearchIndex(cursorHandlerNegative, classId, value, false, includeEqual);
                positiveResult.insert(positiveResult.end(), negativeResult.cbegin(), negativeResult.cend());
                return positiveResult;
            } else {
                auto dataIndexDBHandlerPositive = dsTxnHandler->openDbi(getIndexingName(indexId, true), true, isUnique);
                auto cursorHandlerPositive = dsTxnHandler->openCursor(dataIndexDBHandlerPositive);
                return forwardSearchIndex(cursorHandlerPositive, classId, value, true, includeEqual);
            }
        };

        template<typename T>
        static std::vector<RecordDescriptor> getBetween(const Txn &txn, ClassId classId, IndexId indexId,
                                                        bool isUnique, T lowerBound, T upperBound,
                                                        const std::pair<bool, bool> &isIncludeBound) {
            auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
            if (lowerBound < 0 && upperBound < 0) {
                auto dataIndexDBHandlerNegative = dsTxnHandler->openDbi(getIndexingName(indexId, false), true, isUnique);
                auto cursorHandlerNegative = dsTxnHandler->openCursor(dataIndexDBHandlerNegative);
                return betweenSearchIndex(cursorHandlerNegative, classId, lowerBound, upperBound, false, isIncludeBound);
            } else if (lowerBound < 0 && upperBound >= 0) {
                auto dataIndexDBHandlerPositive = dsTxnHandler->openDbi(getIndexingName(indexId, true), true, isUnique);
                auto dataIndexDBHandlerNegative = dsTxnHandler->openDbi(getIndexingName(indexId, false), true, isUnique);
                auto cursorHandlerPositive = dsTxnHandler->openCursor(dataIndexDBHandlerPositive);
                auto cursorHandlerNegative = dsTxnHandler->openCursor(dataIndexDBHandlerNegative);
                auto positiveResult = betweenSearchIndex(cursorHandlerPositive, classId,
                                                         static_cast<T>(0), upperBound,
                                                         true, {true, isIncludeBound.second});
                auto negativeResult = betweenSearchIndex(cursorHandlerNegative, classId,
                                                         lowerBound, static_cast<T>(0),
                                                         false, {isIncludeBound.first, true});
                positiveResult.insert(positiveResult.end(), negativeResult.cbegin(), negativeResult.cend());
                return positiveResult;
            } else {
                auto dataIndexDBHandlerPositive = dsTxnHandler->openDbi(getIndexingName(indexId, true), true, isUnique);
                auto cursorHandlerPositive = dsTxnHandler->openCursor(dataIndexDBHandlerPositive);
                return betweenSearchIndex(cursorHandlerPositive, classId, lowerBound, upperBound, true, isIncludeBound);
            }
        };

        template<typename T>
        static std::vector<RecordDescriptor>
        exactMatchIndex(const storage_engine::lmdb::Cursor& cursorHandler, ClassId classId, const T &value) {
            auto result = std::vector<RecordDescriptor>{};
            for (auto keyValue = cursorHandler.find(value);
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto key = keyValue.key.data.numeric();
                if (key == value) {
                    auto positionId = keyValue.val.data.numeric<PositionId>();
                    result.emplace_back(RecordDescriptor{classId, positionId});
                } else {
                    break;
                }
            }
            return result;
        };

        inline static std::vector<RecordDescriptor>
        exactMatchIndex(const storage_engine::lmdb::Cursor& cursorHandler, ClassId classId, const std::string &value) {
            auto result = std::vector<RecordDescriptor>{};
            for (auto keyValue = cursorHandler.find(value);
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto key = keyValue.key.data.string();
                if (key == value) {
                    auto positionId = keyValue.val.data.numeric<PositionId>();
                    result.emplace_back(RecordDescriptor{classId, positionId});
                } else {
                    break;
                }
            }
            return result;
        };

        inline static std::vector<RecordDescriptor> fullScanIndex(const storage_engine::lmdb::Cursor& cursorHandler, ClassId classId) {
            auto result = std::vector<RecordDescriptor>{};
            for (auto keyValue = cursorHandler.getNext();
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto positionId = keyValue.val.data.numeric<PositionId>();
                result.emplace_back(RecordDescriptor{classId, positionId});
            }
            return result;
        };

        template<typename T>
        static std::vector<RecordDescriptor> backwardSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler,
                                                                 ClassId classId, const T &value, bool positive,
                                                                 bool isInclude = false) {
            auto result = std::vector<RecordDescriptor>{};
            if (!std::is_same<T, double>::value || positive) {
                if (isInclude) {
                    auto partialResult = exactMatchIndex(cursorHandler, classId, value);
                    result.insert(result.end(), partialResult.cbegin(), partialResult.cend());
                }
                cursorHandler.findRange(value);
                for (auto keyValue = cursorHandler.getPrev();
                     !keyValue.empty();
                     keyValue = cursorHandler.getPrev()) {
                    auto key = keyValue.key.data.numeric();
                    auto positionId = keyValue.val.data.numeric<PositionId>();
                    result.emplace_back(RecordDescriptor{classId, positionId});
                }
            } else {
                for (auto keyValue = cursorHandler.findRange(value);
                     !keyValue.empty();
                     keyValue = cursorHandler.getNext()) {
                    if (!isInclude) {
                        auto key = keyValue.key.data.numeric();
                        if (key == value) continue;
                        else isInclude = true;
                    }
                    auto positionId = keyValue.val.data.numeric<PositionId>();
                    result.emplace_back(RecordDescriptor{classId, positionId});
                }
            }
            return result;
        };

        template<typename T>
        static std::vector<RecordDescriptor> forwardSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler,
                                                                ClassId classId, const T &value, bool positive,
                                                                bool isInclude = false) {
            auto result = std::vector<RecordDescriptor>{};
            if (!std::is_same<T, double>::value || positive) {
                for (auto keyValue = cursorHandler.findRange(value);
                     !keyValue.empty();
                     keyValue = cursorHandler.getNext()) {
                    if (!isInclude) {
                        auto key = keyValue.key.data.numeric();
                        if (key == value) continue;
                        else isInclude = true;
                    }
                    auto positionId = keyValue.val.data.numeric<PositionId>();
                    result.emplace_back(RecordDescriptor{classId, positionId});
                }
            } else {
                if (isInclude) {
                    auto partialResult = exactMatchIndex(cursorHandler, classId, value);
                    result.insert(result.end(), partialResult.cbegin(), partialResult.cend());
                }
                cursorHandler.findRange(value);
                for (auto keyValue = cursorHandler.getPrev();
                     !keyValue.empty();
                     keyValue = cursorHandler.getPrev()) {
                    auto positionId = keyValue.val.data.numeric<PositionId>();
                    result.emplace_back(RecordDescriptor{classId, positionId});
                }
            }
            return result;
        };

        inline static std::vector<RecordDescriptor> forwardSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler,
                                                                       ClassId classId, const std::string &value,
                                                                       bool isInclude = false) {
            auto result = std::vector<RecordDescriptor>{};
            for (auto keyValue = cursorHandler.findRange(value);
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                if (!isInclude) {
                    auto key = keyValue.key.data.string();
                    if (key == value) continue;
                    else isInclude = true;
                }
                auto positionId = keyValue.val.data.numeric<PositionId>();
                result.emplace_back(RecordDescriptor{classId, positionId});
            }
            return result;
        };

        template<typename T>
        static std::vector<RecordDescriptor> betweenSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler,
                                                                ClassId classId,
                                                                const T &lower,
                                                                const T &upper,
                                                                bool isLowerPositive,
                                                                const std::pair<bool, bool> &isIncludeBound) {
            auto result = std::vector<RecordDescriptor>{};
            if (!std::is_same<T, double>::value || isLowerPositive) {
                for (auto keyValue = cursorHandler.findRange(lower);
                     !keyValue.empty();
                     keyValue = cursorHandler.getNext()) {
                    auto key = keyValue.key.data.numeric();
                    if (!isIncludeBound.first && key == lower) continue;
                    else if ((!isIncludeBound.second && key == upper) || key > upper) break;
                    auto positionId = keyValue.val.data.numeric<PositionId>();
                    result.emplace_back(RecordDescriptor{classId, positionId});
                }
            } else {
                if (isIncludeBound.first) {
                    auto partialResult = exactMatchIndex(cursorHandler, classId, lower);
                    result.insert(result.end(), partialResult.cbegin(), partialResult.cend());
                }
                cursorHandler.findRange(lower);
                for (auto keyValue = cursorHandler.getPrev();
                     !keyValue.empty();
                     keyValue = cursorHandler.getPrev()) {
                    auto key = keyValue.key.data.numeric();
                    if ((!isIncludeBound.second && key == upper) || key > upper) break;
                    auto positionId = keyValue.val.data.numeric<PositionId>();
                    result.emplace_back(RecordDescriptor{classId, positionId});
                }
            }
            return result;
        };

        inline static std::vector<RecordDescriptor> betweenSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler,
                                                                       ClassId classId,
                                                                       const std::string &lower,
                                                                       const std::string &upper,
                                                                       const std::pair<bool, bool> &isIncludeBound) {
            auto result = std::vector<RecordDescriptor>{};
            for (auto keyValue = cursorHandler.findRange(lower);
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto key = keyValue.key.data.string();
                if (!isIncludeBound.first && (key == lower)) continue;
                if ((!isIncludeBound.second && (key == upper)) || (key > upper)) break;
                auto positionId = keyValue.val.data.numeric<PositionId>();
                result.emplace_back(RecordDescriptor{classId, positionId});
            }
            return result;
        };

        static const std::vector<Condition::Comparator> validComparators;

    };

}

#endif
