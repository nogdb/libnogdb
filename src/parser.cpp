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

#include <vector>
#include <cmath>
#include <cassert>

#include "datastore.hpp"
#include "generic.hpp"
#include "parser.hpp"

#include "nogdb_errors.h"

namespace nogdb {

    auto emptyString = std::string{"\n"};
    const size_t SIZE_OF_EMPTY_STRING = strlen(emptyString.c_str());
    constexpr size_t UINT16_BITS_COUNT = 8 * sizeof(PropertyId);

    Blob Parser::parseRecord(const BaseTxn &txn, size_t dataSize, const ClassProperty &properties, const Record &record) {
        if (dataSize <= 0) {
            // create an empty property as a raw data for a class
            auto value = Blob(SIZE_OF_EMPTY_STRING);
            value.append(static_cast<void *>(&emptyString), SIZE_OF_EMPTY_STRING);
            return value;
        } else {
            // create properties as a raw data for a class
            auto value = Blob(dataSize);
            for (const auto &property: properties) {
                auto propertyId = static_cast<PropertyId>(property.second.id);
                auto rawData = record.get(property.first);
                auto size = rawData.size();
                assert(property.second.id < std::pow(2, UINT16_BITS_COUNT));
                assert(size < std::pow(2, UINT16_BITS_COUNT));
                value.append(&propertyId, sizeof(PropertyId));
                value.append(&size, sizeof(uint16_t));
                value.append(static_cast<void *>(rawData.getRaw()), size);
            }
            return value;
        }
    }

    Blob Parser::parseRecord(const BaseTxn &txn, const Schema::ClassDescriptorPtr &classDescriptor, const Record &record) {
        auto dataSize = size_t{0};
        auto classInfo = Generic::getClassMapProperty(txn, classDescriptor);
        auto properties = decltype(classInfo.nameToDesc) {};
        // calculate a raw data size of properties in a record
        for (const auto &property: record.getAll()) {
            auto foundProperty = classInfo.nameToDesc.find(property.first);
            if (foundProperty == classInfo.nameToDesc.cend()) {
                throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
            }
            dataSize += sizeof(PropertyId) + sizeof(uint16_t); // for storing a property id and size
            dataSize += property.second.size(); // for storing a property value itself
            properties.emplace(std::make_pair(foundProperty->first, foundProperty->second));
        }
        if (dataSize <= 0) {
            // create an empty property as a raw data for a class
            auto value = Blob(SIZE_OF_EMPTY_STRING);
            value.append(static_cast<void *>(&emptyString), SIZE_OF_EMPTY_STRING);
            return value;
        } else {
            // create properties as a raw data for a class
            auto value = Blob(dataSize);
            for (const auto &property: properties) {
                auto propertyId = static_cast<PropertyId>(property.second.id);
                auto rawData = record.get(property.first);
                auto size = rawData.size();
                assert(property.second.id < std::pow(2, UINT16_BITS_COUNT));
                assert(size < std::pow(2, UINT16_BITS_COUNT));
                value.append(&propertyId, sizeof(PropertyId));
                value.append(&size, sizeof(uint16_t));
                value.append(static_cast<void *>(rawData.getRaw()), size);
            }
            return value;
        }
    }

    Record Parser::parseRawData(const KeyValue &keyValue, const ClassPropertyInfo &classPropertyInfo) {
        auto result = Record{};
        if (keyValue.empty()) {
            return result;
        }
        auto rawData = Datastore::getValueAsBlob(keyValue);
        auto offset = size_t{0};
        if (rawData.capacity() == 0) {
            throw Error(CTX_UNKNOWN_ERR, Error::Type::CONTEXT);
        } else if (rawData.capacity() >= 2 * sizeof(uint16_t)) {
            // convert each block in the raw data to bytes storing in a record object
            // NOTE: each property's block consists of property id, size, and value
            // +----------------------+------------------------+-----------+
            // | propertyId (16bits)  | propertySize (16bits)  |   value   |
            // +----------------------+------------------------+-----------+
            while (offset < rawData.capacity()) {
                auto propertyId = PropertyId{0};
                auto propertySize = uint16_t{};
                offset = rawData.retrieve(&propertyId, offset, sizeof(PropertyId));
                offset = rawData.retrieve(&propertySize, offset, sizeof(uint16_t));
                auto foundInfo = classPropertyInfo.idToName.find(propertyId);
                if (foundInfo != classPropertyInfo.idToName.cend()) {
                    if (propertySize > 0) {
                        Blob::Byte byteData[propertySize];
                        offset = rawData.retrieve(byteData, offset, propertySize);
                        result.set(foundInfo->second, Bytes{byteData, propertySize});
                    } else {
                        result.set(foundInfo->second, Bytes{});
                    }
                } else {
                    offset += propertySize;
                }
            }
        }
        return result;
    }
}
