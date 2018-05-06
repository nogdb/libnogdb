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

#ifndef __GENERIC_HPP_INCLUDED_
#define __GENERIC_HPP_INCLUDED_

#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <utility>
#include <functional>

#include "schema.hpp"
#include "graph.hpp"
#include "base_txn.hpp"
#include "validate.hpp"

#include "nogdb_context.h"
#include "nogdb_txn.h"

namespace nogdb {

#define RECORD_NOT_EXIST                0
#define RECORD_NOT_EXIST_IN_MEMORY      1
#define RECORD_EXIST                    2

    struct Generic {
        Generic() = delete;

        ~Generic() noexcept = delete;

        template<typename T>
        static Schema::ClassDescriptorPtr
        getClassDescriptor(const Txn &txn, const T &className, ClassType type) {
            auto foundClass = Validate::isExistingClass(txn, className);
            if (type != ClassType::UNDEFINED) {
                if (foundClass->type != type) {
                    throw Error(CTX_MISMATCH_CLASSTYPE, Error::Type::CONTEXT);
                }
            }
            return foundClass;
        }

        static Result getRecordResult(Txn &txn,
                                      const ClassPropertyInfo &classPropertyInfo,
                                      const RecordDescriptor &recordDescriptor);

        static ResultSet getRecordFromRdesc(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static ResultSet
        getMultipleRecordFromRdesc(const Txn &txn, const std::vector<RecordDescriptor> &recordDescriptors);

        static ResultSet getRecordFromClassInfo(const Txn &txn, const ClassInfo &classInfo);

        static std::vector<RecordDescriptor> getRdescFromClassInfo(Txn &txn, const ClassInfo &classInfo);

        static ResultSet getEdgeNeighbour(const Txn &txn,
                                          const RecordDescriptor &recordDescriptor,
                                          const std::vector<ClassId> &edgeClassIds,
                                          std::vector<RecordId>
                                          (Graph::*func)(const BaseTxn &baseTxn, const RecordId &rid,
                                                         const ClassId &classId) = nullptr);

        static std::vector<RecordDescriptor>
        getRdescEdgeNeighbour(const Txn &txn,
                              const RecordDescriptor &recordDescriptor,
                              const std::vector<ClassId> &edgeClassIds,
                              std::vector<RecordId>
                              (Graph::*func)(const BaseTxn &baseTxn, const RecordId &rid,
                                             const ClassId &classId) = nullptr);

        static uint8_t checkIfRecordExist(const Txn &txn, const RecordDescriptor &recordDescriptor);

        static std::vector<ClassId> getEdgeClassId(const Txn &txn, const std::set<std::string> &className);

        static std::set<Schema::ClassDescriptorPtr>
        getClassExtend(const BaseTxn &txn, const std::set<Schema::ClassDescriptorPtr> &classDescriptors);

        static const ClassPropertyInfo
        getClassMapProperty(const BaseTxn &txn, const Schema::ClassDescriptorPtr &classDescriptor);

        static std::set<Schema::ClassDescriptorPtr>
        getMultipleClassDescriptor(const Txn &txn, const std::vector<ClassId> &classIds, const ClassType &type);

        static std::set<Schema::ClassDescriptorPtr>
        getMultipleClassDescriptor(const Txn &txn, const std::set<std::string> &className, const ClassType &type);

        static std::vector<ClassInfo>
        getMultipleClassMapProperty(const BaseTxn &txn, const std::set<Schema::ClassDescriptorPtr> &classDescriptors);

    };
}

#endif
