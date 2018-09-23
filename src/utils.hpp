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

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <memory>
#include <unordered_map>
#include <functional>
#include <sstream>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "nogdb/nogdb_errors.h"

namespace nogdb {

    namespace utils {
        // unordered_map cache
        namespace caching {

            template<typename K, typename V>
            class UnorderedCache {
            public:
                UnorderedCache() = default;

                virtual ~UnorderedCache() noexcept = default;

                V get(const K& key, std::function<V(void)> callback) const {
                    auto found = _underlying.find(key);
                    if (found != _underlying.cend()) {
                        return found->second;
                    } else {
                        auto value = callback();
                        _underlying.emplace({key, value});
                        return value;
                    }
                }

                void set(const K& key, const V& val) {
                    _underlying[key] = val;
                }

                void unset(const K& key) {
                    _underlying.erase(key);
                }

            private:
                mutable std::unordered_map<K, V> _underlying{};
            };
        }

        // profiler
        namespace profiler {

#define PROFILE_BLOCK(pbn) Profiler _pf(pbn)

            struct Profiler {

                using Duration = std::chrono::duration<double>;

                Profiler(const std::string &name_)
                        : name{name_}, start{std::chrono::high_resolution_clock::now()} {}

                ~Profiler() noexcept {
                    auto d = std::chrono::high_resolution_clock::now() - start;
                    std::cout << name << ": " << std::chrono::duration_cast<Duration>(d).count() * 1000 << std::endl;
                }

                std::string name;
                std::chrono::high_resolution_clock::time_point start;
            };
        }

        // date & time
        namespace datetime {
            unsigned long long currentTimestamp() {
                using std::chrono::duration_cast;
                using std::chrono::milliseconds;
                using std::chrono::system_clock;
                return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            }
        }

        // string utilities
        namespace string {
            std::vector<std::string> split(const std::string &string, char delimeter) {
                auto elements = std::vector<std::string>{};
                std::stringstream ss{};
                ss.str(string);
                auto subStr = std::string{};
                while (std::getline(ss, subStr, delimeter)) {
                    elements.push_back(subStr);
                }
                return elements;
            }

            void replaceAll(std::string &string, const std::string &from, const std::string &to) {
                if (!from.empty()) {
                    auto position = string.find(from, 0);
                    while (position != std::string::npos) {
                        string.replace(position, from.length(), to);
                        position = string.find(from, position + to.length());
                    }
                }
            }
        }


        // assertion
        namespace assertion {
            void require(bool cmp) {
                if (!cmp) {
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INTERNAL_ERR);
                }
            }
        }

        namespace io {
            bool fileExists(const std::string &fileName) {
                struct stat fileStat;
                return stat((char *) fileName.c_str(), &fileStat) == 0;
            }

#ifdef __MINGW32__
            int mkdir(const char *pathname, int mode) {
                return ::mkdir(pathname) || ::chmod(pathname, mode);
            }

            int openLockFile(const char *pathname) {
                unlink(pathname);
                return open(pathname, O_CREAT | O_RDONLY | O_EXCL, 0644);
            }

            int unlockFile(int fd) {
                return close(fd);
            }
#else
            int mkdir(const char *pathname, int mode) {
                return ::mkdir(pathname, mode);
            }

            int openLockFile(const char *pathname) {
                int lockFileDescriptor = open(pathname, O_CREAT | O_RDONLY, 0644);
                if (flock(lockFileDescriptor, LOCK_EX | LOCK_NB) == -1) {
                    lockFileDescriptor = -1;
                }
                return lockFileDescriptor;
            }

            int unlockFile(int fd) {
                return flock(fd, LOCK_UN);
            }
#endif
        }

    }
}
