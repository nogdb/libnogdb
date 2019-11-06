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

#include <cstdlib>
#include <map>
#include <string>
#include <sys/file.h>
#include <sys/stat.h>
#include <type_traits>
#include <unordered_map>

#include "lmdb_engine.hpp"
#include "utils.hpp"

#include "nogdb/nogdb.h"

#define DEFAULT_NOGDB_MAX_DATABASE_NUMBER 1024U
#define DEFAULT_NOGDB_MAX_DATABASE_SIZE 1073741824UL // 1GB
#define DEFAULT_NOGDB_MAX_READERS 65536U

namespace nogdb {
namespace storage_engine {
    using namespace utils::assertion;

    class LMDBEnv {
    public:
        LMDBEnv(const std::string& dbPath, unsigned int dbNum, unsigned long dbSize, unsigned int readers)
        {
            if (!utils::io::fileExists(dbPath)) {
                mkdir(dbPath.c_str(), 0755);
            }
            _env = std::move(lmdb::Env::create(dbNum, dbSize, readers).open(dbPath));
        }

        ~LMDBEnv() noexcept
        {
            try {
                close();
            } catch (...) {
            }
        }

        LMDBEnv(LMDBEnv&& other) noexcept
        {
            using std::swap;
            swap(_env, other._env);
        }

        LMDBEnv& operator=(LMDBEnv&& other) noexcept
        {
            if (this != &other) {
                using std::swap;
                swap(_env, other._env);
            }
            return *this;
        }

        void close() noexcept
        {
            _env.close();
        }

        lmdb::EnvHandler* handle() const noexcept
        {
            return _env.handle();
        }

    private:
        lmdb::Env _env { nullptr };
    };

    class LMDBTxn {
    public:
        LMDBTxn(LMDBEnv* const env, const unsigned int txnMode)
        {
            _txn = lmdb::Transaction::begin(env->handle(), txnMode);
        }

        ~LMDBTxn() noexcept
        {
            if (_txn.handle()) {
                try {
                    rollback();
                } catch (...) {
                }
                _txn = nullptr;
            }
        }

        LMDBTxn(LMDBTxn&& other) noexcept
        {
            using std::swap;
            swap(_txn, other._txn);
        }

        LMDBTxn& operator=(LMDBTxn&& other) noexcept
        {
            if (this != &other) {
                using std::swap;
                swap(_txn, other._txn);
            }
            return *this;
        }

        lmdb::DBi openDBi(const std::string& dbName, bool numericKey = false, bool unique = true) const
        {
            if (_txn.handle()) {
                return lmdb::DBi::open(_txn.handle(), dbName, numericKey, unique);
            } else {
                throw NOGDB_STORAGE_ERROR(MDB_BAD_TXN);
            }
        }

        lmdb::Cursor openCursor(const lmdb::DBi& dbi) const
        {
            require(_txn.handle() == dbi.txn());
            return lmdb::Cursor::open(_txn.handle(), dbi.handle());
        }

        lmdb::Cursor openCursor(const std::string& dbName, bool numericKey = false, bool unique = true) const
        {
            return openCursor(openDBi(dbName, numericKey, unique));
        }

        void commit()
        {
            _txn.commit();
            _txn = nullptr;
        }

        void rollback() noexcept
        {
            _txn.abort();
            _txn = nullptr;
        }

        lmdb::TransactionHandler* handle() const noexcept
        {
            return _txn.handle();
        }

    private:
        lmdb::Transaction _txn { nullptr };
    };

}

}