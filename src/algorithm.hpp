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
#include <cassert>

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

        static ResultSet breathFirstSearch(const Txn &txn,
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

        static std::vector<RecordDescriptor>
        breathFirstSearchRdesc(const Txn &txn,
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
            auto keyValue = Datastore::getRecord(dsTxnHandler, classDBHandler, rid.second);
            auto record = Parser::parseRawData(keyValue, classPropertyInfo);
            auto name = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
            auto tmpRecord = record.set(CLASS_NAME_PROPERTY, name).set(RECORD_ID_PROPERTY, rid2str(rid));
            if (pathFilter.isSetVertex() && type == ClassType::VERTEX) {
                if ((*pathFilter.vertexFilter)(tmpRecord)) {
                    return Result{RecordDescriptor{rid}, record};
                } else {
                    return Result{};
                }
            } else if (pathFilter.isSetEdge() && type == ClassType::EDGE) {
                if ((*pathFilter.edgeFilter)(tmpRecord)) {
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
            auto keyValue = Datastore::getRecord(dsTxnHandler, classDBHandler, rid.second);
            auto record = Parser::parseRawData(keyValue, classPropertyInfo);
            auto name = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
            auto tmpRecord = record.set(CLASS_NAME_PROPERTY, name).set(RECORD_ID_PROPERTY, rid2str(rid));
            if (pathFilter.isSetVertex() && type == ClassType::VERTEX) {
                if ((*pathFilter.vertexFilter)(tmpRecord)) {
                    return RecordDescriptor{rid};
                } else {
                    return RecordDescriptor{};
                }
            } else if (pathFilter.isSetEdge() && type == ClassType::EDGE) {
                if ((*pathFilter.edgeFilter)(tmpRecord)) {
                    return RecordDescriptor{rid};
                } else {
                    return RecordDescriptor{};
                }
            } else {
                return RecordDescriptor{rid};
            }
        }
    };

}

#endif
