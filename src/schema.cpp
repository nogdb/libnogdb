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

#include "schema.hpp"

namespace nogdb {

  namespace schema {

    ClassAccessInfo SchemaInterface::getExistingClass(const std::string &className) {
      auto foundClass = _txn->_adapter.dbClass()->getInfo(className);
      if (foundClass.type == ClassType::UNDEFINED) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_CLASS);
      }
      return foundClass;
    }

    ClassAccessInfo SchemaInterface::getExistingClass(const ClassId &classId) {
      auto foundClass = _txn->_adapter.dbClass()->getInfo(classId);
      if (foundClass.type == ClassType::UNDEFINED) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_CLASS);
      }
      return foundClass;
    }

    PropertyAccessInfo SchemaInterface::getExistingProperty(const ClassId &classId, const std::string &propertyName) {
      auto foundProperty = _txn->_adapter.dbProperty()->getInfo(classId, propertyName);
      if (foundProperty.type == PropertyType::UNDEFINED) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
      }
      return foundProperty;
    }

    PropertyAccessInfo
    SchemaInterface::getExistingPropertyExtend(const ClassId &classId, const std::string &propertyName) {
      auto foundProperty = _txn->_adapter.dbProperty()->getInfo(classId, propertyName);
      if (foundProperty.type == PropertyType::UNDEFINED) {
        auto superClassId = _txn->_adapter.dbClass()->getSuperClassId(classId);
        if (superClassId != ClassId{}) {
          return getExistingPropertyExtend(classId, propertyName);
        } else {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
        }
      } else {
        return foundProperty;
      }
    }

    std::map<std::string, ClassAccessInfo>
    SchemaInterface::getSubClassInfos(const ClassId &classId, std::map<std::string, ClassAccessInfo> &result) {
      for (const auto &subClassInfo: _txn->_adapter.dbClass()->getSubClassInfos(classId)) {
        result.emplace(subClassInfo.name, subClassInfo);
        getSubClassInfos(subClassInfo.id, result);
      }
      return std::move(result);
    }

    std::vector<PropertyAccessInfo>
    SchemaInterface::getNativePropertyInfo(const ClassId &classId) {
      auto result = std::vector<PropertyAccessInfo>{};
      for (const auto &propertyInfo: _txn->_adapter.dbProperty()->getInfos(classId)) {
        result.emplace_back(propertyInfo);
      }
      return result;
    }

    std::vector<PropertyAccessInfo>
    SchemaInterface::getInheritPropertyInfo(const ClassId &superClassId, std::vector<PropertyAccessInfo> &result) {
      if (superClassId != ClassId{}) {
        auto partialResult = _txn->_adapter.dbProperty()->getInfos(superClassId);
        result.insert(result.cend(), partialResult.cbegin(), partialResult.cend());
        getInheritPropertyInfo(_txn->_adapter.dbClass()->getSuperClassId(superClassId), result);
      }
      return std::move(result);
    }

    PropertyNameMapInfo
    SchemaInterface::getPropertyNameMapInfo(const ClassId &classId, const ClassId &superClassId) {
      auto result = PropertyNameMapInfo{};
      for (const auto &property: getNativePropertyInfo(classId)) {
        result[property.name] = property;
      }
      auto inheritResult = std::vector<PropertyAccessInfo>{};
      for (const auto &property: getInheritPropertyInfo(superClassId, inheritResult)) {
        result[property.name] = property;
      }
      return result;
    }

    PropertyIdMapInfo
    SchemaInterface::getPropertyIdMapInfo(const ClassId &classId, const ClassId &superClassId) {
      auto result = PropertyIdMapInfo{};
      for (const auto &property: getNativePropertyInfo(classId)) {
        result[property.id] = property;
      }
      auto inheritResult = std::vector<PropertyAccessInfo>{};
      for (const auto &property: getInheritPropertyInfo(superClassId, inheritResult)) {
        result[property.id] = property;
      }
      return result;
    }

    IndexAccessInfo
    SchemaInterface::getIndexInfo(const ClassId &classId, const PropertyId &propertyId) {
      auto foundIndexInfo = _txn->_adapter.dbIndex()->getInfo(classId, propertyId);
      if (foundIndexInfo.id == IndexId{}) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_INDEX);
      }
      return foundIndexInfo;
    }

  }

}
