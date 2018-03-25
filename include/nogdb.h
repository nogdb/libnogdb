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

#ifndef __NOGDB_H_INCLUDED_
#define __NOGDB_H_INCLUDED_

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <utility>

#include "nogdb_compare.h"
#include "nogdb_error.h"
#include "nogdb_types.h"
#include "nogdb_context.h"
#include "nogdb_txn.h"

namespace nogdb {

    //*************************************************************
    //*  NogDB class operations.                                  *
    //*************************************************************

    struct Class {
        Class() = delete;

        ~Class() noexcept = delete;

        static const ClassDescriptor create(Txn &txn, const std::string &className, ClassType type,
                                            const PropertyMapType &properties = PropertyMapType{});

        static const ClassDescriptor createExtend(Txn &txn, const std::string &className, const std::string &superClass,
                                                  const PropertyMapType &properties = PropertyMapType{});

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

        static ResultSet get(const Txn &txn, const std::string &className);

        static ResultSetCursor getCursor(Txn &txn, const std::string &className);

        static ResultSet getInEdge(const Txn &txn, const RecordDescriptor &recordDescriptor,
                                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSet getOutEdge(const Txn &txn, const RecordDescriptor &recordDescriptor,
                                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet getAllEdge(const Txn &txn, const RecordDescriptor &recordDescriptor,
                                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor getInEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor,
                                               const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor getOutEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor,
                                                const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor getAllEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor,
                                                const ClassFilter &classFilter = ClassFilter{});

        static ResultSet find(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet find(const Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSet find(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet findIndex(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet findIndex(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor findCursor(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor findCursor(Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSetCursor findCursor(Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor findCursorIndex(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor findCursorIndex(Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet
        findInEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition,
                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        findInEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &),
                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        findInEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition,
                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        findOutEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition,
                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        findOutEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &),
                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        findOutEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition,
                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        findAllEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition,
                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        findAllEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &),
                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet
        findAllEdge(const Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition,
                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        findInEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition,
                         const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        findInEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &),
                         const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        findInEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition,
                         const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        findOutEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition,
                          const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        findOutEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &),
                          const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        findOutEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition,
                          const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        findAllEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, const Condition &condition,
                          const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        findAllEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, bool (*condition)(const Record &),
                          const ClassFilter &classFilter = ClassFilter{});

        static ResultSetCursor
        findAllEdgeCursor(Txn &txn, const RecordDescriptor &recordDescriptor, const MultiCondition &multiCondition,
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

        static ResultSet get(const Txn &txn, const std::string &className);

        static ResultSetCursor getCursor(Txn &txn, const std::string &className);

        static Result getSrc(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static Result getDst(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static ResultSet getSrcDst(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static ResultSet find(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet find(const Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSet find(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSet findIndex(const Txn &txn, const std::string &className, const Condition &condition);

        static ResultSet findIndex(const Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor findCursor(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor findCursor(Txn &txn, const std::string &className, bool (*condition)(const Record &));

        static ResultSetCursor findCursor(Txn &txn, const std::string &className, const MultiCondition &exp);

        static ResultSetCursor findCursorIndex(Txn &txn, const std::string &className, const Condition &condition);

        static ResultSetCursor findCursorIndex(Txn &txn, const std::string &className, const MultiCondition &exp);
    };

    //*************************************************************
    //*  NogDB database operations.                               *
    //*************************************************************

    struct Db {
        Db() = delete;

        ~Db() noexcept = delete;

        static Record getRecord(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static const std::vector<ClassDescriptor> getSchema(const Txn &txn);

        static const ClassDescriptor getSchema(const Txn &txn, const std::string &className);

        static const ClassDescriptor getSchema(const Txn &txn, const ClassId &classId);

        static const DBInfo getDbInfo(const Txn &txn);
    };

    //*************************************************************
    //*  NogDB graph traversal operations.                        *
    //*************************************************************

    struct Traverse {
        static ResultSet inEdgeBfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                   unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSet inEdgeBfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                   unsigned int maxDepth, const PathFilter &pathFilter,
                                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSet outEdgeBfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSet outEdgeBfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const PathFilter &pathFilter,
                                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet allEdgeBfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSet allEdgeBfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const PathFilter &pathFilter,
                                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet inEdgeDfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                   unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSet inEdgeDfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                   unsigned int maxDepth, const PathFilter &pathFilter,
                                   const ClassFilter &classFilter = ClassFilter{});

        static ResultSet outEdgeDfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSet outEdgeDfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const PathFilter &pathFilter,
                                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet allEdgeDfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const ClassFilter &classFilter = ClassFilter{});

        static ResultSet allEdgeDfs(const Txn &txn, const RecordDescriptor &recordDescriptor, unsigned int minDepth,
                                    unsigned int maxDepth, const PathFilter &pathFilter,
                                    const ClassFilter &classFilter = ClassFilter{});

        static ResultSet shortestPath(const Txn &txn, const RecordDescriptor &srcVertexRecordDescriptor,
                                      const RecordDescriptor &dstVertexRecordDescriptor,
                                      const ClassFilter &classFilter = ClassFilter{});

        static ResultSet shortestPath(const Txn &txn, const RecordDescriptor &srcVertexRecordDescriptor,
                                      const RecordDescriptor &dstVertexRecordDescriptor, const PathFilter &pathFilter,
                                      const ClassFilter &classFilter = ClassFilter{});

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

    };
}

#endif
