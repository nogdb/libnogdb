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

#include <vector>

#include "datarecord.hpp"
#include "datarecord_adapter.hpp"
#include "lmdb_engine.hpp"
#include "parser.hpp"
#include "schema.hpp"

#include "nogdb/nogdb.h"

namespace nogdb {

using adapter::schema::PropertyAccessInfo;

const DbInfo Transaction::getDbInfo() const
{
    BEGIN_VALIDATION(this)
        .isTxnCompleted();

    auto dbInfo = DbInfo {};
    dbInfo.maxClassId = _adapter->dbInfo()->getMaxClassId();
    dbInfo.maxPropertyId = _adapter->dbInfo()->getMaxPropertyId();
    dbInfo.maxIndexId = _adapter->dbInfo()->getMaxIndexId();
    dbInfo.numClass = _adapter->dbInfo()->getNumClassId();
    dbInfo.numProperty = _adapter->dbInfo()->getNumPropertyId();
    dbInfo.numIndex = _adapter->dbInfo()->getNumIndexId();
    return dbInfo;
}

const std::vector<ClassDescriptor> Transaction::getClasses() const
{
    BEGIN_VALIDATION(this)
        .isTxnCompleted();

    auto result = std::vector<ClassDescriptor> {};
    for (const auto& classInfo : _adapter->dbClass()->getAllInfos()) {
        result.emplace_back(
            ClassDescriptor {
                classInfo.id,
                classInfo.name,
                classInfo.superClassId,
                classInfo.type });
    }
    return result;
}

const std::vector<PropertyDescriptor> Transaction::getProperties(const std::string& className) const
{
    BEGIN_VALIDATION(this)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto result = std::vector<PropertyDescriptor> {};
    auto foundClass = _interface->schema()->getExistingClass(className);
    // native properties
    for (const auto& property : _interface->schema()->getNativePropertyInfo(foundClass.id)) {
        result.emplace_back(
            PropertyDescriptor {
                property.id,
                property.name,
                property.type,
                false });
    }
    // inherited properties
    auto inheritResult = _interface->schema()->getInheritPropertyInfo(
        _adapter->dbClass()->getSuperClassId(foundClass.id), std::vector<PropertyAccessInfo> {});
    for (const auto& property : inheritResult) {
        result.emplace_back(
            PropertyDescriptor {
                property.id,
                property.name,
                property.type,
                true });
    }
    return result;
}

const std::vector<PropertyDescriptor> Transaction::getProperties(const ClassDescriptor& classDescriptor) const
{
    BEGIN_VALIDATION(this)
        .isTxnCompleted();

    auto result = std::vector<PropertyDescriptor> {};
    auto foundClass = _interface->schema()->getExistingClass(classDescriptor.id);
    // native properties
    for (const auto& property : _interface->schema()->getNativePropertyInfo(foundClass.id)) {
        result.emplace_back(
            PropertyDescriptor {
                property.id,
                property.name,
                property.type,
                false });
    }
    // inherited properties
    auto inheritResult = _interface->schema()->getInheritPropertyInfo(
        _adapter->dbClass()->getSuperClassId(foundClass.id), std::vector<PropertyAccessInfo> {});
    for (const auto& property : inheritResult) {
        result.emplace_back(
            PropertyDescriptor {
                property.id,
                property.name,
                property.type,
                true });
    }
    return result;
}

const std::vector<IndexDescriptor> Transaction::getIndexes(const ClassDescriptor& classDescriptor) const
{
    BEGIN_VALIDATION(this)
        .isTxnCompleted();

    auto classInfo = _interface->schema()->getExistingClass(classDescriptor.id);
    auto indexInfos = _adapter->dbIndex()->getInfos(classInfo.id);
    auto indexDescriptors = std::vector<IndexDescriptor> {};
    for (const auto& indexInfo : indexInfos) {
        indexDescriptors.emplace_back(IndexDescriptor {
            indexInfo.id,
            indexInfo.classId,
            indexInfo.propertyId,
            indexInfo.isUnique });
    }
    return indexDescriptors;
}

const ClassDescriptor Transaction::getClass(const std::string& className) const
{
    BEGIN_VALIDATION(this)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto classInfo = _interface->schema()->getExistingClass(className);
    return ClassDescriptor {
        classInfo.id,
        classInfo.name,
        classInfo.superClassId,
        classInfo.type
    };
}

const ClassDescriptor Transaction::getClass(const ClassId& classId) const
{
    BEGIN_VALIDATION(this)
        .isTxnCompleted();

    auto classInfo = _interface->schema()->getExistingClass(classId);
    return ClassDescriptor {
        classInfo.id,
        classInfo.name,
        classInfo.superClassId,
        classInfo.type
    };
}

const PropertyDescriptor Transaction::getProperty(const std::string& className,
    const std::string& propertyName) const
{
    BEGIN_VALIDATION(this)
        .isTxnCompleted()
        .isClassNameValid(className)
        .isPropertyNameValid(propertyName);

    auto classInfo = _interface->schema()->getExistingClass(className);
    try {
        auto propertyInfo = _interface->schema()->getExistingProperty(classInfo.id, propertyName);
        return PropertyDescriptor {
            propertyInfo.id,
            propertyInfo.name,
            propertyInfo.type,
            false
        };
    } catch (const Error& error) {
        if (error.code() == NOGDB_CTX_NOEXST_PROPERTY) {
            auto propertyInfo = _interface->schema()->getExistingPropertyExtend(classInfo.id, propertyName);
            return PropertyDescriptor {
                propertyInfo.id,
                propertyInfo.name,
                propertyInfo.type,
                true
            };
        } else {
            throw error;
        }
    }
}

const IndexDescriptor Transaction::getIndex(const std::string& className, const std::string& propertyName) const
{
    BEGIN_VALIDATION(this)
        .isTxnCompleted()
        .isClassNameValid(className)
        .isPropertyNameValid(propertyName);

    auto classInfo = _interface->schema()->getExistingClass(className);
    auto propertyInfo = _interface->schema()->getExistingPropertyExtend(classInfo.id, propertyName);
    auto indexInfo = _interface->schema()->getIndexInfo(classInfo.id, propertyInfo.id);
    return IndexDescriptor {
        indexInfo.id,
        indexInfo.classId,
        indexInfo.propertyId,
        indexInfo.isUnique
    };
}

Record Transaction::fetchRecord(const RecordDescriptor& recordDescriptor) const
{
    BEGIN_VALIDATION(this)
        .isTxnCompleted();

    auto classInfo = _interface->schema()->getValidClassInfo(recordDescriptor.rid.first);
    return _interface->record()->getRecordWithBasicInfo(classInfo, recordDescriptor);
}

}
