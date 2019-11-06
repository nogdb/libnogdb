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

#include "parser.hpp"
#include "schema.hpp"
#include "schema_adapter.hpp"
#include "storage_adapter.hpp"

namespace nogdb {
namespace adapter {
namespace datarecord {
    using namespace internal_data_type;
    using namespace utils::assertion;

    class DataRecord : public storage_engine::adapter::LMDBKeyValAccess {
    public:
        DataRecord(const storage_engine::LMDBTxn* const txn,
            const ClassId& classId,
            const ClassType& classType = ClassType::UNDEFINED)
            : LMDBKeyValAccess(txn, std::to_string(classId), true, true, false, true)
            , _classId { classId }
            , _classType { classType }
        {
        }

        virtual ~DataRecord() noexcept = default;

        DataRecord(DataRecord&& other) noexcept
            : LMDBKeyValAccess(std::move(other))
            , _classId { other._classId }
            , _classType { other._classType }
        {
        }

        DataRecord& operator=(DataRecord&& other) noexcept
        {
            if (this != &other) {
                using std::swap;
                swap(*this, other);
            }
            return *this;
        }

        void init()
        {
            put(MAX_RECORD_NUM_EM, PositionId { 1 });
        }

        PositionId insert(const Blob& blob)
        {
            auto result = get(MAX_RECORD_NUM_EM);
            require(!result.empty);
            auto posid = result.data.numeric<PositionId>();
            put(posid, blob);
            put(MAX_RECORD_NUM_EM, posid + PositionId { 1 });
            return posid;
        }

        void update(const PositionId& posid, const Blob& blob)
        {
            auto result = get(posid);
            if (!result.empty) {
                put(posid, blob);
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_RECORD);
            }
        }

        void remove(const PositionId& posid)
        {
            auto result = get(posid);
            if (!result.empty) {
                del(posid);
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_RECORD);
            }
        }

        void destroy()
        {
            drop(true);
        }

        Blob getBlob(const PositionId& posid)
        {
            auto result = get(posid);
            if (!result.empty) {
                return result.data.blob();
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_RECORD);
            }
        }

        storage_engine::lmdb::Result getResult(const PositionId& posid)
        {
            auto result = get(posid);
            if (!result.empty) {
                return result;
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_RECORD);
            }
        }

        storage_engine::lmdb::Cursor getCursor() const
        {
            return cursor();
        }

        void resultSetIter(std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback)
        {
            auto cursorHandler = getCursor();
            for (auto keyValue = cursorHandler.getNext();
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto key = keyValue.key.data.numeric<PositionId>();
                if (key == MAX_RECORD_NUM_EM)
                    continue;
                callback(key, keyValue.val);
            }
        }

        const ClassId& getClassId() const
        {
            return _classId;
        }

        const ClassType& getClassType() const
        {
            return _classType;
        }

    private:
        ClassId _classId {};
        ClassType _classType { ClassType::UNDEFINED };
    };

}
}
}
