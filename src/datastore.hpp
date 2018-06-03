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

#ifndef __DATASTORE_HPP_INCLUDED_
#define __DATASTORE_HPP_INCLUDED_

#include <string>
#include <cstring>

#include "lmdb/lmdb.h"
#include "blob.hpp"
#include "keyval.hpp"

#include "nogdb_errors.h"

namespace nogdb {
    struct Datastore {
        typedef int ErrorType;
        typedef unsigned int DSFlag;
        typedef mdb_mode_t Permission;
        typedef unsigned int Txn;
        typedef MDB_env EnvHandler;
        typedef MDB_txn TxnHandler;
        typedef MDB_dbi DBHandler;
        typedef MDB_cursor CursorHandler;

        // for environment flags, see more at http://104.237.133.194/doc/group__mdb__env.html
        constexpr static unsigned int FLAG = MDB_NOTLS; //| MDB_MAPASYNC | MDB_WRITEMAP;
        constexpr static unsigned int PERMISSION = 0664;
        constexpr static unsigned int MAX_READERS = 65536;
        constexpr static unsigned int TXN_RW = 0;
        constexpr static unsigned int TXN_RO = MDB_RDONLY;

        Datastore() = delete;

        ~Datastore() noexcept = delete;

        static EnvHandler *createEnv(const std::string &dbPath,
                                     unsigned int dbNum,
                                     unsigned long dbSize,
                                     unsigned int dbMaxReaders,
                                     DSFlag flag,
                                     Permission perm);

        static void destroyEnv(EnvHandler *envHandler) noexcept;

        static DBHandler
        openDbi(TxnHandler *txnHandler, const std::string &dbName, bool isNumericKey = false, bool isUnique = true);

        static void dropDbi(TxnHandler *txnHandler, DBHandler dbHandler);

        static void emptyDbi(TxnHandler *txnHandler, DBHandler dbHandler);

        static TxnHandler *beginTxn(EnvHandler *envHandler, Txn txnType);

        static void commitTxn(TxnHandler *txnHandler);

        static void abortTxn(TxnHandler *txnHandler);

        static CursorHandler *openCursor(TxnHandler *txnHandler, DBHandler dbHandler);

        static KeyValue getNextCursor(CursorHandler *cursorHandler);

        static KeyValue getNextDupCursor(CursorHandler *cursorHandler);

        static KeyValue getPrevCursor(CursorHandler *cursorHandler);

        template<typename K>
        static KeyValue getSetKeyCursor(CursorHandler *cursorHandler, const K &key) {
            MDB_val recordKey;
            MDB_val recordValue;
            recordKey.mv_size = sizeof(K);
            recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(&key));
            auto data = KeyValue{};
            if (auto error = mdb_cursor_get(cursorHandler, &recordKey, &recordValue, MDB_SET_KEY)) {
                if (error != MDB_NOTFOUND) {
                    throw error;
                }
            } else {
                data = KeyValue{recordKey, recordValue};
            }
            return data;
        }

        static KeyValue getSetKeyCursor(CursorHandler *cursorHandler, const std::string &key);

        template<typename K>
        static KeyValue getSetRangeCursor(CursorHandler *cursorHandler, const K &key) {
            MDB_val recordKey;
            MDB_val recordValue;
            recordKey.mv_size = sizeof(K);
            recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(&key));
            auto data = KeyValue{};
            if (auto error = mdb_cursor_get(cursorHandler, &recordKey, &recordValue, MDB_SET_RANGE)) {
                if (error != MDB_NOTFOUND) {
                    throw error;
                }
            } else {
                data = KeyValue{recordKey, recordValue};
            }
            return data;
        }

        static KeyValue getSetRangeCursor(CursorHandler *cursorHandler, const std::string &key);

        static void deleteCursor(CursorHandler *cursorHandler);

        static void closeCursor(CursorHandler *cursorHandler);

        class CursorHandlerWrapper {
        public:
            CursorHandlerWrapper(TxnHandler *txnHandler, DBHandler dbHandler);

            ~CursorHandlerWrapper() noexcept;

            CursorHandler* get() {
                return cursorHandler;
            }

        private:
            mutable CursorHandler *cursorHandler;
        };

        template<typename K>
        static void putRecord(TxnHandler *txnHandler, DBHandler dbHandler, const K &key, const std::string &value,
                              bool isAppend = false, bool isOverwrite = true) {
            MDB_val recordKey;
            MDB_val recordValue;
            recordKey.mv_size = sizeof(K);
            recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(&key));
            recordValue.mv_size = value.length();
            recordValue.mv_data = const_cast<void *>(reinterpret_cast<const void *>(value.c_str()));
            auto flags = ((isAppend) ? MDB_APPEND : 0U) | ((isOverwrite) ? 0U : MDB_NOOVERWRITE);
            if (auto error = mdb_put(txnHandler, dbHandler, &recordKey, &recordValue, flags)) {
                throw error;
            }
        }

        template<typename K>
        static void
        putRecord(TxnHandler *txnHandler, DBHandler dbHandler, const K &key, const Blob &value, bool isAppend = false,
                  bool isOverwrite = true) {
            MDB_val recordKey;
            MDB_val recordValue;
            recordKey.mv_size = sizeof(K);
            recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(&key));
            recordValue.mv_size = value.size();
            recordValue.mv_data = reinterpret_cast<void *>(value.bytes());
            auto flags = ((isAppend) ? MDB_APPEND : 0U) | ((isOverwrite) ? 0U : MDB_NOOVERWRITE);
            if (auto err = mdb_put(txnHandler, dbHandler, &recordKey, &recordValue, flags)) {
                throw err;
            }
        }

        static void
        putRecord(TxnHandler *txnHandler, DBHandler dbHandler, const std::string &key, const std::string &value,
                  bool isAppend = false, bool isOverwrite = true);

        static void putRecord(TxnHandler *txnHandler, DBHandler dbHandler, const std::string &key, const Blob &value,
                              bool isAppend = false, bool isOverwrite = true);

        template<typename K, typename V>
        static void
        putRecord(TxnHandler *txnHandler, DBHandler dbHandler, const K &key, const V &value, bool isAppend = false,
                  bool isOverwrite = true) {
            MDB_val recordKey;
            MDB_val recordValue;
            recordKey.mv_size = sizeof(K);
            recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(&key));
            recordValue.mv_size = sizeof(V);
            recordValue.mv_data = const_cast<void *>(reinterpret_cast<const void *>(&value));
            auto flags = ((isAppend) ? MDB_APPEND : 0U) | ((isOverwrite) ? 0U : MDB_NOOVERWRITE);
            if (auto error = mdb_put(txnHandler, dbHandler, &recordKey, &recordValue, flags)) {
                throw error;
            }
        }

        template<typename K>
        static KeyValue getRecord(TxnHandler *txnHandler, DBHandler dbHandler, const K &key) {
            MDB_val recordKey;
            MDB_val recordValue;
            recordKey.mv_size = sizeof(K);
            recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(&key));
            auto data = KeyValue{};
            if (auto error = mdb_get(txnHandler, dbHandler, &recordKey, &recordValue)) {
                if (error != MDB_NOTFOUND) {
                    throw error;
                }
            } else {
                data = KeyValue{recordKey, recordValue};
            }
            return data;
        }

        static KeyValue getRecord(TxnHandler *txnHandler, DBHandler dbHandler, const std::string &key);

        template<typename K>
        static void deleteRecord(TxnHandler *txnHandler, DBHandler dbHandler, const K &key) {
            MDB_val recordKey;
            recordKey.mv_size = sizeof(K);
            recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(&key));
            if (auto error = mdb_del(txnHandler, dbHandler, &recordKey, NULL)) {
                if (error != MDB_NOTFOUND) {
                    throw error;
                }
            }
        }

        template<typename K>
        static void deleteRecord(TxnHandler *txnHandler, DBHandler dbHandler, const K &key, const Blob &value) {
            MDB_val recordKey;
            MDB_val recordValue;
            recordKey.mv_size = sizeof(K);
            recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(&key));
            recordValue.mv_size = value.size();
            recordValue.mv_data = static_cast<void *>(value.bytes());
            if (auto error = mdb_del(txnHandler, dbHandler, &recordKey, &recordValue)) {
                if (error != MDB_NOTFOUND) {
                    throw error;
                }
            }
        }

        static void deleteRecord(TxnHandler *txnHandler, DBHandler dbHandler, const std::string &key);

        static void
        deleteRecord(TxnHandler *txnHandler, DBHandler dbHandler, const std::string &key, const Blob &value);

        template<typename K>
        static const K *getKeyAsNumeric(const KeyValue &keyValue) noexcept {
            return reinterpret_cast<K *>(keyValue.key().mv_data);
        }

        static std::string getKeyAsString(const KeyValue &keyValue);

        template<typename V>
        static const V *getValueAsNumeric(const KeyValue &keyValue) noexcept {
            return reinterpret_cast<V *>(keyValue.value().mv_data);
        }

        static std::string getValueAsString(const KeyValue &keyValue);

        static Blob getValueAsBlob(const KeyValue &keyValue);

        static void forceFlush(EnvHandler *envHandler);
    };
}

#endif
