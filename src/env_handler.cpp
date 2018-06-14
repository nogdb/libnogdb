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

#include <iomanip>
#include <sys/file.h>
#include <sys/stat.h>

#include "utils.hpp"
#include "constant.hpp"
#include "env_handler.hpp"

namespace nogdb {

    EnvHandlerPtr
    EnvHandler::create(const std::string &dbPath,
                       unsigned int dbNum,
                       unsigned long dbSize,
                       unsigned int dbReaders,
                       LMDBInterface::LMDBFlag flag,
                       LMDBInterface::LMDBMode perm) {
        return new(std::nothrow) EnvHandler(dbPath, dbNum, dbSize, dbReaders, flag, perm);
    }

    EnvHandler::EnvHandler(const std::string &dbPath,
                           unsigned int maxdb,
                           unsigned long maxdbSize,
                           unsigned int maxdbReaders,
                           LMDBInterface::LMDBFlag flag,
                           LMDBInterface::LMDBMode perm) : refCount{0} {
        if (!fileExists(dbPath)) {
            mkdir(dbPath.c_str(), 0755);
        }
        const auto lockFile = dbPath + DB_LOCK_FILE;
        lockFileDescriptor = open(lockFile.c_str(), O_CREAT | O_RDONLY, 0644);
        if (flock(lockFileDescriptor, LOCK_EX | LOCK_NB) == -1) {
            if (errno == EWOULDBLOCK) {
                throw Error(NOGDB_CTX_IS_LOCKED, Error::Type::CONTEXT);
            } else {
                throw Error(NOGDB_CTX_UNKNOWN_ERR, Error::Type::CONTEXT);
            }
        }

        try {
            env = LMDBInterface::createEnv(dbPath, maxdb, maxdbSize, maxdbReaders, flag, perm);
        } catch (LMDBInterface::ErrorType &err) {
            throw Error(err, Error::Type::DATASTORE);
        }
    }

    EnvHandlerPtr &
    EnvHandlerPtr::operator=(const EnvHandlerPtr &ptr) {
        if (this != &ptr) {
            ++ptr.pointer_->refCount;
            if (--pointer_->refCount == 0) {
                if (pointer_->env != nullptr) {
                    LMDBInterface::destroyEnv(pointer_->env);
                    flock(pointer_->lockFileDescriptor, LOCK_UN);
                }
                delete pointer_;
            }
            pointer_ = ptr.pointer_;
        }
        return *this;
    }

    EnvHandlerPtr::~EnvHandlerPtr() {
        if (--pointer_->refCount == 0) {
            if (pointer_->env != nullptr) {
                LMDBInterface::destroyEnv(pointer_->env);
                flock(pointer_->lockFileDescriptor, LOCK_UN);
            }
            delete pointer_;
        }
    }

}
