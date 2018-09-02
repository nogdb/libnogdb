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

#include <memory>

#include "constant.hpp"
#include "lmdb_engine.hpp"
#include "index_adapter.hpp"
#include "index.hpp"
#include "validate.hpp"
#include "schema.hpp"
#include "parser.hpp"

#include "nogdb.h"

namespace nogdb {

    using namespace adapter::schema;

    const PropertyDescriptor Property::add(Txn &txn,
                                           const std::string &className,
                                           const std::string &propertyName,
                                           PropertyType type) {

        auto foundClass = txn._iSchema->getExistingClass(className);

        BEGIN_VALIDATION(&txn)
        . isTransactionValid(txn)
        . isPropertyNameValid(propertyName)
        . isPropertyTypeValid(type)
        . isPropertyIdMaxReach(txn)
        . isNotDuplicatedProperty(foundClass.id, propertyName)
        . isNotOverridenProperty(foundClass.id, propertyName);

        try {
            auto propertyId = txn._dbInfo->getMaxPropertyId() + PropertyId{1};
            auto propertyProps = PropertyAccessInfo{foundClass.id, propertyName, propertyId, type};
            txn._property->create(propertyProps);
            txn._dbInfo->setMaxPropertyId(propertyId);
            txn._dbInfo->setNumPropertyId(txn._dbInfo->getNumPropertyId() + PropertyId{1});
            return PropertyDescriptor{propertyProps.id, propertyName, type, false};
        } catch (const Error &err) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(err);
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

    void Property::alter(Txn &txn,
                         const std::string &className,
                         const std::string &oldPropertyName,
                         const std::string &newPropertyName) {

        auto foundClass = txn._iSchema->getExistingClass(className);

        BEGIN_VALIDATION(&txn)
        . isTransactionValid()
        . isPropertyNameValid(newPropertyName)
        . isNotDuplicatedProperty(foundClass.id, newPropertyName)
        . isNotOverridenProperty(foundClass.id, newPropertyName);

        auto foundOldProperty = txn._iSchema->getExistingProperty(foundClass.id, oldPropertyName);
        try {
            txn._property->alterPropertyName(foundClass.id, oldPropertyName, newPropertyName);
        } catch (const Error &err) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(err);
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

    void Property::remove(Txn &txn, const std::string &className, const std::string &propertyName) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto foundClass = txn._iSchema->getExistingClass(className);
        auto foundProperty = txn._iSchema->getExistingProperty(foundClass.id, propertyName);
        // check if all index tables associated with the column have bee removed beforehand
        auto foundIndex = txn._index->getInfo(foundClass.id, foundProperty.id);
        if (foundIndex.id != IndexId{}) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_IN_USED_PROPERTY);
        }
        try {
            txn._property->remove(foundClass.id, propertyName);
            txn._dbInfo->setNumPropertyId(txn._dbInfo->getNumPropertyId() - PropertyId{1});
        } catch (const Error &err) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(err);
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

    void Property::createIndex(Txn &txn, const std::string &className, const std::string &propertyName, bool isUnique) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid()
        . isIndexIdMaxReach();

        auto foundClass = txn._iSchema->getExistingClass(className);
        auto foundProperty = txn._iSchema->getExistingPropertyExtend(foundClass.id, propertyName);
        if (foundProperty.type == PropertyType::BLOB || foundProperty.type == PropertyType::UNDEFINED) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_PROPTYPE_INDEX);
        }
        auto indexInfo = txn._index->getInfo(foundClass.id, foundProperty.id);
        if (indexInfo.id != IndexId{}) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_INDEX);
        }
        try {
            auto indexId = txn._dbInfo->getMaxIndexId() + IndexId{1};
            auto indexProps = IndexAccessInfo{foundClass.id, foundProperty.id, indexId, isUnique};
            // create index metadata in schema
            txn._index->create(indexProps);
            // create index record in index database
            txn._iIndex->initialize(foundProperty, indexProps, foundClass.type);
            txn._dbInfo->setMaxIndexId(indexId);
            txn._dbInfo->setNumIndexId(txn._dbInfo->getNumIndexId() + IndexId{1});
        } catch (const Error &err) {
            if (err.code() == MDB_KEYEXIST) {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_INDEX_CONSTRAINT);
            } else {
                txn.rollback();
                throw NOGDB_FATAL_ERROR(err);
            }
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

    void Property::dropIndex(Txn &txn, const std::string &className, const std::string &propertyName) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto foundClass = txn._iSchema->getExistingClass(className);
        auto foundProperty = txn._iSchema->getExistingPropertyExtend(txn, foundClass.id, propertyName);
        auto indexInfo = txn._index->getInfo(foundClass.id, foundProperty.id);
        if (indexInfo.id == IndexId{}) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_INDEX);
        }
        try {
            // remove index metadata from schema
            txn._index->remove(foundClass.id, foundProperty.id);
            // remove all index data from index database
            txn._iIndex->drop(foundProperty, indexInfo);
            txn._dbInfo->setNumIndexId(txn._dbInfo->getNumIndexId() - IndexId{1});
        } catch (const Error &err) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(err);
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

}
