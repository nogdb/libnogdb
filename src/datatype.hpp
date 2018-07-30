/*
 *  Copyright (C) 2018, Throughwave (Thailand) Co., Ltd.
 *  <peerawich at throughwave dot co dot th>
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

#ifndef __BLOB_HPP_INCLUDED_
#define __BLOB_HPP_INCLUDED_

namespace nogdb {

    namespace internal_data_type {

        class Blob {
        public:
            typedef unsigned char Byte;

            Blob(const size_t capacity = 1);

            Blob(const Byte *value, const size_t capacity);

            ~Blob() noexcept;

            Blob(const Blob &binaryObject);

            Blob &operator=(const Blob &binaryObject) noexcept;

            Blob(Blob &&binaryObject);

            Blob &operator=(Blob &&binaryObject) noexcept;

            size_t capacity() const noexcept { return capacity_; }

            size_t size() const noexcept { return size_; }

            Byte *bytes() const noexcept { return value_; }

            Blob &append(const void *data, size_t size);

            size_t retrieve(void *data, size_t offset, size_t size) const;

        private:
            size_t capacity_;
            size_t size_;
            Byte *value_;
        };

    }

}


#endif
