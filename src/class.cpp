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
#include "storage_engine.hpp"
#include "lmdb_engine.hpp"
#include "datarecord_adapter.hpp"
#include "relation.hpp"
#include "schema.hpp"
#include "parser.hpp"
#include "validate.hpp"
#include "utils.hpp"

#include "nogdb/nogdb.h"

namespace nogdb {

  using namespace utils::assertion;
  using namespace adapter::schema;
  using namespace adapter::datarecord;

  const ClassDescriptor Transaction::addClass(const std::string &className, ClassType type) {
    BEGIN_VALIDATION(this)
        .isTransactionValid()
        .isTransactionCompleted()
        .isClassNameValid(className)
        .isClassTypeValid(type)
        .isNotDuplicatedClass(className)
        .isClassIdMaxReach();

    try {
      auto classId = _adapter->dbInfo()->getMaxClassId() + ClassId{1};
      _adapter->dbClass()->create(ClassAccessInfo{className, classId, ClassId{0}, type});
      _adapter->dbInfo()->setMaxClassId(classId);
      _adapter->dbInfo()->setNumClassId(_adapter->dbInfo()->getNumClassId() + ClassId{1});
      DataRecord(_txnBase, classId, type).init();
      return ClassDescriptor{classId, className, ClassId{0}, type};
    } catch (const Error& err) {
      rollback();
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      rollback();
      std::rethrow_exception(std::current_exception());
    }
  }

  const ClassDescriptor Transaction::addSubClassOf(const std::string &superClass, const std::string &className) {
    BEGIN_VALIDATION(this)
        .isTransactionValid()
        .isTransactionCompleted()
        .isClassNameValid(className)
        .isClassNameValid(superClass)
        .isNotDuplicatedClass(className)
        .isClassIdMaxReach();

    auto superClassInfo = _interface->schema()->getExistingClass(superClass);
    try {
      auto classId = _adapter->dbInfo()->getMaxClassId() + ClassId{1};
      _adapter->dbClass()->create(ClassAccessInfo{className, classId, superClassInfo.id, superClassInfo.type});
      _adapter->dbInfo()->setMaxClassId(classId);
      _adapter->dbInfo()->setNumClassId(_adapter->dbInfo()->getNumClassId() + ClassId{1});
      DataRecord(_txnBase, classId, superClassInfo.type).init();
      return ClassDescriptor{classId, className, superClassInfo.id, superClassInfo.type};
    } catch (const Error& err) {
      rollback();
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      rollback();
      std::rethrow_exception(std::current_exception());
    }
  }

  void Transaction::dropClass(const std::string &className) {
    BEGIN_VALIDATION(this)
        .isTransactionValid()
        .isTransactionCompleted()
        .isClassNameValid(className);

    auto foundClass = _interface->schema()->getExistingClass(className);
    // retrieve relevant properties information
    auto propertyInfos = _adapter->dbProperty()->getInfos(foundClass.id);
    for (const auto &property: propertyInfos) {
      // check if all index tables associated with the column have been removed beforehand
      auto foundIndex = _adapter->dbIndex()->getInfo(foundClass.id, property.id);
      if (foundIndex.id != IndexId{0}) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_IN_USED_PROPERTY);
      }
    }
    try {
      auto rids = std::vector<RecordId>{};
      // delete class from schema
      _adapter->dbClass()->remove(className);
      // delete properties from schema
      for (const auto &property : propertyInfos) {
        _adapter->dbProperty()->remove(property.classId, property.name);
        //TODO: implement existing index deletion if needed
      }
      // delete all associated relations
      auto table = DataRecord(_txnBase, foundClass.id, foundClass.type);
      auto cursorHandler = table.getCursor();
      for (auto keyValue = cursorHandler.getNext();
           !keyValue.empty();
           keyValue = cursorHandler.getNext()) {
        auto key = keyValue.key.data.numeric<PositionId>();
        if (key == MAX_RECORD_NUM_EM) continue;
        auto recordId = RecordId{foundClass.id, key};
        if (foundClass.type == ClassType::EDGE) {
          auto vertices = parser::RecordParser::parseEdgeRawDataVertexSrcDst(keyValue.val.data.blob());
          _interface->graph()->removeRelFromEdge(recordId, vertices.first, vertices.second);
        } else {
          _interface->graph()->removeRelFromVertex(recordId);
        }
      }
      // drop the actual table
      table.destroy();
      // update a superclass of subclasses if existing
      for (const auto &subClassInfo: _adapter->dbClass()->getSubClassInfos(foundClass.id)) {
        _adapter->dbClass()->update(
            ClassAccessInfo{
                subClassInfo.name,
                subClassInfo.id,
                foundClass.superClassId,
                subClassInfo.type
            });
      }
      // update database info
      _adapter->dbInfo()->setNumClassId(_adapter->dbInfo()->getNumClassId() - ClassId{1});
      _adapter->dbInfo()->setNumPropertyId(
          _adapter->dbInfo()->getNumPropertyId() - PropertyId{static_cast<uint16_t>(propertyInfos.size())});
    } catch (const Error& err) {
      rollback();
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      rollback();
      std::rethrow_exception(std::current_exception());
    }

  }

  void Transaction::renameClass(const std::string &oldClassName, const std::string &newClassName) {
    BEGIN_VALIDATION(this)
        .isTransactionValid()
        .isTransactionCompleted()
        .isClassNameValid(oldClassName)
        .isClassNameValid(newClassName)
        .isNotDuplicatedClass(newClassName);

    auto foundClass = _interface->schema()->getExistingClass(oldClassName);
    try {
      _adapter->dbClass()->alterClassName(oldClassName, newClassName);
    } catch (const Error& err) {
      rollback();
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      rollback();
      std::rethrow_exception(std::current_exception());
    }
  }
}

