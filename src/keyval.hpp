/*
 *  Copyright (C) 2018, Throughwave (Thailand) Co., Ltd.
 *  <peerawich at throughwave dot co dot th>
 *
 *  This file is part of libnogdb, the NogDB core library in C++.
 *
 *  libnogdb is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __KEYVAL_HPP_INCLUDED_
#define __KEYVAL_HPP_INCLUDED_

#include "lmdb/lmdb.h"

namespace nogdb {

    class KeyValue {
    public:
        KeyValue();

        KeyValue(const MDB_val &key, const MDB_val &value);

        ~KeyValue() noexcept = default;

        KeyValue(const KeyValue &keyValue);

        KeyValue &operator=(const KeyValue &keyValue);

        KeyValue(KeyValue &&keyValue) noexcept = default;

        KeyValue &operator=(KeyValue &&keyValue) noexcept = default;

        const MDB_val &key() const noexcept { return key_; }

        const MDB_val &value() const noexcept { return value_; }

        bool empty() const noexcept { return empty_; }

    private:
        MDB_val key_;
        MDB_val value_;
        bool empty_;
    };

}

#endif
