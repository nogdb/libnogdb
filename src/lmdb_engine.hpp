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

#include <cstring>
#include <string>

#include "datatype.hpp"
#include "lmdb/lmdb.h"

#include "nogdb/nogdb_errors.h"

namespace nogdb {
namespace storage_engine {
namespace lmdb {
    using namespace internal_data_type;

    constexpr static unsigned int DEFAULT_ENV_FLAG = MDB_NOTLS;
    constexpr static unsigned int DEFAULT_ENV_MODE = 0664;
    constexpr static unsigned int TXN_RW = 0;
    constexpr static unsigned int TXN_RO = MDB_RDONLY;

    class Value {
    public:
        Value() noexcept = default;

        ~Value() noexcept = default;

        explicit Value(const void* const data, size_t size) noexcept
            : _val { size, const_cast<void*>(data) }
        {
        }

        Value(const std::string& data) noexcept
            : Value { data.data(), data.size() }
        {
        }

        explicit Value(const char* const data) noexcept
            : Value { data, strlen(data) }
        {
        }

        Value(Value&& other) noexcept = default;

        Value& operator=(Value&& other) noexcept = default;

        operator const MDB_val*() const noexcept
        {
            return &_val;
        }

        operator MDB_val*() noexcept
        {
            return &_val;
        }

        size_t size() const noexcept
        {
            return _val.mv_size;
        }

        bool empty() const noexcept
        {
            return size() == 0;
        }

        template <typename T>
        T* data() noexcept
        {
            return reinterpret_cast<T*>(_val.mv_data);
        }

        template <typename T>
        const T* data() const noexcept
        {
            return reinterpret_cast<T*>(_val.mv_data);
        }

        char* data() noexcept
        {
            return reinterpret_cast<char*>(_val.mv_data);
        }

        const char* data() const noexcept
        {
            return reinterpret_cast<char*>(_val.mv_data);
        }

        template <typename T>
        T numeric() const noexcept
        {
            T result {};
            memcpy(static_cast<void*>(&result), static_cast<const void*>(_val.mv_data), sizeof(T));
            return result;
        }

        std::string string() const noexcept
        {
            return std::string(static_cast<char*>(_val.mv_data), _val.mv_size);
        }

        Blob blob() const noexcept
        {
            return Blob(static_cast<Blob::Byte*>(_val.mv_data), _val.mv_size);
        }

        template <typename T>
        Value& assign(const T* const data,
            const std::size_t size) noexcept
        {
            _val.mv_size = size;
            _val.mv_data = const_cast<void*>(reinterpret_cast<const void*>(data));
            return *this;
        }

        Value& assign(const char* const data) noexcept
        {
            return assign(data, strlen(data));
        }

        Value& assign(const std::string& data) noexcept
        {
            return assign(data.data(), data.size());
        }

    private:
        MDB_val _val;
    };

    typedef Value Key;
    typedef unsigned int Flag;
    typedef mdb_mode_t Mode;
    typedef MDB_env EnvHandler;
    typedef MDB_txn TransactionHandler;
    typedef MDB_dbi DBHandler;
    typedef MDB_cursor CursorHandler;

    struct Result {
        Value data {};
        bool empty { false };
    };

    struct CursorResult {
        Result key {};
        Result val {};

        bool empty() const noexcept
        {
            return key.empty;
        }
    };

    class Env {
    public:
        static Env create(unsigned int dbNum,
            unsigned long dbSize,
            unsigned int dbMaxReaders)
        {
            EnvHandler* handler = nullptr;
            if (auto error = mdb_env_create(&handler)) {
                throw NOGDB_STORAGE_ERROR(error);
            }
            try {
                if (auto error = mdb_env_set_mapsize(handler, dbSize)) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
                if (auto error = mdb_env_set_maxreaders(handler, dbMaxReaders)) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
                if (dbNum != 0) {
                    if (auto error = mdb_env_set_maxdbs(handler, dbNum)) {
                        throw NOGDB_STORAGE_ERROR(error);
                    }
                }
            } catch (const Error&) {
                mdb_env_close(handler);
                throw;
            }
            return Env { handler };
        }

        Env(EnvHandler* const handle) noexcept
            : _handle { handle }
        {
        }

        ~Env() noexcept
        {
            try {
                close();
            } catch (...) {
            }
        }

        Env(Env&& other) noexcept
        {
            using std::swap;
            std::swap(_handle, other._handle);
        }

        Env& operator=(Env&& other) noexcept
        {
            if (this != &other) {
                using std::swap;
                swap(_handle, other._handle);
            }
            return *this;
        }

        explicit operator EnvHandler*() const noexcept
        {
            return _handle;
        }

        MDB_env* handle() const noexcept
        {
            return _handle;
        }

        void sync(const bool force = true)
        {
            if (auto error = mdb_env_sync(_handle, force)) {
                throw error;
            }
        }

        void close() noexcept
        {
            if (_handle) {
                mdb_env_close(_handle);
                _handle = nullptr;
            }
        }

        Env& open(const std::string dbPath,
            const Flag flag = DEFAULT_ENV_FLAG,
            const Mode mode = DEFAULT_ENV_MODE)
        {
            if (auto error = mdb_env_open(handle(), dbPath.c_str(), flag, mode)) {
                throw NOGDB_STORAGE_ERROR(error);
            }
            return *this;
        }

    protected:
        EnvHandler* _handle { nullptr };
    };

    class Transaction {
    public:
        static Transaction begin(EnvHandler* const env,
            const unsigned int flag,
            TransactionHandler* const parent = nullptr)
        {
            TransactionHandler* handle = nullptr;
            if (auto error = mdb_txn_begin(env, parent, flag, &handle)) {
                throw NOGDB_STORAGE_ERROR(error);
            }
            return Transaction { handle };
        }

        Transaction(TransactionHandler* const handle) noexcept
            : _handle { handle }
        {
        }

        ~Transaction() noexcept
        {
            if (_handle) {
                try {
                    abort();
                } catch (...) {
                }
                _handle = nullptr;
            }
        }

        Transaction(Transaction&& other) noexcept
        {
            using std::swap;
            swap(_handle, other._handle);
        }

        Transaction& operator=(Transaction&& other) noexcept
        {
            if (this != &other) {
                using std::swap;
                swap(_handle, other._handle);
            }
            return *this;
        }

        operator TransactionHandler*() const noexcept
        {
            return _handle;
        }

        TransactionHandler* handle() const noexcept
        {
            return _handle;
        }

        EnvHandler* env() const noexcept
        {
            return mdb_txn_env(_handle);
        }

        void commit()
        {
            if (auto error = mdb_txn_commit(_handle)) {
                throw NOGDB_STORAGE_ERROR(error);
            }
            _handle = nullptr;
        }

        void abort() noexcept
        {
            mdb_txn_abort(_handle);
            _handle = nullptr;
        }

        void reset() noexcept
        {
            mdb_txn_reset(_handle);
        }

        void renew()
        {
            if (auto error = mdb_txn_renew(_handle)) {
                throw NOGDB_STORAGE_ERROR(error);
            }
        }

    protected:
        TransactionHandler* _handle { nullptr };
    };

    class DBi {
    public:
        static DBi open(TransactionHandler* const txnHandler,
            const std::string& dbName,
            bool numericKey = false,
            bool unique = true)
        {
            DBHandler dbHandler = 0;
            auto flags = ((numericKey) ? MDB_INTEGERKEY : 0U) | ((!unique) ? MDB_DUPSORT : 0U);
            if (auto error = mdb_open(txnHandler, dbName.c_str(), MDB_CREATE | flags, &dbHandler)) {
                throw NOGDB_STORAGE_ERROR(error);
            } else {
                return DBi { txnHandler, dbHandler };
            }
        }

        DBi() = default;

        DBi(TransactionHandler* const txnHandler, const DBHandler handle) noexcept
            : _handle { handle }
            , _txn { txnHandler }
        {
        }

        ~DBi() noexcept {}

        DBi(DBi&& other) noexcept
        {
            using std::swap;
            swap(_handle, other._handle);
            _txn = other._txn;
            other._txn = nullptr;
        }

        DBi& operator=(DBi&& other) noexcept
        {
            if (this != &other) {
                using std::swap;
                swap(_handle, other._handle);
                _txn = other._txn;
                other._txn = nullptr;
            }
            return *this;
        }

        operator DBHandler() const noexcept
        {
            return _handle;
        }

        DBHandler handle() const noexcept
        {
            return _handle;
        }

        TransactionHandler* txn() const noexcept
        {
            return _txn;
        }

        unsigned int flags() const
        {
            unsigned int result {};
            if (auto error = mdb_dbi_flags(_txn, _handle, &result)) {
                throw NOGDB_STORAGE_ERROR(error);
            }
            return result;
        }

        size_t size() const
        {
            return stat().ms_entries;
        }

        void drop(const bool del = false)
        {
            if (auto error = mdb_drop(_txn, _handle, del)) {
                throw NOGDB_STORAGE_ERROR(error);
            }
        }

        DBi& setCompareFunc(MDB_cmp_func* const cmp = nullptr)
        {
            if (auto error = mdb_set_compare(_txn, _handle, cmp)) {
                throw NOGDB_STORAGE_ERROR(error);
            }
            return *this;
        }

        DBi& setDupSortFunc(MDB_cmp_func* const cmp = nullptr)
        {
            if (auto error = mdb_set_dupsort(_txn, _handle, cmp)) {
                throw NOGDB_STORAGE_ERROR(error);
            }
            return *this;
        }

        template <typename K>
        Result get(const K& key) const
        {
            Result result {};
            auto found = dbGet(Key { &key, sizeof(K) }, result.data);
            result.empty = !found;
            return result;
        }

        Result get(const Key& key) const
        {
            Result result {};
            auto found = dbGet(key, result.data);
            result.empty = !found;
            return result;
        }

        Result get(const char* const key) const
        {
            Result result {};
            auto found = dbGet(Key { key, strlen(key) }, result.data);
            result.empty = !found;
            return result;
        }

        Result get(const std::string& key) const
        {
            Result result {};
            auto found = dbGet(Key { key }, result.data);
            result.empty = !found;
            return result;
        }

#define LMDB_PUT_FLAGS_GENERATE(append, overwrite) ((append) ? MDB_APPEND : 0U) | ((overwrite) ? 0U : MDB_NOOVERWRITE)

        template <typename K, typename V>
        void put(const K& key,
            const V& val,
            bool append = false,
            bool overwrite = true)
        {
            dbPut(Key { &key, sizeof(K) }, Value { &val, sizeof(V) }, LMDB_PUT_FLAGS_GENERATE(append, overwrite));
        }

        template <typename K>
        void put(const K& key,
            const Blob& blob,
            bool append = false,
            bool overwrite = true)
        {
            dbPut(Key { &key, sizeof(K) },
                Value { blob.bytes(), blob.size() },
                LMDB_PUT_FLAGS_GENERATE(append, overwrite));
        }

        template <typename K>
        void put(const K& key,
            const std::string& data,
            bool append = false,
            bool overwrite = true)
        {
            dbPut(Key { &key, sizeof(K) }, Value { data }, LMDB_PUT_FLAGS_GENERATE(append, overwrite));
        }

        void put(const Key& key,
            const Value& data,
            bool append = false,
            bool overwrite = true)
        {
            dbPut(key, data, LMDB_PUT_FLAGS_GENERATE(append, overwrite));
        }

        template <typename V>
        void put(const char* const key,
            const V& val,
            bool append = false,
            bool overwrite = true)
        {
            dbPut(Key { key, strlen(key) }, Value { &val, sizeof(V) }, LMDB_PUT_FLAGS_GENERATE(append, overwrite));
        }

        void put(const char* const key,
            const Blob& blob,
            bool append = false,
            bool overwrite = true)
        {
            dbPut(Key { key, strlen(key) },
                Value { blob.bytes(), blob.size() },
                LMDB_PUT_FLAGS_GENERATE(append, overwrite));
        }

        void put(const char* const key,
            const char* const val,
            bool append = false,
            bool overwrite = true)
        {
            dbPut(Key { key, strlen(key) }, Value { val, strlen(val) }, LMDB_PUT_FLAGS_GENERATE(append, overwrite));
        }

        template <typename V>
        void put(const std::string& key,
            const V& val,
            bool append = false,
            bool overwrite = true)
        {
            dbPut(Key { key }, Value { &val, sizeof(V) }, LMDB_PUT_FLAGS_GENERATE(append, overwrite));
        }

        void put(const std::string& key,
            const Blob& blob,
            bool append = false,
            bool overwrite = true)
        {
            dbPut(Key { key },
                Value { blob.bytes(), blob.size() },
                LMDB_PUT_FLAGS_GENERATE(append, overwrite));
        }

        void put(const std::string& key,
            const std::string& val,
            bool append = false,
            bool overwrite = true)
        {
            dbPut(Key { key }, Value { val }, LMDB_PUT_FLAGS_GENERATE(append, overwrite));
        }

        template <typename K>
        void del(const K& key)
        {
            dbDel(Key { &key, sizeof(K) });
        }

        void del(const Key& key)
        {
            dbDel(key);
        }

        void del(const char* const key)
        {
            dbDel(Key { key, strlen(key) });
        }

        void del(const std::string& key)
        {
            dbDel(Key { key });
        }

        template <typename K, typename V>
        void del(const K& key, const V& val)
        {
            dbDel(Key { &key, sizeof(key) }, Value { &val, sizeof(V) });
        }

        template <typename K>
        void del(const K& key, const Blob& blob)
        {
            dbDel(Key { &key, sizeof(key) }, Value { blob.bytes(), blob.size() });
        }

        template <typename V>
        void del(const Key& key, const V& val)
        {
            dbDel(key, Value { &val, sizeof(V) });
        }

        void del(const Key& key, const Blob& blob)
        {
            dbDel(key, Value { blob.bytes(), blob.size() });
        }

        void del(const char* const key, const char* const val)
        {
            dbDel(Key { key, strlen(key) }, Value { val, strlen(val) });
        }

        void del(const std::string& key, const std::string& val)
        {
            dbDel(Key { key }, Value { val });
        }

    protected:
        DBHandler _handle { 0 };
        TransactionHandler* _txn { nullptr };

    private:
        MDB_stat stat() const
        {
            MDB_stat result;
            if (auto error = mdb_stat(_txn, _handle, &result)) {
                throw NOGDB_STORAGE_ERROR(error);
            }
            return result;
        }

        inline bool dbGet(const MDB_val* const key,
            MDB_val* const data) const
        {
            if (auto error = mdb_get(_txn, _handle, const_cast<MDB_val*>(key), data)) {
                if (error != MDB_NOTFOUND) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
                return false;
            }
            return true;
        }

        inline void dbPut(const MDB_val* const key,
            const MDB_val* const data,
            const unsigned int flags = 0)
        {
            if (auto error = mdb_put(_txn, _handle, const_cast<MDB_val*>(key), const_cast<MDB_val*>(data), flags)) {
                throw NOGDB_STORAGE_ERROR(error);
            }
        }

        inline void dbDel(const MDB_val* const key,
            const MDB_val* const data = nullptr)
        {
            if (auto error = mdb_del(_txn, _handle, const_cast<MDB_val*>(key), const_cast<MDB_val*>(data))) {
                if (error != MDB_NOTFOUND) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
            }
        }
    };

    class Cursor {
    public:
        static Cursor open(TransactionHandler* const txn, const DBHandler dbi)
        {
            CursorHandler* handle = nullptr;
            if (auto error = mdb_cursor_open(txn, dbi, &handle)) {
                throw NOGDB_STORAGE_ERROR(error);
            }
            return Cursor { txn, handle };
        }

        Cursor(TransactionHandler* const txn, CursorHandler* const handle) noexcept
            : _handle { handle }
            , _txn { txn }
        {
        }

        Cursor(Cursor&& other) noexcept
        {
            using std::swap;
            swap(_handle, other._handle);
            _txn = other._txn;
            other._txn = nullptr;
        }

        Cursor& operator=(Cursor&& other) noexcept
        {
            if (this != &other) {
                using std::swap;
                swap(_handle, other._handle);
                _txn = other._txn;
                other._txn = nullptr;
            }
            return *this;
        }

        ~Cursor() noexcept
        {
            try {
                close();
            } catch (...) {
            }
            _txn = nullptr;
        }

        operator CursorHandler*() const noexcept
        {
            return _handle;
        }

        CursorHandler* handle() const noexcept
        {
            return _handle;
        }

        void close() noexcept
        {
            if (_handle) {
                mdb_cursor_close(_handle);
                _handle = nullptr;
            }
        }

        void renew()
        {
            if (auto error = mdb_cursor_renew(_txn, _handle)) {
                throw NOGDB_STORAGE_ERROR(error);
            }
        }

        TransactionHandler* txn() const noexcept
        {
            return mdb_cursor_txn(_handle);
        }

        DBHandler dbi() const noexcept
        {
            return mdb_cursor_dbi(_handle);
        }

        void del(bool duplicate = false) const
        {
            if (auto error = mdb_cursor_del(_handle, duplicate)) {
                throw NOGDB_STORAGE_ERROR(error);
            }
        }

        CursorResult getNext() const
        {
            return get(MDB_NEXT);
        }

        CursorResult getNextDup() const
        {
            return get(MDB_NEXT_DUP);
        }

        CursorResult getPrev() const
        {
            return get(MDB_PREV);
        }

        CursorResult getPrevDup() const
        {
            return get(MDB_PREV_DUP);
        }

        template <typename K>
        CursorResult find(const K& key) const
        {
            return dbFind(key, MDB_SET_KEY);
        }

        template <typename K>
        CursorResult findRange(const K& key) const
        {
            return dbFind(key, MDB_SET_RANGE);
        }

    protected:
        CursorHandler* _handle { nullptr };
        TransactionHandler* _txn { nullptr };

    private:
        CursorResult get(const MDB_cursor_op op) const
        {
            CursorResult result {};
            if (auto error = mdb_cursor_get(_handle, result.key.data, result.val.data, op)) {
                if (error != MDB_NOTFOUND) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
                result.key.empty = error == MDB_NOTFOUND;
                result.val.empty = result.key.empty;
            }
            return result;
        }

        template <typename K>
        CursorResult dbFind(const K& key, const MDB_cursor_op op) const
        {
            CursorResult result {};
            result.key.data = Key { &key, sizeof(K) };
            if (auto error = mdb_cursor_get(_handle, result.key.data, result.val.data, op)) {
                if (error != MDB_NOTFOUND) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
                result.key.empty = error == MDB_NOTFOUND;
                result.val.empty = result.key.empty;
            }
            return result;
        }

        CursorResult dbFind(const std::string& key, const MDB_cursor_op op) const
        {
            CursorResult result {};
            result.key.data = Key { key };
            if (auto error = mdb_cursor_get(_handle, result.key.data, result.val.data, op)) {
                if (error != MDB_NOTFOUND) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
                result.key.empty = error == MDB_NOTFOUND;
                result.val.empty = result.key.empty;
            }
            return result;
        }
    };
}
}
}