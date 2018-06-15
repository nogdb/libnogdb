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


#ifndef _NOGDB_ALGORITHM_H_
#define _NOGDB_ALGORITHM_H_

namespace nogdb {

    namespace Traverse {

        template<typename CostFuncType,
                typename T = typename std::result_of<CostFuncType(const Txn&, const RecordDescriptor&)>::type,
                typename CompareT = std::greater<T>>
        std::pair<T, ResultSet> shortestPath(const Txn &txn,
                                             const RecordDescriptor &srcVertexRecordDescriptor,
                                             const RecordDescriptor &dstVertexRecordDescriptor,
                                             const CostFuncType &costFunction,
                                             const PathFilter &pathFilter,
                                             const ClassFilter &classFilter = ClassFilter{}) {
            Generic::getClassDescriptor(txn, srcVertexRecordDescriptor.rid.first, ClassType::VERTEX);
            Generic::getClassDescriptor(txn, dstVertexRecordDescriptor.rid.first, ClassType::VERTEX);
            auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
            return nogdb::Algorithm::dijkstraShortestPath<CostFuncType, T, CompareT>(
                    txn, srcVertexRecordDescriptor,
                    dstVertexRecordDescriptor, costFunction, edgeClassIds, pathFilter);
        }

        template<typename CostFuncType,
                typename T = typename std::result_of<CostFuncType(const Txn &, const RecordDescriptor &)>::type,
                typename CompareT = std::greater <T>>
        std::pair <T, ResultSetCursor> shortestPathCursor(Txn &txn,
                                                          const RecordDescriptor &srcVertexRecordDescriptor,
                                                          const RecordDescriptor &dstVertexRecordDescriptor,
                                                          const CostFuncType &costFunction,
                                                          const PathFilter &pathFilter,
                                                          const ClassFilter &classFilter = ClassFilter{}) {
            Generic::getClassDescriptor(txn, srcVertexRecordDescriptor.rid.first, ClassType::VERTEX);
            Generic::getClassDescriptor(txn, dstVertexRecordDescriptor.rid.first, ClassType::VERTEX);
            auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
            auto metadata = nogdb::Algorithm::dijkstraShortestPathRdesc<CostFuncType, T, CompareT>(
                    txn, srcVertexRecordDescriptor, dstVertexRecordDescriptor,
                    costFunction, edgeClassIds, pathFilter);

            return {metadata.first, ResultSetCursor{txn, metadata.second}};
        }
    }

    template<typename CostFuncType,
            typename T = typename std::result_of<CostFuncType(const Txn&, const RecordDescriptor&)>::type,
            typename CompareT = std::greater<T>>
    std::pair<T, ResultSet> Algorithm::dijkstraShortestPath(const Txn &txn,
                                                            const RecordDescriptor &srcVertexRecordDescriptor,
                                                            const RecordDescriptor &dstVertexRecordDescriptor,
                                                            const CostFuncType &costFunction,
                                                            const std::vector<ClassId> &edgeClassIds,
                                                            const PathFilter &pathFilter) {
        auto result = dijkstraShortestPathRdesc<CostFuncType, T, CompareT>(txn, srcVertexRecordDescriptor,
                                                                           dstVertexRecordDescriptor,
                                                                           costFunction,
                                                                           edgeClassIds, pathFilter);
        const auto &descriptors = result.second;
        ResultSet resultSet (descriptors.size());
        std::transform(descriptors.begin(), descriptors.end(), resultSet.begin(), [&txn](const RecordDescriptor &descriptor) {
            return Result(descriptor, retrieveRecord(txn, descriptor.rid));
        });

        return {result.first, resultSet};
    }

    template<typename CostFuncType,
            typename T = typename std::result_of<CostFuncType(const Txn&, const RecordDescriptor&)>::type,
            typename CompareT = std::greater<T>>
    std::pair<T, std::vector<RecordDescriptor>>
    Algorithm::dijkstraShortestPathRdesc(const Txn &txn,
                                         const RecordDescriptor &srcVertexRecordDescriptor,
                                         const RecordDescriptor &dstVertexRecordDescriptor,
                                         const CostFuncType& costFunction,
                                         const std::vector<ClassId> &edgeClassIds,
                                         const PathFilter& pathFilter) {

        auto srcStatus = Generic::checkIfRecordExist(txn, srcVertexRecordDescriptor);
        auto dstStatus = Generic::checkIfRecordExist(txn, dstVertexRecordDescriptor);

        if (srcStatus == RECORD_NOT_EXIST) {
            throw Error(GRAPH_NOEXST_SRC, Error::Type::GRAPH);
        } else if (dstStatus == RECORD_NOT_EXIST) {
            throw Error(GRAPH_NOEXST_DST, Error::Type::GRAPH);
        } else if (srcStatus == RECORD_NOT_EXIST_IN_MEMORY || dstStatus == RECORD_NOT_EXIST_IN_MEMORY) {
            return {T(), std::vector<RecordDescriptor>{}};
        } else {

            auto classDescriptor = Schema::ClassDescriptorPtr{};
            auto classPropertyInfo = ClassPropertyInfo{};
            auto classDBHandler = Datastore::DBHandler{};

            auto parent = std::unordered_map<RecordId, RecordDescriptor, Graph::RecordIdHash> {};
            auto distance = std::unordered_map<RecordId, T, Graph::RecordIdHash> {};

            using nodeInfo = std::pair<T, RecordId>;
            class Comparator {
            public:
                inline bool operator() (const nodeInfo &n, const nodeInfo &m) {
                    return CompareT()(n.first, m.first);
                }
            };
            auto heap = std::priority_queue<nodeInfo, std::vector<nodeInfo>, Comparator> {};

            const RecordId srcId = srcVertexRecordDescriptor.rid;
            const RecordId dstId = dstVertexRecordDescriptor.rid;

            heap.push(nodeInfo{T(), srcId});
            distance.insert({srcId, T()});

            while (!heap.empty()) {

                const T dist = heap.top().first;
                const RecordId vertex = heap.top().second;

                heap.pop();

                if (vertex == dstId) {
                    break;
                }

                if (CompareT()(dist, distance.at(vertex))) {
                    continue;
                }

                const auto edgeRecordDescriptors = getOutEdges(txn, classDescriptor, classPropertyInfo,
                                                               classDBHandler, vertex, pathFilter, edgeClassIds);

                for (const auto &edge : edgeRecordDescriptors) {
                    auto nextVertex = txn.txnCtx.dbRelation->getVertexDst(*(txn.txnBase), edge.rid);
                    auto tmpRdesc = pathFilter.isEnable() ?
                                    retrieveRdesc(txn, classDescriptor, classPropertyInfo,
                                                  classDBHandler, nextVertex, pathFilter, ClassType::VERTEX) :
                                    RecordDescriptor{nextVertex};

                    if (tmpRdesc == RecordDescriptor{}) continue;

                    const T nextDist = dist + costFunction(txn, edge);

                    if (distance.find(nextVertex) == distance.end() || CompareT()(distance.at(nextVertex), nextDist)) {

                        parent[nextVertex] = edge;
                        distance[nextVertex] = nextDist;

                        heap.push(nodeInfo{nextDist, nextVertex});
                    }
                }
            }

            if (distance.find(dstId) == distance.cend()) {
                return {T(), {}};
            }

            std::vector<RecordDescriptor> result {dstId};
            for (RecordId vertex = dstId; vertex != srcId;) {
                vertex = txn.txnCtx.dbRelation->getVertexSrc(*(txn.txnBase), parent.at(vertex).rid);
                result.emplace_back(vertex);
            }

            std::reverse(result.begin(), result.end());
            for (unsigned int i = 0u; i < result.size(); ++i) {
                result[i].depth = i;
            }

            return {distance.at(dstId), result};
        }
    }
}

#endif //_NOGDB_ALGORITHM_H_
