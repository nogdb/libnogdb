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

#ifndef __INDEX_HPP_INCLUDED_
#define __INDEX_HPP_INCLUDED_

#include <iostream> // for debugging
#include <vector>
#include <unordered_set>
#include <type_traits>
#include <functional>

#include "schema.hpp"
#include "lmdb_engine.hpp"
#include "schema_adapter.hpp"
#include "index_adapter.hpp"
#include "datarecord_adapter.hpp"

#include "nogdb_types.h"
#include "nogdb_txn.h"
#include "nogdb_compare.h"

#define UNIQUE_FLAG(_unique)                        (_unique)? INDEX_TYPE_UNIQUE: INDEX_TYPE_NON_UNIQUE
#define INDEX_POSITIVE_NUMERIC_UNIQUE(_unique)      INDEX_TYPE_POSITIVE | INDEX_TYPE_NUMERIC | UNIQUE_FLAG(_unique)
#define INDEX_NEGATIVE_NUMERIC_UNIQUE(_unique)      INDEX_TYPE_NEGATIVE | INDEX_TYPE_NUMERIC | UNIQUE_FLAG(_unique)
#define INDEX_STRING_UNIQUE(_unique)                INDEX_TYPE_POSITIVE | INDEX_TYPE_STRING | UNIQUE_FLAG(_unique)

namespace nogdb {

    namespace index {

        using namespace adapter::schema;

        typedef std::map<PropertyId, IndexAccessInfo> PropertyIdMapIndex;

        class IndexInterface {
        public:

            IndexInterface(const Txn* txn): _txn{txn} {}

            virtual ~IndexInterface() noexcept = default;

            void initialize(const PropertyAccessInfo& propertyInfo, const IndexAccessInfo& indexInfo, const ClassType& classType) {
                switch (propertyInfo.type) {
                    case PropertyType::UNSIGNED_TINYINT:
                        createNumeric<uint64_t>(propertyInfo, indexInfo, classType, [](const Bytes& value) {
                            return static_cast<uint64_t>(value.toTinyIntU());
                        }); break;
                    case PropertyType::UNSIGNED_SMALLINT:
                        createNumeric<uint64_t>(propertyInfo, indexInfo, classType, [](const Bytes& value) {
                            return static_cast<uint64_t>(value.toSmallIntU());
                        }); break;
                    case PropertyType::UNSIGNED_INTEGER:
                        createNumeric<uint64_t>(propertyInfo, indexInfo, classType, [](const Bytes& value) {
                            return static_cast<uint64_t>(value.toIntU());
                        }); break;
                    case PropertyType::UNSIGNED_BIGINT:
                        createNumeric<uint64_t>(propertyInfo, indexInfo, classType, [](const Bytes& value) {
                            return value.toBigIntU();
                        }); break;
                    case PropertyType::TINYINT:
                        createSignedNumeric<int64_t>(propertyInfo, indexInfo, classType, [](const Bytes& value) {
                            return static_cast<int64_t>(value.toTinyInt());
                        }); break;
                    case PropertyType::SMALLINT:
                        createSignedNumeric<int64_t>(propertyInfo, indexInfo, classType, [](const Bytes& value) {
                            return static_cast<int64_t>(value.toSmallInt());
                        }); break;
                    case PropertyType::INTEGER:
                        createSignedNumeric<int64_t>(propertyInfo, indexInfo, classType, [](const Bytes& value) {
                            return static_cast<int64_t>(value.toInt());
                        }); break;
                    case PropertyType::BIGINT:
                        createSignedNumeric<int64_t>(propertyInfo, indexInfo, classType, [](const Bytes& value) {
                            return value.toBigInt();
                        }); break;
                    case PropertyType::REAL:
                        createSignedNumeric<double>(propertyInfo, indexInfo, classType, [](const Bytes& value) {
                            return value.toReal();
                        }); break;
                    case PropertyType::TEXT:
                        createString(propertyInfo, indexInfo, classType); break;
                    default:
                        break;
                }
            }

            void drop(const PropertyAccessInfo& propertyInfo, const IndexAccessInfo& indexInfo) {
                switch (propertyInfo.type) {
                    case PropertyType::UNSIGNED_TINYINT:
                    case PropertyType::UNSIGNED_SMALLINT:
                    case PropertyType::UNSIGNED_INTEGER:
                    case PropertyType::UNSIGNED_BIGINT: {
                        openIndexRecordPositive(indexInfo).destroy();
                        break;
                    }
                    case PropertyType::TINYINT:
                    case PropertyType::SMALLINT:
                    case PropertyType::INTEGER:
                    case PropertyType::BIGINT:
                    case PropertyType::REAL: {
                        openIndexRecordPositive(indexInfo).destroy();
                        openIndexRecordNegative(indexInfo).destroy();
                        break;
                    }
                    case PropertyType::TEXT: {
                        openIndexRecordString(indexInfo).destroy();
                        break;
                    }
                    default:
                        break;
                }
            }

            void drop(const ClassId& classId, const PropertyNameMapInfo& propertyNameMapInfo) {
                for(const auto& property: propertyNameMapInfo) {
                    auto indexInfo = _txn->_index->getInfo(classId, property.second.id);
                    if (indexInfo.id != IndexId{}) {
                        drop(property.second, indexInfo);
                    }
                }
            }

            void insert(const PropertyAccessInfo& propertyInfo, const IndexAccessInfo& indexInfo,
                        const PositionId& posId, const Bytes &value) {
                if (!value.empty()) {
                    try {
                        switch (propertyInfo.type) {
                            case PropertyType::UNSIGNED_TINYINT:
                                insert(indexInfo, posId, static_cast<uint64_t>(value.toTinyIntU())); break;
                            case PropertyType::UNSIGNED_SMALLINT:
                                insert(indexInfo, posId, static_cast<uint64_t>(value.toSmallIntU())); break;
                            case PropertyType::UNSIGNED_INTEGER:
                                insert(indexInfo, posId, static_cast<uint64_t>(value.toIntU())); break;
                            case PropertyType::UNSIGNED_BIGINT:
                                insert(indexInfo, posId, value.toBigIntU()); break;
                            case PropertyType::TINYINT:
                                insertSignedNumeric(indexInfo, posId, static_cast<int64_t>(value.toTinyInt())); break;
                            case PropertyType::SMALLINT:
                                insertSignedNumeric(indexInfo, posId, static_cast<int64_t>(value.toSmallInt())); break;
                            case PropertyType::INTEGER:
                                insertSignedNumeric(indexInfo, posId, static_cast<int64_t>(value.toInt())); break;
                            case PropertyType::BIGINT:
                                insertSignedNumeric(indexInfo, posId, value.toBigInt()); break;
                            case PropertyType::REAL:
                                insertSignedNumeric(indexInfo, posId, value.toReal()); break;
                            case PropertyType::TEXT: {
                                auto valueString = value.toText();
                                if (!valueString.empty()) {
                                    insert(indexInfo, posId, valueString);
                                }
                                break;
                            }
                            default:
                                break;
                        }
                    } catch (const Error &err) {
                        if (err.code() == MDB_KEYEXIST) {
                            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_UNIQUE_CONSTRAINT);
                        } else {
                            throw NOGDB_FATAL_ERROR(err);
                        }
                    }
                }
            }

            void insert(const RecordDescriptor& recordDescriptor,
                        const Record& record,
                        const PropertyNameMapInfo& propertyNameMapInfo) {
                for(const auto& property: record.getAll()) {
                    if (property.second.empty()) continue;
                    auto foundProperty = propertyNameMapInfo.find(property.first);
                    if (foundProperty == propertyNameMapInfo.cend()) {
                        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
                    } else {
                        auto indexInfo = _txn->_index->getInfo(recordDescriptor.rid.first, foundProperty->second.id);
                        if (indexInfo.id != IndexId{}) {
                            insert(foundProperty->second, indexInfo, recordDescriptor.rid.second, property.second);
                        }
                    }
                }
            }

            void remove(const PropertyAccessInfo& propertyInfo, const IndexAccessInfo& indexInfo,
                        const PositionId& posId, const Bytes &value) {
                if (!value.empty()) {
                    switch (propertyInfo.type) {
                        case PropertyType::UNSIGNED_TINYINT:
                            removeByCursor(indexInfo, posId, static_cast<uint64_t>(value.toTinyIntU())); break;
                        case PropertyType::UNSIGNED_SMALLINT:
                            removeByCursor(indexInfo, posId, static_cast<uint64_t>(value.toSmallIntU())); break;
                        case PropertyType::UNSIGNED_INTEGER:
                            removeByCursor(indexInfo, posId, static_cast<uint64_t>(value.toIntU())); break;
                        case PropertyType::UNSIGNED_BIGINT:
                            removeByCursor(indexInfo, posId, value.toBigIntU()); break;
                        case PropertyType::TINYINT:
                            removeByCursorWithSignNumeric(indexInfo, posId, static_cast<int64_t>(value.toTinyInt())); break;
                        case PropertyType::SMALLINT:
                            removeByCursorWithSignNumeric(indexInfo, posId, static_cast<int64_t>(value.toSmallInt())); break;
                        case PropertyType::INTEGER:
                            removeByCursorWithSignNumeric(indexInfo, posId, static_cast<int64_t>(value.toInt())); break;
                        case PropertyType::BIGINT:
                            removeByCursorWithSignNumeric(indexInfo, posId, value.toBigInt()); break;
                        case PropertyType::REAL:
                            removeByCursorWithSignNumeric(indexInfo, posId, value.toReal()); break;
                        case PropertyType::TEXT: {
                            auto valueString = value.toText();
                            if (!valueString.empty()) {
                                removeByCursor(indexInfo, posId, valueString);
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            }

            void remove(const RecordDescriptor& recordDescriptor,
                        const Record& record,
                        const PropertyNameMapInfo& propertyNameMapInfo) {
                for(const auto& property: record.getAll()) {
                    if (property.second.empty()) continue;
                    auto foundProperty = propertyNameMapInfo.find(property.first);
                    if (foundProperty == propertyNameMapInfo.cend()) {
                        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
                    } else {
                        auto indexInfo = _txn->_index->getInfo(recordDescriptor.rid.first, foundProperty->second.id);
                        if (indexInfo.id != IndexId{}) {
                            remove(foundProperty->second, indexInfo, recordDescriptor.rid.second, property.second);
                        }
                    }
                }
            }

            std::pair<bool, IndexAccessInfo>
            hasIndex(const ClassAccessInfo &classInfo, const Condition &condition) const {
                if (isValidComparator(condition)) {
                    // check if NOT is not used for EQUAL
                    if (condition.comp == Condition::Comparator::EQUAL && condition.isNegative) {
                        return std::make_pair(false, IndexAccessInfo{});
                    }
                    auto propertyNameMapInfo = _txn->_iSchema->getPropertyNameMapInfo(classInfo.id, classInfo.superClassId);
                    auto foundProperty = propertyNameMapInfo.find(condition.propName);
                    if (foundProperty != propertyNameMapInfo.cend()) {
                        auto indexInfo = _txn->_index->getInfo(classInfo.id, foundProperty->second.id);
                        return std::make_pair(indexInfo.id != IndexId{}, indexInfo);
                    }
                }
                return std::make_pair(false, IndexAccessInfo{});
            }

            std::pair<bool, PropertyIdMapIndex>
            hasIndex(const ClassAccessInfo &classInfo, const MultiCondition &conditions) const {
                auto isFoundAll = true;
                auto result = PropertyIdMapIndex{};
                auto conditionPropNames = std::unordered_set<std::string>{};
                for (const auto &condition: conditions.conditions) {
                    if (auto conditionPtr = condition.lock()) {
                        auto propertyName = conditionPtr->getCondition().propName;
                        if (conditionPropNames.find(propertyName) == conditionPropNames.cend()) {
                            conditionPropNames.emplace(propertyName);
                            auto searchIndexResult = hasIndex(classInfo, conditionPtr->getCondition());
                            if (searchIndexResult.first) {
                                auto propertyId = _txn->_property->getId(classInfo.id, propertyName);
                                result.emplace(propertyId, searchIndexResult.second);
                            } else {
                                isFoundAll = false;
                                break;
                            }
                        }
                    } else {
                        isFoundAll = false;
                        break;
                    }
                }
                return std::make_pair(isFoundAll, (isFoundAll) ? result : PropertyIdMapIndex{});
            }

            std::vector<RecordDescriptor>
            getRecord(const PropertyAccessInfo& propertyInfo, const IndexAccessInfo& indexInfo,
                      const Condition &condition, bool isNegative) const {
                auto isApplyNegative = condition.isNegative ^isNegative;
                switch (condition.comp) {
                    case Condition::Comparator::EQUAL: {
                        if (!isApplyNegative) {
                            auto result = getEqual(propertyInfo, indexInfo, condition.valueBytes);
                            sortByRdesc(result);
                            return result;
                        } else {
                            auto lessResult = getLessThan(propertyInfo, indexInfo, condition.valueBytes);
                            auto greaterResult = getGreaterThan(propertyInfo, indexInfo, condition.valueBytes);
                            lessResult.insert(lessResult.end(), greaterResult.cbegin(), greaterResult.cend());
                            sortByRdesc(lessResult);
                            return lessResult;
                        }
                    }
                    case Condition::Comparator::LESS_EQUAL: {
                        if (!isApplyNegative) {
                            auto result = getLessOrEqual(propertyInfo, indexInfo, condition.valueBytes);
                            sortByRdesc(result);
                            return result;
                        } else {
                            auto result = getGreaterThan(propertyInfo, indexInfo, condition.valueBytes);
                            sortByRdesc(result);
                            return result;
                        }
                    }
                    case Condition::Comparator::LESS: {
                        if (!isApplyNegative) {
                            auto result = getLessThan(propertyInfo, indexInfo, condition.valueBytes);
                            sortByRdesc(result);
                            return result;
                        } else {
                            auto result = getGreaterOrEqual(propertyInfo, indexInfo, condition.valueBytes);
                            sortByRdesc(result);
                            return result;
                        }
                    }
                    case Condition::Comparator::GREATER_EQUAL: {
                        if (!isApplyNegative) {
                            auto result = getGreaterOrEqual(propertyInfo, indexInfo, condition.valueBytes);
                            sortByRdesc(result);
                            return result;
                        } else {
                            auto result = getLessThan(propertyInfo, indexInfo, condition.valueBytes);
                            sortByRdesc(result);
                            return result;
                        }
                    }
                    case Condition::Comparator::GREATER: {
                        if (!isApplyNegative) {
                            auto result = getGreaterThan(propertyInfo, indexInfo, condition.valueBytes);
                            sortByRdesc(result);
                            return result;
                        } else {
                            auto result = getLessOrEqual(propertyInfo, indexInfo, condition.valueBytes);
                            sortByRdesc(result);
                            return result;
                        }
                    }
                    case Condition::Comparator::BETWEEN_NO_BOUND: {
                        if (!isApplyNegative) {
                            auto result = getBetween(propertyInfo, indexInfo,
                                                     condition.valueSet[0], condition.valueSet[1], {false, false});
                            sortByRdesc(result);
                            return result;
                        } else {
                            auto lessResult = getLessOrEqual(propertyInfo, indexInfo, condition.valueSet[0]);
                            auto greaterResult = getGreaterOrEqual(propertyInfo, indexInfo, condition.valueSet[1]);
                            lessResult.insert(lessResult.end(), greaterResult.cbegin(), greaterResult.cend());
                            sortByRdesc(lessResult);
                            return lessResult;
                        }
                    }
                    case Condition::Comparator::BETWEEN: {
                        if (!isApplyNegative) {
                            auto result = getBetween(propertyInfo, indexInfo,
                                                     condition.valueSet[0], condition.valueSet[1], {true, true});
                            sortByRdesc(result);
                            return result;
                        } else {
                            auto lessResult = getLessThan(propertyInfo, indexInfo, condition.valueSet[0]);
                            auto greaterResult = getGreaterThan(propertyInfo, indexInfo, condition.valueSet[1]);
                            lessResult.insert(lessResult.end(), greaterResult.cbegin(), greaterResult.cend());
                            sortByRdesc(lessResult);
                            return lessResult;
                        }
                    }
                    case Condition::Comparator::BETWEEN_NO_UPPER: {
                        if (!isApplyNegative) {
                            auto result = getBetween(propertyInfo, indexInfo,
                                                     condition.valueSet[0], condition.valueSet[1], {true, false});
                            sortByRdesc(result);
                            return result;
                        } else {
                            auto lessResult = getLessThan(propertyInfo, indexInfo, condition.valueSet[0]);
                            auto greaterResult = getGreaterOrEqual(propertyInfo, indexInfo, condition.valueSet[1]);
                            lessResult.insert(lessResult.end(), greaterResult.cbegin(), greaterResult.cend());
                            sortByRdesc(lessResult);
                            return lessResult;
                        }
                    }
                    case Condition::Comparator::BETWEEN_NO_LOWER: {
                        if (!isApplyNegative) {
                            auto result = getBetween(propertyInfo, indexInfo,
                                                     condition.valueSet[0], condition.valueSet[1], {false, true});
                            sortByRdesc(result);
                            return result;
                        } else {
                            auto lessResult = getLessOrEqual(propertyInfo, indexInfo, condition.valueSet[0]);
                            auto greaterResult = getGreaterThan(propertyInfo, indexInfo, condition.valueSet[1]);
                            lessResult.insert(lessResult.end(), greaterResult.cbegin(), greaterResult.cend());
                            sortByRdesc(lessResult);
                            return lessResult;
                        }
                    }
                    default:
                        break;
                }
                return std::vector<RecordDescriptor>{};
            }

            std::vector<RecordDescriptor>
            getRecord(const PropertyNameMapInfo& propertyInfos,
                      const PropertyIdMapIndex& propertyIndexInfo,
                      const MultiCondition &conditions) const {
                return getRecordFromMultiCondition(propertyInfos, propertyIndexInfo, conditions.root.get(), false);
            }

        protected:

            const std::vector<Condition::Comparator> validComparators {
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

            const Txn *_txn;

            adapter::index::IndexRecord openIndexRecordPositive(const IndexAccessInfo& indexInfo) const {
                auto indexFlags = INDEX_POSITIVE_NUMERIC_UNIQUE(indexInfo.isUnique);
                auto indexAccess = adapter::index::IndexRecord{_txn->_txnBase, indexInfo.id, (unsigned int)indexFlags};
                return std::move(indexAccess);
            }

            adapter::index::IndexRecord openIndexRecordNegative(const IndexAccessInfo& indexInfo) const {
                auto indexFlags = INDEX_NEGATIVE_NUMERIC_UNIQUE(indexInfo.isUnique);
                auto indexAccess = adapter::index::IndexRecord{_txn->_txnBase, indexInfo.id, (unsigned int)indexFlags};
                return std::move(indexAccess);
            }

            adapter::index::IndexRecord openIndexRecordString(const IndexAccessInfo& indexInfo) const {
                auto indexFlags = INDEX_STRING_UNIQUE(indexInfo.isUnique);
                auto indexAccess = adapter::index::IndexRecord{_txn->_txnBase, indexInfo.id, (unsigned int)indexFlags};
                return std::move(indexAccess);
            }

            template <typename T>
            void createNumeric(const PropertyAccessInfo& propertyInfo,
                               const IndexAccessInfo &indexInfo,
                               const ClassType& classType,
                               T(*valueRetrieve)(const Bytes&)) {
                auto propertyIdMapInfo = _txn->_property->getIdMapInfo(indexInfo.classId);
                auto indexAccess = openIndexRecordPositive(indexInfo);
                auto cursorHandler = adapter::datarecord::DataRecord(_txn->_txnBase, indexInfo.classId, classType).getCursor();
                for(auto keyValue = cursorHandler.getNext();
                    !keyValue.empty();
                    keyValue = cursorHandler.getNext()) {
                    auto const positionId = keyValue.key.data.numeric<PositionId>();
                    if (positionId == MAX_RECORD_NUM_EM) continue;
                    auto const record = parser::Parser::parseRawData(keyValue.val, propertyIdMapInfo, classType == ClassType::EDGE);
                    auto bytesValue = record.get(propertyInfo.name);
                    if (!bytesValue.empty()) {
                        auto indexRecord = Blob(sizeof(PositionId)).append(&positionId, sizeof(PositionId));
                        indexAccess.create(valueRetrieve(bytesValue), indexRecord);
                    }
                }
            }

            template <typename T>
            void createSignedNumeric(const PropertyAccessInfo& propertyInfo,
                                     const IndexAccessInfo &indexInfo,
                                     const ClassType & classType,
                                     T(*valueRetrieve)(const Bytes&)) {
                auto propertyIdMapInfo = _txn->_property->getIdMapInfo(indexInfo.classId);
                auto indexPositiveAccess = openIndexRecordPositive(indexInfo);
                auto indexNegativeAccess = openIndexRecordNegative(indexInfo);
                auto cursorHandler = adapter::datarecord::DataRecord{_txn->_txnBase, indexInfo.classId, classType}.getCursor();
                for(auto keyValue = cursorHandler.getNext();
                    !keyValue.empty();
                    keyValue = cursorHandler.getNext()) {
                    auto const positionId = keyValue.key.data.numeric<PositionId>();
                    if (positionId == MAX_RECORD_NUM_EM) continue;
                    auto const record = parser::Parser::parseRawData(keyValue.val, propertyIdMapInfo, classType == ClassType::EDGE);
                    auto bytesValue = record.get(propertyInfo.name);
                    if (!bytesValue.empty()) {
                        auto indexRecord = Blob(sizeof(PositionId)).append(&positionId, sizeof(PositionId));
                        auto value = valueRetrieve(bytesValue);
                        (value >= 0)? indexPositiveAccess.create(value, indexRecord): indexNegativeAccess.create(value, indexRecord);
                    }
                }
            }

            void createString(const PropertyAccessInfo& propertyInfo, const IndexAccessInfo &indexInfo, const ClassType & classType) {
                auto propertyIdMapInfo = _txn->_property->getIdMapInfo(indexInfo.classId);
                auto indexAccess = openIndexRecordString(indexInfo);
                auto cursorHandler = adapter::datarecord::DataRecord{_txn->_txnBase, indexInfo.classId, classType}.getCursor();
                for(auto keyValue = cursorHandler.getNext();
                    !keyValue.empty();
                    keyValue = cursorHandler.getNext()) {
                    auto const positionId = keyValue.key.data.numeric<PositionId>();
                    if (positionId == MAX_RECORD_NUM_EM) continue;
                    auto const record = parser::Parser::parseRawData(keyValue.val, propertyIdMapInfo, classType == ClassType::EDGE);
                    auto value = record.get(propertyInfo.name).toText();
                    if (!value.empty()) {
                        auto indexRecord = Blob(sizeof(PositionId)).append(&positionId, sizeof(PositionId));
                        indexAccess.create(value, indexRecord);
                    }
                }
            }

            template <typename T>
            void insert(const IndexAccessInfo &indexInfo, PositionId positionId, const T &value) {
                auto indexAccess = openIndexRecordPositive(indexInfo);
                auto indexRecord = Blob(sizeof(PositionId)).append(&positionId, sizeof(PositionId));
                indexAccess.create(value, indexRecord);
            }

            void insert(const IndexAccessInfo &indexInfo, PositionId positionId, const std::string &value) {
                auto indexAccess = openIndexRecordString(indexInfo);
                auto indexRecord = Blob(sizeof(PositionId)).append(&positionId, sizeof(PositionId));
                indexAccess.create(value, indexRecord);
            }

            template <typename T>
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

            template <typename T>
            void removeByCursorNumeric(const storage_engine::lmdb::Cursor& cursor, PositionId positionId, const T& value) {
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
            void removeByCursor(const IndexAccessInfo& indexInfo, PositionId positionId, const T& value) {
                auto indexAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
                removeByCursorNumeric(indexAccessCursor, positionId, value);
            }

            void removeByCursor(const IndexAccessInfo& indexInfo, PositionId positionId, const std::string &value) {
                auto indexAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
                for (auto keyValue = indexAccessCursor.find(value);
                     !keyValue.empty();
                     keyValue = indexAccessCursor.getNext()) {
                    auto key = keyValue.key.data.string();
                    if (value == key) {
                        auto valueAsPositionId = keyValue.val.data.numeric<PositionId>();
                        if (positionId == valueAsPositionId) {
                            indexAccessCursor.del();
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }

            template<typename T>
            void removeByCursorWithSignNumeric(const IndexAccessInfo& indexInfo, const PositionId& posId, const T& value) {
                auto indexPositiveAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
                auto indexNegativeAccessCursor = openIndexRecordNegative(indexInfo).getCursor();
                (value < 0) ? removeByCursorNumeric(indexNegativeAccessCursor, posId, value):
                              removeByCursorNumeric(indexPositiveAccessCursor, posId, value);
            }

            inline static bool cmpRecordDescriptor = [](const RecordDescriptor &lhs, const RecordDescriptor &rhs) {
                return lhs.rid < rhs.rid;
            };

            inline static void sortByRdesc(std::vector<RecordDescriptor>& recordDescriptors) {
                std::sort(recordDescriptors.begin(), recordDescriptors.end(), cmpRecordDescriptor);
            };

            inline bool isValidComparator(const Condition& condition) const {
                return std::find(validComparators.cbegin(), validComparators.cend(), condition.comp) != validComparators.cend();
            }

            std::vector<RecordDescriptor>
            getRecordFromMultiCondition(const PropertyNameMapInfo& propertyInfos,
                                        const PropertyIdMapIndex& propertyIndexInfo,
                                        const MultiCondition::CompositeNode *compositeNode,
                                        bool isParentNegative) const {
                auto &opt = compositeNode->getOperator();
                auto &rightNode = compositeNode->getRightNode();
                auto &leftNode = compositeNode->getLeftNode();
                auto isApplyNegative = compositeNode->getIsNegative() ^isParentNegative;
                auto result = std::vector<RecordDescriptor>{};
                auto rightNodeResult = getMultiConditionResult(propertyInfos, propertyIndexInfo, rightNode, isApplyNegative);
                auto leftNodeResult = getMultiConditionResult(propertyInfos, propertyIndexInfo, leftNode, isApplyNegative);
                if ((opt == MultiCondition::Operator::AND && !isApplyNegative) ||
                    (opt == MultiCondition::Operator::OR && isApplyNegative)) {
                    // AND action
                    std::set_intersection(rightNodeResult.begin(), rightNodeResult.end(),
                                          leftNodeResult.begin(), leftNodeResult.end(),
                                          std::back_inserter(result), cmpRecordDescriptor);
                } else {
                    // OR action
                    std::set_union(rightNodeResult.begin(), rightNodeResult.end(),
                                   leftNodeResult.begin(), leftNodeResult.end(),
                                   std::back_inserter(result), cmpRecordDescriptor);
                }
                return result;
            };

            std::vector<RecordDescriptor>
            getMultiConditionResult(const PropertyNameMapInfo& propertyInfos,
                                    const PropertyIdMapIndex& propertyIndexInfo,
                                    const std::shared_ptr<MultiCondition::ExprNode> &exprNode,
                                    bool isNegative) const {
                if (!exprNode->checkIfCondition()) {
                    auto compositeNodePtr = (MultiCondition::CompositeNode *) exprNode.get();
                    return getRecordFromMultiCondition(propertyInfos, propertyIndexInfo, compositeNodePtr, isNegative);
                } else {
                    auto conditionNodePtr = (MultiCondition::ConditionNode *) exprNode.get();
                    auto &condition = conditionNodePtr->getCondition();
                    auto foundProperty = propertyInfos.find(condition.propName);
                    require(foundProperty != propertyInfos.cend());
                    auto &propertyInfo = foundProperty->second;
                    auto foundIndexInfo = propertyIndexInfo.find(propertyInfo.id);
                    require(foundIndexInfo != propertyIndexInfo.cend());
                    return getRecord(propertyInfo, foundIndexInfo->second, condition, isNegative);
                }
            };

            std::vector<RecordDescriptor>
            getLessOrEqual(const PropertyAccessInfo& propertyInfo, const IndexAccessInfo& indexInfo, const Bytes &value) const {
                return getLessCommon(propertyInfo, indexInfo, value, true);
            }

            std::vector<RecordDescriptor>
            getLessThan(const PropertyAccessInfo& propertyInfo, const IndexAccessInfo& indexInfo, const Bytes &value) const {
                return getLessCommon(propertyInfo, indexInfo, value, false);
            }

            std::vector<RecordDescriptor>
            getEqual(const PropertyAccessInfo& propertyInfo, const IndexAccessInfo& indexInfo, const Bytes &value) const {
                switch (propertyInfo.type) {
                    case PropertyType::UNSIGNED_TINYINT:
                    case PropertyType::UNSIGNED_SMALLINT:
                    case PropertyType::UNSIGNED_INTEGER:
                    case PropertyType::UNSIGNED_BIGINT: {
                        auto indexAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
                        if (propertyInfo.type == PropertyType::UNSIGNED_TINYINT) {
                            return exactMatchIndex(indexAccessCursor, indexInfo.classId, static_cast<uint64_t>(value.toTinyIntU()));
                        } else if (propertyInfo.type == PropertyType::UNSIGNED_SMALLINT) {
                            return exactMatchIndex(indexAccessCursor, indexInfo.classId, static_cast<uint64_t>(value.toSmallIntU()));
                        } else if (propertyInfo.type == PropertyType::UNSIGNED_INTEGER) {
                            return exactMatchIndex(indexAccessCursor, indexInfo.classId, static_cast<uint64_t>(value.toIntU()));
                        } else {
                            return exactMatchIndex(indexAccessCursor, indexInfo.classId, value.toBigIntU());
                        }
                    }
                    case PropertyType::TINYINT:
                        return getEqualNumeric(static_cast<int64_t>(value.toTinyInt()), indexInfo);
                    case PropertyType::SMALLINT:
                        return getEqualNumeric(static_cast<int64_t>(value.toSmallInt()), indexInfo);
                    case PropertyType::INTEGER:
                        return getEqualNumeric(static_cast<int64_t>(value.toInt()), indexInfo);
                    case PropertyType::BIGINT:
                        return getEqualNumeric(value.toBigInt(), indexInfo);
                    case PropertyType::REAL:
                        return getEqualNumeric(value.toReal(), indexInfo);
                    case PropertyType::TEXT: {
                        return exactMatchIndex(openIndexRecordString(indexInfo).getCursor(), indexInfo.classId, value.toText());
                    }
                    default:
                        break;
                }
                return std::vector<RecordDescriptor>{};
            }

            std::vector<RecordDescriptor>
            getGreaterOrEqual(const PropertyAccessInfo& propertyInfo, const IndexAccessInfo& indexInfo, const Bytes &value) const {
                return getGreaterCommon(propertyInfo, indexInfo, value, true);
            }

            std::vector<RecordDescriptor>
            getGreaterThan(const PropertyAccessInfo& propertyInfo, const IndexAccessInfo& indexInfo, const Bytes &value) const {
                return getGreaterCommon(propertyInfo, indexInfo, value, false);
            }

            std::vector<RecordDescriptor>
            getBetween(const PropertyAccessInfo& propertyInfo, const IndexAccessInfo& indexInfo,
                       const Bytes &lowerBound, const Bytes &upperBound, const std::pair<bool, bool> &isIncludeBound) const {
                switch (propertyInfo.type) {
                    case PropertyType::UNSIGNED_TINYINT:
                    case PropertyType::UNSIGNED_SMALLINT:
                    case PropertyType::UNSIGNED_INTEGER:
                    case PropertyType::UNSIGNED_BIGINT: {
                        auto indexAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
                        if (propertyInfo.type == PropertyType::UNSIGNED_TINYINT) {
                            return betweenSearchIndex(indexAccessCursor, indexInfo.classId,
                                                      static_cast<uint64_t>(lowerBound.toTinyIntU()),
                                                      static_cast<uint64_t>(upperBound.toTinyIntU()),
                                                      true, isIncludeBound);
                        } else if (propertyInfo.type == PropertyType::UNSIGNED_SMALLINT) {
                            return betweenSearchIndex(indexAccessCursor, indexInfo.classId,
                                                      static_cast<uint64_t>(lowerBound.toSmallIntU()),
                                                      static_cast<uint64_t>(upperBound.toSmallIntU()),
                                                      true, isIncludeBound);
                        } else if (propertyInfo.type == PropertyType::UNSIGNED_INTEGER) {
                            return betweenSearchIndex(indexAccessCursor, indexInfo.classId,
                                                      static_cast<uint64_t>(lowerBound.toIntU()),
                                                      static_cast<uint64_t>(upperBound.toIntU()),
                                                      true, isIncludeBound);
                        } else {
                            return betweenSearchIndex(indexAccessCursor, indexInfo.classId,
                                                      lowerBound.toBigIntU(), upperBound.toBigIntU(),
                                                      true, isIncludeBound);
                        }
                    }
                    case PropertyType::TINYINT:
                        return getBetweenNumeric(static_cast<int64_t>(lowerBound.toTinyInt()),
                                                 static_cast<int64_t>(upperBound.toTinyInt()),
                                                 indexInfo, isIncludeBound);
                    case PropertyType::SMALLINT:
                        return getBetweenNumeric(static_cast<int64_t>(lowerBound.toSmallInt()),
                                                 static_cast<int64_t>(upperBound.toSmallInt()),
                                                 indexInfo, isIncludeBound);
                    case PropertyType::INTEGER:
                        return getBetweenNumeric(static_cast<int64_t>(lowerBound.toInt()),
                                                 static_cast<int64_t>(upperBound.toInt()),
                                                 indexInfo, isIncludeBound);
                    case PropertyType::BIGINT:
                        return getBetweenNumeric(lowerBound.toBigInt(), upperBound.toBigInt(), indexInfo, isIncludeBound);
                    case PropertyType::REAL:
                        return getBetweenNumeric(lowerBound.toReal(), upperBound.toReal(), indexInfo, isIncludeBound);
                    case PropertyType::TEXT: {
                        return betweenSearchIndex(openIndexRecordString(indexInfo).getCursor(), indexInfo.classId,
                                                  lowerBound.toText(), upperBound.toText(), isIncludeBound);
                    }
                    default:
                        break;
                }
                return std::vector<RecordDescriptor>{};
            }

            std::vector<RecordDescriptor>
            getLessCommon(const PropertyAccessInfo& propertyInfo, const IndexAccessInfo& indexInfo, const Bytes &value, bool isEqual) const {
                switch (propertyInfo.type) {
                    case PropertyType::UNSIGNED_TINYINT:
                    case PropertyType::UNSIGNED_SMALLINT:
                    case PropertyType::UNSIGNED_INTEGER:
                    case PropertyType::UNSIGNED_BIGINT: {
                        auto indexAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
                        if (propertyInfo.type == PropertyType::UNSIGNED_TINYINT) {
                            return backwardSearchIndex(indexAccessCursor, indexInfo.classId, static_cast<uint64_t>(value.toTinyIntU()), true, isEqual);
                        } else if (propertyInfo.type == PropertyType::UNSIGNED_SMALLINT) {
                            return backwardSearchIndex(indexAccessCursor, indexInfo.classId, static_cast<uint64_t>(value.toSmallIntU()), true, isEqual);
                        } else if (propertyInfo.type == PropertyType::UNSIGNED_INTEGER) {
                            return backwardSearchIndex(indexAccessCursor, indexInfo.classId, static_cast<uint64_t>(value.toIntU()), true, isEqual);
                        } else {
                            return backwardSearchIndex(indexAccessCursor, indexInfo.classId, value.toBigIntU(), true, isEqual);
                        }
                    }
                    case PropertyType::TINYINT:
                        return getLessNumeric(static_cast<int64_t>(value.toTinyInt()), indexInfo, isEqual);
                    case PropertyType::SMALLINT:
                        return getLessNumeric(static_cast<int64_t>(value.toSmallInt()), indexInfo, isEqual);
                    case PropertyType::INTEGER:
                        return getLessNumeric(static_cast<int64_t>(value.toInt()), indexInfo, isEqual);
                    case PropertyType::BIGINT:
                        return getLessNumeric(value.toBigInt(), indexInfo, isEqual);
                    case PropertyType::REAL:
                        return getLessNumeric(value.toReal(), indexInfo, isEqual);
                    case PropertyType::TEXT: {
                        return backwardSearchIndex(openIndexRecordString(indexInfo).getCursor(), indexInfo.classId, value.toText(), true, isEqual);
                    }
                    default:
                        break;
                }
            }

            std::vector<RecordDescriptor>
            getGreaterCommon(const PropertyAccessInfo& propertyInfo, const IndexAccessInfo& indexInfo, const Bytes &value, bool isEqual) const {
                switch (propertyInfo.type) {
                    case PropertyType::UNSIGNED_TINYINT:
                    case PropertyType::UNSIGNED_SMALLINT:
                    case PropertyType::UNSIGNED_INTEGER:
                    case PropertyType::UNSIGNED_BIGINT: {
                        auto indexAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
                        if (propertyInfo.type == PropertyType::UNSIGNED_TINYINT) {
                            return forwardSearchIndex(indexAccessCursor, indexInfo.classId, static_cast<uint64_t>(value.toTinyIntU()), true, isEqual);
                        } else if (propertyInfo.type == PropertyType::UNSIGNED_SMALLINT) {
                            return forwardSearchIndex(indexAccessCursor, indexInfo.classId, static_cast<uint64_t>(value.toSmallIntU()), true, isEqual);
                        } else if (propertyInfo.type == PropertyType::UNSIGNED_INTEGER) {
                            return forwardSearchIndex(indexAccessCursor, indexInfo.classId, static_cast<uint64_t>(value.toIntU()), true, isEqual);
                        } else {
                            return forwardSearchIndex(indexAccessCursor, indexInfo.classId, value.toBigIntU(), true, isEqual);
                        }
                    }
                    case PropertyType::TINYINT:
                        return getGreaterNumeric(static_cast<int64_t>(value.toTinyInt()), indexInfo, isEqual);
                    case PropertyType::SMALLINT:
                        return getGreaterNumeric(static_cast<int64_t>(value.toSmallInt()), indexInfo, isEqual);
                    case PropertyType::INTEGER:
                        return getGreaterNumeric(static_cast<int64_t>(value.toInt()), indexInfo, isEqual);
                    case PropertyType::BIGINT:
                        return getGreaterNumeric(value.toBigInt(), indexInfo, isEqual);
                    case PropertyType::REAL:
                        return getGreaterNumeric(value.toReal(), indexInfo, isEqual);
                    case PropertyType::TEXT: {
                        return forwardSearchIndex(openIndexRecordString(indexInfo).getCursor(), indexInfo.classId, value.toText(), isEqual);
                    }
                    default:
                        break;
                }
                return std::vector<RecordDescriptor>{};
            }

            template<typename T>
            std::vector<RecordDescriptor>
            getLessNumeric(const T &value, const IndexAccessInfo &indexInfo, bool includeEqual = false) const {
                if (value < 0) {
                    auto indexAccessCursor = openIndexRecordNegative(indexInfo).getCursor();
                    return backwardSearchIndex(indexAccessCursor, indexInfo.classId, value, false, includeEqual);
                } else {
                    auto indexPositiveAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
                    auto indexNegativeAccessCursor = openIndexRecordNegative(indexInfo).getCursor();
                    auto positiveResult = backwardSearchIndex(indexPositiveAccessCursor, indexInfo.classId, value, true, includeEqual);
                    auto negativeResult = fullScanIndex(indexNegativeAccessCursor, indexInfo.classId);
                    positiveResult.insert(positiveResult.end(), negativeResult.cbegin(), negativeResult.cend());
                    return positiveResult;
                }
            };

            template<typename T>
            std::vector<RecordDescriptor>
            getEqualNumeric(const T &value, const IndexAccessInfo &indexInfo) const {
                auto indexAccess = (value < 0)? openIndexRecordNegative(indexInfo): openIndexRecordPositive(indexInfo);
                return exactMatchIndex(indexAccess.getCursor(), indexInfo.classId, value);
            };

            template<typename T>
            std::vector<RecordDescriptor>
            getGreaterNumeric(const T &value, const IndexAccessInfo &indexInfo, bool includeEqual = false) const {
                if (value < 0) {
                    auto indexPositiveAccessCursor = openIndexRecordPositive(indexInfo).getCursor();
                    auto indexNegativeAccessCursor = openIndexRecordNegative(indexInfo).getCursor();
                    auto positiveResult = fullScanIndex(indexPositiveAccessCursor, indexInfo.classId);
                    auto negativeResult = forwardSearchIndex(indexNegativeAccessCursor, indexInfo.classId, value, false, includeEqual);
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
                    return betweenSearchIndex(indexAccessCursor, indexInfo.classId, lowerBound, upperBound, false, isIncludeBound);
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
            backwardSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler, const ClassId& classId,
                                const T &value, bool positive, bool isInclude = false) {
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
            exactMatchIndex(const storage_engine::lmdb::Cursor& cursorHandler, const ClassId& classId, const T &value) {
                auto result = std::vector<RecordDescriptor>{};
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
                return result;
            };

            static std::vector<RecordDescriptor>
            exactMatchIndex(const storage_engine::lmdb::Cursor& cursorHandler, const ClassId& classId, const std::string &value) {
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

            static std::vector<RecordDescriptor>
            fullScanIndex(const storage_engine::lmdb::Cursor& cursorHandler, const ClassId& classId) {
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
            static std::vector<RecordDescriptor>
            forwardSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler, const ClassId& classId,
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
                        auto partialResult = exactMatchIndex(cursorHandler, classId, value);
                        result.insert(result.end(), partialResult.cbegin(), partialResult.cend());
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
            forwardSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler, const ClassId& classId,
                               const std::string &value, bool isInclude = false) {
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
            static std::vector<RecordDescriptor>
            betweenSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler, const ClassId& classId,
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
                        auto partialResult = exactMatchIndex(cursorHandler, classId, lower);
                        result.insert(result.end(), partialResult.cbegin(), partialResult.cend());
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
            betweenSearchIndex(const storage_engine::lmdb::Cursor& cursorHandler, const ClassId& classId,
                               const std::string &lower, const std::string &upper,
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

        };

    }

}

#endif
