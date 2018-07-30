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

#ifndef __VERSION_CONTROL_HPP_INCLUDED_
#define __VERSION_CONTROL_HPP_INCLUDED_

#include <set>
#include <utility>

#include "constant.hpp"
#include "txn_object.hpp"
#include "spinlock.hpp"

namespace nogdb {

    template<typename T>
    class VersionControl {
    public:
        enum Status {
            INACTIVE = 0, ACTIVE = 1
        };
        enum Visibility {
            INVISIBLE = 0, VISIBLE = 1
        };

        struct ControlObject {
            ControlObject() = default;

            ControlObject(const T &object_) : object{object_} {}

            TxnId versionId{0};
            Status status{ACTIVE};
            T object{};
        };

        VersionControl() = default;

        VersionControl(const VersionControl &) = delete;

        VersionControl &operator=(const VersionControl &) = delete;

        void addLatestVersion(const T &object) {
            unstableVersion_ = std::make_pair(ControlObject{object}, VISIBLE);
        }

        void deleteLatestVersion() {
            if (unstableVersion_.second == INVISIBLE) {
                RWSpinLockGuard<RWSpinLock> _(spinlock_);
                auto it = stableVersions_.rbegin();
                if (it != stableVersions_.crend()) {
                    unstableVersion_ = std::make_pair(ControlObject{it->object}, VISIBLE);
                }
            }
            unstableVersion_.first.status = INACTIVE;
        }

        std::pair<T, bool> getLatestVersion() const {
            if (unstableVersion_.second == VISIBLE) {
                if (unstableVersion_.first.status == ACTIVE) {
                    return std::make_pair(unstableVersion_.first.object, true);
                }
            } else {
                RWSpinLockGuard<RWSpinLock> _(spinlock_);
                auto it = stableVersions_.rbegin();
                if (it != stableVersions_.crend()) {
                    if (it->status == ACTIVE) {
                        return std::make_pair(it->object, true);
                    }
                }
            }
            return std::make_pair(T{}, false);
        }

        std::pair<T, bool> getUnstableVersion() const {
            if (unstableVersion_.second == VISIBLE) {
                if (unstableVersion_.first.status == ACTIVE) {
                    return std::make_pair(unstableVersion_.first.object, true);
                }
            }
            return std::make_pair(T{}, false);
        }

        std::pair<T, bool> getStableVersion() const {
            RWSpinLockGuard<RWSpinLock> _(spinlock_);
            auto it = stableVersions_.rbegin();
            if (it != stableVersions_.rend()) {
                return std::make_pair(it->object, it->status == ACTIVE);
            }
            return std::make_pair(T{}, false);
        };

        std::pair<T, bool> getStableVersion(TxnId currentVersionId) const {
            RWSpinLockGuard<RWSpinLock> _(spinlock_);
            for (auto it = stableVersions_.rbegin(); it != stableVersions_.rend(); ++it) {
                if (it->versionId <= currentVersionId) {
                    if (it->status == ACTIVE) {
                        return std::make_pair(it->object, true);
                    } else {
                        return std::make_pair(T{}, false);
                    }
                }
            }
            return std::make_pair(T{}, false);
        }

        size_t clearStableVersion(TxnId baseVersionId) {
            RWSpinLockGuard<RWSpinLock> _(spinlock_, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            if (stableVersions_.size() > 0) {
                for (auto it = stableVersions_.begin(); it != stableVersions_.end() - 1;) {
                    if (it->versionId < baseVersionId) {
                        it = stableVersions_.erase(it);
                    } else {
                        break; //++it;
                    }
                }
                if (stableVersions_.size() == 1 &&
                    (stableVersions_.cbegin())->versionId < baseVersionId &&
                    (stableVersions_.cbegin())->status == INACTIVE) {
                    stableVersions_.clear();
                }
            }
            return stableVersions_.size() + ((unstableVersion_.second == VISIBLE) ? 1 : 0);
        }

        size_t clearUnstableVersion() {
            disableUnstableVersion();
            RWSpinLockGuard<RWSpinLock> _(spinlock_, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            return stableVersions_.size();
        }

        bool checkStableVersionSize() {
            RWSpinLockGuard<RWSpinLock> _(spinlock_, RWSpinLockMode::EXCLUSIVE_SPLOCK);
            return stableVersions_.size() <= MAX_VERSION_CONTROL_SIZE;
        }

        // aka. commit
        void upgradeStableVersion(TxnId versionId) {
            if (unstableVersion_.second == VISIBLE) {
                disableUnstableVersion();
                RWSpinLockGuard<RWSpinLock> _(spinlock_, RWSpinLockMode::EXCLUSIVE_SPLOCK);
                unstableVersion_.first.versionId = versionId;
                stableVersions_.emplace_back(unstableVersion_.first);
            }
        }

        // aka. rollback
        void disableUnstableVersion() {
            unstableVersion_.second = INVISIBLE;
        }

        friend bool operator<(const ControlObject &lhs, const ControlObject &rhs) {
            return lhs.versionId < rhs.versionId;
        }

    private:
        mutable RWSpinLock spinlock_{};
        std::vector<ControlObject> stableVersions_{};
        std::pair<ControlObject, Visibility> unstableVersion_{ControlObject{}, INVISIBLE};
    };

}

#endif
