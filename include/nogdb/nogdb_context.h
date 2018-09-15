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
        friend struct Validate;
        friend struct Algorithm;
        friend struct Class;
        friend struct Property;
        friend struct DB;
        friend struct Vertex;
        friend struct Edge;
        friend struct Traverse;

        friend class Txn;

        Context() = default;

        ~Context() noexcept;

        Context(const std::string &dbPath);

        explicit Context(const std::string &dbPath, unsigned int maxDbNum);

        explicit Context(const std::string &dbPath, unsigned long maxDbSize);

        Context(const std::string &dbPath, unsigned int maxDbNum, unsigned long maxDbSize);

        Context(Context &&ctx) noexcept;

        Context &operator=(Context &&ctx) noexcept;

        std::string getDBPath() const { return _dbPath; }

        unsigned int getMaxDB() const { return _maxDB; }

        unsigned long getMaxDBSize() const { return _maxDBSize; }

    private:
        std::string _dbPath{};
        unsigned int _maxDB{};
        unsigned long _maxDBSize{};
        std::shared_ptr<storage_engine::LMDBEnv> _envHandler;
    };

}

#endif
