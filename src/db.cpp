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

#include <vector>

#include "schema.hpp"
#include "lmdb_engine.hpp"
#include "datarecord.hpp"
#include "datarecord_adapter.hpp"
#include "parser.hpp"

#include "nogdb.h"

namespace nogdb {

  DBInfo DB::getDBInfo(const Txn &txn) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted();

    auto dbInfo = DBInfo{};
    dbInfo.maxClassId = txn._adapter->dbInfo()->getMaxClassId();
    dbInfo.maxPropertyId = txn._adapter->dbInfo()->getMaxPropertyId();
    dbInfo.maxIndexId = txn._adapter->dbInfo()->getMaxIndexId();
    dbInfo.numClass = txn._adapter->dbInfo()->getNumClassId();
    dbInfo.numProperty = txn._adapter->dbInfo()->getNumPropertyId();
    dbInfo.numIndex = txn._adapter->dbInfo()->getNumIndexId();
    return dbInfo;
  }

  Record DB::getRecord(const Txn &txn, const RecordDescriptor &recordDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted();

    auto classInfo = txn._interface->schema()->getValidClassInfo(recordDescriptor.rid.first);
    return txn._interface->record()->getRecordWithBasicInfo(classInfo, recordDescriptor);
  }

  const std::vector<ClassDescriptor> DB::getClasses(const Txn &txn) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted();

    auto result = std::vector<ClassDescriptor>{};
    for (const auto &classInfo: txn._adapter->dbClass()->getAllInfos()) {
      result.emplace_back(
          ClassDescriptor{
              classInfo.id,
              classInfo.name,
              classInfo.superClassId,
              classInfo.type
          });
    }
    return result;
  }

  const std::vector<PropertyDescriptor> DB::getProperties(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto result = std::vector<PropertyDescriptor>{};
    auto foundClass = txn._interface->schema()->getExistingClass(className);
    // native properties
    for (const auto &property: txn._interface->schema()->getNativePropertyInfo(foundClass.id)) {
      result.emplace_back(
          PropertyDescriptor{
              property.id,
              property.name,
              property.type,
              false
          });
    }
    // inherited properties
    auto inheritResult = txn._interface->schema()->getInheritPropertyInfo(
        txn._adapter->dbClass()->getSuperClassId(foundClass.id),
        std::vector<schema::PropertyAccessInfo>{});
    for (const auto &property: inheritResult) {
      result.emplace_back(
          PropertyDescriptor{
              property.id,
              property.name,
              property.type,
              true
          });
    }
    return result;
  }

  const std::vector<PropertyDescriptor> DB::getProperties(const Txn &txn, const ClassDescriptor &classDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted();

    auto result = std::vector<PropertyDescriptor>{};
    auto foundClass = txn._interface->schema()->getExistingClass(classDescriptor.id);
    // native properties
    for (const auto &property: txn._interface->schema()->getNativePropertyInfo(foundClass.id)) {
      result.emplace_back(
          PropertyDescriptor{
              property.id,
              property.name,
              property.type,
              false
          });
    }
    // inherited properties
    auto inheritResult = txn._interface->schema()->getInheritPropertyInfo(
        txn._adapter->dbClass()->getSuperClassId(foundClass.id),
        std::vector<schema::PropertyAccessInfo>{});
    for (const auto &property: inheritResult) {
      result.emplace_back(
          PropertyDescriptor{
              property.id,
              property.name,
              property.type,
              true
          });
    }
    return result;
  }

  const ClassDescriptor DB::getClass(const Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className);

    auto classInfo = txn._interface->schema()->getExistingClass(className);
    return ClassDescriptor{
        classInfo.id,
        classInfo.name,
        classInfo.superClassId,
        classInfo.type
    };
  }

  const ClassDescriptor DB::getClass(const Txn &txn, const ClassId &classId) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted();

    auto classInfo = txn._interface->schema()->getExistingClass(classId);
    return ClassDescriptor{
        classInfo.id,
        classInfo.name,
        classInfo.superClassId,
        classInfo.type
    };
  }

  const PropertyDescriptor
  DB::getProperty(const Txn &txn, const std::string &className, const std::string &propertyName) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className)
        .isPropertyNameValid(propertyName);

    auto classInfo = txn._interface->schema()->getExistingClass(className);
    try {
      auto propertyInfo = txn._interface->schema()->getExistingProperty(classInfo.id, propertyName);
      return PropertyDescriptor{
          propertyInfo.id,
          propertyInfo.name,
          propertyInfo.type,
          false
      };
    } catch (const Error &error) {
      if (error.code() == NOGDB_CTX_NOEXST_PROPERTY) {
        auto propertyInfo = txn._interface->schema()->getExistingPropertyExtend(classInfo.id, propertyName);
        return PropertyDescriptor{
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

  const std::vector<IndexDescriptor> DB::getIndexes(const Txn &txn, const ClassDescriptor &classDescriptor) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted();

    auto classInfo = txn._interface->schema()->getExistingClass(classDescriptor.id);
    auto indexInfos = txn._adapter->dbIndex()->getInfos(classInfo.id);
    auto indexDescriptors = std::vector<IndexDescriptor>{};
    for (const auto &indexInfo: indexInfos) {
      indexDescriptors.emplace_back(IndexDescriptor{
          indexInfo.id,
          indexInfo.classId,
          indexInfo.propertyId,
          indexInfo.isUnique
      });
    }
    return indexDescriptors;
  }

  const IndexDescriptor DB::getIndex(const Txn &txn, const std::string &className, const std::string &propertyName) {
    BEGIN_VALIDATION(&txn)
        .isTxnCompleted()
        .isClassNameValid(className)
        .isPropertyNameValid(propertyName);

    auto classInfo = txn._interface->schema()->getExistingClass(className);
    auto propertyInfo = txn._interface->schema()->getExistingPropertyExtend(classInfo.id, propertyName);
    auto indexInfo = txn._interface->schema()->getIndexInfo(classInfo.id, propertyInfo.id);
    return IndexDescriptor{
        indexInfo.id,
        indexInfo.classId,
        indexInfo.propertyId,
        indexInfo.isUnique
    };
  }
}


