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

#pragma once

#include <functional>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <stack>

#include "constant.hpp"
#include "lmdb_engine.hpp"
#include "parser.hpp"
#include "schema.hpp"
#include "datarecord.hpp"
#include "relation.hpp"

#include "nogdb_compare.h"
#include "nogdb_types.h"
#include "nogdb_txn.h"

namespace nogdb {

  namespace algorithm {

    class GraphTraversal {
    public:
      GraphTraversal() = delete;

      ~GraphTraversal() noexcept = delete;

      static ResultSet breadthFirstSearch(const Txn &txn,
                                          const schema::ClassAccessInfo &classInfo,
                                          const RecordDescriptor &recordDescriptor,
                                          unsigned int minDepth,
                                          unsigned int maxDepth,
                                          const adapter::relation::Direction &direction,
                                          const PathFilter &pathFilter) {
        const auto searchResultDescriptor = breadthFirstSearchRdesc(
            txn, classInfo, recordDescriptor, minDepth, maxDepth, direction, pathFilter);
        ResultSet result(searchResultDescriptor.size());
        std::transform(searchResultDescriptor.begin(), searchResultDescriptor.end(), result.begin(),
            [&txn](const RecordDescriptor &descriptor) {
              const auto edgeClassInfo = txn._class->getInfo(descriptor.rid.first);
              const auto &record = txn._iRecord->getRecord(edgeClassInfo, descriptor);
              record.setBasicInfo(DEPTH_PROPERTY, descriptor._depth);
              return Result(descriptor, record);
        });

        return result;
      }

      static std::vector<RecordDescriptor>
      breadthFirstSearchRdesc(const Txn &txn,
                              const schema::ClassAccessInfo &classInfo,
                              const RecordDescriptor &recordDescriptor,
                              unsigned int minDepth,
                              unsigned int maxDepth,
                              const adapter::relation::Direction &direction,
                              const PathFilter &pathFilter) {
        auto result = std::vector<RecordDescriptor>{};
        auto visited = std::unordered_set<RecordId, RecordIdHash>{recordDescriptor.rid};
        auto queue = std::queue<RecordId>{};
        auto currentLevel = 0U;
        auto firstRecordId = RecordId{recordDescriptor.rid};
        queue.push(recordDescriptor.rid);

        try {
          auto addUniqueVertex = [&](const RecordId &vertex, const PathFilter &pathFilter) {
            if (visited.find(vertex) != visited.cend()) return;
            auto rdesc = pathFilter.isEnable() ?
                         filterRecord(txn, RecordDescriptor{vertex}, pathFilter) : RecordDescriptor{vertex};
            if ((currentLevel + 1 >= minDepth) && (currentLevel + 1 <= maxDepth) && (rdesc != RecordDescriptor{})) {
              rdesc._depth = currentLevel + 1;
              result.emplace_back(rdesc);
            }
            visited.insert(vertex);
            if (firstRecordId == RecordId{}) {
              firstRecordId = vertex;
            }
            if ((currentLevel + 1 < maxDepth) && (rdesc != RecordDescriptor{})) {
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

            for (const auto &edge: filterIncidentEdges(txn, vertexId, direction, pathFilter)) {
              auto srcDstVertex = txn._iGraph->getSrcDstVertices(edge.rid);
              switch (direction) {
                case adapter::relation::Direction::IN:
                  addUniqueVertex(srcDstVertex.first, pathFilter);
                  break;
                case adapter::relation::Direction::OUT:
                  addUniqueVertex(srcDstVertex.second, pathFilter);
                  break;
                case adapter::relation::Direction::ALL:
                  addUniqueVertex(srcDstVertex.first != vertexId ? srcDstVertex.first : srcDstVertex.second,
                                  pathFilter);
                  break;
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

      static ResultSet depthFirstSearch(const Txn &txn,
                                        const schema::ClassAccessInfo &classInfo,
                                        const RecordDescriptor &recordDescriptor,
                                        unsigned int minDepth,
                                        unsigned int maxDepth,
                                        const adapter::relation::Direction &direction,
                                        const PathFilter &pathFilter) {
        const auto searchResultDescriptor = depthFirstSearchRdesc(
            txn, classInfo, recordDescriptor, minDepth, maxDepth, direction, pathFilter);
        ResultSet result(searchResultDescriptor.size());
        std::transform(searchResultDescriptor.begin(), searchResultDescriptor.end(), result.begin(),
            [&txn](const RecordDescriptor &descriptor) {
              const auto edgeClassInfo = txn._class->getInfo(descriptor.rid.first);
              const auto &record = txn._iRecord->getRecord(edgeClassInfo, descriptor);
              record.setBasicInfo(DEPTH_PROPERTY, descriptor._depth);
              return Result(descriptor, record);
        });

        return result;
      }

      static std::vector<RecordDescriptor>
      depthFirstSearchRdesc(const Txn &txn,
                            const schema::ClassAccessInfo &classInfo,
                            const RecordDescriptor &recordDescriptor,
                            unsigned int minDepth,
                            unsigned int maxDepth,
                            const adapter::relation::Direction &direction,
                            const PathFilter &pathFilter) {
        auto result = std::vector<RecordDescriptor>{};
        auto visited = std::unordered_set<RecordId, RecordIdHash>{};
        auto stack = std::vector<std::vector<RecordId>>{{recordDescriptor.rid}};
        auto currentLevel = 0U;
        try {
          while (!stack[currentLevel].empty()) {
            const RecordId vertexId = stack[currentLevel].back();
            stack[currentLevel].pop_back();

            if (visited.find(vertexId) == visited.cend()) {
              visited.insert(vertexId);

              if (currentLevel >= minDepth) {
                auto rdesc = pathFilter.isEnable() ?
                             filterRecord(txn, vertexId, currentLevel ? pathFilter : PathFilter{}) : RecordDescriptor{
                        vertexId};
                if (rdesc != RecordDescriptor{}) {
                  rdesc._depth = currentLevel;
                  result.emplace_back(rdesc);
                }
              }

              if (currentLevel < maxDepth) {
                ++currentLevel;
                if (currentLevel == stack.size()) {
                  stack.emplace_back(decltype(stack)::value_type());
                }

                const auto edgeRecordDescriptors = filterIncidentEdges(txn, vertexId, direction, pathFilter);
                for (auto it = edgeRecordDescriptors.crbegin(); it != edgeRecordDescriptors.crend(); ++it) {
                  const RecordDescriptor &edge = *it;
                  RecordId nextVertex;
                  auto srcDstVertex = txn._iGraph->getSrcDstVertices(edge.rid);
                  switch (direction) {
                    case adapter::relation::Direction::IN:
                      nextVertex = srcDstVertex.first;
                      break;
                    case adapter::relation::Direction::OUT:
                      nextVertex = srcDstVertex.second;
                      break;
                    case adapter::relation::Direction::ALL:
                      nextVertex = srcDstVertex.first != vertexId ? srcDstVertex.first : srcDstVertex.second;
                      break;
                  }
                  if (visited.find(nextVertex) != visited.cend()) continue;
                  stack[currentLevel].emplace_back(std::move(nextVertex));
                }
              }
            }

            while (currentLevel && stack[currentLevel].empty()) {
              --currentLevel;
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

      static ResultSet bfsShortestPath(const Txn &txn,
                                       const schema::ClassAccessInfo &srcVertexClassInfo,
                                       const schema::ClassAccessInfo &dstVertexClassInfo,
                                       const RecordDescriptor &srcVertexRecordDescriptor,
                                       const RecordDescriptor &dstVertexRecordDescriptor,
                                       const PathFilter &pathFilter) {
        const auto searchResultDescriptor = bfsShortestPathRdesc(
            txn, srcVertexClassInfo, dstVertexClassInfo,
            srcVertexRecordDescriptor, dstVertexRecordDescriptor, pathFilter);

        ResultSet result(searchResultDescriptor.size());
        std::transform(searchResultDescriptor.begin(), searchResultDescriptor.end(), result.begin(),
                       [&txn](const RecordDescriptor &descriptor) {
                         const auto edgeClassInfo = txn._class->getInfo(descriptor.rid.first);
                         const auto &record = txn._iRecord->getRecord(edgeClassInfo, descriptor);
                         record.setBasicInfo(DEPTH_PROPERTY, descriptor._depth);
                         return Result(descriptor, record);
                       });

        return result;
      }

      static std::vector<RecordDescriptor>
      bfsShortestPathRdesc(const Txn &txn,
                           const schema::ClassAccessInfo &srcVertexClassInfo,
                           const schema::ClassAccessInfo &dstVertexClassInfo,
                           const RecordDescriptor &srcVertexRecordDescriptor,
                           const RecordDescriptor &dstVertexRecordDescriptor,
                           const PathFilter &pathFilter) {
        auto result = std::vector<RecordDescriptor>{};
        try {
          if (srcVertexRecordDescriptor == dstVertexRecordDescriptor) {
            result.emplace_back(srcVertexRecordDescriptor);
          } else {
            auto found = false;
            auto visited = std::unordered_map<RecordId, std::pair<RecordDescriptor, RecordId>, RecordIdHash>{};
            visited.insert({srcVertexRecordDescriptor.rid, {RecordDescriptor{}, RecordId{}}});
            auto queue = std::queue<RecordId>{};
            queue.push(srcVertexRecordDescriptor.rid);
            while (!queue.empty() && !found) {
              auto vertex = queue.front();
              queue.pop();

              for (const auto &edge: filterIncidentEdges(txn, vertex, adapter::relation::Direction::OUT, pathFilter)) {
                auto nextVertex = txn._iGraph->getSrcDstVertices(edge.rid).second;
                if (visited.find(nextVertex) == visited.cend()) {
                  auto rdesc = (pathFilter.isSetVertex() || pathFilter.isSetEdge()) ?
                                  filterRecord(txn, nextVertex, pathFilter) : RecordDescriptor{nextVertex};
                  if (rdesc != RecordDescriptor{}) {
                    visited.insert({nextVertex, {rdesc, vertex}});
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
              for (auto &res: result) {
                res._depth = currentLevel++;
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

    private:

      struct RecordIdHash {
        inline uint64_t operator()(const std::pair<ClassId, PositionId> &rid) const {
          return (static_cast<uint64_t>(rid.first) << 32) + static_cast<uint64_t>(rid.second);
        }
      };

      static RecordDescriptor filterRecord(const Txn &txn,
                                           const RecordDescriptor &recordDescriptor,
                                           const PathFilter &pathFilter) {
        auto classInfo = txn._class->getInfo(recordDescriptor.rid.first);
        auto record = txn._iRecord->getRecord(classInfo, recordDescriptor);
        if (pathFilter.isSetVertex() && classInfo.type == ClassType::VERTEX) {
          if ((*pathFilter.vertexFilter)(record)) {
            return recordDescriptor;
          } else {
            return RecordDescriptor{};
          }
        } else if (pathFilter.isSetEdge() && classInfo.type == ClassType::EDGE) {
          if ((*pathFilter.edgeFilter)(record)) {
            return recordDescriptor;
          } else {
            return RecordDescriptor{};
          }
        } else {
          return recordDescriptor;
        }
      }

      static std::vector<RecordDescriptor>
      filterIncidentEdges(const Txn &txn,
                          const RecordId &vertex,
                          const adapter::relation::Direction &direction,
                          const PathFilter &pathFilter) {
        auto edgeRecordDescriptors = std::vector<RecordDescriptor>{};
        auto edgeNeighbours = std::vector<RecordId>{};
        switch (direction) {
          case adapter::relation::Direction::IN:
            edgeNeighbours = txn._iGraph->getInEdges(vertex);
            break;
          case adapter::relation::Direction::OUT:
            edgeNeighbours = txn._iGraph->getOutEdges(vertex);
            break;
          case adapter::relation::Direction::ALL:
            edgeNeighbours = txn._iGraph->getInEdges(vertex);
            auto moreEdges = txn._iGraph->getOutEdges(vertex);
            edgeNeighbours.insert(edgeNeighbours.cend(), moreEdges.cbegin(), moreEdges.cend());
            break;
        }

        for (const auto &edge: edgeNeighbours) {
          auto rdesc = (pathFilter.isSetVertex() || pathFilter.isSetEdge()) ?
                       filterRecord(txn, RecordDescriptor{edge}, pathFilter) : RecordDescriptor{edge};
          if (rdesc != RecordDescriptor{}) {
            edgeRecordDescriptors.emplace_back(RecordDescriptor{edge});
          }
        }

        return edgeRecordDescriptors;
      }

    };

  }
}
