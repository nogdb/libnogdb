/*
 *  Copyright (C) 2019, NogDB <https://nogdb.org>
 *  <nogdb at throughwave dot co dot th>
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

#include <string>
#include <memory>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sys/file.h>
#include <sys/stat.h>

#include "utils.hpp"
#include "constant.hpp"
#include "storage_engine.hpp"
#include "validate.hpp"
#include "schema.hpp"

#include "nogdb/nogdb.h"

using namespace nogdb::utils::assertion;

namespace nogdb {

  auto Context::_underlying = std::unordered_map<std::string, LMDBInstance>{};

  Context::Context(const std::string &dbPath)
      : Context{dbPath, DEFAULT_NOGDB_MAX_DATABASE_NUMBER, DEFAULT_NOGDB_MAX_DATABASE_SIZE} {};

  Context::Context(const std::string &dbPath, unsigned int maxDbNum)
      : Context{dbPath, maxDbNum, DEFAULT_NOGDB_MAX_DATABASE_SIZE} {};

  Context::Context(const std::string &dbPath, unsigned long maxDbSize)
      : Context{dbPath, DEFAULT_NOGDB_MAX_DATABASE_NUMBER, maxDbSize} {};

  Context::Context(const std::string &dbPath, unsigned int maxDbNum, unsigned long maxDbSize)
      : _dbPath{dbPath}, _maxDB{maxDbNum}, _maxDBSize{maxDbSize} {
    auto foundContext = _underlying.find(dbPath);
    if (foundContext == _underlying.cend()) {
      if (!utils::io::fileExists(dbPath)) {
        mkdir(dbPath.c_str(), 0755);
      }
      auto instance = LMDBInstance{};
      instance._handler = new storage_engine::LMDBEnv(dbPath, maxDbNum, maxDbSize, DEFAULT_NOGDB_MAX_READERS);
      instance._refCount = 1;
      _underlying.emplace(dbPath, instance);
      _envHandler = instance._handler;
    } else {
      _envHandler = foundContext->second._handler;
      ++foundContext->second._refCount;
    }
  }

  Context::~Context() noexcept {
    auto foundContext = _underlying.find(_dbPath);
    if (foundContext != _underlying.cend()) {
      if (foundContext->second._refCount <= 1) {
        delete foundContext->second._handler;
        foundContext->second._handler = nullptr;
        _underlying.erase(_dbPath);
      } else {
        --foundContext->second._refCount;
      }
    }
    _envHandler = nullptr;
  }

  Context::Context(const Context &ctx)
      : _dbPath{ctx._dbPath},
        _maxDB{ctx._maxDB},
        _maxDBSize{ctx._maxDBSize},
        _envHandler{ctx._envHandler} {
    ++_underlying.find(_dbPath)->second._refCount;
  }

  Context &Context::operator=(const Context &ctx) {
    if (this != &ctx) {
      _dbPath = ctx._dbPath;
      _maxDB = ctx._maxDB;
      _maxDBSize = ctx._maxDBSize;
      _envHandler = ctx._envHandler;
      ++_underlying.find(_dbPath)->second._refCount;
    }
    return *this;
  }

  Context::Context(Context &&ctx) noexcept
      : _dbPath{ctx._dbPath},
      _maxDB{ctx._maxDB},
      _maxDBSize{ctx._maxDBSize},
      _envHandler{ctx._envHandler} {}

  Context &Context::operator=(Context &&ctx) noexcept {
    if (this != &ctx) {
      _envHandler = ctx._envHandler;
      _dbPath = ctx._dbPath;
      _maxDB = ctx._maxDB;
      _maxDBSize = ctx._maxDBSize;
      ctx._dbPath = std::string{};
      ctx._envHandler = nullptr;
      ctx._maxDB = 0;
      ctx._maxDBSize = 0;
    }
    return *this;
  }

  Transaction Context::beginTxn(const TxnMode &txnMode) {
    return Transaction(*this, txnMode);
  }

}
