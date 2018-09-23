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

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <utility>

#include "nogdb_errors.h"
#include "nogdb_compare.h"
#include "nogdb_types.h"
#include "nogdb_context.h"
#include "nogdb_txn.h"
#include "nogdb_sql.h"

namespace nogdb {

    //*************************************************************
    //*  NogDB class operations.                                  *
    //*************************************************************

    struct Class {
        Class() = delete;

        ~Class() noexcept = delete;

        static const ClassDescriptor create(const Txn &txn, const std::string &className, ClassType type);

        static const ClassDescriptor
        createExtend(const Txn &txn, const std::string &className, const std::string &superClass);

        static void drop(const Txn &txn, const std::string &className);

        static void alter(const Txn &txn, const std::string &oldClassName, const std::string &newClassName);
    };

    //*************************************************************
    //*  NogDB property operations.                               *
    //*************************************************************

    struct Property {
        Property() = delete;

        ~Property() noexcept = delete;

        static const PropertyDescriptor
        add(const Txn &txn, const std::string &className, const std::string &propertyName, PropertyType type);

        static void alter(const Txn &txn, const std::string &className, const std::string &oldPropertyName,
                          const std::string &newPropertyName);

        static void remove(const Txn &txn, const std::string &className, const std::string &propertyName);

        static void
        createIndex(const Txn &txn, const std::string &className, const std::string &propertyName, bool isUnique = false);

        static void dropIndex(const Txn &txn, const std::string &className, const std::string &propertyName);
    };

    //*************************************************************
    //*  NogDB vertex operations.                                 *
    //*************************************************************

    struct Vertex {
        Vertex() = delete;

        ~Vertex() noexcept = delete;

        static const RecordDescriptor create(const Txn &txn, const std::string &className, const Record &record = Record{});

        static void update(const Txn &txn, const RecordDescriptor &recordDescriptor, const Record &record);

        static void destroy(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static void destroy(const Txn &txn, const std::string &className);

        static ResultSet get(const Txn &txn, const std::string &className);

        static ResultSet getExtend(const Txn &txn, const std::string &className);

        static ResultSetCursor getCursor(const Txn &txn, const std::string &className);

        static ResultSetCursor getExtendCursor(const Txn &txn, const std::string &className);

        static ResultSet getInEdge(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static ResultSet getOutEdge(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static ResultSet getAllEdge(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static ResultSetCursor getInEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static ResultSetCursor getOutEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static ResultSetCursor getAllEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static ResultSet get(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet get(const Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSet get(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet getExtend(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet getExtend(const Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSet getExtend(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet getIndex(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet getIndex(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet getExtendIndex(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet getExtendIndex(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor getCursor(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor getCursor(const Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSetCursor getCursor(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor getExtendCursor(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor getExtendCursor(const Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSetCursor getExtendCursor(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor getIndexCursor(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor getIndexCursor(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor getExtendIndexCursor(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor getExtendIndexCursor(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet
        getInEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition);

        static ResultSet
        getInEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &));

        static ResultSet
        getInEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition);

        static ResultSet
        getOutEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition);

        static ResultSet
        getOutEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &));

        static ResultSet
        getOutEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition);

        static ResultSet
        getAllEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition);

        static ResultSet
        getAllEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &));

        static ResultSet
        getAllEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition);

        static ResultSetCursor
        getInEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition);

        static ResultSetCursor
        getInEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &));

        static ResultSetCursor
        getInEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition);

        static ResultSetCursor
        getOutEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition);

        static ResultSetCursor
        getOutEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &));

        static ResultSetCursor
        getOutEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition);

        static ResultSetCursor
        getAllEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition);

        static ResultSetCursor
        getAllEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &));

        static ResultSetCursor
        getAllEdgeCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition);
    };

    //*************************************************************
    //*  NogDB edge operations.                                   *
    //*************************************************************

    struct Edge {
        Edge() = delete;

        ~Edge() noexcept = delete;

        static const RecordDescriptor
        create(const Txn &txn, const std::string &className, const RecordDescriptor &srcVertexRecordDescriptor,
               const RecordDescriptor &dstVertexRecordDescriptor, const Record &record = Record{});

        static void update(const Txn &txn, const RecordDescriptor &recordDescriptor, const Record &record);

        static void updateSrc(const Txn &txn, const RecordDescriptor &recordDescriptor,
                              const RecordDescriptor &newSrcVertexRecordDescriptor);

        static void
        updateDst(const Txn &txn, const RecordDescriptor &recordDescriptor, const RecordDescriptor &newDstRecordDescriptor);

        static void destroy(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static void destroy(const Txn &txn, const std::string &className);

        static ResultSet get(const Txn &txn, const std::string &className);

        static ResultSet getExtend(const Txn &txn, const std::string &className);

        static ResultSetCursor getCursor(const Txn &txn, const std::string &className);

        static ResultSetCursor getExtendCursor(const Txn &txn, const std::string& className);

        static Result getSrc(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static Result getDst(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static ResultSet getSrcDst(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static ResultSet get(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet get(const Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSet get(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet getExtend(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet getExtend(const Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSet getExtend(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet getIndex(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet getIndex(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet getExtendIndex(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet getExtendIndex(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor getCursor(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor getCursor(const Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSetCursor getCursor(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor getExtendCursor(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor getExtendCursor(const Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSetCursor getExtendCursor(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor getIndexCursor(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor getIndexCursor(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor getExtendIndexCursor(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor getExtendIndexCursor(const Txn &txn, const std::string &className, const MultiCondition &exp);
    };

    //*************************************************************
    //*  NogDB database operations.                               *
    //*************************************************************

    struct DB {
        DB() = delete;

        ~DB() noexcept = delete;

        static Record getRecord(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static const std::vector<ClassDescriptor> getClasses(const Txn &txn);

        static const std::vector<PropertyDescriptor> getProperties(const Txn &txn, const ClassDescriptor& classDescriptor);

        static const ClassDescriptor getClass(const Txn &txn, const std::string &className);

        static const ClassDescriptor getClass(const Txn &txn, const ClassId &classId);
    };

    //*************************************************************
    //*  NogDB graph traversal operations.                        *
    //*************************************************************

    struct Traverse {
        Traverse() = delete;

        ~Traverse() noexcept = delete;

        static ResultSet
        inEdgeBfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth);

        static ResultSet
        inEdgeBfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth,
                  const PathFilter &pathFilter);

        static ResultSet
        outEdgeBfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth);

        static ResultSet
        outEdgeBfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth,
                   const PathFilter &pathFilter);

        static ResultSet
        allEdgeBfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth);

        static ResultSet
        allEdgeBfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth,
                   const PathFilter &pathFilter);

        static ResultSet
        inEdgeDfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth);

        static ResultSet
        inEdgeDfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth,
                  const PathFilter &pathFilter);

        static ResultSet
        outEdgeDfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth);

        static ResultSet
        outEdgeDfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth,
                   const PathFilter &pathFilter);

        static ResultSet
        allEdgeDfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth);

        static ResultSet
        allEdgeDfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth,
                   const PathFilter &pathFilter);

        static ResultSet shortestPath(const Txn &txn,
                                      const RecordDescriptor &srcVertexRecordDescriptor,
                                      const RecordDescriptor &dstVertexRecordDescriptor);

        static ResultSet shortestPath(const Txn &txn,
                                      const RecordDescriptor &srcVertexRecordDescriptor,
                                      const RecordDescriptor &dstVertexRecordDescriptor,
                                      const PathFilter &pathFilter);

        /*
        template<typename CostFuncType, typename T = typename std::result_of<CostFuncType(Txn&, const RecordDescriptor&)>::type, typename CompareT = std::greater<T>>
        static std::pair<T, ResultSet> shortestPath(const Txn &txn,
                                         const RecordDescriptor &srcVertexRecordDescriptor,
                                         const RecordDescriptor &dstVertexRecordDescriptor,
                                         const CostFuncType &costFunction,
                                         const PathFilter &pathFilter,
                                         ) {
            Generic::getClassInfo(txn, srcVertexRecordDescriptor.rid.first, ClassType::VERTEX);
            Generic::getClassInfo(txn, dstVertexRecordDescriptor.rid.first, ClassType::VERTEX);
            auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
            return Algorithm::dijkstraShortestPath<CostFuncType, T, CompareT>(
                    txn, srcVertexRecordDescriptor,
                    dstVertexRecordDescriptor, costFunction, edgeClassIds, pathFilter);
        }
         */

        static ResultSetCursor
        inEdgeBfsCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth);

        static ResultSetCursor
        inEdgeBfsCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth,
                        const PathFilter &pathFilter);

        static ResultSetCursor
        outEdgeBfsCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth);

        static ResultSetCursor
        outEdgeBfsCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth,
                         const PathFilter &pathFilter);

        static ResultSetCursor
        allEdgeBfsCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth);

        static ResultSetCursor
        allEdgeBfsCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth,
                         const PathFilter &pathFilter);

        static ResultSetCursor
        inEdgeDfsCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth);

        static ResultSetCursor
        inEdgeDfsCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth,
                        const PathFilter &pathFilter);

        static ResultSetCursor
        outEdgeDfsCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth);

        static ResultSetCursor
        outEdgeDfsCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth,
                         const PathFilter &pathFilter);

        static ResultSetCursor
        allEdgeDfsCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth);

        static ResultSetCursor
        allEdgeDfsCursor(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth, unsigned int maxDepth,
                         const PathFilter &pathFilter);

        static ResultSetCursor shortestPathCursor(const Txn &txn,
                                                  const RecordDescriptor &srcVertexRecordDescriptor,
                                                  const RecordDescriptor &dstVertexRecordDescriptor);

        static ResultSetCursor shortestPathCursor(const Txn &txn,
                                                  const RecordDescriptor &srcVertexRecordDescriptor,
                                                  const RecordDescriptor &dstVertexRecordDescriptor,
                                                  const PathFilter &pathFilter);

        /*
        template<typename CostFuncType, typename T = typename std::result_of<CostFuncType(Txn&, const RecordDescriptor&)>::type, typename CompareT = std::greater<T>>
        static std::pair<T, ResultSetCursor> shortestPathCursor(const Txn &txn,
                                                                const RecordDescriptor &srcVertexRecordDescriptor,
                                                                const RecordDescriptor &dstVertexRecordDescriptor,
                                                                const CostFuncType &costFunction,
                                                                const PathFilter &pathFilter,
                                                                ) {
            Generic::getClassInfo(txn, srcVertexRecordDescriptor.rid.first, ClassType::VERTEX);
            Generic::getClassInfo(txn, dstVertexRecordDescriptor.rid.first, ClassType::VERTEX);
            auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
            auto result = ResultSetCursor{txn};
            auto metadata = Algorithm::dijkstraShortestPathRdesc<CostFuncType, T, CompareT>(
                    txn, srcVertexRecordDescriptor, dstVertexRecordDescriptor,
                    costFunction, edgeClassIds, pathFilter);

            result.metadata.insert(result.metadata.end(), metadata.second.cbegin(), metadata.second.cend());
            return {metadata.first, result};
        }
        */
    };

}
