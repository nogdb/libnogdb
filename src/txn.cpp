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

#include <iostream> // for debugging

#include "shared_lock.hpp"
#include "graph.hpp"
#include "lmdb_engine.hpp"
#include "base_txn.hpp"

#include "nogdb_txn.h"

namespace nogdb {

    Txn::Txn(Context &ctx, Mode mode) : txnCtx{ctx}, txnMode{mode} {
        try {
            txnBase = std::make_shared<BaseTxn>(txnCtx, txnMode == Mode::READ_WRITE);
        } catch (const Error &err) {
            throw err;
        } catch (...) {
            std::rethrow_exception(std::current_exception());
        }
    }

    Txn::~Txn() noexcept {
        rollback();
    }

    Txn::Txn(const Txn &txn) : txnCtx{txn.txnCtx}, txnMode{txn.txnMode}, txnBase{txn.txnBase} {}

    Txn &Txn::operator=(const Txn &txn) {
        if (this != &txn) {
            auto tmp(txn);
            using std::swap;
            swap(tmp, *this);
        }
        return *this;
    }

    Txn::Txn(Txn &&txn) noexcept: txnCtx{txn.txnCtx}, txnMode{txn.txnMode}, txnBase{std::move(txn.txnBase)} {}

    Txn &Txn::operator=(Txn &&txn) noexcept {
        if (this != &txn) {
            txnCtx = txn.txnCtx;
            txnMode = txn.txnMode;
            txnBase = std::move(txn.txnBase);
        }
        return *this;
    }

    void Txn::commit() {
        try {
            txnBase->commit(txnCtx);
        } catch (const Error &err) {
            rollback();
            throw err;
        } catch (...) {
            rollback();
            std::rethrow_exception(std::current_exception());
        }
    }

    void Txn::rollback() noexcept {
        if (txnBase != nullptr) {
            txnBase->rollback(txnCtx);
        }
    }

    TxnId Txn::getTxnId() const {
        return txnBase->getTxnId();
    }

    TxnId Txn::getVersionId() const {
        return txnBase->getVersionId();
    }

}

