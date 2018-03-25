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

#ifndef __SPINLOCK_HPP_INCLUDED_
#define __SPINLOCK_HPP_INCLUDED_

#include <atomic>
#include <thread>

namespace nogdb {

    class SpinLock {
    public:
        SpinLock() = default;

        void acquireLock() {
            while (lock.test_and_set(std::memory_order_acquire)) {
                asm volatile("pause\n": : :"memory");
            }
        }

        void releaseLock() {
            lock.clear(std::memory_order_release);
        }

    private:
        std::atomic_flag lock = ATOMIC_FLAG_INIT;
    };

    template<typename SpinLock>
    class SpinLockGuard {
    public:
        explicit SpinLockGuard(SpinLock &spinLock) : spinLock_{spinLock} {
            spinLock_.acquireLock();
        }

        ~SpinLockGuard() {
            spinLock_.releaseLock();
        }

        SpinLockGuard(const SpinLockGuard &) = delete;

        SpinLockGuard &operator=(const SpinLockGuard &) = delete;

    private:
        SpinLock &spinLock_;
    };

#define SPINLOCK_MAXCOUNT_DELAY        1000

    class RWSpinLock {
    public:
        RWSpinLock() = default;

        RWSpinLock(const RWSpinLock &) = delete;

        RWSpinLock &operator=(const RWSpinLock &) = delete;

        void lock() {
            auto delayCount = 0U;
            while (!tryLock()) {
                if (++delayCount > SPINLOCK_MAXCOUNT_DELAY) {
                    std::this_thread::yield(); //TODO: should be removed?
                }
            }
            delayCount = 0U;
            while (numOfReaders.load(std::memory_order_acquire) > 0) {
                if (++delayCount > SPINLOCK_MAXCOUNT_DELAY) {
                    std::this_thread::yield(); //TODO: should be removed?
                }
            }
        }

        bool tryLock() {
            auto doWriting = false;
            return isWriting.compare_exchange_strong(doWriting, true, std::memory_order_acq_rel);
        }

        void unlock() {
            isWriting.store(false, std::memory_order_release);
        }

        void lockShared() {
            auto delayCount = 0U;
            while (!tryLockShared()) {
                if (++delayCount > SPINLOCK_MAXCOUNT_DELAY) {
                    std::this_thread::yield(); //TODO: should be removed?
                }
            }
        }

        bool tryLockShared() {
            auto delayCount = 0U;
            while (isWriting.load(std::memory_order_acquire)) {
                if (++delayCount > SPINLOCK_MAXCOUNT_DELAY) {
                    std::this_thread::yield(); //TODO: should be removed?
                }
            }
            numOfReaders.fetch_add(1U, std::memory_order_acquire);
            if (isWriting.load(std::memory_order_acquire)) {
                numOfReaders.fetch_sub(1U, std::memory_order_release);
                return false;
            }
            return true;
        }

        void unlockShared() {
            numOfReaders.fetch_sub(1U, std::memory_order_release);
        }

    private:
        std::atomic<unsigned int> numOfReaders{0};
        std::atomic<bool> isWriting{false};
    };

    enum RWSpinLockMode {
        SHARED_SPLOCK = 0, EXCLUSIVE_SPLOCK = 1
    };

    template<typename RWSpinLock>
    class RWSpinLockGuard {
    public:
        explicit RWSpinLockGuard(RWSpinLock &spinlock, RWSpinLockMode mode = SHARED_SPLOCK)
                : mode_{mode}, spinlock_{spinlock} {
            if (mode_ == SHARED_SPLOCK) {
                spinlock_.lockShared();
            } else {
                spinlock_.lock();
            }
        }

        ~RWSpinLockGuard() {
            if (mode_ == SHARED_SPLOCK) {
                spinlock_.unlockShared();
            } else {
                spinlock_.unlock();
            }
        }

        RWSpinLockGuard(const RWSpinLockGuard &) = delete;

        RWSpinLockGuard &operator=(const RWSpinLockGuard &) = delete;

    private:
        RWSpinLockMode mode_;
        RWSpinLock &spinlock_;
    };

}

#endif
