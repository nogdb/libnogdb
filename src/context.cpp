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
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sys/file.h>
#include <sys/stat.h>

#include "utils.hpp"
#include "constant.hpp"
#include "storage_engine.hpp"
#include "graph.hpp"
#include "validate.hpp"
#include "schema.hpp"

#include "nogdb_context.h"

using namespace nogdb::utils::assertion;

namespace nogdb {

    Context::Context(const std::string &dbPath)
            : Context{dbPath, DEFAULT_NOGDB_MAX_DATABASE_NUMBER, DEFAULT_NOGDB_MAX_DATABASE_SIZE} {};

    Context::Context(const std::string &dbPath, unsigned int maxDbNum)
            : Context{dbPath, maxDbNum, DEFAULT_NOGDB_MAX_DATABASE_SIZE} {};

    Context::Context(const std::string &dbPath, unsigned long maxDbSize)
            : Context{dbPath, DEFAULT_NOGDB_MAX_DATABASE_NUMBER, maxDbSize} {};

    Context::Context(const std::string &dbPath, unsigned int maxDbNum, unsigned long maxDbSize)
            : _dbPath{dbPath}, _maxDB{maxDbNum}, _maxDBSize{maxDbSize} {
        if (!utils::io::fileExists(dbPath)) {
            mkdir(dbPath.c_str(), 0755);
        }
        _envHandler = new storage_engine::LMDBEnv(dbPath, maxDbNum, maxDbSize, DEFAULT_NOGDB_MAX_READERS);
    }

    Context::~Context() noexcept {
        if (_envHandler) {
            delete _envHandler;
            _envHandler = nullptr;
        }
    }
    Context::Context(Context &&ctx) noexcept
            : _envHandler{std::move(ctx._envHandler)} {}

    Context &Context::operator=(Context &&ctx) noexcept {
        if (this != &ctx) {
            delete _envHandler;
            _envHandler = ctx._envHandler;
            ctx._envHandler = nullptr;
        }
        return *this;
    }

}
