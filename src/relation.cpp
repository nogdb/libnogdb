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

#include "relation.hpp"

namespace nogdb {
namespace relation {
    using parser::RecordParser;

    void GraphUtils::addRel(const RecordId& edgeRid,
        const RecordId& srcRid,
        const RecordId& dstRid)
    {
        _outRel->create(RelationAccessInfo { srcRid, edgeRid, dstRid });
        _inRel->create(RelationAccessInfo { dstRid, edgeRid, srcRid });
    }

    void GraphUtils::updateSrcRel(const RecordId& edgeRid,
        const RecordId& newSrcRid,
        const RecordId& srcRid,
        const RecordId& dstRid)
    {
        _outRel->removeByCursor(RelationAccessInfo { srcRid, edgeRid, dstRid });
        _outRel->create(RelationAccessInfo { newSrcRid, edgeRid, dstRid });
        _inRel->removeByCursor(RelationAccessInfo { dstRid, edgeRid, srcRid });
        _inRel->create(RelationAccessInfo { dstRid, edgeRid, newSrcRid });
    }

    void GraphUtils::updateDstRel(const RecordId& edgeRid,
        const RecordId& newDstRid,
        const RecordId& srcRid,
        const RecordId& dstRid)
    {
        _outRel->removeByCursor(RelationAccessInfo { srcRid, edgeRid, dstRid });
        _outRel->create(RelationAccessInfo { srcRid, edgeRid, newDstRid });
        _inRel->removeByCursor(RelationAccessInfo { dstRid, edgeRid, srcRid });
        _inRel->create(RelationAccessInfo { newDstRid, edgeRid, srcRid });
    }

    void GraphUtils::removeRelFromEdge(const RecordId& edgeRid, const RecordId& srcRid, const RecordId& dstRid)
    {
        _inRel->removeByCursor(RelationAccessInfo { dstRid, edgeRid, srcRid });
        _outRel->removeByCursor(RelationAccessInfo { srcRid, edgeRid, dstRid });
    }

    std::unordered_set<RecordId, RecordIdHash> GraphUtils::removeRelFromVertex(const RecordId& rid)
    {
        auto neighbours = std::unordered_set<RecordId, RecordIdHash> {};
        // source
        for (const auto& relInfo : _inRel->getInfos(rid)) {
            std::function<std::shared_ptr<DataRecord>(void)> callback = [&]() {
                return std::make_shared<DataRecord>(_txn, relInfo.edgeId.first, ClassType::EDGE);
            };
            try {
                _edgeDataRecordCache.get(relInfo.edgeId.first, callback)->remove(relInfo.edgeId.second);
            } catch (const Error& err) {
                if (err.code() != NOGDB_CTX_NOEXST_RECORD) {
                    throw err;
                }
            }
            _outRel->removeByCursor(RelationAccessInfo { relInfo.neighborId, relInfo.edgeId, rid });
            neighbours.insert(relInfo.neighborId);
        }
        _inRel->remove(rid);
        // destination
        for (const auto& relInfo : _outRel->getInfos(rid)) {
            std::function<std::shared_ptr<DataRecord>(void)> callback = [&]() {
                return std::make_shared<DataRecord>(_txn, relInfo.edgeId.first, ClassType::EDGE);
            };
            try {
                _edgeDataRecordCache.get(relInfo.edgeId.first, callback)->remove(relInfo.edgeId.second);
            } catch (const Error& err) {
                if (err.code() != NOGDB_CTX_NOEXST_RECORD) {
                    throw err;
                }
            }
            _inRel->removeByCursor(RelationAccessInfo { relInfo.neighborId, relInfo.edgeId, rid });
            neighbours.insert(relInfo.neighborId);
        }
        _outRel->remove(rid);

        return neighbours;
    }

    std::vector<RecordId> GraphUtils::getInEdges(const RecordId& recordId) const
    {
        return _inRel->getEdges(recordId);
    }

    std::vector<RecordId> GraphUtils::getOutEdges(const RecordId& recordId) const
    {
        return _outRel->getEdges(recordId);
    }

    std::vector<std::pair<RecordId, RecordId>> GraphUtils::getInEdgeAndNeighbours(const RecordId& recordId) const
    {
        return _inRel->getEdgeAndNeighbours(recordId);
    }

    std::vector<std::pair<RecordId, RecordId>> GraphUtils::getOutEdgeAndNeighbours(const RecordId& recordId) const
    {
        return _outRel->getEdgeAndNeighbours(recordId);
    }

    std::pair<RecordId, RecordId> GraphUtils::getSrcDstVertices(const RecordId& recordId) const
    {
        std::function<std::shared_ptr<DataRecord>(void)> callback = [&]() {
            return std::make_shared<DataRecord>(_txn, recordId.first, ClassType::EDGE);
        };
        auto edgeDataRecord = _edgeDataRecordCache.get(recordId.first, callback);
        auto rawData = edgeDataRecord->getResult(recordId.second);
        return RecordParser::parseEdgeRawDataVertexSrcDst(rawData, _isVersionEnabled);
    }
}
}
