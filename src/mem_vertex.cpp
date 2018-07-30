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

#include <algorithm>

#include "base_txn.hpp"
#include "graph.hpp"

#include "nogdb_errors.h"
#include "nogdb_types.h"

namespace nogdb {

    bool Graph::createVertex(BaseTxn &txn, const RecordId &rid) {
        if (auto vertex = lookupVertex(txn, rid)) {
            return false;
        }
        auto vertexPtr = std::make_shared<Vertex>(rid);
        txn.addUncommittedVertex(vertexPtr);
        return true;
    }

    void Graph::deleteVertex(BaseTxn &txn, const RecordId &rid) noexcept {
        if (auto vertex = lookupVertex(txn, rid)) {
            // delete all in-edges
            for (const auto &inEdgeRids: vertex->in.keys()) {
                auto inEdgeClassId = inEdgeRids.first;
                // get each in-edge
                for (const auto &inEdgePosId: inEdgeRids.second) {
                    auto findInEdge = vertex->in.find(inEdgeClassId, inEdgePosId);
                    if (findInEdge.second) {
                        if (auto inEdge = findInEdge.first.lock()) {
                            // get a source vertex of an in-edge
                            auto findSrcVertex = inEdge->source.getLatestVersion();
                            if (findSrcVertex.second) {
                                // delete an in-edge as an out-edge of a source vertex
                                if (auto sourceVertex = findSrcVertex.first.lock()) {
                                    sourceVertex->out.erase(inEdge->rid.first, inEdge->rid.second);
                                }
                            }
                            // delete an in-edge
                            if (inEdge->getState().second == TxnObject::StatusFlag::UNCOMMITTED_CREATE) {
                                //forceDeleteEdge(inEdge->rid);
                                txn.deleteUncommittedEdge(inEdge->rid);
                            } else {
                                inEdge->setStatus(TxnObject::StatusFlag::UNCOMMITTED_DELETE);
                                txn.addUncommittedEdge(inEdge);
                            }
                        }
                    }
                }
            }
            // delete all out-edges
            for (const auto &outEdgeRids: vertex->out.keys()) {
                auto outEdgeClassId = outEdgeRids.first;
                // get each out-edge
                for (const auto &outEdgePosId: outEdgeRids.second) {
                    auto findOutEdge = vertex->out.find(outEdgeClassId, outEdgePosId);
                    if (findOutEdge.second) {
                        if (auto outEdge = findOutEdge.first.lock()) {
                            // get a target vertex of an out-edge
                            auto findDstVertex = outEdge->target.getLatestVersion();
                            if (findDstVertex.second) {
                                // delete an out-edge as an in-edge of a target vertex
                                if (auto targetVertex = findDstVertex.first.lock()) {
                                    targetVertex->in.erase(outEdge->rid.first, outEdge->rid.second);
                                }
                            }
                            // delete an out-edge
                            if (outEdge->getState().second == TxnObject::StatusFlag::UNCOMMITTED_CREATE) {
                                //forceDeleteEdge(outEdge->rid);
                                txn.deleteUncommittedEdge(outEdge->rid);
                            } else {
                                outEdge->setStatus(TxnObject::StatusFlag::UNCOMMITTED_DELETE);
                                txn.addUncommittedEdge(outEdge);
                            }
                        }
                    }
                }
            }
            // delete a vertex
            if (vertex->getState().second == TxnObject::StatusFlag::UNCOMMITTED_CREATE) {
                //forceDeleteVertex(vertex->rid);
                txn.deleteUncommittedVertex(vertex->rid);
            } else {
                vertex->setStatus(TxnObject::StatusFlag::UNCOMMITTED_DELETE);
                txn.addUncommittedVertex(vertex);
            }
        }
    }

    std::vector<RecordId> Graph::getEdgeIn(const BaseTxn &txn, const RecordId &rid, const ClassId &classId) {
        auto vertex = lookupVertex(txn, rid);
        if (vertex == nullptr) {
            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
        }
        auto result = std::vector<RecordId> {};
        if (classId) {
            for (const auto &posId: vertex->in.keys(classId)) {
                auto findInEdge = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                                  vertex->in.find(txn.getVersionId(), classId, posId) : vertex->in.find(classId, posId);
                if (findInEdge.second) {
                    if (auto inEdge = findInEdge.first.lock()) {
                        result.push_back(inEdge->rid);
                    }
                }
            }
        } else {
            for (const auto &inEdgeRids: vertex->in.keys()) {
                auto inEdgeClassId = inEdgeRids.first;
                for (const auto &inEdgePosId: inEdgeRids.second) {
                    auto findInEdge = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                                      vertex->in.find(txn.getVersionId(), inEdgeClassId, inEdgePosId) :
                                      vertex->in.find(inEdgeClassId, inEdgePosId);
                    if (findInEdge.second) {
                        if (auto inEdge = findInEdge.first.lock()) {
                            result.push_back(inEdge->rid);
                        }
                    }
                }
            }
        }
        return result;
    }

    std::vector<ClassId> Graph::getEdgeClassIn(const BaseTxn &txn, const RecordId &rid) {
        auto vertex = lookupVertex(txn, rid);
        if (vertex == nullptr) {
            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
        }
        auto result = std::vector<ClassId> {};
        for (const auto &inEdgeRids: vertex->in.keys()) {
            auto inEdgeClassId = inEdgeRids.first;
            for (const auto &inEdgePosId: inEdgeRids.second) {
                auto findInEdge = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                                  vertex->in.find(txn.getVersionId(), inEdgeClassId, inEdgePosId) :
                                  vertex->in.find(inEdgeClassId, inEdgePosId);
                if (findInEdge.second) {
                    if (auto inEdge = findInEdge.first.lock()) {
                        result.push_back(inEdgeClassId);
                        break;
                    }
                }
            }
        }
        return result;
    }

    std::vector<RecordId> Graph::getEdgeOut(const BaseTxn &txn, const RecordId &rid, const ClassId &classId) {
        auto vertex = lookupVertex(txn, rid);
        if (vertex == nullptr) {
            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
        }
        auto result = std::vector<RecordId> {};
        if (classId) {
            for (const auto &posId: vertex->out.keys(classId)) {
                auto findOutEdge = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                                   vertex->out.find(txn.getVersionId(), classId, posId) :
                                   vertex->out.find(classId, posId);
                if (findOutEdge.second) {
                    if (auto outEdge = findOutEdge.first.lock()) {
                        result.push_back(outEdge->rid);
                    }
                }
            }
        } else {
            for (const auto &outEdgeRids: vertex->out.keys()) {
                auto outEdgeClassId = outEdgeRids.first;
                for (const auto &outEdgePosId: outEdgeRids.second) {
                    auto findOutEdge = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                                       vertex->out.find(txn.getVersionId(), outEdgeClassId, outEdgePosId) :
                                       vertex->out.find(outEdgeClassId, outEdgePosId);
                    if (findOutEdge.second) {
                        if (auto outEdge = findOutEdge.first.lock()) {
                            result.push_back(outEdge->rid);
                        }
                    }
                }
            }
        }
        return result;
    }

    std::vector<ClassId> Graph::getEdgeClassOut(const BaseTxn &txn, const RecordId &rid) {
        auto vertex = lookupVertex(txn, rid);
        if (vertex == nullptr) {
            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
        }
        auto result = std::vector<ClassId> {};
        for (const auto &outEdgeRids: vertex->out.keys()) {
            auto outEdgeClassId = outEdgeRids.first;
            for (const auto &outEdgePosId: outEdgeRids.second) {
                auto findOutEdge = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                                   vertex->out.find(txn.getVersionId(), outEdgeClassId, outEdgePosId) :
                                   vertex->out.find(outEdgeClassId, outEdgePosId);
                if (findOutEdge.second) {
                    if (auto outEdge = findOutEdge.first.lock()) {
                        result.push_back(outEdgeClassId);
                        break;
                    }
                }
            }
        }
        return result;
    }

    std::vector<RecordId> Graph::getEdgeInOut(const BaseTxn &txn, const RecordId &rid, const ClassId &classId) {
        auto vertex = lookupVertex(txn, rid);
        if (vertex == nullptr) {
            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
        }
        auto result = std::vector<RecordId> {};
        if (classId) {
            for (const auto &posId: vertex->in.keys(classId)) {
                auto findInEdge = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                                  vertex->in.find(txn.getVersionId(), classId, posId) : vertex->in.find(classId, posId);
                if (findInEdge.second) {
                    if (auto inEdge = findInEdge.first.lock()) {
                        result.push_back(inEdge->rid);
                    }
                }
            }
            for (const auto &posId: vertex->out.keys(classId)) {
                auto findOutEdge = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                                   vertex->out.find(txn.getVersionId(), classId, posId) :
                                   vertex->out.find(classId, posId);
                if (findOutEdge.second) {
                    if (auto outEdge = findOutEdge.first.lock()) {
                        result.push_back(outEdge->rid);
                    }
                }
            }
        } else {
            for (const auto &inEdgeRids: vertex->in.keys()) {
                auto inEdgeClassId = inEdgeRids.first;
                for (const auto &inEdgePosId: inEdgeRids.second) {
                    auto findInEdge = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                                      vertex->in.find(txn.getVersionId(), inEdgeClassId, inEdgePosId) :
                                      vertex->in.find(inEdgeClassId, inEdgePosId);
                    if (findInEdge.second) {
                        if (auto inEdge = findInEdge.first.lock()) {
                            result.push_back(inEdge->rid);
                        }
                    }
                }
            }
            for (const auto &outEdgeRids: vertex->out.keys()) {
                auto outEdgeClassId = outEdgeRids.first;
                for (const auto &outEdgePosId: outEdgeRids.second) {
                    auto findOutEdge = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                                       vertex->out.find(txn.getVersionId(), outEdgeClassId, outEdgePosId) :
                                       vertex->out.find(outEdgeClassId, outEdgePosId);
                    if (findOutEdge.second) {
                        if (auto outEdge = findOutEdge.first.lock()) {
                            result.push_back(outEdge->rid);
                        }
                    }
                }
            }
        }
        std::sort(result.begin(), result.end(), [](const RecordId &lhs, const RecordId &rhs) {
            return (lhs.first == rhs.first) ? lhs.second < rhs.second : lhs.first < rhs.first;
        });
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }

    std::vector<ClassId> Graph::getEdgeClassInOut(const BaseTxn &txn, const RecordId &rid) {
        auto vertex = lookupVertex(txn, rid);
        if (vertex == nullptr) {
            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
        }
        auto result = std::vector<ClassId> {};
        for (const auto &inEdgeRids: vertex->in.keys()) {
            auto inEdgeClassId = inEdgeRids.first;
            for (const auto &inEdgePosId: inEdgeRids.second) {
                auto findInEdge = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                                  vertex->in.find(txn.getVersionId(), inEdgeClassId, inEdgePosId) :
                                  vertex->in.find(inEdgeClassId, inEdgePosId);
                if (findInEdge.second) {
                    if (auto inEdge = findInEdge.first.lock()) {
                        result.push_back(inEdgeClassId);
                        break;
                    }
                }
            }
        }
        for (const auto &outEdgeRids: vertex->out.keys()) {
            auto outEdgeClassId = outEdgeRids.first;
            for (const auto &outEdgePosId: outEdgeRids.second) {
                auto findOutEdge = (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                                   vertex->out.find(txn.getVersionId(), outEdgeClassId, outEdgePosId) :
                                   vertex->out.find(outEdgeClassId, outEdgePosId);
                if (findOutEdge.second) {
                    if (auto outEdge = findOutEdge.first.lock()) {
                        result.push_back(outEdgeClassId);
                        break;
                    }
                }
            }
        }
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }

    std::shared_ptr<Graph::Vertex> Graph::lookupVertex(const BaseTxn &txn, const RecordId &rid) {
        RWSpinLockGuard<RWSpinLock> _(vertices.splock);
        auto iterator = vertices.elements.find(rid);
        if (iterator == vertices.elements.cend()) {
            if (txn.getType() == BaseTxn::TxnType::READ_ONLY) {
                return nullptr;
            } else {
                auto vertexPtr = txn.findUncommittedVertex(rid);
                if (vertexPtr != nullptr) {
                    return (vertexPtr->checkReadWrite()) ? nullptr : vertexPtr;
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

    void Graph::forceDeleteVertex(const RecordId &rid) noexcept {
        vertices.lockAndErase(rid);
    }

    void Graph::forceDeleteVertices(const std::vector<RecordId> &rids) noexcept {
        RWSpinLockGuard<RWSpinLock> _(vertices.splock, RWSpinLockMode::EXCLUSIVE_SPLOCK);
        for (const auto &rid: rids) {
            vertices.elements.erase(rid);
        }
    }

}

