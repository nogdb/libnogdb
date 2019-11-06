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

#include <algorithm>
#include <cstring>
#include <new>

#include "datatype.hpp"
#include "utils.hpp"

namespace nogdb {
namespace internal_data_type {
    using namespace nogdb::utils::assertion;

    Blob::Blob(const size_t capacity)
        : _capacity { capacity }
        , _size { 0 }
        , _value { nullptr }
    {
        _value = new (std::nothrow) Byte[capacity] {};
    }

    Blob::Blob(const Byte* value, const size_t capacity)
        : _capacity { capacity }
        , _size { capacity }
        , _value { nullptr }
    {
        _value = new (std::nothrow) Byte[capacity] {};
        std::copy(value, value + capacity, _value);
    }

    Blob::Blob(const Byte* value, const size_t capacity, const size_t size)
        : _capacity { capacity }
        , _size { size }
        , _value { nullptr }
    {
        _value = new (std::nothrow) Byte[capacity] {};
        std::copy(value, value + size, _value);
    }

    Blob::~Blob() noexcept
    {
        delete[] _value;
    }

    Blob::Blob(const Blob& binaryObject)
        : _capacity { binaryObject._capacity }
        , _size { binaryObject._size }
        , _value { nullptr }
    {
        _value = new (std::nothrow) Byte[_capacity] {};
        std::copy(binaryObject._value, binaryObject._value + binaryObject._capacity, _value);
    }

    Blob& Blob::operator=(const Blob& binaryObject) noexcept
    {
        if (this != &binaryObject) {
            auto tmp(binaryObject);
            delete[] _value;
            _value = nullptr;
            using std::swap;
            swap(tmp, *this);
        }
        return *this;
    }

    Blob::Blob(Blob&& binaryObject)
        : _capacity { binaryObject._capacity }
        , _size { binaryObject._size }
        , _value { std::move(binaryObject._value) }
    {
        binaryObject._value = nullptr;
        binaryObject._capacity = 0;
        binaryObject._size = 0;
    }

    Blob& Blob::operator=(Blob&& binaryObject) noexcept
    {
        if (this != &binaryObject) {
            delete[] _value;
            _value = binaryObject._value;
            _capacity = binaryObject._capacity;
            _size = binaryObject._size;
            binaryObject._value = nullptr;
            binaryObject._capacity = 0;
            binaryObject._size = 0;
        }
        return *this;
    }

    Blob& Blob::append(const void* data, size_t size)
    {
        require(_size + size <= _capacity);
        memcpy(static_cast<void*>(_value + _size), data, size);
        _size += size;
        return *this;
    }

    size_t Blob::retrieve(void* data, size_t offset, size_t size) const
    {
        require(offset + size <= _capacity);
        memcpy(data, static_cast<const void*>(_value + offset), size);
        return offset + size;
    }

    Blob& Blob::update(const void* data, size_t offset, size_t size)
    {
        require(offset + size <= _capacity);
        require(offset <= _size);
        memcpy(static_cast<void*>(_value + offset), data, size);
        if (_size < offset + size) {
            _size = offset + size;
        }
        return *this;
    }

    Blob Blob::overwrite(const void* data, size_t offset, size_t size) const
    {
        require(offset <= _capacity);
        auto capacity = offset + size;
        auto blob = Blob(capacity);
        memcpy(blob._value, _value, offset);
        memcpy(blob._value + offset, data, size);
        blob._size = capacity;
        return blob;
    }

    Blob Blob::operator+(const Blob& suffix) const
    {
        auto capacity = _capacity + suffix._capacity;
        auto blob = Blob { _value, capacity, _size };
        blob.append(suffix._value, suffix._size);
        return blob;
    }

}
}
