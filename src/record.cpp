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
#include <cstdlib>

#include "constant.hpp"
#include "validate.hpp"
#include "utils.hpp"

#include "nogdb/nogdb_errors.h"
#include "nogdb/nogdb_types.h"

namespace nogdb {

const Record::PropertyToBytesMap& Record::getAll() const
{
    return properties;
}

const Record::PropertyToBytesMap& Record::getBasicInfo() const
{
    return basicProperties;
}

Bytes Record::get(const std::string& propName) const
{
    const PropertyToBytesMap& prop = (isBasicInfo(propName) ? basicProperties : properties);
    const PropertyToBytesMap::const_iterator it = prop.find(propName);
    return it == prop.cend() ? Bytes {} : it->second;
}

std::vector<std::string> Record::getProperties() const
{
    auto propertyNames = std::vector<std::string> {};
    for (const auto& property : properties) {
        propertyNames.emplace_back(property.first);
    }
    return propertyNames;
}

uint8_t Record::getTinyIntU(const std::string& propName) const
{
    auto bytes = get(propName);
    if (bytes.empty()) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
    } else {
        return bytes.toTinyIntU();
    }
}

int8_t Record::getTinyInt(const std::string& propName) const
{
    auto bytes = get(propName);
    if (bytes.empty()) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
    } else {
        return bytes.toTinyInt();
    }
}

uint16_t Record::getSmallIntU(const std::string& propName) const
{
    auto bytes = get(propName);
    if (bytes.empty()) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
    } else {
        return bytes.toSmallIntU();
    }
}

int16_t Record::getSmallInt(const std::string& propName) const
{
    auto bytes = get(propName);
    if (bytes.empty()) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
    } else {
        return bytes.toSmallInt();
    }
}

uint32_t Record::getIntU(const std::string& propName) const
{
    auto bytes = get(propName);
    if (bytes.empty()) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
    } else {
        return bytes.toIntU();
    }
}

int32_t Record::getInt(const std::string& propName) const
{
    auto bytes = get(propName);
    if (bytes.empty()) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
    } else {
        return bytes.toInt();
    }
}

uint64_t Record::getBigIntU(const std::string& propName) const
{
    auto bytes = get(propName);
    if (bytes.empty()) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
    } else {
        return bytes.toBigIntU();
    }
}

int64_t Record::getBigInt(const std::string& propName) const
{
    auto bytes = get(propName);
    if (bytes.empty()) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
    } else {
        return bytes.toBigInt();
    }
}

double Record::getReal(const std::string& propName) const
{
    auto bytes = get(propName);
    if (bytes.empty()) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
    } else {
        return bytes.toReal();
    }
}

std::string Record::getText(const std::string& propName) const
{
    auto bytes = get(propName);
    if (bytes.empty()) {
        return "";
    } else {
        return bytes.toText();
    }
}

std::string Record::getClassName() const
{
    return getText(CLASS_NAME_PROPERTY);
}

RecordId Record::getRecordId() const
{
    auto ridAsString = getText(RECORD_ID_PROPERTY);
    auto sp = utils::string::split(ridAsString, ':');
    if (sp.size() != 2) {
        try {
            auto classId = strtoul(sp[0].c_str(), nullptr, 0);
            auto positionId = strtoul(sp[1].c_str(), nullptr, 0);
            ;
            return RecordId { classId, positionId };
        } catch (...) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INTERNAL_ERR);
        }
    } else {
        return RecordId {};
    }
}

uint32_t Record::getDepth() const
{
    return getIntU(DEPTH_PROPERTY);
}

uint64_t Record::getVersion() const
{
    return getBigIntU(VERSION_PROPERTY);
}

void Record::unset(const std::string& propName)
{
    (isBasicInfo(propName) ? basicProperties : properties).erase(propName);
}

size_t Record::size() const
{
    return properties.size();
}

bool Record::empty() const
{
    return properties.empty();
}

void Record::clear()
{
    basicProperties.clear();
    properties.clear();
}

Record::Record(PropertyToBytesMap properties)
    : properties(std::move(properties))
{
    for (auto it = this->properties.begin(); it != this->properties.end();) {
        if (isBasicInfo(it->first)) {
            basicProperties.insert(*it);
            this->properties.erase(it++);
        } else {
            ++it;
        }
    }
}

}