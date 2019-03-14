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

using adapter::schema::ClassAccessInfo;
using adapter::schema::IndexAccessInfo;
using adapter::schema::PropertyAccessInfo;
using adapter::schema::PropertyIdMapInfo;
using adapter::schema::PropertyNameMapInfo;

namespace schema {

    class SchemaInterface {
    public:
        SchemaInterface(const Transaction* txn)
            : _txn { txn }
        {
        }

        virtual ~SchemaInterface() noexcept = default;

        ClassAccessInfo getExistingClass(const std::string& className);

        ClassAccessInfo getExistingClass(const ClassId& classId);

        PropertyAccessInfo getExistingProperty(const ClassId& classId, const std::string& propertyName);

        PropertyAccessInfo getExistingPropertyExtend(const ClassId& classId, const std::string& propertyName);

        template <typename T>
        ClassAccessInfo getValidClassInfo(const T& classSearchKey, ClassType type = ClassType::UNDEFINED)
        {
            auto foundClass = getExistingClass(classSearchKey);
            if (type != ClassType::UNDEFINED) {
                if (foundClass.type != type) {
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_MISMATCH_CLASSTYPE);
                }
            }
            return foundClass;
        }

        std::map<std::string, ClassAccessInfo> getSubClassInfos(const ClassId& classId);

        std::vector<PropertyAccessInfo> getNativePropertyInfo(const ClassId& classId);

        std::vector<PropertyAccessInfo>
        getInheritPropertyInfo(const ClassId& superClassId, const std::vector<PropertyAccessInfo>& result);

        PropertyNameMapInfo
        getPropertyNameMapInfo(const ClassId& classId, const ClassId& superClassId);

        PropertyIdMapInfo
        getPropertyIdMapInfo(const ClassId& classId, const ClassId& superClassId);

        IndexAccessInfo
        getIndexInfo(const ClassId& classId, const PropertyId& propertyId);

    private:
        const Transaction* _txn;

        inline PropertyNameMapInfo& addBasicInfo(PropertyNameMapInfo& propertyInfo)
        {
            propertyInfo[CLASS_NAME_PROPERTY] = PropertyAccessInfo(
                0, CLASS_NAME_PROPERTY, CLASS_NAME_PROPERTY_ID, PropertyType::TEXT);
            propertyInfo[RECORD_ID_PROPERTY] = PropertyAccessInfo(
                0, RECORD_ID_PROPERTY, RECORD_ID_PROPERTY_ID, PropertyType::UNSIGNED_SMALLINT);
            propertyInfo[DEPTH_PROPERTY] = PropertyAccessInfo(
                0, DEPTH_PROPERTY, DEPTH_PROPERTY_ID, PropertyType::UNSIGNED_SMALLINT);
            return propertyInfo;
        }

        inline PropertyIdMapInfo& addBasicInfo(PropertyIdMapInfo& propertyInfo)
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
