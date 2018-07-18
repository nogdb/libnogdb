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
#include <map>
#include <unordered_map>
#include <vector>
#include <utility>
#include <set>
#include <queue>

#include "datatype.hpp"
#include "keyval.hpp"
#include "constant.hpp"
#include "env_handler.hpp"
#include "lmdb_engine.hpp"
#include "parser.hpp"
#include "generic.hpp"
#include "schema.hpp"
#include "utils.hpp"

#include "nogdb_errors.h"

namespace nogdb {
    Result Generic::getRecordResult(Txn &txn,
                                    const ClassPropertyInfo &classPropertyInfo,
                                    const RecordDescriptor &recordDescriptor) {
        auto classDescriptor = getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::UNDEFINED);
        try {
            auto classDBHandler = LMDBInterface::openDbi(txn.txnBase->getDsTxnHandler(),
                                                     std::to_string(recordDescriptor.rid.first), true);
            auto keyValue = LMDBInterface::getRecord(txn.txnBase->getDsTxnHandler(), classDBHandler,
                                                 recordDescriptor.rid.second);
            auto className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
            auto record = Parser::parseRawDataWithBasicInfo(className, recordDescriptor.rid, keyValue, classPropertyInfo);
            record.setBasicInfo(DEPTH_PROPERTY, recordDescriptor.depth);
            return Result{recordDescriptor, record};
        } catch (LMDBInterface::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
    }

    ResultSet Generic::getRecordFromRdesc(const Txn &txn,
                                          const RecordDescriptor &recordDescriptor) {
        auto result = ResultSet{};
        auto classDescriptor = getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::UNDEFINED);
        auto classPropertyInfo = getClassMapProperty(*txn.txnBase, classDescriptor);
        try {
            auto classDBHandler = LMDBInterface::openDbi(txn.txnBase->getDsTxnHandler(),
                                                     std::to_string(recordDescriptor.rid.first), true);
            auto keyValue = LMDBInterface::getRecord(txn.txnBase->getDsTxnHandler(), classDBHandler,
                                                 recordDescriptor.rid.second);
            auto className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
            auto record = Parser::parseRawDataWithBasicInfo(className, recordDescriptor.rid, keyValue, classPropertyInfo);
            record.setBasicInfo(DEPTH_PROPERTY, recordDescriptor.depth);
            result.emplace_back(Result{recordDescriptor, record});
        } catch (LMDBInterface::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
        return result;
    }

    ResultSet
    Generic::getMultipleRecordFromRdesc(const Txn &txn, const std::vector<RecordDescriptor> &recordDescriptors) {
        auto result = ResultSet{};
        if (!recordDescriptors.empty()) {
            auto classId = recordDescriptors.cbegin()->rid.first;
            auto classDescriptor = getClassDescriptor(txn, classId, ClassType::UNDEFINED);
            auto classPropertyInfo = getClassMapProperty(*txn.txnBase, classDescriptor);
            auto className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
            try {
                auto classDBHandler = LMDBInterface::openDbi(txn.txnBase->getDsTxnHandler(), std::to_string(classId), true);
                for (const auto &recordDescriptor: recordDescriptors) {
                    auto keyValue = LMDBInterface::getRecord(txn.txnBase->getDsTxnHandler(), classDBHandler,
                                                         recordDescriptor.rid.second);
                    auto record = Parser::parseRawDataWithBasicInfo(className, recordDescriptor.rid, keyValue, classPropertyInfo);
                    result.emplace_back(Result{recordDescriptor, record});
                }
            } catch (LMDBInterface::ErrorType &err) {
                throw Error(err, Error::Type::DATASTORE);
            }
        }
        return result;
    }

    ResultSet Generic::getRecordFromClassInfo(const Txn &txn, const ClassInfo &classInfo) {
        auto result = ResultSet{};
        try {
            auto classDBHandler = LMDBInterface::openDbi(txn.txnBase->getDsTxnHandler(), std::to_string(classInfo.id), true);
            auto cursorHandler = LMDBInterface::CursorHandlerWrapper(txn.txnBase->getDsTxnHandler(), classDBHandler);
            auto keyValue = LMDBInterface::getNextCursor(cursorHandler.get());
            while (!keyValue.empty()) {
                auto key = LMDBInterface::getKeyAsNumeric<PositionId>(keyValue);
                if (*key != EM_MAXRECNUM) {
                    auto rid = RecordId{classInfo.id, *key};
                    auto record = Parser::parseRawDataWithBasicInfo(classInfo.name, rid, keyValue, classInfo.propertyInfo);
                    result.push_back(Result{RecordDescriptor{rid}, record});
                }
                keyValue = LMDBInterface::getNextCursor(cursorHandler.get());
            }
        } catch (LMDBInterface::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
        return result;
    }

    std::vector<RecordDescriptor> Generic::getRdescFromClassInfo(Txn &txn, const ClassInfo &classInfo) {
        auto result = std::vector<RecordDescriptor>{};
        try {
            auto classDBHandler = LMDBInterface::openDbi(txn.txnBase->getDsTxnHandler(), std::to_string(classInfo.id), true);
            auto cursorHandler = LMDBInterface::CursorHandlerWrapper(txn.txnBase->getDsTxnHandler(), classDBHandler);
            auto keyValue = LMDBInterface::getNextCursor(cursorHandler.get());
            while (!keyValue.empty()) {
                auto key = LMDBInterface::getKeyAsNumeric<PositionId>(keyValue);
                if (*key != EM_MAXRECNUM) {
                    result.emplace_back(RecordDescriptor{classInfo.id, *key});
                }
                keyValue = LMDBInterface::getNextCursor(cursorHandler.get());
            }
        } catch (LMDBInterface::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
        return result;
    }

    std::vector<ClassId> Generic::getEdgeClassId(const Txn &txn, const std::set<std::string> &className) {
        auto edgeClassIds = std::vector<ClassId>();
        auto edgeClassDescriptors = getMultipleClassDescriptor(txn, className, ClassType::EDGE);
        for (const auto &edgeClassDescriptor: edgeClassDescriptors) {
            edgeClassIds.push_back(edgeClassDescriptor->id);
        }
        return edgeClassIds;
    }

    ResultSet Generic::getEdgeNeighbour(const Txn &txn,
                                        const RecordDescriptor &recordDescriptor,
                                        const std::vector<ClassId> &edgeClassIds,
                                        std::vector<RecordId>
                                        (Graph::*func)(const BaseTxn &baseTxn, const RecordId &rid, const ClassId &classId)) {
        switch (checkIfRecordExist(txn, recordDescriptor)) {
            case RECORD_NOT_EXIST:
                throw Error(NOGDB_GRAPH_NOEXST_VERTEX, Error::Type::GRAPH);
            case RECORD_NOT_EXIST_IN_MEMORY:
                return ResultSet{};
            default:
                auto result = ResultSet{};
                try {
                    auto classDescriptor = Schema::ClassDescriptorPtr{};
                    auto classPropertyInfo = ClassPropertyInfo{};
                    auto classDBHandler = LMDBInterface::DBHandler{};
                    auto className = std::string{};
                    auto retrieve = [&](ResultSet &result, const RecordId &edge) {
                        if (classDescriptor == nullptr || classDescriptor->id != edge.first) {
                            classDescriptor = getClassDescriptor(txn, edge.first, ClassType::UNDEFINED);
                            classPropertyInfo = getClassMapProperty(*txn.txnBase, classDescriptor);
                            classDBHandler = LMDBInterface::openDbi(txn.txnBase->getDsTxnHandler(), std::to_string(edge.first), true);
                            className = BaseTxn::getCurrentVersion(*txn.txnBase, classDescriptor->name).first;
                        }
                        auto keyValue = LMDBInterface::getRecord(txn.txnBase->getDsTxnHandler(), classDBHandler, edge.second);
                        auto record = Parser::parseRawDataWithBasicInfo(className, edge, keyValue, classPropertyInfo);
                        result.push_back(Result{RecordDescriptor{edge}, record});
                    };
                    if (edgeClassIds.empty()) {
                        for (const auto &edge: ((*txn.txnCtx.dbRelation).*func)(*txn.txnBase, recordDescriptor.rid, 0)) {
                            retrieve(result, edge);
                        }
                    } else {
                        for (const auto &edgeId: edgeClassIds) {
                            for (const auto &edge: ((*txn.txnCtx.dbRelation).*func)(*txn.txnBase, recordDescriptor.rid, edgeId)) {
                                retrieve(result, edge);
                            }
                        }
                    }
                } catch (Graph::ErrorType &err) {
                    if (err == NOGDB_GRAPH_NOEXST_VERTEX) {
                        throw Error(NOGDB_GRAPH_UNKNOWN_ERR, Error::Type::GRAPH);
                    } else {
                        throw Error(err, Error::Type::GRAPH);
                    }
                } catch (LMDBInterface::ErrorType &err) {
                    throw Error(err, Error::Type::DATASTORE);
                }
                return result;
        }
    }

    std::vector<RecordDescriptor>
    Generic::getRdescEdgeNeighbour(const Txn &txn,
                                   const RecordDescriptor &recordDescriptor,
                                   const std::vector<ClassId> &edgeClassIds,
                                   std::vector<RecordId>
                                   (Graph::*func)(const BaseTxn &baseTxn, const RecordId &rid, const ClassId &classId)) {
        switch (checkIfRecordExist(txn, recordDescriptor)) {
            case RECORD_NOT_EXIST:
                throw Error(NOGDB_GRAPH_NOEXST_VERTEX, Error::Type::GRAPH);
            case RECORD_NOT_EXIST_IN_MEMORY:
                return std::vector<RecordDescriptor>{};
            default:
                auto result = std::vector<RecordDescriptor>{};
                try {
                    if (edgeClassIds.empty()) {
                        for (const auto &edge: ((*txn.txnCtx.dbRelation).*func)(*txn.txnBase, recordDescriptor.rid, 0)) {
                            result.emplace_back(RecordDescriptor{edge});
                        }
                    } else {
                        for (const auto &edgeId: edgeClassIds) {
                            for (const auto &edge: ((*txn.txnCtx.dbRelation).*func)(*txn.txnBase, recordDescriptor.rid, edgeId)) {
                                result.emplace_back(RecordDescriptor{edge});
                            }
                        }
                    }
                } catch (Graph::ErrorType &err) {
                    if (err == NOGDB_GRAPH_NOEXST_VERTEX) {
                        throw Error(NOGDB_GRAPH_UNKNOWN_ERR, Error::Type::GRAPH);
                    } else {
                        throw Error(err, Error::Type::GRAPH);
                    }
                } catch (LMDBInterface::ErrorType &err) {
                    throw Error(err, Error::Type::DATASTORE);
                }
                return result;
        }
    }

    uint8_t Generic::checkIfRecordExist(const Txn &txn, const RecordDescriptor &recordDescriptor) {
        if (txn.txnCtx.dbRelation->lookupVertex(*(txn.txnBase), recordDescriptor.rid)) {
            return RECORD_EXIST;
        } else {
            auto keyValue = KeyValue{};
            try {
                auto classDBHandler = LMDBInterface::openDbi(txn.txnBase->getDsTxnHandler(),
                                                         std::to_string(recordDescriptor.rid.first), true);
                keyValue = LMDBInterface::getRecord(txn.txnBase->getDsTxnHandler(), classDBHandler,
                                                recordDescriptor.rid.second);
            } catch (LMDBInterface::ErrorType &err) {
                throw Error(err, Error::Type::DATASTORE);
            }
            return (keyValue.empty()) ? RECORD_NOT_EXIST : RECORD_NOT_EXIST_IN_MEMORY;
        }
    }

    std::set<Schema::ClassDescriptorPtr>
    Generic::getClassExtend(const BaseTxn &txn, const std::set<Schema::ClassDescriptorPtr> &classDescriptors) {
        auto subClasses = classDescriptors;
        std::function<void(const Schema::ClassDescriptorPtr &classDescriptor)>
                resolveSubclass = [&txn, &subClasses, &resolveSubclass](
                const Schema::ClassDescriptorPtr &classDescriptor) -> void {
            for (const auto &subClassDescriptor: BaseTxn::getCurrentVersion(txn, classDescriptor->sub).first) {
                auto subClassDescriptorPtr = subClassDescriptor.lock();
                require(subClassDescriptorPtr != nullptr);
                subClasses.insert(subClassDescriptorPtr);
                resolveSubclass(subClassDescriptorPtr);
            }
        };
        for (const auto &classDescriptor: classDescriptors) {
            resolveSubclass(classDescriptor);
        }
        return subClasses;
    }

    const ClassPropertyInfo
    Generic::getClassMapProperty(const BaseTxn &txn, const Schema::ClassDescriptorPtr &classDescriptor) {
        auto classPropertyInfo = ClassPropertyInfo{};
        classPropertyInfo.insert(CLASS_NAME_PROPERTY_ID, CLASS_NAME_PROPERTY, PropertyType::TEXT);
        classPropertyInfo.insert(RECORD_ID_PROPERTY_ID, RECORD_ID_PROPERTY, PropertyType::TEXT);
        classPropertyInfo.insert(DEPTH_PROPERTY_ID, DEPTH_PROPERTY, PropertyType::UNSIGNED_INTEGER);
        classPropertyInfo.insert(VERSION_PROPERTY_ID, VERSION_PROPERTY, PropertyType::UNSIGNED_BIGINT); //NOTE: inserted by default
        classPropertyInfo.insert(TXN_VERSION_ID, TXN_VERSION, PropertyType::UNSIGNED_BIGINT);

        for (const auto &property: BaseTxn::getCurrentVersion(txn, classDescriptor->properties).first) {
            classPropertyInfo.insert(property.first, property.second);
        }
        std::function<void(const Schema::ClassDescriptorPtr &)>
                getInheritProperties = [&txn, &classPropertyInfo, &getInheritProperties]
                (const Schema::ClassDescriptorPtr &classDescriptorPtr) -> void {
            for (const auto &property: BaseTxn::getCurrentVersion(txn, classDescriptorPtr->properties).first) {
                classPropertyInfo.insert(property.first, property.second);
            }
            if (auto superClassDescriptor = BaseTxn::getCurrentVersion(txn, classDescriptorPtr->super).first.lock()) {
                getInheritProperties(superClassDescriptor);
            }
        };
        if (auto superClassDescriptor = BaseTxn::getCurrentVersion(txn, classDescriptor->super).first.lock()) {
            getInheritProperties(superClassDescriptor);
        }
        return classPropertyInfo;
    }

    std::set<Schema::ClassDescriptorPtr>
    Generic::getMultipleClassDescriptor(const Txn &txn, const std::vector<ClassId> &classIds, const ClassType &type) {
        auto setOfClassDescriptors = std::set<Schema::ClassDescriptorPtr>();
        if (!classIds.empty()) {
            for (const auto &classId: classIds) {
                if (classId > 0) {
                    setOfClassDescriptors.insert(getClassDescriptor(txn, classId, type));
                }
            }
            // check inheritance before return
            return getClassExtend(*txn.txnBase, setOfClassDescriptors);
        }
        return std::set<Schema::ClassDescriptorPtr>{};
    }

    std::set<Schema::ClassDescriptorPtr>
    Generic::getMultipleClassDescriptor(const Txn &txn, const std::set<std::string> &className, const ClassType &type) {
        auto setOfClassDescriptors = std::set<Schema::ClassDescriptorPtr>();
        if (!className.empty()) {
            for (const auto &name: className) {
                if (name.length() > 0) {
                    setOfClassDescriptors.insert(getClassDescriptor(txn, name, type));
                }
            }
            // check inheritance before return
            return getClassExtend(*txn.txnBase, setOfClassDescriptors);
        }
        return std::set<Schema::ClassDescriptorPtr>{};
    }

    std::vector<ClassInfo>
    Generic::getMultipleClassMapProperty(const BaseTxn &txn,
                                         const std::set<Schema::ClassDescriptorPtr> &classDescriptors) {
        auto result = std::vector<ClassInfo> {};
        if (!classDescriptors.empty()) {
            for (const auto &classDescriptor: classDescriptors) {
                result.push_back(
                        ClassInfo{
                                classDescriptor->id,
                                BaseTxn::getCurrentVersion(txn, classDescriptor->name).first,
                                getClassMapProperty(txn, classDescriptor)
                        }
                );
            }
        }
        return result;
    }

}
