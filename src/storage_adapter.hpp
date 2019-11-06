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

#include <string>

#include "storage_engine.hpp"

namespace nogdb {
namespace storage_engine {
namespace adapter {

    class LMDBKeyValAccess {
    public:
        LMDBKeyValAccess() = default;

        LMDBKeyValAccess(const LMDBTxn* txn,
            const std::string& dbName,
            bool numericKey = false,
            bool unique = true,
            bool append = false,
            bool overwrite = true)
            : _txn { txn }
            , _append { append }
            , _overwrite { overwrite }
        {
            _dbi = txn->openDBi(dbName, numericKey, unique);
        }

        virtual ~LMDBKeyValAccess() noexcept = default;

        LMDBKeyValAccess(LMDBKeyValAccess&& other) noexcept
            : _txn { other._txn }
            , _dbi { std::move(other._dbi) }
            , _append { other._append }
            , _overwrite { other._overwrite }
        {
        }

        LMDBKeyValAccess& operator=(LMDBKeyValAccess&& other) noexcept
        {
            if (this != &other) {
                using std::swap;
                swap(*this, other);
            }
            return *this;
        }

    protected:
        template <typename K, typename V>
        void put(const K& key, const V& val)
        {
            if (_dbi == 0) {
                throw NOGDB_INTERNAL_ERROR(NOGDB_INTERNAL_EMPTY_DBI);
            }
            _dbi.put(key, val, _append, _overwrite);
        }

        template <typename K>
        lmdb::Result get(const K& key) const
        {
            if (_dbi == 0) {
                throw NOGDB_INTERNAL_ERROR(NOGDB_INTERNAL_EMPTY_DBI);
            }
            return _dbi.get(key);
        }

        template <typename K>
        void del(const K& key)
        {
            if (_dbi == 0) {
                throw NOGDB_INTERNAL_ERROR(NOGDB_INTERNAL_EMPTY_DBI);
            }
            _dbi.del(key);
        }

        template <typename K, typename V>
        void del(const K& key, const V& val)
        {
            if (_dbi == 0) {
                throw NOGDB_INTERNAL_ERROR(NOGDB_INTERNAL_EMPTY_DBI);
            }
            _dbi.del(key, val);
        };

        void drop(const bool del = false)
        {
            if (_dbi == 0) {
                throw NOGDB_INTERNAL_ERROR(NOGDB_INTERNAL_EMPTY_DBI);
            }
            _dbi.drop(del);
        }

        lmdb::Cursor cursor() const
        {
            if (_txn == nullptr) {
                throw NOGDB_INTERNAL_ERROR(NOGDB_INTERNAL_NULL_TXN);
            }
            if (_dbi == 0) {
                throw NOGDB_INTERNAL_ERROR(NOGDB_INTERNAL_EMPTY_DBI);
            }
            return _txn->openCursor(_dbi);
        }

    private:
        const LMDBTxn* _txn;
        lmdb::DBi _dbi {};
        bool _append { false };
        bool _overwrite { true };
    };

}
}
}
