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

#ifndef __DATARECORD_ADAPTER_HPP_INCLUDED_
#define __DATARECORD_ADAPTER_HPP_INCLUDED_

#include "storage_adapter.hpp"
#include "schema_adapter.hpp"

namespace nogdb {

    namespace adapter {

        namespace datarecord {

            class DataRecord : public storage_engine::adapter::LMDBKeyValAccess {
            public:
                DataRecord(const storage_engine::LMDBTxn *const txn,
                           const ClassId &classId,
                           const ClassType &classType = ClassType::UNDEFINED)
                          : _classId{classId},
                            _classType{classType},
                            LMDBKeyValAccess(txn, std::to_string(classId), true, true, true, true) {}

                virtual ~DataRecord() noexcept = default;

                DataRecord(DataRecord&& other) noexcept {
                    *this = std::move(other);
                }

                DataRecord& operator=(DataRecord&& other) noexcept {
                    if (this != &other) {
                        using std::swap;
                        swap(*this, other);
                    }
                    return *this;
                }

                void init() {
                    put(MAX_RECORD_NUM_EM, PositionId{1});
                }

                PositionId insert(const Blob &blob) {
                    auto result = get(MAX_RECORD_NUM_EM);
                    require(!result.empty);
                    auto posid = result.data.numeric<PositionId>();
                    put(posid, blob);
                    put(MAX_RECORD_NUM_EM, posid + PositionId{1});
                    return posid;
                }

                void update(const PositionId &posid, const Blob &blob) {
                    auto result = get(posid);
                    if (!result.empty) {
                        put(posid, blob);
                    } else {
                        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_RECORD);
                    }
                }

                void remove(const PositionId &posid) {
                    auto result = get(posid);
                    if (!result.empty) {
                        del(posid);
                    } else {
                        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_RECORD);
                    }
                }

                void destroy() {
                    drop(true);
                }

                Blob getBlob(const PositionId &posid) {
                    auto result = get(posid);
                    if (!result.empty) {
                        return result.data.blob();
                    } else {
                        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_RECORD);
                    }
                }

                storage_engine::lmdb::Result getResult(const PositionId &posid) {
                    auto result = get(posid);
                    if (!result.empty) {
                        return result;
                    } else {
                        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_RECORD);
                    }
                }

                storage_engine::lmdb::Cursor getCursor() const {
                    return cursor();
                }

            private:
                ClassId _classId{};
                ClassType _classType{ClassType::UNDEFINED};
            };

        }
    }
}

#endif //__DATARECORD_ADAPTER_HPP_INCLUDED_
