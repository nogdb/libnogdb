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

#include "constant.hpp"
#include "storage_adapter.hpp"

namespace nogdb {
namespace adapter {
namespace metadata {

    class DBInfoAccess : public storage_engine::adapter::LMDBKeyValAccess {
    public:
        DBInfoAccess() = default;

        DBInfoAccess(const storage_engine::LMDBTxn* txn)
            : LMDBKeyValAccess(txn, TB_DBINFO)
        {
        }

        ~DBInfoAccess() noexcept = default;

        DBInfoAccess(DBInfoAccess&& other) noexcept
        {
            using std::swap;
            swap(_cache, other._cache);
        }

        DBInfoAccess& operator=(DBInfoAccess&& other) noexcept
        {
            if (this != &other) {
                using std::swap;
                swap(_cache, other._cache);
            }
            return *this;
        }

        void setMaxClassId(ClassId maxClassId)
        {
            put(MAX_CLASS_ID_KEY, maxClassId);
            _cache.maxClassId = maxClassId;
        }

        ClassId getMaxClassId() const
        {
            if (_cache.maxClassId == 0) {
                auto result = get(MAX_CLASS_ID_KEY);
                if (result.empty) {
                    return ClassId { INIT_NUM_CLASSES };
                } else {
                    auto maxClassId = result.data.numeric<ClassId>();
                    _cache.maxClassId = maxClassId;
                    return maxClassId;
                }
            }
            return _cache.maxClassId;
        }

        void setNumClassId(ClassId numClass)
        {
            put(NUM_CLASS_KEY, numClass);
            _cache.numClass = numClass;
        }

        ClassId getNumClassId() const
        {
            if (_cache.numClass == 0) {
                auto result = get(NUM_CLASS_KEY);
                if (result.empty) {
                    return ClassId { 0 };
                } else {
                    auto numClass = result.data.numeric<ClassId>();
                    _cache.numClass = numClass;
                    return numClass;
                }
            }
            return _cache.numClass;
        }

        void setMaxPropertyId(PropertyId maxPropertyId)
        {
            put(MAX_PROPERTY_ID_KEY, maxPropertyId);
            _cache.maxPropertyId = maxPropertyId;
        }

        PropertyId getMaxPropertyId() const
        {
            if (_cache.maxPropertyId == 0) {
                auto result = get(MAX_PROPERTY_ID_KEY);
                if (result.empty) {
                    return PropertyId { INIT_NUM_PROPERTIES };
                } else {
                    auto maxPropertyId = result.data.numeric<PropertyId>();
                    _cache.maxPropertyId = maxPropertyId;
                    return maxPropertyId;
                }
            }
            return _cache.maxPropertyId;
        }

        void setNumPropertyId(PropertyId numProperty)
        {
            put(NUM_PROPERTY_KEY, numProperty);
            _cache.numProperty = numProperty;
        }

        PropertyId getNumPropertyId() const
        {
            if (_cache.numProperty == 0) {
                auto result = get(NUM_PROPERTY_KEY);
                if (result.empty) {
                    return PropertyId { 0 };
                } else {
                    auto numProperty = result.data.numeric<PropertyId>();
                    _cache.numProperty = numProperty;
                    return numProperty;
                }
            }
            return _cache.numProperty;
        }

        void setMaxIndexId(IndexId maxIndexId)
        {
            put(MAX_INDEX_ID_KEY, maxIndexId);
            _cache.maxIndexId = maxIndexId;
        }

        IndexId getMaxIndexId() const
        {
            if (_cache.maxIndexId == 0) {
                auto result = get(MAX_INDEX_ID_KEY);
                if (result.empty) {
                    return IndexId { 0 };
                } else {
                    auto maxIndexId = result.data.numeric<IndexId>();
                    _cache.maxIndexId = maxIndexId;
                    return maxIndexId;
                }
            }
            return _cache.maxIndexId;
        }

        void setNumIndexId(IndexId numIndex)
        {
            put(NUM_INDEX_KEY, numIndex);
            _cache.numIndex = numIndex;
        }

        IndexId getNumIndexId() const
        {
            if (_cache.numIndex == 0) {
                auto result = get(NUM_INDEX_KEY);
                if (result.empty) {
                    return IndexId { 0 };
                } else {
                    auto numIndex = result.data.numeric<IndexId>();
                    _cache.numIndex = numIndex;
                    return numIndex;
                }
            }
            return _cache.numIndex;
        }

    protected:
        struct DBInfoAccessCache {
            PropertyId maxPropertyId { 0 };
            PropertyId numProperty { 0 };
            ClassId maxClassId { 0 };
            ClassId numClass { 0 };
            IndexId maxIndexId { 0 };
            IndexId numIndex { 0 };
        };

        mutable DBInfoAccessCache _cache {};
    };

}
}
}
