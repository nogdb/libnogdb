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
#include "generic.hpp"
#include "parser.hpp"

#include "nogdb.h"

namespace nogdb {

    const PropertyDescriptor Property::add(Txn &txn,
                                           const std::string &className,
                                           const std::string &propertyName,
                                           PropertyType type) {
        Validate::isTransactionValid(txn);
        Validate::isPropertyNameValid(propertyName);
        Validate::isPropertyTypeValid(type);
        Validate::isPropertyIdMaxReach(txn);
        auto foundClass = Validate::isExistingClass(txn, className);
        Validate::isNotDuplicatedProperty(txn, foundClass.id, propertyName);
        Validate::isNotOverridenProperty(txn, foundClass.id, propertyName);
        try {
            auto propertyId = txn._dbinfo->getMaxPropertyId() + PropertyId{1};
            auto propertyProps = adapter::schema::PropertyAccessInfo{foundClass.id, propertyName, propertyId, type};
            txn._property->create(propertyProps);
            txn._dbinfo->setMaxPropertyId(propertyId);
            txn._dbinfo->setNumPropertyId(txn._dbinfo->getNumPropertyId() + PropertyId{1});
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
        Validate::isTransactionValid(txn);
        Validate::isPropertyNameValid(newPropertyName);
        auto foundClass = Validate::isExistingClass(txn, className);
        auto foundOldProperty = Validate::isExistingProperty(txn, foundClass.id, oldPropertyName);
        Validate::isNotDuplicatedProperty(txn, foundClass.id, newPropertyName);
        Validate::isNotOverridenProperty(txn, foundClass.id, newPropertyName);
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
        Validate::isTransactionValid(txn);
        auto foundClass = Validate::isExistingClass(txn, className);
        auto foundProperty = Validate::isExistingProperty(txn, foundClass.id, propertyName);
        // check if all index tables associated with the column have bee removed beforehand
        auto foundIndex = txn._index->getInfo(foundClass.id, foundProperty.id);
        if (foundIndex.id != IndexId{}) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_IN_USED_PROPERTY);
        }
        try {
            txn._property->remove(foundClass.id, propertyName);
            txn._dbinfo->setNumPropertyId(txn._dbinfo->getNumPropertyId() - PropertyId{1});
        } catch (const Error &err) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(err);
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

    void Property::createIndex(Txn &txn, const std::string &className, const std::string &propertyName, bool isUnique) {
        Validate::isTransactionValid(txn);
        Validate::isIndexIdMaxReach(txn);
        auto foundClass = Validate::isExistingClass(txn, className);
        auto foundProperty = Validate::isExistingPropertyExtend(txn, foundClass.id, propertyName);
        if (foundProperty.type == PropertyType::BLOB || foundProperty.type == PropertyType::UNDEFINED) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_PROPTYPE_INDEX);
        }
        auto indexInfo = txn._index->getInfo(foundClass.id, foundProperty.id);
        if (indexInfo.id != IndexId{}) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_INDEX);
        }
        try {
            auto indexId = txn._dbinfo->getMaxIndexId() + IndexId{1};
            auto indexProps = adapter::schema::IndexAccessInfo{foundClass.id, foundProperty.id, indexId, isUnique};
            txn._index->create(indexProps);
            auto indexHelper = index::IndexInterface(&txn);
            indexHelper.create(foundProperty, indexProps, foundClass.type);
            txn._dbinfo->setMaxIndexId(indexId);
            txn._dbinfo->setNumIndexId(txn._dbinfo->getNumIndexId() + IndexId{1});
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
        Validate::isTransactionValid(txn);
        auto foundClass = Validate::isExistingClass(txn, className);
        auto foundProperty = Validate::isExistingPropertyExtend(txn, foundClass.id, propertyName);
        auto indexInfo = txn._index->getInfo(foundClass.id, foundProperty.id);
        if (indexInfo.id == IndexId{}) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_INDEX);
        }
        try {
            txn._index->remove(foundClass.id, foundProperty.id);
            auto indexHelper = index::IndexInterface(&txn);
            indexHelper.drop(foundProperty, indexInfo);
            txn._dbinfo->setNumIndexId(txn._dbinfo->getNumIndexId() - IndexId{1});
        } catch (const Error &err) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(err);
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

}
