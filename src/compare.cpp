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

#include <iostream> // for debugging
#include <vector>
#include <algorithm>
#include <regex>

#include "shared_lock.hpp"
#include "constant.hpp"
#include "env_handler.hpp"
#include "datastore.hpp"
#include "graph.hpp"
#include "parser.hpp"
#include "generic.hpp"
#include "compare.hpp"
#include "index.hpp"
#include "utils.hpp"

#include "nogdb_errors.h"
#include "nogdb_compare.h"

namespace nogdb {

    bool Compare::compareBytesValue(const Bytes &value, PropertyType type, const Condition &condition) {
        auto to_lower = [](const std::string &text) {
            auto tmp = std::string{};
            std::transform(text.cbegin(), text.cend(), std::back_inserter(tmp), ::tolower);
            return tmp;
        };
        auto cmp_function = [&](const Bytes &cmpValue1,
                                const Bytes &cmpValue2,
                                Condition::Comparator cmp,
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
                            throw Error(CTX_INVALID_COMPARATOR, Error::Type::CONTEXT);
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
                            throw Error(CTX_INVALID_COMPARATOR, Error::Type::CONTEXT);
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
                            throw Error(CTX_INVALID_COMPARATOR, Error::Type::CONTEXT);
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
                            throw Error(CTX_INVALID_COMPARATOR, Error::Type::CONTEXT);
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
                            throw Error(CTX_INVALID_COMPARATOR, Error::Type::CONTEXT);
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
                            throw Error(CTX_INVALID_COMPARATOR, Error::Type::CONTEXT);
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
                            throw Error(CTX_INVALID_COMPARATOR, Error::Type::CONTEXT);
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
                            throw Error(CTX_INVALID_COMPARATOR, Error::Type::CONTEXT);
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
                            throw Error(CTX_INVALID_COMPARATOR, Error::Type::CONTEXT);
                    }
                case PropertyType::TEXT: {
                    auto textValue = (isIgnoreCase) ? to_lower(value.toText()) : value.toText();
                    auto textCmpValue1 = (isIgnoreCase) ? to_lower(cmpValue1.toText()) : cmpValue1.toText();
                    auto textCmpValue2 = (cmpValue2.empty()) ? "" : ((isIgnoreCase) ? to_lower(cmpValue2.toText())
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
                            replaceAll(textCmpValue1, "%", "(.*)");
                            replaceAll(textCmpValue1, "_", "(.)");
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
                            throw Error(CTX_INVALID_COMPARATOR, Error::Type::CONTEXT);
                    }
                }
                case PropertyType::BLOB:
                    switch (cmp) {
                        case Condition::Comparator::EQUAL:
                            return memcmp(value.getRaw(), cmpValue1.getRaw(), value.size()) == 0;
                        default:
                            throw Error(CTX_INVALID_COMPARATOR, Error::Type::CONTEXT);
                    }
                default:
                    throw Error(CTX_INVALID_PROPTYPE, Error::Type::CONTEXT);
            }
        };
        if (condition.comp == Condition::Comparator::IN) {
            for (const auto &valueBytes: condition.valueSet) {
                if (cmp_function(valueBytes, Bytes{}, Condition::Comparator::EQUAL, condition.isIgnoreCase) ^
                    condition.isNegative) {
                    return true;
                }
            }
        } else if (condition.comp >= Condition::Comparator::BETWEEN &&
                   condition.comp <= Condition::Comparator::BETWEEN_NO_BOUND) {
            return cmp_function(condition.valueSet[0], condition.valueSet[1], condition.comp, condition.isIgnoreCase) ^
                   condition.isNegative;
        } else {
            return cmp_function(condition.valueBytes, Bytes{}, condition.comp, condition.isIgnoreCase) ^
                   condition.isNegative;
        }
        return false;
    }

//*****************************************************************
//*  compare by condition and multi-condition object              *
//*****************************************************************

    ResultSet Compare::getRecordCondition(const Txn &txn,
                                          const std::vector<ClassInfo> &classInfos,
                                          const Condition &condition,
                                          PropertyType type) {
        auto result = ResultSet{};
        try {
            for (const auto &classInfo: classInfos) {
                auto classDBHandler = Datastore::openDbi(txn.txnBase->getDsTxnHandler(), std::to_string(classInfo.id), true);
                auto cursorHandler = Datastore::CursorHandlerWrapper(txn.txnBase->getDsTxnHandler(), classDBHandler);
                auto keyValue = Datastore::getNextCursor(cursorHandler.get());
                while (!keyValue.empty()) {
                    auto key = Datastore::getKeyAsNumeric<PositionId>(keyValue);
                    if (*key != EM_MAXRECNUM) {
                        auto rid = RecordId{classInfo.id, *key};
                        auto record = Parser::parseRawDataWithBasicInfo(classInfo.name, rid, keyValue, classInfo.propertyInfo);
                        if (condition.comp != Condition::Comparator::IS_NULL &&
                            condition.comp != Condition::Comparator::NOT_NULL) {
                            if (record.get(condition.propName).empty()) {
                                keyValue = Datastore::getNextCursor(cursorHandler.get());
                                continue;
                            }
                            if (compareBytesValue(record.get(condition.propName), type, condition)) {
                                result.push_back(Result{RecordDescriptor{classInfo.id, *key}, record});
                            }
                        } else {
                            switch (condition.comp) {
                                case Condition::Comparator::IS_NULL:
                                    if (record.get(condition.propName).empty()) {
                                        result.push_back(Result{RecordDescriptor{classInfo.id, *key}, record});
                                    }
                                    break;
                                case Condition::Comparator::NOT_NULL:
                                    if (!record.get(condition.propName).empty()) {
                                        result.push_back(Result{RecordDescriptor{classInfo.id, *key}, record});
                                    }
                                    break;
                                default:
                                    throw Error(CTX_INVALID_COMPARATOR, Error::Type::CONTEXT);
                            }
                        }
                    }
                    keyValue = Datastore::getNextCursor(cursorHandler.get());
                }
            }
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
        return result;
    }

    ResultSet Compare::getRecordMultiCondition(const Txn &txn,
                                               const std::vector<ClassInfo> &classInfos,
                                               const MultiCondition &conditions,
                                               const PropertyMapType &types) {
        auto result = ResultSet{};
        try {
            for (const auto &classInfo: classInfos) {
                auto classDBHandler = Datastore::openDbi(txn.txnBase->getDsTxnHandler(), std::to_string(classInfo.id), true);
                auto cursorHandler = Datastore::CursorHandlerWrapper(txn.txnBase->getDsTxnHandler(), classDBHandler);
                auto keyValue = Datastore::getNextCursor(cursorHandler.get());
                while (!keyValue.empty()) {
                    auto key = Datastore::getKeyAsNumeric<PositionId>(keyValue);
                    if (*key != EM_MAXRECNUM) {
                        auto rid = RecordId{classInfo.id, *key};
                        auto record = Parser::parseRawDataWithBasicInfo(classInfo.name, rid, keyValue, classInfo.propertyInfo);
                        if (conditions.execute(record, types)) {
                            result.push_back(Result{RecordDescriptor{classInfo.id, *key}, record});
                        }
                    }
                    keyValue = Datastore::getNextCursor(cursorHandler.get());
                }
            }
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
        return result;
    }

    ResultSet Compare::getEdgeCondition(const Txn &txn,
                                        const RecordDescriptor &recordDescriptor,
                                        const std::vector<ClassId> &edgeClassIds,
                                        std::vector<RecordId>
                                        (Graph::*func)(const BaseTxn &txn, const RecordId &rid, const ClassId &classId),
                                        const Condition &condition,
                                        PropertyType type) {
        switch (Generic::checkIfRecordExist(txn, recordDescriptor)) {
            case RECORD_NOT_EXIST:
                throw Error(GRAPH_NOEXST_VERTEX, Error::Type::GRAPH);
            case RECORD_NOT_EXIST_IN_MEMORY:
                return ResultSet{};
            default:
                auto result = ResultSet{};
                try {
                    auto classDescriptor = Schema::ClassDescriptorPtr{};
                    auto classPropertyInfo = ClassPropertyInfo{};
                    auto classDBHandler = Datastore::DBHandler{};
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
                                    throw Error(CTX_INVALID_COMPARATOR, Error::Type::CONTEXT);
                            }
                        }
                        return false;
                    };
                    auto retrieve = [&](ResultSet &result, const RecordId &edge) {
                        if (classDescriptor == nullptr || classDescriptor->id != edge.first) {
                            classDescriptor = Generic::getClassDescriptor(txn, edge.first, ClassType::UNDEFINED);
                            classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
                            classDBHandler = Datastore::openDbi(txn.txnBase->getDsTxnHandler(), std::to_string(edge.first), true);
                            className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
                        }
                        auto keyValue = Datastore::getRecord(txn.txnBase->getDsTxnHandler(), classDBHandler, edge.second);
                        auto record = Parser::parseRawDataWithBasicInfo(className, edge, keyValue, classPropertyInfo);
                        if (filter(record)) {
                            result.push_back(Result{RecordDescriptor{edge}, record});
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
                } catch (Graph::ErrorType &err) {
                    if (err == GRAPH_NOEXST_VERTEX) {
                        throw Error(GRAPH_UNKNOWN_ERR, Error::Type::GRAPH);
                    } else {
                        throw Error(err, Error::Type::GRAPH);
                    }
                } catch (Datastore::ErrorType &err) {
                    throw Error(err, Error::Type::DATASTORE);
                }
                return result;
        }
    }

    ResultSet Compare::getEdgeMultiCondition(const Txn &txn,
                                             const RecordDescriptor &recordDescriptor,
                                             const std::vector<ClassId> &edgeClassIds,
                                             std::vector<RecordId>
                                             (Graph::*func)(const BaseTxn &baseTxn, const RecordId &rid, const ClassId &classId),
                                             const MultiCondition &conditions,
                                             const PropertyMapType &types) {
        switch (Generic::checkIfRecordExist(txn, recordDescriptor)) {
            case RECORD_NOT_EXIST:
                throw Error(GRAPH_NOEXST_VERTEX, Error::Type::GRAPH);
            case RECORD_NOT_EXIST_IN_MEMORY:
                return ResultSet{};
            default:
                auto result = ResultSet{};
                try {
                    auto classDescriptor = Schema::ClassDescriptorPtr{};
                    auto classPropertyInfo = ClassPropertyInfo{};
                    auto classDBHandler = Datastore::DBHandler{};
                    auto className = std::string{};
                    auto retrieve = [&](ResultSet &result, const RecordId &edge) {
                        if (classDescriptor == nullptr || classDescriptor->id != edge.first) {
                            classDescriptor = Generic::getClassDescriptor(txn, edge.first, ClassType::UNDEFINED);
                            classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
                            classDBHandler = Datastore::openDbi(txn.txnBase->getDsTxnHandler(), std::to_string(edge.first), true);
                            className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
                        }
                        auto keyValue = Datastore::getRecord(txn.txnBase->getDsTxnHandler(), classDBHandler, edge.second);
                        auto record = Parser::parseRawDataWithBasicInfo(className, edge, keyValue, classPropertyInfo);
                        if (conditions.execute(record, types)) {
                            result.push_back(Result{RecordDescriptor{edge}, record});
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
                } catch (Graph::ErrorType &err) {
                    if (err == GRAPH_NOEXST_VERTEX) {
                        throw Error(GRAPH_UNKNOWN_ERR, Error::Type::GRAPH);
                    } else {
                        throw Error(err, Error::Type::GRAPH);
                    }
                } catch (Datastore::ErrorType &err) {
                    throw Error(err, Error::Type::DATASTORE);
                }
                return result;
        }
    }

    ResultSet Compare::compareCondition(const Txn &txn,
                                        const std::string &className,
                                        ClassType type,
                                        const Condition &condition,
                                        bool searchIndexOnly) {
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
                        throw Error(CTX_CONFLICT_PROPTYPE, Error::Type::CONTEXT);
                    }
                }
            }
        }
        if (propertyType == PropertyType::UNDEFINED) {
            throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
        }
        auto foundClassId = std::find_if(classDescriptors.cbegin(), classDescriptors.cend(),
                                         [&txn, &className](const Schema::ClassDescriptorPtr& ptr) {
            return BaseTxn::getCurrentVersion(*txn.txnBase, ptr->name).first == className;
        });
        auto &classId = (*foundClassId)->id;
        auto foundIndex = Index::hasIndex(classId, *classInfos.cbegin(), condition);
        if (foundIndex.second) {
            return Generic::getMultipleRecordFromRdesc(txn, Index::getIndexRecord(txn, classId, foundIndex.first, condition));
        }
        // if indexing is not found, then
        if (searchIndexOnly) {
            return ResultSet{};
        } else {
            return getRecordCondition(txn, classInfos, condition, propertyType);
        }
    }

    ResultSet Compare::compareMultiCondition(const Txn &txn,
                                             const std::string &className,
                                             ClassType type,
                                             const MultiCondition &conditions,
                                             bool searchIndexOnly) {
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
                            throw Error(CTX_CONFLICT_PROPTYPE, Error::Type::CONTEXT);
                        }
                    }
                }
            }
        }
        if (numOfUndefPropertyType != 0) {
            throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
        }
        auto foundClassId = std::find_if(classDescriptors.cbegin(), classDescriptors.cend(),
                                         [&txn, &className](const Schema::ClassDescriptorPtr& ptr) {
            return BaseTxn::getCurrentVersion(*txn.txnBase, ptr->name).first == className;
        });
        auto &classId = (*foundClassId)->id;
        auto foundIndex = Index::hasIndex(classId, *classInfos.cbegin(), conditions);
        if (foundIndex.second) {
            return Generic::getMultipleRecordFromRdesc(txn, Index::getIndexRecord(txn, classId, foundIndex.first, conditions));
        }
        if (searchIndexOnly) {
            return ResultSet{};
        } else {
            return getRecordMultiCondition(txn, classInfos, conditions, conditionPropertyTypes);
        }
    }

    ResultSet Compare::compareEdgeCondition(const Txn &txn,
                                            const RecordDescriptor &recordDescriptor,
                                            std::vector<RecordId>
                                            (Graph::*func1)(const BaseTxn &baseTxn, const RecordId &rid, const ClassId &classId),
                                            std::vector<ClassId>
                                            (Graph::*func2)(const BaseTxn &baseTxn, const RecordId &rid),
                                            const Condition &condition,
                                            const ClassFilter &classFilter) {
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
                            throw Error(CTX_CONFLICT_PROPTYPE, Error::Type::CONTEXT);
                        }
                    }
                }
            }
            return isFoundProperty;
        };

        auto edgeClassDescriptors = Generic::getMultipleClassDescriptor(txn, classFilter.getClassName(),
                                                                        ClassType::EDGE);
        if (!edgeClassDescriptors.empty()) {
            auto edgeClassInfos = Generic::getMultipleClassMapProperty(*txn.txnBase, edgeClassDescriptors);
            if (!validateProperty(edgeClassInfos, condition.propName)) {
                throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
            }
        } else {
            auto edgeClassIds = ((*txn.txnCtx.dbRelation).*func2)(*txn.txnBase, recordDescriptor.rid);
            auto edgeClassDescriptors = Generic::getMultipleClassDescriptor(txn, edgeClassIds, ClassType::EDGE);
            auto edgeClassInfos = Generic::getMultipleClassMapProperty(*txn.txnBase, edgeClassDescriptors);
            if (!validateProperty(edgeClassInfos, condition.propName)) {
                throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
            }
        }
        return getEdgeCondition(txn, recordDescriptor, edgeClassIds, func1, condition, propertyType);
    }

    ResultSet Compare::compareEdgeMultiCondition(const Txn &txn,
                                                 const RecordDescriptor &recordDescriptor,
                                                 std::vector<RecordId>
                                                 (Graph::*func1)(const BaseTxn &baseTxn, const RecordId &rid,
                                                                 const ClassId &classId),
                                                 std::vector<ClassId>
                                                 (Graph::*func2)(const BaseTxn &baseTxn, const RecordId &rid),
                                                 const MultiCondition &conditions,
                                                 const ClassFilter &classFilter) {
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
                                throw Error(CTX_CONFLICT_PROPTYPE, Error::Type::CONTEXT);
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
                throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
            }
        } else {
            auto edgeClassIds = ((*txn.txnCtx.dbRelation).*func2)(*txn.txnBase, recordDescriptor.rid);
            auto edgeClassDescriptors = Generic::getMultipleClassDescriptor(txn, edgeClassIds, ClassType::EDGE);
            auto edgeClassInfos = Generic::getMultipleClassMapProperty(*txn.txnBase, edgeClassDescriptors);
            if (!validateAndResolveProperties(edgeClassInfos, conditionPropertyTypes)) {
                throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
            }
        }
        return getEdgeMultiCondition(txn, recordDescriptor, edgeClassIds, func1, conditions, conditionPropertyTypes);
    }

//*****************************************************************
//*  compare by a conditional function                            *
//*****************************************************************

    ResultSet Compare::getRecordCondition(const Txn &txn,
                                          const std::vector<ClassInfo> &classInfos,
                                          bool (*condition)(const Record &record)) {
        auto result = ResultSet{};
        try {
            for (const auto &classInfo: classInfos) {
                auto classDBHandler = Datastore::openDbi(txn.txnBase->getDsTxnHandler(), std::to_string(classInfo.id), true);
                auto cursorHandler = Datastore::CursorHandlerWrapper(txn.txnBase->getDsTxnHandler(), classDBHandler);
                auto keyValue = Datastore::getNextCursor(cursorHandler.get());
                while (!keyValue.empty()) {
                    auto key = Datastore::getKeyAsNumeric<PositionId>(keyValue);
                    if (*key != EM_MAXRECNUM) {
                        auto rid = RecordId{classInfo.id, *key};
                        auto record = Parser::parseRawDataWithBasicInfo(classInfo.name, rid, keyValue, classInfo.propertyInfo);
                        if ((*condition)(record)) {
                            result.push_back(Result{RecordDescriptor{classInfo.id, *key}, record});
                        }
                    }
                    keyValue = Datastore::getNextCursor(cursorHandler.get());
                }
            }
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
        return result;
    }

    ResultSet Compare::compareCondition(const Txn &txn,
                                        const std::string &className,
                                        ClassType type,
                                        bool (*condition)(const Record &)) {
        auto classDescriptors = Generic::getMultipleClassDescriptor(txn, std::set<std::string>{className}, type);
        auto classInfos = Generic::getMultipleClassMapProperty(*txn.txnBase, classDescriptors);
        return getRecordCondition(txn, classInfos, condition);
    }

    ResultSet Compare::getEdgeCondition(const Txn &txn,
                                        const RecordDescriptor &recordDescriptor,
                                        const std::vector<ClassId> &edgeClassIds,
                                        std::vector<RecordId>
                                        (Graph::*func)(const BaseTxn &baseTxn, const RecordId &rid, const ClassId &classId),
                                        bool (*condition)(const Record &record)) {
        switch (Generic::checkIfRecordExist(txn, recordDescriptor)) {
            case RECORD_NOT_EXIST:
                throw Error(GRAPH_NOEXST_VERTEX, Error::Type::GRAPH);
            case RECORD_NOT_EXIST_IN_MEMORY:
                return ResultSet{};
            default:
                auto result = ResultSet{};
                try {
                    auto classDescriptor = Schema::ClassDescriptorPtr{};
                    auto classPropertyInfo = ClassPropertyInfo{};
                    auto classDBHandler = Datastore::DBHandler{};
                    auto className = std::string{};
                    auto retrieve = [&](ResultSet &result, const RecordId &edge) {
                        if (classDescriptor == nullptr || classDescriptor->id != edge.first) {
                            classDescriptor = Generic::getClassDescriptor(txn, edge.first, ClassType::UNDEFINED);
                            classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
                            classDBHandler = Datastore::openDbi(txn.txnBase->getDsTxnHandler(), std::to_string(edge.first), true);
                            className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
                        }
                        auto keyValue = Datastore::getRecord(txn.txnBase->getDsTxnHandler(), classDBHandler, edge.second);
                        auto record = Parser::parseRawDataWithBasicInfo(className, edge, keyValue, classPropertyInfo);
                        if ((*condition)(record)) {
                            result.push_back(Result{RecordDescriptor{edge}, record});
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
                } catch (Graph::ErrorType &err) {
                    if (err == GRAPH_NOEXST_VERTEX) {
                        throw Error(GRAPH_UNKNOWN_ERR, Error::Type::GRAPH);
                    } else {
                        throw Error(err, Error::Type::GRAPH);
                    }
                } catch (Datastore::ErrorType &err) {
                    throw Error(err, Error::Type::DATASTORE);
                }
                return result;
        }
    }

    ResultSet Compare::compareEdgeCondition(const Txn &txn,
                                            const RecordDescriptor &recordDescriptor,
                                            std::vector<RecordId>
                                            (Graph::*func1)(const BaseTxn &baseTxn, const RecordId &rid, const ClassId &classId),
                                            std::vector<ClassId>
                                            (Graph::*func2)(const BaseTxn &baseTxn, const RecordId &rid),
                                            bool (*condition)(const Record &),
                                            const ClassFilter &classFilter) {
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
        return getEdgeCondition(txn, recordDescriptor, edgeClassIds, func1, condition);
    }

}
