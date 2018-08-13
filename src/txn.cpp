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

#include "shared_lock.hpp"
#include "graph.hpp"
#include "lmdb_engine.hpp"
#include "dbinfo_adapter.hpp"
#include "schema_adapter.hpp"

#include "nogdb_txn.h"

namespace nogdb {

    Txn::Txn(Context &ctx, Mode mode)
            : _txnCtx{ctx},
              _txnMode{mode},
              _txnId{ctx.dbTxnStat->fetchAddMaxTxnId()},
//              _txnId{(mode == READ_WRITE)? 0: ctx.dbTxnStat->fetchAddMaxTxnId()},
              _completed{false} {
        try {
            _versionId = (mode == READ_WRITE)? ctx.getMaxVersionId() + 1: ctx.dbTxnStat->getMaxVersionId();
            if (_versionId == std::numeric_limits<TxnId>::max()) {
                throw NOGDB_TXN_ERROR(NOGDB_TXN_VERSION_MAXREACH);
            }
            _txnBase = new storage_engine::LMDBTxn(
                    ctx.envHandler.get(),
                    (mode == READ_WRITE)? storage_engine::lmdb::TXN_RW: storage_engine::lmdb::TXN_RO
            );
            _dbInfo = adapter::metadata::DBInfoAccess{_txnBase};
            _class = adapter::schema::ClassAccess{_txnBase};
            _property = adapter::schema::PropertyAccess{_txnBase};
            _index = adapter::schema::IndexAccess{_txnBase};
        } catch (const Error &err) {
            throw err;
        } catch (...) {
            std::rethrow_exception(std::current_exception());
        }
    }

    Txn::~Txn() noexcept {
        if (_txnBase) {
            try { rollback(); } catch (...) {}
        }
    }

    Txn::Txn(Txn &&txn) noexcept
            : _txnCtx{txn._txnCtx} {
        *this = std::move(txn);
    }

    Txn &Txn::operator=(Txn &&txn) noexcept {
        if (this != &txn) {
            delete _txnBase;
            _txnCtx = txn._txnCtx;
            _txnMode = txn._txnMode;
            _txnId = txn._txnId;
            _versionId = txn._versionId;
            _txnBase = txn._txnBase;
            _completed = txn._completed;
            _dbInfo = std::move(txn._dbInfo);
            _class = std::move(txn._class);
            _property = std::move(txn._property);
            _index = std::move(txn._index);
            txn._txnBase = nullptr;
            txn._completed = true;
        }
        return *this;
    }

    void Txn::commit() {
        try {
            _txnBase->commit();
            delete _txnBase;
            _txnCtx.dbTxnStat->fetchAddMaxVersionId();
            _txnBase = nullptr;
            _completed = true;
        } catch (const Error &err) {
            rollback();
            throw err;
        } catch (...) {
            rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

    void Txn::rollback() noexcept {
        if (_txnBase) {
            _txnBase->rollback();
            delete _txnBase;
            _txnBase = nullptr;
            _completed = true;
        }
    }

}

