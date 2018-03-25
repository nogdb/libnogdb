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

#include <cstring>

#include "shared_lock.hpp"
#include "schema.hpp"
#include "env_handler.hpp"
#include "datastore.hpp"
#include "parser.hpp"
#include "generic.hpp"

#include "nogdb.h"

namespace nogdb {
    Record Db::getRecord(const Txn &txn, const RecordDescriptor &recordDescriptor) {
        auto classDescriptor = Generic::getClassDescriptor(txn, recordDescriptor.rid.first, ClassType::UNDEFINED);
        auto classPropertyInfo = Generic::getClassMapProperty(*txn.txnBase, classDescriptor);
        auto keyValue = KeyValue{};
        try {
            auto classDBHandler = Datastore::openDbi(txn.txnBase->getDsTxnHandler(), std::to_string(classDescriptor->id), true);
            keyValue = Datastore::getRecord(txn.txnBase->getDsTxnHandler(), classDBHandler, recordDescriptor.rid.second);
        } catch (Datastore::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
        return Parser::parseRawData(keyValue, classPropertyInfo);
    }

    const std::vector<ClassDescriptor> Db::getSchema(const Txn &txn) {
        auto result = std::vector<ClassDescriptor>{};
        for (const auto &c: txn.txnCtx.dbSchema->getNameToDescMapping(*txn.txnBase)) {
            if (auto cPtr = c.second.lock()) {
                result.push_back(cPtr->transform(*txn.txnBase));
            }
        }
        return result;
    }

    const ClassDescriptor Db::getSchema(const Txn &txn, const std::string &className) {
        auto foundClass = Validate::isExistingClass(txn, className);
        return foundClass->transform(*txn.txnBase);
    }

    const ClassDescriptor Db::getSchema(const Txn &txn, const ClassId &classId) {
        auto foundClass = Validate::isExistingClass(txn, classId);
        return foundClass->transform(*txn.txnBase);
    }

    const DBInfo Db::getDbInfo(const Txn &txn) {
        auto ctx = txn.txnCtx;
        if (txn.txnMode == Txn::Mode::READ_ONLY) {
            ReadLock<boost::shared_mutex> _(*ctx.dbInfoMutex);
            return *ctx.dbInfo;
        } else {
            return txn.txnBase->dbInfo;
        }
    }

}


