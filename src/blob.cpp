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
#include <cstring>
#include <new>
#include <cassert>
#include <algorithm>

#include "blob.hpp"

namespace nogdb {

    Blob::Blob(const size_t capacity)
            : capacity_{capacity}, size_{0}, value_{nullptr} {
        value_ = new(std::nothrow) Byte[capacity]{};
    }

    Blob::Blob(const Byte *value, const size_t capacity)
            : capacity_{capacity}, size_{capacity}, value_{nullptr} {
        value_ = new(std::nothrow) Byte[capacity]{};
        std::copy(value, value + capacity, value_);
    }

    Blob::~Blob() noexcept {
        delete[] value_;
    }

    Blob::Blob(const Blob &binaryObject)
            : capacity_{binaryObject.capacity_}, size_{binaryObject.size_}, value_{nullptr} {
        value_ = new(std::nothrow) Byte[capacity_]{};
        std::copy(binaryObject.value_, binaryObject.value_ + binaryObject.capacity_, value_);
    }

    Blob &Blob::operator=(const Blob &binaryObject) noexcept {
        if (this != &binaryObject) {
            auto tmp(binaryObject);
            delete[] value_;
            value_ = nullptr;
            using std::swap;
            swap(tmp, *this);
        }
        return *this;
    }

    Blob::Blob(Blob &&binaryObject)
            : capacity_{binaryObject.capacity_}, size_{binaryObject.size_}, value_{std::move(binaryObject.value_)} {
        binaryObject.value_ = nullptr;
        binaryObject.capacity_ = 0;
        binaryObject.size_ = 0;
    }

    Blob &Blob::operator=(Blob &&binaryObject) noexcept {
        if (this != &binaryObject) {
            delete[] value_;
            value_ = binaryObject.value_;
            capacity_ = binaryObject.capacity_;
            size_ = binaryObject.size_;
            binaryObject.value_ = nullptr;
            binaryObject.capacity_ = 0;
            binaryObject.size_ = 0;
        }
        return *this;
    }

    Blob &Blob::append(const void *data, size_t size) {
        assert(size_ + size <= capacity_);
        memcpy(static_cast<void *>(value_ + size_), data, size);
        size_ += size;
        return *this;
    }

    size_t Blob::retrieve(void *data, size_t offset, size_t size) {
        assert(offset + size <= capacity_);
        memcpy(data, static_cast<const void *>(value_ + offset), size);
        return offset + size;
    }

}
