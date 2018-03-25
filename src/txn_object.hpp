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

#ifndef __TXN_OBJECT_HPP_INCLUDED_
#define __TXN_OBJECT_HPP_INCLUDED_

#include <atomic>
#include <set>
#include <utility>

#include "spinlock.hpp"

#include "nogdb_types.h"

namespace nogdb {

    class TxnObject {
    public:
        enum StatusFlag {
            UNCOMMITTED_CREATE = 00,
            COMMITTED_CREATE = 01,
            UNCOMMITTED_DELETE = 02,
            COMMITTED_DELETE = 03
        };

        struct State {
            State() : versionId{0}, status{UNCOMMITTED_CREATE} {}

            State(TxnId versionId_, StatusFlag status_) : versionId{versionId_}, status{status_} {}

            TxnId versionId;
            StatusFlag status;
        };

        typedef std::atomic<State> AtomicState;
        typedef std::atomic<TxnId> AtomicTxnId;

        StatusFlag updateState(TxnId commitId) { // atomic guarantee because only one writer can write at a time
            State prevState = state;
            if (prevState.status == StatusFlag::UNCOMMITTED_CREATE) {
                State newState{commitId, StatusFlag::COMMITTED_CREATE};
                while (!state.compare_exchange_weak(prevState, newState));
                return StatusFlag::COMMITTED_CREATE;
            } else if (prevState.status == StatusFlag::UNCOMMITTED_DELETE) {
                State newState{commitId, StatusFlag::COMMITTED_DELETE};
                while (!state.compare_exchange_weak(prevState, newState));
                return StatusFlag::COMMITTED_DELETE;
            }
            return prevState.status;
        }

        void setStatus(StatusFlag status) { // atomic guarantee because only one writer can write at a time
            State prevState = state;
            State newState{prevState.versionId, status};
            while (!state.compare_exchange_weak(prevState, newState));
        }

        std::pair<TxnId, StatusFlag> getState() const {
            State currentState = state;
            return std::make_pair(currentState.versionId, currentState.status);
        }

        // return true if not visible, otherwise false
        bool checkReadOnly(TxnId versionId) const {
            State currentState = state;
            return (currentState.status == StatusFlag::UNCOMMITTED_CREATE ||
                    (currentState.status == StatusFlag::COMMITTED_DELETE && versionId >= currentState.versionId) ||
                    (currentState.status == StatusFlag::COMMITTED_CREATE && versionId < currentState.versionId));
        }

        // return true if not visible, otherwise false
        bool checkReadWrite() const {
            State currentState = state;
            return currentState.status == StatusFlag::UNCOMMITTED_DELETE ||
                   currentState.status == StatusFlag::COMMITTED_DELETE;
        }

    protected:
        TxnObject() = default;

        virtual ~TxnObject() noexcept = default;

    private:
        AtomicState state{};
    };

    struct TxnStat {
        TxnId fetchAddMaxTxnId() {
            return maxTxnId.fetch_add(static_cast<TxnId>(1), std::memory_order_relaxed);
        }

        TxnId fetchAddMaxVersionId() {
            return maxVersionId.fetch_add(static_cast<TxnId>(1), std::memory_order_relaxed);
        }

        void addActiveTxnId(TxnId txnId, TxnId versionId) {
            SpinLockGuard<SpinLock> _(lockActiveTxnIds);
            activeTxnIds.emplace(txnId, versionId);
        }

        void removeActiveTxnId(TxnId txnId) {
            SpinLockGuard<SpinLock> _(lockActiveTxnIds);
            activeTxnIds.erase(txnId);
        }

        std::pair<TxnId, TxnId> minActiveTxnId() {
            SpinLockGuard<SpinLock> _(lockActiveTxnIds);
            auto iter = activeTxnIds.cbegin();
            if (iter == activeTxnIds.cend()) {
                return std::make_pair(TxnId{0}, TxnId{0});
            }
            return std::make_pair(iter->first, iter->second);
        }

        bool isLastMinVersionId(TxnId txnId) {
            SpinLockGuard<SpinLock> _(lockActiveTxnIds);
            auto iter = activeTxnIds.find(txnId);
            if (iter == activeTxnIds.cbegin()) {
                auto version = iter->second;
                if (++iter != activeTxnIds.cend()) {
                    return version < iter->second;
                }
                return true;
            }
            return false;
        }

        TxnObject::AtomicTxnId maxTxnId{1};
        TxnObject::AtomicTxnId maxVersionId{0};
        SpinLock lockActiveTxnIds{};
        std::map<TxnId, TxnId> activeTxnIds{};
    };

}

#endif
