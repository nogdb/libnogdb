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

#ifndef __STORAGE_ENGINE_HPP_INCLUDED_
#define __STORAGE_ENGINE_HPP_INCLUDED_

#include <type_traits>
#include <string>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <sys/file.h>
#include <sys/stat.h>

#include "lmdb_engine.hpp"

#define NOGDB_MAX_DATABASE_NUMBER        "max_database_number"
#define NOGDB_MAX_DATABASE_SIZE          "max_database_size"
#define NOGDB_MAX_DATABASE_READERS       "max_database_readers"

#define NOGDB_LMDB_ENGINE                0
//#define NOGDB_ROCKSDB_ENGINE             1
#define NOGDB_DEFAULT_ENGINE             NOGDB_LMDB_ENGINE

namespace nogdb {

    namespace storage_engine {

        class Settings {
        public:
            Settings() = default;

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

        class Env {
        public:

            virtual ~Env() noexcept = default;

            virtual void close() noexcept = 0;

            unsigned int engine() const noexcept {
                return _engine;
            }

        protected:

            unsigned int _engine{NOGDB_DEFAULT_ENGINE};

            inline bool fileExists(const std::string &fileName) {
                struct stat fileStat;
                return stat((char *) fileName.c_str(), &fileStat) == 0;
            }

        };

        class Txn {
        public:

            virtual ~Txn() noexcept = default;

            virtual void commit() = 0;

            virtual void rollback() = 0;
        };

        class LMDBEnv: public Env {
        public:

            LMDBEnv(const std::string& dbPath, const Settings& settings) {
                auto dbNum = static_cast<unsigned int>(settings.getValueAsNumeric(NOGDB_MAX_DATABASE_NUMBER, 1024));
                auto dbSize = settings.getValueAsNumeric(NOGDB_MAX_DATABASE_SIZE, 1073741824); // 1GB
                if (!fileExists(dbPath)) {
                    mkdir(dbPath.c_str(), 0755);
                }
                _env = std::move(lmdb::Env::create(dbNum, dbSize).open(dbPath));
                _engine = NOGDB_LMDB_ENGINE;
            }

            ~LMDBEnv() noexcept = default;

            LMDBEnv(LMDBEnv &&other) noexcept {
                using std::swap;
                swap(_env, other._env);
            }

            LMDBEnv &operator=(LMDBEnv &&other) noexcept {
                if (this != &other) {
                    using std::swap;
                    swap(_env, other._env);
                }
                return *this;
            }

            void close() noexcept override {
                _env.close();
            }

            lmdb::EnvHandler* handle() const noexcept {
                return _env.handle();
            }

        private:
            lmdb::Env _env{nullptr};
        };


        class LMDBTxn: public Txn {
        public:

            LMDBTxn(LMDBEnv* const env, const unsigned int txnMode) {
                _txn = lmdb::Txn::begin(env->handle(), txnMode);
            }

            ~LMDBTxn() noexcept = default;

            LMDBTxn(LMDBTxn &&other) noexcept {
                using std::swap;
                swap(_txn, other._txn);
            }

            LMDBTxn &operator=(LMDBTxn &&other) noexcept {
                if (this != &other) {
                    using std::swap;
                    swap(_txn, other._txn);
                }
                return *this;
            }

            lmdb::Dbi openDbi(const std::string& dbName, bool numericKey, bool unique) {
                return lmdb::Dbi::open(_txn.handle(), dbName, numericKey, unique);
            }

            lmdb::Cursor openCursor(const lmdb::Dbi& dbi) {
                return lmdb::Cursor::open(_txn.handle(), dbi.handle());
            }

            lmdb::Cursor openCursor(const std::string& dbName, bool numericKey, bool unique) {
                return openCursor(openDbi(dbName, numericKey, unique));
            }

            void commit() {
                _txn.commit();
            }

            void rollback() {
                _txn.abort();
            }

            lmdb::TxnHandler* handle() const noexcept {
                return _txn.handle();
            }

        private:
            lmdb::Txn _txn{nullptr};
        };

        //TODO: implement more storage engine interfaces when other databases are available
//        class RocksDBEnv: public Env {
//        public:
//
//        private:
//
//        };
//
//        class RocksDBTxn: public Txn {
//        public:
//
//        private:
//
//        };

    }

}

#endif