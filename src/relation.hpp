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

#include <memory>
#include <functional>

#include "storage_engine.hpp"
#include "relation_adapter.hpp"
#include "datarecord_adapter.hpp"
#include "utils.hpp"
#include "parser.hpp"

namespace nogdb {

  namespace relation {

    using namespace adapter::schema;
    using namespace adapter::relation;
    using namespace adapter::datarecord;

    class GraphInterface {
    public:
      explicit GraphInterface(const Transaction *txn)
          : _txn{txn},
            _inRel{new RelationAccess(txn->_txnBase, Direction::IN)},
            _outRel{new RelationAccess(txn->_txnBase, Direction::OUT)} {}

      virtual ~GraphInterface() noexcept {
        if (_inRel) {
          delete _inRel;
          _inRel = nullptr;
        }
        if (_outRel) {
          delete _outRel;
          _outRel = nullptr;
        }
      };

      void addRel(const RecordId &edgeRid,
                  const RecordId &srcRid,
                  const RecordId &dstRid);

      void updateSrcRel(const RecordId &edgeRid,
                        const RecordId &newSrcRid,
                        const RecordId &srcRid,
                        const RecordId &dstRid);

      void updateDstRel(const RecordId &edgeRid,
                        const RecordId &newDstRid,
                        const RecordId &srcRid,
                        const RecordId &dstRid);

      void removeRelFromEdge(const RecordId &edgeRid, const RecordId &srcRid, const RecordId &dstRid);

      void removeRelFromVertex(const RecordId &rid);

      std::vector<RecordId> getInEdges(const RecordId &recordId) const;

      std::vector<RecordId> getOutEdges(const RecordId &recordId) const;

      std::pair<RecordId, RecordId> getSrcDstVertices(const RecordId& recordId) const;

    private:
      const Transaction *_txn;
      RelationAccess *_inRel;
      RelationAccess *_outRel;

      using InternalCache = utils::caching::UnorderedCache<ClassId, std::shared_ptr<DataRecord>>;
      InternalCache _edgeDataRecordCache{};
    };
  }
}
