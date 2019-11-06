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

#include <memory>

#include "datarecord.hpp"
#include "dbinfo_adapter.hpp"
#include "index.hpp"
#include "lmdb_engine.hpp"
#include "relation.hpp"
#include "schema_adapter.hpp"

#include "nogdb/nogdb.h"

namespace nogdb {

Transaction::Adapter::Adapter()
    : _dbInfo { nullptr }
    , _class { nullptr }
    , _property { nullptr }
    , _index { nullptr }
{
}

Transaction::Adapter::Adapter(const storage_engine::LMDBTxn* txn)
    : _dbInfo { new adapter::metadata::DBInfoAccess(txn) }
    , _class { new adapter::schema::ClassAccess(txn) }
    , _property { new adapter::schema::PropertyAccess(txn) }
    , _index { new adapter::schema::IndexAccess(txn) }
{
}

Transaction::Adapter::~Adapter() noexcept
{
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

Transaction::Transaction(Context& ctx, const TxnMode& mode)
    : _txnMode { mode }
    , _txnCtx { &ctx }
{
    try {
        _txnBase = new storage_engine::LMDBTxn(
            _txnCtx->_envHandler,
            (mode == TxnMode::READ_WRITE) ? storage_engine::lmdb::TXN_RW : storage_engine::lmdb::TXN_RO);
        _adapter = new Adapter(_txnBase);
        _graph = new relation::GraphUtils(_txnBase, _txnCtx->_versionEnabled);
    } catch (const Error& err) {
        try {
            rollback();
        } catch (...) {
        }
        throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
        try {
            rollback();
        } catch (...) {
        }
        std::rethrow_exception(std::current_exception());
    }
}

Transaction::~Transaction() noexcept
{
    try {
        rollback();
    } catch (...) {
    }
}

Transaction::Transaction(Transaction&& txn) noexcept
    : _txnMode { txn._txnMode }
    , _txnCtx { txn._txnCtx }
    , _txnBase { txn._txnBase }
    , _adapter { txn._adapter }
    , _graph { txn._graph }
{
    txn._txnCtx = nullptr;
    txn._txnBase = nullptr;
    txn._adapter = nullptr;
    txn._graph = nullptr;
}

Transaction& Transaction::operator=(Transaction&& txn) noexcept
{
    if (this != &txn) {
        delete _txnBase;
        delete _adapter;
        delete _graph;

        _txnCtx = txn._txnCtx;
        _txnMode = txn._txnMode;
        _txnBase = txn._txnBase;
        _adapter = txn._adapter;
        _graph = txn._graph;

        txn._txnCtx = nullptr;
        txn._txnBase = nullptr;
        txn._adapter = nullptr;
        txn._graph = nullptr;
    }
    return *this;
}

void Transaction::commit()
{
    if (_txnBase) {
        try {
            _txnBase->commit();
            delete _txnBase;
            _txnBase = nullptr;
        } catch (const Error& err) {
            try {
                rollback();
            } catch (...) {
            }
            throw NOGDB_FATAL_ERROR(err);
        } catch (...) {
            try {
                rollback();
            } catch (...) {
            }
            std::rethrow_exception(std::current_exception());
        }
    } else {
        throw NOGDB_TXN_ERROR(NOGDB_TXN_COMPLETED);
    }
    if (_adapter) {
        delete _adapter;
        _adapter = nullptr;
    }
    if (_graph) {
        delete _graph;
        _graph = nullptr;
    }
}

void Transaction::rollback() noexcept
{
    if (_txnBase) {
        _txnBase->rollback();
        delete _txnBase;
        _txnBase = nullptr;
    }
    if (_adapter) {
        delete _adapter;
        _adapter = nullptr;
    }
    if (_graph) {
        delete _graph;
        _graph = nullptr;
    }
}

}
