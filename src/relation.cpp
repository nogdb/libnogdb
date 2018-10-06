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

#include "relation.hpp"

namespace nogdb {

  namespace relation {

    void GraphInterface::addRel(const RecordId &edgeRid,
                                const RecordId &srcRid,
                                const RecordId &dstRid) {
      _outRel->create(RelationAccessInfo{srcRid, edgeRid, dstRid});
      _inRel->create(RelationAccessInfo{dstRid, edgeRid, srcRid});
    }

    void GraphInterface::updateSrcRel(const RecordId &edgeRid,
                                      const RecordId &newSrcRid,
                                      const RecordId &srcRid,
                                      const RecordId &dstRid) {
      _outRel->remove(RelationAccessInfo{srcRid, edgeRid, dstRid});
      _outRel->create(RelationAccessInfo{newSrcRid, edgeRid, dstRid});
      _inRel->remove(RelationAccessInfo{dstRid, edgeRid, srcRid});
      _inRel->create(RelationAccessInfo{dstRid, edgeRid, newSrcRid});
    }

    void GraphInterface::updateDstRel(const RecordId &edgeRid,
                                      const RecordId &newDstRid,
                                      const RecordId &srcRid,
                                      const RecordId &dstRid) {
      _outRel->remove(RelationAccessInfo{srcRid, edgeRid, dstRid});
      _outRel->create(RelationAccessInfo{srcRid, edgeRid, newDstRid});
      _inRel->remove(RelationAccessInfo{dstRid, edgeRid, srcRid});
      _inRel->create(RelationAccessInfo{newDstRid, edgeRid, srcRid});
    }

    void GraphInterface::removeRelFromEdge(const RecordId &edgeRid, const RecordId &srcRid, const RecordId &dstRid) {
      _inRel->remove(RelationAccessInfo{dstRid, edgeRid, srcRid});
      _outRel->remove(RelationAccessInfo{srcRid, edgeRid, dstRid});
    }

    void GraphInterface::removeRelFromVertex(const RecordId &rid) {
      // source
      for (const auto &relInfo: _inRel->getInfos(rid)) {
        std::function<std::shared_ptr<DataRecord>(void)> callback = [&]() {
          return std::make_shared<DataRecord>(_txn->_txnBase, relInfo.edgeId.first, ClassType::EDGE);
        };
        try {
          _edgeDataRecordCache.get(relInfo.edgeId.first, callback)->remove(relInfo.edgeId.second);
        } catch (const Error &err) {
          if (err.code() != NOGDB_CTX_NOEXST_RECORD) {
            throw err;
          }
        }
        _outRel->remove(RelationAccessInfo{relInfo.neighborId, relInfo.edgeId, rid});
      }
      _inRel->remove(rid);
      // destination
      for (const auto &relInfo: _outRel->getInfos(rid)) {
        std::function<std::shared_ptr<DataRecord>(void)> callback = [&]() {
          return std::make_shared<DataRecord>(_txn->_txnBase, relInfo.edgeId.first, ClassType::EDGE);
        };
        try {
          _edgeDataRecordCache.get(relInfo.edgeId.first, callback)->remove(relInfo.edgeId.second);
        } catch (const Error &err) {
          if (err.code() != NOGDB_CTX_NOEXST_RECORD) {
            throw err;
          }
        }
        _inRel->remove(RelationAccessInfo{relInfo.neighborId, relInfo.edgeId, rid});
      }
      _outRel->remove(rid);
    }

    std::vector<RecordId> GraphInterface::getInEdges(const RecordId &recordId) const {
      return _inRel->getEdges(recordId);
    }

    std::vector<RecordId> GraphInterface::getOutEdges(const RecordId &recordId) const {
      return _outRel->getEdges(recordId);
    }

    std::pair<RecordId, RecordId> GraphInterface::getSrcDstVertices(const RecordId &recordId) const {
      std::function<std::shared_ptr<DataRecord>(void)> callback = [&]() {
        return std::make_shared<DataRecord>(_txn->_txnBase, recordId.first, ClassType::EDGE);
      };
      try {
        auto edgeDataRecord = _edgeDataRecordCache.get(recordId.first, callback);
        auto rawData = edgeDataRecord->getBlob(recordId.second);
        return parser::RecordParser::parseEdgeRawDataVertexSrcDst(rawData);
      } catch (const Error &err) {
        if (err.code() != NOGDB_CTX_NOEXST_RECORD) {
          throw err;
        }
      }
      return {RecordId{}, RecordId{}};
    }

  }
}
