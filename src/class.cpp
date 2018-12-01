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

#include <iostream> // for debugging
#include <memory>

#include "constant.hpp"
#include "storage_engine.hpp"
#include "lmdb_engine.hpp"
#include "datarecord_adapter.hpp"
#include "relation.hpp"
#include "schema.hpp"
#include "parser.hpp"
#include "validate.hpp"
#include "utils.hpp"

#include "nogdb.h"

using namespace nogdb::utils::assertion;

namespace nogdb {

  using namespace adapter::schema;
  using namespace adapter::datarecord;

  const ClassDescriptor Class::create(Txn &txn, const std::string &className, ClassType type) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isClassIdMaxReach()
        .isClassNameValid(className)
        .isClassTypeValid(type)
        .isNotDuplicatedClass(className);

    try {
      auto classId = txn._adapter->dbInfo()->getMaxClassId() + ClassId{1};
      txn._adapter->dbClass()->create(ClassAccessInfo{className, classId, ClassId{0}, type});
      txn._adapter->dbInfo()->setMaxClassId(classId);
      txn._adapter->dbInfo()->setNumClassId(txn._adapter->dbInfo()->getNumClassId() + ClassId{1});
      DataRecord(txn._txnBase, classId, type).init();
      return ClassDescriptor{classId, className, ClassId{0}, type};
    } catch (const Error &err) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      txn.rollback();
      std::rethrow_exception(std::current_exception());
    }
  }

  const ClassDescriptor
  Class::createExtend(Txn &txn, const std::string &className, const std::string &superClass) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isClassIdMaxReach()
        .isClassNameValid(className)
        .isClassNameValid(superClass)
        .isNotDuplicatedClass(className);

    auto superClassInfo = txn._interface->schema()->getExistingClass(superClass);
    try {
      auto classId = txn._adapter->dbInfo()->getMaxClassId() + ClassId{1};
      txn._adapter->dbClass()->create(ClassAccessInfo{className, classId, superClassInfo.id, superClassInfo.type});
      txn._adapter->dbInfo()->setMaxClassId(classId);
      txn._adapter->dbInfo()->setNumClassId(txn._adapter->dbInfo()->getNumClassId() + ClassId{1});
      DataRecord(txn._txnBase, classId, superClassInfo.type).init();
      return ClassDescriptor{classId, className, superClassInfo.id, superClassInfo.type};
    } catch (const Error &err) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      txn.rollback();
      std::rethrow_exception(std::current_exception());
    }
  }

  void Class::drop(Txn &txn, const std::string &className) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isClassNameValid(className);

    auto foundClass = txn._interface->schema()->getExistingClass(className);
    // retrieve relevant properties information
    auto propertyInfos = txn._adapter->dbProperty()->getInfos(foundClass.id);
    for (const auto &property: propertyInfos) {
      // check if all index tables associated with the column have been removed beforehand
      auto foundIndex = txn._adapter->dbIndex()->getInfo(foundClass.id, property.id);
      if (foundIndex.id != IndexId{0}) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_IN_USED_PROPERTY);
      }
    }
    try {
      auto rids = std::vector<RecordId>{};
      // delete class from schema
      txn._adapter->dbClass()->remove(className);
      // delete properties from schema
      for (const auto &property : propertyInfos) {
        txn._adapter->dbProperty()->remove(property.classId, property.name);
        //TODO: implement existing index deletion if needed
      }
      // delete all associated relations
      auto table = DataRecord(txn._txnBase, foundClass.id, foundClass.type);
      auto cursorHandler = table.getCursor();
      for (auto keyValue = cursorHandler.getNext();
           !keyValue.empty();
           keyValue = cursorHandler.getNext()) {
        auto key = keyValue.key.data.numeric<PositionId>();
        if (key == MAX_RECORD_NUM_EM) continue;
        auto recordId = RecordId{foundClass.id, key};
        if (foundClass.type == ClassType::EDGE) {
          auto vertices = parser::RecordParser::parseEdgeRawDataVertexSrcDst(keyValue.val.data.blob());
          txn._interface->graph()->removeRelFromEdge(recordId, vertices.first, vertices.second);
        } else {
          txn._interface->graph()->removeRelFromVertex(recordId);
        }
      }
      // drop the actual table
      table.destroy();
      // update a superclass of subclasses if existing
      for (const auto &subClassInfo: txn._adapter->dbClass()->getSubClassInfos(foundClass.id)) {
        txn._adapter->dbClass()->update(
            ClassAccessInfo{
                subClassInfo.name,
                subClassInfo.id,
                foundClass.superClassId,
                subClassInfo.type
            });
      }
      // update database info
      txn._adapter->dbInfo()->setNumClassId(txn._adapter->dbInfo()->getNumClassId() - ClassId{1});
      txn._adapter->dbInfo()->setNumPropertyId(
          txn._adapter->dbInfo()->getNumPropertyId() - PropertyId{static_cast<uint16_t>(propertyInfos.size())});
    } catch (const Error &err) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      txn.rollback();
      std::rethrow_exception(std::current_exception());
    }
  }

  void Class::alter(Txn &txn, const std::string &oldClassName, const std::string &newClassName) {
    BEGIN_VALIDATION(&txn)
        .isTransactionValid()
        .isClassNameValid(oldClassName)
        .isClassNameValid(newClassName)
        .isNotDuplicatedClass(newClassName);

    auto foundClass = txn._interface->schema()->getExistingClass(oldClassName);
    try {
      txn._adapter->dbClass()->alterClassName(oldClassName, newClassName);
    } catch (const Error &err) {
      txn.rollback();
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      txn.rollback();
      std::rethrow_exception(std::current_exception());
    }
  }

}

