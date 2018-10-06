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

#include <iostream> // for debugging

#include "lmdb_engine.hpp"
#include "dbinfo_adapter.hpp"
#include "schema_adapter.hpp"
#include "schema.hpp"
#include "index.hpp"
#include "datarecord.hpp"
#include "relation.hpp"

#include "nogdb_txn.h"

namespace nogdb {

  Txn::Txn(Context &ctx, Mode mode)
      : _txnCtx{ctx},
        _txnMode{mode},
        _completed{false} {
    try {
      _txnBase = new storage_engine::LMDBTxn(
          ctx._envHandler,
          (mode == READ_WRITE) ? storage_engine::lmdb::TXN_RW : storage_engine::lmdb::TXN_RO);
      _dbInfo = new adapter::metadata::DBInfoAccess{_txnBase};
      _class = new adapter::schema::ClassAccess{_txnBase};
      _property = new adapter::schema::PropertyAccess{_txnBase};
      _index = new adapter::schema::IndexAccess{_txnBase};
      _iSchema = new schema::SchemaInterface(this);
      _iIndex = new index::IndexInterface(this);
      _iGraph = new relation::GraphInterface(this);
      _iRecord = new datarecord::DataRecordInterface(this);
    } catch (const Error &err) {
      try { rollback(); } catch (...) {}
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      try { rollback(); } catch (...) {}
      std::rethrow_exception(std::current_exception());
    }
  }

  Txn::~Txn() noexcept {
    if (_txnBase) {
      try { rollback(); } catch (...) {}
    }
    if (_dbInfo) {
      delete _dbInfo;
      _dbInfo = nullptr;
    }
    if (_class) {
      delete _class;
      _class = nullptr;
    }
    if (_property) {
      delete _property;
      _property = nullptr;
    }
    if (_index) {
      delete _index;
      _index = nullptr;
    }
    if (_iSchema) {
      delete _iSchema;
      _iSchema = nullptr;
    }
    if (_iIndex) {
      delete _iIndex;
      _iIndex = nullptr;
    }
    if (_iGraph) {
      delete _iGraph;
      _iGraph = nullptr;
    }
    if (_iRecord) {
      delete _iRecord;
      _iRecord = nullptr;
    }
  }

  Txn::Txn(Txn &&txn) noexcept
      : _txnCtx{txn._txnCtx} {
    *this = std::move(txn);
  }

  Txn &Txn::operator=(Txn &&txn) noexcept {
    if (this != &txn) {
      delete _txnBase;
      _txnCtx = std::move(txn._txnCtx);
      _txnMode = txn._txnMode;
      _txnBase = txn._txnBase;
      _completed = txn._completed;
      _dbInfo = txn._dbInfo;
      _class = txn._class;
      _property = txn._property;
      _index = txn._index;
      _iSchema = txn._iSchema;
      _iIndex = txn._iIndex;
      _iGraph = txn._iGraph;
      _iRecord = txn._iRecord;
      txn._txnBase = nullptr;
      txn._dbInfo = nullptr;
      txn._class = nullptr;
      txn._property = nullptr;
      txn._index = nullptr;
      txn._iSchema = nullptr;
      txn._iIndex = nullptr;
      txn._iGraph = nullptr;
      txn._iRecord = nullptr;
      txn._completed = true;
    }
    return *this;
  }

  void Txn::commit() const {
    try {
      _txnBase->commit();
      delete _txnBase;
      _txnBase = nullptr;
      _completed = true;
    } catch (const Error &err) {
      try { rollback(); } catch (...) {}
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      try { rollback(); } catch (...) {}
      std::rethrow_exception(std::current_exception());
    }
  }

  void Txn::rollback() const noexcept {
    if (_txnBase) {
      _txnBase->rollback();
      delete _txnBase;
      _txnBase = nullptr;
      _completed = true;
    }
  }

}

