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

#include "constant.hpp"
#include "datarecord_adapter.hpp"
#include "parser.hpp"
#include "schema_adapter.hpp"
#include "storage_adapter.hpp"

#define INDEX_TYPE_POSITIVE 0 //0000
#define INDEX_TYPE_NEGATIVE 1 //0001
#define INDEX_TYPE_NUMERIC 0 //0000
#define INDEX_TYPE_STRING 2 //0010
#define INDEX_TYPE_UNIQUE 0 //0000
#define INDEX_TYPE_NON_UNIQUE 4 //0100

namespace nogdb {
namespace adapter {
namespace index {
    using namespace schema;

    class IndexRecord : public storage_engine::adapter::LMDBKeyValAccess {
    public:
        IndexRecord(const storage_engine::LMDBTxn* const txn, const IndexId& indexId, const unsigned int flags)
            : LMDBKeyValAccess(txn, buildIndexName(indexId, getPositiveFlag(flags)), getNumericFlag(flags),
                getUniqueFlag(flags), false, !getUniqueFlag(flags))
            , _positive { getPositiveFlag(flags) }
            , _numeric { getNumericFlag(flags) }
            , _unique { getUniqueFlag(flags) }
        {
        }

        virtual ~IndexRecord() noexcept = default;

        IndexRecord(IndexRecord&& other) noexcept
        {
            *this = std::move(other);
        }

        IndexRecord& operator=(IndexRecord&& other) noexcept
        {
            if (this != &other) {
                using std::swap;
                swap(*this, other);
            }
            return *this;
        }

        template <typename K>
        void create(const K& key, const Blob& blob)
        {
            put(key, blob);
        }

        void destroy()
        {
            drop(true);
        }

        storage_engine::lmdb::Cursor getCursor() const
        {
            return cursor();
        }

    private:
        bool _positive;
        bool _numeric;
        bool _unique;

        static std::string buildIndexName(const IndexId& indexId, bool positive)
        {
            auto indexName = TB_INDEXING_PREFIX + std::to_string(indexId);
            if (!positive) {
                return indexName + "_n";
            } else {
                return indexName;
            }
        }

        static bool getPositiveFlag(const unsigned int flags)
        {
            return ((flags & INDEX_TYPE_NEGATIVE) == INDEX_TYPE_POSITIVE);
        }

        static bool getNumericFlag(const unsigned int flags)
        {
            return ((flags & INDEX_TYPE_STRING) == INDEX_TYPE_NUMERIC);
        }

        static bool getUniqueFlag(const unsigned int flags)
        {
            return ((flags & INDEX_TYPE_NON_UNIQUE) == INDEX_TYPE_UNIQUE);
        }
    };

}
}
}