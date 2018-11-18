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

  Txn::Adapter::Adapter()
    : _dbInfo{nullptr}, _class{nullptr}, _property{nullptr}, _index{nullptr} {}

  Txn::Adapter::Adapter(const storage_engine::LMDBTxn *txn)
    : _dbInfo{new adapter::metadata::DBInfoAccess(txn)},
      _class{new adapter::schema::ClassAccess(txn)},
      _property{new adapter::schema::PropertyAccess(txn)},
      _index{new adapter::schema::IndexAccess(txn)} {}

  Txn::Adapter::~Adapter() noexcept {
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
  }

  Txn::Adapter::Adapter(Txn::Adapter &&other) noexcept {
    *this = std::move(other);
  }

  Txn::Adapter& Txn::Adapter::operator=(Txn::Adapter &&other) noexcept {
    if (this != &other) {
      delete _dbInfo;
      delete _class;
      delete _property;
      delete _index;

      _dbInfo = other._dbInfo;
      _class = other._class;
      _property = other._property;
      _index = other._index;

      other._dbInfo = nullptr;
      other._class = nullptr;
      other._property = nullptr;
      other._index = nullptr;
    }
    return *this;
  }

  Txn::Interface::Interface()
    : _txn{nullptr}, _schema{nullptr}, _record{nullptr}, _graph{nullptr}, _index{nullptr} {}


  Txn::Interface::Interface(const Txn *txn)
    : _txn{txn},
      _schema{new schema::SchemaInterface(_txn)},
      _record{new datarecord::DataRecordInterface(_txn)},
      _graph{new relation::GraphInterface(_txn)},
      _index{new index::IndexInterface(_txn)} {}

  Txn::Interface::~Interface() noexcept {
    _txn = nullptr;
    destroy();
  }

  Txn::Interface::Interface(nogdb::Txn::Interface &&other) noexcept {
    *this = std::move(other);
  }

  Txn::Interface& Txn::Interface::operator=(Txn::Interface &&other) noexcept {
    if (this != &other) {
      destroy();

      _txn = other._txn;
      init();

      other.destroy();
      other._txn = nullptr;
    }
    return *this;
  }

  void Txn::Interface::init() {
    _schema = new schema::SchemaInterface(_txn);
    _record = new datarecord::DataRecordInterface(_txn);
    _graph = new relation::GraphInterface(_txn);
    _index = new index::IndexInterface(_txn);
  }

  void Txn::Interface::destroy() {
    if (_schema) {
      delete _schema;
      _schema = nullptr;
    }
    if (_record) {
      delete _record;
      _record = nullptr;
    }
    if (_graph) {
      delete _graph;
      _graph = nullptr;
    }
    if (_index) {
      delete _index;
      _index = nullptr;
    }
  }

  Txn::Txn(Context &ctx, Mode mode)
    : _txnMode{mode}, _txnCtx{&ctx} {
    try {
      _txnBase = new storage_engine::LMDBTxn(
          _txnCtx->_envHandler.get(),
          (mode == READ_WRITE) ? storage_engine::lmdb::TXN_RW : storage_engine::lmdb::TXN_RO);
      _adapter = Adapter(_txnBase);
      _interface = Interface(this);
    } catch (const Error &err) {
      try { rollback(); } catch (...) {}
      throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
      try { rollback(); } catch (...) {}
      std::rethrow_exception(std::current_exception());
    }
  }

  Txn::~Txn() noexcept {
    try { rollback(); } catch (...) {}
  }

  Txn::Txn(Txn &&txn) noexcept {
    *this = std::move(txn);
  }

  Txn &Txn::operator=(Txn &&txn) noexcept {
    if (this != &txn) {
      try { rollback(); } catch (...) {}

      _txnCtx = txn._txnCtx;
      _txnMode = txn._txnMode;
      _txnBase = txn._txnBase;
      _adapter = std::move(txn._adapter);
      _interface = std::move(txn._interface);

      txn.rollback();
    }
    return *this;
  }

  void Txn::commit() {
    if (_txnBase) {
      try {
        _txnBase->commit();
        _txnBase = nullptr;
      } catch (const Error &err) {
        try { rollback(); } catch (...) {}
        throw NOGDB_FATAL_ERROR(err);
      } catch (...) {
        try { rollback(); } catch (...) {}
        std::rethrow_exception(std::current_exception());
      }
    } else {
      throw NOGDB_TXN_ERROR(NOGDB_TXN_COMPLETED);
    }
  }

  void Txn::rollback() noexcept {
    if (_txnBase) {
      _txnBase->rollback();
      _txnBase = nullptr;
    }
  }

}

