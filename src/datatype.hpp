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

#include <unistd.h>

namespace nogdb {
namespace internal_data_type {

    class Blob {
    public:
        typedef unsigned char Byte;

        Blob(const size_t capacity = 1);

        Blob(const Byte* value, const size_t capacity);

        Blob(const Byte* value, const size_t capacity, const size_t size);

        ~Blob() noexcept;

        Blob(const Blob& binaryObject);

        Blob& operator=(const Blob& binaryObject) noexcept;

        Blob(Blob&& binaryObject);

        Blob& operator=(Blob&& binaryObject) noexcept;

        size_t capacity() const noexcept { return _capacity; }

        size_t size() const noexcept { return _size; }

        Byte* bytes() const noexcept { return _value; }

        Blob& append(const void* data, size_t size);

        size_t retrieve(void* data, size_t offset, size_t size) const;

        Blob& update(const void* data, size_t offset, size_t size);

        Blob overwrite(const void* data, size_t offset, size_t size) const;

        Blob operator+(const Blob& suffix) const;

    private:
        size_t _capacity;
        size_t _size;
        Byte* _value;
    };

}

}
