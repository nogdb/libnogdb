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

#include "base_txn.hpp"
#include "graph.hpp"

#include "nogdb_error.h"

namespace nogdb {

    void Graph::createEdge(BaseTxn &txn, const RecordId &rid, const RecordId &srcRid, const RecordId &dstRid) {
        auto edge = lookupEdge(txn, rid);
        if (edge != nullptr) {
            throw ErrorType{GRAPH_DUP_EDGE};
        }
        auto sourceVertex = lookupVertex(txn, srcRid);
        auto targetVertex = lookupVertex(txn, dstRid);
        if (sourceVertex == nullptr) {
            sourceVertex = std::make_shared<Graph::Vertex>(srcRid);
            txn.addUncommittedVertex(sourceVertex);
        }
        if (targetVertex == nullptr) {
            targetVertex = std::make_shared<Graph::Vertex>(dstRid);
            txn.addUncommittedVertex(targetVertex);
        }
        auto newEdge = std::make_shared<Graph::Edge>(rid, sourceVertex, targetVertex);
        txn.addUncommittedEdge(newEdge);
        // update outgoing edge of a source vertex
        sourceVertex->out.insert(rid.first, rid.second, newEdge);
        // update incoming edge of a target vertex
        targetVertex->in.insert(rid.first, rid.second, newEdge);
    }

    void Graph::deleteEdge(BaseTxn &txn, const RecordId &rid) noexcept {
        if (auto edge = lookupEdge(txn, rid)) {
            auto findSrcVertex = edge->source.getLatestVersion();
            if (findSrcVertex.second) {
                if (auto sourceVertex = findSrcVertex.first.lock()) {
                    sourceVertex->out.erase(rid.first, rid.second);
                }
            }
            auto findTgtVertex = edge->target.getLatestVersion();
            if (findTgtVertex.second) {
                if (auto targetVertex = findTgtVertex.first.lock()) {
                    targetVertex->in.erase(rid.first, rid.second);
                }
            }
            if (edge->getState().second == TxnObject::StatusFlag::UNCOMMITTED_CREATE) {
                //forceDeleteEdge(rid);
                txn.deleteUncommittedEdge(rid);
            } else {
                edge->setStatus(TxnObject::StatusFlag::UNCOMMITTED_DELETE);
                txn.addUncommittedEdge(edge);
            }
        }
    }

    RecordId Graph::getVertexSrc(const BaseTxn &txn, const RecordId &rid) {
        auto edge = lookupEdge(txn, rid);
        if (edge == nullptr) {
            throw ErrorType{GRAPH_NOEXST_EDGE};
        }
        auto findSrcVertex = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                             edge->source.getStableVersion(txn.getVersionId()) : edge->source.getLatestVersion();
        if (findSrcVertex.second) {
            if (auto sourceVertex = findSrcVertex.first.lock()) {
                return sourceVertex->rid;
            }
        }
        throw ErrorType{GRAPH_UNKNOWN_ERR};
    }

    RecordId Graph::getVertexDst(const BaseTxn &txn, const RecordId &rid) {
        auto edge = lookupEdge(txn, rid);
        if (edge == nullptr) {
            throw ErrorType{GRAPH_NOEXST_EDGE};
        }
        auto findTgtVertex = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                             edge->target.getStableVersion(txn.getVersionId()) : edge->target.getLatestVersion();
        if (findTgtVertex.second) {
            if (auto targetVertex = findTgtVertex.first.lock()) {
                return targetVertex->rid;
            }
        }
        throw ErrorType{GRAPH_UNKNOWN_ERR};
    }

    std::pair<RecordId, RecordId> Graph::getVertexSrcDst(const BaseTxn &txn, const RecordId &rid) {
        auto edge = lookupEdge(txn, rid);
        if (edge == nullptr) {
            throw ErrorType{GRAPH_NOEXST_EDGE};
        }
        auto findSrcVertex = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                             edge->source.getStableVersion(txn.getVersionId()) : edge->source.getLatestVersion();
        auto findTgtVertex = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                             edge->target.getStableVersion(txn.getVersionId()) : edge->target.getLatestVersion();
        if (findSrcVertex.second && findTgtVertex.second) {
            if (auto sourceVertex = findSrcVertex.first.lock()) {
                if (auto targetVertex = findTgtVertex.first.lock()) {
                    return std::make_pair(sourceVertex->rid, targetVertex->rid);
                }
            }
        }
        throw ErrorType{GRAPH_UNKNOWN_ERR};
    }

    void Graph::alterVertexSrc(BaseTxn &txn, const RecordId &rid, const RecordId &srcRid) {
        auto edge = lookupEdge(txn, rid);
        if (edge == nullptr) {
            throw ErrorType{GRAPH_NOEXST_EDGE};
        }
        auto findOldSrcVertex = edge->source.getLatestVersion();
        if (findOldSrcVertex.second) {
            if (auto oldSrcVertex = findOldSrcVertex.first.lock()) {
                auto newSrcVertex = lookupVertex(txn, srcRid);
                if (newSrcVertex == nullptr) {
                    newSrcVertex = std::make_shared<Graph::Vertex>(srcRid);
                    txn.addUncommittedVertex(newSrcVertex);
                }
                // update outgoing edge of an old source vertex
                oldSrcVertex->out.erase(rid.first, rid.second);
                // update edge
                edge->source.addLatestVersion(newSrcVertex);
                txn.addUncommittedEdge(edge);
                // update outgoing edge of a new source vertex
                newSrcVertex->out.insert(rid.first, rid.second, edge);
                return;
            }
        }
        throw ErrorType{GRAPH_UNKNOWN_ERR};
    }

    void Graph::alterVertexDst(BaseTxn &txn, const RecordId &rid, const RecordId &dstRid) {
        auto edge = lookupEdge(txn, rid);
        if (edge == nullptr) {
            throw ErrorType{GRAPH_NOEXST_EDGE};
        }
        auto findOldDstVertex = edge->target.getLatestVersion();
        if (findOldDstVertex.second) {
            if (auto oldDstVertex = findOldDstVertex.first.lock()) {
                auto newDstVertex = lookupVertex(txn, dstRid);
                if (newDstVertex == nullptr) {
                    newDstVertex = std::make_shared<Graph::Vertex>(dstRid);
                    txn.addUncommittedVertex(newDstVertex);
                }
                // update incoming edge of an old destination vertex
                oldDstVertex->in.erase(rid.first, rid.second);
                // update edge
                edge->target.addLatestVersion(newDstVertex);
                txn.addUncommittedEdge(edge);
                // update incoming edge of a new destination vertex
                newDstVertex->in.insert(rid.first, rid.second, edge);
                return;
            }
        }
        throw ErrorType{GRAPH_UNKNOWN_ERR};
    }

    std::shared_ptr<Graph::Edge> Graph::lookupEdge(const BaseTxn &txn, const RecordId &rid) {
        RWSpinLockGuard<RWSpinLock> _(edges.splock);
        auto iterator = edges.elements.find(rid);
        if (iterator == edges.elements.cend()) {
            if (txn.getType() == BaseTxn::TxnType::READ_ONLY) {
                return nullptr;
            } else {
                auto edgePtr = txn.findUncommittedEdge(rid);
                if (edgePtr != nullptr) {
                    return (edgePtr->checkReadWrite()) ? nullptr : edgePtr;
                }
                return nullptr;
            }
        } else {
            if ((txn.getType() == BaseTxn::TxnType::READ_ONLY && iterator->second->checkReadOnly(txn.getVersionId())) ||
                (txn.getType() == BaseTxn::TxnType::READ_WRITE && iterator->second->checkReadWrite())) {
                return nullptr;
            }
        }
        return iterator->second;
    }

    void Graph::forceDeleteEdge(const RecordId &rid) noexcept {
        edges.lockAndErase(rid);
    }

    void Graph::forceDeleteEdges(const std::vector<RecordId> &rids) noexcept {
        RWSpinLockGuard<RWSpinLock> _(edges.splock, RWSpinLockMode::EXCLUSIVE_SPLOCK);
        for (const auto &rid: rids) {
            edges.elements.erase(rid);
        }
    }

}
