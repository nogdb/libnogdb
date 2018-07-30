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

#ifndef __GRAPH_HPP_INCLUDED_
#define __GRAPH_HPP_INCLUDED_

#include <vector>
#include <unordered_map>
#include <set>
#include <memory>
#include <sstream>
#include <utility>
#include <cstdint>
#include <atomic>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <mutex>

#include "boost/functional/hash.hpp"
#include "multiversion_hashmap.hpp"
#include "concurrent.hpp"
#include "txn_object.hpp"

#include "nogdb_errors.h"
#include "nogdb_types.h"

namespace nogdb {

    class BaseTxn;

    struct Graph {
        Graph() = default;

        ~Graph() noexcept = default;

        typedef boost::hash<RecordId> RecordIdHash;

        template<typename T>
        using GraphElements = std::unordered_map<RecordId, std::shared_ptr<T>, RecordIdHash>;

        template<typename T>
        using ConcurrentGraphElements = ConcurrentHashMap<GraphElements<T>, RecordId, T>;

        struct Edge;

        struct Vertex : public TxnObject {
            Vertex(RecordId rid_)
                    : TxnObject{}, rid{rid_} {};
            const RecordId rid;

            TwoLevelMultiVersionHashMap<ClassId, PositionId, std::weak_ptr<Edge>> in{};
            TwoLevelMultiVersionHashMap<ClassId, PositionId, std::weak_ptr<Edge>> out{};
        };

        struct Edge : public TxnObject {
            Edge(RecordId rid_, const std::weak_ptr<Vertex> source_, const std::weak_ptr<Vertex> target_)
                    : TxnObject{}, rid{rid_} {
                source.addLatestVersion(source_);
                target.addLatestVersion(target_);
            }

            const RecordId rid;

            VersionControl<std::weak_ptr<Vertex>> source{};
            VersionControl<std::weak_ptr<Vertex>> target{};
        };

        ConcurrentGraphElements<Vertex> vertices{};
        ConcurrentGraphElements<Edge> edges{};
        ConcurrentDeleteQueue<RecordId> deletedVertices;
        ConcurrentDeleteQueue<RecordId> deletedEdges;

        // return false if there is an existing vertex in a graph, otherwise, true
        bool createVertex(BaseTxn &txn, const RecordId &rid);

        void deleteVertex(BaseTxn &txn, const RecordId &rid) noexcept;

        void forceDeleteVertex(const RecordId &rid) noexcept;

        void forceDeleteVertices(const std::vector<RecordId> &rids) noexcept;

        std::vector<RecordId> getEdgeIn(const BaseTxn &txn, const RecordId &rid, const ClassId &classId = 0);

        std::vector<ClassId> getEdgeClassIn(const BaseTxn &txn, const RecordId &rid);

        std::vector<RecordId> getEdgeOut(const BaseTxn &txn, const RecordId &rid, const ClassId &classId = 0);

        std::vector<ClassId> getEdgeClassOut(const BaseTxn &txn, const RecordId &rid);

        std::vector<RecordId> getEdgeInOut(const BaseTxn &txn, const RecordId &rid, const ClassId &classId = 0);

        std::vector<ClassId> getEdgeClassInOut(const BaseTxn &txn, const RecordId &rid);

        std::shared_ptr<Vertex> lookupVertex(const BaseTxn &txn, const RecordId &rid);

        void createEdge(BaseTxn &txn, const RecordId &rid, const RecordId &srcRid, const RecordId &dstRid);

        void deleteEdge(BaseTxn &txn, const RecordId &rid) noexcept;

        void forceDeleteEdge(const RecordId &rid) noexcept;

        void forceDeleteEdges(const std::vector<RecordId> &rids) noexcept;

        RecordId getVertexSrc(const BaseTxn &txn, const RecordId &rid);

        RecordId getVertexDst(const BaseTxn &txn, const RecordId &rid);

        // return a pair of source and destination vertices rid
        std::pair<RecordId, RecordId> getVertexSrcDst(const BaseTxn &txn, const RecordId &rid);

        void alterVertexSrc(BaseTxn &txn, const RecordId &rid, const RecordId &srcRid);

        void alterVertexDst(BaseTxn &txn, const RecordId &rid, const RecordId &dstRid);

        std::shared_ptr<Edge> lookupEdge(const BaseTxn &txn, const RecordId &rid);

        //NOTE: should be called only when the transaction is safe.
        inline void clear() noexcept {
            edges.lockAndClear();
            vertices.lockAndClear();
        }

        inline void clearDeletedElements(TxnId versionId) {
            forceDeleteEdges(deletedEdges.pop_front(versionId));
            forceDeleteVertices(deletedVertices.pop_front(versionId));
        }
    };

    inline std::string rid2str(const RecordId &rid) {
        std::stringstream ss{};
        ss << std::to_string(rid.first) << ":" << std::to_string(rid.second);
        return ss.str();
    }

}

#endif

