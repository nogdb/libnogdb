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

#ifndef __NOGDB_CONTEXT_H_INCLUDED_
#define __NOGDB_CONTEXT_H_INCLUDED_

#include <map>
#include <vector>
#include <utility>
#include <mutex>
#include <memory>

#include "nogdb_types.h"

#define NOGDB_MAX_DATABASE_NUMBER        "max_database_number"
#define NOGDB_MAX_DATABASE_SIZE          "max_database_size"
#define NOGDB_MAX_DATABASE_READERS       "max_database_readers"

//******************************************************************
//*  A declaration of NogDB database context.                      *
//******************************************************************

namespace nogdb {

    class StorageEngineSettings {
    public:
        StorageEngineSettings() = default;

        void set(const std::string& settingKey, const std::string& settingValue) {
            settings[settingKey] = settingValue;
        }

        unsigned long getValueAsNumeric(const std::string& settingKey, const unsigned long defaultValue) const {
            auto iter = settings.find(settingKey);
            if (iter != settings.cend()) {
                return strtoul(iter->second.c_str(), nullptr, 10);
            } else {
                return defaultValue;
            }
        }

        std::string getValueAsString(const std::string& settingKey, const std::string& defaultValue) const {
            auto iter = settings.find(settingKey);
            if (iter != settings.cend()) {
                return iter->second;
            } else {
                return defaultValue;
            }
        }

    private:
        std::map<std::string, std::string> settings;
    };

    class Context {
    public:
        friend struct LMDBInterface;
        friend struct Validate;
        friend struct Algorithm;
        friend struct Compare;
        friend struct TxnStat;
        friend struct Generic;
        friend struct Class;
        friend struct Property;
        friend struct Db;
        friend struct Vertex;
        friend struct Edge;
        friend struct Traverse;

        friend class BaseTxn;

        friend class Txn;

        Context() = default;

        Context(const std::string &dbPath);

        explicit Context(const std::string &dbPath, unsigned int maxDbNum);

        explicit Context(const std::string &dbPath, unsigned long maxDbSize);

        Context(const std::string &dbPath, unsigned int maxDbNum, unsigned long maxDbSize);

        Context(const Context &ctx);

        Context &operator=(const Context &ctx);

        Context(Context &&ctx) noexcept;

        Context &operator=(Context &&ctx) noexcept;

        TxnId getMaxVersionId() const;

        TxnId getMaxTxnId() const;

        std::pair<TxnId, TxnId> getMinActiveTxnId() const;

    private:
        std::shared_ptr<EnvHandlerPtr> envHandler;
        std::shared_ptr<DBInfo> dbInfo;
        std::shared_ptr<Schema> dbSchema;
        std::shared_ptr<TxnStat> dbTxnStat;
        std::shared_ptr<Graph> dbRelation;

        mutable std::shared_ptr<boost::shared_mutex> dbInfoMutex;
        mutable std::shared_ptr<boost::shared_mutex> dbWriterMutex;

        void initDatabase();
    };

}

#endif
