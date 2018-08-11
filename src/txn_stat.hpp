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

#ifndef __TXN_STAT_HPP_INCLUDED_
#define __TXN_STAT_HPP_INCLUDED_

#include <atomic>

#include "nogdb/nogdb_types.h"

namespace nogdb {
    class TxnStat {
    public:
        TxnStat() = default;

        virtual ~TxnStat() noexcept = default;

        TxnId fetchAddMaxTxnId() {
            return maxTxnId.fetch_add(static_cast<TxnId>(1), std::memory_order_relaxed);
        }

        TxnId fetchAddMaxVersionId() {
            return maxVersionId.fetch_add(static_cast<TxnId>(1), std::memory_order_relaxed);
        }

        TxnId getMaxTxnId() const {
            return maxTxnId;
        }

        TxnId getMaxVersionId() const {
            return maxVersionId;
        }

    private:
        std::atomic<TxnId> maxTxnId{1};
        std::atomic<TxnId> maxVersionId{0};
    };
}

#endif //__TXN_STAT_HPP_INCLUDED_
