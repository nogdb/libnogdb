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

#pragma once

#include <functional>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "compare.hpp"
#include "constant.hpp"
#include "datarecord.hpp"
#include "lmdb_engine.hpp"
#include "parser.hpp"
#include "relation.hpp"
#include "schema.hpp"

#include "nogdb/nogdb.h"
#include "nogdb/nogdb_types.h"

namespace nogdb {

namespace algorithm {

    class GraphTraversal {
    public:
        GraphTraversal() = delete;

        ~GraphTraversal() noexcept = delete;

        static ResultSet breadthFirstSearch(const Transaction& txn,
            const schema::ClassAccessInfo& classInfo,
            const RecordDescriptor& recordDescriptor,
            unsigned int minDepth,
            unsigned int maxDepth,
            const adapter::relation::Direction& direction,
            const GraphFilter& edgeFilter,
            const GraphFilter& vertexFilter);

        static std::vector<RecordDescriptor>
        breadthFirstSearchRdesc(const Transaction& txn,
            const schema::ClassAccessInfo& classInfo,
            const RecordDescriptor& recordDescriptor,
            unsigned int minDepth,
            unsigned int maxDepth,
            const adapter::relation::Direction& direction,
            const GraphFilter& edgeFilter,
            const GraphFilter& vertexFilter);

        static ResultSet depthFirstSearch(const Transaction& txn,
            const schema::ClassAccessInfo& classInfo,
            const RecordDescriptor& recordDescriptor,
            unsigned int minDepth,
            unsigned int maxDepth,
            const adapter::relation::Direction& direction,
            const GraphFilter& edgeFilter,
            const GraphFilter& vertexFilter);

        static std::vector<RecordDescriptor>
        depthFirstSearchRdesc(const Transaction& txn,
            const schema::ClassAccessInfo& classInfo,
            const RecordDescriptor& recordDescriptor,
            unsigned int minDepth,
            unsigned int maxDepth,
            const adapter::relation::Direction& direction,
            const GraphFilter& edgeFilter,
            const GraphFilter& vertexFilter);

        static ResultSet bfsShortestPath(const Transaction& txn,
            const schema::ClassAccessInfo& srcVertexClassInfo,
            const schema::ClassAccessInfo& dstVertexClassInfo,
            const RecordDescriptor& srcVertexRecordDescriptor,
            const RecordDescriptor& dstVertexRecordDescriptor,
            const GraphFilter& edgeFilter,
            const GraphFilter& vertexFilter);

        static std::vector<RecordDescriptor>
        bfsShortestPathRdesc(const Transaction& txn,
            const schema::ClassAccessInfo& srcVertexClassInfo,
            const schema::ClassAccessInfo& dstVertexClassInfo,
            const RecordDescriptor& srcVertexRecordDescriptor,
            const RecordDescriptor& dstVertexRecordDescriptor,
            const GraphFilter& edgeFilter,
            const GraphFilter& vertexFilter);
    };
}
}