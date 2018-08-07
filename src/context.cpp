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
#include "storage_engine.hpp"
#include "graph.hpp"
#include "validate.hpp"
#include "schema.hpp"

#include "nogdb_context.h"

using namespace nogdb::utils::assertion;

namespace nogdb {

    Context::Context(const std::string &dbPath)
            : Context{dbPath, DEFAULT_NOGDB_MAX_DATABASE_NUMBER, DEFAULT_NOGDB_MAX_DATABASE_SIZE} {};

    Context::Context(const std::string &dbPath, unsigned int maxDbNum)
            : Context{dbPath, maxDbNum, DEFAULT_NOGDB_MAX_DATABASE_SIZE} {};

    Context::Context(const std::string &dbPath, unsigned long maxDbSize)
            : Context{dbPath, DEFAULT_NOGDB_MAX_DATABASE_NUMBER, maxDbSize} {};

    Context::Context(const std::string &dbPath, unsigned int maxDbNum, unsigned long maxDbSize) {
        if (!utils::io::fileExists(dbPath)) {
            mkdir(dbPath.c_str(), 0755);
        }
        const auto lockFile = dbPath + DB_LOCK_FILE;
        lockContextFileDescriptor = utils::io::openLockFile(lockFile.c_str());
        if (lockContextFileDescriptor == -1) {
            if (errno == EWOULDBLOCK || errno == EEXIST) {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_IS_LOCKED);
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_UNKNOWN_ERR);
            }
        } else {
            envHandler = std::make_shared<storage_engine::LMDBEnv>(
                    dbPath, maxDbNum, maxDbSize, DEFAULT_NOGDB_MAX_READERS
            );
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
            initDatabase();
        }
    }

    Context::~Context() noexcept {
        utils::io::unlockFile(lockContextFileDescriptor);
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
        auto currentTime = std::to_string(utils::datetime::currentTimestamp());
        // perform read-write operations
        auto wtxn = storage_engine::LMDBTxn(envHandler.get(), storage_engine::lmdb::TXN_RW);
        // prepare schema for classes, properties, and relations
        try {
            auto classDBHandler = wtxn.openDbi(TB_CLASSES, true);
            auto propDBHndler = wtxn.openDbi(TB_PROPERTIES, true);
            auto indexDBHandler = wtxn.openDbi(TB_INDEXES, true, false);
            auto relationDBHandler = wtxn.openDbi(TB_RELATIONS);
            classDBHandler.put(ClassId{INIT_UINT16_EM}, currentTime);
            propDBHndler.put(PropertyId{INIT_UINT16_EM}, currentTime);
//            indexDBHandler.put(PropertyId{INIT_UINT16_EM}, currentTime);
            relationDBHandler.put(INIT_STRING_EM, currentTime);
            wtxn.commit();
        } catch (const Error &err) {
            wtxn.rollback();
            throw err;
        }
        // perform read-only operations
        auto rtxn = storage_engine::LMDBTxn(envHandler.get(), storage_engine::lmdb::TXN_RO);
        // create read-write in memory transaction
        BaseTxn baseTxn{*this, true, true};
        // retrieve classes information
        try {
            auto inheritanceInfo = Schema::InheritanceInfo{};
            auto classCursor = rtxn.openCursor(TB_CLASSES, true);
            for (auto classKeyValue = classCursor.getNext();
                 !classKeyValue.empty();
                 classKeyValue = classCursor.getNext()) {
                auto key = classKeyValue.key.data.numeric<ClassId>();
                if (key == ClassId{INIT_UINT16_EM}) {
                    continue;
                }
                auto data = classKeyValue.val.data.blob();
                auto classType = ClassType::UNDEFINED;
                auto offset = data.retrieve(&classType, 0, sizeof(ClassType));
                auto superClassId = ClassId{0};
                offset = data.retrieve(&superClassId, offset, sizeof(superClassId));
                auto nameLength = data.size() - offset;
                require(nameLength > 0);
                Blob::Byte nameBytes[nameLength];
                data.retrieve(nameBytes, offset, nameLength);
                auto className = std::string(reinterpret_cast<char *>(nameBytes), nameLength);
                auto classDescriptor = std::make_shared<Schema::ClassDescriptor>(key, className, classType);
                dbSchema->insert(baseTxn, classDescriptor);
                inheritanceInfo.emplace_back(std::make_pair(classDescriptor->id, superClassId));
                if (classDescriptor->id > dbInfo->maxClassId) {
                    baseTxn.dbInfo.maxClassId = classDescriptor->id;
                }
                ++baseTxn.dbInfo.numClass;
            }
            dbSchema->apply(baseTxn, inheritanceInfo);
        } catch (const Error &err) {
            baseTxn.rollback(*this);
            dbSchema->clear();
            throw err;
        }

        // retrieve properties and indexing information
        try {
            auto propCursor = rtxn.openCursor(TB_PROPERTIES, true);
            auto indexCursor = rtxn.openCursor(TB_INDEXES, true, false);
            for (auto propKeyValue = propCursor.getNext();
                 !propKeyValue.empty();
                 propKeyValue = propCursor.getNext()) {
                auto key = propKeyValue.key.data.numeric<PropertyId>();
                if (key == PropertyId{INIT_UINT16_EM}) {
                    continue;
                }
                auto data = propKeyValue.val.data.blob();
                auto propType = PropertyType::UNDEFINED;
                auto propClassId = PropertyId{0};
                auto offset = data.retrieve(&propType, 0, sizeof(PropertyDescriptor::type));
                offset = data.retrieve(&propClassId, offset, sizeof(propClassId));
                auto nameLength = data.size() - offset;
                require(nameLength > 0);
                Blob::Byte nameBytes[nameLength];
                data.retrieve(nameBytes, offset, nameLength);
                auto propertyName = std::string(reinterpret_cast<char *>(nameBytes), nameLength);
                auto propertyDescriptor = Schema::PropertyDescriptor{key, propType};
                auto ptrClassDescriptor = dbSchema->find(baseTxn, propClassId);
                require(ptrClassDescriptor != nullptr);
                // get indexes associated with property
                auto isCompositeNumeric = uint8_t{0};
                auto isUniqueNumeric = uint8_t{1};
                auto indexId = IndexId{0};
                auto classId = ClassId{0};
                for (auto indexKeyValue = indexCursor.find(propertyDescriptor.id);
                     !indexKeyValue.empty();
                     indexKeyValue = indexCursor.getNextDup()) {
                    key = indexKeyValue.key.data.numeric<PropertyId>();
                    if (key != propertyDescriptor.id) {
                        break;
                    }
                    data = indexKeyValue.val.data.blob();
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
        } catch (const Error &err) {
            baseTxn.rollback(*this);
            dbSchema->clear();
            throw err;
        }

        // retrieve relations information
        try {
            auto relationCursor = rtxn.openCursor(TB_RELATIONS);
            for (auto relationKeyValue = relationCursor.getNext();
                 !relationKeyValue.empty();
                 relationKeyValue = relationCursor.getNext()) {
                auto key = relationKeyValue.key.data.string();
                if (key == INIT_STRING_EM) {
                    continue;
                }
                auto data = relationKeyValue.val.data.blob();
                // resolve a rid of an edge from a key
                auto sp = utils::string::split(key, ':');
                if (sp.size() != 2) {
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_UNKNOWN_ERR);
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
            throw err;
        }
        // end of transaction
    }

}
