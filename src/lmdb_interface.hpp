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

#ifndef __LMDB_INTERFACE_HPP_INCLUDED_
#define __LMDB_INTERFACE_HPP_INCLUDED_

#include <string>
#include <cstring>

#include "lmdb/lmdb.h"
#include "blob.hpp"

#include "nogdb_errors.h"

namespace nogdb {

    /**
     * An implementation of LMDB wrapper and interfaces
     *
     * see more @ http://www.lmdb.tech/
     *
     * NOTE: Special thanks to https://github.com/bendiken/lmdbxx/blob/master/lmdb%2B%2B.h
     * for the original idea and source code of lmdbxx
     */
    namespace lmdb_interface {

        /**
         * Definitions of mdb environment flags
         *
         * see more @ http://www.lmdb.tech/doc/group__mdb__env.html
         */
        constexpr static unsigned int DEFAULT_ENV_FLAG          = MDB_NOTLS;
        constexpr static unsigned int DEFAULT_ENV_MODE          = 0664;
        constexpr static unsigned int DEFAULT_ENV_MAX_READERS   = 65536;
        constexpr static unsigned int TXN_RW                    = 0;
        constexpr static unsigned int TXN_RO                    = MDB_RDONLY;

        /**
         * A wrapper class for 'MDB_val' support multiple primitive data types
         *
         * see more @ http://www.lmdb.tech/doc/group__mdb.html#structMDB__val
         */
        class Value {
        public:
            /**
             * Default Constructor
             */
            Value() noexcept = default;

            /**
             * Destructor
             */
            ~Value() noexcept = default;

            /**
             * Default Constructor
             */
            explicit Value(const void *const data, size_t size) noexcept
                    : _val{size, const_cast<void *>(data)} {}

            /**
             * Constructor
             */
            Value(const std::string &data) noexcept
                    : Value{data.data(), data.size()} {}

            /**
             * Constructor
             */
            explicit Value(const char *const data) noexcept
                    : Value{data, strlen(data)} {}

            /**
             * Move Constructor
             */
            Value(Value &&other) noexcept = default;

            /**
             * Move assignment operator
             */
            Value &operator=(Value &&other) noexcept = default;

            /**
             * Return a const pointer of 'MDB_val'
             */
            operator const MDB_val *() const noexcept {
                return &_val;
            }

            /**
             * Return a pointer of 'MDB_val'
             */
            operator MDB_val *() noexcept {
                return &_val;
            }

            /**
             * Return a size of value
             */
            size_t size() const noexcept {
                return _val.mv_size;
            }

            /**
             * Determine if value is empty
             */
            bool empty() const noexcept {
                return size() == 0;
            }

            /**
             * Returns a pointer to the data
             */
            template<typename T>
            T *data() noexcept {
                return reinterpret_cast<T *>(_val.mv_data);
            }

            /**
             * Return a pointer to the data
             */
            template<typename T>
            const T *data() const noexcept {
                return reinterpret_cast<T *>(_val.mv_data);
            }

            /**
             * Return a pointer to the data
             */
            char *data() noexcept {
                return reinterpret_cast<char *>(_val.mv_data);
            }

            /**
             * Return a pointer to the data
             */
            const char *data() const noexcept {
                return reinterpret_cast<char *>(_val.mv_data);
            }

            /**
             * Return a pointer to the blob data
             */
             Blob blob() noexcept {
                return Blob(static_cast<Blob::Byte *>(_val.mv_data), _val.mv_size);
             }

            /**
             * Assign the value
             */
            template<typename T>
            Value &assign(const T *const data,
                          const std::size_t size) noexcept {
                _val.mv_size = size;
                _val.mv_data = const_cast<void *>(reinterpret_cast<const void *>(data));
                return *this;
            }

            /**
             * Assign the value
             */
            Value &assign(const char *const data) noexcept {
                return assign(data, std::strlen(data));
            }

            /**
             * Assign the value
             */
            Value &assign(const std::string &data) noexcept {
                return assign(data.data(), data.size());
            }

        private:
            MDB_val _val;
        };

        /**
         * Declarations of the data types used by LMDB interface
         */
        typedef Value                   Key;
        typedef int                     ErrorType;
        typedef unsigned int            Flag;
        typedef mdb_mode_t              Mode;
        typedef MDB_env                 EnvHandler;
        typedef MDB_txn                 TxnHandler;
        typedef MDB_dbi                 DBHandler;
        typedef MDB_cursor              CursorHandler;

        /**
         * A pair of LMDB value and found status returned from *get functions
         */
        struct Result {
            Value data{};
            bool found{false};
        };

        struct CursorResult {
            Result key{};
            Result val{};

            bool found() const noexcept {
                return key.found;
            }
        };

        /**
         * A set of LMDB (interface) operations to create environment, open database,
         * put/get/del records, including cursors
         *
         * see more @ http://www.lmdb.tech/doc/group__mdb.html
         */
        class Env {
        public:
            /**
             * Create a new LMDB environment
             *
             * @param dbPath a name of LMDB file including its prefix path
             * @param dbNum a pre-defined maximum number of databases in a LMDB file
             * @paran dbSize a pre-defined size of total databases in a LMDB file
             * @param dbMaxReaders a maximum number of readers who will be accessing a LMDB file
             * @param flag a flag used in creating and opening LMDB environment
             * @param mode a mode used in creating and opening LMDB environment
             */
            static Env create(unsigned int dbNum,
                              unsigned long dbSize,
                              unsigned int dbMaxReaders = DEFAULT_ENV_MAX_READERS) {
                EnvHandler *handler = nullptr;
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
                } catch (const ErrorType&) {
                    mdb_env_close(handler);
                    throw;
                }
                return Env{handler};
            }

            /**
             * Constructor
             *
             * @param handle a valid `EnvHandler*` handle
             */
            Env(EnvHandler* const handle) noexcept
                    : _handle{handle} {}

            /**
             * Destructor
             */
            ~Env() noexcept {
                try { close(); } catch (...) {}
            }

            /**
             * Move constructor
             */
            Env(Env&& other) noexcept {
                using std::swap;
                std::swap(_handle, other._handle);
            }

            /**
             * Move assignment operator
             */
            Env& operator=(Env&& other) noexcept {
                if (this != &other) {
                    using std::swap;
                    swap(_handle, other._handle);
                }
                return *this;
            }

            /**
             * Return the underlying `EnvHandler*` handle
             */
            explicit operator EnvHandler*() const noexcept {
                return _handle;
            }

            /**
             * Return the underlying `EnvHandler*` handle
             */
            MDB_env* handle() const noexcept {
                return _handle;
            }

            /**
             * Flush data buffers to disk
             *
             * @param force if non-zero, force a synchronous flush
             */
            void sync(const bool force = true) {
                if (auto error = mdb_env_sync(_handle, force)) {
                    throw error;
                }
            }

            /**
             * Close this environment, releasing the memory map
             */
            void close() noexcept {
                if (handle()) {
                    mdb_env_close(handle());
                    _handle = nullptr;
                }
            }

            /**
             * Open this environment
             *
             * @param dbPath a name of LMDB file including its prefix path
             * @param flag a flag used in creating and opening LMDB environment
             * @param mode a mode used in creating and opening LMDB environment
             */
            Env& open(const std::string dbPath,
                      const Flag flag = DEFAULT_ENV_FLAG,
                      const Mode mode = DEFAULT_ENV_MODE) {
                if (auto error = mdb_env_open(handle(), dbPath.c_str(), flag, mode)) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
                return *this;
            }

        protected:
            EnvHandler* _handle{nullptr};
        };

        class Txn {
        public:
            /**
             * Creates a new LMDB transaction.
             *
             * @param env the environment handle
             * @param flag
             * @param parent a parent transaction handle
             */
            static Txn begin(EnvHandler* const env,
                             const unsigned int flag,
                             TxnHandler* const parent = nullptr) {
                TxnHandler *handle = nullptr;
                if (auto error = mdb_txn_begin(env, parent, flag, &handle)) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
                return Txn{handle};
            }

            /**
             * Constructor
             *
             * @param handle a valid `TxnHandler*` handle
             */
            Txn(TxnHandler* const handle) noexcept
                    : _handle{handle} {}

            /**
             * Destructor
             */
            ~Txn() noexcept {
                if (_handle) {
                    try { abort(); } catch (...) {}
                    _handle = nullptr;
                }
            }

            /**
             * Move constructor
             */
            Txn(Txn&& other) noexcept {
                using std::swap;
                swap(_handle, other._handle);
            }

            /**
             * Move assignment operator
             */
            Txn& operator=(Txn&& other) noexcept {
                if (this != &other) {
                    using std::swap;
                    swap(_handle, other._handle);
                }
                return *this;
            }

            /**
             * Return the underlying `TxnHandler*` handle
             */
            operator TxnHandler*() const noexcept {
                return _handle;
            }

            /**
             * Return the underlying `TxnHandler*` handle
             */
            TxnHandler* handle() const noexcept {
                return _handle;
            }

            /**
             * Return the transaction's `EnvHandler*` handle
             */
            EnvHandler* env() const noexcept {
                return mdb_txn_env(handle());
            }

            /**
             * Commit this transaction
             */
            void commit() {
                if (auto error = mdb_txn_commit(_handle)) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
                _handle = nullptr;
            }

            /**
             * Abort this transaction
             */
            void abort() noexcept {
                mdb_txn_abort(_handle);
                _handle = nullptr;
            }

            /**
             * Reset this read-only transaction
             */
            void reset() noexcept {
                mdb_txn_reset(_handle);
            }

            /**
             * Renew this read-only transaction
             */
            void renew() {
                if (auto error = mdb_txn_renew(_handle)) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
            }

        protected:
            TxnHandler* _handle{nullptr};
        };

        class Dbi {
        public:
            /**
             * Opens a database handle
             *
             * @param txnHandler the transaction handler
             * @param dbName a database name
             * @param isNumericKey this database is using numeric as keys, if not, using string
             * @param isUnique this database has unique values
             */
            static Dbi open(TxnHandler* const txnHandler,
                            const std::string& dbName,
                            bool isNumericKey = false,
                            bool isUnique = true) {
                DBHandler dbHandler = 0;
                auto flags = ((isNumericKey) ? MDB_INTEGERKEY : 0U) | ((!isUnique) ? MDB_DUPSORT : 0U);
                if (auto error = mdb_open(txnHandler, dbName.c_str(), MDB_CREATE | flags, &dbHandler)) {
                    throw NOGDB_STORAGE_ERROR(error);
                } else {
                    return Dbi{dbHandler};
                }
            }

            /**
             * Constructor
             *
             * @param handle a valid `DBHandler` handle
             */
            Dbi(const DBHandler handle) noexcept
                    : _handle{handle} {}

            /**
             * Destructor
             */
            ~Dbi() noexcept {}

            /**
             * Move constructor
             */
            Dbi(Dbi&& other) noexcept {
                using std::swap;
                swap(_handle, other._handle);
            }

            /**
             * Move assignment operator
             */
            Dbi& operator=(Dbi&& other) noexcept {
                if (this != &other) {
                    using std::swap;
                    swap(_handle, other._handle);
                }
                return *this;
            }

            /**
             * Return the underlying `DBHandler` handle
             */
            operator DBHandler() const noexcept {
                return _handle;
            }

            /**
             * Returns the underlying `DBHandler` handle
             */
            DBHandler handle() const noexcept {
                return _handle;
            }

            /**
             * Retrieve the flags for this database handle
             *
             * @param txn a transaction handle
             */
            unsigned int flags(TxnHandler* const txn) const {
                unsigned int result{};
                if (auto error = mdb_dbi_flags(txn, handle(), &result)) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
                return result;
            }

            /**
             * Return the number of records in this database
             *
             * @param txn a transaction handle
             */
            size_t size(TxnHandler* const txn) const {
                return stat(txn).ms_entries;
            }

            /**
             * Drop all records abd this database
             *
             * @param txn a transaction handle
             * @param del 0 to empty the DB, 1 to delete it from the environment and close the DB handle
             */
            void drop(TxnHandler* const txn, const bool del = false) {
                if (auto error = mdb_drop(txn, handle(), del)) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
            }

            /**
             * Set a custom key comparison function for this database
             *
             * @param txn a transaction handle
             * @param cmp the comparison function
             */
            Dbi& setCompareFunc(TxnHandler* const txn, MDB_cmp_func* const cmp = nullptr) {
                if (auto error = mdb_set_compare(txn, handle(), cmp)) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
                return *this;
            }

            /**
             * Retrieve a key/value pair from this database
             *
             * @param txn a transaction handle
             * @param key
             */
            template<typename K>
            Result get(TxnHandler* const txn, const K& key) const {
                const Key k{&key, sizeof(K)};
                Result result{};
                if (auto error = mdb_get(txn, handle(), k, result.data)) {
                    if (error != MDB_NOTFOUND) {
                        throw NOGDB_STORAGE_ERROR(error);
                    }
                    result.found = error != MDB_NOTFOUND;
                }
                return result;
            }

            /**
             * Retrieve a key/value pair from this database
             *
             * @param txn a transaction handle
             * @param key
             */
            Result get(TxnHandler* const txn, const Key& key) {
                Result result{};
                if (auto error = mdb_get(txn, handle(), key, result.data)) {
                    if (error != MDB_NOTFOUND) {
                        throw NOGDB_STORAGE_ERROR(error);
                    }
                    result.found = error != MDB_NOTFOUND;
                }
                return result;
            }

            /**
             * Retrieve a key/value pair from this database
             *
             * @param txn a transaction handle
             * @param key a NUL-terminated string key
             */
            Result get(TxnHandler* const txn, const char* const key) const {
                const Key k{key, strlen(key)};
                Result result{};
                if (auto error = mdb_get(txn, handle(), k, result.data)) {
                    if (error != MDB_NOTFOUND) {
                        throw NOGDB_STORAGE_ERROR(error);
                    }
                    result.found = error != MDB_NOTFOUND;
                }
                return result;
            }

#define LMDB_PUT_FLAGS_GENERATE(append, overwrite)  ((append) ? MDB_APPEND : 0U) | ((overwrite) ? 0U : MDB_NOOVERWRITE)

            /**
             * Store a key/value pair into this database
             *
             * @param txn a transaction handle
             * @param key
             * @param val
             * @param append a new value will be appended at the bottom of the database
             * @param overwrite enter the new key/data pair although the key already appears in the database
             */
            template<typename K, typename V>
            void put(TxnHandler* const txn,
                     const K& key,
                     const V& val,
                     bool append = false,
                     bool overwrite = true) {
                const Key k{&key, sizeof(K)};
                Value v{&val, sizeof(V)};
                if (auto error = mdb_put(txn, handle(), k, v, LMDB_PUT_FLAGS_GENERATE(append, overwrite))) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
            }

            /**
             * Store a key/blob pair into this database
             *
             * @param txn a transaction handle
             * @param key
             * @param blob
             * @param append a new value will be appended at the bottom of the database
             * @param overwrite enter the new key/data pair although the key already appears in the database
             */
            template<typename K>
            void put(TxnHandler* const txn,
                     const K& key,
                     const Blob& blob,
                     bool append = false,
                     bool overwrite = true) {
                const Key k{&key, sizeof(K)};
                Value v{blob.bytes(), blob.size()};
                if (auto error = mdb_put(txn, handle(), k, v, LMDB_PUT_FLAGS_GENERATE(append, overwrite))) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
            }

            /**
             * Store a key/value pair into this database
             *
             * @param txn a transaction handle
             * @param key
             * @param data
             * @param append a new value will be appended at the bottom of the database
             * @param overwrite enter the new key/data pair although the key already appears in the database
             */
            void put(TxnHandler* const txn,
                     const Key& key,
                     const Value& data,
                     bool append = false,
                     bool overwrite = true) {
                if (auto error = mdb_put(txn, handle(), key, data, LMDB_PUT_FLAGS_GENERATE(append, overwrite))) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
            }

            /**
             * Store a key/value pair into this database
             *
             * @param txn a transaction handle
             * @param key a NUL-terminated string key
             * @param val
             * @param append a new value will be appended at the bottom of the database
             * @param overwrite enter the new key/data pair although the key already appears in the database
             */
            template<typename V>
            void put(TxnHandler* const txn,
                     const char* const key,
                     const V& val,
                     bool append = false,
                     bool overwrite = true) {
                const Key k{key, std::strlen(key)};
                Value v{&val, sizeof(V)};
                if (auto error = mdb_put(txn, handle(), k, v, LMDB_PUT_FLAGS_GENERATE(append, overwrite))) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
            }

            /**
             * Store a key/blob pair into this database
             *
             * @param txn a transaction handle
             * @param key a NUL-terminated string key
             * @param blob
             * @param append a new value will be appended at the bottom of the database
             * @param overwrite enter the new key/data pair although the key already appears in the database
             */
            void put(TxnHandler* const txn,
                     const char* const key,
                     const Blob& blob,
                     bool append = false,
                     bool overwrite = true) {
                const Key k{key, std::strlen(key)};
                Value v{blob.bytes(), blob.size()};
                if (auto error = mdb_put(txn, handle(), k, v, LMDB_PUT_FLAGS_GENERATE(append, overwrite))) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
            }

            /**
             * Store a key/value pair into this database
             *
             * @param txn a transaction handle
             * @param key a NUL-terminated string key
             * @param val a NUL-terminated string key
             * @param append a new value will be appended at the bottom of the database
             * @param overwrite enter the new key/data pair although the key already appears in the database
             */
            bool put(TxnHandler* const txn,
                     const char* const key,
                     const char* const val,
                     bool append = false,
                     bool overwrite = true) {
                const Key k{key, std::strlen(key)};
                Value v{val, std::strlen(val)};
                if (auto error = mdb_put(txn, handle(), k, v, LMDB_PUT_FLAGS_GENERATE(append, overwrite))) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
            }

            /**
             * Remove a key/value pair from this database
             *
             * @param txn a transaction handle
             * @param key
             */
            template<typename K>
            void del(TxnHandler* const txn, const K& key) {
                const Key k{&key, sizeof(K)};
                if (auto error = mdb_del(txn, handle(), k, nullptr)) {
                    if (error != MDB_NOTFOUND) {
                        throw NOGDB_STORAGE_ERROR(error);
                    }
                }
            }

            /**
             * Remove a key/value pair from this database
             *
             * @param txn a transaction handle
             * @param key
             */
            void del(TxnHandler* const txn, const Key& key) {
                if (auto error = mdb_del(txn, handle(), key, nullptr)) {
                    if (error != MDB_NOTFOUND) {
                        throw NOGDB_STORAGE_ERROR(error);
                    }
                }
            }

            /**
             * Remove a key/value pair from this database
             *
             * @param txn a transaction handle
             * @param key
             */
            void del(TxnHandler* const txn, const char* const key) {
                const Key k{key, std::strlen(key)};
                if (auto error = mdb_del(txn, handle(), k, nullptr)) {
                    if (error != MDB_NOTFOUND) {
                        throw NOGDB_STORAGE_ERROR(error);
                    }
                }
            }

            /**
             * Remove a key/value pair from this database
             *
             * @param txn a transaction handle
             * @param key
             * @param value
             */
            template<typename K, typename V>
            void del(TxnHandler* const txn, const K& key, const V& val) {
                const Key k{key, std::strlen(key)};
                Value v{&val, sizeof(V)};
                if (auto error = mdb_del(txn, handle(), k, v)) {
                    if (error != MDB_NOTFOUND) {
                        throw NOGDB_STORAGE_ERROR(error);
                    }
                }
            }

            /**
             * Remove a key/value pair from this database
             *
             * @param txn a transaction handle
             * @param key
             * @param value
             */
            template<typename V>
            void del(TxnHandler* const txn, const Key& key, const V& val) {
                Value v{&val, sizeof(V)};
                if (auto error = mdb_del(txn, handle(), key, v)) {
                    if (error != MDB_NOTFOUND) {
                        throw NOGDB_STORAGE_ERROR(error);
                    }
                }
            }

            /**
             * Remove a key/value pair from this database
             *
             * @param txn a transaction handle
             * @param key
             * @param value
             */
            void del(TxnHandler* const txn, const char* const key, const char* const val) {
                const Key k{key, std::strlen(key)};
                Value v{val, std::strlen(val)};
                if (auto error = mdb_del(txn, handle(), k, v)) {
                    if (error != MDB_NOTFOUND) {
                        throw NOGDB_STORAGE_ERROR(error);
                    }
                }
            }

        protected:
            DBHandler _handle{0};

        private:
            /**
             * Returns statistics for this database
             *
             * @param txn a transaction handle
             */
            MDB_stat stat(TxnHandler* const txn) const {
                MDB_stat result;
                if (auto error = mdb_stat(txn, handle(), &result)) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
                return result;
            }
        };

        class Cursor {
        public:
            /**
             * Create an LMDB cursor
             *
             * @param txn the transaction handle
             * @param dbi the database handle
             */
            static Cursor open(TxnHandler* const txn, const DBHandler dbi) {
                CursorHandler* handle = nullptr;
                if (auto error = mdb_cursor_open(txn, dbi, &handle)) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
                return Cursor{handle};
            }

            /**
             * Constructor
             *
             * @param handle a valid `CursorHandler*` handle
             */
            Cursor(CursorHandler* const handle) noexcept
                    : _handle{handle} {}

            /**
             * Move constructor
             */
            Cursor(Cursor&& other) noexcept {
                using std::swap;
                swap(_handle, other._handle);
            }

            /**
             * Move assignment operator
             */
            Cursor& operator=(Cursor&& other) noexcept {
                if (this != &other) {
                    using std::swap;
                    swap(_handle, other._handle);
                }
                return *this;
            }

            /**
             * Destructor
             */
            ~Cursor() noexcept {
                try { close(); } catch (...) {}
            }

            /**
             * Return the underlying `CursorHandler*` handle
             */
            operator CursorHandler*() const noexcept {
                return _handle;
            }

            /**
             * Return the underlying `CursorHandler*` handle
             */
            CursorHandler* handle() const noexcept {
                return _handle;
            }

            /**
             * Close this cursor
             */
            void close() noexcept {
                if (_handle) {
                    mdb_cursor_close(_handle);
                    _handle = nullptr;
                }
            }

            /**
             * Renew this cursor
             *
             * @param txn the transaction scope
             */
            void renew(TxnHandler* const txn) {
                if (auto error = mdb_cursor_renew(txn, handle())) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
            }

            /**
             * Return the cursor's transaction handle
             */
            TxnHandler* txn() const noexcept {
                return mdb_cursor_txn(handle());
            }

            /**
             * Return the cursor's database handle
             */
            DBHandler dbi() const noexcept {
                return mdb_cursor_dbi(handle());
            }

            /**
             * Retrieve a key/value from the database
             *
             * @param op
             */
            CursorResult get(const MDB_cursor_op op) {
                CursorResult result{};
                if (auto error = mdb_cursor_get(handle(), result.key.data, result.val.data, op)) {
                    if (error != MDB_NOTFOUND) {
                        throw NOGDB_STORAGE_ERROR(error);
                    }
                    result.key.found = error != MDB_NOTFOUND;
                    result.val.found = result.key.found;
                }
                return result;
            }

            /**
             * Position this cursor at the given key
             *
             * @param key
             * @param op
             */
            template<typename K>
            Result find(const K& key, const MDB_cursor_op op) {
                Key k{&key, sizeof(K)};
                Result result{};
                if (auto error = mdb_cursor_get(handle(), k, result.data, op)) {
                    if (error != MDB_NOTFOUND) {
                        throw NOGDB_STORAGE_ERROR(error);
                    }
                    result.found = error != MDB_NOTFOUND;
                }
                return result;
            }

            /**
             * Remove a key/value at the position of the cursor
             *
             * @param duplicate delete all of the data items for the current key
             */
            void del(bool duplicate = false) {
                if (auto error = mdb_cursor_del(handle(), duplicate)) {
                    throw NOGDB_STORAGE_ERROR(error);
                }
            }

        protected:
            CursorHandler* _handle{nullptr};
        };

        struct CursorHelper: public Cursor {
            using Cursor::Cursor;

            /**
             * Retrieve a key/value from the database by the next cursor
             */
            CursorResult getNext() {
                return get(MDB_NEXT);
            }

            /**
             * Retrieve a duplicate key/value from the database by the next cursor
             */
            CursorResult getNextDup() {
                return get(MDB_NEXT_DUP);
            }

            /**
             * Retrieve a key/value from the database by the previous cursor
             */
            CursorResult getPrev() {
                return get(MDB_PREV);
            }

            /**
             * Retrieve a key/value from the database
             */
            template<typename K>
            Result find(const K& key) {
                return Cursor::find(key, MDB_SET_KEY);
            }

            /**
             * Retrieve a range of key/value from the database
             */
            template<typename K>
            Result findRange(const K& key) {
                return Cursor::find(key, MDB_SET_RANGE);
            }
        };
    }
}

#endif
