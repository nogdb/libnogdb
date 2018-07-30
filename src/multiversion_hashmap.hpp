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

#ifndef __MULTIVERSION_HASHMAP_HPP_INCLUDED_
#define __MULTIVERSION_HASHMAP_HPP_INCLUDED_

#include <utility>
#include <map>
#include <unordered_map>
#include <set>
#include <vector>

#include "spinlock.hpp"
#include "version_control.hpp"

namespace nogdb {

    template<typename Key, typename T>
    class MultiVersionHashMap {
    public:
        typedef std::unordered_map<Key, std::shared_ptr<VersionControl<T>>> InnerHashMap;

        MultiVersionHashMap() = default;

        MultiVersionHashMap(const MultiVersionHashMap &) = delete;

        MultiVersionHashMap &operator=(const MultiVersionHashMap &) = delete;

        std::weak_ptr<VersionControl<T>> insert(const Key &key, const T &object) {
            RWSpinLockGuard<RWSpinLock> _(spinlock_, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            auto iter = hashMap_.find(key);
            if (iter == hashMap_.cend()) {
                auto objectPtr = std::make_shared<VersionControl<T>>();
                objectPtr->addLatestVersion(object);
                hashMap_.emplace(key, objectPtr);
                return objectPtr;
            } else {
                iter->second->addLatestVersion(object);
                return iter->second;
            }
        }

        std::pair<T, bool> find(const Key &key) const {
            RWSpinLockGuard<RWSpinLock> _(spinlock_);
            auto iter = hashMap_.find(key);
            if (iter == hashMap_.cend()) {
                return std::make_pair(T{}, false);
            }
            return iter->second->getLatestVersion();
        }

        std::pair<T, bool> find(TxnId refTxnId, const Key &key) const {
            RWSpinLockGuard<RWSpinLock> _(spinlock_);
            auto iter = hashMap_.find(key);
            if (iter == hashMap_.cend()) {
                return std::make_pair(T{}, false);
            }
            return iter->second->getStableVersion(refTxnId);
        }

        std::shared_ptr<VersionControl<T>> get(const Key &key) const {
            RWSpinLockGuard<RWSpinLock> _(spinlock_);
            auto iter = hashMap_.find(key);
            if (iter == hashMap_.cend()) {
                return nullptr;
            } else {
                return iter->second;
            }
        }

        void erase(const Key &key) {
            RWSpinLockGuard<RWSpinLock> _(spinlock_, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            auto iter = hashMap_.find(key);
            if (iter != hashMap_.cend()) {
                iter->second->deleteLatestVersion();
            }
        }

        size_t clear(const Key &key, TxnId baseTxnId) {
            RWSpinLockGuard<RWSpinLock> _(spinlock_, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            auto iter = hashMap_.find(key);
            if (iter != hashMap_.cend()) {
                if (baseTxnId == 0) {
                    if (iter->second->clearUnstableVersion() == 0) {
                        hashMap_.erase(iter);
                    }
                } else {
                    if (iter->second->clearStableVersion(baseTxnId) == 0) {
                        hashMap_.erase(iter);
                    }
                }
            }
            return hashMap_.size();
        }

        size_t clear(TxnId baseTxnId) {
            RWSpinLockGuard<RWSpinLock> _(spinlock_, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            for (auto it = hashMap_.begin(); it != hashMap_.end();) {
                if (it->second->clearStableVersion(baseTxnId) == 0) {
                    it = hashMap_.erase(it);
                } else {
                    ++it;
                }
            }
            return hashMap_.size();
        }

        std::vector<Key> keys() const {
            RWSpinLockGuard<RWSpinLock> _(spinlock_);
            auto keys = std::vector<Key> {};
            for (const auto &element: hashMap_) {
                keys.push_back(element.first);
            }
            return keys;
        }


    private:
        mutable RWSpinLock spinlock_{};
        InnerHashMap hashMap_{};
    };

    template<typename FirstKeyT, typename SecondKeyT, typename T>
    class TwoLevelMultiVersionHashMap {
    public:
        typedef MultiVersionHashMap<SecondKeyT, T> MultiVersionHashMapT;
        typedef std::unordered_map<FirstKeyT, std::unique_ptr<MultiVersionHashMapT>> OuterHashMap;

        TwoLevelMultiVersionHashMap() = default;

        TwoLevelMultiVersionHashMap(const TwoLevelMultiVersionHashMap &) = delete;

        TwoLevelMultiVersionHashMap &operator=(const TwoLevelMultiVersionHashMap &) = delete;

        std::weak_ptr<VersionControl<T>> insert(const FirstKeyT &key1, const SecondKeyT &key2, const T &object) {
            RWSpinLockGuard<RWSpinLock> _(spinlock_, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            auto iter = outerHashMap_.find(key1);
            if (iter == outerHashMap_.cend()) {
                auto multiVersionHashMap = std::unique_ptr<MultiVersionHashMapT>(new MultiVersionHashMapT{});
                auto objectPtr = multiVersionHashMap->insert(key2, object);
                outerHashMap_.emplace(key1, std::move(multiVersionHashMap));
                return objectPtr;
            } else {
                return iter->second->insert(key2, object);
            }
        }

        std::pair<T, bool> find(const FirstKeyT &key1, const SecondKeyT &key2) const {
            RWSpinLockGuard<RWSpinLock> _(spinlock_);
            auto iter = outerHashMap_.find(key1);
            if (iter == outerHashMap_.cend()) {
                return std::make_pair(T{}, false);
            }
            return iter->second->find(key2);
        }

        std::pair<T, bool> find(TxnId refTxnId, const FirstKeyT &key1, const SecondKeyT &key2) const {
            RWSpinLockGuard<RWSpinLock> _(spinlock_);
            auto iter = outerHashMap_.find(key1);
            if (iter == outerHashMap_.cend()) {
                return std::make_pair(T{}, false);
            }
            return iter->second->find(refTxnId, key2);
        }

        std::shared_ptr<VersionControl<T>> get(const FirstKeyT &key1, const SecondKeyT &key2) const {
            RWSpinLockGuard<RWSpinLock> _(spinlock_);
            auto iter = outerHashMap_.find(key1);
            if (iter == outerHashMap_.cend()) {
                return nullptr;
            } else {
                return iter->second->get(key2);
            }
        }

        void erase(const FirstKeyT &key1, const SecondKeyT &key2) {
            RWSpinLockGuard<RWSpinLock> _(spinlock_, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            auto iter = outerHashMap_.find(key1);
            if (iter != outerHashMap_.cend()) {
                iter->second->erase(key2);
            }
        }

        void clear(const FirstKeyT &key1, const SecondKeyT &key2, TxnId baseTxnId) {
            RWSpinLockGuard<RWSpinLock> _(spinlock_, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            auto iter = outerHashMap_.find(key1);
            if (iter != outerHashMap_.cend()) {
                if (iter->second->clear(key2, baseTxnId) == 0) {
                    outerHashMap_.erase(iter);
                }
            }
        }

        void clear(TxnId baseTxnId) {
            RWSpinLockGuard<RWSpinLock> _(spinlock_, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            for (auto it = outerHashMap_.begin(); it != outerHashMap_.end();) {
                if (it->second->clear(baseTxnId) == 0) {
                    it = outerHashMap_.erase(it);
                } else {
                    ++it;
                }
            }
        }

        std::map<FirstKeyT, std::vector<SecondKeyT>> keys() const {
            RWSpinLockGuard<RWSpinLock> _(spinlock_);
            auto keys = std::map<FirstKeyT, std::vector<SecondKeyT>> {};
            for (const auto &outerElement: outerHashMap_) {
                for (const auto &elementKey: outerElement.second->keys()) {
                    keys[outerElement.first].push_back(elementKey);
                }
            }
            return keys;
        }

        std::vector<SecondKeyT> keys(const FirstKeyT &firstKey) const {
            RWSpinLockGuard<RWSpinLock> _(spinlock_);
            auto findElement = outerHashMap_.find(firstKey);
            if (findElement != outerHashMap_.cend()) {
                return findElement->second->keys();
            }
            return std::vector<SecondKeyT> {};
        }

    private:
        mutable RWSpinLock spinlock_{};
        OuterHashMap outerHashMap_{};
    };

}

#endif
