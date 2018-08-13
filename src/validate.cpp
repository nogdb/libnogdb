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

#include <algorithm>
#include <regex>

#include "validate.hpp"

#include "nogdb_txn.h"

#define CLASS_ID_UPPER_LIMIT    (UINT16_MAX - 1)

namespace nogdb {

    void Validate::isTransactionValid(const Txn &txn) {
        if (txn.getTxnMode() == Txn::Mode::READ_ONLY) {
            throw NOGDB_TXN_ERROR(NOGDB_TXN_INVALID_MODE);
        }
        if (txn.isCompleted()) {
            throw NOGDB_TXN_ERROR(NOGDB_TXN_COMPLETED);
        }
    }

    void Validate::isClassIdMaxReach(const Txn &txn) {
        if ((txn._dbInfo.getMaxClassId() >= CLASS_ID_UPPER_LIMIT)) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_LIMIT_DBSCHEMA);
        }
    }

    void Validate::isClassNameValid(const std::string &className) {
        if (!isNameValid(className)) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_CLASSNAME);
        }
    }

    void Validate::isPropertyNameValid(const std::string &propName) {
        if (!isNameValid(propName)) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_PROPERTYNAME);
        }
    }

    bool Validate::isNameValid(const std::string &name) {
        return std::regex_match(name, std::regex("^[A-Za-z_][A-Za-z0-9_]*$"));
    }

    void Validate::isClassTypeValid(const ClassType &type) {
        switch (type) {
            case ClassType::VERTEX:
            case ClassType::EDGE:
                return;
            default:
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_CLASSTYPE);
        }
    }

    void Validate::isPropertyTypeValid(const PropertyType &type) {
        switch (type) {
            case PropertyType::TINYINT:
            case PropertyType::UNSIGNED_TINYINT:
            case PropertyType::SMALLINT:
            case PropertyType::UNSIGNED_SMALLINT:
            case PropertyType::INTEGER:
            case PropertyType::UNSIGNED_INTEGER:
            case PropertyType::BIGINT:
            case PropertyType::UNSIGNED_BIGINT:
            case PropertyType::TEXT:
            case PropertyType::REAL:
            case PropertyType::BLOB:
                return;
            default:
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_PROPTYPE);
        }
    }

    void Validate::isNotDuplicatedClass(const Txn &txn, const std::string &className) {
        auto foundClass = txn._class.getId(className);
        if (foundClass != ClassId{}) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_CLASS);
        }
    }

    adapter::schema::ClassAccessInfo Validate::isExistingClass(const Txn &txn, const std::string &className) {
        auto foundClass = txn._class.getInfo(className);
        if (foundClass.type == ClassType::UNDEFINED) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_CLASS);
        }
        return foundClass;
    }

    adapter::schema::ClassAccessInfo Validate::isExistingClass(const Txn &txn, const ClassId &classId) {
        auto foundClass = txn._class.getInfo(classId);
        if (foundClass.type == ClassType::UNDEFINED) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_CLASS);
        }
        return foundClass;
    }

    adapter::schema::PropertyAccessInfo
    Validate::isExistingProperty(const Txn &txn, const ClassId& classId, const std::string &propertyName) {
        auto foundProperty = txn._property.getInfo(classId, propertyName);
        if (foundProperty.type == PropertyType::UNDEFINED) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
        }
        return foundProperty;
    }

    adapter::schema::PropertyAccessInfo
    Validate::isExistingPropertyExtend(const Txn &txn, const ClassId& classId, const std::string &propertyName) {
        auto foundProperty = txn._property.getInfo(classId, propertyName);
        if (foundProperty.type == PropertyType::UNDEFINED) {
            auto superClassId = txn._class.getSuperClassId(classId);
            if (superClassId != ClassId{}) {
                return isExistingPropertyExtend(txn, classId, propertyName);
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
            }
        } else {
            return foundProperty;
        }
    }

    void Validate::isNotDuplicatedProperty(const Txn &txn, const ClassId& classId, const std::string &propertyName) {
        auto foundProperty = txn._property.getId(classId, propertyName);
        if (foundProperty != PropertyId{}) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_PROPERTY);
        }
        auto superClassId = txn._class.getSuperClassId(classId);
        if (superClassId != ClassId{}) {
            isNotDuplicatedProperty(txn, superClassId, propertyName);
        }
    }

    void Validate::isNotOverridenProperty(const Txn &txn, const ClassId& classId, const std::string &propertyName) {
        auto foundProperty = txn._property.getId(classId, propertyName);
        if (foundProperty != PropertyId{}) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_OVERRIDE_PROPERTY);
        }
        for (const auto &subClassId: txn._class.getSubClassIds(classId)) {
            if (subClassId != ClassId{}) {
                isNotOverridenProperty(txn, subClassId, propertyName);
            }
        }
    }
}



