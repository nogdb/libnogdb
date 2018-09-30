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

#pragma once

#include <regex>

#include "schema.hpp"
#include "schema_adapter.hpp"
#include "datarecord_adapter.hpp"

#include "nogdb_errors.h"
#include "nogdb_context.h"
#include "nogdb_types.h"
#include "nogdb_txn.h"

#define BEGIN_VALIDATION(_txn)    nogdb::validate::Validator(_txn)
#define CLASS_ID_UPPER_LIMIT      UINT16_MAX - 1

namespace nogdb {

  namespace validate {

    using namespace adapter::schema;

    class Validator {
    public:
      Validator(const Txn *txn) : _txn{txn} {}

      virtual ~Validator() noexcept = default;

      Validator &isTransactionValid() {
        if (_txn->getTxnMode() == Txn::Mode::READ_ONLY) {
          throw NOGDB_TXN_ERROR(NOGDB_TXN_INVALID_MODE);
        }
        if (_txn->isCompleted()) {
          throw NOGDB_TXN_ERROR(NOGDB_TXN_COMPLETED);
        }
        return *this;
      }

      Validator &isClassIdMaxReach() {
        if ((_txn->_dbInfo->getMaxClassId() >= CLASS_ID_UPPER_LIMIT)) {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_LIMIT_DBSCHEMA);
        }
        return *this;
      }

      Validator &isPropertyIdMaxReach() {
        if ((_txn->_dbInfo->getMaxPropertyId() >= UINT16_MAX)) {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_LIMIT_DBSCHEMA);
        }
        return *this;
      }

      Validator &isIndexIdMaxReach() {
        if ((_txn->_dbInfo->getMaxIndexId() >= UINT32_MAX)) {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_LIMIT_DBSCHEMA);
        }
        return *this;
      }

      Validator &isClassNameValid(const std::string &className) {
        if (!isNameValid(className)) {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_CLASSNAME);
        }
        return *this;
      }

      Validator &isPropertyNameValid(const std::string &propName) {
        if (!isNameValid(propName)) {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_PROPERTYNAME);
        }
        return *this;
      }

      Validator &isClassTypeValid(const ClassType &type) {
        switch (type) {
          case ClassType::VERTEX:
          case ClassType::EDGE:
            return *this;
          default:
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_CLASSTYPE);
        }
      }

      Validator &isPropertyTypeValid(const PropertyType &type) {
        switch (type) {
          case PropertyType::TINYINT:
          case PropertyType::UNSIGNED_TINYINT:
          case PropertyType::SMALLINT:
          case PropertyType::UNSIGNED_SMALLINT:
          case PropertyType::INTEGER:
          case PropertyType::UNSIGNED_INTEGER:
          case PropertyType::BIGINT:
          case PropertyType::UNSIGNED_BIGINT:
          case PropertyType::TEXT:
          case PropertyType::REAL:
          case PropertyType::BLOB:
            return *this;
          default:
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_PROPTYPE);
        }
      }

      Validator &isNotDuplicatedClass(const std::string &className) {
        auto foundClass = _txn->_class->getId(className);
        if (foundClass != ClassId{}) {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_CLASS);
        }
        return *this;
      }

      Validator &isNotDuplicatedProperty(const ClassId &classId, const std::string &propertyName) {
        auto foundProperty = _txn->_property->getId(classId, propertyName);
        if (foundProperty != PropertyId{}) {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_PROPERTY);
        }
        auto superClassId = _txn._class->getSuperClassId(classId);
        if (superClassId != ClassId{}) {
          isNotDuplicatedProperty(superClassId, propertyName);
        }
        return *this;
      }

      Validator &isNotOverridenProperty(const ClassId &classId, const std::string &propertyName) {
        auto foundProperty = _txn->_property->getId(classId, propertyName);
        if (foundProperty != PropertyId{}) {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_OVERRIDE_PROPERTY);
        }
        for (const auto &subClassId: _txn->_class->getSubClassIds(classId)) {
          if (subClassId != ClassId{}) {
            isNotOverridenProperty(subClassId, propertyName);
          }
        }
        return *this;
      }

      Validator &isExistingSrcVertex(const RecordDescriptor &vertex) {
        auto foundClass = _txn->_iSchema->getExistingClass(vertex.rid.first);
        if (foundClass.type == ClassType::VERTEX) {
          auto vertexDataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, foundClass.id, ClassType::VERTEX);
          try {
            vertexDataRecord.getBlob(vertex.rid.second);
          } catch (const Error &error) {
            if (error.code() == NOGDB_CTX_NOEXST_RECORD) {
              throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_SRC);
            } else {
              throw NOGDB_FATAL_ERROR(error);
            }
          }
          return *this;
        } else {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_MISMATCH_CLASSTYPE);
        }
      }

      Validator &isExistingDstVertex(const RecordDescriptor &vertex) {
        auto foundClass = _txn->_iSchema->getExistingClass(vertex.rid.first);
        if (foundClass.type == ClassType::VERTEX) {
          auto vertexDataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, foundClass.id, ClassType::VERTEX);
          try {
            vertexDataRecord.getBlob(vertex.rid.second);
          } catch (const Error &error) {
            if (error.code() == NOGDB_CTX_NOEXST_RECORD) {
              throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_DST);
            } else {
              throw NOGDB_FATAL_ERROR(error);
            }
          }
          return *this;
        } else {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_MISMATCH_CLASSTYPE);
        }
      }

      Validator &isExistingVertex(const RecordDescriptor &vertex) {
        auto foundClass = _txn->_iSchema->getExistingClass(vertex.rid.first);
        if (foundClass.type == ClassType::VERTEX) {
          auto vertexDataRecord = adapter::datarecord::DataRecord(_txn->_txnBase, foundClass.id, ClassType::VERTEX);
          try {
            vertexDataRecord.getBlob(vertex.rid.second);
          } catch (const Error &error) {
            if (error.code() == NOGDB_CTX_NOEXST_RECORD) {
              throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
            } else {
              throw NOGDB_FATAL_ERROR(error);
            }
          }
          return *this;
        } else {
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_MISMATCH_CLASSTYPE);
        }
      }

    private:

      const Txn *_txn;

      inline static bool isNameValid(const std::string &name) {
        return std::regex_match(name, std::regex("^[A-Za-z_][A-Za-z0-9_]*$"));
      }

    };
  }
}
