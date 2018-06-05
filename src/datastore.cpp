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

#include <string>

#include "datastore.hpp"

namespace nogdb {

    Datastore::EnvHandler *
    Datastore::createEnv(const std::string &dbPath, unsigned int dbNum, unsigned long dbSize, unsigned int dbReaders,
                         DSFlag flag, Permission perm) {
        EnvHandler *envHandler = nullptr;
        auto error = ErrorType{0};
        if ((error = mdb_env_create(&envHandler)) != 0) {
            throw error;
        }
        if ((error = mdb_env_set_mapsize(envHandler, dbSize)) != 0) {
            throw error;
        }
        if ((error = mdb_env_set_maxreaders(envHandler, dbReaders)) != 0) {
            throw error;
        }
        if (dbNum != 0) {
            if ((error = mdb_env_set_maxdbs(envHandler, dbNum)) != 0) {
                throw error;
            }
        }
        if ((error = mdb_env_open(envHandler, dbPath.c_str(), flag, perm)) != 0) {
            mdb_env_close(envHandler);
            throw error;
        }
        return envHandler;
    }

    void Datastore::destroyEnv(EnvHandler *envHandler) noexcept {
        mdb_env_close(envHandler);
    }

    Datastore::DBHandler
    Datastore::openDbi(TxnHandler *txnHandler, const std::string &dbName, bool isNumericKey, bool isUnique) {
        auto dbHandler = DBHandler{0};
        auto flags = ((isNumericKey) ? MDB_INTEGERKEY : 0U) | ((!isUnique) ? MDB_DUPSORT : 0U);
        if (auto error = mdb_open(txnHandler, dbName.c_str(), MDB_CREATE | flags, &dbHandler)) {
            throw error;
        } else {
            return dbHandler;
        }
    }

    void Datastore::dropDbi(TxnHandler *txnHandler, DBHandler dbHandler) {
        if (auto error = mdb_drop(txnHandler, dbHandler, 1)) {
            throw error;
        }
    }

    void Datastore::emptyDbi(TxnHandler *txnHandler, DBHandler dbHandler) {
        if (auto error = mdb_drop(txnHandler, dbHandler, 0)) {
            throw error;
        }
    }

    Datastore::TxnHandler *Datastore::beginTxn(EnvHandler *envHandler, Txn flag) {
        TxnHandler *txnHandler = nullptr;
        if (auto error = mdb_txn_begin(envHandler, 0, flag, &txnHandler)) {
            throw error;
        } else {
            return txnHandler;
        }
    }

    void Datastore::commitTxn(TxnHandler *txnHandler) {
        if (auto error = mdb_txn_commit(txnHandler)) {
            throw error;
        }
    }

    void Datastore::abortTxn(TxnHandler *txnHandler) {
        mdb_txn_abort(txnHandler);
    }

    void Datastore::putRecord(TxnHandler *txnHandler, DBHandler dbHandler, const std::string &key,
                              const std::string &value, bool isAppend, bool isOverwrite) {
        MDB_val recordKey;
        MDB_val recordValue;
        recordKey.mv_size = key.length();
        recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(key.c_str()));
        recordValue.mv_size = value.length();
        recordValue.mv_data = const_cast<void *>(reinterpret_cast<const void *>(value.c_str()));
        auto flags = ((isAppend) ? MDB_APPEND : 0U) | ((isOverwrite) ? 0U : MDB_NOOVERWRITE);
        if (auto error = mdb_put(txnHandler, dbHandler, &recordKey, &recordValue, flags)) {
            throw error;
        }
    }

    void Datastore::putRecord(TxnHandler *txnHandler, DBHandler dbHandler, const std::string &key, const Blob &value,
                              bool isAppend, bool isOverwrite) {
        MDB_val recordKey;
        MDB_val recordValue;
        recordKey.mv_size = key.length();
        recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(key.c_str()));
        recordValue.mv_size = value.size();
        recordValue.mv_data = static_cast<void *>(value.bytes());
        auto flags = ((isAppend) ? MDB_APPEND : 0U) | ((isOverwrite) ? 0U : MDB_NOOVERWRITE);
        if (auto error = mdb_put(txnHandler, dbHandler, &recordKey, &recordValue, flags)) {
            throw error;
        }
    }

    KeyValue Datastore::getRecord(TxnHandler *txnHandler, DBHandler dbHandler, const std::string &key) {
        MDB_val recordKey;
        MDB_val recordValue;
        recordKey.mv_size = key.length();
        recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(key.c_str()));
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

    void Datastore::deleteRecord(TxnHandler *txnHandler, DBHandler dbHandler, const std::string &key) {
        MDB_val recordKey;
        recordKey.mv_size = strlen(const_cast<char *>(key.c_str()));
        recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(key.c_str()));
        if (auto error = mdb_del(txnHandler, dbHandler, &recordKey, NULL)) {
            if (error != MDB_NOTFOUND) {
                throw error;
            }
        }
    }

    void Datastore::deleteRecord(TxnHandler *txnHandler, DBHandler dbHandler, const std::string &key, const Blob &value) {
        MDB_val recordKey;
        MDB_val recordValue;
        recordKey.mv_size = strlen(const_cast<char *>(key.c_str()));
        recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(key.c_str()));
        recordValue.mv_size = value.size();
        recordValue.mv_data = static_cast<void *>(value.bytes());
        if (auto error = mdb_del(txnHandler, dbHandler, &recordKey, &recordValue)) {
            if (error != MDB_NOTFOUND) {
                throw error;
            }
        }
    }

    Datastore::CursorHandler *Datastore::openCursor(TxnHandler *txnHandler, DBHandler dbHandler) {
        CursorHandler *cursorHandler = nullptr;
        if (auto error = mdb_cursor_open(txnHandler, dbHandler, &cursorHandler)) {
            throw error;
        }
        return cursorHandler;
    }

    KeyValue Datastore::getNextCursor(CursorHandler *cursorHandler) {
        MDB_val recordKey;
        MDB_val recordValue;
        auto data = KeyValue{};
        if (auto error = mdb_cursor_get(cursorHandler, &recordKey, &recordValue, MDB_NEXT)) {
            if (error != MDB_NOTFOUND) {
                throw error;
            }
        } else {
            data = KeyValue{recordKey, recordValue};
        }
        return data;
    }

    KeyValue Datastore::getNextDupCursor(CursorHandler *cursorHandler) {
        MDB_val recordKey;
        MDB_val recordValue;
        auto data = KeyValue{};
        if (auto error = mdb_cursor_get(cursorHandler, &recordKey, &recordValue, MDB_NEXT_DUP)) {
            if (error != MDB_NOTFOUND) {
                throw error;
            }
        } else {
            data = KeyValue{recordKey, recordValue};
        }
        return data;
    }

    KeyValue Datastore::getPrevCursor(CursorHandler *cursorHandler) {
        MDB_val recordKey;
        MDB_val recordValue;
        auto data = KeyValue{};
        if (auto error = mdb_cursor_get(cursorHandler, &recordKey, &recordValue, MDB_PREV)) {
            if (error != MDB_NOTFOUND) {
                throw error;
            }
        } else {
            data = KeyValue{recordKey, recordValue};
        }
        return data;
    }

    KeyValue Datastore::getSetKeyCursor(CursorHandler *cursorHandler, const std::string &key) {
        MDB_val recordKey;
        MDB_val recordValue;
        recordKey.mv_size = strlen(const_cast<char *>(key.c_str()));
        recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(key.c_str()));
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

    KeyValue Datastore::getSetRangeCursor(CursorHandler *cursorHandler, const std::string &key) {
        MDB_val recordKey;
        MDB_val recordValue;
        recordKey.mv_size = strlen(const_cast<char *>(key.c_str()));
        recordKey.mv_data = const_cast<void *>(reinterpret_cast<const void *>(key.c_str()));
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

    void Datastore::deleteCursor(CursorHandler *cursorHandler) {
        if (auto error = mdb_cursor_del(cursorHandler, 0)) {
            throw error;
        }
    }

    void Datastore::closeCursor(CursorHandler *cursorHandler) {
        mdb_cursor_close(cursorHandler);
    }

    Datastore::CursorHandlerWrapper::CursorHandlerWrapper(TxnHandler *txnHandler, DBHandler dbHandler) {
        cursorHandler = openCursor(txnHandler, dbHandler);
    }

    Datastore::CursorHandlerWrapper::~CursorHandlerWrapper() noexcept {
        closeCursor(cursorHandler);
    }

    std::string Datastore::getKeyAsString(const KeyValue &data) {
        return std::string(static_cast<char *>(data.key().mv_data), data.key().mv_size);
    }

    std::string Datastore::getValueAsString(const KeyValue &data) {
        return std::string(static_cast<char *>(data.value().mv_data), data.value().mv_size);
    }

    Blob Datastore::getValueAsBlob(const KeyValue &data) {
        return Blob(static_cast<Blob::Byte *>(data.value().mv_data), data.value().mv_size);
    }

    void Datastore::forceFlush(EnvHandler *envHandler) {
        if (auto error = mdb_env_sync(envHandler, 0)) {
            throw error;
        }
    }

}
