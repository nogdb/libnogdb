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

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <queue>
#include <stack>

#include "datatype.hpp"
#include "storage_engine.hpp"
#include "lmdb_engine.hpp"
#include "schema.hpp"
#include "algorithm.hpp"

#include "nogdb_errors.h"
#include "nogdb_compare.h"

namespace nogdb {

    ResultSet Algorithm::breadthFirstSearch(const Txn &txn,
                                           const RecordDescriptor &recordDescriptor,
                                           const unsigned int minDepth,
                                           const unsigned int maxDepth,
                                           const std::vector<ClassId> &edgeClassIds,
                                           std::vector<RecordId>
                                           (Graph::*edgeFunc)(const BaseTxn &baseTxn, const RecordId &rid,
                                                              const ClassId &classId),
                                           RecordId (Graph::*vertexFunc)(const BaseTxn &baseTxn, const RecordId &rid),
                                           const PathFilter &pathFilter) {
        const auto searchResultDescriptor = breadthFirstSearchRdesc(txn, recordDescriptor, minDepth, maxDepth,
                                                                  edgeClassIds, edgeFunc, vertexFunc, pathFilter);

        ResultSet result (searchResultDescriptor.size());
        std::transform(searchResultDescriptor.begin(), searchResultDescriptor.end(), result.begin(),
                       [&txn] (const RecordDescriptor& descriptor) {
            const auto &record = retrieveRecord(txn, descriptor);
            record.setBasicInfo(DEPTH_PROPERTY, descriptor.depth);
            return Result(descriptor, record);
        });

        return result;
    }

    ResultSet Algorithm::depthFirstSearch(const Txn &txn,
                                          const RecordDescriptor &recordDescriptor,
                                          const unsigned int minDepth,
                                          const unsigned int maxDepth,
                                          const std::vector<ClassId> &edgeClassIds,
                                          std::vector<RecordId>
                                          (Graph::*edgeFunc)(const BaseTxn &baseTxn, const RecordId &rid,
                                                             const ClassId &classId),
                                          RecordId (Graph::*vertexFunc)(const BaseTxn &baseTxn, const RecordId &rid),
                                          const PathFilter &pathFilter) {
        const auto searchResultDescriptor = depthFirstSearchRdesc(txn, recordDescriptor, minDepth, maxDepth,
                                                                  edgeClassIds, edgeFunc, vertexFunc, pathFilter);
        ResultSet result (searchResultDescriptor.size());
        std::transform(searchResultDescriptor.begin(), searchResultDescriptor.end(), result.begin(),
                       [&txn] (const RecordDescriptor& descriptor) {
            const auto &record = retrieveRecord(txn, descriptor);
            record.setBasicInfo(DEPTH_PROPERTY, descriptor.depth);
            return Result(descriptor, record);
        });

        return result;
    }

    ResultSet Algorithm::bfsShortestPath(const Txn &txn,
                                         const RecordDescriptor &srcVertexRecordDescriptor,
                                         const RecordDescriptor &dstVertexRecordDescriptor,
                                         const std::vector<ClassId> &edgeClassIds,
                                         const PathFilter &pathFilter) {
        const auto searchResultDescriptor = bfsShortestPathRdesc(txn, srcVertexRecordDescriptor,
                                                                 dstVertexRecordDescriptor, edgeClassIds, pathFilter);

        ResultSet result (searchResultDescriptor.size());
        std::transform(searchResultDescriptor.begin(), searchResultDescriptor.end(), result.begin(),
                       [&txn] (const RecordDescriptor& descriptor) {
                           const auto &record = retrieveRecord(txn, descriptor);
                           record.setBasicInfo(DEPTH_PROPERTY, descriptor.depth);
                           return Result(descriptor, record);
                       });

        return result;
    }

    std::vector<RecordDescriptor>
    Algorithm::breadthFirstSearchRdesc(const Txn &txn,
                                      const RecordDescriptor &recordDescriptor,
                                      const unsigned int minDepth,
                                      const unsigned int maxDepth,
                                      const std::vector<ClassId> &edgeClassIds,
                                      std::vector<RecordId> (Graph::*edgeFunc)(const BaseTxn &baseTxn,
                                                                               const RecordId &rid,
                                                                               const ClassId &classId),
                                      RecordId (Graph::*vertexFunc)(const BaseTxn &baseTxn, const RecordId &rid),
                                      const PathFilter &pathFilter) {
        switch (Generic::checkIfRecordExist(txn, recordDescriptor)) {
            case RECORD_NOT_EXIST:
                throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
            case RECORD_NOT_EXIST_IN_MEMORY:
                return ((minDepth == 0) && (minDepth <= maxDepth)) ?
                       std::vector<RecordDescriptor>{recordDescriptor} : std::vector<RecordDescriptor>{};
            default:
                auto result = std::vector<RecordDescriptor>{};
                auto classDescriptor = Schema::ClassDescriptorPtr{};
                auto classPropertyInfo = ClassPropertyInfo{};
                auto classDBHandler = storage_engine::lmdb::Dbi{};
                auto visited = std::unordered_set<RecordId, Graph::RecordIdHash>{recordDescriptor.rid};
                auto queue = std::queue<RecordId> {};

                unsigned int currentLevel = 0u;
                RecordId firstRecordId{recordDescriptor.rid};

                queue.push(recordDescriptor.rid);

                try {
                    auto addUniqueVertex = [&](const RecordId &vertex, const PathFilter &pathFilter) {
                        if (visited.find(vertex) != visited.cend()) return;
                        auto tmpRdesc = pathFilter.isEnable() ?
                                        retrieveRdesc(txn, classDescriptor, classPropertyInfo,
                                                      classDBHandler, vertex, pathFilter, ClassType::VERTEX) :
                                        RecordDescriptor{vertex};
                        if ((currentLevel + 1 >= minDepth) && (currentLevel + 1 <= maxDepth) &&
                            (tmpRdesc != RecordDescriptor{})) {
                            tmpRdesc.depth = currentLevel + 1;
                            result.emplace_back(tmpRdesc);
                        }

                        visited.insert(vertex);

                        if (firstRecordId == RecordId{}) {
                            firstRecordId = vertex;
                        }

                        if ((currentLevel + 1 < maxDepth) && (tmpRdesc != RecordDescriptor{})) {
                            queue.push(vertex);
                        }
                    };

                    if (minDepth == 0) {
                        result.emplace_back(recordDescriptor);
                    }

                    while (!queue.empty()) {
                        auto vertexId = queue.front();
                        queue.pop();

                        if (vertexId == firstRecordId) {
                            currentLevel += (vertexId != recordDescriptor.rid);
                            firstRecordId = RecordId{};
                        }

                        const auto edgeRecordDescriptors = getIncidentEdges(txn, classDescriptor, classPropertyInfo,
                                                                            classDBHandler, edgeFunc, vertexId, pathFilter, edgeClassIds);

                        for (const auto &edge: edgeRecordDescriptors) {
                            if (vertexFunc != nullptr) {
                                addUniqueVertex(((*txn.txnCtx.dbRelation).*vertexFunc)(*(txn.txnBase), edge.rid), pathFilter);
                            } else {
                                auto vertices = txn.txnCtx.dbRelation->getVertexSrcDst(*(txn.txnBase), edge.rid);
                                addUniqueVertex(vertices.first != vertexId ? vertices.first : vertices.second, pathFilter);
                            }
                        }
                    }
                } catch (const Error &err) {
                    if (err.code() == NOGDB_GRAPH_NOEXST_VERTEX) {
                        throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_UNKNOWN_ERR);
                    } else {
                        throw err;
                    }
                }
                return result;
        }
    }

    std::vector<RecordDescriptor>
    Algorithm::depthFirstSearchRdesc(const Txn &txn,
                                     const RecordDescriptor &recordDescriptor,
                                     const unsigned int minDepth,
                                     const unsigned int maxDepth,
                                     const std::vector<ClassId> &edgeClassIds,
                                     std::vector<RecordId> (Graph::*edgeFunc)(const BaseTxn &baseTxn,
                                                                              const RecordId &rid,
                                                                              const ClassId &classId),
                                     RecordId (Graph::*vertexFunc)(const BaseTxn &baseTxn, const RecordId &rid),
                                     const PathFilter &pathFilter) {
        switch (Generic::checkIfRecordExist(txn, recordDescriptor)) {
            case RECORD_NOT_EXIST:
                throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
            case RECORD_NOT_EXIST_IN_MEMORY:
                return ((minDepth == 0) && (minDepth <= maxDepth)) ?
                       std::vector<RecordDescriptor>{recordDescriptor} : std::vector<RecordDescriptor>{};
            default:
                auto result = std::vector<RecordDescriptor>{};
                auto classDescriptor = Schema::ClassDescriptorPtr{};
                auto classPropertyInfo = ClassPropertyInfo{};
                auto classDBHandler = storage_engine::lmdb::Dbi{};
                try {

                    std::unordered_set<RecordId, Graph::RecordIdHash> visited {};
                    std::vector<std::vector<RecordId>> stk {{recordDescriptor.rid}};
                    unsigned int currentLevel = 0u;

                    while (!stk[currentLevel].empty()) {

                        const RecordId vertex = stk[currentLevel].back();
                        stk[currentLevel].pop_back();

                        if (visited.find(vertex) == visited.cend()) {
                            visited.insert(vertex);

                            if (currentLevel >= minDepth) {
                                auto tmpRdesc = pathFilter.isEnable() ?
                                                retrieveRdesc(txn, classDescriptor, classPropertyInfo,
                                                              classDBHandler, vertex,
                                                              currentLevel ? pathFilter : PathFilter{},
                                                              ClassType::VERTEX) :
                                                RecordDescriptor{vertex};

                                if (tmpRdesc != RecordDescriptor{}) {
                                    tmpRdesc.depth = currentLevel;
                                    result.push_back(tmpRdesc);
                                }
                            }

                            if (currentLevel < maxDepth) {

                                ++currentLevel;
                                if (currentLevel == stk.size()) {
                                    stk.emplace_back(decltype(stk)::value_type());
                                }

                                const auto edgeRecordDescriptors = getIncidentEdges(txn, classDescriptor,
                                                                                    classPropertyInfo,
                                                                                    classDBHandler, edgeFunc, vertex,
                                                                                    pathFilter,
                                                                                    edgeClassIds);

                                for (auto it = edgeRecordDescriptors.crbegin(); it != edgeRecordDescriptors.crend(); ++it) {
                                    const RecordDescriptor &edge = *it;

                                    RecordId nextVertex;
                                    if (vertexFunc != nullptr) {
                                        nextVertex = ((*txn.txnCtx.dbRelation).*vertexFunc)(*(txn.txnBase), edge.rid);
                                    } else {
                                        auto vertices = txn.txnCtx.dbRelation->getVertexSrcDst(*(txn.txnBase), edge.rid);
                                        nextVertex = (vertices.first != vertex ? vertices.first : vertices.second);
                                    }

                                    if (visited.find(nextVertex) != visited.cend()) continue;

                                    stk[currentLevel].emplace_back(std::move(nextVertex));
                                }
                            }
                        }

                        while (currentLevel && stk[currentLevel].empty()) {
                            --currentLevel;
                        }
                    }
                } catch (Graph::ErrorType &err) {
                    if (err == NOGDB_GRAPH_NOEXST_VERTEX) {
                        throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_UNKNOWN_ERR);
                    } else {
                        throw err;
                    }
                }
                return result;
        }
    }

    std::vector<RecordDescriptor>
    Algorithm::bfsShortestPathRdesc(const Txn &txn,
                                    const RecordDescriptor &srcVertexRecordDescriptor,
                                    const RecordDescriptor &dstVertexRecordDescriptor,
                                    const std::vector<ClassId> &edgeClassIds,
                                    const PathFilter &pathFilter) {
        auto srcStatus = Generic::checkIfRecordExist(txn, srcVertexRecordDescriptor);
        auto dstStatus = Generic::checkIfRecordExist(txn, dstVertexRecordDescriptor);
        if (srcStatus == RECORD_NOT_EXIST) {
            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_SRC);
        } else if (dstStatus == RECORD_NOT_EXIST) {
            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_DST);
        } else if (srcStatus == RECORD_NOT_EXIST_IN_MEMORY || dstStatus == RECORD_NOT_EXIST_IN_MEMORY) {
            return std::vector<RecordDescriptor>{};
        } else {
            auto result = std::vector<RecordDescriptor>{};
            auto classDescriptor = Schema::ClassDescriptorPtr{};
            auto classPropertyInfo = ClassPropertyInfo{};
            auto classDBHandler = storage_engine::lmdb::Dbi{};
            try {
                if (srcVertexRecordDescriptor == dstVertexRecordDescriptor) {
                    result.emplace_back(srcVertexRecordDescriptor);
                } else {
                    bool found = false;
                    auto visited = std::unordered_map<RecordId, std::pair<RecordDescriptor, RecordId>, Graph::RecordIdHash> {};
                    visited.insert({srcVertexRecordDescriptor.rid, {RecordDescriptor{}, RecordId{}}});
                    auto queue = std::queue<RecordId> {};
                    queue.push(srcVertexRecordDescriptor.rid);
                    while (!queue.empty() && !found) {
                        auto vertex = queue.front();
                        queue.pop();

                        const auto edgeRecordDescriptors = getOutEdges(txn, classDescriptor, classPropertyInfo,
                                                                            classDBHandler, vertex, pathFilter, edgeClassIds);

                        for (const auto &edge: edgeRecordDescriptors) {
                            auto nextVertex = txn.txnCtx.dbRelation->getVertexDst(*(txn.txnBase), edge.rid);
                            if (visited.find(nextVertex) == visited.cend()) {
                                auto tmpRdesc = (pathFilter.isSetVertex() || pathFilter.isSetEdge()) ?
                                                retrieveRdesc(txn, classDescriptor, classPropertyInfo,
                                                              classDBHandler, nextVertex, pathFilter, ClassType::VERTEX)
                                                                                                     : RecordDescriptor{nextVertex};
                                if (tmpRdesc != RecordDescriptor{}) {
                                    visited.insert({nextVertex, {tmpRdesc, vertex}});
                                    queue.push(nextVertex);
                                }
                            }
                            if (nextVertex == dstVertexRecordDescriptor.rid) {
                                found = true;
                                break;
                            }
                        }
                    }

                    if (found) {
                        auto vertex = dstVertexRecordDescriptor.rid;
                        while (vertex != srcVertexRecordDescriptor.rid) {
                            auto data = visited.at(vertex);
                            result.emplace_back(data.first);
                            vertex = data.second;
                        }
                        result.emplace_back(srcVertexRecordDescriptor);
                        std::reverse(result.begin(), result.end());
                        auto currentLevel = 0U;
                        for(auto& res: result) {
                            res.depth = currentLevel++;
                        }
                    }
                }
            } catch (const Error &err) {
                if (err.code() == NOGDB_GRAPH_NOEXST_VERTEX) {
                    throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_UNKNOWN_ERR);
                } else {
                    throw err;
                }
            }
            return result;
        }
    }

}


