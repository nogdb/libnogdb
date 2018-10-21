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

#include "nogdb_compare.h"

namespace nogdb {

  GraphFilter &GraphFilter::where(const Condition* condition) {
    cmpCondition = condition;
    cmpMultiCondition = nullptr;
    cmpFunction = nullptr;
    return *this;
  }

  GraphFilter &GraphFilter::where(const MultiCondition* multiCondition) {
    cmpCondition = nullptr;
    cmpMultiCondition = multiCondition;
    cmpFunction = nullptr;
    return *this;
  }

  GraphFilter &GraphFilter::where(bool (*function)(const Record &record)) {
    cmpCondition = nullptr;
    cmpMultiCondition = nullptr;
    cmpFunction = function;
    return *this;
  }

  GraphFilter &GraphFilter::only(const std::initializer_list<std::string> &initializerList) {
    onlyClasses.clear();
    for (const auto &value: initializerList) {
      onlyClasses.insert(value);
    }
    return *this;
  }

  GraphFilter &GraphFilter::only(const std::vector<std::string> &classNames) {
    onlyClasses.clear();
    for (const auto &value: classNames) {
      onlyClasses.insert(value);
    }
    return *this;
  }

  GraphFilter &GraphFilter::only(const std::list<std::string> &classNames) {
    onlyClasses.clear();
    for (const auto &value: classNames) {
      onlyClasses.insert(value);
    }
    return *this;
  }

  GraphFilter &GraphFilter::only(const std::set<std::string> &classNames) {
    onlyClasses.clear();
    for (const auto &value: classNames) {
      onlyClasses.insert(value);
    }
    return *this;
  }

  GraphFilter &GraphFilter::only(const std::vector<std::string>::const_iterator &begin,
                                 const std::vector<std::string>::const_iterator &end) {
    onlyClasses.clear();
    for (auto it = begin; it != end; ++it) {
      onlyClasses.insert(*it);
    }
    return *this;
  }

  GraphFilter &GraphFilter::only(const std::list<std::string>::const_iterator &begin,
                                 const std::list<std::string>::const_iterator &end) {
    onlyClasses.clear();
    for (auto it = begin; it != end; ++it) {
      onlyClasses.insert(*it);
    }
    return *this;
  }

  GraphFilter &GraphFilter::only(const std::set<std::string>::const_iterator &begin,
                                 const std::set<std::string>::const_iterator &end) {
    onlyClasses.clear();
    for (auto it = begin; it != end; ++it) {
      onlyClasses.insert(*it);
    }
    return *this;
  }

  GraphFilter &GraphFilter::exclude(const std::initializer_list<std::string> &initializerList) {
    ignoreClasses.clear();
    for (const auto &value: initializerList) {
      ignoreClasses.insert(value);
    }
    return *this;
  }

  GraphFilter &GraphFilter::exclude(const std::vector<std::string> &classNames) {
    ignoreClasses.clear();
    for (const auto &value: classNames) {
      ignoreClasses.insert(value);
    }
    return *this;
  }

  GraphFilter &GraphFilter::exclude(const std::list<std::string> &classNames) {
    ignoreClasses.clear();
    for (const auto &value: classNames) {
      ignoreClasses.insert(value);
    }
    return *this;
  }

  GraphFilter &GraphFilter::exclude(const std::set<std::string> &classNames) {
    ignoreClasses.clear();
    for (const auto &value: classNames) {
      ignoreClasses.insert(value);
    }
    return *this;
  }

  GraphFilter &GraphFilter::exclude(const std::vector<std::string>::const_iterator &begin,
                                    const std::vector<std::string>::const_iterator &end) {
    ignoreClasses.clear();
    for (auto it = begin; it != end; ++it) {
      ignoreClasses.insert(*it);
    }
    return *this;
  }

  GraphFilter &GraphFilter::exclude(const std::list<std::string>::const_iterator &begin,
                                    const std::list<std::string>::const_iterator &end) {
    ignoreClasses.clear();
    for (auto it = begin; it != end; ++it) {
      ignoreClasses.insert(*it);
    }
    return *this;
  }

  GraphFilter &GraphFilter::exclude(const std::set<std::string>::const_iterator &begin,
                                    const std::set<std::string>::const_iterator &end) {
    ignoreClasses.clear();
    for (auto it = begin; it != end; ++it) {
      ignoreClasses.insert(*it);
    }
    return *this;
  }

}