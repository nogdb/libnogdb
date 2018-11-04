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
#include <memory>

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
      _txnBase = std::make_shared<storage_engine::LMDBTxn>(
          ctx._envHandler.get(),
          (mode == READ_WRITE) ? storage_engine::lmdb::TXN_RW : storage_engine::lmdb::TXN_RO);
      _dbInfo = std::make_shared<adapter::metadata::DBInfoAccess>(_txnBase.get());
      _class = std::make_shared<adapter::schema::ClassAccess>(_txnBase.get());
      _property = std::make_shared<adapter::schema::PropertyAccess>(_txnBase.get());
      _index = std::make_shared<adapter::schema::IndexAccess>(_txnBase.get());
      _iSchema = std::make_shared<schema::SchemaInterface>(this);
      _iIndex = std::make_shared<index::IndexInterface>(this);
      _iGraph = std::make_shared<relation::GraphInterface>(this);
      _iRecord = std::make_shared<datarecord::DataRecordInterface>(this);
    } catch (const Error &err) {
      try { rollback(); } catch (...) {}
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      try { rollback(); } catch (...) {}
      std::rethrow_exception(std::current_exception());
    }
  }

  Txn::Txn(const Txn &txn)
      : _txnCtx{txn._txnCtx},
      _txnBase{txn._txnBase},
      _dbInfo{txn._dbInfo},
      _class{txn._class},
      _property{txn._property},
      _index{txn._index},
      _iSchema{txn._iSchema},
      _iIndex{txn._iIndex},
      _iGraph{txn._iGraph},
      _iRecord{txn._iRecord},
      _txnMode{txn._txnMode},
      _completed{txn._completed} {}

  Txn &Txn::operator=(const Txn &txn) {
    if (this != &txn) {
      auto tmp(txn);
      using std::swap;
      swap(tmp, *this);
    }
    return *this;
  }

  Txn::Txn(Txn &&txn) noexcept
      : _txnCtx{txn._txnCtx},
        _txnMode{txn._txnMode},
        _completed{txn._completed} {
    _txnBase = std::move(txn._txnBase);
    _dbInfo = std::move(txn._dbInfo);
    _class = std::move(txn._class);
    _property = std::move(txn._property);
    _index = std::move(txn._index);
    _iSchema = std::move(txn._iSchema);
    _iIndex = std::move(txn._iIndex);
    _iGraph = std::move(txn._iGraph);
    _iRecord = std::move(txn._iRecord);
  }

  Txn &Txn::operator=(Txn &&txn) noexcept {
    if (this != &txn) {
      auto tmp(std::move(txn));
      using std::swap;
      swap(tmp, *this);
    }
    return *this;
  }

  void Txn::commit() const {
    try {
      _txnBase->commit();
    } catch (const Error &err) {
      try { rollback(); } catch (...) {}
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      try { rollback(); } catch (...) {}
      std::rethrow_exception(std::current_exception());
    }
    _completed = true;
  }

  void Txn::rollback() const noexcept {
    if (!_completed) {
      _txnBase->rollback();
      _completed = true;
    }
  }

}

