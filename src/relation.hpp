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
#include <unordered_set>

#include "datarecord_adapter.hpp"
#include "parser.hpp"
#include "relation_adapter.hpp"
#include "storage_engine.hpp"
#include "utils.hpp"

namespace nogdb {
namespace relation {
    using namespace adapter::schema;
    using namespace adapter::relation;
    using namespace adapter::datarecord;
    using utils::caching::UnorderedCache;

    class GraphUtils {
    public:
        explicit GraphUtils(const storage_engine::LMDBTxn* txn, bool isVersionEnabled)
            : _txn { txn }
            , _inRel { new RelationAccess(txn, Direction::IN) }
            , _outRel { new RelationAccess(txn, Direction::OUT) }
            , _isVersionEnabled { isVersionEnabled }
        {
        }

        virtual ~GraphUtils() noexcept
        {
            if (_inRel) {
                delete _inRel;
                _inRel = nullptr;
            }
            if (_outRel) {
                delete _outRel;
                _outRel = nullptr;
            }
        };

        void addRel(const RecordId& edgeRid, const RecordId& srcRid, const RecordId& dstRid);

        void updateSrcRel(const RecordId& edgeRid,
            const RecordId& newSrcRid,
            const RecordId& srcRid,
            const RecordId& dstRid);

        void updateDstRel(const RecordId& edgeRid,
            const RecordId& newDstRid,
            const RecordId& srcRid,
            const RecordId& dstRid);

        void removeRelFromEdge(const RecordId& edgeRid, const RecordId& srcRid, const RecordId& dstRid);

        std::unordered_set<RecordId, RecordIdHash> removeRelFromVertex(const RecordId& rid);

        std::vector<RecordId> getInEdges(const RecordId& recordId) const;

        std::vector<RecordId> getOutEdges(const RecordId& recordId) const;

        std::vector<std::pair<RecordId, RecordId>> getInEdgeAndNeighbours(const RecordId& recordId) const;

        std::vector<std::pair<RecordId, RecordId>> getOutEdgeAndNeighbours(const RecordId& recordId) const;

        std::pair<RecordId, RecordId> getSrcDstVertices(const RecordId& recordId) const;

    private:
        const storage_engine::LMDBTxn* _txn;
        RelationAccess* _inRel;
        RelationAccess* _outRel;
        bool _isVersionEnabled;

        using InternalCache = UnorderedCache<ClassId, std::shared_ptr<DataRecord>>;
        InternalCache _edgeDataRecordCache {};
    };
}
}
