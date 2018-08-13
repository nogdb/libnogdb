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

#include "shared_lock.hpp"
#include "schema.hpp"
#include "constant.hpp"
#include "storage_engine.hpp"
#include "lmdb_engine.hpp"
#include "datarecord_adapter.hpp"
#include "generic.hpp"
#include "validate.hpp"
#include "utils.hpp"

#include "nogdb.h"
#include "parser.hpp"

using namespace nogdb::utils::assertion;

namespace nogdb {

    const ClassDescriptor Class::create(Txn &txn, const std::string &className, ClassType type) {
        Validate::isTransactionValid(txn);
        Validate::isClassIdMaxReach(txn);
        Validate::isClassNameValid(className);
        Validate::isClassTypeValid(type);
        Validate::isNotDuplicatedClass(txn, className);
        try {
            auto classId = txn._dbInfo.getMaxClassId() + ClassId{1};
            auto classProps = adapter::schema::ClassAccessInfo{className, classId, ClassId{0}, type};
            txn._class.create(classProps);
            adapter::datarecord::DataRecord(txn._txnBase, classId, type).init();
            txn._dbInfo.setMaxClassId(classId);
            txn._dbInfo.setNumClassId(txn._dbInfo.getNumClassId() + ClassId{1});
            return ClassDescriptor{classProps.id, className, type};
        } catch (const Error &err) {
            txn.rollback();
            throw err;
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

    const ClassDescriptor Class::createExtend(Txn &txn, const std::string &className, const std::string &superClass) {
        Validate::isTransactionValid(txn);
        Validate::isClassIdMaxReach(txn);
        Validate::isClassNameValid(className);
        Validate::isClassNameValid(superClass);
        Validate::isNotDuplicatedClass(txn, className);
        try {
            auto superClassInfo = txn._class.getInfo(superClass);
            auto classId = txn._dbInfo.getMaxClassId() + ClassId{1};
            auto classProps = adapter::schema::ClassAccessInfo{className, classId, superClassInfo.id, superClassInfo.type};
            txn._class.create(classProps);
            adapter::datarecord::DataRecord(txn._txnBase, classId, superClassInfo.type).init();
            txn._dbInfo.setMaxClassId(classId);
            txn._dbInfo.setNumClassId(txn._dbInfo.getNumClassId() + ClassId{1});
            return ClassDescriptor{classProps.id, className, superClassInfo.type};
        } catch (const Error &err) {
            txn.rollback();
            throw err;
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

    void Class::drop(Txn &txn, const std::string &className) {
        Validate::isTransactionValid(txn);
        auto foundClass = Validate::isExistingClass(txn, className);
        try {
            // retrieve relevant properties information
            auto propertyInfos = txn._property.getInfos(foundClass.id);
            for (const auto &property: propertyInfos) {
                // check if all index tables associated with the column have been removed beforehand
                auto foundIndex = txn._index.getInfo(foundClass.id, property.id);
                if (foundIndex.id != IndexId{0}) {
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_IN_USED_PROPERTY);
                }
            }
            auto rids = std::vector<RecordId>{};
            // delete class from schema
            txn._class.remove(className);
            // delete properties from schema
            for (const auto &property : propertyInfos) {
                txn._property.remove(property.classId, property.name);
                //TODO: implement existing index deletion if needed
            }
            // delete all associated relations
            auto table = adapter::datarecord::DataRecord(txn._txnBase, foundClass.id, foundClass.type);
            auto cursorHandler = table.getCursor();
            for(auto keyValue = cursorHandler.getNext();
                !keyValue.empty();
                keyValue = cursorHandler.getNext()) {
                auto key = keyValue.key.data.numeric<PositionId>();
                if (key == MAX_RECORD_NUM_EM) continue;
                auto recordId = RecordId{foundClass.id, key};
                auto relation = adapter::relation::RelationHelper(&txn);
                if (foundClass.type == ClassType::EDGE) {
                    auto vertices = parser::Parser::parseRawDataVertexSrcDst(keyValue.val.data.blob());
                    relation.removeRel(recordId, vertices.first, vertices.second);
                } else {
                    relation.removeRelFromVertex(recordId);
                }
            }
            // drop the actual table
            table.destroy();
            // update a superclass of subclasses if existing
            for (const auto &subClassInfo: txn._class.getSubClassInfos(foundClass.id)) {
                txn._class.update(
                        adapter::schema::ClassAccessInfo{
                            subClassInfo.name,
                            subClassInfo.id,
                            foundClass.superClassId,
                            subClassInfo.type
                        });
            }
            // update database info
            txn._dbInfo.setNumClassId(txn._dbInfo.getNumClassId() - ClassId{1});
            txn._dbInfo.setNumPropertyId(txn._dbInfo.getNumPropertyId() - PropertyId{static_cast<uint16_t>(propertyInfos.size())});
        } catch (const Error &err) {
            txn.rollback();
            throw err;
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

    void Class::alter(Txn &txn, const std::string &oldClassName, const std::string &newClassName) {
        Validate::isTransactionValid(txn);
        Validate::isClassNameValid(newClassName);
        Validate::isNotDuplicatedClass(txn, newClassName);
        auto foundClass = Validate::isExistingClass(txn, oldClassName);
        try {
            txn._class.update(
                    adapter::schema::ClassAccessInfo{
                        newClassName,
                        foundClass.id,
                        foundClass.superClassId,
                        foundClass.type
                    });
        } catch (const Error &err) {
            txn.rollback();
            throw err;
        } catch (...) {
            txn.rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

}

