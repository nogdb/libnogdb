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

#include <chrono>
#include <ctime>
#include <sstream>
#include <sys/file.h>
#include <sys/stat.h>

#include "utils.hpp"

namespace nogdb {

    unsigned long long currentTimestamp() {
        using std::chrono::duration_cast;
        using std::chrono::milliseconds;
        using std::chrono::system_clock;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    bool fileExists(const std::string &fileName) {
        struct stat fileStat;
        return stat((char *) fileName.c_str(), &fileStat) == 0;
    }

    std::vector<std::string> split(const std::string &string, char delimeter) {
        auto elements = std::vector<std::string>{};
        std::stringstream stringStream{};
        stringStream.str(string);
        auto subString = std::string{};
        while (std::getline(stringStream, subString, delimeter)) {
            elements.push_back(subString);
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
