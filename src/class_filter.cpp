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

#include <algorithm>

#include "nogdb_compare.h"

namespace nogdb {

    ClassFilter::ClassFilter(const std::initializer_list<std::string> &initializerList) {
        for (const auto &value: initializerList) {
            classNames.insert(value);
        }
    }

    ClassFilter::ClassFilter(const std::vector<std::string> &classNames_) {
        for (const auto &className: classNames_) {
            classNames.insert(className);
        }
    }

    ClassFilter::ClassFilter(const std::list<std::string> &classNames_) {
        for (const auto &className: classNames_) {
            classNames.insert(className);
        }
    }

    ClassFilter::ClassFilter(const std::set<std::string> &classNames_) {
        for (const auto &className: classNames_) {
            classNames.insert(className);
        }
    }

    ClassFilter::ClassFilter(const std::vector<std::string>::const_iterator &begin,
                             const std::vector<std::string>::const_iterator &end) {
        for (auto it = begin; it != end; ++it) {
            classNames.insert(*it);
        }
    }

    ClassFilter::ClassFilter(const std::list<std::string>::const_iterator &begin,
                             const std::list<std::string>::const_iterator &end) {
        for (auto it = begin; it != end; ++it) {
            classNames.insert(*it);
        }
    }

    ClassFilter::ClassFilter(const std::set<std::string>::const_iterator &begin,
                             const std::set<std::string>::const_iterator &end) {
        for (auto it = begin; it != end; ++it) {
            classNames.insert(*it);
        }
    }

//ClassFilter& ClassFilter::exclude() {
//    isExclude = true;
//    return *this;
//}

    void ClassFilter::add(const std::string &className) {
        classNames.insert(className);
    }

    void ClassFilter::remove(const std::string &className) {
        classNames.erase(className);
    }

    size_t ClassFilter::size() const {
        return classNames.size();
    }

    bool ClassFilter::empty() const {
        return classNames.empty();
    }

    const std::set<std::string> &ClassFilter::getClassName() const {
        return classNames;
    }

//bool ClassFilter::isExcludeClassName() const {
//    return isExclude;
//}

}
