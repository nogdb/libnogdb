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

#include <memory>

#include "shared_lock.hpp"
#include "constant.hpp"
#include "base_txn.hpp"
#include "env_handler.hpp"
#include "lmdb_interface.hpp"
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

        auto &dbInfo = txn.txnBase->dbInfo;
        if (dbInfo.maxPropertyId >= UINT16_MAX) {
            throw Error(NOGDB_CTX_LIMIT_DBSCHEMA, Error::Type::CONTEXT);
        } else {
            ++dbInfo.maxPropertyId;
        }

        // schema validations
        auto foundClass = Validate::isExistingClass(txn, className);
        Validate::isNotDuplicatedProperty(*txn.txnBase, foundClass, propertyName);
        Validate::isNotOverridenProperty(*txn.txnBase, foundClass, propertyName);

        auto propertyDescriptor = Schema::PropertyDescriptor{dbInfo.maxPropertyId, type};
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            auto propDBHandler = LMDBInterface::openDbi(dsTxnHandler, TB_PROPERTIES, true);
            auto totalLength = sizeof(type) + sizeof(ClassId) + propertyName.length();
            auto value = Blob(totalLength);
            value.append(&type, sizeof(type));
            value.append(&foundClass->id, sizeof(ClassId));
            value.append(propertyName.c_str(), propertyName.length());
            LMDBInterface::putRecord(dsTxnHandler, propDBHandler, dbInfo.maxPropertyId, value);

            // update in-memory schema and info
            txn.txnCtx.dbSchema->addProperty(*txn.txnBase, foundClass->id, propertyName, propertyDescriptor);
            ++dbInfo.numProperty;
        } catch (LMDBInterface::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
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
        auto foundOldProperty = Validate::isExistingProperty(*txn.txnBase, foundClass, oldPropertyName);
        Validate::isNotDuplicatedProperty(*txn.txnBase, foundClass, newPropertyName);
        Validate::isNotOverridenProperty(*txn.txnBase, foundClass, newPropertyName);

        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            auto propDBHandler = LMDBInterface::openDbi(dsTxnHandler, TB_PROPERTIES, true);
            auto type = foundOldProperty.type;
            auto totalLength = sizeof(type) + sizeof(ClassId) + newPropertyName.length();
            auto value = Blob(totalLength);
            value.append(&type, sizeof(decltype(type)));
            value.append(&foundClass->id, sizeof(ClassId));
            value.append(newPropertyName.c_str(), newPropertyName.length());
            LMDBInterface::putRecord(dsTxnHandler, propDBHandler, foundOldProperty.id, value);

            // update in-memory schema
            txn.txnCtx.dbSchema->updateProperty(*txn.txnBase, foundClass->id, oldPropertyName, newPropertyName);
        } catch (LMDBInterface::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
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
        auto foundProperty = Validate::isExistingProperty(*txn.txnBase, foundClass, propertyName);

        // check if all index tables associated with the column have bee removed beforehand
        if (!foundProperty.indexInfo.empty()) {
            throw Error(NOGDB_CTX_IN_USED_PROPERTY, Error::Type::CONTEXT);
        }

        auto &dbInfo = txn.txnBase->dbInfo;
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            auto propDBHandler = LMDBInterface::openDbi(dsTxnHandler, TB_PROPERTIES, true);
            LMDBInterface::deleteRecord(dsTxnHandler, propDBHandler, foundProperty.id);

            // update in-memory schema
            txn.txnCtx.dbSchema->deleteProperty(*txn.txnBase, foundClass->id, propertyName);
            // update in-memory database info
            --dbInfo.numProperty;
        } catch (LMDBInterface::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        } catch (...) {
            // NOTE: too risky since this may cause undefined behaviour after throwing any exceptions
            // other than errors from datastore due to failures in updating in-memory schema or database info
            std::rethrow_exception(std::current_exception());
        }
    }

    void Property::createIndex(Txn &txn, const std::string &className, const std::string &propertyName, bool isUnique) {
        // transaction validations
        Validate::isTransactionValid(txn);

        auto &dbInfo = txn.txnBase->dbInfo;
        if (dbInfo.maxIndexId >= UINT32_MAX) {
            throw Error(NOGDB_CTX_LIMIT_DBSCHEMA, Error::Type::CONTEXT);
        } else {
            ++dbInfo.maxIndexId;
        }

        // schema validations
        auto foundClass = Validate::isExistingClass(txn, className);
        auto result = Validate::isExistingPropertyExtend(*txn.txnBase, foundClass, propertyName);
        auto foundPropertyBasedClassId = result.first;
        auto foundProperty = result.second;

        // index validations
        if (foundProperty.type == PropertyType::BLOB || foundProperty.type == PropertyType::UNDEFINED) {
            throw Error(NOGDB_CTX_INVALID_PROPTYPE_INDEX, Error::Type::CONTEXT);
        }
        auto indexInfo = foundProperty.indexInfo.find(foundClass->id);
        if (indexInfo != foundProperty.indexInfo.cend()) {
            throw Error(NOGDB_CTX_DUPLICATE_INDEX, Error::Type::CONTEXT);
        }

        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            auto indexDBHandler = LMDBInterface::openDbi(dsTxnHandler, TB_INDEXES, true, false);
            auto totalLength = sizeof(uint8_t) + sizeof(uint8_t) + sizeof(IndexId) + sizeof(ClassId);
            auto isCompositeNumeric = uint8_t{0}; //TODO: change it when a composite index is available
            auto isUniqueNumeric = (isUnique) ? uint8_t{1} : uint8_t{0};
            auto valueIndex = Blob(totalLength);
            valueIndex.append(&isCompositeNumeric, sizeof(isCompositeNumeric));
            valueIndex.append(&isUniqueNumeric, sizeof(isUniqueNumeric));
            valueIndex.append(&dbInfo.maxIndexId, sizeof(IndexId));
            valueIndex.append(&foundClass->id, sizeof(ClassId));
            LMDBInterface::putRecord(dsTxnHandler, indexDBHandler, foundProperty.id, valueIndex);
            switch (foundProperty.type) {
                case PropertyType::UNSIGNED_TINYINT:
                case PropertyType::UNSIGNED_SMALLINT:
                case PropertyType::UNSIGNED_INTEGER:
                case PropertyType::UNSIGNED_BIGINT: {
                    auto dataIndexDBHandler = LMDBInterface::openDbi(dsTxnHandler, Index::getIndexingName(dbInfo.maxIndexId), true, isUnique);
                    auto classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, foundClass);
                    auto classDBHandler = LMDBInterface::openDbi(dsTxnHandler, std::to_string(foundClass->id), true);
                    auto cursorHandler = LMDBInterface::CursorHandlerWrapper(dsTxnHandler, classDBHandler);
                    auto keyValue = LMDBInterface::getNextCursor(cursorHandler.get());
                    while (!keyValue.empty()) {
                        auto key = LMDBInterface::getKeyAsNumeric<PositionId>(keyValue);
                        if (*key != EM_MAXRECNUM) {
                            auto const positionId = *key;
                            auto const record = Parser::parseRawData(keyValue, classPropertyInfo);
                            auto bytesValue = record.get(propertyName);
                            if (!bytesValue.empty()) {
                                auto indexRecord = Blob(sizeof(PositionId));
                                indexRecord.append(&positionId, sizeof(PositionId));
                                if (foundProperty.type == PropertyType::UNSIGNED_TINYINT) {
                                    LMDBInterface::putRecord(dsTxnHandler, dataIndexDBHandler,
                                                         static_cast<uint64_t>(bytesValue.toTinyIntU()),
                                                         indexRecord, false, !isUnique);
                                } else if (foundProperty.type == PropertyType::UNSIGNED_SMALLINT) {
                                    LMDBInterface::putRecord(dsTxnHandler, dataIndexDBHandler,
                                                         static_cast<uint64_t>(bytesValue.toSmallIntU()),
                                                         indexRecord, false, !isUnique);
                                } else if (foundProperty.type == PropertyType::UNSIGNED_INTEGER) {
                                    LMDBInterface::putRecord(dsTxnHandler, dataIndexDBHandler,
                                                         static_cast<uint64_t>(bytesValue.toIntU()),
                                                         indexRecord, false, !isUnique);
                                } else {
                                    LMDBInterface::putRecord(dsTxnHandler, dataIndexDBHandler, bytesValue.toBigIntU(),
                                                         indexRecord, false, !isUnique);
                                }
                            }
                        }
                        keyValue = LMDBInterface::getNextCursor(cursorHandler.get());
                    }
                    break;
                }
                case PropertyType::TINYINT:
                case PropertyType::SMALLINT:
                case PropertyType::INTEGER:
                case PropertyType::BIGINT:
                case PropertyType::REAL: {
                    auto dataIndexDBHandlerPositive = LMDBInterface::openDbi(dsTxnHandler, Index::getIndexingName(dbInfo.maxIndexId, true), true, isUnique);
                    auto dataIndexDBHandlerNegative = LMDBInterface::openDbi(dsTxnHandler, Index::getIndexingName(dbInfo.maxIndexId, false), true, isUnique);
                    auto classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, foundClass);
                    auto classDBHandler = LMDBInterface::openDbi(dsTxnHandler, std::to_string(foundClass->id), true);
                    auto cursorHandler = LMDBInterface::CursorHandlerWrapper(dsTxnHandler, classDBHandler);
                    auto keyValue = LMDBInterface::getNextCursor(cursorHandler.get());
                    while (!keyValue.empty()) {
                        auto key = LMDBInterface::getKeyAsNumeric<PositionId>(keyValue);
                        if (*key != EM_MAXRECNUM) {
                            auto const positionId = *key;
                            auto const record = Parser::parseRawData(keyValue, classPropertyInfo);
                            auto bytesValue = record.get(propertyName);
                            if (!bytesValue.empty()) {
                                auto indexRecord = Blob(sizeof(PositionId));
                                indexRecord.append(&positionId, sizeof(PositionId));
                                if (foundProperty.type == PropertyType::TINYINT) {
                                    auto value = static_cast<int64_t>(bytesValue.toTinyInt());
                                    LMDBInterface::putRecord(dsTxnHandler, (value >= 0) ? dataIndexDBHandlerPositive
                                                                                    : dataIndexDBHandlerNegative,
                                                         value, indexRecord, false, !isUnique);
                                } else if (foundProperty.type == PropertyType::SMALLINT) {
                                    auto value = static_cast<int64_t>(bytesValue.toSmallInt());
                                    LMDBInterface::putRecord(dsTxnHandler, (value >= 0) ? dataIndexDBHandlerPositive
                                                                                    : dataIndexDBHandlerNegative,
                                                         value, indexRecord, false, !isUnique);
                                } else if (foundProperty.type == PropertyType::INTEGER) {
                                    auto value = static_cast<int64_t>(bytesValue.toInt());
                                    LMDBInterface::putRecord(dsTxnHandler, (value >= 0) ? dataIndexDBHandlerPositive
                                                                                    : dataIndexDBHandlerNegative,
                                                         value, indexRecord, false, !isUnique);
                                } else if (foundProperty.type == PropertyType::REAL) {
                                    auto value = bytesValue.toReal();
                                    LMDBInterface::putRecord(dsTxnHandler, (value >= 0) ? dataIndexDBHandlerPositive
                                                                                    : dataIndexDBHandlerNegative,
                                                         value, indexRecord, false, !isUnique);
                                } else {
                                    auto value = bytesValue.toBigInt();
                                    LMDBInterface::putRecord(dsTxnHandler, (value >= 0) ? dataIndexDBHandlerPositive
                                                                                    : dataIndexDBHandlerNegative,
                                                         value, indexRecord, false, !isUnique);
                                }
                            }
                        }
                        keyValue = LMDBInterface::getNextCursor(cursorHandler.get());
                    }
                    break;
                }
                case PropertyType::TEXT: {
                    auto dataIndexDBHandler = LMDBInterface::openDbi(dsTxnHandler, Index::getIndexingName(dbInfo.maxIndexId), false, isUnique);
                    auto classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, foundClass);
                    auto classDBHandler = LMDBInterface::openDbi(dsTxnHandler, std::to_string(foundClass->id), true);
                    auto cursorHandler = LMDBInterface::CursorHandlerWrapper(dsTxnHandler, classDBHandler);
                    auto keyValue = LMDBInterface::getNextCursor(cursorHandler.get());
                    while (!keyValue.empty()) {
                        auto key = LMDBInterface::getKeyAsNumeric<PositionId>(keyValue);
                        if (*key != EM_MAXRECNUM) {
                            auto const positionId = *key;
                            auto const record = Parser::parseRawData(keyValue, classPropertyInfo);
                            auto value = record.get(propertyName).toText();
                            if (!value.empty()) {
                                auto indexRecord = Blob(sizeof(PositionId));
                                indexRecord.append(&positionId, sizeof(PositionId));
                                LMDBInterface::putRecord(dsTxnHandler, dataIndexDBHandler, value, indexRecord, false, !isUnique);
                            }
                        }
                        keyValue = LMDBInterface::getNextCursor(cursorHandler.get());
                    }
                    break;
                }
                default:
                    break;
            }

            // update in-memory database schema and info
            foundProperty.indexInfo.emplace(foundClass->id, std::make_pair(dbInfo.maxIndexId, isUnique));
            txn.txnCtx.dbSchema->updateProperty(*txn.txnBase, foundPropertyBasedClassId, propertyName, foundProperty);
            ++dbInfo.numIndex;
        } catch (LMDBInterface::ErrorType &err) {
            if (err == MDB_KEYEXIST) {
                throw Error(NOGDB_CTX_INVALID_INDEX_CONSTRAINT, Error::Type::CONTEXT);
            } else {
                throw Error(err, Error::Type::DATASTORE);
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
        auto result = Validate::isExistingPropertyExtend(*txn.txnBase, foundClass, propertyName);
        auto foundPropertyBasedClassId = result.first;
        auto foundProperty = result.second;

        // index validations
        auto indexInfo = foundProperty.indexInfo.find(foundClass->id);
        if (indexInfo == foundProperty.indexInfo.cend()) {
            throw Error(NOGDB_CTX_NOEXST_INDEX, Error::Type::CONTEXT);
        }

        auto &dbInfo = txn.txnBase->dbInfo;
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            auto indexDBHandler = LMDBInterface::openDbi(dsTxnHandler, TB_INDEXES, true, false);
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
            LMDBInterface::deleteRecord(dsTxnHandler, indexDBHandler, foundProperty.id, value);
            // drop the actual index data table
            switch (foundProperty.type) {
                case PropertyType::UNSIGNED_TINYINT:
                case PropertyType::UNSIGNED_SMALLINT:
                case PropertyType::UNSIGNED_INTEGER:
                case PropertyType::UNSIGNED_BIGINT: {
                    auto dataIndexDBHandler = LMDBInterface::openDbi(dsTxnHandler, Index::getIndexingName(indexId), true, isUnique);
                    LMDBInterface::dropDbi(dsTxnHandler, dataIndexDBHandler);
                    break;
                }
                case PropertyType::TINYINT:
                case PropertyType::SMALLINT:
                case PropertyType::INTEGER:
                case PropertyType::BIGINT:
                case PropertyType::REAL: {
                    auto dataIndexDBHandlerPositive = LMDBInterface::openDbi(dsTxnHandler, Index::getIndexingName(indexId, true), true, isUnique);
                    auto dataIndexDBHandlerNegative = LMDBInterface::openDbi(dsTxnHandler, Index::getIndexingName(indexId, false), true, isUnique);
                    LMDBInterface::dropDbi(dsTxnHandler, dataIndexDBHandlerPositive);
                    LMDBInterface::dropDbi(dsTxnHandler, dataIndexDBHandlerNegative);
                    break;
                }
                case PropertyType::TEXT: {
                    auto dataIndexDBHandler = LMDBInterface::openDbi(dsTxnHandler, Index::getIndexingName(indexId), false, isUnique);
                    LMDBInterface::dropDbi(dsTxnHandler, dataIndexDBHandler);
                    break;
                }
                default:
                    break;
            }

            // update in-memory schema
            foundProperty.indexInfo.erase(foundClass->id);
            txn.txnCtx.dbSchema->updateProperty(*txn.txnBase, foundPropertyBasedClassId, propertyName, foundProperty);
            // update in-memory database info
            --dbInfo.numIndex;
        } catch (LMDBInterface::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        } catch (...) {
            // NOTE: too risky since this may cause undefined behaviour after throwing any exceptions
            // other than errors from datastore due to failures in updating in-memory schema or database info
            std::rethrow_exception(std::current_exception());
        }
    }

}
