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

#ifndef __RELATION_HPP_INCLUDED_
#define __RELATION_HPP_INCLUDED_

#include "storage_engine.hpp"
#include "relation_adapter.hpp"

namespace nogdb {
    namespace relation {

        using adapter::relation::Direction;

        class GraphInterface {
        public:
            GraphInterface(const storage_engine::LMDBTxn *const txn)
                    : _txn{txn},
                      _inRel{adapter::relation::RelationAccess(txn, Direction::IN)},
                      _outRel{adapter::relation::RelationAccess(txn, Direction::OUT)} {}

            ~GraphInterface() noexcept = default;

            void addVertex(const RecordId& rid, const Record& record) {
                auto dsHandler = openRecordDb(rid);
            }

            void addEdge(const RecordId& rid, const RecordDescriptor& src, const RecordDescriptor& dst, const Record& record) {

            }

            void removeVertex(const RecordId& rid) {

            }

            void removeEdge(const RecordId& rid) {

            }

        private:
            const storage_engine::LMDBTxn* _txn;
            adapter::relation::RelationAccess _inRel;
            adapter::relation::RelationAccess _outRel;
        };
    }
}

#endif //__RELATION_HPP_INCLUDED_
