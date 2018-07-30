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
#include <limits>
#include <tuple>

#include "shared_lock.hpp"
#include "base_txn.hpp"
#include "utils.hpp" // for benchmarking

#include "nogdb_errors.h"

namespace nogdb {

    BaseTxn::BaseTxn(Context &ctx, bool isReadWrite, bool inMemory)
            : dsTxnHandler{nullptr},
              txnType{(isReadWrite) ? TxnType::READ_WRITE : TxnType::READ_ONLY},
              isWithDataStore{!inMemory} {
        auto envHandler = ctx.envHandler.get();
        if (!isReadWrite) {
            if (isWithDataStore) {
                dsTxnHandler = new storage_engine::LMDBTxn(envHandler, storage_engine::lmdb::TXN_RO);
            }
            txnId = ctx.dbTxnStat->fetchAddMaxTxnId();
            // get the recent version that was committed
            versionId = ctx.dbTxnStat->maxVersionId;
            ctx.dbTxnStat->addActiveTxnId(txnId, versionId);
        } else {
            if (isWithDataStore) {
                dsTxnHandler = new storage_engine::LMDBTxn(envHandler, storage_engine::lmdb::TXN_RW);
                // check if the previous writer committed all stuff in memory
                {
                    ReadLock<boost::shared_mutex> _(*(ctx.dbWriterMutex));
                }
            } else {
                //NOTE: caller must take a responsibility of concurrency control
            }
            // copy a current version of dbInfo
            {
                ReadLock<boost::shared_mutex> _(*ctx.dbInfoMutex);
                dbInfo = *ctx.dbInfo;
            }
            // txnId for read-write txn is unused
            txnId = 0;
            versionId = ctx.dbTxnStat->maxVersionId + 1;
            if (versionId == std::numeric_limits<TxnId>::max()) {
                if (isWithDataStore) {
                    dsTxnHandler->rollback();
                }
                throw NOGDB_TXN_ERROR(NOGDB_TXN_VERSION_MAXREACH);
            }
        }
    }

    void BaseTxn::addUncommittedVertex(const std::shared_ptr<Graph::Vertex> &vertex) {
        if (ucVertices.find(vertex->rid) == ucVertices.cend()) {
            ucVertices.emplace(vertex->rid, vertex);
        }
    }

    void BaseTxn::deleteUncommittedVertex(const RecordId &rid) {
        ucVertices.erase(rid);
    }

    void BaseTxn::addUncommittedEdge(const std::shared_ptr<Graph::Edge> &edge) {
        if (ucEdges.find(edge->rid) == ucEdges.cend()) {
            ucEdges.emplace(edge->rid, edge);
        }
    }

    void BaseTxn::deleteUncommittedEdge(const RecordId &rid) {
        ucEdges.erase(rid);
    }

    void BaseTxn::addUncommittedSchema(const std::shared_ptr<Schema::ClassDescriptor> &classPtr) {
        if (ucSchema.find(classPtr->id) == ucSchema.cend()) {
            ucSchema.emplace(classPtr->id, classPtr);
        }
    }

    void BaseTxn::deleteUncommittedSchema(const ClassId &classId) {
        ucSchema.erase(classId);
    }

    bool BaseTxn::commit(Context &ctx) {
        if (!isCompleted) {
            if (txnType == TxnType::READ_WRITE) {
                // prevent another txn writer to enter a critical section
                // after datastore has committed
                WriteLock<boost::shared_mutex> _(*(ctx.dbWriterMutex));
                if (isWithDataStore) {
                    dsTxnHandler->commit();
                    isCommitDatastore = true;
                }
                auto oldestTxn = ctx.dbTxnStat->minActiveTxnId();
                auto currentMinVersion = (oldestTxn.first != 0) ? oldestTxn.second : versionId - 1;
                // commit changes in database schema
                if (!ucSchema.empty()) {
                    DeleteQueue<ClassId> tmpDeletedClassId;
                    for (const auto &classDescriptor: ucSchema) {
                        if (auto classDescriptorPtr = classDescriptor.second) {
                            auto currentStatus = classDescriptorPtr->getState().second;
                            if (currentStatus == TxnObject::StatusFlag::UNCOMMITTED_DELETE) {
                                tmpDeletedClassId.emplace_back(std::make_pair(classDescriptorPtr->id, versionId));
                            } else if (currentStatus == TxnObject::StatusFlag::UNCOMMITTED_CREATE) {
                                ctx.dbSchema->schemaInfo.lockAndEmplace(classDescriptorPtr->id, classDescriptorPtr);
                            } else {
                                classDescriptorPtr->name.clearStableVersion(currentMinVersion);
                                classDescriptorPtr->properties.clearStableVersion(currentMinVersion);
                                classDescriptorPtr->super.clearStableVersion(currentMinVersion);
                                classDescriptorPtr->sub.clearStableVersion(currentMinVersion);
                            }
                            classDescriptorPtr->updateState(versionId);
                            classDescriptorPtr->name.upgradeStableVersion(versionId);
                            classDescriptorPtr->properties.upgradeStableVersion(versionId);
                            classDescriptorPtr->super.upgradeStableVersion(versionId);
                            classDescriptorPtr->sub.upgradeStableVersion(versionId);
                        }
                    }
                    ctx.dbSchema->deletedClassId.push_back(tmpDeletedClassId);
                }
                // commit changes in database relation
                if (ucVertices.size() + ucEdges.size() > 0) {
                    DeleteQueue<RecordId> tmpDeletedVertices;
                    DeleteQueue<RecordId> tmpDeletedEdges;
                    for (const auto &vertex: ucVertices) {
                        if (auto vertexPtr = vertex.second) {
                            auto currentStatus = vertexPtr->getState().second;
                            if (currentStatus == TxnObject::StatusFlag::UNCOMMITTED_DELETE) {
                                tmpDeletedVertices.emplace_back(std::make_pair(vertexPtr->rid, versionId));
                            } else if (currentStatus == TxnObject::StatusFlag::UNCOMMITTED_CREATE) {
                                ctx.dbRelation->vertices.lockAndEmplace(vertex.first, vertex.second);
                            }
                            vertexPtr->updateState(versionId);
                        }
                    }
                    for (const auto &edge: ucEdges) {
                        if (auto edgePtr = edge.second) {
                            auto currentStatus = edgePtr->getState().second;
                            if (currentStatus == TxnObject::StatusFlag::UNCOMMITTED_DELETE) {
                                tmpDeletedEdges.emplace_back(std::make_pair(edgePtr->rid, versionId));
                            } else if (currentStatus == TxnObject::StatusFlag::UNCOMMITTED_CREATE) {
                                ctx.dbRelation->edges.lockAndEmplace(edge.first, edge.second);
                            } else {
                                edgePtr->source.clearStableVersion(currentMinVersion);
                                edgePtr->target.clearStableVersion(currentMinVersion);
                            }

                            auto srcVertexUnstable = edgePtr->source.getUnstableVersion();
                            if (auto srcVertexUnstablePtr = srcVertexUnstable.first.lock()) {
                                srcVertexUnstablePtr->out.clear(currentMinVersion);
                                if (auto tmp = srcVertexUnstablePtr->out.get(edgePtr->rid.first, edgePtr->rid.second)) {
                                    tmp->upgradeStableVersion(versionId);
                                }
                            }
                            auto srcVertexStable = edgePtr->source.getStableVersion();
                            if (auto srcVertexStablePtr = srcVertexStable.first.lock()) {
                                srcVertexStablePtr->out.clear(currentMinVersion);
                                if (auto tmp = srcVertexStablePtr->out.get(edgePtr->rid.first, edgePtr->rid.second)) {
                                    tmp->upgradeStableVersion(versionId);
                                }
                            }
                            auto dstVertexUnstable = edgePtr->target.getUnstableVersion();
                            if (auto dstVertexUnstablePtr = dstVertexUnstable.first.lock()) {
                                dstVertexUnstablePtr->in.clear(currentMinVersion);
                                if (auto tmp = dstVertexUnstablePtr->in.get(edgePtr->rid.first, edgePtr->rid.second)) {
                                    tmp->upgradeStableVersion(versionId);
                                }
                            }
                            auto dstVertexStable = edgePtr->target.getStableVersion();
                            if (auto dstVertexStablePtr = dstVertexStable.first.lock()) {
                                dstVertexStablePtr->in.clear(currentMinVersion);
                                if (auto tmp = dstVertexStablePtr->in.get(edgePtr->rid.first, edgePtr->rid.second)) {
                                    tmp->upgradeStableVersion(versionId);
                                }
                            }
                            edgePtr->updateState(versionId);
                            edgePtr->source.upgradeStableVersion(versionId);
                            edgePtr->target.upgradeStableVersion(versionId);
                        }
                    }
                    ctx.dbRelation->deletedVertices.push_back(tmpDeletedVertices);
                    ctx.dbRelation->deletedEdges.push_back(tmpDeletedEdges);
                }
                if (ucSchema.size() + ucVertices.size() + ucEdges.size() > 0) {
                    {   // save changes in dbInfo
                        WriteLock<boost::shared_mutex> _(*(ctx.dbInfoMutex));
                        (*ctx.dbInfo) = dbInfo;
                    }
                }
                // allow the next txns to see the latest version and updates
                ctx.dbTxnStat->fetchAddMaxVersionId();
                auto minTxn = ctx.dbTxnStat->minActiveTxnId();
                if (minTxn.first == 0 && minTxn.second == 0) {
                    ctx.dbSchema->clearDeletedElements(versionId);
                    ctx.dbRelation->clearDeletedElements(versionId);
                }
            } else {
                if (ctx.dbTxnStat->isLastMinVersionId(txnId)) {
                    ctx.dbSchema->clearDeletedElements(versionId + 1);
                    ctx.dbRelation->clearDeletedElements(versionId + 1);
                }
                ctx.dbTxnStat->removeActiveTxnId(txnId);
                if (isWithDataStore) {
                    dsTxnHandler->rollback();
                }
            }
            isCompleted = true;
            return true;
        }
        return false;
    }

    bool BaseTxn::rollback(Context &ctx) noexcept {
        if (!isCompleted) {
            if (txnType == TxnType::READ_WRITE) {
                for (const auto &vertex: ucVertices) {
                    auto vertexPtr = vertex.second;
                    require(vertexPtr != nullptr);
                    auto state = vertexPtr->getState();
                    auto status = state.second;
                    if (status == TxnObject::StatusFlag::UNCOMMITTED_DELETE) {
                        vertexPtr->setStatus(TxnObject::StatusFlag::COMMITTED_CREATE);
                    }
                }
                for (const auto &edge: ucEdges) {
                    auto edgePtr = edge.second;
                    require(edgePtr != nullptr);
                    auto state = edgePtr->getState();
                    auto status = state.second;
                    if (status == TxnObject::StatusFlag::UNCOMMITTED_DELETE) {
                        edgePtr->setStatus(TxnObject::StatusFlag::COMMITTED_CREATE);
                    }

                    auto srcVertexUnstable = edgePtr->source.getUnstableVersion();
                    if (srcVertexUnstable.second) {
                        if (auto srcVertexUnstablePtr = srcVertexUnstable.first.lock()) {
                            // clear only uncommitted version
                            srcVertexUnstablePtr->out.clear(edgePtr->rid.first, edgePtr->rid.second, 0);
                        }
                    }
                    auto srcVertexStable = edgePtr->source.getStableVersion();
                    if (srcVertexStable.second) {
                        if (auto srcVertexStablePtr = srcVertexStable.first.lock()) {
                            // clear only uncommitted version
                            srcVertexStablePtr->out.clear(edgePtr->rid.first, edgePtr->rid.second, 0);
                        }
                    }
                    auto dstVertexUnstable = edgePtr->target.getUnstableVersion();
                    if (dstVertexUnstable.second) {
                        if (auto dstVertexUnablePtr = dstVertexUnstable.first.lock()) {
                            // clear only uncommitted version
                            dstVertexUnablePtr->in.clear(edgePtr->rid.first, edgePtr->rid.second, 0);
                        }
                    }
                    auto dstVertexStable = edgePtr->target.getStableVersion();
                    if (dstVertexStable.second) {
                        if (auto dstVertexStablePtr = dstVertexStable.first.lock()) {
                            // clear only uncommitted version
                            dstVertexStablePtr->in.clear(edgePtr->rid.first, edgePtr->rid.second, 0);
                        }
                    }
                    edgePtr->source.disableUnstableVersion();
                    edgePtr->target.disableUnstableVersion();
                }
                for (const auto &classDescriptor: ucSchema) {
                    auto classDescriptorPtr = classDescriptor.second;
                    require(classDescriptorPtr != nullptr);
                    auto state = classDescriptorPtr->getState();
                    auto status = state.second;
                    if (status == TxnObject::StatusFlag::UNCOMMITTED_DELETE) {
                        classDescriptorPtr->setStatus(TxnObject::StatusFlag::COMMITTED_CREATE);
                    }
                    classDescriptorPtr->name.disableUnstableVersion();
                    classDescriptorPtr->properties.disableUnstableVersion();
                    classDescriptorPtr->super.disableUnstableVersion();
                    classDescriptorPtr->sub.disableUnstableVersion();
                }
            } else {
                if (ctx.dbTxnStat->isLastMinVersionId(txnId)) {
                    ctx.dbRelation->clearDeletedElements(versionId + 1);
                    ctx.dbSchema->clearDeletedElements(versionId + 1);
                }
                ctx.dbTxnStat->removeActiveTxnId(txnId);
            }
            if (isWithDataStore && !isCommitDatastore) {
                dsTxnHandler->rollback();
            }
            isCompleted = true;
            return true;
        }
        return false;
    }

}
