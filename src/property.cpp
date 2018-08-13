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

#include <memory>

#include "shared_lock.hpp"
#include "constant.hpp"
#include "base_txn.hpp"
#include "lmdb_engine.hpp"
#include "validate.hpp"
#include "schema.hpp"
#include "index.hpp"
#include "generic.hpp"
#include "parser.hpp"

#include "nogdb.h"

namespace nogdb {

    const PropertyDescriptor Property::add(Txn &txn,
                                           const std::string &className,
                                           const std::string &propertyName,
                                           PropertyType type) {
        // transaction validations
        Validate::isTransactionValid(txn);
        // basic validations
        Validate::isPropertyNameValid(propertyName);
        Validate::isPropertyTypeValid(type);

        auto &dbInfo = txn._txnBase->dbInfo;
        if (dbInfo.maxPropertyId >= UINT16_MAX) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_LIMIT_DBSCHEMA);
        } else {
            ++dbInfo.maxPropertyId;
        }

        // schema validations
        auto foundClass = Validate::isExistingClass(txn, className);
        Validate::isNotDuplicatedProperty(*txn._txnBase, foundClass, propertyName);
        Validate::isNotOverridenProperty(*txn._txnBase, foundClass, propertyName);

        auto propertyDescriptor = Schema::PropertyDescriptor{dbInfo.maxPropertyId, type};
        auto dsTxnHandler = txn._txnBase->getDsTxnHandler();
        try {
            auto propDBHandler = dsTxnHandler->openDbi(TB_PROPERTIES, true);
            auto totalLength = sizeof(type) + sizeof(ClassId) + propertyName.length();
            auto value = Blob(totalLength);
            value.append(&type, sizeof(type));
            value.append(&foundClass->id, sizeof(ClassId));
            value.append(propertyName.c_str(), propertyName.length());
            propDBHandler.put(dbInfo.maxPropertyId, value);

            // update in-memory schema and info
            txn._txnCtx.dbSchema->addProperty(*txn._txnBase, foundClass->id, propertyName, propertyDescriptor);
            ++dbInfo.numProperty;
        } catch (const Error &err) {
            throw err;
        } catch (...) {
            // NOTE: too risky since this may cause undefined behaviour after throwing any exceptions
            // other than errors from datastore due to failures in updating in-memory schema or database info
            std::rethrow_exception(std::current_exception());
        }

        return propertyDescriptor.transform();
    }

    void Property::alter(Txn &txn,
                         const std::string &className,
                         const std::string &oldPropertyName,
                         const std::string &newPropertyName) {
        // transaction validations
        Validate::isTransactionValid(txn);
        // basic validation
        Validate::isPropertyNameValid(newPropertyName);

        // schema validations
        auto foundClass = Validate::isExistingClass(txn, className);
        auto foundOldProperty = Validate::isExistingProperty(*txn._txnBase, foundClass, oldPropertyName);
        Validate::isNotDuplicatedProperty(*txn._txnBase, foundClass, newPropertyName);
        Validate::isNotOverridenProperty(*txn._txnBase, foundClass, newPropertyName);

        auto dsTxnHandler = txn._txnBase->getDsTxnHandler();
        try {
            auto propDBHandler = dsTxnHandler->openDbi(TB_PROPERTIES, true);
            auto type = foundOldProperty.type;
            auto totalLength = sizeof(type) + sizeof(ClassId) + newPropertyName.length();
            auto value = Blob(totalLength);
            value.append(&type, sizeof(decltype(type)));
            value.append(&foundClass->id, sizeof(ClassId));
            value.append(newPropertyName.c_str(), newPropertyName.length());
            propDBHandler.put(foundOldProperty.id, value);

            // update in-memory schema
            txn._txnCtx.dbSchema->updateProperty(*txn._txnBase, foundClass->id, oldPropertyName, newPropertyName);
        } catch (const Error &err) {
            throw err;
        } catch (...) {
            // NOTE: too risky since this may cause undefined behaviour after throwing any exceptions
            // other than errors from datastore due to failures in updating in-memory schema or database info
            std::rethrow_exception(std::current_exception());
        }
    }

    void Property::remove(Txn &txn, const std::string &className, const std::string &propertyName) {
        // transaction validations
        Validate::isTransactionValid(txn);
        // schema validations
        auto foundClass = Validate::isExistingClass(txn, className);
        auto foundProperty = Validate::isExistingProperty(*txn._txnBase, foundClass, propertyName);

        // check if all index tables associated with the column have bee removed beforehand
        if (!foundProperty.indexInfo.empty()) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_IN_USED_PROPERTY);
        }

        auto &dbInfo = txn._txnBase->dbInfo;
        auto dsTxnHandler = txn._txnBase->getDsTxnHandler();
        try {
            auto propDBHandler = dsTxnHandler->openDbi(TB_PROPERTIES, true);
            propDBHandler.del(foundProperty.id);

            // update in-memory schema
            txn._txnCtx.dbSchema->deleteProperty(*txn._txnBase, foundClass->id, propertyName);
            // update in-memory database info
            --dbInfo.numProperty;
        } catch (const Error &err) {
            throw err;
        } catch (...) {
            // NOTE: too risky since this may cause undefined behaviour after throwing any exceptions
            // other than errors from datastore due to failures in updating in-memory schema or database info
            std::rethrow_exception(std::current_exception());
        }
    }

    void Property::createIndex(Txn &txn, const std::string &className, const std::string &propertyName, bool isUnique) {
        // transaction validations
        Validate::isTransactionValid(txn);

        auto &dbInfo = txn._txnBase->dbInfo;
        if (dbInfo.maxIndexId >= UINT32_MAX) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_LIMIT_DBSCHEMA);
        } else {
            ++dbInfo.maxIndexId;
        }

        // schema validations
        auto foundClass = Validate::isExistingClass(txn, className);
        auto result = Validate::isExistingPropertyExtend(*txn._txnBase, foundClass, propertyName);
        auto foundPropertyBasedClassId = result.first;
        auto foundProperty = result.second;

        // index validations
        if (foundProperty.type == PropertyType::BLOB || foundProperty.type == PropertyType::UNDEFINED) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_PROPTYPE_INDEX);
        }
        auto indexInfo = foundProperty.indexInfo.find(foundClass->id);
        if (indexInfo != foundProperty.indexInfo.cend()) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_INDEX);
        }

        auto dsTxnHandler = txn._txnBase->getDsTxnHandler();
        try {
            auto indexDBHandler = dsTxnHandler->openDbi(TB_INDEXES, true, false);
            auto totalLength = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(IndexId) + sizeof(ClassId);
            auto isCompositeNumeric = uint8_t{0}; //TODO: change it when a composite index is available
            auto isUniqueNumeric = (isUnique) ? uint8_t{1} : uint8_t{0};
            auto valueIndex = Blob(totalLength);
            valueIndex.append(&isCompositeNumeric, sizeof(isCompositeNumeric));
            valueIndex.append(&isUniqueNumeric, sizeof(isUniqueNumeric));
            valueIndex.append(&dbInfo.maxIndexId, sizeof(IndexId));
            valueIndex.append(&foundClass->id, sizeof(ClassId));
            indexDBHandler.put(foundProperty.id, valueIndex);
            switch (foundProperty.type) {
                case PropertyType::UNSIGNED_TINYINT:
                case PropertyType::UNSIGNED_SMALLINT:
                case PropertyType::UNSIGNED_INTEGER:
                case PropertyType::UNSIGNED_BIGINT: {
                    auto dataIndexDBHandler = dsTxnHandler->openDbi(Index::getIndexingName(dbInfo.maxIndexId), true, isUnique);
                    auto classPropertyInfo = Generic::getClassMapProperty(*txn._txnBase, foundClass);
                    auto cursorHandler = dsTxnHandler->openCursor(std::to_string(foundClass->id), true);
                    auto keyValue = cursorHandler.getNext();
                    while (!keyValue.empty()) {
                        auto key = keyValue.key.data.numeric<PositionId>();
                        if (key != MAX_RECORD_NUM_EM) {
                            auto const positionId = key;
                            auto const record = Parser::parseRawData(keyValue.val, classPropertyInfo);
                            auto bytesValue = record.get(propertyName);
                            if (!bytesValue.empty()) {
                                auto indexRecord = Blob(sizeof(PositionId));
                                indexRecord.append(&positionId, sizeof(PositionId));
                                if (foundProperty.type == PropertyType::UNSIGNED_TINYINT) {
                                    dataIndexDBHandler.put(static_cast<uint64_t>(bytesValue.toTinyIntU()), indexRecord, false, !isUnique);
                                } else if (foundProperty.type == PropertyType::UNSIGNED_SMALLINT) {
                                    dataIndexDBHandler.put(static_cast<uint64_t>(bytesValue.toSmallIntU()), indexRecord, false, !isUnique);
                                } else if (foundProperty.type == PropertyType::UNSIGNED_INTEGER) {
                                    dataIndexDBHandler.put(static_cast<uint64_t>(bytesValue.toIntU()), indexRecord, false, !isUnique);
                                } else {
                                    dataIndexDBHandler.put(bytesValue.toBigIntU(), indexRecord, false, !isUnique);
                                }
                            }
                        }
                        keyValue = cursorHandler.getNext();
                    }
                    break;
                }
                case PropertyType::TINYINT:
                case PropertyType::SMALLINT:
                case PropertyType::INTEGER:
                case PropertyType::BIGINT:
                case PropertyType::REAL: {
                    auto dataIndexDBHandlerPositive = dsTxnHandler->openDbi(Index::getIndexingName(dbInfo.maxIndexId, true), true, isUnique);
                    auto dataIndexDBHandlerNegative = dsTxnHandler->openDbi(Index::getIndexingName(dbInfo.maxIndexId, false), true, isUnique);
                    auto classPropertyInfo = Generic::getClassMapProperty(*txn._txnBase, foundClass);
                    auto cursorHandler = dsTxnHandler->openCursor(std::to_string(foundClass->id), true);
                    auto keyValue = cursorHandler.getNext();
                    while (!keyValue.empty()) {
                        auto key = keyValue.key.data.numeric<PositionId>();
                        if (key != MAX_RECORD_NUM_EM) {
                            auto const positionId = key;
                            auto const record = Parser::parseRawData(keyValue.val, classPropertyInfo);
                            auto bytesValue = record.get(propertyName);
                            if (!bytesValue.empty()) {
                                auto indexRecord = Blob(sizeof(PositionId));
                                indexRecord.append(&positionId, sizeof(PositionId));
                                if (foundProperty.type == PropertyType::TINYINT) {
                                    auto value = static_cast<int64_t>(bytesValue.toTinyInt());
                                    if (value >= 0) {
                                        dataIndexDBHandlerPositive.put(value, indexRecord, false, !isUnique);
                                    } else {
                                        dataIndexDBHandlerNegative.put(value, indexRecord, false, !isUnique);
                                    }
                                } else if (foundProperty.type == PropertyType::SMALLINT) {
                                    auto value = static_cast<int64_t>(bytesValue.toSmallInt());
                                    if (value >= 0) {
                                        dataIndexDBHandlerPositive.put(value, indexRecord, false, !isUnique);
                                    } else {
                                        dataIndexDBHandlerNegative.put(value, indexRecord, false, !isUnique);
                                    }
                                } else if (foundProperty.type == PropertyType::INTEGER) {
                                    auto value = static_cast<int64_t>(bytesValue.toInt());
                                    if (value >= 0) {
                                        dataIndexDBHandlerPositive.put(value, indexRecord, false, !isUnique);
                                    } else {
                                        dataIndexDBHandlerNegative.put(value, indexRecord, false, !isUnique);
                                    }
                                } else if (foundProperty.type == PropertyType::REAL) {
                                    auto value = bytesValue.toReal();
                                    if (value >= 0) {
                                        dataIndexDBHandlerPositive.put(value, indexRecord, false, !isUnique);
                                    } else {
                                        dataIndexDBHandlerNegative.put(value, indexRecord, false, !isUnique);
                                    }
                                } else {
                                    auto value = bytesValue.toBigInt();
                                    if (value >= 0) {
                                        dataIndexDBHandlerPositive.put(value, indexRecord, false, !isUnique);
                                    } else {
                                        dataIndexDBHandlerNegative.put(value, indexRecord, false, !isUnique);
                                    }
                                }
                            }
                        }
                        keyValue = cursorHandler.getNext();
                    }
                    break;
                }
                case PropertyType::TEXT: {
                    auto dataIndexDBHandler = dsTxnHandler->openDbi(Index::getIndexingName(dbInfo.maxIndexId), false, isUnique);
                    auto classPropertyInfo = Generic::getClassMapProperty(*txn._txnBase, foundClass);
                    auto cursorHandler = dsTxnHandler->openCursor(std::to_string(foundClass->id), true);
                    auto keyValue = cursorHandler.getNext();
                    while (!keyValue.empty()) {
                        auto key = keyValue.key.data.numeric<PositionId>();
                        if (key != MAX_RECORD_NUM_EM) {
                            auto const positionId = key;
                            auto const record = Parser::parseRawData(keyValue.val, classPropertyInfo);
                            auto value = record.get(propertyName).toText();
                            if (!value.empty()) {
                                auto indexRecord = Blob(sizeof(PositionId));
                                indexRecord.append(&positionId, sizeof(PositionId));
                                dataIndexDBHandler.put(value, indexRecord, false, !isUnique);
                            }
                        }
                        keyValue = cursorHandler.getNext();
                    }
                    break;
                }
                default:
                    break;
            }

            // update in-memory database schema and info
            foundProperty.indexInfo.emplace(foundClass->id, std::make_pair(dbInfo.maxIndexId, isUnique));
            txn._txnCtx.dbSchema->updateProperty(*txn._txnBase, foundPropertyBasedClassId, propertyName, foundProperty);
            ++dbInfo.numIndex;
        } catch (const Error &err) {
            if (err.code() == MDB_KEYEXIST) {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_INDEX_CONSTRAINT);
            } else {
                throw err;
            }
        } catch (...) {
            // NOTE: too risky since this may cause undefined behaviour after throwing any exceptions
            // other than errors from datastore due to failures in updating in-memory schema or database info
            std::rethrow_exception(std::current_exception());
        }
    }

    void Property::dropIndex(Txn &txn, const std::string &className, const std::string &propertyName) {
        // transaction validations
        Validate::isTransactionValid(txn);

        // schema validations
        auto foundClass = Validate::isExistingClass(txn, className);
        auto result = Validate::isExistingPropertyExtend(*txn._txnBase, foundClass, propertyName);
        auto foundPropertyBasedClassId = result.first;
        auto foundProperty = result.second;

        // index validations
        auto indexInfo = foundProperty.indexInfo.find(foundClass->id);
        if (indexInfo == foundProperty.indexInfo.cend()) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_INDEX);
        }

        auto &dbInfo = txn._txnBase->dbInfo;
        auto dsTxnHandler = txn._txnBase->getDsTxnHandler();
        try {
            auto indexDBHandler = dsTxnHandler->openDbi(TB_INDEXES, true, false);
            auto totalLength = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(IndexId) + sizeof(ClassId);
            auto isCompositeNumeric = uint8_t{0}; //TODO: change it when a composite index is available
            auto isUnique = indexInfo->second.second;
            auto isUniqueNumeric = (isUnique) ? uint8_t{1} : uint8_t{0};
            auto indexId = indexInfo->second.first;
            auto value = Blob(totalLength);
            value.append(&isCompositeNumeric, sizeof(isCompositeNumeric));
            value.append(&isUniqueNumeric, sizeof(isUniqueNumeric));
            value.append(&indexId, sizeof(IndexId));
            value.append(&foundClass->id, sizeof(ClassId));
            // delete metadata from index mapping table
            indexDBHandler.del(foundProperty.id, value);
            // drop the actual index data table
            switch (foundProperty.type) {
                case PropertyType::UNSIGNED_TINYINT:
                case PropertyType::UNSIGNED_SMALLINT:
                case PropertyType::UNSIGNED_INTEGER:
                case PropertyType::UNSIGNED_BIGINT: {
                    auto dataIndexDBHandler = dsTxnHandler->openDbi(Index::getIndexingName(indexId), true, isUnique);
                    dataIndexDBHandler.drop(true);
                    break;
                }
                case PropertyType::TINYINT:
                case PropertyType::SMALLINT:
                case PropertyType::INTEGER:
                case PropertyType::BIGINT:
                case PropertyType::REAL: {
                    auto dataIndexDBHandlerPositive = dsTxnHandler->openDbi(Index::getIndexingName(indexId, true), true, isUnique);
                    auto dataIndexDBHandlerNegative = dsTxnHandler->openDbi(Index::getIndexingName(indexId, false), true, isUnique);
                    dataIndexDBHandlerPositive.drop(true);
                    dataIndexDBHandlerNegative.drop(true);
                    break;
                }
                case PropertyType::TEXT: {
                    auto dataIndexDBHandler = dsTxnHandler->openDbi(Index::getIndexingName(indexId), false, isUnique);
                    dataIndexDBHandler.drop(true);
                    break;
                }
                default:
                    break;
            }

            // update in-memory schema
            foundProperty.indexInfo.erase(foundClass->id);
            txn._txnCtx.dbSchema->updateProperty(*txn._txnBase, foundPropertyBasedClassId, propertyName, foundProperty);
            // update in-memory database info
            --dbInfo.numIndex;
        } catch (const Error &err) {
            throw err;
        } catch (...) {
            // NOTE: too risky since this may cause undefined behaviour after throwing any exceptions
            // other than errors from datastore due to failures in updating in-memory schema or database info
            std::rethrow_exception(std::current_exception());
        }
    }

}
