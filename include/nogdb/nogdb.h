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

#ifndef __NOGDB_H_INCLUDED_
#define __NOGDB_H_INCLUDED_

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

        static const ClassDescriptor create(Txn &txn, const std::string &className, ClassType type);

        static const ClassDescriptor
        createExtend(Txn &txn, const std::string &className, const std::string &superClass);

        static void drop(Txn &txn, const std::string &className);

        static void alter(Txn &txn, const std::string &oldClassName, const std::string &newClassName);
    };

    //*************************************************************
    //*  NogDB property operations.                               *
    //*************************************************************

    struct Property {
        Property() = delete;

        ~Property() noexcept = delete;

        static const PropertyDescriptor
        add(Txn &txn, const std::string &className, const std::string &propertyName, PropertyType type);

        static void alter(Txn &txn, const std::string &className, const std::string &oldPropertyName,
                          const std::string &newPropertyName);

        static void remove(Txn &txn, const std::string &className, const std::string &propertyName);

        static void
        createIndex(Txn &txn, const std::string &className, const std::string &propertyName, bool isUnique = false);

        static void dropIndex(Txn &txn, const std::string &className, const std::string &propertyName);
    };

    //*************************************************************
    //*  NogDB vertex operations.                                 *
    //*************************************************************

    struct Vertex {
        Vertex() = delete;

        ~Vertex() noexcept = delete;

        static const RecordDescriptor create(Txn &txn, const std::string &className, const Record &record = Record{});

        static void update(Txn &txn, const RecordDescriptor &recordDescriptor, const Record &record);

        static void destroy(Txn &txn, const RecordDescriptor &recordDescriptor);

        static void destroy(Txn &txn, const std::string &className);

        static ResultSet get(Txn &txn, const std::string &className);

        static ResultSetCursor getCursor(Txn &txn, const std::string &className);

        static ResultSet getInEdge(Txn &txn, const RecordDescriptor &recordDescriptor,
                                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSet getOutEdge(Txn &txn, const RecordDescriptor &recordDescriptor,
                                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet getAllEdge(Txn &txn, const RecordDescriptor &recordDescriptor,
                                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor getInEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor,
                                               const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor getOutEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor,
                                                const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor getAllEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor,
                                                const ClassFilter &classFilter = ClassFilter{});

        static ResultSet get(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet get(Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSet get(Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet getIndex(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet getIndex(Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor getCursor(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor getCursor(Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSetCursor getCursor(Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor getIndexCursor(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor getIndexCursor(Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet
        getInEdge(Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition,
                  const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        getInEdge(Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &),
                  const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        getInEdge(Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition,
                  const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        getOutEdge(Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition,
                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        getOutEdge(Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &),
                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        getOutEdge(Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition,
                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        getAllEdge(Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition,
                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        getAllEdge(Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &),
                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        getAllEdge(Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition,
                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        getInEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition,
                        const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        getInEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &),
                        const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        getInEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition,
                        const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        getOutEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition,
                         const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        getOutEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &),
                         const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        getOutEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition,
                         const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        getAllEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition,
                         const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        getAllEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &),
                         const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        getAllEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition,
                         const ClassFilter &classFilter = ClassFilter{});
    };

    //*************************************************************
    //*  NogDB edge operations.                                   *
    //*************************************************************

    struct Edge {
        Edge() = delete;

        ~Edge() noexcept = delete;

        static const RecordDescriptor
        create(Txn &txn, const std::string &className, const RecordDescriptor &srcVertexRecordDescriptor,
               const RecordDescriptor &dstVertexRecordDescriptor, const Record &record = Record{});

        static void update(Txn &txn, const RecordDescriptor &recordDescriptor, const Record &record);

        static void updateSrc(Txn &txn, const RecordDescriptor &recordDescriptor,
                              const RecordDescriptor &newSrcVertexRecordDescriptor);

        static void
        updateDst(Txn &txn, const RecordDescriptor &recordDescriptor, const RecordDescriptor &newDstRecordDescriptor);

        static void destroy(Txn &txn, const RecordDescriptor &recordDescriptor);

        static void destroy(Txn &txn, const std::string &className);

        static ResultSet get(Txn &txn, const std::string &className);

        static ResultSet getExtend(Txn &txn, const std::string &className);

        static ResultSetCursor getCursor(Txn &txn, const std::string &className);

        static ResultSetCursor getExtendCursor(Txn& txn, const std::string& className);

        static Result getSrc(Txn &txn, const RecordDescriptor &recordDescriptor);

        static Result getDst(Txn &txn, const RecordDescriptor &recordDescriptor);

        static ResultSet getSrcDst(Txn &txn, const RecordDescriptor &recordDescriptor);

        static ResultSet get(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet get(Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSet get(Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet getExtend(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet getExtend(Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSet getExtend(Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet getIndex(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet getIndex(Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet getExtendIndex(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet getExtendIndex(Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor getCursor(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor getCursor(Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSetCursor getCursor(Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor getExtendCursor(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor getExtendCursor(Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSetCursor getExtendCursor(Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor getIndexCursor(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor getIndexCursor(Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor getExtendIndexCursor(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor getExtendIndexCursor(Txn &txn, const std::string &className, const MultiCondition &exp);
    };

    //*************************************************************
    //*  NogDB database operations.                               *
    //*************************************************************

    struct DB {
        DB() = delete;

        ~DB() noexcept = delete;

        static Record getRecord(Txn &txn, const RecordDescriptor &recordDescriptor);

        static const std::vector<ClassDescriptor> getClasses(Txn &txn);

        static const std::vector<PropertyDescriptor> getProperties(Txn &txn, const ClassDescriptor& classDescriptor);

        static const ClassDescriptor getClass(Txn &txn, const std::string &className);

        static const ClassDescriptor getClass(Txn &txn, const ClassId &classId);
    };

    //*************************************************************
    //*  NogDB graph traversal operations.                        *
    //*************************************************************

    struct Traverse {
        Traverse() = delete;

        ~Traverse() noexcept = delete;

        static ResultSet inEdgeBfs(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                   unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSet inEdgeBfs(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                   unsigned int maxDepth, const PathFilter &pathFilter,
                                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSet outEdgeBfs(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSet outEdgeBfs(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const PathFilter &pathFilter,
                                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet allEdgeBfs(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSet allEdgeBfs(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const PathFilter &pathFilter,
                                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet inEdgeDfs(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                   unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSet inEdgeDfs(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                   unsigned int maxDepth, const PathFilter &pathFilter,
                                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSet outEdgeDfs(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSet outEdgeDfs(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const PathFilter &pathFilter,
                                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet allEdgeDfs(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSet allEdgeDfs(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const PathFilter &pathFilter,
                                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet shortestPath(Txn &txn, const RecordDescriptor &srcVertexRecordDescriptor,
                                      const RecordDescriptor &dstVertexRecordDescriptor,
                                      const ClassFilter &classFilter = ClassFilter{});

        static ResultSet shortestPath(Txn &txn, const RecordDescriptor &srcVertexRecordDescriptor,
                                      const RecordDescriptor &dstVertexRecordDescriptor, const PathFilter &pathFilter,
                                      const ClassFilter &classFilter = ClassFilter{});

        /*
        template<typename CostFuncType, typename T = typename std::result_of<CostFuncType(Txn&, const RecordDescriptor&)>::type, typename CompareT = std::greater<T>>
        static std::pair<T, ResultSet> shortestPath(Txn &txn,
                                         const RecordDescriptor &srcVertexRecordDescriptor,
                                         const RecordDescriptor &dstVertexRecordDescriptor,
                                         const CostFuncType &costFunction,
                                         const PathFilter &pathFilter,
                                         const ClassFilter &classFilter = ClassFilter{}) {
            Generic::getClassInfo(txn, srcVertexRecordDescriptor.rid.first, ClassType::VERTEX);
            Generic::getClassInfo(txn, dstVertexRecordDescriptor.rid.first, ClassType::VERTEX);
            auto edgeClassIds = Generic::getEdgeClassId(txn, classFilter.getClassName());
            return Algorithm::dijkstraShortestPath<CostFuncType, T, CompareT>(
                    txn, srcVertexRecordDescriptor,
                    dstVertexRecordDescriptor, costFunction, edgeClassIds, pathFilter);
        }
         */

        static ResultSetCursor
        inEdgeBfsCursor(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                        unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        inEdgeBfsCursor(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                        unsigned int maxDepth, const PathFilter &pathFilter,
                        const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        outEdgeBfsCursor(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                         unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        outEdgeBfsCursor(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                         unsigned int maxDepth, const PathFilter &pathFilter,
                         const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        allEdgeBfsCursor(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                         unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        allEdgeBfsCursor(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                         unsigned int maxDepth, const PathFilter &pathFilter,
                         const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        inEdgeDfsCursor(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                        unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        inEdgeDfsCursor(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                        unsigned int maxDepth, const PathFilter &pathFilter,
                        const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        outEdgeDfsCursor(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                         unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        outEdgeDfsCursor(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                         unsigned int maxDepth, const PathFilter &pathFilter,
                         const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        allEdgeDfsCursor(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                         unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        allEdgeDfsCursor(Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                         unsigned int maxDepth, const PathFilter &pathFilter,
                         const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor shortestPathCursor(Txn &txn, const RecordDescriptor &srcVertexRecordDescriptor,
                                                  const RecordDescriptor &dstVertexRecordDescriptor,
                                                  const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor shortestPathCursor(Txn &txn, const RecordDescriptor &srcVertexRecordDescriptor,
                                                  const RecordDescriptor &dstVertexRecordDescriptor,
                                                  const PathFilter &pathFilter,
                                                  const ClassFilter &classFilter = ClassFilter{});

        /*
        template<typename CostFuncType, typename T = typename std::result_of<CostFuncType(Txn&, const RecordDescriptor&)>::type, typename CompareT = std::greater<T>>
        static std::pair<T, ResultSetCursor> shortestPathCursor(Txn &txn,
                                                                const RecordDescriptor &srcVertexRecordDescriptor,
                                                                const RecordDescriptor &dstVertexRecordDescriptor,
                                                                const CostFuncType &costFunction,
                                                                const PathFilter &pathFilter,
                                                                const ClassFilter &classFilter = ClassFilter{}) {
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

#endif
