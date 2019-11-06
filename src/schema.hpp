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

#pragma once

#include <vector>

#include "schema_adapter.hpp"
#include "validate.hpp"

#include "nogdb/nogdb.h"
#include "nogdb/nogdb_types.h"

namespace nogdb {
namespace schema {
    using namespace adapter::schema;

    struct SchemaUtils {

        static ClassAccessInfo getExistingClass(const Transaction *txn, const std::string& className);

        static ClassAccessInfo getExistingClass(const Transaction *txn, const ClassId& classId);

        static PropertyAccessInfo getExistingProperty(const Transaction *txn,
            const ClassId& classId,
            const std::string& propertyName);

        static PropertyAccessInfo getExistingPropertyExtend(const Transaction *txn,
            const ClassId& classId,
            const std::string& propertyName);

        template <typename T>static ClassAccessInfo getValidClassInfo(const Transaction *txn,
            const T& classSearchKey,
            ClassType type = ClassType::UNDEFINED)
        {
            auto foundClass = getExistingClass(txn, classSearchKey);
            if (type != ClassType::UNDEFINED) {
                if (foundClass.type != type) {
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_MISMATCH_CLASSTYPE);
                }
            }
            return foundClass;
        }

        static std::map<std::string, ClassAccessInfo> getSubClassInfos(const Transaction *txn, const ClassId& classId);

        static std::vector<PropertyAccessInfo> getNativePropertyInfo(const Transaction *txn, const ClassId& classId);

        static std::vector<PropertyAccessInfo> getInheritPropertyInfo(const Transaction *txn,
            const ClassId& superClassId,
            const std::vector<PropertyAccessInfo>& result);

        static PropertyNameMapInfo getPropertyNameMapInfo(const Transaction *txn,
            const ClassId& classId, const ClassId& superClassId);

        static PropertyIdMapInfo getPropertyIdMapInfo(const Transaction *txn,
            const ClassId& classId,
            const ClassId& superClassId);

        static IndexAccessInfo getIndexInfo(const Transaction *txn,
            const ClassId& classId,
            const PropertyId& propertyId);

    private:

        static inline PropertyNameMapInfo& addBasicInfo(PropertyNameMapInfo& propertyInfo)
        {
            propertyInfo[CLASS_NAME_PROPERTY] = PropertyAccessInfo(
                0, CLASS_NAME_PROPERTY, CLASS_NAME_PROPERTY_ID, PropertyType::TEXT);
            propertyInfo[RECORD_ID_PROPERTY] = PropertyAccessInfo(
                0, RECORD_ID_PROPERTY, RECORD_ID_PROPERTY_ID, PropertyType::UNSIGNED_SMALLINT);
            propertyInfo[DEPTH_PROPERTY] = PropertyAccessInfo(
                0, DEPTH_PROPERTY, DEPTH_PROPERTY_ID, PropertyType::UNSIGNED_SMALLINT);
            return propertyInfo;
        }

        static inline PropertyIdMapInfo& addBasicInfo(PropertyIdMapInfo& propertyInfo)
        {
            propertyInfo[CLASS_NAME_PROPERTY_ID] = PropertyAccessInfo(
                0, CLASS_NAME_PROPERTY, CLASS_NAME_PROPERTY_ID, PropertyType::TEXT);
            propertyInfo[RECORD_ID_PROPERTY_ID] = PropertyAccessInfo(
                0, RECORD_ID_PROPERTY, RECORD_ID_PROPERTY_ID, PropertyType::UNSIGNED_SMALLINT);
            propertyInfo[DEPTH_PROPERTY_ID] = PropertyAccessInfo(
                0, DEPTH_PROPERTY, DEPTH_PROPERTY_ID, PropertyType::UNSIGNED_SMALLINT);
            return propertyInfo;
        }
    };

}
}
