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

#ifndef __ENV_HANDLER_HPP_INCLUDED_
#define __ENV_HANDLER_HPP_INCLUDED_

#include "lmdb_interface.hpp"

namespace nogdb {

    class EnvHandlerPtr;

    class EnvHandler {
    public:
        friend class EnvHandlerPtr;

        static EnvHandlerPtr create(
                const std::string &dbPath,
                unsigned int maxdb,
                unsigned long maxdbSize,
                unsigned int maxdbReaders,
                LMDBInterface::LMDBFlag flag,
                LMDBInterface::LMDBMode perm
        );

    private:
        EnvHandler() : env{nullptr}, refCount{0}, lockFileDescriptor{-1} {}

        EnvHandler(
                const std::string &dbPath,
                unsigned int maxdb,
                unsigned long maxdbSize,
                unsigned int maxdbReaders,
                LMDBInterface::LMDBFlag flag,
                LMDBInterface::LMDBMode perm
        );

        LMDBInterface::EnvHandler *env;
        unsigned int refCount;
        int lockFileDescriptor;
    };

    class EnvHandlerPtr {
    public:
        EnvHandler *operator->() {
            return pointer_;
        }

        EnvHandler &operator*() {
            return *pointer_;
        }

        EnvHandlerPtr() : pointer_{new(std::nothrow) EnvHandler()} {
            ++pointer_->refCount;
        }

        EnvHandlerPtr(EnvHandler *pointer) : pointer_{pointer} {
            ++pointer_->refCount;
        }

        ~EnvHandlerPtr();

        EnvHandlerPtr(const EnvHandlerPtr &pointer) : pointer_{pointer.pointer_} {
            ++pointer_->refCount;
        }

        EnvHandlerPtr &operator=(const EnvHandlerPtr &p);

        LMDBInterface::EnvHandler *get() const {
            return pointer_->env;
        }

    private:
        EnvHandler *pointer_;
    };

}

#endif
