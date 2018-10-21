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

  GraphFilter::GraphFilter(const Condition& condition) {
    filter._condition = condition;
    mode = FilterMode::CONDITION;
  }

  GraphFilter::GraphFilter(const MultiCondition& multiCondition) {
    filter._multiCondition = multiCondition;
    mode = FilterMode::MULTI_CONDITION;
  }

  GraphFilter::GraphFilter(bool (*function)(const Record &record)) {
    filter._function = function;
    mode = FilterMode::COMPARE_FUNCTION;
  }

  GraphFilter::~GraphFilter() noexcept {
    switch(mode) {
      case FilterMode::CONDITION:
        filter._condition.~Condition();
        break;
      case FilterMode::MULTI_CONDITION:
        filter._multiCondition.~MultiCondition();
        break;
      default:
        break;
    }
  }

  GraphFilter::GraphFilter(const GraphFilter& other)
    : mode{other.mode}, onlyClasses{other.onlyClasses}, ignoreClasses{other.ignoreClasses} {
    switch(mode) {
      case FilterMode::CONDITION:
        filter._condition = other.filter._condition;
        break;
      case FilterMode::MULTI_CONDITION:
        filter._multiCondition = other.filter._multiCondition;
        break;
      case FilterMode::COMPARE_FUNCTION:
        filter._function = other.filter._function;
        break;
    }
  }

  GraphFilter& GraphFilter::operator=(const GraphFilter& other) {
    if (this != &other) {
      auto tmp = GraphFilter(other);
      using std::swap;
      swap(*this, tmp);
    }
    return *this;
  }

  GraphFilter::GraphFilter(GraphFilter&& other) noexcept {
    auto tmp = std::move(other);
    using std::swap;
    swap(*this, tmp);
  }

  GraphFilter& GraphFilter::operator=(GraphFilter&& other) noexcept {
    if (this != &other) {
      mode = other.mode;
      onlyClasses = std::move(other.onlyClasses);
      ignoreClasses = std::move(other.ignoreClasses);
      switch(mode) {
        case FilterMode::CONDITION:
          filter._condition = std::move(other.filter._condition);
          break;
        case FilterMode::MULTI_CONDITION:
          filter._multiCondition = std::move(other.filter._multiCondition);
          break;
        case FilterMode::COMPARE_FUNCTION:
          filter._function = other.filter._function;
          other.filter._function = nullptr;
          break;
      }
    }
    return *this;
  }

  GraphFilter &GraphFilter::only(const std::string &className) {
    onlyClasses.insert(className);
    return *this;
  }

  GraphFilter &GraphFilter::only(const std::vector<std::string> &classNames) {
    for (const auto &value: classNames) {
      onlyClasses.insert(value);
    }
    return *this;
  }

  GraphFilter &GraphFilter::only(const std::list<std::string> &classNames) {
    for (const auto &value: classNames) {
      onlyClasses.insert(value);
    }
    return *this;
  }

  GraphFilter &GraphFilter::only(const std::set<std::string> &classNames) {
    for (const auto &value: classNames) {
      onlyClasses.insert(value);
    }
    return *this;
  }

  GraphFilter &GraphFilter::only(const std::vector<std::string>::const_iterator &begin,
                                 const std::vector<std::string>::const_iterator &end) {
    for (auto it = begin; it != end; ++it) {
      onlyClasses.insert(*it);
    }
    return *this;
  }

  GraphFilter &GraphFilter::only(const std::list<std::string>::const_iterator &begin,
                                 const std::list<std::string>::const_iterator &end) {
    for (auto it = begin; it != end; ++it) {
      onlyClasses.insert(*it);
    }
    return *this;
  }

  GraphFilter &GraphFilter::only(const std::set<std::string>::const_iterator &begin,
                                 const std::set<std::string>::const_iterator &end) {
    for (auto it = begin; it != end; ++it) {
      onlyClasses.insert(*it);
    }
    return *this;
  }

  GraphFilter &GraphFilter::exclude(const std::string &className) {
    ignoreClasses.insert(className);
    return *this;
  }

  GraphFilter &GraphFilter::exclude(const std::vector<std::string> &classNames) {
    for (const auto &value: classNames) {
      ignoreClasses.insert(value);
    }
    return *this;
  }

  GraphFilter &GraphFilter::exclude(const std::list<std::string> &classNames) {
    for (const auto &value: classNames) {
      ignoreClasses.insert(value);
    }
    return *this;
  }

  GraphFilter &GraphFilter::exclude(const std::set<std::string> &classNames) {
    for (const auto &value: classNames) {
      ignoreClasses.insert(value);
    }
    return *this;
  }

  GraphFilter &GraphFilter::exclude(const std::vector<std::string>::const_iterator &begin,
                                    const std::vector<std::string>::const_iterator &end) {
    for (auto it = begin; it != end; ++it) {
      ignoreClasses.insert(*it);
    }
    return *this;
  }

  GraphFilter &GraphFilter::exclude(const std::list<std::string>::const_iterator &begin,
                                    const std::list<std::string>::const_iterator &end) {
    for (auto it = begin; it != end; ++it) {
      ignoreClasses.insert(*it);
    }
    return *this;
  }

  GraphFilter &GraphFilter::exclude(const std::set<std::string>::const_iterator &begin,
                                    const std::set<std::string>::const_iterator &end) {
    for (auto it = begin; it != end; ++it) {
      ignoreClasses.insert(*it);
    }
    return *this;
  }

}