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

#include <memory>

#include "index.hpp"
#include "lmdb_engine.hpp"
#include "schema.hpp"
#include "validate.hpp"

#include "nogdb/nogdb.h"

namespace nogdb {
using namespace adapter::schema;
using namespace schema;
using namespace index;

const PropertyDescriptor Transaction::addProperty(const std::string& className,
    const std::string& propertyName,
    PropertyType type)
{
    auto validators = BEGIN_VALIDATION(this)
                          .isTxnValid()
                          .isTxnCompleted()
                          .isClassNameValid(className)
                          .isPropertyNameValid(propertyName)
                          .isPropertyTypeValid(type)
                          .isPropertyIdMaxReach();

    auto foundClass = SchemaUtils::getExistingClass(this, className);
    validators.isNotDuplicatedProperty(foundClass.id, propertyName);
    validators.isNotOverriddenProperty(foundClass.id, propertyName);

    try {
        auto propertyId = _adapter->dbInfo()->getMaxPropertyId() + PropertyId { 1 };
        auto propertyProps = PropertyAccessInfo { foundClass.id, propertyName, propertyId, type };
        _adapter->dbProperty()->create(propertyProps);
        _adapter->dbInfo()->setMaxPropertyId(propertyId);
        _adapter->dbInfo()->setNumPropertyId(_adapter->dbInfo()->getNumPropertyId() + PropertyId { 1 });
        return PropertyDescriptor { propertyProps.id, propertyName, type, false };
    } catch (const Error& err) {
        rollback();
        throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
        rollback();
        std::rethrow_exception(std::current_exception());
    }
}

void Transaction::renameProperty(const std::string& className,
    const std::string& oldPropertyName,
    const std::string& newPropertyName)
{
    auto validators = BEGIN_VALIDATION(this)
                          .isTxnValid()
                          .isTxnCompleted()
                          .isClassNameValid(className)
                          .isPropertyNameValid(oldPropertyName)
                          .isPropertyNameValid(newPropertyName);

    auto foundClass = SchemaUtils::getExistingClass(this, className);
    validators.isNotDuplicatedProperty(foundClass.id, newPropertyName);
    validators.isNotOverriddenProperty(foundClass.id, newPropertyName);

    auto foundOldProperty = SchemaUtils::getExistingProperty(this, foundClass.id, oldPropertyName);
    try {
        _adapter->dbProperty()->alterPropertyName(foundClass.id, oldPropertyName, newPropertyName);
    } catch (const Error& err) {
        rollback();
        throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
        rollback();
        std::rethrow_exception(std::current_exception());
    }
}

void Transaction::dropProperty(const std::string& className, const std::string& propertyName)
{
    BEGIN_VALIDATION(this)
        .isTxnValid()
        .isTxnCompleted()
        .isClassNameValid(className)
        .isPropertyNameValid(propertyName);

    auto foundClass = SchemaUtils::getExistingClass(this, className);
    auto foundProperty = SchemaUtils::getExistingProperty(this, foundClass.id, propertyName);
    // check if all index tables associated with the column have bee removed beforehand
    auto foundIndex = _adapter->dbIndex()->getInfo(foundClass.id, foundProperty.id);
    if (foundIndex.id != IndexId {}) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_IN_USED_PROPERTY);
    }
    try {
        _adapter->dbProperty()->remove(foundClass.id, propertyName);
        _adapter->dbInfo()->setNumPropertyId(_adapter->dbInfo()->getNumPropertyId() - PropertyId { 1 });
    } catch (const Error& err) {
        rollback();
        throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
        rollback();
        std::rethrow_exception(std::current_exception());
    }
}

const IndexDescriptor Transaction::addIndex(const std::string& className,
    const std::string& propertyName,
    bool isUnique)
{
    BEGIN_VALIDATION(this)
        .isTxnValid()
        .isTxnCompleted()
        .isClassNameValid(className)
        .isPropertyNameValid(propertyName)
        .isIndexIdMaxReach();

    auto foundClass = SchemaUtils::getExistingClass(this, className);
    auto foundProperty = SchemaUtils::getExistingPropertyExtend(this, foundClass.id, propertyName);
    if (foundProperty.type == PropertyType::BLOB || foundProperty.type == PropertyType::UNDEFINED) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_PROPTYPE_INDEX);
    }
    auto indexInfo = _adapter->dbIndex()->getInfo(foundClass.id, foundProperty.id);
    if (indexInfo.id != IndexId {}) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_INDEX);
    }
    try {
        auto indexId = _adapter->dbInfo()->getMaxIndexId() + IndexId { 1 };
        auto indexProps = IndexAccessInfo { foundClass.id, foundProperty.id, indexId, isUnique };
        // create index metadata in schema
        _adapter->dbIndex()->create(indexProps);
        // create index record in index database
        IndexUtils::initialize(this, foundProperty, indexProps, foundClass.superClassId, foundClass.type);
        _adapter->dbInfo()->setMaxIndexId(indexId);
        _adapter->dbInfo()->setNumIndexId(_adapter->dbInfo()->getNumIndexId() + IndexId { 1 });
        return IndexDescriptor {
            indexId,
            foundClass.id,
            foundProperty.id,
            isUnique
        };
    } catch (const Error& err) {
        if (err.code() == MDB_KEYEXIST) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_INDEX_CONSTRAINT);
        } else {
            rollback();
            throw NOGDB_FATAL_ERROR(err);
        }
    } catch (...) {
        rollback();
        std::rethrow_exception(std::current_exception());
    }
}

void Transaction::dropIndex(const std::string& className, const std::string& propertyName)
{
    BEGIN_VALIDATION(this)
        .isTxnValid()
        .isTxnCompleted()
        .isClassNameValid(className)
        .isPropertyNameValid(propertyName);

    auto foundClass = SchemaUtils::getExistingClass(this, className);
    auto foundProperty = SchemaUtils::getExistingPropertyExtend(this, foundClass.id, propertyName);
    auto indexInfo = _adapter->dbIndex()->getInfo(foundClass.id, foundProperty.id);
    if (indexInfo.id == IndexId {}) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_INDEX);
    }
    try {
        // remove index metadata from schema
        _adapter->dbIndex()->remove(foundClass.id, foundProperty.id);
        // remove all index data from index database
        IndexUtils::drop(this, foundProperty, indexInfo);
        _adapter->dbInfo()->setNumIndexId(_adapter->dbInfo()->getNumIndexId() - IndexId { 1 });
    } catch (const Error& err) {
        rollback();
        throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
        rollback();
        std::rethrow_exception(std::current_exception());
    }
}

}
