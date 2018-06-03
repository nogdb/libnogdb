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

#include <iostream> // for debugging
#include <bitset> // for debugging
#include <vector>
#include <cmath>

#include "datastore.hpp"
#include "generic.hpp"
#include "parser.hpp"
#include "utils.hpp"

#include "nogdb_errors.h"

#include <iostream>

namespace nogdb {

    auto emptyString = std::string{"\n"};
    const size_t SIZE_OF_EMPTY_STRING = strlen(emptyString.c_str());

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
                require(property.second.id < std::pow(2, UINT16_BITS_COUNT));
                require(rawData.size() < std::pow(2, UINT32_BITS_COUNT - 1));
                if (rawData.size() < std::pow(2, UINT8_BITS_COUNT - 1)) {
                    auto size = static_cast<uint8_t>(rawData.size()) << 1;
                    value.append(&propertyId, sizeof(PropertyId));
                    value.append(&size, sizeof(uint8_t));
                    value.append(static_cast<void *>(rawData.getRaw()), rawData.size());
                } else {
                    auto size = (static_cast<uint32_t>(rawData.size()) << 1) + 0x1;
                    value.append(&propertyId, sizeof(PropertyId));
                    value.append(&size, sizeof(uint32_t));
                    value.append(static_cast<void *>(rawData.getRaw()), rawData.size());
                }
            }
            return value;
        }
    }

    Blob Parser::parseRecord(const BaseTxn &txn,
                             const Schema::ClassDescriptorPtr &classDescriptor,
                             const Record &record,
                             ClassPropertyInfo& classInfo,
                             std::map<std::string, std::tuple<PropertyType, IndexId, bool>>& indexInfos) {
        auto dataSize = size_t{0};
        auto properties = decltype(classInfo.nameToDesc) {};
        classInfo = Generic::getClassMapProperty(txn, classDescriptor);

        // calculate a raw data size of properties in a record
        for (const auto &property: record.getAll()) {
            auto foundProperty = classInfo.nameToDesc.find(property.first);
            if (foundProperty == classInfo.nameToDesc.cend()) {
                throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
            }
            // check if having any index
            for (const auto &indexIter: foundProperty->second.indexInfo) {
                if (indexIter.second.first == classDescriptor->id) {
                    indexInfos.emplace(
                            property.first,
                            std::make_tuple(
                                    foundProperty->second.type,
                                    indexIter.first,
                                    indexIter.second.second
                            )
                    );
                    break;
                }
            }
            dataSize += getRawDataSize(property.second.size());
            properties.emplace(std::make_pair(foundProperty->first, foundProperty->second));
        }

        // calculate a raw data from basic property info
        for (const auto &property : record.getBasicInfo()) {
            auto foundProperty = classInfo.nameToDesc.find(property.first);
            if (foundProperty == classInfo.nameToDesc.cend()) {
                throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
            }
            if (property.first == VERSION_PROPERTY) {
                dataSize += getRawDataSize(property.second.size());
                properties.emplace(std::make_pair(foundProperty->first, foundProperty->second));
            }
        }

        return parseRecord(txn, dataSize, properties, record);
    }

    Record Parser::parseRawData(const KeyValue &keyValue, const ClassPropertyInfo &classPropertyInfo) {
        if (keyValue.empty()) {
            return Record{};
        }
        auto rawData = Datastore::getValueAsBlob(keyValue);
        auto offset = size_t{0};
        Record::RecordPropertyType properties;
        if (rawData.capacity() == 0) {
            throw Error(CTX_UNKNOWN_ERR, Error::Type::CONTEXT);
        } else if (rawData.capacity() >= 2 * sizeof(uint16_t)) {
            //TODO: should be concerned about ENDIAN?
            // NOTE: each property block consists of property id, flag, size, and value
            // when option flag = 0
            // +----------------------+--------------------+-----------------------+-----------+
            // | propertyId (16bits)  | option flag (1bit) | propertySize (7bits)  |   value   | (next block) ...
            // +----------------------+--------------------+-----------------------+-----------+
            // when option flag = 1 (for extra large size of value)
            // +----------------------+--------------------+------------------------+-----------+
            // | propertyId (16bits)  | option flag (1bit) | propertySize (31bits)  |   value   | (next block) ...
            // +----------------------+--------------------+------------------------+-----------+
            while (offset < rawData.size()) {
                auto propertyId = PropertyId{};
                auto optionFlag = uint8_t{};
                offset = rawData.retrieve(&propertyId, offset, sizeof(PropertyId));
                rawData.retrieve(&optionFlag, offset, sizeof(optionFlag));
                auto propertySize = size_t{};
                if ((optionFlag & 0x1) == 1) {
                    //extra large size of value (exceed 127 bytes)
                    auto tmpSize =  uint32_t{};
                    offset = rawData.retrieve(&tmpSize, offset, sizeof(uint32_t));
                    propertySize = static_cast<size_t>(tmpSize >> 1);
                } else {
                    //normal size of value (not exceed 127 bytes)
                    auto tmpSize = uint8_t{};
                    offset = rawData.retrieve(&tmpSize, offset, sizeof(uint8_t));
                    propertySize = static_cast<size_t>(tmpSize >> 1);
                }
                auto foundInfo = classPropertyInfo.idToName.find(propertyId);
                if (foundInfo != classPropertyInfo.idToName.cend()) {
                    if (propertySize > 0) {
                        Blob::Byte byteData[propertySize];
                        offset = rawData.retrieve(byteData, offset, propertySize);
                        properties[foundInfo->second] = Bytes{byteData, propertySize};
                    } else {
                        properties[foundInfo->second] = Bytes{};
                    }
                } else {
                    offset += propertySize;
                }
            }
        }
        return Record(properties);
    }

    Record Parser::parseRawDataWithBasicInfo(const std::string className,
                                             const RecordId& rid,
                                             const KeyValue &keyValue,
                                             const ClassPropertyInfo &classPropertyInfo) {
        return parseRawData(keyValue, classPropertyInfo)
                .setBasicInfoIfNotExists(CLASS_NAME_PROPERTY, className)
                .setBasicInfoIfNotExists(RECORD_ID_PROPERTY, rid2str(rid))
                .setBasicInfoIfNotExists(VERSION_PROPERTY, 1LL)
                .setBasicInfoIfNotExists(DEPTH_PROPERTY, 0U);
    }
}
