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

#include "nogdb_error.h"
#include "nogdb_types.h"

namespace nogdb {

    Record &Record::set(const std::string &propName, const unsigned char *value) {
        properties[propName] = Bytes{value, strlen((char *) value)};
        return *this;
    }

    Record &Record::set(const std::string &propName, const char *value) {
        properties[propName] = Bytes{reinterpret_cast<const unsigned char *>(value), strlen(value)};
        return *this;
    }

    Record &Record::set(const std::string &propName, const std::string &value) {
        properties[propName] = Bytes{static_cast<const unsigned char *>((void *) value.c_str()), strlen(value.c_str())};
        return *this;
    }

    Record &Record::set(const std::string &propName, const nogdb::Bytes &b) {
        properties[propName] = b;
        return *this;
    }

    const std::map<std::string, Bytes> &Record::getAll() const {
        return properties;
    }

    Bytes Record::get(const std::string &propName) const {
        auto value = properties.find(propName);
        if (value == properties.cend()) {
            return Bytes{};
        } else {
            return value->second;
        }
    }

    uint8_t Record::getTinyIntU(const std::string &propName) const {
        auto bytes = get(propName);
        if (bytes.empty()) {
            throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
        } else {
            return bytes.toTinyIntU();
        }
    }

    int8_t Record::getTinyInt(const std::string &propName) const {
        auto bytes = get(propName);
        if (bytes.empty()) {
            throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
        } else {
            return bytes.toTinyInt();
        }
    }

    uint16_t Record::getSmallIntU(const std::string &propName) const {
        auto bytes = get(propName);
        if (bytes.empty()) {
            throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
        } else {
            return bytes.toSmallIntU();
        }
    }

    int16_t Record::getSmallInt(const std::string &propName) const {
        auto bytes = get(propName);
        if (bytes.empty()) {
            throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
        } else {
            return bytes.toSmallInt();
        }
    }

    uint32_t Record::getIntU(const std::string &propName) const {
        auto bytes = get(propName);
        if (bytes.empty()) {
            throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
        } else {
            return bytes.toIntU();
        }
    }

    int32_t Record::getInt(const std::string &propName) const {
        auto bytes = get(propName);
        if (bytes.empty()) {
            throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
        } else {
            return bytes.toInt();
        }
    }

    uint64_t Record::getBigIntU(const std::string &propName) const {
        auto bytes = get(propName);
        if (bytes.empty()) {
            throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
        } else {
            return bytes.toBigIntU();
        }
    }

    int64_t Record::getBigInt(const std::string &propName) const {
        auto bytes = get(propName);
        if (bytes.empty()) {
            throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
        } else {
            return bytes.toBigInt();
        }
    }

    double Record::getReal(const std::string &propName) const {
        auto bytes = get(propName);
        if (bytes.empty()) {
            throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
        } else {
            return bytes.toReal();
        }
    }

    std::string Record::getText(const std::string &propName) const {
        auto bytes = get(propName);
        if (bytes.empty()) {
//        throw Error(CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
            return "";
        } else {
            return bytes.toText();
        }
    }

    void Record::unset(const std::string &className) {
        properties.erase(className);
    }

    bool Record::empty() const {
        return properties.empty();
    }

    void Record::clear() {
        properties.clear();
    }


}