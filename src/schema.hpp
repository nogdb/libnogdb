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

#pragma once

#include <vector>

#include "schema_adapter.hpp"

#include "nogdb_txn.h"
#include "nogdb_types.h"
#include "validate.hpp"

namespace nogdb {

  namespace schema {

    using adapter::schema::ClassAccessInfo;
    using adapter::schema::PropertyAccessInfo;
    using adapter::schema::PropertyNameMapInfo;
    using adapter::schema::PropertyIdMapInfo;

    class SchemaInterface {
    public:
      SchemaInterface(const Txn *txn) : _txn{txn} {}

      virtual ~SchemaInterface() noexcept = default;

      ClassAccessInfo getExistingClass(const std::string &className) {
        auto foundClass = _txn->_class->getInfo(className);
        if (foundClass.type == ClassType::UNDEFINED) {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_CLASS);
        }
        return foundClass;
      }

      ClassAccessInfo getExistingClass(const ClassId &classId) {
        auto foundClass = _txn->_class->getInfo(classId);
        if (foundClass.type == ClassType::UNDEFINED) {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_CLASS);
        }
        return foundClass;
      }

      PropertyAccessInfo getExistingProperty(const ClassId &classId, const std::string &propertyName) {
        auto foundProperty = _txn->_property->getInfo(classId, propertyName);
        if (foundProperty.type == PropertyType::UNDEFINED) {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
        }
        return foundProperty
      }

      PropertyAccessInfo getExistingPropertyExtend(const ClassId &classId, const std::string &propertyName) {
        auto foundProperty = _txn->_property->getInfo(classId, propertyName);
        if (foundProperty.type == PropertyType::UNDEFINED) {
          auto superClassId = _txn->_class->getSuperClassId(classId);
          if (superClassId != ClassId{}) {
            return getExistingPropertyExtend(classId, propertyName);
          } else {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
          }
        } else {
          return foundProperty;
        }
      }

      template<typename T>
      ClassAccessInfo getValidClassInfo(const T &classSearchKey, ClassType type = ClassType::UNDEFINED) {
        auto foundClass = getExistingClass(classSearchKey);
        if (type != ClassType::UNDEFINED) {
          if (foundClass.type != type) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_MISMATCH_CLASSTYPE);
          }
        }
        return foundClass;
      }

      std::map<std::string, ClassAccessInfo>
      getSubClassInfos(const ClassId &classId, std::map<std::string, ClassAccessInfo> &result) {
        for (const auto &subClassInfo: _txn->_class->getSubClassInfos(classId)) {
          result.emplace(subClassInfo.name, subClassInfo);
          getSubClassInfos(subClassInfo.id, result);
        }
        return std::move(result);
      }

      std::vector<PropertyAccessInfo>
      getNativePropertyInfo(const ClassId &classId) {
        auto result = std::vector<PropertyAccessInfo>{};
        for (const auto &propertyInfo: _txn->_property->getInfos(classId)) {
          result.emplace_back(propertyInfo);
        }
        return result;
      }

      std::vector<PropertyAccessInfo>
      getInheritPropertyInfo(const ClassId &superClassId, std::vector<PropertyAccessInfo> &result) {
        if (superClassId != ClassId{}) {
          result.emplace_back(_txn->_property->getInfos(superClassId));
          getInheritPropertyInfo(_txn->_class->getSuperClassId(superClassId));
        }
        return std::move(result);
      }

      PropertyNameMapInfo
      getPropertyNameMapInfo(const ClassId &classId, const ClassId &superClassId) {
        auto result = PropertyNameMapInfo{};
        for (const auto &property: getNativePropertyInfo(classId)) {
          result[property.name] = property;
        }
        for (const auto &property: getInheritPropertyInfo(superClassId)) {
          result[property.name] = property;
        }
        return result;
      }

      PropertyIdMapInfo
      getPropertyIdMapInfo(const ClassId &classId, const ClassId &superClassId) {
        auto result = PropertyIdMapInfo{};
        for (const auto &property: getNativePropertyInfo(classId)) {
          result[property.id] = property;
        }
        for (const auto &property: getInheritPropertyInfo(superClassId)) {
          result[property.id] = property;
        }
        return result;
      }

    private:
      const Txn *_txn;

    };

  }

}
