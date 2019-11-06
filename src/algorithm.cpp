/*
 *  Copyright (C) 2019, NogDB <https://nogdb.org>
 *  <nogdb at throughwave dot co dot th>
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

#include "algorithm.hpp"

namespace nogdb {
namespace algorithm {
    using namespace adapter::schema;
    using namespace adapter::relation;
    using namespace datarecord;
    using compare::RecordCompare;
    using compare::ClassFilter;

    ResultSet GraphTraversal::breadthFirstSearch(const Transaction& txn,
        const std::set<RecordDescriptor>& recordDescriptors,
        unsigned int minDepth,
        unsigned int maxDepth,
        const Direction& direction,
        const GraphFilter& edgeFilter,
        const GraphFilter& vertexFilter)
    {
        const auto searchResultDescriptor = breadthFirstSearchRdesc(
            txn, recordDescriptors, minDepth, maxDepth, direction, edgeFilter, vertexFilter);
        ResultSet result(searchResultDescriptor.size());
        std::transform(searchResultDescriptor.begin(), searchResultDescriptor.end(), result.begin(),
            [&txn](const RecordDescriptor& descriptor) {
                const auto classInfo = txn._adapter->dbClass()->getInfo(descriptor.rid.first);
                const auto& record = DataRecordUtils::getRecordWithBasicInfo(&txn, classInfo, descriptor);
                record.setBasicInfo(DEPTH_PROPERTY, descriptor._depth);
                return Result(descriptor, record);
            });

        return result;
    }

    std::vector<RecordDescriptor> GraphTraversal::breadthFirstSearchRdesc(const Transaction& txn,
        const std::set<RecordDescriptor>& recordDescriptors,
        unsigned int minDepth,
        unsigned int maxDepth,
        const Direction& direction,
        const GraphFilter& edgeFilter,
        const GraphFilter& vertexFilter)
    {
        auto result = std::vector<RecordDescriptor> {};
        auto visited = std::unordered_set<RecordId, RecordIdHash> {};
        auto queue = std::queue<std::pair<RecordDescriptor, unsigned int>> {};
        for(const auto& recordDescriptor: recordDescriptors) {
            visited.insert(recordDescriptor.rid);
            queue.push(std::make_pair(recordDescriptor, 0));
        }

        try {
            auto edgeClassFilter = RecordCompare::getFilterClasses(txn, edgeFilter);
            auto vertexClassFilter = RecordCompare::getFilterClasses(txn, vertexFilter);
            auto addUniqueVertex = [&](const std::pair<RecordDescriptor, unsigned int>& nextVertexInfo) {
                auto currentVertex = nextVertexInfo.first;
                auto currentLevel = nextVertexInfo.second;
                if (visited.find(currentVertex.rid) != visited.cend())
                    return;
                auto vertexRdesc = RecordCompare::filterRecord(txn, currentVertex, vertexFilter, vertexClassFilter);
                if ((currentLevel >= minDepth) && (currentLevel <= maxDepth) && (vertexRdesc != RecordDescriptor {})) {
                    vertexRdesc._depth = currentLevel;
                    result.emplace_back(vertexRdesc);
                }
                visited.insert(currentVertex.rid);
                if ((currentLevel < maxDepth) && (vertexRdesc != RecordDescriptor {})) {
                    queue.push(std::make_pair(currentVertex.rid, currentLevel));
                }
            };

            if (minDepth == 0) {
                result.assign(recordDescriptors.cbegin(), recordDescriptors.cend());
            }
            while (!queue.empty()) {
                auto vertex = queue.front();
                queue.pop();
                for (const auto& edgeNeighbour :
                    RecordCompare::filterIncidentEdges(txn, vertex.first.rid, direction, edgeFilter, edgeClassFilter)) {
                    addUniqueVertex(std::make_pair(edgeNeighbour.second, vertex.second + 1));
                }
            }
        } catch (const Error& err) {
            if (err.code() == NOGDB_GRAPH_NOEXST_VERTEX) {
                throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_UNKNOWN_ERR);
            } else {
                throw NOGDB_FATAL_ERROR(err);
            }
        }
        return result;
    }

    ResultSet GraphTraversal::bfsShortestPath(const Transaction& txn,
        const RecordDescriptor& srcVertexRecordDescriptor,
        const RecordDescriptor& dstVertexRecordDescriptor,
        const GraphFilter& edgeFilter,
        const GraphFilter& vertexFilter)
    {
        const auto searchResultDescriptor = bfsShortestPathRdesc(
            txn, srcVertexRecordDescriptor, dstVertexRecordDescriptor, edgeFilter, vertexFilter);

        ResultSet result(searchResultDescriptor.size());
        std::transform(searchResultDescriptor.begin(), searchResultDescriptor.end(), result.begin(),
            [&txn](const RecordDescriptor& descriptor) {
                const auto classInfo = txn._adapter->dbClass()->getInfo(descriptor.rid.first);
                const auto& record = DataRecordUtils::getRecordWithBasicInfo(&txn, classInfo, descriptor);
                record.setBasicInfo(DEPTH_PROPERTY, descriptor._depth);
                return Result(descriptor, record);
            });

        return result;
    }

    std::vector<RecordDescriptor> GraphTraversal::bfsShortestPathRdesc(const Transaction& txn,
        const RecordDescriptor& srcVertexRecordDescriptor,
        const RecordDescriptor& dstVertexRecordDescriptor,
        const GraphFilter& edgeFilter,
        const GraphFilter& vertexFilter)
    {
        auto result = std::vector<RecordDescriptor> {};
        try {
            if (srcVertexRecordDescriptor == dstVertexRecordDescriptor) {
                result.emplace_back(srcVertexRecordDescriptor);
            } else {
                auto edgeClassFilter = RecordCompare::getFilterClasses(txn, edgeFilter);
                auto vertexClassFilter = RecordCompare::getFilterClasses(txn, vertexFilter);
                auto found = false;
                auto visited = std::unordered_map<RecordId, std::pair<RecordDescriptor, RecordId>, RecordIdHash> {};
                visited.insert({ srcVertexRecordDescriptor.rid, { RecordDescriptor {}, RecordId {} } });
                auto queue = std::queue<RecordId> {};
                queue.push(srcVertexRecordDescriptor.rid);
                while (!queue.empty() && !found) {
                    auto vertex = queue.front();
                    queue.pop();

                    for (const auto& edgeNeighbour :
                        RecordCompare::filterIncidentEdges(txn, vertex, Direction::OUT, edgeFilter, edgeClassFilter)) {
                        auto nextVertex = edgeNeighbour.second.rid;
                        if (visited.find(nextVertex) == visited.cend()) {
                            auto vertexRdesc = RecordCompare::filterRecord(
                                txn, nextVertex, vertexFilter, vertexClassFilter);
                            if (vertexRdesc != RecordDescriptor {}) {
                                visited.insert({ nextVertex, { vertexRdesc, vertex } });
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
                    for (auto& res : result) {
                        res._depth = currentLevel++;
                    }
                }
            }
        } catch (const Error& err) {
            if (err.code() == NOGDB_GRAPH_NOEXST_VERTEX) {
                throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_UNKNOWN_ERR);
            } else {
                throw NOGDB_FATAL_ERROR(err);
            }
        }
        return result;
    }
}
}
