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

#pragma once

#include "../../../src/storage_engine.hpp"
#include "../../../src/utils.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class LMDBCommonOperations : public ::testing::Test {
public:
    LMDBCommonOperations(const std::string& dbName)
        : _dbName { dbName }
    {
    }

    virtual ~LMDBCommonOperations() noexcept = default;

protected:
    nogdb::storage_engine::LMDBEnv* env = nullptr;
    nogdb::storage_engine::LMDBTxn* txn = nullptr;

    virtual void SetUp() { beforeAll(); }

    virtual void TearDown() { afterAll(); }

    virtual void beforeEach() { txn = new nogdb::storage_engine::LMDBTxn(env, 0); }

    virtual void afterEach()
    {
        delete txn;
        txn = nullptr;
    }

private:
    std::string _dbName {};

    inline void beforeAll()
    {
        env = new nogdb::storage_engine::LMDBEnv(_dbName.c_str(), DEFAULT_NOGDB_MAX_DATABASE_NUMBER,
            DEFAULT_NOGDB_MAX_DATABASE_SIZE, DEFAULT_NOGDB_MAX_READERS);
    }

    inline void afterAll()
    {
        delete env;
        env = nullptr;
        const std::string command = "rm -rf " + _dbName;
        system(command.c_str());
    }
};

class LMDBBasicOperations : public LMDBCommonOperations {
protected:
    LMDBBasicOperations()
        : LMDBCommonOperations { "./test_basic_operations.db" }
    {
    }

    virtual ~LMDBBasicOperations() noexcept = default;
};

class LMDBCursorOperations : public LMDBCommonOperations {
protected:
    LMDBCursorOperations()
        : LMDBCommonOperations { "./test_cursor_operations.db" }
    {
    }

    virtual ~LMDBCursorOperations() noexcept = default;
};
