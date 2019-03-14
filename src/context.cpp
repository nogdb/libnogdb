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

#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <sys/file.h>
#include <sys/stat.h>

#include "constant.hpp"
#include "schema.hpp"
#include "storage_engine.hpp"
#include "utils.hpp"
#include "validate.hpp"

#include "nogdb/nogdb.h"

namespace nogdb {

using namespace nogdb::utils::assertion;
using namespace utils::io;

std::unordered_map<std::string, Context::LMDBInstance> Context::_underlying = std::unordered_map<std::string, Context::LMDBInstance> {};

ContextInitializer::ContextInitializer(const std::string& dbPath)
    : _dbPath { dbPath }
{
    _settings._maxDB = DEFAULT_NOGDB_MAX_DATABASE_NUMBER;
    _settings._maxDBSize = DEFAULT_NOGDB_MAX_DATABASE_SIZE;
    _settings._enableVersion = false;
}

ContextInitializer& ContextInitializer::setMaxDB(unsigned int maxDbNum) noexcept
{
    _settings._maxDB = maxDbNum;
    return *this;
}

ContextInitializer& ContextInitializer::setMaxDBSize(unsigned long maxDbSize) noexcept
{
    _settings._maxDBSize = maxDbSize;
    return *this;
}

ContextInitializer& ContextInitializer::enableVersion() noexcept
{
    _settings._enableVersion = true;
    return *this;
}

Context ContextInitializer::init()
{
    // create a database folder if not exist
    if (!fileExists(_dbPath)) {
        mkdir(_dbPath.c_str(), 0755);
        auto settingFilePath = _dbPath + DB_SETTING_NAME;
        // write database settings to disk
        writeBinaryFile(settingFilePath.c_str(), static_cast<const char*>((void*)&_settings), sizeof(_settings));
        return Context(_dbPath, _settings);
    } else {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_ALREADY_INITIALIZED);
    }
}

Context::Context(const std::string& dbPath)
    : _dbPath { dbPath }
{
    if (!fileExists(_dbPath)) {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_UNINITIALIZED);
    } else {
        auto settingFilePath = _dbPath + DB_SETTING_NAME;
        if (!fileExists(settingFilePath)) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_UNKNOWN_ERR);
        } else {
            // read database settings from disk
            auto binary = readBinaryFile(settingFilePath.c_str(), sizeof(_settings));
            memcpy(&_settings, binary, sizeof(_settings));
            delete [] binary;
            auto foundContext = _underlying.find(dbPath);
            if (foundContext == _underlying.cend()) {
                auto instance = LMDBInstance {};
                instance._handler = new storage_engine::LMDBEnv(
                    _dbPath, _settings._maxDB, _settings._maxDBSize, DEFAULT_NOGDB_MAX_READERS);
                instance._refCount = 1;
                _underlying.emplace(dbPath, instance);
                _envHandler = instance._handler;
            } else {
                _envHandler = foundContext->second._handler;
                ++foundContext->second._refCount;
            }
        }
    }
}

Context::Context(const std::string& dbPath, const ContextSetting& settings)
    : _dbPath { dbPath }
{
    auto foundContext = _underlying.find(dbPath);
    if (foundContext == _underlying.cend()) {
        auto instance = LMDBInstance {};
        instance._handler = new storage_engine::LMDBEnv(
            _dbPath, _settings._maxDB, _settings._maxDBSize, DEFAULT_NOGDB_MAX_READERS);
        instance._refCount = 1;
        _underlying.emplace(dbPath, instance);
        _envHandler = instance._handler;
    } else {
        _envHandler = foundContext->second._handler;
        ++foundContext->second._refCount;
    }
}

Context::~Context() noexcept
{
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

Context::Context(const Context& ctx)
    : _dbPath { ctx._dbPath }
    , _settings { ctx._settings }
    , _envHandler { ctx._envHandler }
{
    ++_underlying.find(_dbPath)->second._refCount;
}

Context& Context::operator=(const Context& ctx)
{
    if (this != &ctx) {
        _dbPath = ctx._dbPath;
        _settings = ctx._settings;
        _envHandler = ctx._envHandler;
        ++_underlying.find(_dbPath)->second._refCount;
    }
    return *this;
}

Context::Context(Context&& ctx) noexcept
    : _dbPath { ctx._dbPath }
    , _settings { ctx._settings }
    , _envHandler { ctx._envHandler }
{
}

Context& Context::operator=(Context&& ctx) noexcept
{
    if (this != &ctx) {
        _envHandler = ctx._envHandler;
        _dbPath = ctx._dbPath;
        _settings = ctx._settings;
        ctx._dbPath = std::string {};
        ctx._envHandler = nullptr;
        ctx._settings = ContextSetting {};
    }
    return *this;
}

Transaction Context::beginTxn(const TxnMode& txnMode)
{
    return Transaction(*this, txnMode);
}

}
