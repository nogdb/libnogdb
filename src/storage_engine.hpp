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
#include <map>
#include <sys/file.h>
#include <sys/stat.h>

#include "utils.hpp"
#include "lmdb_engine.hpp"

#include "nogdb_context.h"

namespace nogdb {

    namespace storage_engine {

        auto

        class Env {
        public:
            /**
             * Destructor
             */
            virtual ~Env() noexcept = default;

            /**
             * Close a common database environment
             */
            virtual void close() noexcept = 0;
        };

        class Txn {
        public:
            /**
             * Destructor
             */
            virtual ~Txn() noexcept = default;

            /**
             * Commit a common transaction
             */
            virtual void commit() = 0;

            /**
             * Abort a common transaction
             */
            virtual void rollback() = 0;
        };

        class LMDBEnv: public Env {
        public:
            /**
             * Constructor
             *
             * @param dbPath a full path (including name) of a database file
             * @param params given parameters for database configurations
             */
            LMDBEnv(const std::string& dbPath, const StorageEngineSettings& settings) {
                auto dbNum = static_cast<unsigned int>(settings.getValueAsNumeric(NOGDB_MAX_DATABASE_NUMBER, 1024));
                auto dbSize = settings.getValueAsNumeric(NOGDB_MAX_DATABASE_SIZE, 1073741824); // 1GB
                if (!fileExists(dbPath)) {
                    mkdir(dbPath.c_str(), 0755);
                }
                _env = lmdb::Env::create(dbNum, dbSize).open(dbPath);
            }

            /**
             * Destructor
             */
            ~LMDBEnv() noexcept {
                try { close(); } catch (...) {}
            }

            /**
             * Move constructor
             */
            LMDBEnv(LMDBEnv &&other) noexcept {
                using std::swap;
                swap(_env, other._env);
            }

            /**
             * Move assignment operator
             */
            LMDBEnv &operator=(LMDBEnv &&other) noexcept {
                if (this != &other) {
                    using std::swap;
                    swap(_env, other._env);
                }
                return *this;
            }

            /**
             * Close LMDB environment
             */
            void close() noexcept override {
                _env.close();
            }

        private:
            lmdb::Env _env{nullptr};
        };


        class LMDBTxn: public Txn {
        public:
            LMDBTxn(LMDBEnv* const env, const unsigned int txnMode) {

            }

        private:

        };


        //TODO: implement more storage engine interfaces when other databases are available

//        class InMemEnv: public Env {
//        public:
//
//        private:
//
//        };
//
//        class InMemTxn: public Txn {
//        public:
//
//        private:
//
//        };

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
