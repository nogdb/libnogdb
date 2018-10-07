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
    auto dbInfo = DBInfo{};
    dbInfo.maxClassId = txn._dbInfo->getMaxClassId();
    dbInfo.maxPropertyId = txn._dbInfo->getMaxPropertyId();
    dbInfo.maxIndexId = txn._dbInfo->getMaxIndexId();
    dbInfo.numClass = txn._dbInfo->getNumClassId();
    dbInfo.numProperty = txn._dbInfo->getNumPropertyId();
    dbInfo.numIndex = txn._dbInfo->getNumIndexId();
    return dbInfo;
  }

  Record DB::getRecord(const Txn &txn, const RecordDescriptor &recordDescriptor) {
    auto classInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first);
    return txn._iRecord->getRecord(classInfo, recordDescriptor);
  }

  const std::vector<ClassDescriptor> DB::getClasses(const Txn &txn) {
    auto result = std::vector<ClassDescriptor>{};
    for (const auto &classInfo: txn._class->getAllInfos()) {
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

  const std::vector<PropertyDescriptor> DB::getProperties(const Txn &txn, const ClassDescriptor &classDescriptor) {
    auto result = std::vector<PropertyDescriptor>{};
    auto foundClass = txn._iSchema->getExistingClass(classDescriptor.id);
    // native properties
    for (const auto &property: txn._iSchema->getNativePropertyInfo(foundClass.id)) {
      result.emplace_back(
          PropertyDescriptor{
              property.id,
              property.name,
              property.type,
              false
          });
    }
    // inherited properties
    auto inheritResult = std::vector<schema::PropertyAccessInfo>{};
    for (const auto &property: txn._iSchema->getInheritPropertyInfo(txn._class->getSuperClassId(foundClass.id),
                                                                    inheritResult)) {
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
    auto classInfo = txn._iSchema->getExistingClass(className);
    return ClassDescriptor{
        classInfo.id,
        classInfo.name,
        classInfo.superClassId,
        classInfo.type
    };
  }

  const ClassDescriptor DB::getClass(const Txn &txn, const ClassId &classId) {
    auto classInfo = txn._iSchema->getExistingClass(classId);
    return ClassDescriptor{
        classInfo.id,
        classInfo.name,
        classInfo.superClassId,
        classInfo.type
    };
  }

  const PropertyDescriptor
  DB::getProperty(const Txn &txn, const std::string &className, const std::string &propertyName) {
    auto classInfo = txn._iSchema->getExistingClass(className);
    try {
      auto propertyInfo = txn._iSchema->getExistingProperty(classInfo.id, propertyName);
      return PropertyDescriptor{
        propertyInfo.id,
        propertyInfo.name,
        propertyInfo.type,
        false
      };
    } catch(const Error& error) {
      if (error.code() == NOGDB_CTX_NOEXST_PROPERTY) {
        auto propertyInfo = txn._iSchema->getExistingPropertyExtend(classInfo.id, propertyName);
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

  const std::vector<IndexDescriptor> DB::getIndexes(const Txn& txn, const ClassDescriptor& classDescriptor) {
    auto classInfo = txn._iSchema->getExistingClass(classDescriptor.id);
    auto indexInfos = txn._index->getInfos(classInfo.id);
    auto indexDescriptors = std::vector<IndexDescriptor>{};
    for(const auto& indexInfo: indexInfos) {
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
    auto classInfo = txn._iSchema->getExistingClass(className);
    auto propertyInfo = txn._iSchema->getExistingPropertyExtend(classInfo.id, propertyName);
    auto indexInfo = txn._iSchema->getIndexInfo(classInfo.id, propertyInfo.id);
    return IndexDescriptor{
        indexInfo.id,
        indexInfo.classId,
        indexInfo.propertyId,
        indexInfo.isUnique
    };
  }
}


