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

#include "blob.hpp"
#include "keyval.hpp"
#include "schema.hpp"
#include "env_handler.hpp"
#include "algorithm.hpp"

#include "nogdb_errors.h"
#include "nogdb_compare.h"

#define RECORD_NOT_EXIST                0
#define RECORD_NOT_EXIST_IN_MEMORY      1
#define RECORD_EXIST                    2

namespace nogdb {

    ResultSet Algorithm::breathFirstSearch(const Txn &txn,
                                           const RecordDescriptor &recordDescriptor,
                                           const unsigned int minDepth,
                                           const unsigned int maxDepth,
                                           const std::vector<ClassId> &edgeClassIds,
                                           std::vector<RecordId>
                                           (Graph::*edgeFunc)(const BaseTxn &baseTxn, const RecordId &rid,
                                                              const ClassId &classId),
                                           RecordId (Graph::*vertexFunc)(const BaseTxn &baseTxn, const RecordId &rid),
                                           const PathFilter &pathFilter) {
        switch (Generic::checkIfRecordExist(txn, recordDescriptor)) {
            case RECORD_NOT_EXIST:
                throw Error(GRAPH_NOEXST_VERTEX, Error::Type::GRAPH);
            case RECORD_NOT_EXIST_IN_MEMORY:
                return ((minDepth == 0) && (minDepth <= maxDepth)) ?
                       Generic::getRecordFromRdesc(txn, recordDescriptor) : ResultSet{};
            default:
                auto result = ResultSet{};
                auto classDescriptor = Schema::ClassDescriptorPtr{};
                auto classPropertyInfo = ClassPropertyInfo{};
                auto classDBHandler = Datastore::DBHandler{};
                auto visited = std::unordered_set<RecordId, Graph::RecordIdHash>{recordDescriptor.rid};
                auto queue = std::queue<std::pair<unsigned int, RecordId>> {};
                queue.push(std::make_pair(0, recordDescriptor.rid));
                try {
                    auto addUniqueVertex = [&](const RecordId &vertex, unsigned int currentLevel,
                                               const PathFilter &pathFilter) {
                        if (visited.find(vertex) == visited.cend()) {
                            auto tmpResult = retrieve(txn, classDescriptor, classPropertyInfo,
                                                      classDBHandler, vertex, pathFilter, ClassType::VERTEX);
                            if ((currentLevel + 1 >= minDepth) && (currentLevel + 1 <= maxDepth) &&
                                (!tmpResult.record.empty())) {
                                tmpResult.record.setBasicInfo(DEPTH_PROPERTY, currentLevel + 1);
                                tmpResult.descriptor.depth = currentLevel + 1;
                                result.push_back(tmpResult);
                            }
                            visited.insert(vertex);
                            if ((currentLevel + 1 < maxDepth) && (!tmpResult.record.empty())) {
                                queue.push(std::make_pair(currentLevel + 1, vertex));
                            }
                        }
                    };

                    if (minDepth == 0) {
                        classDescriptor = Generic::getClassDescriptor(txn, recordDescriptor.rid.first,
                                                                      ClassType::UNDEFINED);
                        classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
                        classDBHandler = Datastore::openDbi(txn.txnBase->getDsTxnHandler(),
                                                            std::to_string(recordDescriptor.rid.first), true);
                        auto className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
                        auto keyValue = Datastore::getRecord(txn.txnBase->getDsTxnHandler(),
                                                             classDBHandler, recordDescriptor.rid.second);
                        auto record = Parser::parseRawDataWithBasicInfo(className, recordDescriptor.rid, keyValue, classPropertyInfo);
                        result.emplace_back(Result{recordDescriptor, record});
                    }
                    while (!queue.empty()) {
                        auto element = queue.front();
                        queue.pop();
                        auto currentLevel = element.first;
                        auto vertexId = element.second;
                        auto edgeRecordDescriptors = std::vector<RecordDescriptor> {};
                        if (edgeClassIds.empty()) {
                            for (const auto &edge: ((*txn.txnCtx.dbRelation).*edgeFunc)(*(txn.txnBase), vertexId, 0)) {
                                auto tmpResult = retrieve(txn, classDescriptor, classPropertyInfo,
                                                          classDBHandler, edge,
                                                          pathFilter, ClassType::EDGE);
                                if (!tmpResult.record.empty()) {
                                    edgeRecordDescriptors.emplace_back(RecordDescriptor{edge});
                                }
                            }
                        } else {
                            for (const auto &edgeId: edgeClassIds) {
                                for (const auto &edge: ((*txn.txnCtx.dbRelation).*edgeFunc)(*(txn.txnBase), vertexId, edgeId)) {
                                    auto tmpResult = retrieve(txn, classDescriptor, classPropertyInfo,
                                                              classDBHandler, edge,
                                                              pathFilter, ClassType::EDGE);
                                    if (!tmpResult.record.empty()) {
                                        edgeRecordDescriptors.emplace_back(RecordDescriptor{edge});
                                    }
                                }
                            }
                        }
                        for (const auto &edge: edgeRecordDescriptors) {
                            if (vertexFunc != nullptr) {
                                addUniqueVertex(((*txn.txnCtx.dbRelation).*vertexFunc)(*(txn.txnBase), edge.rid),
                                                currentLevel, pathFilter);
                            } else {
                                auto vertices = txn.txnCtx.dbRelation->getVertexSrcDst(*(txn.txnBase), edge.rid);
                                addUniqueVertex(vertices.first, currentLevel, pathFilter);
                                addUniqueVertex(vertices.second, currentLevel, pathFilter);
                            }
                        }
                    }
                } catch (Graph::ErrorType &err) {
                    if (err == GRAPH_NOEXST_VERTEX) {
                        throw Error(GRAPH_UNKNOWN_ERR, Error::Type::GRAPH);
                    } else {
                        throw Error(err, Error::Type::GRAPH);
                    }
                } catch (Datastore::ErrorType &err) {
                    throw Error(err, Error::Type::DATASTORE);
                }
                return result;
        }
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
        switch (Generic::checkIfRecordExist(txn, recordDescriptor)) {
            case RECORD_NOT_EXIST:
                throw Error(GRAPH_NOEXST_VERTEX, Error::Type::GRAPH);
            case RECORD_NOT_EXIST_IN_MEMORY:
                return ((minDepth == 0) && (minDepth <= maxDepth)) ?
                       Generic::getRecordFromRdesc(txn, recordDescriptor) : ResultSet{};
            default:
                auto result = ResultSet{};
                auto classDescriptor = Schema::ClassDescriptorPtr{};
                auto classPropertyInfo = ClassPropertyInfo{};
                auto classDBHandler = Datastore::DBHandler{};
                auto visited = std::unordered_set<RecordId, Graph::RecordIdHash> {};
                auto usedEdges = std::unordered_set<RecordId, Graph::RecordIdHash> {};
                try {
                    std::function<void(const RecordId &, unsigned int, const PathFilter &)>
                            addUniqueVertex = [&](const RecordId &vertexId, unsigned int currentLevel,
                                                  const PathFilter &pathFilter) -> void {
                        if (visited.find(vertexId) == visited.cend()) {
                            auto tmpResult = Result{};
                            if (currentLevel == 0) {
                                tmpResult = retrieve(txn, classDescriptor, classPropertyInfo, classDBHandler,
                                                     vertexId, PathFilter{}, ClassType::VERTEX);
                            } else {
                                tmpResult = retrieve(txn, classDescriptor, classPropertyInfo, classDBHandler,
                                                     vertexId, pathFilter, ClassType::VERTEX);
                            }
                            if ((currentLevel >= minDepth) && (!tmpResult.record.empty())) {
                                tmpResult.record.setBasicInfo(DEPTH_PROPERTY, currentLevel);
                                tmpResult.descriptor.depth = currentLevel;
                                result.push_back(tmpResult);
                            }
                            visited.insert(vertexId);
                            if ((currentLevel < maxDepth) && (!tmpResult.record.empty())) {
                                auto edgeRecordDescriptors = std::vector<RecordDescriptor> {};
                                if (edgeClassIds.empty()) {
                                    for (const auto &edge: ((*txn.txnCtx.dbRelation).*edgeFunc)(*(txn.txnBase), vertexId, 0)) {
                                        auto tmpResult = retrieve(txn, classDescriptor, classPropertyInfo,
                                                                  classDBHandler, edge, pathFilter, ClassType::EDGE);
                                        if (!tmpResult.record.empty()) {
                                            edgeRecordDescriptors.emplace_back(RecordDescriptor{edge});
                                        }
                                    }
                                } else {
                                    for (const auto &edgeId: edgeClassIds) {
                                        for (const auto &edge: ((*txn.txnCtx.dbRelation).*edgeFunc)(*(txn.txnBase), vertexId, edgeId)) {
                                            auto tmpResult = retrieve(txn, classDescriptor, classPropertyInfo,
                                                                      classDBHandler, edge, pathFilter,
                                                                      ClassType::EDGE);
                                            if (!tmpResult.record.empty()) {
                                                edgeRecordDescriptors.emplace_back(RecordDescriptor{edge});
                                            }
                                        }
                                    }
                                }
                                for (const auto &edge: edgeRecordDescriptors) {
                                    if (usedEdges.find(edge.rid) == usedEdges.cend()) {
                                        usedEdges.insert(edge.rid);
                                        if (vertexFunc != nullptr) {
                                            auto nextVertex = ((*txn.txnCtx.dbRelation).*vertexFunc)(*(txn.txnBase), edge.rid);
                                            if (nextVertex != vertexId) {
                                                addUniqueVertex(nextVertex, currentLevel + 1, pathFilter);
                                            }
                                        } else {
                                            auto vertices = txn.txnCtx.dbRelation->getVertexSrcDst(*(txn.txnBase), edge.rid);
                                            if (vertices.first != vertexId) {
                                                addUniqueVertex(vertices.first, currentLevel + 1, pathFilter);
                                            }
                                            if (vertices.second != vertexId) {
                                                addUniqueVertex(vertices.second, currentLevel + 1, pathFilter);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    };

                    addUniqueVertex(recordDescriptor.rid, 0, pathFilter);
                } catch (Graph::ErrorType &err) {
                    if (err == GRAPH_NOEXST_VERTEX) {
                        throw Error(GRAPH_UNKNOWN_ERR, Error::Type::GRAPH);
                    } else {
                        throw Error(err, Error::Type::GRAPH);
                    }
                } catch (Datastore::ErrorType &err) {
                    throw Error(err, Error::Type::DATASTORE);
                }
                return result;
        }
    }

    ResultSet Algorithm::bfsShortestPath(const Txn &txn,
                                         const RecordDescriptor &srcVertexRecordDescriptor,
                                         const RecordDescriptor &dstVertexRecordDescriptor,
                                         const std::vector<ClassId> &edgeClassIds,
                                         const PathFilter &pathFilter) {
        auto srcStatus = Generic::checkIfRecordExist(txn, srcVertexRecordDescriptor);
        auto dstStatus = Generic::checkIfRecordExist(txn, dstVertexRecordDescriptor);
        if (srcStatus == RECORD_NOT_EXIST) {
            throw Error(GRAPH_NOEXST_SRC, Error::Type::GRAPH);
        } else if (dstStatus == RECORD_NOT_EXIST) {
            throw Error(GRAPH_NOEXST_DST, Error::Type::GRAPH);
        } else if (srcStatus == RECORD_NOT_EXIST_IN_MEMORY || dstStatus == RECORD_NOT_EXIST_IN_MEMORY) {
            return ResultSet{};
        } else {
            auto result = ResultSet{};
            auto classDescriptor = Schema::ClassDescriptorPtr{};
            auto classPropertyInfo = ClassPropertyInfo{};
            auto classDBHandler = Datastore::DBHandler{};
            try {
                if (srcVertexRecordDescriptor == dstVertexRecordDescriptor) {
                    classDescriptor = Generic::getClassDescriptor(txn, srcVertexRecordDescriptor.rid.first,
                                                                  ClassType::UNDEFINED);
                    classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
                    classDBHandler = Datastore::openDbi(txn.txnBase->getDsTxnHandler(),
                                                        std::to_string(srcVertexRecordDescriptor.rid.first), true);
                    auto keyValue = Datastore::getRecord(txn.txnBase->getDsTxnHandler(),
                                                         classDBHandler, srcVertexRecordDescriptor.rid.second);
                    auto className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
                    auto record = Parser::parseRawDataWithBasicInfo(className, srcVertexRecordDescriptor.rid, keyValue, classPropertyInfo);
                    result.emplace_back(Result{srcVertexRecordDescriptor, record});
                } else {
                    bool found = false;
                    auto visited = std::unordered_map<RecordId, std::pair<Result, RecordId>, Graph::RecordIdHash> {};
                    visited.insert({srcVertexRecordDescriptor.rid, {Result{}, RecordId{}}});
                    auto queue = std::queue<RecordId> {};
                    queue.push(srcVertexRecordDescriptor.rid);
                    while (!queue.empty() && !found) {
                        auto vertex = queue.front();
                        queue.pop();
                        auto edgeRecordDescriptors = std::vector<RecordDescriptor>{};
                        if (edgeClassIds.empty()) {
                            for (const auto &edge: txn.txnCtx.dbRelation->getEdgeOut(*(txn.txnBase), vertex, 0)) {
                                auto tmpResult = retrieve(txn, classDescriptor, classPropertyInfo,
                                                          classDBHandler, edge, pathFilter, ClassType::EDGE);
                                if (!tmpResult.record.empty()) {
                                    edgeRecordDescriptors.emplace_back(RecordDescriptor{edge});
                                }
                            }
                        } else {
                            for (const auto &edgeId: edgeClassIds) {
                                for (const auto &edge: txn.txnCtx.dbRelation->getEdgeOut(*(txn.txnBase), vertex, edgeId)) {
                                    auto tmpResult = retrieve(txn, classDescriptor, classPropertyInfo,
                                                              classDBHandler, edge, pathFilter, ClassType::EDGE);
                                    if (!tmpResult.record.empty()) {
                                        edgeRecordDescriptors.emplace_back(RecordDescriptor{edge});
                                    }
                                }
                            }
                        }
                        for (const auto &edge: edgeRecordDescriptors) {
                            auto nextVertex = txn.txnCtx.dbRelation->getVertexDst(*(txn.txnBase), edge.rid);
                            if (visited.find(nextVertex) == visited.cend()) {
                                auto tmpResult = retrieve(txn, classDescriptor, classPropertyInfo,
                                                          classDBHandler, nextVertex, pathFilter, ClassType::VERTEX);
                                if (!tmpResult.record.empty()) {
                                    visited.insert({nextVertex, {tmpResult, vertex}});
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
                            result.push_back(data.first);
                            vertex = data.second;
                        }
                        classDescriptor = Generic::getClassDescriptor(txn, srcVertexRecordDescriptor.rid.first,
                                                                      ClassType::UNDEFINED);
                        classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
                        classDBHandler = Datastore::openDbi(txn.txnBase->getDsTxnHandler(),
                                                            std::to_string(srcVertexRecordDescriptor.rid.first), true);
                        auto keyValue = Datastore::getRecord(txn.txnBase->getDsTxnHandler(),
                                                             classDBHandler, srcVertexRecordDescriptor.rid.second);
                        auto className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
                        auto record = Parser::parseRawDataWithBasicInfo(className, srcVertexRecordDescriptor.rid, keyValue, classPropertyInfo);
                        result.emplace_back(Result{srcVertexRecordDescriptor, record});
                        std::reverse(result.begin(), result.end());
                        auto currentLevel = 0U;
                        for(auto& res: result) {
                            res.record.setBasicInfo(DEPTH_PROPERTY, currentLevel);
                            res.descriptor.depth = currentLevel;
                            ++currentLevel;
                        }
                    }
                }
            } catch (Graph::ErrorType &err) {
                if (err == GRAPH_NOEXST_VERTEX) {
                    throw Error(GRAPH_UNKNOWN_ERR, Error::Type::GRAPH);
                } else {
                    throw Error(err, Error::Type::GRAPH);
                }
            } catch (Datastore::ErrorType &err) {
                throw Error(err, Error::Type::DATASTORE);
            }
            return result;
        }
    }

    std::vector<RecordDescriptor>
    Algorithm::breathFirstSearchRdesc(const Txn &txn,
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
                throw Error(GRAPH_NOEXST_VERTEX, Error::Type::GRAPH);
            case RECORD_NOT_EXIST_IN_MEMORY:
                return ((minDepth == 0) && (minDepth <= maxDepth)) ?
                       std::vector<RecordDescriptor>{recordDescriptor} : std::vector<RecordDescriptor>{};
            default:
                auto result = std::vector<RecordDescriptor>{};
                auto classDescriptor = Schema::ClassDescriptorPtr{};
                auto classPropertyInfo = ClassPropertyInfo{};
                auto classDBHandler = Datastore::DBHandler{};
                auto visited = std::unordered_set<RecordId, Graph::RecordIdHash>{recordDescriptor.rid};
                auto queue = std::queue<std::pair<unsigned int, RecordId>> {};
                queue.push(std::make_pair(0, recordDescriptor.rid));
                try {
                    auto addUniqueVertex = [&](const RecordId &vertex, unsigned int currentLevel,
                                               const PathFilter &pathFilter) {
                        if (visited.find(vertex) == visited.cend()) {
                            auto tmpRdesc = (pathFilter.isSetVertex() || pathFilter.isSetEdge()) ?
                                            retrieveRdesc(txn, classDescriptor, classPropertyInfo,
                                                          classDBHandler, vertex, pathFilter, ClassType::VERTEX) :
                                            RecordDescriptor{vertex};
                            if ((currentLevel + 1 >= minDepth) && (currentLevel + 1 <= maxDepth) &&
                                (tmpRdesc != RecordDescriptor{})) {
                                tmpRdesc.depth = currentLevel + 1;
                                result.emplace_back(tmpRdesc);
                            }
                            visited.insert(vertex);
                            if ((currentLevel + 1 < maxDepth) && (tmpRdesc != RecordDescriptor{})) {
                                queue.push(std::make_pair(currentLevel + 1, vertex));
                            }
                        }
                    };

                    if (minDepth == 0) {
                        result.emplace_back(recordDescriptor);
                    }
                    while (!queue.empty()) {
                        auto element = queue.front();
                        queue.pop();
                        auto currentLevel = element.first;
                        auto vertexId = element.second;
                        auto edgeRecordDescriptors = std::vector<RecordDescriptor>{};
                        if (edgeClassIds.empty()) {
                            for (const auto &edge: ((*txn.txnCtx.dbRelation).*edgeFunc)(*(txn.txnBase), vertexId, 0)) {
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
                                for (const auto &edge: ((*txn.txnCtx.dbRelation).*edgeFunc)(*(txn.txnBase), vertexId, edgeId)) {
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
                        for (const auto &edge: edgeRecordDescriptors) {
                            if (vertexFunc != nullptr) {
                                addUniqueVertex(((*txn.txnCtx.dbRelation).*vertexFunc)(*(txn.txnBase), edge.rid),
                                                currentLevel, pathFilter);
                            } else {
                                auto vertices = txn.txnCtx.dbRelation->getVertexSrcDst(*(txn.txnBase), edge.rid);
                                addUniqueVertex(vertices.first, currentLevel, pathFilter);
                                addUniqueVertex(vertices.second, currentLevel, pathFilter);
                            }
                        }
                    }
                } catch (Graph::ErrorType &err) {
                    if (err == GRAPH_NOEXST_VERTEX) {
                        throw Error(GRAPH_UNKNOWN_ERR, Error::Type::GRAPH);
                    } else {
                        throw Error(err, Error::Type::GRAPH);
                    }
                } catch (Datastore::ErrorType &err) {
                    throw Error(err, Error::Type::DATASTORE);
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
                throw Error(GRAPH_NOEXST_VERTEX, Error::Type::GRAPH);
            case RECORD_NOT_EXIST_IN_MEMORY:
                return ((minDepth == 0) && (minDepth <= maxDepth)) ?
                       std::vector<RecordDescriptor>{recordDescriptor} : std::vector<RecordDescriptor>{};
            default:
                auto result = std::vector<RecordDescriptor>{};
                auto classDescriptor = Schema::ClassDescriptorPtr{};
                auto classPropertyInfo = ClassPropertyInfo{};
                auto classDBHandler = Datastore::DBHandler{};
                auto visited = std::unordered_set<RecordId, Graph::RecordIdHash> {};
                auto usedEdges = std::unordered_set<RecordId, Graph::RecordIdHash> {};
                try {
                    std::function<void(const RecordId &, unsigned int, const PathFilter &)>
                            addUniqueVertex = [&](const RecordId &vertexId, unsigned int currentLevel,
                                                  const PathFilter &pathFilter) -> void {
                        if (visited.find(vertexId) == visited.cend()) {
                            auto tmpRdesc = RecordDescriptor{};
                            if (currentLevel == 0) {
                                tmpRdesc = (pathFilter.isSetVertex() || pathFilter.isSetEdge()) ?
                                           retrieveRdesc(txn, classDescriptor, classPropertyInfo,
                                                         classDBHandler, vertexId, PathFilter{}, ClassType::VERTEX) :
                                           RecordDescriptor{vertexId};
                            } else {
                                tmpRdesc = (pathFilter.isSetVertex() || pathFilter.isSetEdge()) ?
                                           retrieveRdesc(txn, classDescriptor, classPropertyInfo,
                                                         classDBHandler, vertexId, pathFilter, ClassType::VERTEX) :
                                           RecordDescriptor{vertexId};
                            }
                            if ((currentLevel >= minDepth) && (tmpRdesc != RecordDescriptor{})) {
                                tmpRdesc.depth = currentLevel;
                                result.push_back(tmpRdesc);
                            }
                            visited.insert(vertexId);
                            if ((currentLevel < maxDepth) && (tmpRdesc != RecordDescriptor{})) {
                                auto edgeRecordDescriptors = std::vector<RecordDescriptor> {};
                                if (edgeClassIds.empty()) {
                                    for (const auto &edge: ((*txn.txnCtx.dbRelation).*edgeFunc)(*(txn.txnBase), vertexId, 0)) {
                                        auto tmpRdesc = (pathFilter.isSetVertex() || pathFilter.isSetEdge()) ?
                                                        retrieveRdesc(txn, classDescriptor, classPropertyInfo,
                                                                      classDBHandler, edge, pathFilter, ClassType::EDGE)
                                                                                                             :
                                                        RecordDescriptor{edge};
                                        if (tmpRdesc != RecordDescriptor{}) {
                                            edgeRecordDescriptors.emplace_back(RecordDescriptor{edge});
                                        }
                                    }
                                } else {
                                    for (const auto &edgeId: edgeClassIds) {
                                        for (const auto &edge: ((*txn.txnCtx.dbRelation).*edgeFunc)(*(txn.txnBase), vertexId, edgeId)) {
                                            auto tmpRdesc = (pathFilter.isSetVertex() || pathFilter.isSetEdge()) ?
                                                            retrieveRdesc(txn, classDescriptor, classPropertyInfo,
                                                                          classDBHandler, edge, pathFilter,
                                                                          ClassType::EDGE) :
                                                            RecordDescriptor{edge};
                                            if (tmpRdesc != RecordDescriptor{}) {
                                                edgeRecordDescriptors.emplace_back(RecordDescriptor{edge});
                                            }
                                        }
                                    }
                                }
                                for (const auto &edge: edgeRecordDescriptors) {
                                    if (usedEdges.find(edge.rid) == usedEdges.cend()) {
                                        usedEdges.insert(edge.rid);
                                        if (vertexFunc != nullptr) {
                                            auto nextVertex = ((*txn.txnCtx.dbRelation).*vertexFunc)(*(txn.txnBase), edge.rid);
                                            if (nextVertex != vertexId) {
                                                addUniqueVertex(nextVertex, currentLevel + 1, pathFilter);
                                            }
                                        } else {
                                            auto vertices = txn.txnCtx.dbRelation->getVertexSrcDst(*(txn.txnBase), edge.rid);
                                            if (vertices.first != vertexId) {
                                                addUniqueVertex(vertices.first, currentLevel + 1, pathFilter);
                                            }
                                            if (vertices.second != vertexId) {
                                                addUniqueVertex(vertices.second, currentLevel + 1, pathFilter);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    };

                    addUniqueVertex(recordDescriptor.rid, 0, pathFilter);
                } catch (Graph::ErrorType &err) {
                    if (err == GRAPH_NOEXST_VERTEX) {
                        throw Error(GRAPH_UNKNOWN_ERR, Error::Type::GRAPH);
                    } else {
                        throw Error(err, Error::Type::GRAPH);
                    }
                } catch (Datastore::ErrorType &err) {
                    throw Error(err, Error::Type::DATASTORE);
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
            throw Error(GRAPH_NOEXST_SRC, Error::Type::GRAPH);
        } else if (dstStatus == RECORD_NOT_EXIST) {
            throw Error(GRAPH_NOEXST_DST, Error::Type::GRAPH);
        } else if (srcStatus == RECORD_NOT_EXIST_IN_MEMORY || dstStatus == RECORD_NOT_EXIST_IN_MEMORY) {
            return std::vector<RecordDescriptor>{};
        } else {
            auto result = std::vector<RecordDescriptor>{};
            auto classDescriptor = Schema::ClassDescriptorPtr{};
            auto classPropertyInfo = ClassPropertyInfo{};
            auto classDBHandler = Datastore::DBHandler{};
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
                        auto edgeRecordDescriptors = std::vector<RecordDescriptor>{};
                        if (edgeClassIds.empty()) {
                            for (const auto &edge: txn.txnCtx.dbRelation->getEdgeOut(*(txn.txnBase), vertex, 0)) {
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
                                for (const auto &edge: txn.txnCtx.dbRelation->getEdgeOut(*(txn.txnBase), vertex, edgeId)) {
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
                            result.push_back(data.first);
                            vertex = data.second;
                        }
                        result.emplace_back(srcVertexRecordDescriptor);
                        std::reverse(result.begin(), result.end());
                        auto currentLevel = 0U;
                        for(auto& res: result) {
                            res.depth = currentLevel;
                            ++currentLevel;
                        }
                    }
                }
            } catch (Graph::ErrorType &err) {
                if (err == GRAPH_NOEXST_VERTEX) {
                    throw Error(GRAPH_UNKNOWN_ERR, Error::Type::GRAPH);
                } else {
                    throw Error(err, Error::Type::GRAPH);
                }
            } catch (Datastore::ErrorType &err) {
                throw Error(err, Error::Type::DATASTORE);
            }
            return result;
        }
    }

}


