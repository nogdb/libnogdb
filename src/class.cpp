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
#include "base_txn.hpp"
#include "storage_engine.hpp"
#include "lmdb_engine.hpp"
#include "generic.hpp"
#include "validate.hpp"
#include "utils.hpp"

#include "nogdb.h"

#define CLASS_ID_UPPER_LIMIT    (UINT16_MAX - 1)

using namespace nogdb::utils::assertion;

namespace nogdb {

    const ClassDescriptor Class::create(Txn &txn, const std::string &className, ClassType type) {
        // transaction validations
        Validate::isTransactionValid(txn);
        // basic validations
        Validate::isClassNameValid(className);
        Validate::isClassTypeValid(type);

        auto &dbInfo = txn.txnBase->dbInfo;
        if ((dbInfo.maxClassId >= CLASS_ID_UPPER_LIMIT)) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_LIMIT_DBSCHEMA);
        } else {
            ++dbInfo.maxClassId;
        }

        // schema validations
        Validate::isNotDuplicatedClass(txn, className);

        auto classDescriptor = std::make_shared<Schema::ClassDescriptor>(dbInfo.maxClassId, className, type);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            // create interface for .classes
            auto classDBHandler = dsTxnHandler->openDbi(TB_CLASSES, true);
            auto superClassId = ClassId{0};
            auto totalLength = sizeof(type) + sizeof(ClassId) + className.length();
            auto value = Blob(totalLength);
            value.append(&type, sizeof(type));
            value.append(&superClassId, sizeof(ClassId));
            value.append(className.c_str(), className.length());
            classDBHandler.put(dbInfo.maxClassId, value, true);
            // create interface for itself
            auto newClassDBHandler = dsTxnHandler->openDbi(std::to_string(dbInfo.maxClassId), true);
            newClassDBHandler.put(MAX_RECORD_NUM_EM, PositionId{1}, true);

            // update in-memory schema and info
            (*txn.txnCtx.dbSchema).insert(*txn.txnBase, classDescriptor);
            ++dbInfo.numClass;
        } catch (const Error &err) {
            throw err;
        } catch (...) {
            // NOTE: too risky since this may cause undefined behaviour after throwing any exceptions
            // other than errors from datastore due to failures in updating in-memory schema or database info
            std::rethrow_exception(std::current_exception());
        }
        return classDescriptor->transform(*txn.txnBase);
    }

    const ClassDescriptor Class::createExtend(Txn &txn,
                                              const std::string &className,
                                              const std::string &superClass) {
        // transaction validations
        Validate::isTransactionValid(txn);
        // basic validations
        Validate::isClassNameValid(className);
        Validate::isClassNameValid(superClass);

        auto &dbInfo = txn.txnBase->dbInfo;
        if ((dbInfo.maxClassId >= CLASS_ID_UPPER_LIMIT)) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_LIMIT_DBSCHEMA);
        } else {
            ++dbInfo.maxClassId;
        }

        // schema validations
        Validate::isNotDuplicatedClass(txn, className);

        auto superClassDescriptor = Generic::getClassDescriptor(txn, superClass, ClassType::UNDEFINED);
        auto type = superClassDescriptor->type;
        auto classDescriptor = std::make_shared<Schema::ClassDescriptor>(dbInfo.maxClassId, className, type);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            // create interface for .classes
            auto classDBHandler = dsTxnHandler->openDbi(TB_CLASSES, true);
            auto superClassId = static_cast<ClassId>(superClassDescriptor->id);
            auto totalLength = sizeof(type) + sizeof(ClassId) + className.length();
            auto value = Blob(totalLength);
            value.append(&type, sizeof(type));
            value.append(&superClassId, sizeof(ClassId));
            value.append(className.c_str(), className.length());
            classDBHandler.put(dbInfo.maxClassId, value, true);
            // create interface for itself
            auto newClassDBHandler = dsTxnHandler->openDbi(std::to_string(dbInfo.maxClassId), true);
            newClassDBHandler.put(MAX_RECORD_NUM_EM, PositionId{1}, true);

            // update in-memory schema and info
            auto superClassDescriptorPtr = txn.txnCtx.dbSchema->find(*txn.txnBase, superClassDescriptor->id);
            require(superClassDescriptorPtr != nullptr);
            classDescriptor->super.addLatestVersion(superClassDescriptorPtr);
            auto subClasses = superClassDescriptorPtr->sub.getLatestVersion().first;
            txn.txnCtx.dbSchema->insert(*txn.txnBase, classDescriptor);
            subClasses.emplace_back(classDescriptor);
            superClassDescriptorPtr->sub.addLatestVersion(subClasses);
            txn.txnBase->addUncommittedSchema(superClassDescriptorPtr);

            txn.txnCtx.dbSchema->insert(*txn.txnBase, classDescriptor);
            ++dbInfo.numClass;
        } catch (const Error &err) {
            throw err;
        } catch (...) {
            // NOTE: too risky since this may cause undefined behaviour after throwing any exceptions
            // other than errors from datastore due to failures in updating in-memory schema or database info
            std::rethrow_exception(std::current_exception());
        }
        return classDescriptor->transform(*txn.txnBase);
    }

    void Class::drop(Txn &txn, const std::string &className) {
        // transaction validations
        Validate::isTransactionValid(txn);
        // schema validations
        auto foundClass = Validate::isExistingClass(txn, className);
        if (foundClass->type != ClassType::VERTEX && foundClass->type != ClassType::EDGE) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_UNKNOWN_ERR);
        }
        // retrieve relevant properties information
        auto propertyIds = std::vector<PropertyId>{};
        for (const auto &property: foundClass->properties.getLatestVersion().first) {
            // check if all index tables associated with the column have been removed beforehand
            if (!property.second.indexInfo.empty()) {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_IN_USED_PROPERTY);
            }
            propertyIds.push_back(property.second.id);
        }

        auto rids = std::vector<RecordId>{};
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        // update data store
        try {
            // delete class schema from .classes
            auto classDBHandler = dsTxnHandler->openDbi(TB_CLASSES, true);
            classDBHandler.del(foundClass->id);

            // delete property schema from .properties
            auto propi = dsTxnHandler->openDbi(TB_PROPERTIES, true);
            for (const auto &id : propertyIds) {
                propi.del(id);
                //TODO: implement existing index deletion if needed
            }
            // delete all associated relations
            auto relationDBHandler = dsTxnHandler->openDbi(TB_RELATIONS);
            auto dbHandler = dsTxnHandler->openDbi(std::to_string(foundClass->id), true);
            auto cursorHandler = dsTxnHandler->openCursor(dbHandler);
            for(auto keyValue = cursorHandler.getNext();
                !keyValue.empty();
                keyValue = cursorHandler.getNext()) {
                auto key = keyValue.key.data.numeric<PositionId>();
                if (key != MAX_RECORD_NUM_EM) {
                    auto recordId = RecordId{foundClass->id, key};
                    if (foundClass->type == ClassType::EDGE) {
                        relationDBHandler.del(rid2str(recordId));
                    } else {
                        try {
                            for (const auto &edgeId : txn.txnCtx.dbRelation->getEdgeInOut(*txn.txnBase, recordId)) {
                                auto edgeClassDBHandler = dsTxnHandler->openDbi(std::to_string(edgeId.first), true);
                                edgeClassDBHandler.del(edgeId.second);
                                relationDBHandler.del(rid2str(edgeId));
                            }
                        } catch (const Error &err) {
                            if (err.code() != NOGDB_GRAPH_NOEXST_VERTEX) {
                                throw err;
                            }
                        }
                    }
                    rids.push_back(recordId);
                }
            }

            // drop the actual table
            dbHandler.drop(true);

            // prepare for class inheritance
            auto superClassDescriptor = foundClass->super.getLatestVersion().first.lock();
            auto superClassId = ClassId{0};
            auto const subClassDescriptors = foundClass->sub.getLatestVersion().first;

            // update a superclass of subclasses if existing
            if (superClassDescriptor != nullptr) {
                superClassId = static_cast<ClassId>(superClassDescriptor->id);
            }
            for (const auto &subClassDescriptor: subClassDescriptors) {
                auto subClassDescriptorPtr = subClassDescriptor.lock();
                require(subClassDescriptorPtr != nullptr);
                auto name = subClassDescriptorPtr->name.getLatestVersion().first;
                require(!name.empty());
                auto totalLength = sizeof(subClassDescriptorPtr->type) + sizeof(ClassId) + name.length();
                auto value = Blob(totalLength);
                value.append(&subClassDescriptorPtr->type, sizeof(subClassDescriptorPtr->type));
                value.append(&superClassId, sizeof(ClassId));
                value.append(name.c_str(), name.length());
                classDBHandler.put(subClassDescriptorPtr->id, value);
            }

            // update in-memory relations
            for (const auto &recordId: rids) {
                if (foundClass->type == ClassType::VERTEX) {
                    txn.txnCtx.dbRelation->deleteVertex(*txn.txnBase, recordId);
                } else {
                    txn.txnCtx.dbRelation->deleteEdge(*txn.txnBase, recordId);
                }
            }

            // update in-memory schema
            if (superClassDescriptor != nullptr) {
                auto subClassesOfSuperClassDescriptor = superClassDescriptor->sub.getLatestVersion().first;
                subClassesOfSuperClassDescriptor.erase(
                        std::remove_if(subClassesOfSuperClassDescriptor.begin(), subClassesOfSuperClassDescriptor.end(),
                                       [&className](const std::weak_ptr<Schema::ClassDescriptor> &ptr) {
                                           if (auto classDescriptor = ptr.lock()) {
                                               return classDescriptor->name.getLatestVersion().first == className;
                                           } else {
                                               return false;
                                           }
                                       }));
                for (const auto &subClassDescriptor: subClassDescriptors) {
                    subClassesOfSuperClassDescriptor.push_back(subClassDescriptor);
                    auto subClassDescriptorPtr = subClassDescriptor.lock();
                    require(subClassDescriptorPtr != nullptr);
                    subClassDescriptorPtr->super.addLatestVersion(superClassDescriptor);
                    txn.txnBase->addUncommittedSchema(subClassDescriptorPtr);
                }
                superClassDescriptor->sub.addLatestVersion(subClassesOfSuperClassDescriptor);
                txn.txnBase->addUncommittedSchema(superClassDescriptor);
            } else {
                for (const auto &subClassDescriptor: subClassDescriptors) {
                    auto subClassDescriptorPtr = subClassDescriptor.lock();
                    require(subClassDescriptorPtr != nullptr);
                    subClassDescriptorPtr->super.addLatestVersion(std::weak_ptr<Schema::ClassDescriptor>{});
                    txn.txnBase->addUncommittedSchema(subClassDescriptorPtr);
                }
            }
            txn.txnCtx.dbSchema->erase(*txn.txnBase, foundClass->id);

            // update in-memory database info
            auto &dbInfo = txn.txnBase->dbInfo;
            dbInfo.numProperty -= propertyIds.size();
            --dbInfo.numClass;
        } catch (const Error &err) {
            throw err;
        } catch (...) {
            // NOTE: too risky since this may cause undefined behaviour after throwing any exceptions
            // other than errors from datastore due to failures in updating in-memory schema or database info
            std::rethrow_exception(std::current_exception());
        }
    }

    void Class::alter(Txn &txn, const std::string &oldClassName, const std::string &newClassName) {
        // transaction validations
        Validate::isTransactionValid(txn);
        // basic validations
        Validate::isClassNameValid(newClassName);

        // schema validations
        auto foundClass = Validate::isExistingClass(txn, oldClassName);
        Validate::isNotDuplicatedClass(txn, newClassName);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            auto classDBHandler = dsTxnHandler->openDbi(TB_CLASSES, true);
            auto superClassId = ClassId{0};
            if (auto superClassDescriptor = foundClass->super.getLatestVersion().first.lock()) {
                superClassId = superClassDescriptor->id;
            }
            auto totalLength = sizeof(foundClass->type) + sizeof(ClassId) + newClassName.length();
            auto value = Blob(totalLength);
            value.append(&foundClass->type, sizeof(foundClass->type));
            value.append(&superClassId, sizeof(ClassId));
            value.append(newClassName.c_str(), newClassName.length());
            classDBHandler.put(foundClass->id, value);

            // update in-memory schema
            txn.txnCtx.dbSchema->replace(*txn.txnBase, foundClass, newClassName);
        } catch (const Error &err) {
            throw err;
        } catch (...) {
            // NOTE: too risky since this may cause undefined behaviour after throwing any exceptions
            // other than errors from datastore due to failures in updating in-memory schema or database info
            std::rethrow_exception(std::current_exception());
        }
    }

}

