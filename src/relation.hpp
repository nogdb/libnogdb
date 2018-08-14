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

#include <memory>
#include <functional>

#include "storage_engine.hpp"
#include "relation_adapter.hpp"
#include "datarecord_adapter.hpp"
#include "utils.hpp"

namespace nogdb {

    namespace relation {

        using namespace adapter::relation;
        using namespace adapter::datarecord;

        class GraphInterface {
        public:
            explicit GraphInterface(const Txn* txn)
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

//            void addVertex(const RecordId& rid, const Record& record) {
//
//            }
//
//            void addEdge(const RecordId& rid, const RecordDescriptor& src, const RecordDescriptor& dst, const Record& record) {
//
//            }

            void removeRel(const RecordId& edgeRid, const RecordId& srcRid, const RecordId& dstRid) {
                _inRel->remove(RelationAccessInfo{dstRid, edgeRid, srcRid});
                _outRel->remove(RelationAccessInfo{srcRid, edgeRid, dstRid});
            }

            void removeRelFromVertex(const RecordId& rid) {
                // source
                for(const auto& relInfo: _inRel->getInfos(rid)) {
                    std::function<std::shared_ptr<DataRecord>(void)> callback = [&](){
                        return std::make_shared<DataRecord>(_txn->_txnBase, relInfo.edgeId.first, ClassType::EDGE);
                    };
                    auto edgeDataRecord = _edgeDataRecordCache.get(relInfo.edgeId.first, callback);
                    try {
                        edgeDataRecord->remove(relInfo.edgeId.second);
                    } catch (const Error & err){
                        if (err.code() != NOGDB_CTX_NOEXST_RECORD) {
                            throw err;
                        }
                    }
                    _outRel->remove(RelationAccessInfo{relInfo.neighborId, relInfo.edgeId, rid});
                }
                _inRel->remove(rid);
                // destination
                for(const auto& relInfo: _outRel->getInfos(rid)) {
                    std::function<std::shared_ptr<DataRecord>(void)> callback = [&](){
                        return std::make_shared<DataRecord>(_txn->_txnBase, relInfo.edgeId.first, ClassType::EDGE);
                    };
                    auto edgeDataRecord = _edgeDataRecordCache.get(relInfo.edgeId.first, callback);
                    try {
                        edgeDataRecord->remove(relInfo.edgeId.second);
                    } catch (const Error & err){
                        if (err.code() != NOGDB_CTX_NOEXST_RECORD) {
                            throw err;
                        }
                    }
                    _inRel->remove(RelationAccessInfo{relInfo.neighborId, relInfo.edgeId, rid});
                }
                _outRel->remove(rid);
            }


        private:
            const Txn* _txn;
            RelationAccess* _inRel;
            RelationAccess* _outRel;

            using InternalCache = utils::caching::UnorderedCache<ClassId, std::shared_ptr<DataRecord>>;
            InternalCache _edgeDataRecordCache{};
        };
    }
}

#endif //__RELATION_HPP_INCLUDED_
