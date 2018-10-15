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

#include "schema.hpp"
#include "lmdb_engine.hpp"
#include "algorithm.hpp"
#include "relation_adapter.hpp"

#include "nogdb.h"

namespace nogdb {

  ResultSet Traverse::inEdgeBfs(const Txn &txn,
                                const RecordDescriptor &recordDescriptor,
                                unsigned int minDepth,
                                unsigned int maxDepth,
                                const GraphFilter &pathFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingVertex(recordDescriptor);

    auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    return algorithm::GraphTraversal::breadthFirstSearch(
        txn, vertexClassInfo, recordDescriptor, minDepth, maxDepth, adapter::relation::Direction::IN, pathFilter);
  }

  ResultSetCursor Traverse::inEdgeBfsCursor(const Txn &txn,
                                            const RecordDescriptor &recordDescriptor,
                                            unsigned int minDepth,
                                            unsigned int maxDepth,
                                            const GraphFilter &pathFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingVertex(recordDescriptor);

    auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    auto result = algorithm::GraphTraversal::breadthFirstSearchRdesc(
        txn, vertexClassInfo, recordDescriptor, minDepth, maxDepth, adapter::relation::Direction::IN, pathFilter);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSet Traverse::outEdgeBfs(const Txn &txn,
                                 const RecordDescriptor &recordDescriptor,
                                 unsigned int minDepth,
                                 unsigned int maxDepth,
                                 const GraphFilter &pathFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingVertex(recordDescriptor);

    auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    return algorithm::GraphTraversal::breadthFirstSearch(
        txn, vertexClassInfo, recordDescriptor, minDepth, maxDepth, adapter::relation::Direction::OUT, pathFilter);
  }

  ResultSetCursor Traverse::outEdgeBfsCursor(const Txn &txn,
                                             const RecordDescriptor &recordDescriptor,
                                             unsigned int minDepth,
                                             unsigned int maxDepth,
                                             const GraphFilter &pathFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingVertex(recordDescriptor);

    auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    auto result = algorithm::GraphTraversal::breadthFirstSearchRdesc(
        txn, vertexClassInfo, recordDescriptor, minDepth, maxDepth, adapter::relation::Direction::OUT, pathFilter);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSet Traverse::allEdgeBfs(const Txn &txn,
                                 const RecordDescriptor &recordDescriptor,
                                 unsigned int minDepth,
                                 unsigned int maxDepth,
                                 const GraphFilter &pathFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingVertex(recordDescriptor);

    auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    return algorithm::GraphTraversal::breadthFirstSearch(
        txn, vertexClassInfo, recordDescriptor, minDepth, maxDepth, adapter::relation::Direction::ALL, pathFilter);
  }

  ResultSetCursor Traverse::allEdgeBfsCursor(const Txn &txn,
                                             const RecordDescriptor &recordDescriptor,
                                             unsigned int minDepth,
                                             unsigned int maxDepth,
                                             const GraphFilter &pathFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingVertex(recordDescriptor);

    auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    auto result = algorithm::GraphTraversal::breadthFirstSearchRdesc(
        txn, vertexClassInfo, recordDescriptor, minDepth, maxDepth, adapter::relation::Direction::ALL, pathFilter);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSet Traverse::inEdgeDfs(const Txn &txn,
                                const RecordDescriptor &recordDescriptor,
                                unsigned int minDepth,
                                unsigned int maxDepth,
                                const GraphFilter &pathFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingVertex(recordDescriptor);

    auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    return algorithm::GraphTraversal::depthFirstSearch(
        txn, vertexClassInfo, recordDescriptor, minDepth, maxDepth, adapter::relation::Direction::IN, pathFilter);
  }

  ResultSetCursor Traverse::inEdgeDfsCursor(const Txn &txn,
                                            const RecordDescriptor &recordDescriptor,
                                            unsigned int minDepth,
                                            unsigned int maxDepth,
                                            const GraphFilter &pathFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingVertex(recordDescriptor);

    auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    auto result = algorithm::GraphTraversal::depthFirstSearchRdesc(
        txn, vertexClassInfo, recordDescriptor, minDepth, maxDepth, adapter::relation::Direction::IN, pathFilter);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSet Traverse::outEdgeDfs(const Txn &txn,
                                 const RecordDescriptor &recordDescriptor,
                                 unsigned int minDepth,
                                 unsigned int maxDepth,
                                 const GraphFilter &pathFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingVertex(recordDescriptor);

    auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    return algorithm::GraphTraversal::depthFirstSearch(
        txn, vertexClassInfo, recordDescriptor, minDepth, maxDepth, adapter::relation::Direction::OUT, pathFilter);
  }

  ResultSetCursor Traverse::outEdgeDfsCursor(const Txn &txn,
                                             const RecordDescriptor &recordDescriptor,
                                             unsigned int minDepth,
                                             unsigned int maxDepth,
                                             const GraphFilter &pathFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingVertex(recordDescriptor);

    auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    auto result = algorithm::GraphTraversal::depthFirstSearchRdesc(
        txn, vertexClassInfo, recordDescriptor, minDepth, maxDepth, adapter::relation::Direction::OUT, pathFilter);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSet Traverse::allEdgeDfs(const Txn &txn,
                                 const RecordDescriptor &recordDescriptor,
                                 unsigned int minDepth,
                                 unsigned int maxDepth,
                                 const GraphFilter &pathFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingVertex(recordDescriptor);

    auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    return algorithm::GraphTraversal::depthFirstSearch(
        txn, vertexClassInfo, recordDescriptor, minDepth, maxDepth, adapter::relation::Direction::ALL, pathFilter);
  }

  ResultSetCursor Traverse::allEdgeDfsCursor(const Txn &txn,
                                             const RecordDescriptor &recordDescriptor,
                                             unsigned int minDepth,
                                             unsigned int maxDepth,
                                             const GraphFilter &pathFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingVertex(recordDescriptor);

    auto vertexClassInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first, ClassType::VERTEX);
    auto result = algorithm::GraphTraversal::depthFirstSearchRdesc(
        txn, vertexClassInfo, recordDescriptor, minDepth, maxDepth, adapter::relation::Direction::ALL, pathFilter);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

  ResultSet Traverse::shortestPath(const Txn &txn,
                                   const RecordDescriptor &srcVertexRecordDescriptor,
                                   const RecordDescriptor &dstVertexRecordDescriptor,
                                   const GraphFilter &pathFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingSrcVertex(srcVertexRecordDescriptor)
        .isExistingDstVertex(dstVertexRecordDescriptor);

    auto srcVertexClassInfo = txn._iSchema->getValidClassInfo(srcVertexRecordDescriptor.rid.first, ClassType::VERTEX);
    auto dstVertexClassInfo = txn._iSchema->getValidClassInfo(dstVertexRecordDescriptor.rid.first, ClassType::VERTEX);
    return algorithm::GraphTraversal::bfsShortestPath(
        txn, srcVertexClassInfo, dstVertexClassInfo,
        srcVertexRecordDescriptor, dstVertexRecordDescriptor, pathFilter);
  }

  ResultSetCursor Traverse::shortestPathCursor(const Txn &txn,
                                               const RecordDescriptor &srcVertexRecordDescriptor,
                                               const RecordDescriptor &dstVertexRecordDescriptor,
                                               const GraphFilter &pathFilter) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isExistingSrcVertex(srcVertexRecordDescriptor)
        .isExistingDstVertex(dstVertexRecordDescriptor);

    auto srcVertexClassInfo = txn._iSchema->getValidClassInfo(srcVertexRecordDescriptor.rid.first, ClassType::VERTEX);
    auto dstVertexClassInfo = txn._iSchema->getValidClassInfo(dstVertexRecordDescriptor.rid.first, ClassType::VERTEX);
    auto result = algorithm::GraphTraversal::bfsShortestPathRdesc(
        txn, srcVertexClassInfo, dstVertexClassInfo,
        srcVertexRecordDescriptor, dstVertexRecordDescriptor, pathFilter);
    return std::move(ResultSetCursor{txn}.addMetadata(result));
  }

}
