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

#ifndef __SHARED_LOCK_HPP_INCLUDED_
#define __SHARED_LOCK_HPP_INCLUDED_

#include "boost/shared_mutex.hpp"

namespace nogdb {

    template<typename mutex_t>
    class WriteLock {
    public:
        explicit WriteLock(mutex_t &mutex) : mutex_{mutex} {
            mutex_.lock();
        }

        ~WriteLock() {
            mutex_.unlock();
        }

    private:
        mutex_t &mutex_;

        WriteLock(const WriteLock &) = delete;

        WriteLock &operator=(const WriteLock &) = delete;
    };

    template<typename mutex_t>
    class ReadLock {
    public:
        explicit ReadLock(mutex_t &mutex) : mutex_{mutex} {
            mutex_.lock_shared();
        }

        ~ReadLock() {
            mutex_.unlock_shared();
        }

    private:
        mutex_t &mutex_;

        ReadLock(const ReadLock &) = delete;

        ReadLock &operator=(const ReadLock &) = delete;
    };


}

#endif
