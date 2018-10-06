/*
 *  Copyright (C) 2018, Throughwave (Thailand) Co., Ltd.
 *  <peerawich at throughwave dot co dot th>
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

#include <string>
#include "storage_engine.hpp"

namespace nogdb {

  namespace storage_engine {

    namespace adapter {

      class LMDBKeyValAccess {
      public:

        LMDBKeyValAccess() = default;

        LMDBKeyValAccess(const LMDBTxn *txn,
                         const std::string &dbName,
                         bool numericKey = false,
                         bool unique = true,
                         bool append = false,
                         bool overwrite = true)
            : _txn{txn}, _append{append}, _overwrite{overwrite} {
          _dbi = txn->openDbi(dbName, numericKey, unique);
        }

        virtual ~LMDBKeyValAccess() noexcept = default;

        LMDBKeyValAccess(LMDBKeyValAccess &&other) noexcept
            : _txn{nullptr} {
          *this = std::move(other);
        }

        LMDBKeyValAccess &operator=(LMDBKeyValAccess &&other) noexcept {
          if (this != &other) {
            _txn = other._txn;
            _dbi = std::move(other._dbi);
            _append = other._append;
            _overwrite = other._overwrite;
            other._txn = nullptr;
          }
          return *this;
        }

      protected:

        template<typename K, typename V>
        void put(const K &key, const V &val) {
          _dbi.put(key, val, _append, _overwrite);
        }

        template<typename K>
        lmdb::Result get(const K &key) const {
          return _dbi.get(key);
        }

        template<typename K>
        void del(const K &key) {
          _dbi.del(key);
        }

        template<typename K, typename V>
        void del(const K &key, const V &val) {
          _dbi.del(key, val);
        };

        void drop(const bool del = false) {
          _dbi.drop(del);
        }

        lmdb::Cursor cursor() const {
          return _txn->openCursor(_dbi);
        }

      private:
        const LMDBTxn *_txn;
        lmdb::Dbi _dbi{};
        bool _append{false};
        bool _overwrite{true};
      };

    }
  }
}
