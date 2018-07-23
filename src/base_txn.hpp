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

#ifndef __BASE_TXN_HPP_INCLUDED_
#define __BASE_TXN_HPP_INCLUDED_

#include <atomic>

#include "storage_engine.hpp"
#include "lmdb_engine.hpp"
#include "txn_object.hpp"
#include "graph.hpp"
#include "schema.hpp"
#include "spinlock.hpp"
#include "version_control.hpp"

#include "nogdb_types.h"
#include "nogdb_context.h"

namespace nogdb {

    struct TxnStat;

    class BaseTxn {
    public:
        enum TxnType {
            READ_ONLY, READ_WRITE
        };

        BaseTxn(Context &ctx, bool isReadWrite, bool inMemory = false);

        ~BaseTxn() noexcept {
            if (isWithDataStore && !isCompleted) {
                dsTxnHandler->rollback();
            }
            if (dsTxnHandler) {
                delete dsTxnHandler;
                dsTxnHandler = nullptr;
            }
        }

        BaseTxn(const BaseTxn &txn) = delete;

        BaseTxn &operator=(const BaseTxn &txn) = delete;

        storage_engine::LMDBTxn *getDsTxnHandler() const { return dsTxnHandler; }

        const TxnId &getVersionId() const { return versionId; }

        TxnType getType() const { return txnType; }

        const TxnId &getTxnId() const { return txnId; }

        void addUncommittedVertex(const std::shared_ptr<Graph::Vertex> &vertex);

        void addUncommittedEdge(const std::shared_ptr<Graph::Edge> &edge);

        void deleteUncommittedVertex(const RecordId &rid);

        void deleteUncommittedEdge(const RecordId &rid);

        void addUncommittedSchema(const std::shared_ptr<Schema::ClassDescriptor> &classPtr);

        void deleteUncommittedSchema(const ClassId &classId);

        bool commit(Context &ctx);

        bool rollback(Context &ctx) noexcept;

        bool isNotCompleted() const { return !isCompleted; }

        std::shared_ptr<Graph::Vertex> findUncommittedVertex(const RecordId &rid) const {
            auto iterator = ucVertices.find(rid);
            return (iterator == ucVertices.cend()) ? nullptr : iterator->second;
        }

        std::shared_ptr<Graph::Edge> findUncommittedEdge(const RecordId &rid) const {
            auto iterator = ucEdges.find(rid);
            return (iterator == ucEdges.cend()) ? nullptr : iterator->second;
        }

        const Schema::SchemaElements<ClassId, Schema::ClassDescriptor> &findUncommittedSchema() const {
            return ucSchema;
        }

        Schema::ClassDescriptorPtr findUncommittedSchema(const ClassId &classId) const {
            auto iterator = ucSchema.find(classId);
            if (iterator == ucSchema.cend()) {
                return nullptr;
            } else {
                return iterator->second;
            }
        }

        template<typename T>
        inline static std::pair<T, bool> getCurrentVersion(const BaseTxn &txn, const VersionControl<T> &element) {
            return (txn.getType() == BaseTxn::TxnType::READ_ONLY) ?
                   element.getStableVersion(txn.getVersionId()) : element.getLatestVersion();
        };

        DBInfo dbInfo{};

    private:
        storage_engine::LMDBTxn *dsTxnHandler{nullptr};
        TxnId txnId;
        TxnId versionId;
        TxnType txnType;
        Schema::SchemaElements<ClassId, Schema::ClassDescriptor> ucSchema;
        Graph::GraphElements<Graph::Vertex> ucVertices;
        Graph::GraphElements<Graph::Edge> ucEdges;

        bool isWithDataStore;
        bool isCompleted{false}; // throw error if working with isCompleted = true
        bool isCommitDatastore{false};

    };

}

#endif
