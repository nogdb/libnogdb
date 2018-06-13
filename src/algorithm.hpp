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

#ifndef __ALGORITHM_HPP_INCLUDED_
#define __ALGORITHM_HPP_INCLUDED_

#include <functional>
#include <queue>

#include "constant.hpp"
#include "datastore.hpp"
#include "graph.hpp"
#include "parser.hpp"
#include "generic.hpp"
#include "schema.hpp"

#include "nogdb_compare.h"
#include "nogdb_types.h"
#include "nogdb_txn.h"

namespace nogdb {
    struct Algorithm {
        Algorithm() = delete;

        ~Algorithm() noexcept = delete;

        static ResultSet breadthFirstSearch(const Txn &txn,
                                           const RecordDescriptor &recordDescriptor,
                                           unsigned int minDepth,
                                           unsigned int maxDepth,
                                           const std::vector<ClassId> &edgeClassIds,
                                           std::vector<RecordId>
                                           (Graph::*edgeFunc)(const BaseTxn &baseTxn, const RecordId &rid,
                                                              const ClassId &classId),
                                           RecordId (Graph::*vertexFunc)(const BaseTxn &baseTxn, const RecordId &rid),
                                           const PathFilter &pathFilter);

        static ResultSet depthFirstSearch(const Txn &txn,
                                          const RecordDescriptor &recordDescriptor,
                                          unsigned int minDepth,
                                          unsigned int maxDepth,
                                          const std::vector<ClassId> &edgeClassIds,
                                          std::vector<RecordId>
                                          (Graph::*edgeFunc)(const BaseTxn &baseTxn, const RecordId &rid,
                                                             const ClassId &classId),
                                          RecordId (Graph::*vertexFunc)(const BaseTxn &baseTxn, const RecordId &rid),
                                          const PathFilter &pathFilter);

        static ResultSet bfsShortestPath(const Txn &txn,
                                         const RecordDescriptor &srcVertexRecordDescriptor,
                                         const RecordDescriptor &dstVertexRecordDescriptor,
                                         const std::vector<ClassId> &edgeClassIds,
                                         const PathFilter &pathFilter);

        template<typename CostFuncType,
                typename T = typename std::result_of<CostFuncType(const Txn&, const RecordDescriptor&)>::type,
                typename CompareT = std::greater<T>>
        static std::pair<T, ResultSet> dijkstraShortestPath(const Txn &txn,
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

        static std::vector<RecordDescriptor>
        breadthFirstSearchRdesc(const Txn &txn,
                               const RecordDescriptor &recordDescriptor,
                               unsigned int minDepth,
                               unsigned int maxDepth,
                               const std::vector<ClassId> &edgeClassIds,
                               std::vector<RecordId> (Graph::*edgeFunc)(const BaseTxn &baseTxn, const RecordId &rid,
                                                                        const ClassId &classId),
                               RecordId (Graph::*vertexFunc)(const BaseTxn &baseTxn, const RecordId &rid),
                               const PathFilter &pathFilter);

        static std::vector<RecordDescriptor>
        depthFirstSearchRdesc(const Txn &txn,
                              const RecordDescriptor &recordDescriptor,
                              unsigned int minDepth,
                              unsigned int maxDepth,
                              const std::vector<ClassId> &edgeClassIds,
                              std::vector<RecordId>
                              (Graph::*edgeFunc)(const BaseTxn &baseTxn, const RecordId &rid, const ClassId &classId),
                              RecordId (Graph::*vertexFunc)(const BaseTxn &baseTxn, const RecordId &rid),
                              const PathFilter &pathFilter);

        static std::vector<RecordDescriptor>
        bfsShortestPathRdesc(const Txn &txn,
                             const RecordDescriptor &srcVertexRecordDescriptor,
                             const RecordDescriptor &dstVertexRecordDescriptor,
                             const std::vector<ClassId> &edgeClassIds,
                             const PathFilter &pathFilter);

        template<typename CostFuncType,
                typename T = typename std::result_of<CostFuncType(const Txn&, const RecordDescriptor&)>::type,
                typename CompareT = std::greater<T>>
        static std::pair<T, std::vector<RecordDescriptor>>
        dijkstraShortestPathRdesc(const Txn &txn,
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

                            parent.insert({nextVertex, edge});
                            distance.insert({nextVertex, nextDist});

                            heap.push(nodeInfo{nextDist, nextVertex});
                        }
                    }
                }

                std::vector<RecordDescriptor> result {};
                for (RecordId vertex = dstId; vertex != srcId;) {
                    if (parent.find(vertex) == parent.cend()) {
                        return {T(), std::vector<RecordDescriptor>()};
                    }
                    result.emplace_back(std::move(parent.at(vertex)));
                    vertex = txn.txnCtx.dbRelation->getVertexSrc(*(txn.txnBase), result.back().rid);
                }

                std::reverse(result.begin(), result.end());
                for (unsigned int i = 0u; i < result.size(); ++i) {
                    result[i].depth = i;
                }

                return {distance.at(dstId), result};
            }
        }

        inline static Result retrieve(const Txn &txn,
                                      Schema::ClassDescriptorPtr &classDescriptor,
                                      ClassPropertyInfo &classPropertyInfo,
                                      Datastore::DBHandler &classDBHandler,
                                      const RecordId &rid,
                                      const PathFilter &pathFilter,
                                      ClassType type) {
            auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
            if (classDescriptor == nullptr || classDescriptor->id != rid.first) {
                classDescriptor = Generic::getClassDescriptor(txn, rid.first, ClassType::UNDEFINED);
                classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
                classDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(rid.first), true);
            }
            auto className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
            auto keyValue = Datastore::getRecord(dsTxnHandler, classDBHandler, rid.second);
            auto record = Parser::parseRawDataWithBasicInfo(className, rid, keyValue, classPropertyInfo);
            if (pathFilter.isSetVertex() && type == ClassType::VERTEX) {
                if ((*pathFilter.vertexFilter)(record)) {
                    return Result{RecordDescriptor{rid}, record};
                } else {
                    return Result{};
                }
            } else if (pathFilter.isSetEdge() && type == ClassType::EDGE) {
                if ((*pathFilter.edgeFilter)(record)) {
                    return Result{RecordDescriptor{rid}, record};
                } else {
                    return Result{};
                }
            } else {
                return Result{RecordDescriptor{rid}, record};
            }
        }

        inline static RecordDescriptor retrieveRdesc(const Txn &txn,
                                                     Schema::ClassDescriptorPtr &classDescriptor,
                                                     ClassPropertyInfo &classPropertyInfo,
                                                     Datastore::DBHandler &classDBHandler,
                                                     const RecordId &rid,
                                                     const PathFilter &pathFilter,
                                                     ClassType type) {
            auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
            if (classDescriptor == nullptr || classDescriptor->id != rid.first) {
                classDescriptor = Generic::getClassDescriptor(txn, rid.first, ClassType::UNDEFINED);
                classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
                classDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(rid.first), true);
            }
            auto className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
            auto keyValue = Datastore::getRecord(dsTxnHandler, classDBHandler, rid.second);
            auto record = Parser::parseRawDataWithBasicInfo(className, rid, keyValue, classPropertyInfo);
            if (pathFilter.isSetVertex() && type == ClassType::VERTEX) {
                if ((*pathFilter.vertexFilter)(record)) {
                    return RecordDescriptor{rid};
                } else {
                    return RecordDescriptor{};
                }
            } else if (pathFilter.isSetEdge() && type == ClassType::EDGE) {
                if ((*pathFilter.edgeFilter)(record)) {
                    return RecordDescriptor{rid};
                } else {
                    return RecordDescriptor{};
                }
            } else {
                return RecordDescriptor{rid};
            }
        }

        inline static Record retrieveRecord(const nogdb::Txn &txn, const nogdb::RecordDescriptor &descriptor) {

            auto classDescriptor = Generic::getClassDescriptor(txn, descriptor.rid.first,
                                                               ClassType::UNDEFINED);
            auto classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
            auto classDBHandler = Datastore::openDbi(txn.txnBase->getDsTxnHandler(),
                                                     std::to_string(descriptor.rid.first), true);
            auto keyValue = Datastore::getRecord(txn.txnBase->getDsTxnHandler(),
                                                 classDBHandler, descriptor.rid.second);
            auto className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;

            return Parser::parseRawDataWithBasicInfo(className, descriptor.rid, keyValue, classPropertyInfo);
        }

        inline static std::vector<RecordDescriptor>
        getIncidentEdges(const nogdb::Txn &txn,
                                    nogdb::Schema::ClassDescriptorPtr &classDescriptor,
                                    nogdb::ClassPropertyInfo &classPropertyInfo,
                                    nogdb::Datastore::DBHandler &classDBHandler,
                                    std::vector<RecordId>
                                    (Graph::*edgeFunc)(const BaseTxn &baseTxn, const RecordId &rid,
                                                       const ClassId &classId),
                                    const nogdb::RecordId &vertex,
                                    const nogdb::PathFilter &pathFilter,
                                    const std::vector<nogdb::ClassId> &edgeClassIds){

            auto edgeRecordDescriptors = std::vector<RecordDescriptor>{};
            if (edgeClassIds.empty()) {
                for (const auto &edge: ((*txn.txnCtx.dbRelation).*edgeFunc)(*(txn.txnBase), vertex, 0)) {
                    auto tmpRdesc = (pathFilter.isSetVertex() || pathFilter.isSetEdge()) ?
                                    retrieveRdesc(txn, classDescriptor, classPropertyInfo,
                                                  classDBHandler, edge, pathFilter, ClassType::EDGE) :
                                    RecordDescriptor{edge};
                    if (tmpRdesc != RecordDescriptor{}) {
                        edgeRecordDescriptors.emplace_back(RecordDescriptor{edge});
                    }
                }
            } else {
                for (const auto &edgeId: edgeClassIds) {
                    for (const auto &edge: ((*txn.txnCtx.dbRelation).*edgeFunc)(*(txn.txnBase), vertex, edgeId)) {
                        auto tmpRdesc = (pathFilter.isSetVertex() || pathFilter.isSetEdge()) ?
                                        retrieveRdesc(txn, classDescriptor, classPropertyInfo,
                                                      classDBHandler, edge, pathFilter, ClassType::EDGE) :
                                        RecordDescriptor{edge};
                        if (tmpRdesc != RecordDescriptor{}) {
                            edgeRecordDescriptors.emplace_back(RecordDescriptor{edge});
                        }
                    }
                }
            }

            return edgeRecordDescriptors;
        }

        inline static std::vector<RecordDescriptor>
        getOutEdges(const Txn &txn,
                    Schema::ClassDescriptorPtr &classDescriptor,
                    ClassPropertyInfo &classPropertyInfo,
                    Datastore::DBHandler &classDBHandler,
                    const RecordId &vertex,
                    const PathFilter &pathFilter,
                    const std::vector<ClassId> &edgeClassIds) {

            std::vector<RecordDescriptor> edgeRecordDescriptors {};

            if (edgeClassIds.empty()) {
                for (const auto &edge: txn.txnCtx.dbRelation->getEdgeOut(*(txn.txnBase), vertex, 0)) {
                    auto tmpRdesc = pathFilter.isEnable() ?
                                    retrieveRdesc(txn, classDescriptor, classPropertyInfo,
                                                  classDBHandler, edge, pathFilter, ClassType::EDGE) :
                                    RecordDescriptor{edge};
                    if (tmpRdesc != RecordDescriptor{}) {
                        edgeRecordDescriptors.emplace_back(RecordDescriptor{edge});
                    }
                }
            } else {
                for (const auto &edgeId: edgeClassIds) {
                    for (const auto &edge: txn.txnCtx.dbRelation->getEdgeOut(*(txn.txnBase), vertex, edgeId)) {
                        auto tmpRdesc = pathFilter.isEnable() ?
                                        retrieveRdesc(txn, classDescriptor, classPropertyInfo,
                                                      classDBHandler, edge, pathFilter, ClassType::EDGE) :
                                        RecordDescriptor{edge};
                        if (tmpRdesc != RecordDescriptor{}) {
                            edgeRecordDescriptors.emplace_back(RecordDescriptor{edge});
                        }
                    }
                }
            }

            return edgeRecordDescriptors;
        }
    };

}

#endif
