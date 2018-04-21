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

#ifndef __PARSER_HPP_INCLUDED_
#define __PARSER_HPP_INCLUDED_

#include <map>

#include "blob.hpp"
#include "keyval.hpp"
#include "schema.hpp"

#include "nogdb_types.h"

namespace nogdb {

    constexpr size_t UINT8_BITS_COUNT = 8 * sizeof(uint8_t);
    constexpr size_t UINT16_BITS_COUNT = 8 * sizeof(uint16_t);
    constexpr size_t UINT32_BITS_COUNT = 8 * sizeof(uint32_t);

    struct Parser {
        Parser() = delete;

        ~Parser() noexcept = delete;

        static Blob parseRecord(const BaseTxn &txn, size_t dataSize, const ClassProperty &properties, const Record &record);

        static Blob parseRecord(const BaseTxn &txn,
                                const Schema::ClassDescriptorPtr &classDescriptor,
                                const Record &record,
                                ClassPropertyInfo& classInfo,
                                std::map<std::string, std::tuple<PropertyType, IndexId, bool>>& indexInfos);

        static Record parseRawData(const KeyValue &keyValue, const ClassPropertyInfo &classPropertyInfo);

        inline static size_t getRawDataSize(size_t size) {
            return sizeof(PropertyId) + size + ((size >= std::pow(2, UINT8_BITS_COUNT - 1))? sizeof(uint32_t): sizeof(uint8_t));
        };
    };

}

#endif
