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

#ifndef __CONCURRENT_HPP_INCLUDED_
#define __CONCURRENT_HPP_INCLUDED_

#include <map>
#include <deque>
#include <vector>

#include "spinlock.hpp"

#include "nogdb_types.h"

namespace nogdb {

    template<typename Collection, typename Key, typename T>
    struct ConcurrentHashMap {
        void lockAndErase(const Key &key) {
            RWSpinLockGuard<RWSpinLock> _(splock, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            elements.erase(key);
        }

        void lockAndClear() {
            RWSpinLockGuard<RWSpinLock> _(splock, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            elements.clear();
        }

        void lockAndEmplace(const Key &key, const std::shared_ptr<T> &element) {
            RWSpinLockGuard<RWSpinLock> _(splock, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            elements.emplace(key, element);
        }

        RWSpinLock splock{};
        Collection elements;
    };

    template<typename T>
    using DeleteQueue = std::deque<std::pair<T, TxnId>>;

    template<typename T>
    struct ConcurrentDeleteQueue {
        std::vector<T> pop_front(TxnId versionId) {
            auto result = std::vector<T>{};
            RWSpinLockGuard<RWSpinLock> _(splock, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            while (!elements.empty() && elements.front().second <= versionId) {
                result.emplace_back(elements.front().first);
                elements.pop_front();
            }
            return result;
        }

        void push_back(const DeleteQueue<T> &deleteQueue) {
            RWSpinLockGuard<RWSpinLock> _(splock, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            elements.insert(elements.end(), deleteQueue.begin(), deleteQueue.end());
        }

        RWSpinLock splock{};
        DeleteQueue<T> elements;
    };

}


#endif