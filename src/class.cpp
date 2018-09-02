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

    const ClassDescriptor Class::create(const Txn &txn, const std::string &className, ClassType type) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid()
        . isClassIdMaxReach()
        . isClassNameValid(className)
        . isClassTypeValid(type)
        . isNotDuplicatedClass(className);

        try {
            auto classId = txn._dbInfo->getMaxClassId() + ClassId{1};
            auto classProps = ClassAccessInfo{className, classId, ClassId{0}, type};
            txn._class->create(classProps);
            DataRecord(txn._txnBase, classId, type).init();
            txn._dbInfo->setMaxClassId(classId);
            txn._dbInfo->setNumClassId(txn._dbInfo->getNumClassId() + ClassId{1});
            return ClassDescriptor{classProps.id, className, ClassId{0}, type};
        } catch (const Error &err) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(err);
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

    const ClassDescriptor Class::createExtend(const Txn &txn, const std::string &className, const std::string &superClass) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid()
        . isClassIdMaxReach()
        . isClassNameValid(className)
        . isClassNameValid(superClass)
        . isNotDuplicatedClass(className);

        auto superClassInfo = txn._iSchema->getExistingClass(superClass);
        try {
            auto classId = txn._dbInfo->getMaxClassId() + ClassId{1};
            auto classProps = ClassAccessInfo{className, classId, superClassInfo.id, superClassInfo.type};
            txn._class->create(classProps);
            DataRecord(txn._txnBase, classId, superClassInfo.type).init();
            txn._dbInfo->setMaxClassId(classId);
            txn._dbInfo->setNumClassId(txn._dbInfo->getNumClassId() + ClassId{1});
            return ClassDescriptor{classProps.id, className, superClassInfo.id, superClassInfo.type};
        } catch (const Error &err) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(err);
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

    void Class::drop(const Txn &txn, const std::string &className) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid();

        auto foundClass = txn._iSchema->getExistingClass(className);
        // retrieve relevant properties information
        auto propertyInfos = txn._property->getInfos(foundClass.id);
        for (const auto &property: propertyInfos) {
            // check if all index tables associated with the column have been removed beforehand
            auto foundIndex = txn._index->getInfo(foundClass.id, property.id);
            if (foundIndex.id != IndexId{0}) {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_IN_USED_PROPERTY);
            }
        }
        try {
            auto rids = std::vector<RecordId>{};
            // delete class from schema
            txn._class->remove(className);
            // delete properties from schema
            for (const auto &property : propertyInfos) {
                txn._property->remove(property.classId, property.name);
                //TODO: implement existing index deletion if needed
            }
            // delete all associated relations
            auto table = DataRecord(txn._txnBase, foundClass.id, foundClass.type);
            auto cursorHandler = table.getCursor();
            for(auto keyValue = cursorHandler.getNext();
                !keyValue.empty();
                keyValue = cursorHandler.getNext()) {
                auto key = keyValue.key.data.numeric<PositionId>();
                if (key == MAX_RECORD_NUM_EM) continue;
                auto recordId = RecordId{foundClass.id, key};
                if (foundClass.type == ClassType::EDGE) {
                    auto vertices = parser::Parser::parseEdgeRawDataVertexSrcDst(keyValue.val.data.blob());
                    txn._iGraph->removeRel(recordId, vertices.first, vertices.second);
                } else {
                    txn._iGraph->removeRelFromVertex(recordId);
                }
            }
            // drop the actual table
            table.destroy();
            // update a superclass of subclasses if existing
            for (const auto &subClassInfo: txn._class->getSubClassInfos(foundClass.id)) {
                txn._class->update(
                        ClassAccessInfo{
                            subClassInfo.name,
                            subClassInfo.id,
                            foundClass.superClassId,
                            subClassInfo.type
                        });
            }
            // update database info
            txn._dbInfo->setNumClassId(txn._dbInfo->getNumClassId() - ClassId{1});
            txn._dbInfo->setNumPropertyId(txn._dbInfo->getNumPropertyId() - PropertyId{static_cast<uint16_t>(propertyInfos.size())});
        } catch (const Error &err) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(err);
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

    void Class::alter(const Txn &txn, const std::string &oldClassName, const std::string &newClassName) {
        BEGIN_VALIDATION(&txn)
        . isTransactionValid()
        . isClassNameValid(newClassName)
        . isNotDuplicatedClass(newClassName);

        auto foundClass = txn._iSchema->getExistingClass(oldClassName);
        try {
            txn._class->alterClassName(oldClassName, newClassName);
        } catch (const Error &err) {
            txn.rollback();
            throw NOGDB_FATAL_ERROR(err);
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

}

