/*
 *  Copyright (C) 2019, NogDB <https://nogdb.org>
 *  <nogdb at throughwave dot co dot th>
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

#include "utils.hpp"

namespace nogdb {
namespace utils {
// date & time
namespace datetime {
    unsigned long long currentTimestamp()
    {
        using std::chrono::duration_cast;
        using std::chrono::milliseconds;
        using std::chrono::system_clock;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }
}

// string utilities
namespace string {
    std::vector<std::string> split(const std::string& str, char delimeter)
    {
        auto elements = std::vector<std::string> {};
        std::stringstream ss {};
        ss.str(str);
        auto subStr = std::string {};
        while (std::getline(ss, subStr, delimeter)) {
            elements.push_back(subStr);
        }
        return elements;
    }

    void replaceAll(std::string& string, const std::string& from, const std::string& to)
    {
        if (!from.empty()) {
            auto position = string.find(from, 0);
            while (position != std::string::npos) {
                string.replace(position, from.length(), to);
                position = string.find(from, position + to.length());
            }
        }
    }

    void toUpperCase(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    }

    std::string frontPadding(const std::string& str, const size_t length, const char paddingChar)
    {
        auto tmp(str);
        if (length > tmp.size()) {
            tmp.insert(0, length - tmp.size(), paddingChar);
        }
        return tmp;
    }
}

// assertion
namespace assertion {
    void require(bool cmp)
    {
        if (!cmp) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INTERNAL_ERR);
        }
    }
}

namespace io {
    bool fileExists(const std::string& fileName)
    {
        struct stat fileStat;
        return stat((char*)fileName.c_str(), &fileStat) == 0;
    }

#ifdef __MINGW32__
    int mkdir(const char* pathname, int mode)
    {
        return ::mkdir(pathname) || ::chmod(pathname, mode);
    }

    int openLockFile(const char* pathname)
    {
        unlink(pathname);
        return open(pathname, O_CREAT | O_RDONLY | O_EXCL, 0644);
    }

    int unlockFile(int fd)
    {
        return close(fd);
    }
#else

    int mkdir(const char* pathname, int mode)
    {
        return ::mkdir(pathname, mode);
    }

    int openLockFile(const char* pathname)
    {
        int lockFileDescriptor = open(pathname, O_CREAT | O_RDONLY, 0644);
        if (flock(lockFileDescriptor, LOCK_EX | LOCK_NB) == -1) {
            lockFileDescriptor = -1;
        }
        return lockFileDescriptor;
    }

    int unlockFile(int fd)
    {
        return flock(fd, LOCK_UN);
    }

#endif

    void writeBinaryFile(const char* pathname, const char* data, size_t size)
    {
        std::ofstream bin(pathname, std::ios::out | std::ios::binary);
        bin.write(data, size);
        bin.close();
    }

    const char* readBinaryFile(const char* pathname, size_t size)
    {
        auto data = new char[size];
        std::ifstream bin(pathname, std::ios::in | std::ios::binary | std::ios::ate);
        if (bin.is_open()) {
            bin.seekg(0, std::ios::end);
            size_t fileSize = bin.tellg();
            bin.seekg(0, std::ios::beg);
            if (size != fileSize) {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_UNKNOWN_ERR);
            }
            bin.read(data, fileSize);
        } else {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_UNKNOWN_ERR);
        }
        bin.close();
        return data;
    }

}
}
}
