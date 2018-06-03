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

#ifndef __UTILS_HPP_INCLUDED_
#define __UTILS_HPP_INCLUDED_

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <sys/time.h>

namespace nogdb {

    struct Profiler {

        using Duration = std::chrono::duration<double>;

        Profiler(const std::string &name_)
                : name{name_}, start{std::chrono::high_resolution_clock::now()} {}

        ~Profiler() noexcept {
            ;
            auto d = std::chrono::high_resolution_clock::now() - start;
            std::cout << name << ": " << std::chrono::duration_cast<Duration>(d).count() * 1000 << std::endl;
        }

        std::string name;
        std::chrono::high_resolution_clock::time_point start;
    };

#define PROFILE_BLOCK(pbn) Profiler _pf(pbn)

    unsigned long long currentTimestamp();

    bool fileExists(const std::string &fileName);

    std::vector<std::string> split(const std::string &string, char delimeter);

    void replaceAll(std::string &string, const std::string &from, const std::string &to);

    void require(bool cmp);

}

#endif
