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

#include "schema.hpp"

namespace nogdb {
namespace schema {

    ClassAccessInfo SchemaUtils::getExistingClass(const Transaction *txn, const std::string& className)
    {
        auto foundClass = txn->_adapter->dbClass()->getInfo(className);
        if (foundClass.type == ClassType::UNDEFINED) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_CLASS);
        }
        return foundClass;
    }

    ClassAccessInfo SchemaUtils::getExistingClass(const Transaction *txn, const ClassId& classId)
    {
        auto foundClass = txn->_adapter->dbClass()->getInfo(classId);
        if (foundClass.type == ClassType::UNDEFINED) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_CLASS);
        }
        return foundClass;
    }

    PropertyAccessInfo SchemaUtils::getExistingProperty(const Transaction *txn,
        const ClassId& classId,
        const std::string& propertyName)
    {
        auto foundProperty = txn->_adapter->dbProperty()->getInfo(classId, propertyName);
        if (foundProperty.type == PropertyType::UNDEFINED) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
        }
        return foundProperty;
    }

    PropertyAccessInfo SchemaUtils::getExistingPropertyExtend(const Transaction *txn,
        const ClassId& classId,
        const std::string& propertyName)
    {
        auto foundProperty = txn->_adapter->dbProperty()->getInfo(classId, propertyName);
        if (foundProperty.type == PropertyType::UNDEFINED) {
            auto superClassId = txn->_adapter->dbClass()->getSuperClassId(classId);
            if (superClassId != ClassId {}) {
                return getExistingPropertyExtend(txn, superClassId, propertyName);
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
            }
        } else {
            return foundProperty;
        }
    }

    std::map<std::string, ClassAccessInfo> SchemaUtils::getSubClassInfos(const Transaction *txn, const ClassId& classId)
    {
        auto tmpResult = std::map<std::string, ClassAccessInfo> {};
        for (const auto& subClassInfo : txn->_adapter->dbClass()->getSubClassInfos(classId)) {
            tmpResult.emplace(subClassInfo.name, subClassInfo);
            auto partialResult = getSubClassInfos(txn, subClassInfo.id);
            tmpResult.insert(partialResult.cbegin(), partialResult.cend());
        }
        return tmpResult;
    }

    std::vector<PropertyAccessInfo> SchemaUtils::getNativePropertyInfo(const Transaction *txn, const ClassId& classId)
    {
        auto result = std::vector<PropertyAccessInfo> {};
        for (const auto& propertyInfo : txn->_adapter->dbProperty()->getInfos(classId)) {
            result.emplace_back(propertyInfo);
        }
        return result;
    }

    std::vector<PropertyAccessInfo> SchemaUtils::getInheritPropertyInfo(const Transaction *txn,
        const ClassId& superClassId,
        const std::vector<PropertyAccessInfo>& result)
    {
        if (superClassId != ClassId {}) {
            auto tmpResult = result;
            auto partialResult = txn->_adapter->dbProperty()->getInfos(superClassId);
            tmpResult.insert(tmpResult.cend(), partialResult.cbegin(), partialResult.cend());
            return getInheritPropertyInfo(txn, txn->_adapter->dbClass()->getSuperClassId(superClassId), tmpResult);
        }
        return result;
    }

    PropertyNameMapInfo SchemaUtils::getPropertyNameMapInfo(const Transaction *txn,
        const ClassId& classId,
        const ClassId& superClassId)
    {
        auto result = PropertyNameMapInfo {};
        for (const auto& property : getNativePropertyInfo(txn, classId)) {
            result[property.name] = property;
        }
        auto inheritResult = getInheritPropertyInfo(txn, superClassId, std::vector<PropertyAccessInfo> {});
        for (const auto& property : inheritResult) {
            result[property.name] = property;
        }
        return addBasicInfo(result);
    }

    PropertyIdMapInfo SchemaUtils::getPropertyIdMapInfo(const Transaction *txn,
        const ClassId& classId,
        const ClassId& superClassId)
    {
        auto result = PropertyIdMapInfo {};
        for (const auto& property : getNativePropertyInfo(txn, classId)) {
            result[property.id] = property;
        }
        auto inheritResult = getInheritPropertyInfo(txn, superClassId, std::vector<PropertyAccessInfo> {});
        for (const auto& property : inheritResult) {
            result[property.id] = property;
        }
        return addBasicInfo(result);
    }

    IndexAccessInfo SchemaUtils::getIndexInfo(const Transaction *txn,
        const ClassId& classId,
        const PropertyId& propertyId)
    {
        auto foundIndexInfo = txn->_adapter->dbIndex()->getInfo(classId, propertyId);
        if (foundIndexInfo.id == IndexId {}) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_INDEX);
        }
        return foundIndexInfo;
    }

}
}
