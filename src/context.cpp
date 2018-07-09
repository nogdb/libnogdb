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
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sys/file.h>
#include <sys/stat.h>

#include "shared_lock.hpp"
#include "utils.hpp"
#include "constant.hpp"
#include "spinlock.hpp"
#include "base_txn.hpp"
#include "env_handler.hpp"
#include "lmdb_engine.hpp"
#include "graph.hpp"
#include "validate.hpp"
#include "schema.hpp"

#include "nogdb_context.h"

namespace nogdb {

    Context::Context(const std::string &dbPath)
            : Context{dbPath, MAX_DB_NUM, MAX_DB_SIZE} {};

    Context::Context(const std::string &dbPath, unsigned int maxDbNum)
            : Context{dbPath, maxDbNum, MAX_DB_SIZE} {};

    Context::Context(const std::string &dbPath, unsigned long maxDbSize)
            : Context{dbPath, MAX_DB_NUM, maxDbSize} {};

    Context::Context(const std::string &dbPath, unsigned int maxDbNum, unsigned long maxDbSize) {
        dbInfo = std::make_shared<DBInfo>();
        dbSchema = std::make_shared<Schema>();
        dbTxnStat = std::make_shared<TxnStat>();
        dbRelation = std::make_shared<Graph>();
        dbInfoMutex = std::make_shared<boost::shared_mutex>();
        dbWriterMutex = std::make_shared<boost::shared_mutex>();
        dbInfo->dbPath = dbPath;
        dbInfo->maxDB = maxDbNum;
        dbInfo->maxDBSize = maxDbSize;
        dbInfo->maxClassId = ClassId{INIT_NUM_CLASSES};
        dbInfo->maxPropertyId = PropertyId{INIT_NUM_PROPERTIES};
        dbInfo->numClass = ClassId{0};
        dbInfo->numProperty = PropertyId{0};
        envHandler = std::make_shared<EnvHandlerPtr>(
                EnvHandler::create(dbInfo->dbPath, dbInfo->maxDB, dbInfo->maxDBSize, LMDBInterface::MAX_READERS,
                                   LMDBInterface::FLAG, LMDBInterface::PERMISSION));
        initDatabase();
    }

    Context::Context(const Context &ctx)
            : envHandler{ctx.envHandler}, dbInfo{ctx.dbInfo}, dbSchema{ctx.dbSchema}, dbTxnStat{ctx.dbTxnStat},
              dbRelation{ctx.dbRelation}, dbInfoMutex{ctx.dbInfoMutex}, dbWriterMutex{ctx.dbWriterMutex} {};

    Context &Context::operator=(const Context &ctx) {
        if (this != &ctx) {
            auto tmp(ctx);
            using std::swap;
            swap(tmp, *this);
        }
        return *this;
    }

    Context::Context(Context &&ctx) noexcept
            : envHandler{std::move(ctx.envHandler)}, dbInfo{std::move(ctx.dbInfo)}, dbSchema{std::move(ctx.dbSchema)},
              dbTxnStat{std::move(ctx.dbTxnStat)}, dbRelation{std::move(ctx.dbRelation)},
              dbInfoMutex{std::move(ctx.dbInfoMutex)}, dbWriterMutex{std::move(ctx.dbWriterMutex)} {}

    Context &Context::operator=(Context &&ctx) noexcept {
        if (this != &ctx) {
            envHandler = std::move(ctx.envHandler);
            dbInfo = std::move(ctx.dbInfo);
            dbSchema = std::move(ctx.dbSchema);
            dbTxnStat = std::move(ctx.dbTxnStat);
            dbRelation = std::move(ctx.dbRelation);
            dbInfoMutex = std::move(ctx.dbInfoMutex);
            dbWriterMutex = std::move(ctx.dbWriterMutex);
        }
        return *this;
    }

    TxnId Context::getMaxVersionId() const {
        return dbTxnStat->maxVersionId;
    }

    TxnId Context::getMaxTxnId() const {
        return dbTxnStat->maxTxnId;
    }

    std::pair<TxnId, TxnId> Context::getMinActiveTxnId() const {
        return dbTxnStat->minActiveTxnId();
    }

    void Context::initDatabase() {
        auto currentTime = std::to_string(currentTimestamp());
        LMDBInterface::TxnHandler *txn = nullptr;
        // create read-write transaction
        try {
            txn = LMDBInterface::beginTxn(envHandler->get(), LMDBInterface::TXN_RW);
        } catch (LMDBInterface::ErrorType &err) {
            LMDBInterface::abortTxn(txn);
            throw Error(err, Error::Type::DATASTORE);
        }
        // prepare schema for classes, properties, and relations
        auto classDBHandler = LMDBInterface::DBHandler{};
        auto propDBHndler = LMDBInterface::DBHandler{};
        auto indexDBHandler = LMDBInterface::DBHandler{};
        auto relationDBHandler = LMDBInterface::DBHandler{};
        try {
            classDBHandler = LMDBInterface::openDbi(txn, TB_CLASSES, true);
            propDBHndler = LMDBInterface::openDbi(txn, TB_PROPERTIES, true);
            indexDBHandler = LMDBInterface::openDbi(txn, TB_INDEXES, true, false);
            relationDBHandler = LMDBInterface::openDbi(txn, TB_RELATIONS);
            LMDBInterface::putRecord(txn, classDBHandler, ClassId{UINT16_EM_INIT}, currentTime);
            LMDBInterface::putRecord(txn, propDBHndler, PropertyId{UINT16_EM_INIT}, currentTime);
            LMDBInterface::putRecord(txn, indexDBHandler, PropertyId{UINT16_EM_INIT}, currentTime);
            LMDBInterface::putRecord(txn, relationDBHandler, STRING_EM_INIT, currentTime);
            LMDBInterface::commitTxn(txn);
        } catch (LMDBInterface::ErrorType &err) {
            LMDBInterface::abortTxn(txn);
            throw Error(err, Error::Type::DATASTORE);
        }
        // create read-only transaction
        try {
            txn = LMDBInterface::beginTxn(envHandler->get(), LMDBInterface::TXN_RO);
        } catch (LMDBInterface::ErrorType &err) {
            LMDBInterface::abortTxn(txn);
            throw Error(err, Error::Type::DATASTORE);
        }
        // create read-write in memory transaction
        BaseTxn baseTxn{*this, true, true};
        // retrieve classes information
        try {
            auto inheritanceInfo = Schema::InheritanceInfo{};
            auto classCursor = LMDBInterface::CursorHandlerWrapper(txn, classDBHandler);
            for (auto classKeyValue = LMDBInterface::getNextCursor(classCursor.get());
                 !classKeyValue.empty();
                 classKeyValue = LMDBInterface::getNextCursor(classCursor.get())) {
                auto key = LMDBInterface::getKeyAsNumeric<ClassId>(classKeyValue);
                if (*key == ClassId{UINT16_EM_INIT}) {
                    continue;
                }
                auto data = LMDBInterface::getValueAsBlob(classKeyValue);
                auto classType = ClassType::UNDEFINED;
                auto offset = data.retrieve(&classType, 0, sizeof(ClassType));
                auto superClassId = ClassId{0};
                offset = data.retrieve(&superClassId, offset, sizeof(superClassId));
                auto nameLength = data.size() - offset;
                require(nameLength > 0);
                Blob::Byte nameBytes[nameLength];
                data.retrieve(nameBytes, offset, nameLength);
                auto className = std::string(reinterpret_cast<char *>(nameBytes), nameLength);
                auto classDescriptor = std::make_shared<Schema::ClassDescriptor>(ClassId{*key}, className, classType);
                dbSchema->insert(baseTxn, classDescriptor);
                inheritanceInfo.emplace_back(std::make_pair(classDescriptor->id, superClassId));
                if (classDescriptor->id > dbInfo->maxClassId) {
                    baseTxn.dbInfo.maxClassId = classDescriptor->id;
                }
                ++baseTxn.dbInfo.numClass;
            }
            dbSchema->apply(baseTxn, inheritanceInfo);
        } catch (LMDBInterface::ErrorType &err) {
            baseTxn.rollback(*this);
            dbSchema->clear();
            LMDBInterface::abortTxn(txn);
            throw Error(err, Error::Type::DATASTORE);
        }

        // retrieve properties and indexing information
        try {
            auto propCursor = LMDBInterface::CursorHandlerWrapper(txn, propDBHndler);
            auto indexCursor = LMDBInterface::CursorHandlerWrapper(txn, indexDBHandler);
            for (auto propKeyValue = LMDBInterface::getNextCursor(propCursor.get());
                 !propKeyValue.empty();
                 propKeyValue = LMDBInterface::getNextCursor(propCursor.get())) {
                auto key = LMDBInterface::getKeyAsNumeric<PropertyId>(propKeyValue);
                if (*key == PropertyId{UINT16_EM_INIT}) {
                    continue;
                }
                auto data = LMDBInterface::getValueAsBlob(propKeyValue);
                auto propType = PropertyType::UNDEFINED;
                auto propClassId = PropertyId{0};
                auto offset = data.retrieve(&propType, 0, sizeof(PropertyDescriptor::type));
                offset = data.retrieve(&propClassId, offset, sizeof(propClassId));
                auto nameLength = data.size() - offset;
                require(nameLength > 0);
                Blob::Byte nameBytes[nameLength];
                data.retrieve(nameBytes, offset, nameLength);
                auto propertyName = std::string(reinterpret_cast<char *>(nameBytes), nameLength);
                auto propertyDescriptor = Schema::PropertyDescriptor{PropertyId{*key}, propType};
                auto ptrClassDescriptor = dbSchema->find(baseTxn, propClassId);
                require(ptrClassDescriptor != nullptr);
                // get indexes associated with property
                auto isCompositeNumeric = uint8_t{0};
                auto isUniqueNumeric = uint8_t{1};
                auto indexId = IndexId{0};
                auto classId = ClassId{0};
                for (auto indexKeyValue = LMDBInterface::getSetKeyCursor(indexCursor.get(), propertyDescriptor.id);
                     !indexKeyValue.empty();
                     indexKeyValue = LMDBInterface::getNextDupCursor(indexCursor.get())) {
                    data = LMDBInterface::getValueAsBlob(indexKeyValue);
                    offset = data.retrieve(&isCompositeNumeric, 0, sizeof(isCompositeNumeric));
                    offset = data.retrieve(&isUniqueNumeric, offset, sizeof(isUniqueNumeric));
                    offset = data.retrieve(&indexId, offset, sizeof(IndexId));
                    offset = data.retrieve(&classId, offset, sizeof(ClassId));
                    propertyDescriptor.indexInfo.emplace(classId, std::make_pair(indexId, isUniqueNumeric));
                    if (indexId > baseTxn.dbInfo.maxIndexId) {
                        baseTxn.dbInfo.maxIndexId = indexId;
                    }
                    ++baseTxn.dbInfo.numIndex;
                }
                // insert property into class descriptor
                auto properties = ptrClassDescriptor->properties.getLatestVersion().first;
                properties.emplace(propertyName, propertyDescriptor);
                ptrClassDescriptor->properties.addLatestVersion(properties);
                if (propertyDescriptor.id > baseTxn.dbInfo.maxPropertyId) {
                    baseTxn.dbInfo.maxPropertyId = propertyDescriptor.id;
                }
                ++baseTxn.dbInfo.numProperty;
            }
        } catch (LMDBInterface::ErrorType &err) {
            baseTxn.rollback(*this);
            dbSchema->clear();
            LMDBInterface::abortTxn(txn);
            throw Error(err, Error::Type::DATASTORE);
        }

        // retrieve relations information
        try {
            auto relationCursor = LMDBInterface::CursorHandlerWrapper(txn, relationDBHandler);
            for (auto relationKeyValue = LMDBInterface::getNextCursor(relationCursor.get());
                 !relationKeyValue.empty();
                 relationKeyValue = LMDBInterface::getNextCursor(relationCursor.get())) {
                auto key = LMDBInterface::getKeyAsString(relationKeyValue);
                if (key == STRING_EM_INIT) {
                    continue;
                }
                auto data = LMDBInterface::getValueAsBlob(relationKeyValue);
                // resolve a rid of an edge from a key
                auto sp = split(key, ':');
                if (sp.size() != 2) {
                    throw Error(NOGDB_CTX_UNKNOWN_ERR, Error::Type::CONTEXT);
                }
                auto edgeId = RecordId{
                        static_cast<ClassId>(std::stoul(std::string{sp[0]}, nullptr, 0)),
                        static_cast<PositionId>(std::stoul(std::string{sp[1]}, nullptr, 0))
                };
                // resolve a rid of source and destination vertices from a value
                auto classId = ClassId{0};
                auto positionId = PositionId{0};
                auto offset = data.retrieve(&classId, 0, sizeof(ClassId));
                offset = data.retrieve(&positionId, offset, sizeof(PositionId));
                auto srcRid = RecordId{classId, positionId};
                offset = data.retrieve(&classId, offset, sizeof(ClassId));
                data.retrieve(&positionId, offset, sizeof(PositionId));
                auto dstRid = RecordId{classId, positionId};
                auto ptrEdgeClassDescriptor = dbSchema->find(baseTxn, edgeId.first);
                require(ptrEdgeClassDescriptor != nullptr);
                auto ptrSrcVertexClassDescriptor = dbSchema->find(baseTxn, srcRid.first);
                require(ptrSrcVertexClassDescriptor != nullptr);
                auto ptrDstVertexClassDescriptor = dbSchema->find(baseTxn, dstRid.first);
                require(ptrDstVertexClassDescriptor != nullptr);
                // update the relation in the graph structure
                dbRelation->createEdge(baseTxn, edgeId, srcRid, dstRid);
            }
            baseTxn.commit(*this);
        } catch (const Error &err) {
            baseTxn.rollback(*this);
            dbRelation->clear();
            dbSchema->clear();
            LMDBInterface::abortTxn(txn);
            throw err;
        } catch (Graph::ErrorType &err) {
            baseTxn.rollback(*this);
            dbRelation->clear();
            dbSchema->clear();
            LMDBInterface::abortTxn(txn);
            throw Error(err, Error::Type::GRAPH);
        } catch (LMDBInterface::ErrorType &err) {
            baseTxn.rollback(*this);
            dbRelation->clear();
            dbSchema->clear();
            LMDBInterface::abortTxn(txn);
            throw Error(err, Error::Type::DATASTORE);
        }
        // end of transaction
        LMDBInterface::abortTxn(txn);
    }

}
