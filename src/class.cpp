/*
 *  Copyright (C) 2018, Throughwave (Thailand) Co., Ltd.
 *  <peerawich at throughwave dot co dot th>
 *
 *  This file is part of libnogdb, the NogDB core library in C++.
 *
 *  libnogdb is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream> // for debugging
#include <memory>
#include <cassert>

#include "shared_lock.hpp"
#include "schema.hpp"
#include "config.hpp"
#include "base_txn.hpp"
#include "env_handler.hpp"
#include "datastore.hpp"
#include "generic.hpp"
#include "validate.hpp"

#include "nogdb.h"

#define CLASS_ID_UPPER_LIMIT    (UINT16_MAX - 1)

namespace nogdb {

    const ClassDescriptor Class::create(Txn &txn,
                                        const std::string &className,
                                        ClassType type,
                                        const PropertyMapType &properties) {
        // transaction validations
        Validate::isTransactionValid(txn);
        // basic validations
        Validate::isNotEmptyClassname(className);
        Validate::isClassTypeValid(type);
        for (const auto &property : properties) {
            Validate::isNotEmptyPropname(property.first);
            Validate::isPropertyTypeValid(property.second);
        }

        auto &dbInfo = txn.txnBase->dbInfo;
        if ((dbInfo.maxClassId >= CLASS_ID_UPPER_LIMIT) && (dbInfo.maxPropertyId + properties.size() > UINT16_MAX)) {
            throw Error(CTX_LIMIT_DBSCHEMA, Error::Type::CONTEXT);
        } else {
            ++dbInfo.maxClassId;
        }

        // schema validations
        Validate::isNotDuplicatedClass(txn, className);

        auto classDescriptor = std::make_shared<Schema::ClassDescriptor>(dbInfo.maxClassId, className, type);
        auto beginMaxPropertyId = dbInfo.maxPropertyId;
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            // create interface for .classes
            auto classDBHandler = Datastore::openDbi(dsTxnHandler, TB_CLASSES, true);
            auto superClassId = ClassId{0};
            auto totalLength = sizeof(type) + sizeof(ClassId) + strlen(className.c_str());
            auto value = Blob(totalLength);
            value.append(&type, sizeof(type));
            value.append(&superClassId, sizeof(ClassId));
            value.append(className.c_str(), strlen(className.c_str()));
            Datastore::putRecord(dsTxnHandler, classDBHandler, dbInfo.maxClassId, value, true);
            // create interface for itself
            auto newClassDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(dbInfo.maxClassId), true);
            Datastore::putRecord(dsTxnHandler, newClassDBHandler, EM_MAXRECNUM, PositionId{1}, true);
            if (!properties.empty()) {
                auto propDBHandler = Datastore::openDbi(dsTxnHandler, TB_PROPERTIES, true);
                auto classId = dbInfo.maxClassId;
                for (const auto &property: properties) {
                    auto totalLength = sizeof(property.second) + sizeof(ClassId) + strlen(property.first.c_str());
                    auto value = Blob(totalLength);
                    value.append(&(property.second), sizeof(property.second));
                    value.append(&classId, sizeof(ClassId));
                    value.append(property.first.c_str(), strlen(property.first.c_str()));
                    Datastore::putRecord(dsTxnHandler, propDBHandler, ++beginMaxPropertyId, value, true);
                }
            }

            // update in-memory schema and info
            auto classProperties = Schema::ClassProperty{};
            for (const auto &property: properties) {
                classProperties.emplace(property.first,
                                        Schema::PropertyDescriptor{++dbInfo.maxPropertyId, property.second});
                ++dbInfo.numProperty;
            }
            classDescriptor->properties.addLatestVersion(classProperties);
            (*txn.txnCtx.dbSchema).insert(*txn.txnBase, classDescriptor);
            ++dbInfo.numClass;
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        } catch (...) {
            // NOTE: too risky since this may cause undefined behaviour after throwing any exceptions
            // other than errors from datastore due to failures in updating in-memory schema or database info
            std::rethrow_exception(std::current_exception());
        }
        return classDescriptor->transform(*txn.txnBase);
    }

    const ClassDescriptor Class::createExtend(Txn &txn,
                                              const std::string &className,
                                              const std::string &superClass,
                                              const std::map<std::string, PropertyType> &properties) {
        // transaction validations
        Validate::isTransactionValid(txn);
        // basic validations
        Validate::isNotEmptyClassname(className);
        Validate::isNotEmptyClassname(superClass);
        for (const auto &property: properties) {
            Validate::isNotEmptyPropname(property.first);
            Validate::isPropertyTypeValid(property.second);
        }

        auto &dbInfo = txn.txnBase->dbInfo;
        if ((dbInfo.maxClassId >= CLASS_ID_UPPER_LIMIT) && (dbInfo.maxPropertyId + properties.size() > UINT16_MAX)) {
            throw Error(CTX_LIMIT_DBSCHEMA, Error::Type::CONTEXT);
        } else {
            ++dbInfo.maxClassId;
        }

        // schema validations
        Validate::isNotDuplicatedClass(txn, className);
        auto superClassDescriptor = Generic::getClassDescriptor(txn, superClass, ClassType::UNDEFINED);
        auto type = superClassDescriptor->type;
        auto superClassInfo = Generic::getClassMapProperty(*txn.txnBase, superClassDescriptor);
        for (const auto &property: properties) {
            auto foundInfo = superClassInfo.nameToDesc.find(property.first);
            if (foundInfo != superClassInfo.nameToDesc.cend()) {
                throw Error(CTX_DUPLICATE_PROPERTY, Error::Type::CONTEXT);
            }
        }

        auto classDescriptor = std::make_shared<Schema::ClassDescriptor>(dbInfo.maxClassId, className, type);
        auto beginMaxPropertyId = dbInfo.maxPropertyId;
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            // create interface for .classes
            auto classDBHandler = Datastore::openDbi(dsTxnHandler, TB_CLASSES, true);
            auto superClassId = static_cast<ClassId>(superClassDescriptor->id);
            auto totalLength = sizeof(type) + sizeof(ClassId) + strlen(className.c_str());
            auto value = Blob(totalLength);
            value.append(&type, sizeof(type));
            value.append(&superClassId, sizeof(ClassId));
            value.append(className.c_str(), strlen(className.c_str()));
            Datastore::putRecord(dsTxnHandler, classDBHandler, dbInfo.maxClassId, value, true);
            // create interface for itself
            auto newClassDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(dbInfo.maxClassId), true);
            Datastore::putRecord(dsTxnHandler, newClassDBHandler, EM_MAXRECNUM, PositionId{1}, true);
            if (!properties.empty()) {
                auto propDBHandler = Datastore::openDbi(dsTxnHandler, TB_PROPERTIES, true);
                auto classId = dbInfo.maxClassId;
                for (const auto &property: properties) {
                    auto totalLength = sizeof(property.second) + sizeof(ClassId) + strlen(property.first.c_str());
                    auto value = Blob(totalLength);
                    value.append(&(property.second), sizeof(property.second));
                    value.append(&classId, sizeof(ClassId));
                    value.append(property.first.c_str(), strlen(property.first.c_str()));
                    Datastore::putRecord(dsTxnHandler, propDBHandler, ++beginMaxPropertyId, value, true);
                }
            }

            // update in-memory schema and info
            auto superClassDescriptorPtr = txn.txnCtx.dbSchema->find(*txn.txnBase, superClassDescriptor->id);
            assert(superClassDescriptorPtr != nullptr);
            classDescriptor->super.addLatestVersion(superClassDescriptorPtr);
            auto subClasses = superClassDescriptorPtr->sub.getLatestVersion().first;
            txn.txnCtx.dbSchema->insert(*txn.txnBase, classDescriptor);
            subClasses.emplace_back(classDescriptor);
            superClassDescriptorPtr->sub.addLatestVersion(subClasses);
            txn.txnBase->addUncommittedSchema(superClassDescriptorPtr);

            auto classProperties = Schema::ClassProperty{};
            for (const auto &property: properties) {
                classProperties.emplace(property.first,
                                        Schema::PropertyDescriptor{++dbInfo.maxPropertyId, property.second});
                ++dbInfo.numProperty;
            }
            classDescriptor->properties.addLatestVersion(classProperties);
            txn.txnCtx.dbSchema->insert(*txn.txnBase, classDescriptor);
            ++dbInfo.numClass;
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
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
            throw Error(CTX_UNKNOWN_ERR, Error::Type::CONTEXT);
        }
        // retrieve relevant properties information
        auto propertyIds = std::vector<PropertyId>{};
        for (const auto &property: foundClass->properties.getLatestVersion().first) {
            // check if all index tables associated with the column have been removed beforehand
            if (!property.second.indexInfo.empty()) {
                throw Error(CTX_IN_USED_PROPERTY, Error::Type::CONTEXT);
            }
            propertyIds.push_back(property.second.id);
        }

        auto rids = std::vector<RecordId>{};
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        // update data store
        try {
            // delete class schema from .classes
            auto classDBHandler = Datastore::openDbi(dsTxnHandler, TB_CLASSES, true);
            Datastore::deleteRecord(dsTxnHandler, classDBHandler, foundClass->id);

            // delete property schema from .properties
            auto propi = Datastore::openDbi(dsTxnHandler, TB_PROPERTIES, true);
            for (const auto &id : propertyIds) {
                Datastore::deleteRecord(dsTxnHandler, propi, id);
                //TODO: implement existing index deletion if needed
            }
            // delete all associated relations
            auto relationDBHandler = Datastore::openDbi(dsTxnHandler, TB_RELATIONS);
            auto dbHandler = Datastore::openDbi(dsTxnHandler, std::to_string(foundClass->id), true);
            auto cursor = Datastore::openCursor(dsTxnHandler, dbHandler);
            auto keyValue = Datastore::getNextCursor(cursor);
            while (!keyValue.empty()) {
                auto key = Datastore::getKeyAsNumeric<PositionId>(keyValue);
                if (*key != EM_MAXRECNUM) {
                    auto recordId = RecordId{foundClass->id, *key};
                    if (foundClass->type == ClassType::EDGE) {
                        Datastore::deleteRecord(dsTxnHandler, relationDBHandler, rid2str(recordId));
                    } else {
                        try {
                            for (const auto &edgeId : txn.txnCtx.dbRelation->getEdgeInOut(*txn.txnBase, recordId)) {
                                auto edgeClassDBHandler = Datastore::openDbi(dsTxnHandler, std::to_string(edgeId.first), true);
                                Datastore::deleteRecord(dsTxnHandler, edgeClassDBHandler, edgeId.second);
                                Datastore::deleteRecord(dsTxnHandler, relationDBHandler, rid2str(edgeId));
                            }
                        } catch (Graph::ErrorType &err) {
                            if (err != GRAPH_NOEXST_VERTEX) {
                                throw Error(err, Error::Type::GRAPH);
                            }
                        }
                    }
                    rids.push_back(recordId);
                }
                keyValue = Datastore::getNextCursor(cursor);
            }
            Datastore::closeCursor(cursor);

            // drop the actual table
            Datastore::dropDbi(dsTxnHandler, dbHandler);

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
                assert(subClassDescriptorPtr != nullptr);
                auto name = subClassDescriptorPtr->name.getLatestVersion().first;
                assert(!name.empty());
                auto totalLength = sizeof(subClassDescriptorPtr->type) + sizeof(ClassId) + strlen(name.c_str());
                auto value = Blob(totalLength);
                value.append(&subClassDescriptorPtr->type, sizeof(subClassDescriptorPtr->type));
                value.append(&superClassId, sizeof(ClassId));
                value.append(name.c_str(), strlen(name.c_str()));
                Datastore::putRecord(dsTxnHandler, classDBHandler, subClassDescriptorPtr->id, value);
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
                    assert(subClassDescriptorPtr != nullptr);
                    subClassDescriptorPtr->super.addLatestVersion(superClassDescriptor);
                    txn.txnBase->addUncommittedSchema(subClassDescriptorPtr);
                }
                superClassDescriptor->sub.addLatestVersion(subClassesOfSuperClassDescriptor);
                txn.txnBase->addUncommittedSchema(superClassDescriptor);
            } else {
                for (const auto &subClassDescriptor: subClassDescriptors) {
                    auto subClassDescriptorPtr = subClassDescriptor.lock();
                    assert(subClassDescriptorPtr != nullptr);
                    subClassDescriptorPtr->super.addLatestVersion(std::weak_ptr<Schema::ClassDescriptor>{});
                    txn.txnBase->addUncommittedSchema(subClassDescriptorPtr);
                }
            }
            txn.txnCtx.dbSchema->erase(*txn.txnBase, foundClass->id);

            // update in-memory database info
            auto &dbInfo = txn.txnBase->dbInfo;
            dbInfo.numProperty -= propertyIds.size();
            --dbInfo.numClass;
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        } catch (Graph::ErrorType &err) {
            throw Error(err, Error::Type::GRAPH);
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
        Validate::isNotEmptyClassname(newClassName);

        // schema validations
        auto foundClass = Validate::isExistingClass(txn, oldClassName);
        Validate::isNotDuplicatedClass(txn, newClassName);
        auto dsTxnHandler = txn.txnBase->getDsTxnHandler();
        try {
            auto classDBHandler = Datastore::openDbi(dsTxnHandler, TB_CLASSES, true);
            auto superClassId = ClassId{0};
            if (auto superClassDescriptor = foundClass->super.getLatestVersion().first.lock()) {
                superClassId = superClassDescriptor->id;
            }
            auto totalLength = sizeof(foundClass->type) + sizeof(ClassId) + strlen(newClassName.c_str());
            auto value = Blob(totalLength);
            value.append(&foundClass->type, sizeof(foundClass->type));
            value.append(&superClassId, sizeof(ClassId));
            value.append(newClassName.c_str(), strlen(newClassName.c_str()));
            Datastore::putRecord(dsTxnHandler, classDBHandler, foundClass->id, value);

            // update in-memory schema
            txn.txnCtx.dbSchema->replace(*txn.txnBase, foundClass, newClassName);
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        } catch (...) {
            // NOTE: too risky since this may cause undefined behaviour after throwing any exceptions
            // other than errors from datastore due to failures in updating in-memory schema or database info
            std::rethrow_exception(std::current_exception());
        }
    }

}

