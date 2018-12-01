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
    auto validators = BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isClassNameValid(className)
        .isPropertyNameValid(propertyName)
        .isPropertyTypeValid(type)
        .isPropertyIdMaxReach();

    auto foundClass = txn._interface->schema()->getExistingClass(className);
    validators.isNotDuplicatedProperty(foundClass.id, propertyName);
    validators.isNotOverridenProperty(foundClass.id, propertyName);

    try {
      auto propertyId = txn._adapter->dbInfo()->getMaxPropertyId() + PropertyId{1};
      auto propertyProps = PropertyAccessInfo{foundClass.id, propertyName, propertyId, type};
      txn._adapter->dbProperty()->create(propertyProps);
      txn._adapter->dbInfo()->setMaxPropertyId(propertyId);
      txn._adapter->dbInfo()->setNumPropertyId(txn._adapter->dbInfo()->getNumPropertyId() + PropertyId{1});
      return PropertyDescriptor{propertyProps.id, propertyName, type, false};
    } catch (const Error *err) {
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
    auto validators = BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isClassNameValid(className)
        .isPropertyNameValid(oldPropertyName)
        .isPropertyNameValid(newPropertyName);

    auto foundClass = txn._interface->schema()->getExistingClass(className);
    validators.isNotDuplicatedProperty(foundClass.id, newPropertyName);
    validators.isNotOverridenProperty(foundClass.id, newPropertyName);

    auto foundOldProperty = txn._interface->schema()->getExistingProperty(foundClass.id, oldPropertyName);
    try {
      txn._adapter->dbProperty()->alterPropertyName(foundClass.id, oldPropertyName, newPropertyName);
    } catch (const Error *err) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      txn.rollback();
      std::rethrow_exception(std::current_exception());
    }
  }

  void Property::remove(Txn &txn, const std::string &className, const std::string &propertyName) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isClassNameValid(className)
        .isPropertyNameValid(propertyName);

    auto foundClass = txn._interface->schema()->getExistingClass(className);
    auto foundProperty = txn._interface->schema()->getExistingProperty(foundClass.id, propertyName);
    // check if all index tables associated with the column have bee removed beforehand
    auto foundIndex = txn._adapter->dbIndex()->getInfo(foundClass.id, foundProperty.id);
    if (foundIndex.id != IndexId{}) {
      throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_IN_USED_PROPERTY);
    }
    try {
      txn._adapter->dbProperty()->remove(foundClass.id, propertyName);
      txn._adapter->dbInfo()->setNumPropertyId(txn._adapter->dbInfo()->getNumPropertyId() - PropertyId{1});
    } catch (const Error *err) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      txn.rollback();
      std::rethrow_exception(std::current_exception());
    }
  }

  const IndexDescriptor
  Property::createIndex(Txn &txn, const std::string &className, const std::string &propertyName, bool isUnique) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isClassNameValid(className)
        .isPropertyNameValid(propertyName)
        .isIndexIdMaxReach();

    auto foundClass = txn._interface->schema()->getExistingClass(className);
    auto foundProperty = txn._interface->schema()->getExistingPropertyExtend(foundClass.id, propertyName);
    if (foundProperty.type == PropertyType::BLOB || foundProperty.type == PropertyType::UNDEFINED) {
      throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_PROPTYPE_INDEX);
    }
    auto indexInfo = txn._adapter->dbIndex()->getInfo(foundClass.id, foundProperty.id);
    if (indexInfo.id != IndexId{}) {
      throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_INDEX);
    }
    try {
      auto indexId = txn._adapter->dbInfo()->getMaxIndexId() + IndexId{1};
      auto indexProps = IndexAccessInfo{foundClass.id, foundProperty.id, indexId, isUnique};
      // create index metadata in schema
      txn._adapter->dbIndex()->create(indexProps);
      // create index record in index database
      txn._interface->index()->initialize(foundProperty, indexProps, foundClass.type);
      txn._adapter->dbInfo()->setMaxIndexId(indexId);
      txn._adapter->dbInfo()->setNumIndexId(txn._adapter->dbInfo()->getNumIndexId() + IndexId{1});
      return IndexDescriptor{
        indexId,
        foundClass.id,
        foundProperty.id,
        isUnique
      };
    } catch (const Error *err) {
      if (err->code() == MDB_KEYEXIST) {
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
        .isTransactionValid()
        .isClassNameValid(className)
        .isPropertyNameValid(propertyName);

    auto foundClass = txn._interface->schema()->getExistingClass(className);
    auto foundProperty = txn._interface->schema()->getExistingPropertyExtend(foundClass.id, propertyName);
    auto indexInfo = txn._adapter->dbIndex()->getInfo(foundClass.id, foundProperty.id);
    if (indexInfo.id == IndexId{}) {
      throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_INDEX);
    }
    try {
      // remove index metadata from schema
      txn._adapter->dbIndex()->remove(foundClass.id, foundProperty.id);
      // remove all index data from index database
      txn._interface->index()->drop(foundProperty, indexInfo);
      txn._adapter->dbInfo()->setNumIndexId(txn._adapter->dbInfo()->getNumIndexId() - IndexId{1});
    } catch (const Error *err) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      txn.rollback();
      std::rethrow_exception(std::current_exception());
    }
  }

}
