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

#include "nogdb/nogdb_types.h"

namespace nogdb {

Bytes::Bytes(const unsigned char* data, size_t len, bool copy)
    : _value { nullptr }
    , _size { len }
{
    if (copy) {
        _value = new (std::nothrow) unsigned char[_size];
        std::copy(data, data + _size, _value);
    } else {
        _value = const_cast<unsigned char*>(data);
    }
}

Bytes::Bytes(const unsigned char* data)
    : Bytes { data, strlen((char*)data) }
{
}

Bytes::Bytes(const char* data)
    : Bytes { reinterpret_cast<const unsigned char*>(data), strlen(data) }
{
}

Bytes::Bytes(const std::string& data)
    : Bytes { static_cast<const unsigned char*>((void*)data.c_str()), strlen(data.c_str()) }
{
}

Bytes::~Bytes() noexcept
{
    if (_value) {
        delete[] _value;
        _value = nullptr;
    }
}

Bytes::Bytes(const Bytes& binaryObject)
    : Bytes { binaryObject._value, binaryObject._size }
{
}

Bytes& Bytes::operator=(const Bytes& binaryObject)
{
    if (this != &binaryObject) {
        auto tmp(binaryObject);
        using std::swap;
        swap(tmp, *this);
    }
    return *this;
}

Bytes::Bytes(Bytes&& binaryObject) noexcept
    : _value { binaryObject._value }
    , _size { binaryObject._size }
{
    binaryObject._value = nullptr;
    binaryObject._size = 0;
}

Bytes& Bytes::operator=(Bytes&& binaryObject) noexcept
{
    if (this != &binaryObject) {
        delete[] _value;
        _value = binaryObject._value;
        _size = binaryObject._size;
        binaryObject._value = nullptr;
        binaryObject._size = 0;
    }
    return *this;
}

uint8_t Bytes::toTinyIntU() const
{
    return convert<uint8_t>();
}

int8_t Bytes::toTinyInt() const
{
    return convert<int8_t>();
}

uint16_t Bytes::toSmallIntU() const
{
    return convert<uint16_t>();
}

int16_t Bytes::toSmallInt() const
{
    return convert<int16_t>();
}

uint32_t Bytes::toIntU() const
{
    return convert<uint32_t>();
}

int32_t Bytes::toInt() const
{
    return convert<int32_t>();
}

uint64_t Bytes::toBigIntU() const
{
    return convert<uint64_t>();
}

int64_t Bytes::toBigInt() const
{
    return convert<int64_t>();
}

double Bytes::toReal() const
{
    return convert<double>();
}

std::string Bytes::toText() const
{
    return convert<std::string>();
}

Bytes::operator unsigned char*() const
{
    return _value;
}

unsigned char* Bytes::getRaw() const
{
    return _value;
}

size_t Bytes::size() const
{
    return _size;
}

bool Bytes::empty() const
{
    return !_size;
}

Bytes Bytes::merge(const Bytes& bytes1, const Bytes& bytes2)
{

    const size_t total_size = bytes1.size() + bytes2.size();

    auto* data = new unsigned char[total_size];

    std::copy(bytes1.getRaw(), bytes1.getRaw() + bytes1.size(), data);
    std::copy(bytes2.getRaw(), bytes2.getRaw() + bytes2.size(), data + bytes1.size());

    return Bytes { data, total_size, false };
};

Bytes Bytes::merge(const std::vector<Bytes>& bytes)
{

    size_t total_size = 0U;
    for (const Bytes& b : bytes) {
        total_size += b.size();
    }

    auto* data = new unsigned char[total_size];

    size_t idx = 0;
    for (const Bytes& b : bytes) {
        std::copy(b.getRaw(), b.getRaw() + b.size(), data + idx);
        idx += b.size();
    }

    return Bytes { data, total_size, false };
}
}
