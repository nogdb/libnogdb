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

#include <cstdlib>

#include "keyval.hpp"

namespace nogdb {

    KeyValue::KeyValue()
            : empty_{true} {
        key_ = MDB_val{0, nullptr};
        value_ = MDB_val{0, nullptr};
    }

    KeyValue::KeyValue(const MDB_val &key, const MDB_val &value)
            : empty_{false} {
        key_ = key;
        value_ = value;
    }

    KeyValue::KeyValue(const KeyValue &kv)
            : empty_{kv.empty_} {
        key_ = kv.key_;
        value_ = kv.value_;
    }

    KeyValue &KeyValue::operator=(const KeyValue &kv) {
        if (this != &kv) {
            key_ = kv.key_;
            value_ = kv.value_;
            empty_ = kv.empty_;
        }
        return *this;
    }

}
