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

#include <vector>

#include "schema.hpp"
#include "lmdb_engine.hpp"
#include "datarecord.hpp"
#include "datarecord_adapter.hpp"
#include "parser.hpp"

#include "nogdb/nogdb.h"

namespace nogdb {
    Record DB::getRecord(const Txn &txn, const RecordDescriptor &recordDescriptor) {
        auto classInfo = txn._iSchema->getValidClassInfo(recordDescriptor.rid.first);
        return txn._iRecord->getRecord(classInfo, recordDescriptor);
    }

    const std::vector<ClassDescriptor> DB::getClasses(const Txn &txn) {
        auto result = std::vector<ClassDescriptor>{};
        for (const auto &classInfo: txn._class->getAllInfos()) {
            result.emplace_back(
                    ClassDescriptor{
                        classInfo.id,
                        classInfo.name,
                        classInfo.superClassId,
                        classInfo.type
                    });
        }
        return result;
    }

    const std::vector<PropertyDescriptor> DB::getProperties(const Txn &txn, const ClassDescriptor& classDescriptor) {
        auto result = std::vector<PropertyDescriptor>{};
        auto foundClass = txn._iSchema->getExistingClass(classDescriptor.id);
        // native properties
        for(const auto& property: txn._iSchema->getNativePropertyInfo(txn, foundClass.id)) {
            result.emplace_back(
                    PropertyDescriptor{
                       property.id,
                       property.name,
                       property.type,
                       false
                    });
        }
        // inherited properties
        for(const auto& property: txn._iSchema->getInheritPropertyInfo(txn, txn._class->getSuperClassId(foundClass.id))) {
            result.emplace_back(
                    PropertyDescriptor{
                        property.id,
                        property.name,
                        property.type,
                        true
                    });
        }
        return result;
    }

    const ClassDescriptor DB::getClass(const Txn &txn, const std::string &className) {
        auto classInfo = txn._class->getInfo(className);
        return ClassDescriptor{
            classInfo.id,
            classInfo.name,
            classInfo.superClassId,
            classInfo.type
        };
    }

    const ClassDescriptor DB::getClass(const Txn &txn, const ClassId &classId) {
        auto classInfo = txn._class->getInfo(classId);
        return ClassDescriptor{
            classInfo.id,
            classInfo.name,
            classInfo.superClassId,
            classInfo.type
        };
    }
}


