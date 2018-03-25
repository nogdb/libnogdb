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

#include "nogdb_types.h"

namespace nogdb {

    Bytes::Bytes(const unsigned char *data, size_t len)
            : value_{nullptr}, size_{len} {
        value_ = new(std::nothrow) unsigned char[size_];
        std::copy(data, data + size_, value_);
    }

    Bytes::Bytes(const unsigned char *data)
            : Bytes{data, strlen((char *) data)} {}

    Bytes::Bytes(const char *data)
            : Bytes{reinterpret_cast<const unsigned char *>(data), strlen(data)} {}

    Bytes::Bytes(const std::string &data)
            : Bytes{static_cast<const unsigned char *>((void *) data.c_str()), strlen(data.c_str())} {}

    Bytes::~Bytes() noexcept {
        delete[] value_;
    }

    Bytes::Bytes(const Bytes &binaryObject)
            : Bytes{binaryObject.value_, binaryObject.size_} {}

    Bytes &Bytes::operator=(const Bytes &binaryObject) {
        if (this != &binaryObject) {
            auto tmp(binaryObject);
            using std::swap;
            swap(tmp, *this);
        }
        return *this;
    }

    Bytes::Bytes(Bytes &&binaryObject) noexcept
            : value_{binaryObject.value_}, size_{binaryObject.size_} {
        binaryObject.value_ = nullptr;
        binaryObject.size_ = 0;
    }

    Bytes &Bytes::operator=(Bytes &&binaryObject) noexcept {
        if (this != &binaryObject) {
            delete[] value_;
            value_ = binaryObject.value_;
            size_ = binaryObject.size_;
            binaryObject.value_ = nullptr;
            binaryObject.size_ = 0;
        }
        return *this;
    }

    uint8_t Bytes::toTinyIntU() const {
        return convert<uint8_t>();
    }

    int8_t Bytes::toTinyInt() const {
        return convert<int8_t>();
    }

    uint16_t Bytes::toSmallIntU() const {
        return convert<uint16_t>();
    }

    int16_t Bytes::toSmallInt() const {
        return convert<int16_t>();
    }

    uint32_t Bytes::toIntU() const {
        return convert<uint32_t>();
    }

    int32_t Bytes::toInt() const {
        return convert<int32_t>();
    }

    uint64_t Bytes::toBigIntU() const {
        return convert<uint64_t>();
    }

    int64_t Bytes::toBigInt() const {
        return convert<int64_t>();
    }

    double Bytes::toReal() const {
        return convert<double>();
    }

    std::string Bytes::toText() const {
        return (size_ == 0) ? std::string{} : std::string{reinterpret_cast<char *>(value_), size_};
    }

    Bytes::operator unsigned char *() const {
        return value_;
    }

    unsigned char *Bytes::getRaw() const {
        return value_;
    }

    size_t Bytes::size() const {
        return size_;
    }

    bool Bytes::empty() const {
        return !size_;
    }

}
