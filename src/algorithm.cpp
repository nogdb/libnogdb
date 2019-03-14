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

using adapter::schema::ClassAccessInfo;
using adapter::relation::Direction;
using compare::RecordCompare;
using compare::ClassFilter;

namespace algorithm {


    ResultSet
    GraphTraversal::breadthFirstSearch(const Transaction& txn,
        const ClassAccessInfo& classInfo,
        const RecordDescriptor& recordDescriptor,
        unsigned int minDepth,
        unsigned int maxDepth,
        const Direction& direction,
        const GraphFilter& edgeFilter,
        const GraphFilter& vertexFilter)
    {
        const auto searchResultDescriptor = breadthFirstSearchRdesc(
            txn, classInfo, recordDescriptor, minDepth, maxDepth, direction, edgeFilter, vertexFilter);
        ResultSet result(searchResultDescriptor.size());
        std::transform(searchResultDescriptor.begin(), searchResultDescriptor.end(), result.begin(),
            [&txn](const RecordDescriptor& descriptor) {
                const auto edgeClassInfo = txn._adapter->dbClass()->getInfo(descriptor.rid.first);
                const auto& record = txn._interface->record()->getRecordWithBasicInfo(edgeClassInfo, descriptor);
                record.setBasicInfo(DEPTH_PROPERTY, descriptor._depth);
                return Result(descriptor, record);
            });

        return result;
    }

    std::vector<RecordDescriptor>
    GraphTraversal::breadthFirstSearchRdesc(const Transaction& txn,
        const ClassAccessInfo& classInfo,
        const RecordDescriptor& recordDescriptor,
        unsigned int minDepth,
        unsigned int maxDepth,
        const Direction& direction,
        const GraphFilter& edgeFilter,
        const GraphFilter& vertexFilter)
    {
        auto result = std::vector<RecordDescriptor> {};
        auto visited = std::unordered_set<RecordId, RecordIdHash> { recordDescriptor.rid };
        auto queue = std::queue<RecordId> {};
        auto currentLevel = 0U;
        auto firstRecordId = RecordId { recordDescriptor.rid };
        queue.push(recordDescriptor.rid);

        try {
            auto edgeClassFilter = RecordCompare::getFilterClasses(txn, edgeFilter);
            auto vertexClassFilter = RecordCompare::getFilterClasses(txn, vertexFilter);
            auto addUniqueVertex = [&](const RecordId& vertex) {
                if (visited.find(vertex) != visited.cend())
                    return;
                auto vertexRdesc = RecordCompare::filterRecord(
                    txn, RecordDescriptor { vertex }, vertexFilter, vertexClassFilter);
                if ((currentLevel + 1 >= minDepth) && (currentLevel + 1 <= maxDepth) && (vertexRdesc != RecordDescriptor {})) {
                    vertexRdesc._depth = currentLevel + 1;
                    result.emplace_back(vertexRdesc);
                }
                visited.insert(vertex);
                if (firstRecordId == RecordId {}) {
                    firstRecordId = vertex;
                }
                if ((currentLevel + 1 < maxDepth) && (vertexRdesc != RecordDescriptor {})) {
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
                    firstRecordId = RecordId {};
                }

                for (const auto& edgeNeighbour :
                    RecordCompare::filterIncidentEdges(txn, vertexId, direction, edgeFilter, edgeClassFilter)) {
                    addUniqueVertex(edgeNeighbour.second.rid);
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

    //TODO: DO NOT USE UNTIL "depthFirstSearchRdesc" IS FIXED
//    ResultSet
//    GraphTraversal::depthFirstSearch(const Transaction& txn,
//        const ClassAccessInfo& classInfo,
//        const RecordDescriptor& recordDescriptor,
//        unsigned int minDepth,
//        unsigned int maxDepth,
//        const Direction& direction,
//        const GraphFilter& edgeFilter,
//        const GraphFilter& vertexFilter)
//    {
//        const auto searchResultDescriptor = depthFirstSearchRdesc(
//            txn, classInfo, recordDescriptor, minDepth, maxDepth, direction, edgeFilter, vertexFilter);
//        ResultSet result(searchResultDescriptor.size());
//        std::transform(searchResultDescriptor.begin(), searchResultDescriptor.end(), result.begin(),
//            [&txn](const RecordDescriptor& descriptor) {
//                const auto edgeClassInfo = txn._adapter->dbClass()->getInfo(descriptor.rid.first);
//                const auto& record = txn._interface->record()->getRecordWithBasicInfo(edgeClassInfo, descriptor);
//                record.setBasicInfo(DEPTH_PROPERTY, descriptor._depth);
//                return Result(descriptor, record);
//            });
//
//        return result;
//    }

    //TODO: a buggy version of DFS, please fix this... DO NOT USE UNTIL IT IS FIXED
//    std::vector<RecordDescriptor>
//    GraphTraversal::depthFirstSearchRdesc(const Transaction& txn,
//        const ClassAccessInfo& classInfo,
//        const RecordDescriptor& recordDescriptor,
//        unsigned int minDepth,
//        unsigned int maxDepth,
//        const Direction& direction,
//        const GraphFilter& edgeFilter,
//        const GraphFilter& vertexFilter)
//    {
//        auto result = std::vector<RecordDescriptor> {};
//        auto visited = std::unordered_set<RecordId, RecordIdHash> {};
//        auto stack = std::vector<std::vector<RecordId>> { { recordDescriptor.rid } };
//        auto currentLevel = 0U;
//        try {
//            auto edgeClassFilter = RecordCompare::getFilterClasses(txn, edgeFilter);
//            auto vertexClassFilter = RecordCompare::getFilterClasses(txn, vertexFilter);
//            while (!stack[currentLevel].empty()) {
//                const RecordId vertexId = stack[currentLevel].back();
//                stack[currentLevel].pop_back();
//
//                if (visited.find(vertexId) == visited.cend()) {
//                    visited.insert(vertexId);
//
//                    if (currentLevel >= minDepth) {
//                        auto vertexRdesc = RecordCompare::filterRecord(
//                            txn, RecordDescriptor { vertexId }, currentLevel ? vertexFilter : GraphFilter {},
//                            currentLevel ? vertexClassFilter : ClassFilter {});
//                        if (vertexRdesc != RecordDescriptor {}) {
//                            vertexRdesc._depth = currentLevel;
//                            result.emplace_back(vertexRdesc);
//                        }
//                    }
//
//                    if (currentLevel < maxDepth) {
//                        ++currentLevel;
//                        if (currentLevel == stack.size()) {
//                            stack.emplace_back(decltype(stack)::value_type());
//                        }
//
//                        const auto incidentEdges = RecordCompare::filterIncidentEdges(
//                            txn, vertexId, direction, edgeFilter, edgeClassFilter);
//                        for (auto it = incidentEdges.crbegin(); it != incidentEdges.crend(); ++it) {
//                            const RecordDescriptor& edge = *it;
//                            RecordId nextVertex;
//                            auto srcDstVertex = txn._interface->graph()->getSrcDstVertices(edge.rid);
//                            switch (direction) {
//                            case Direction::IN:
//                                nextVertex = srcDstVertex.first;
//                                break;
//                            case Direction::OUT:
//                                nextVertex = srcDstVertex.second;
//                                break;
//                            case Direction::ALL:
//                                nextVertex = srcDstVertex.first != vertexId ? srcDstVertex.first : srcDstVertex.second;
//                                break;
//                            }
//                            if (visited.find(nextVertex) != visited.cend())
//                                continue;
//                            stack[currentLevel].emplace_back(std::move(nextVertex));
//                        }
//                    }
//                }
//
//                while (currentLevel && stack[currentLevel].empty()) {
//                    --currentLevel;
//                }
//            }
//        } catch (const Error& err) {
//            if (err.code() == NOGDB_GRAPH_NOEXST_VERTEX) {
//                throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_UNKNOWN_ERR);
//            } else {
//                throw NOGDB_FATAL_ERROR(err);
//            }
//        }
//        return result;
//    }

    ResultSet
    GraphTraversal::bfsShortestPath(const Transaction& txn,
        const ClassAccessInfo& srcVertexClassInfo,
        const ClassAccessInfo& dstVertexClassInfo,
        const RecordDescriptor& srcVertexRecordDescriptor,
        const RecordDescriptor& dstVertexRecordDescriptor,
        const GraphFilter& edgeFilter,
        const GraphFilter& vertexFilter)
    {
        const auto searchResultDescriptor = bfsShortestPathRdesc(
            txn, srcVertexClassInfo, dstVertexClassInfo,
            srcVertexRecordDescriptor, dstVertexRecordDescriptor, edgeFilter, vertexFilter);

        ResultSet result(searchResultDescriptor.size());
        std::transform(searchResultDescriptor.begin(), searchResultDescriptor.end(), result.begin(),
            [&txn](const RecordDescriptor& descriptor) {
                const auto edgeClassInfo = txn._adapter->dbClass()->getInfo(descriptor.rid.first);
                const auto& record = txn._interface->record()->getRecordWithBasicInfo(edgeClassInfo, descriptor);
                record.setBasicInfo(DEPTH_PROPERTY, descriptor._depth);
                return Result(descriptor, record);
            });

        return result;
    }

    std::vector<RecordDescriptor>
    GraphTraversal::bfsShortestPathRdesc(const Transaction& txn,
        const ClassAccessInfo& srcVertexClassInfo,
        const ClassAccessInfo& dstVertexClassInfo,
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
