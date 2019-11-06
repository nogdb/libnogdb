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
#include <unordered_map>

#include "datarecord_adapter.hpp"
#include "schema.hpp"
#include "validate.hpp"

namespace nogdb {
namespace validate {
    using namespace adapter::schema;
    using namespace adapter::datarecord;
    using namespace schema;

    Validator& Validator::isTxnValid()
    {
        if (_txn->getTxnMode() == TxnMode::READ_ONLY) {
            throw NOGDB_TXN_ERROR(NOGDB_TXN_INVALID_MODE);
        }
        return *this;
    }

    Validator& Validator::isTxnCompleted()
    {
        if (_txn->isCompleted()) {
            throw NOGDB_TXN_ERROR(NOGDB_TXN_COMPLETED);
        }
        return *this;
    }

    Validator& Validator::isClassIdMaxReach()
    {
        if ((_txn->_adapter->dbInfo()->getMaxClassId() >= CLASS_ID_UPPER_LIMIT)) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_MAXCLASS_REACH);
        }
        return *this;
    }

    Validator& Validator::isPropertyIdMaxReach()
    {
        if ((_txn->_adapter->dbInfo()->getMaxPropertyId() >= UINT16_MAX)) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_MAXPROPERTY_REACH);
        }
        return *this;
    }

    Validator& Validator::isIndexIdMaxReach()
    {
        if ((_txn->_adapter->dbInfo()->getMaxIndexId() >= UINT32_MAX)) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_MAXINDEX_REACH);
        }
        return *this;
    }

    Validator& Validator::isClassNameValid(const std::string& className)
    {
        if (!isNameValid(className) || className.length() > MAX_CLASS_NAME_LEN) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_CLASSNAME);
        }
        return *this;
    }

    Validator& Validator::isPropertyNameValid(const std::string& propName)
    {
        if (!isNameValid(propName) || propName.length() > MAX_PROPERTY_NAME_LEN) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_PROPERTYNAME);
        }
        return *this;
    }

    Validator& Validator::isClassTypeValid(const ClassType& type)
    {
        switch (type) {
        case ClassType::VERTEX:
        case ClassType::EDGE:
            return *this;
        default:
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_CLASSTYPE);
        }
    }

    Validator& Validator::isPropertyTypeValid(const PropertyType& type)
    {
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

    Validator& Validator::isNotDuplicatedClass(const std::string& className)
    {
        auto foundClass = _txn->_adapter->dbClass()->getId(className);
        if (foundClass != ClassId {}) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_CLASS);
        }
        return *this;
    }

    Validator& Validator::isNotDuplicatedProperty(const ClassId& classId, const std::string& propertyName)
    {
        auto foundProperty = _txn->_adapter->dbProperty()->getId(classId, propertyName);
        if (foundProperty != PropertyId {}) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_PROPERTY);
        }
        auto superClassId = _txn->_adapter->dbClass()->getSuperClassId(classId);
        if (superClassId != ClassId {}) {
            isNotDuplicatedProperty(superClassId, propertyName);
        }
        return *this;
    }

    Validator& Validator::isNotOverriddenProperty(const ClassId& classId, const std::string& propertyName)
    {
        auto foundProperty = _txn->_adapter->dbProperty()->getId(classId, propertyName);
        if (foundProperty != PropertyId {}) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_OVERRIDE_PROPERTY);
        }
        for (const auto& subClassId : _txn->_adapter->dbClass()->getSubClassIds(classId)) {
            if (subClassId != ClassId {}) {
                isNotOverriddenProperty(subClassId, propertyName);
            }
        }
        return *this;
    }

    Validator& Validator::isExistingSrcVertex(const RecordDescriptor& vertex)
    {
        auto foundClass = SchemaUtils::getExistingClass(_txn, vertex.rid.first);
        if (foundClass.type == ClassType::VERTEX) {
            auto vertexDataRecord = DataRecord(_txn->_txnBase, foundClass.id, ClassType::VERTEX);
            try {
                vertexDataRecord.getBlob(vertex.rid.second);
            } catch (const Error& error) {
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

    Validator& Validator::isExistingDstVertex(const RecordDescriptor& vertex)
    {
        auto foundClass = SchemaUtils::getExistingClass(_txn, vertex.rid.first);
        if (foundClass.type == ClassType::VERTEX) {
            auto vertexDataRecord = DataRecord(_txn->_txnBase, foundClass.id, ClassType::VERTEX);
            try {
                vertexDataRecord.getBlob(vertex.rid.second);
            } catch (const Error& error) {
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

    Validator& Validator::isExistingVertex(const RecordDescriptor& vertex)
    {
        auto foundClass = SchemaUtils::getExistingClass(_txn, vertex.rid.first);
        if (foundClass.type == ClassType::VERTEX) {
            try {
                DataRecord(_txn->_txnBase, foundClass.id, ClassType::VERTEX).getBlob(vertex.rid.second);
            } catch (const Error& error) {
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

    Validator& Validator::isExistingVertices(const std::set<RecordDescriptor> &vertices)
    {
        auto foundClasses = std::unordered_map<ClassId, std::shared_ptr<DataRecord>> {};
        for(const auto& vertex: vertices) {
            auto foundClass = foundClasses.find(vertex.rid.first);
            if (foundClass != foundClasses.cend()) {
                try {
                    foundClass->second->getBlob(vertex.rid.second);
                } catch (const Error& error) {
                    if (error.code() == NOGDB_CTX_NOEXST_RECORD) {
                        throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
                    } else {
                        throw NOGDB_FATAL_ERROR(error);
                    }
                }
            } else {
                auto foundNewClass = SchemaUtils::getExistingClass(_txn, vertex.rid.first);
                if (foundNewClass.type == ClassType::VERTEX) {
                    try {
                        auto vertexDataRecord = std::make_shared<DataRecord>(
                            DataRecord(_txn->_txnBase, foundNewClass.id, ClassType::VERTEX));
                        vertexDataRecord->getBlob(vertex.rid.second);
                        foundClasses.emplace(foundNewClass.id, vertexDataRecord);
                    } catch (const Error& error) {
                        if (error.code() == NOGDB_CTX_NOEXST_RECORD) {
                            throw NOGDB_GRAPH_ERROR(NOGDB_GRAPH_NOEXST_VERTEX);
                        } else {
                            throw NOGDB_FATAL_ERROR(error);
                        }
                    }
                } else {
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_MISMATCH_CLASSTYPE);
                }
            }
        }
        return *this;
    }

}

}
