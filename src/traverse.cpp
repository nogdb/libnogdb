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

#include "shared_lock.hpp"
#include "schema.hpp"
#include "env_handler.hpp"
#include "lmdb_engine.hpp"
#include "graph.hpp"
#include "algorithm.hpp"
#include "generic.hpp"

#include "nogdb.h"

namespace nogdb {

    ResultSet Traverse::inEdgeBfs(const Txn &txn,
                                  const RecordDescriptor &recordDescriptor,
                                  unsigned int minDepth,
                                  unsigned int maxDepth,
                                  const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Algorithm::breadthFirstSearch(txn,
                                            recordDescriptor,
                                            minDepth,
                                            maxDepth,
                                            edgeClassIds,
                                            &Graph::getEdgeIn,
                                            &Graph::getVertexSrc,
                                            PathFilter{});
    }

    ResultSet Traverse::inEdgeBfs(const Txn &txn,
                                  const RecordDescriptor &recordDescriptor,
                                  unsigned int minDepth,
                                  unsigned int maxDepth,
                                  const PathFilter &pathFilter,
                                  const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Algorithm::breadthFirstSearch(txn,
                                            recordDescriptor,
                                            minDepth,
                                            maxDepth,
                                            edgeClassIds,
                                            &Graph::getEdgeIn,
                                            &Graph::getVertexSrc,
                                            pathFilter);
    }

    ResultSetCursor Traverse::inEdgeBfsCursor(Txn &txn,
                                              const RecordDescriptor &recordDescriptor,
                                              unsigned int minDepth,
                                              unsigned int maxDepth,
                                              const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Algorithm::breadthFirstSearchRdesc(txn,
                                                          recordDescriptor,
                                                          minDepth,
                                                          maxDepth,
                                                          edgeClassIds,
                                                          &Graph::getEdgeIn,
                                                          &Graph::getVertexSrc,
                                                          PathFilter{});
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Traverse::inEdgeBfsCursor(Txn &txn,
                                              const RecordDescriptor &recordDescriptor,
                                              unsigned int minDepth,
                                              unsigned int maxDepth,
                                              const PathFilter &pathFilter,
                                              const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Algorithm::breadthFirstSearchRdesc(txn,
                                                          recordDescriptor,
                                                          minDepth,
                                                          maxDepth,
                                                          edgeClassIds,
                                                          &Graph::getEdgeIn,
                                                          &Graph::getVertexSrc,
                                                          pathFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }


    ResultSet Traverse::outEdgeBfs(const Txn &txn,
                                   const RecordDescriptor &recordDescriptor,
                                   unsigned int minDepth,
                                   unsigned int maxDepth,
                                   const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Algorithm::breadthFirstSearch(txn,
                                            recordDescriptor,
                                            minDepth,
                                            maxDepth,
                                            edgeClassIds,
                                            &Graph::getEdgeOut,
                                            &Graph::getVertexDst,
                                            PathFilter{});
    }

    ResultSet Traverse::outEdgeBfs(const Txn &txn,
                                   const RecordDescriptor &recordDescriptor,
                                   unsigned int minDepth,
                                   unsigned int maxDepth,
                                   const PathFilter &pathFilter,
                                   const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Algorithm::breadthFirstSearch(txn,
                                            recordDescriptor,
                                            minDepth,
                                            maxDepth,
                                            edgeClassIds,
                                            &Graph::getEdgeOut,
                                            &Graph::getVertexDst,
                                            pathFilter);
    }

    ResultSetCursor Traverse::outEdgeBfsCursor(Txn &txn,
                                               const RecordDescriptor &recordDescriptor,
                                               unsigned int minDepth,
                                               unsigned int maxDepth,
                                               const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Algorithm::breadthFirstSearchRdesc(txn,
                                                          recordDescriptor,
                                                          minDepth,
                                                          maxDepth,
                                                          edgeClassIds,
                                                          &Graph::getEdgeOut,
                                                          &Graph::getVertexDst,
                                                          PathFilter{});
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;

    }

    ResultSetCursor Traverse::outEdgeBfsCursor(Txn &txn,
                                               const RecordDescriptor &recordDescriptor,
                                               unsigned int minDepth,
                                               unsigned int maxDepth,
                                               const PathFilter &pathFilter,
                                               const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Algorithm::breadthFirstSearchRdesc(txn,
                                                          recordDescriptor,
                                                          minDepth,
                                                          maxDepth,
                                                          edgeClassIds,
                                                          &Graph::getEdgeOut,
                                                          &Graph::getVertexDst,
                                                          pathFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSet Traverse::allEdgeBfs(const Txn &txn,
                                   const RecordDescriptor &recordDescriptor,
                                   unsigned int minDepth,
                                   unsigned int maxDepth,
                                   const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Algorithm::breadthFirstSearch(txn,
                                            recordDescriptor,
                                            minDepth,
                                            maxDepth,
                                            edgeClassIds,
                                            &Graph::getEdgeInOut,
                                            nullptr,
                                            PathFilter{});
    }

    ResultSet Traverse::allEdgeBfs(const Txn &txn,
                                   const RecordDescriptor &recordDescriptor,
                                   unsigned int minDepth,
                                   unsigned int maxDepth,
                                   const PathFilter &pathFilter,
                                   const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Algorithm::breadthFirstSearch(txn,
                                            recordDescriptor,
                                            minDepth,
                                            maxDepth,
                                            edgeClassIds,
                                            &Graph::getEdgeInOut,
                                            nullptr,
                                            pathFilter);
    }

    ResultSetCursor Traverse::allEdgeBfsCursor(Txn &txn,
                                               const RecordDescriptor &recordDescriptor,
                                               unsigned int minDepth,
                                               unsigned int maxDepth,
                                               const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Algorithm::breadthFirstSearchRdesc(txn,
                                                          recordDescriptor,
                                                          minDepth,
                                                          maxDepth,
                                                          edgeClassIds,
                                                          &Graph::getEdgeInOut,
                                                          nullptr,
                                                          PathFilter{});
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Traverse::allEdgeBfsCursor(Txn &txn,
                                               const RecordDescriptor &recordDescriptor,
                                               unsigned int minDepth,
                                               unsigned int maxDepth,
                                               const PathFilter &pathFilter,
                                               const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Algorithm::breadthFirstSearchRdesc(txn,
                                                          recordDescriptor,
                                                          minDepth,
                                                          maxDepth,
                                                          edgeClassIds,
                                                          &Graph::getEdgeInOut,
                                                          nullptr,
                                                          pathFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSet Traverse::inEdgeDfs(const Txn &txn,
                                  const RecordDescriptor &recordDescriptor,
                                  unsigned int minDepth,
                                  unsigned int maxDepth,
                                  const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Algorithm::depthFirstSearch(txn,
                                           recordDescriptor,
                                           minDepth,
                                           maxDepth,
                                           edgeClassIds,
                                           &Graph::getEdgeIn,
                                           &Graph::getVertexSrc,
                                           PathFilter{});
    }

    ResultSet Traverse::inEdgeDfs(const Txn &txn,
                                  const RecordDescriptor &recordDescriptor,
                                  unsigned int minDepth,
                                  unsigned int maxDepth,
                                  const PathFilter &pathFilter,
                                  const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Algorithm::depthFirstSearch(txn,
                                           recordDescriptor,
                                           minDepth,
                                           maxDepth,
                                           edgeClassIds,
                                           &Graph::getEdgeIn,
                                           &Graph::getVertexSrc,
                                           pathFilter);
    }

    ResultSetCursor Traverse::inEdgeDfsCursor(Txn &txn,
                                              const RecordDescriptor &recordDescriptor,
                                              unsigned int minDepth,
                                              unsigned int maxDepth,
                                              const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Algorithm::depthFirstSearchRdesc(txn,
                                                         recordDescriptor,
                                                         minDepth,
                                                         maxDepth,
                                                         edgeClassIds,
                                                         &Graph::getEdgeIn,
                                                         &Graph::getVertexSrc,
                                                         PathFilter{});
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Traverse::inEdgeDfsCursor(Txn &txn,
                                              const RecordDescriptor &recordDescriptor,
                                              unsigned int minDepth,
                                              unsigned int maxDepth,
                                              const PathFilter &pathFilter,
                                              const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Algorithm::depthFirstSearchRdesc(txn,
                                                         recordDescriptor,
                                                         minDepth,
                                                         maxDepth,
                                                         edgeClassIds,
                                                         &Graph::getEdgeIn,
                                                         &Graph::getVertexSrc,
                                                         pathFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSet Traverse::outEdgeDfs(const Txn &txn,
                                   const RecordDescriptor &recordDescriptor,
                                   unsigned int minDepth,
                                   unsigned int maxDepth,
                                   const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Algorithm::depthFirstSearch(txn,
                                           recordDescriptor,
                                           minDepth,
                                           maxDepth,
                                           edgeClassIds,
                                           &Graph::getEdgeOut,
                                           &Graph::getVertexDst,
                                           PathFilter{});
    }

    ResultSet Traverse::outEdgeDfs(const Txn &txn,
                                   const RecordDescriptor &recordDescriptor,
                                   unsigned int minDepth,
                                   unsigned int maxDepth,
                                   const PathFilter &pathFilter,
                                   const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Algorithm::depthFirstSearch(txn,
                                           recordDescriptor,
                                           minDepth,
                                           maxDepth,
                                           edgeClassIds,
                                           &Graph::getEdgeOut,
                                           &Graph::getVertexDst,
                                           pathFilter);
    }

    ResultSetCursor Traverse::outEdgeDfsCursor(Txn &txn,
                                               const RecordDescriptor &recordDescriptor,
                                               unsigned int minDepth,
                                               unsigned int maxDepth,
                                               const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Algorithm::depthFirstSearchRdesc(txn,
                                                         recordDescriptor,
                                                         minDepth,
                                                         maxDepth,
                                                         edgeClassIds,
                                                         &Graph::getEdgeOut,
                                                         &Graph::getVertexDst,
                                                         PathFilter{});
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Traverse::outEdgeDfsCursor(Txn &txn,
                                               const RecordDescriptor &recordDescriptor,
                                               unsigned int minDepth,
                                               unsigned int maxDepth,
                                               const PathFilter &pathFilter,
                                               const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Algorithm::depthFirstSearchRdesc(txn,
                                                         recordDescriptor,
                                                         minDepth,
                                                         maxDepth,
                                                         edgeClassIds,
                                                         &Graph::getEdgeOut,
                                                         &Graph::getVertexDst,
                                                         pathFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSet Traverse::allEdgeDfs(const Txn &txn,
                                   const RecordDescriptor &recordDescriptor,
                                   unsigned int minDepth,
                                   unsigned int maxDepth,
                                   const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Algorithm::depthFirstSearch(txn,
                                           recordDescriptor,
                                           minDepth,
                                           maxDepth,
                                           edgeClassIds,
                                           &Graph::getEdgeInOut,
                                           nullptr,
                                           PathFilter{});
    }

    ResultSet Traverse::allEdgeDfs(const Txn &txn,
                                   const RecordDescriptor &recordDescriptor,
                                   unsigned int minDepth,
                                   unsigned int maxDepth,
                                   const PathFilter &pathFilter,
                                   const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Algorithm::depthFirstSearch(txn,
                                           recordDescriptor,
                                           minDepth,
                                           maxDepth,
                                           edgeClassIds,
                                           &Graph::getEdgeInOut,
                                           nullptr,
                                           pathFilter);
    }

    ResultSetCursor Traverse::allEdgeDfsCursor(Txn &txn,
                                               const RecordDescriptor &recordDescriptor,
                                               unsigned int minDepth,
                                               unsigned int maxDepth,
                                               const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Algorithm::depthFirstSearchRdesc(txn,
                                                         recordDescriptor,
                                                         minDepth,
                                                         maxDepth,
                                                         edgeClassIds,
                                                         &Graph::getEdgeInOut,
                                                         nullptr,
                                                         PathFilter{});
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Traverse::allEdgeDfsCursor(Txn &txn,
                                               const RecordDescriptor &recordDescriptor,
                                               unsigned int minDepth,
                                               unsigned int maxDepth,
                                               const PathFilter &pathFilter,
                                               const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Algorithm::depthFirstSearchRdesc(txn,
                                                         recordDescriptor,
                                                         minDepth,
                                                         maxDepth,
                                                         edgeClassIds,
                                                         &Graph::getEdgeInOut,
                                                         nullptr,
                                                         pathFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSet Traverse::shortestPath(const Txn &txn,
                                     const RecordDescriptor &srcVertexRecordDescriptor,
                                     const RecordDescriptor &dstVertexRecordDescriptor,
                                     const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, srcVertexRecordDescriptor.rid.first, ClassType::VERTEX);
        Generic::getClassDescriptor(txn, dstVertexRecordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Algorithm::bfsShortestPath(txn, srcVertexRecordDescriptor, dstVertexRecordDescriptor, edgeClassIds,
                                          PathFilter{});
    }

    ResultSet Traverse::shortestPath(const Txn &txn,
                                     const RecordDescriptor &srcVertexRecordDescriptor,
                                     const RecordDescriptor &dstVertexRecordDescriptor,
                                     const PathFilter &pathFilter,
                                     const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, srcVertexRecordDescriptor.rid.first, ClassType::VERTEX);
        Generic::getClassDescriptor(txn, dstVertexRecordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        return Algorithm::bfsShortestPath(txn, srcVertexRecordDescriptor, dstVertexRecordDescriptor, edgeClassIds,
                                          pathFilter);
    }

    ResultSetCursor Traverse::shortestPathCursor(Txn &txn,
                                                 const RecordDescriptor &srcVertexRecordDescriptor,
                                                 const RecordDescriptor &dstVertexRecordDescriptor,
                                                 const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, srcVertexRecordDescriptor.rid.first, ClassType::VERTEX);
        Generic::getClassDescriptor(txn, dstVertexRecordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Algorithm::bfsShortestPathRdesc(txn, srcVertexRecordDescriptor, dstVertexRecordDescriptor,
                                                        edgeClassIds, PathFilter{});
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

    ResultSetCursor Traverse::shortestPathCursor(Txn &txn,
                                                 const RecordDescriptor &srcVertexRecordDescriptor,
                                                 const RecordDescriptor &dstVertexRecordDescriptor,
                                                 const PathFilter &pathFilter,
                                                 const ClassFilter &classFilter) {
        Generic::getClassDescriptor(txn, srcVertexRecordDescriptor.rid.first, ClassType::VERTEX);
        Generic::getClassDescriptor(txn, dstVertexRecordDescriptor.rid.first, ClassType::VERTEX);
        auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
        auto result = ResultSetCursor{txn};
        auto metadata = Algorithm::bfsShortestPathRdesc(txn, srcVertexRecordDescriptor, dstVertexRecordDescriptor,
                                                        edgeClassIds, pathFilter);
        result.metadata.insert(result.metadata.end(), metadata.cbegin(), metadata.cend());
        return result;
    }

}
