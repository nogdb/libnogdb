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

//******************************************************************
//*  A declaration of NogDB database context.                      *
//******************************************************************

namespace nogdb {
    class Context {
    public:
        friend struct Datastore;
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
