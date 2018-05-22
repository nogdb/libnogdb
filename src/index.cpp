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

#include <utility>
#include <algorithm>

#include "index.hpp"
#include "generic.hpp"
#include "parser.hpp"
#include "utils.hpp"

#include "nogdb_txn.h"

namespace nogdb {

    const std::vector<Condition::Comparator>
            Index::validComparators = std::vector<Condition::Comparator>{
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

    auto cmpRecordDescriptor = [](const RecordDescriptor &lhs, const RecordDescriptor &rhs) {
        return lhs.rid < rhs.rid;
    };

    void Index::addIndex(BaseTxn &txn, IndexId indexId, PositionId positionId, const Bytes &bytesValue,
                         PropertyType type, bool isUnique) {
        auto dsTxnHandler = txn.getDsTxnHandler();
        if (!bytesValue.empty()) {
            auto indexRecord = Blob(sizeof(PositionId));
            indexRecord.append(&positionId, sizeof(PositionId));
            try {
                switch (type) {
                    case PropertyType::UNSIGNED_TINYINT:
                    case PropertyType::UNSIGNED_SMALLINT:
                    case PropertyType::UNSIGNED_INTEGER:
                    case PropertyType::UNSIGNED_BIGINT: {
                        auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                                     TB_INDEXING_PREFIX + std::to_string(indexId),
                                                                     true, isUnique);
                        if (type == PropertyType::UNSIGNED_TINYINT) {
                            Datastore::putRecord(dsTxnHandler, dataIndexDBHandler, bytesValue.toTinyIntU(), indexRecord,
                                                 false, !isUnique);
                        } else if (type == PropertyType::UNSIGNED_SMALLINT) {
                            Datastore::putRecord(dsTxnHandler, dataIndexDBHandler, bytesValue.toSmallIntU(), indexRecord,
                                                 false, !isUnique);
                        } else if (type == PropertyType::UNSIGNED_INTEGER) {
                            Datastore::putRecord(dsTxnHandler, dataIndexDBHandler, bytesValue.toIntU(), indexRecord,
                                                 false, !isUnique);
                        } else if (type == PropertyType::UNSIGNED_BIGINT) {
                            Datastore::putRecord(dsTxnHandler, dataIndexDBHandler, bytesValue.toBigIntU(), indexRecord,
                                                 false, !isUnique);
                        }
                        break;
                    }
                    case PropertyType::TINYINT:
                    case PropertyType::SMALLINT:
                    case PropertyType::INTEGER:
                    case PropertyType::BIGINT:
                    case PropertyType::REAL: {
                        auto dataIndexDBHandlerPositive =
                                Datastore::openDbi(dsTxnHandler,
                                                   TB_INDEXING_PREFIX + std::to_string(indexId) + "_positive",
                                                   true, isUnique);
                        auto dataIndexDBHandlerNegative =
                                Datastore::openDbi(dsTxnHandler,
                                                   TB_INDEXING_PREFIX + std::to_string(indexId) + "_negative",
                                                   true, isUnique);
                        if (type == PropertyType::TINYINT) {
                            auto value = bytesValue.toTinyInt();
                            Datastore::putRecord(dsTxnHandler,
                                                 (value >= 0) ? dataIndexDBHandlerPositive : dataIndexDBHandlerNegative,
                                                 value, indexRecord, false, !isUnique);
                        } else if (type == PropertyType::SMALLINT) {
                            auto value = bytesValue.toSmallInt();
                            Datastore::putRecord(dsTxnHandler,
                                                 (value >= 0) ? dataIndexDBHandlerPositive : dataIndexDBHandlerNegative,
                                                 value, indexRecord, false, !isUnique);
                        } else if (type == PropertyType::INTEGER) {
                            auto value = bytesValue.toInt();
                            Datastore::putRecord(dsTxnHandler,
                                                 (value >= 0) ? dataIndexDBHandlerPositive : dataIndexDBHandlerNegative,
                                                 value, indexRecord, false, !isUnique);
                        } else if (type == PropertyType::BIGINT) {
                            auto value = bytesValue.toBigInt();
                            Datastore::putRecord(dsTxnHandler,
                                                 (value >= 0) ? dataIndexDBHandlerPositive : dataIndexDBHandlerNegative,
                                                 value, indexRecord, false, !isUnique);
                        } else if (type == PropertyType::REAL) {
                            auto value = bytesValue.toReal();
                            Datastore::putRecord(dsTxnHandler,
                                                 (value >= 0) ? dataIndexDBHandlerPositive : dataIndexDBHandlerNegative,
                                                 value, indexRecord, false, !isUnique);
                        }
                        break;
                    }
                    case PropertyType::TEXT: {
                        auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                                     TB_INDEXING_PREFIX + std::to_string(indexId),
                                                                     false, isUnique);
                        auto value = bytesValue.toText();
                        if (!value.empty()) {
                            Datastore::putRecord(dsTxnHandler, dataIndexDBHandler, value, indexRecord,
                                                 false, !isUnique);
                        }
                        break;
                    }
                    default:
                        break;
                }
            } catch (Datastore::ErrorType &err) {
                if (err == MDB_KEYEXIST) {
                    throw Error(CTX_UNIQUE_CONSTRAINT, Error::Type::CONTEXT);
                } else {
                    throw Error(err, Error::Type::DATASTORE);
                }
            }
        }
    }

    void
    Index::deleteIndex(BaseTxn &txn, IndexId indexId, PositionId positionId, const Bytes &bytesValue, PropertyType type,
                       bool isUnique) {
        auto dsTxnHandler = txn.getDsTxnHandler();
        if (!bytesValue.empty()) {
            switch (type) {
                case PropertyType::UNSIGNED_TINYINT:
                case PropertyType::UNSIGNED_SMALLINT:
                case PropertyType::UNSIGNED_INTEGER:
                case PropertyType::UNSIGNED_BIGINT: {
                    auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                                 TB_INDEXING_PREFIX + std::to_string(indexId),
                                                                 true, isUnique);
                    auto cursorHandler = Datastore::openCursor(dsTxnHandler, dataIndexDBHandler);
                    if (type == PropertyType::UNSIGNED_TINYINT) {
                        deleteIndexCursor(cursorHandler, positionId, bytesValue.toTinyIntU());
                    } else if (type == PropertyType::UNSIGNED_SMALLINT) {
                        deleteIndexCursor(cursorHandler, positionId, bytesValue.toSmallIntU());
                    } else if (type == PropertyType::UNSIGNED_INTEGER) {
                        deleteIndexCursor(cursorHandler, positionId, bytesValue.toIntU());
                    } else if (type == PropertyType::UNSIGNED_BIGINT) {
                        deleteIndexCursor(cursorHandler, positionId, bytesValue.toBigIntU());
                    }
                    break;
                }
                case PropertyType::TINYINT:
                case PropertyType::SMALLINT:
                case PropertyType::INTEGER:
                case PropertyType::BIGINT:
                case PropertyType::REAL: {
                    auto dataIndexDBHandlerPositive =
                            Datastore::openDbi(dsTxnHandler, TB_INDEXING_PREFIX + std::to_string(indexId) + "_positive",
                                               true, isUnique);
                    auto dataIndexDBHandlerNegative =
                            Datastore::openDbi(dsTxnHandler, TB_INDEXING_PREFIX + std::to_string(indexId) + "_negative",
                                               true, isUnique);
                    auto cursorHandlerPositive = Datastore::openCursor(dsTxnHandler, dataIndexDBHandlerPositive);
                    auto cursorHandlerNegative = Datastore::openCursor(dsTxnHandler, dataIndexDBHandlerNegative);
                    if (type == PropertyType::TINYINT) {
                        auto value = bytesValue.toTinyInt();
                        (value < 0) ? deleteIndexCursor(cursorHandlerNegative, positionId, value)
                                    : deleteIndexCursor(cursorHandlerPositive, positionId, value);
                    } else if (type == PropertyType::SMALLINT) {
                        auto value = bytesValue.toSmallInt();
                        (value < 0) ? deleteIndexCursor(cursorHandlerNegative, positionId, value)
                                    : deleteIndexCursor(cursorHandlerPositive, positionId, value);
                    } else if (type == PropertyType::INTEGER) {
                        auto value = bytesValue.toInt();
                        (value < 0) ? deleteIndexCursor(cursorHandlerNegative, positionId, value)
                                    : deleteIndexCursor(cursorHandlerPositive, positionId, value);
                    } else if (type == PropertyType::BIGINT) {
                        auto value = bytesValue.toBigInt();
                        (value < 0) ? deleteIndexCursor(cursorHandlerNegative, positionId, value)
                                    : deleteIndexCursor(cursorHandlerPositive, positionId, value);
                    } else if (type == PropertyType::REAL) {
                        auto value = bytesValue.toReal();
                        (value < 0) ? deleteIndexCursor(cursorHandlerNegative, positionId, value)
                                    : deleteIndexCursor(cursorHandlerPositive, positionId, value);
                    }
                    break;
                }
                case PropertyType::TEXT: {
                    auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                                 TB_INDEXING_PREFIX + std::to_string(indexId),
                                                                 false, isUnique);
                    auto cursorHandler = Datastore::openCursor(dsTxnHandler, dataIndexDBHandler);
                    auto value = bytesValue.toText();
                    if (!value.empty()) {
                        deleteIndexCursor(cursorHandler, positionId, value);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    std::pair<Index::IndexPropertyType, bool>
    Index::hasIndex(ClassId classId, const ClassInfo &classInfo, const Condition &condition) {
        if (std::find(validComparators.cbegin(), validComparators.cend(), condition.comp) != validComparators.cend()) {
            // check if NOT is not used for EQUAL
            if (condition.comp == Condition::Comparator::EQUAL && condition.isNegative) {
                return std::make_pair(IndexPropertyType{}, false);
            }
            auto foundProperty = classInfo.propertyInfo.nameToDesc.find(condition.propName);
            if (foundProperty != classInfo.propertyInfo.nameToDesc.cend()) {
                for (const auto &index: foundProperty->second.indexInfo) {
                    if (index.second.first == classId) {
                        return std::make_pair(
                                IndexPropertyType{index.first, index.second.second, foundProperty->second.type}, true);
                    }
                }
            }
        }
        return std::make_pair(IndexPropertyType{}, false);
    }

    std::pair<std::map<std::string, Index::IndexPropertyType>, bool>
    Index::hasIndex(ClassId classId, const ClassInfo &classInfo, const MultiCondition &conditions) {
        auto result = std::map<std::string, Index::IndexPropertyType>{};
        auto isFoundAll = true;
        for (const auto &condition: conditions.conditions) {
            if (auto conditionPtr = condition.lock()) {
                auto propertyName = conditionPtr->getCondition().propName;
                if (result.find(propertyName) == result.cend()) {
                    auto tmpResult = hasIndex(classId, classInfo, conditionPtr->getCondition());
                    if (tmpResult.second) {
                        result.emplace(propertyName, tmpResult.first);
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
        return std::make_pair((isFoundAll) ? result : std::map<std::string, Index::IndexPropertyType>{}, isFoundAll);
    }

    std::vector<RecordDescriptor> Index::getIndexRecord(const Txn &txn,
                                                        ClassId classId,
                                                        IndexPropertyType indexPropertyType,
                                                        const Condition &condition,
                                                        bool isNegative) {
        auto sortByRdesc = [](std::vector<RecordDescriptor>& recordDescriptors) {
            std::sort(recordDescriptors.begin(), recordDescriptors.end(), cmpRecordDescriptor);
        };

        auto isApplyNegative = condition.isNegative ^isNegative;
        switch (condition.comp) {
            case Condition::Comparator::EQUAL: {
                if (!isApplyNegative) {
                    auto result = getEqual(txn, classId, indexPropertyType, condition.valueBytes);
                    sortByRdesc(result);
                    return result;
                } else {
                    auto lessResult = getLess(txn, classId, indexPropertyType, condition.valueBytes);
                    auto greaterResult = getGreater(txn, classId, indexPropertyType, condition.valueBytes);
                    lessResult.insert(lessResult.end(), greaterResult.cbegin(), greaterResult.cend());
                    sortByRdesc(lessResult);
                    return lessResult;
                }
            }
            case Condition::Comparator::LESS_EQUAL: {
                if (!isApplyNegative) {
                    auto result = getLessEqual(txn, classId, indexPropertyType, condition.valueBytes);
                    sortByRdesc(result);
                    return result;
                } else {
                    auto result = getGreater(txn, classId, indexPropertyType, condition.valueBytes);
                    sortByRdesc(result);
                    return result;
                }
            }
            case Condition::Comparator::LESS: {
                if (!isApplyNegative) {
                    auto result = getLess(txn, classId, indexPropertyType, condition.valueBytes);
                    sortByRdesc(result);
                    return result;
                } else {
                    auto result = getGreaterEqual(txn, classId, indexPropertyType, condition.valueBytes);
                    sortByRdesc(result);
                    return result;
                }
            }
            case Condition::Comparator::GREATER_EQUAL: {
                if (!isApplyNegative) {
                    auto result = getGreaterEqual(txn, classId, indexPropertyType, condition.valueBytes);
                    sortByRdesc(result);
                    return result;
                } else {
                    auto result = getLess(txn, classId, indexPropertyType, condition.valueBytes);
                    sortByRdesc(result);
                    return result;
                }
            }
            case Condition::Comparator::GREATER: {
                if (!isApplyNegative) {
                    auto result = getGreater(txn, classId, indexPropertyType, condition.valueBytes);
                    sortByRdesc(result);
                    return result;
                } else {
                    auto result = getLessEqual(txn, classId, indexPropertyType, condition.valueBytes);
                    sortByRdesc(result);
                    return result;
                }
            }
            case Condition::Comparator::BETWEEN_NO_BOUND: {
                if (!isApplyNegative) {
                    auto result = getBetween(txn, classId, indexPropertyType,
                                             condition.valueSet[0], condition.valueSet[1], {false, false});
                    sortByRdesc(result);
                    return result;
                } else {
                    auto lessResult = getLessEqual(txn, classId, indexPropertyType, condition.valueSet[0]);
                    auto greaterResult = getGreaterEqual(txn, classId, indexPropertyType, condition.valueSet[1]);
                    lessResult.insert(lessResult.end(), greaterResult.cbegin(), greaterResult.cend());
                    sortByRdesc(lessResult);
                    return lessResult;
                }
            }
            case Condition::Comparator::BETWEEN: {
                if (!isApplyNegative) {
                    auto result = getBetween(txn, classId, indexPropertyType,
                                             condition.valueSet[0], condition.valueSet[1], {true, true});
                    sortByRdesc(result);
                    return result;
                } else {
                    auto lessResult = getLess(txn, classId, indexPropertyType, condition.valueSet[0]);
                    auto greaterResult = getGreater(txn, classId, indexPropertyType, condition.valueSet[1]);
                    lessResult.insert(lessResult.end(), greaterResult.cbegin(), greaterResult.cend());
                    sortByRdesc(lessResult);
                    return lessResult;
                }
            }
            case Condition::Comparator::BETWEEN_NO_UPPER: {
                if (!isApplyNegative) {
                    auto result = getBetween(txn, classId, indexPropertyType,
                                             condition.valueSet[0], condition.valueSet[1], {true, false});
                    sortByRdesc(result);
                    return result;
                } else {
                    auto lessResult = getLess(txn, classId, indexPropertyType, condition.valueSet[0]);
                    auto greaterResult = getGreaterEqual(txn, classId, indexPropertyType, condition.valueSet[1]);
                    lessResult.insert(lessResult.end(), greaterResult.cbegin(), greaterResult.cend());
                    sortByRdesc(lessResult);
                    return lessResult;
                }
            }
            case Condition::Comparator::BETWEEN_NO_LOWER: {
                if (!isApplyNegative) {
                    auto result = getBetween(txn, classId, indexPropertyType,
                                             condition.valueSet[0], condition.valueSet[1], {false, true});
                    sortByRdesc(result);
                    return result;
                } else {
                    auto lessResult = getLessEqual(txn, classId, indexPropertyType, condition.valueSet[0]);
                    auto greaterResult = getGreater(txn, classId, indexPropertyType, condition.valueSet[1]);
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

    std::vector<RecordDescriptor> Index::getIndexRecord(const Txn &txn,
                                                        ClassId classId,
                                                        const std::map<std::string, IndexPropertyType> &indexPropertyTypes,
                                                        const MultiCondition &conditions) {
        std::function<std::vector<RecordDescriptor>(const MultiCondition::CompositeNode *, bool)>
                getRecordFromIndex = [&](const MultiCondition::CompositeNode *compositeNode, bool isParentNegative) {
            auto getResult = [&](const std::shared_ptr<MultiCondition::ExprNode> &exprNode,
                                 bool isNegative) -> std::vector<RecordDescriptor> {
                if (!exprNode->checkIfCondition()) {
                    auto compositeNodePtr = (MultiCondition::CompositeNode *) exprNode.get();
                    return getRecordFromIndex(compositeNodePtr, isNegative);
                } else {
                    auto conditionNodePtr = (MultiCondition::ConditionNode *) exprNode.get();
                    auto &condition = conditionNodePtr->getCondition();
                    auto indexPropertyTypeMap = indexPropertyTypes.find(condition.propName);
                    require(indexPropertyTypeMap != indexPropertyTypes.cend());
                    return getIndexRecord(txn, classId, indexPropertyTypeMap->second, condition, isNegative);
                }
            };
            auto &opt = compositeNode->getOperator();
            auto &rightNode = compositeNode->getRightNode();
            auto &leftNode = compositeNode->getLeftNode();
            auto isApplyNegative = compositeNode->getIsNegative() ^isParentNegative;
            auto result = std::vector<RecordDescriptor>{};
            auto rightNodeResult = getResult(rightNode, isApplyNegative);
            auto leftNodeResult = getResult(leftNode, isApplyNegative);
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
        getRecordFromIndex(conditions.root.get(), false);
        return std::vector<RecordDescriptor>{};
    }

    std::vector<RecordDescriptor>
    Index::getLessEqual(const Txn &txn, ClassId classId, const IndexPropertyType &indexPropertyType,
                        const Bytes &value) {
        auto &indexId = std::get<0>(indexPropertyType);
        auto &isUnique = std::get<1>(indexPropertyType);
        auto &propertyType = std::get<2>(indexPropertyType);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        switch (propertyType) {
            case PropertyType::UNSIGNED_TINYINT:
            case PropertyType::UNSIGNED_SMALLINT:
            case PropertyType::UNSIGNED_INTEGER:
            case PropertyType::UNSIGNED_BIGINT: {
                auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                             TB_INDEXING_PREFIX + std::to_string(indexId),
                                                             true, isUnique);
                auto cursorHandler = Datastore::openCursor(dsTxnHandler, dataIndexDBHandler);
                if (propertyType == PropertyType::UNSIGNED_TINYINT) {
                    return backwardSearchIndex(cursorHandler, classId, value.toTinyIntU(), true, true);
                } else if (propertyType == PropertyType::UNSIGNED_SMALLINT) {
                    return backwardSearchIndex(cursorHandler, classId, value.toSmallIntU(), true, true);
                } else if (propertyType == PropertyType::UNSIGNED_INTEGER) {
                    return backwardSearchIndex(cursorHandler, classId, value.toIntU(), true, true);
                } else {
                    return backwardSearchIndex(cursorHandler, classId, value.toBigIntU(), true, true);
                }
            }
            case PropertyType::TINYINT:
                return getLess(txn, classId, indexId, isUnique, value.toTinyInt(), true);
            case PropertyType::SMALLINT:
                return getLess(txn, classId, indexId, isUnique, value.toSmallInt(), true);
            case PropertyType::INTEGER:
                return getLess(txn, classId, indexId, isUnique, value.toInt(), true);
            case PropertyType::BIGINT:
                return getLess(txn, classId, indexId, isUnique, value.toBigInt(), true);
            case PropertyType::REAL:
                return getLess(txn, classId, indexId, isUnique, value.toReal(), true);
            case PropertyType::TEXT: {
                auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                             TB_INDEXING_PREFIX + std::to_string(indexId),
                                                             false, isUnique);
                auto cursorHandler = Datastore::openCursor(dsTxnHandler, dataIndexDBHandler);
                return backwardSearchIndex(cursorHandler, classId, value.toText(), true, true);
            }
            default:
                break;
        }
        return std::vector<RecordDescriptor>{};
    }

    std::vector<RecordDescriptor>
    Index::getLess(const Txn &txn, ClassId classId, const IndexPropertyType &indexPropertyType, const Bytes &value) {
        auto &indexId = std::get<0>(indexPropertyType);
        auto &isUnique = std::get<1>(indexPropertyType);
        auto &propertyType = std::get<2>(indexPropertyType);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        switch (propertyType) {
            case PropertyType::UNSIGNED_TINYINT:
            case PropertyType::UNSIGNED_SMALLINT:
            case PropertyType::UNSIGNED_INTEGER:
            case PropertyType::UNSIGNED_BIGINT: {
                auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                             TB_INDEXING_PREFIX + std::to_string(indexId),
                                                             true, isUnique);
                auto cursorHandler = Datastore::openCursor(dsTxnHandler, dataIndexDBHandler);
                if (propertyType == PropertyType::UNSIGNED_TINYINT) {
                    return backwardSearchIndex(cursorHandler, classId, value.toTinyIntU(), true);
                } else if (propertyType == PropertyType::UNSIGNED_SMALLINT) {
                    return backwardSearchIndex(cursorHandler, classId, value.toSmallIntU(), true);
                } else if (propertyType == PropertyType::UNSIGNED_INTEGER) {
                    return backwardSearchIndex(cursorHandler, classId, value.toIntU(), true);
                } else {
                    return backwardSearchIndex(cursorHandler, classId, value.toBigIntU(), true);
                }
            }
            case PropertyType::TINYINT:
                return getLess(txn, classId, indexId, isUnique, value.toTinyInt());
            case PropertyType::SMALLINT:
                return getLess(txn, classId, indexId, isUnique, value.toSmallInt());
            case PropertyType::INTEGER:
                return getLess(txn, classId, indexId, isUnique, value.toInt());
            case PropertyType::BIGINT:
                return getLess(txn, classId, indexId, isUnique, value.toBigInt());
            case PropertyType::REAL:
                return getLess(txn, classId, indexId, isUnique, value.toReal());
            case PropertyType::TEXT: {
                auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                             TB_INDEXING_PREFIX + std::to_string(indexId),
                                                             false, isUnique);
                auto cursorHandler = Datastore::openCursor(dsTxnHandler, dataIndexDBHandler);
                return backwardSearchIndex(cursorHandler, classId, value.toText(), true);
            }
            default:
                break;
        }
        return std::vector<RecordDescriptor>{};
    }

    std::vector<RecordDescriptor>
    Index::getEqual(const Txn &txn, ClassId classId, const IndexPropertyType &indexPropertyType, const Bytes &value) {
        auto &indexId = std::get<0>(indexPropertyType);
        auto &isUnique = std::get<1>(indexPropertyType);
        auto &propertyType = std::get<2>(indexPropertyType);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        switch (propertyType) {
            case PropertyType::UNSIGNED_TINYINT:
            case PropertyType::UNSIGNED_SMALLINT:
            case PropertyType::UNSIGNED_INTEGER:
            case PropertyType::UNSIGNED_BIGINT: {
                auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                             TB_INDEXING_PREFIX + std::to_string(indexId),
                                                             true, isUnique);
                auto cursorHandler = Datastore::openCursor(dsTxnHandler, dataIndexDBHandler);
                if (propertyType == PropertyType::UNSIGNED_TINYINT) {
                    return exactMatchIndex(cursorHandler, classId, value.toTinyIntU());
                } else if (propertyType == PropertyType::UNSIGNED_SMALLINT) {
                    return exactMatchIndex(cursorHandler, classId, value.toSmallIntU());
                } else if (propertyType == PropertyType::UNSIGNED_INTEGER) {
                    return exactMatchIndex(cursorHandler, classId, value.toIntU());
                } else {
                    return exactMatchIndex(cursorHandler, classId, value.toBigIntU());
                }
            }
            case PropertyType::TINYINT:
                return getEqual(txn, classId, indexId, isUnique, value.toTinyInt());
            case PropertyType::SMALLINT:
                return getEqual(txn, classId, indexId, isUnique, value.toSmallInt());
            case PropertyType::INTEGER:
                return getEqual(txn, classId, indexId, isUnique, value.toInt());
            case PropertyType::BIGINT:
                return getEqual(txn, classId, indexId, isUnique, value.toBigInt());
            case PropertyType::REAL:
                return getEqual(txn, classId, indexId, isUnique, value.toReal());
            case PropertyType::TEXT: {
                auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                             TB_INDEXING_PREFIX + std::to_string(indexId),
                                                             false, isUnique);
                auto cursorHandler = Datastore::openCursor(dsTxnHandler, dataIndexDBHandler);
                return exactMatchIndex(cursorHandler, classId, value.toText());
            }
            default:
                break;
        }
        return std::vector<RecordDescriptor>{};
    }

    std::vector<RecordDescriptor>
    Index::getGreaterEqual(const Txn &txn, ClassId classId, const IndexPropertyType &indexPropertyType,
                           const Bytes &value) {
        auto &indexId = std::get<0>(indexPropertyType);
        auto &isUnique = std::get<1>(indexPropertyType);
        auto &propertyType = std::get<2>(indexPropertyType);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        switch (propertyType) {
            case PropertyType::UNSIGNED_TINYINT:
            case PropertyType::UNSIGNED_SMALLINT:
            case PropertyType::UNSIGNED_INTEGER:
            case PropertyType::UNSIGNED_BIGINT: {
                auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                             TB_INDEXING_PREFIX + std::to_string(indexId),
                                                             true, isUnique);
                auto cursorHandler = Datastore::openCursor(dsTxnHandler, dataIndexDBHandler);
                if (propertyType == PropertyType::UNSIGNED_TINYINT) {
                    return forwardSearchIndex(cursorHandler, classId, value.toTinyIntU(), true, true);
                } else if (propertyType == PropertyType::UNSIGNED_SMALLINT) {
                    return forwardSearchIndex(cursorHandler, classId, value.toSmallIntU(), true, true);
                } else if (propertyType == PropertyType::UNSIGNED_INTEGER) {
                    return forwardSearchIndex(cursorHandler, classId, value.toIntU(), true, true);
                } else {
                    return forwardSearchIndex(cursorHandler, classId, value.toBigIntU(), true, true);
                }
            }
            case PropertyType::TINYINT:
                return getGreater(txn, classId, indexId, isUnique, value.toTinyInt(), true);
            case PropertyType::SMALLINT:
                return getGreater(txn, classId, indexId, isUnique, value.toSmallInt(), true);
            case PropertyType::INTEGER:
                return getGreater(txn, classId, indexId, isUnique, value.toInt(), true);
            case PropertyType::BIGINT:
                return getGreater(txn, classId, indexId, isUnique, value.toBigInt(), true);
            case PropertyType::REAL:
                return getGreater(txn, classId, indexId, isUnique, value.toReal(), true);
            case PropertyType::TEXT: {
                auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                             TB_INDEXING_PREFIX + std::to_string(indexId),
                                                             false, isUnique);
                auto cursorHandler = Datastore::openCursor(dsTxnHandler, dataIndexDBHandler);
                return forwardSearchIndex(cursorHandler, classId, value.toText(), true);
            }
            default:
                break;
        }
        return std::vector<RecordDescriptor>{};
    }

    std::vector<RecordDescriptor>
    Index::getGreater(const Txn &txn, ClassId classId, const IndexPropertyType &indexPropertyType, const Bytes &value) {
        auto &indexId = std::get<0>(indexPropertyType);
        auto &isUnique = std::get<1>(indexPropertyType);
        auto &propertyType = std::get<2>(indexPropertyType);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        switch (propertyType) {
            case PropertyType::UNSIGNED_TINYINT:
            case PropertyType::UNSIGNED_SMALLINT:
            case PropertyType::UNSIGNED_INTEGER:
            case PropertyType::UNSIGNED_BIGINT: {
                auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                             TB_INDEXING_PREFIX + std::to_string(indexId),
                                                             true, isUnique);
                auto cursorHandler = Datastore::openCursor(dsTxnHandler, dataIndexDBHandler);
                if (propertyType == PropertyType::UNSIGNED_TINYINT) {
                    return forwardSearchIndex(cursorHandler, classId, value.toTinyIntU(), true);
                } else if (propertyType == PropertyType::UNSIGNED_SMALLINT) {
                    return forwardSearchIndex(cursorHandler, classId, value.toSmallIntU(), true);
                } else if (propertyType == PropertyType::UNSIGNED_INTEGER) {
                    return forwardSearchIndex(cursorHandler, classId, value.toIntU(), true);
                } else {
                    return forwardSearchIndex(cursorHandler, classId, value.toBigIntU(), true);
                }
            }
            case PropertyType::TINYINT:
                return getGreater(txn, classId, indexId, isUnique, value.toTinyInt());
            case PropertyType::SMALLINT:
                return getGreater(txn, classId, indexId, isUnique, value.toSmallInt());
            case PropertyType::INTEGER:
                return getGreater(txn, classId, indexId, isUnique, value.toInt());
            case PropertyType::BIGINT:
                return getGreater(txn, classId, indexId, isUnique, value.toBigInt());
            case PropertyType::REAL:
                return getGreater(txn, classId, indexId, isUnique, value.toReal());
            case PropertyType::TEXT: {
                auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                             TB_INDEXING_PREFIX + std::to_string(indexId),
                                                             false, isUnique);
                auto cursorHandler = Datastore::openCursor(dsTxnHandler, dataIndexDBHandler);
                return forwardSearchIndex(cursorHandler, classId, value.toText());
            }
            default:
                break;
        }
        return std::vector<RecordDescriptor>{};
    }

    std::vector<RecordDescriptor> Index::getBetween(const Txn &txn,
                                                    ClassId classId,
                                                    const IndexPropertyType &indexPropertyType,
                                                    const Bytes &lowerBound,
                                                    const Bytes &upperBound,
                                                    const std::pair<bool, bool> &isIncludeBound) {
        auto &indexId = std::get<0>(indexPropertyType);
        auto &isUnique = std::get<1>(indexPropertyType);
        auto &propertyType = std::get<2>(indexPropertyType);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        switch (propertyType) {
            case PropertyType::UNSIGNED_TINYINT:
            case PropertyType::UNSIGNED_SMALLINT:
            case PropertyType::UNSIGNED_INTEGER:
            case PropertyType::UNSIGNED_BIGINT: {
                auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                             TB_INDEXING_PREFIX + std::to_string(indexId),
                                                             true, isUnique);
                auto cursorHandler = Datastore::openCursor(dsTxnHandler, dataIndexDBHandler);
                if (propertyType == PropertyType::UNSIGNED_TINYINT) {
                    return betweenSearchIndex(cursorHandler, classId, lowerBound.toTinyIntU(), upperBound.toTinyIntU(),
                                              true, isIncludeBound);
                } else if (propertyType == PropertyType::UNSIGNED_SMALLINT) {
                    return betweenSearchIndex(cursorHandler, classId, lowerBound.toSmallIntU(),
                                              upperBound.toSmallIntU(), true, isIncludeBound);
                } else if (propertyType == PropertyType::UNSIGNED_INTEGER) {
                    return betweenSearchIndex(cursorHandler, classId, lowerBound.toIntU(), upperBound.toIntU(),
                                              true, isIncludeBound);
                } else {
                    return betweenSearchIndex(cursorHandler, classId, lowerBound.toBigIntU(), upperBound.toBigIntU(),
                                              true, isIncludeBound);
                }
            }
            case PropertyType::TINYINT:
                return getBetween(txn, classId, indexId, isUnique, lowerBound.toTinyInt(), upperBound.toTinyInt(),
                                  isIncludeBound);
            case PropertyType::SMALLINT:
                return getBetween(txn, classId, indexId, isUnique, lowerBound.toSmallInt(), upperBound.toSmallInt(),
                                  isIncludeBound);
            case PropertyType::INTEGER:
                return getBetween(txn, classId, indexId, isUnique, lowerBound.toInt(), upperBound.toInt(),
                                  isIncludeBound);
            case PropertyType::BIGINT:
                return getBetween(txn, classId, indexId, isUnique, lowerBound.toBigInt(), upperBound.toBigInt(),
                                  isIncludeBound);
            case PropertyType::REAL:
                return getBetween(txn, classId, indexId, isUnique, lowerBound.toReal(), upperBound.toReal(),
                                  isIncludeBound);
            case PropertyType::TEXT: {
                auto dataIndexDBHandler = Datastore::openDbi(dsTxnHandler,
                                                             TB_INDEXING_PREFIX + std::to_string(indexId),
                                                             false, isUnique);
                auto cursorHandler = Datastore::openCursor(dsTxnHandler, dataIndexDBHandler);
                return betweenSearchIndex(cursorHandler, classId, lowerBound.toText(), upperBound.toText(),
                                          isIncludeBound);
            }
            default:
                break;
        }
        return std::vector<RecordDescriptor>{};
    }

}