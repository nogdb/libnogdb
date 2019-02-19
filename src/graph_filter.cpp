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

#include "nogdb/nogdb_types.h"

namespace nogdb {

GraphFilter::GraphFilter()
{
    _function = nullptr;
    _mode = FilterMode::COMPARE_FUNCTION;
}

GraphFilter::GraphFilter(const Condition& condition)
{
    _condition = std::make_shared<Condition>(condition);
    _mode = FilterMode::CONDITION;
}

GraphFilter::GraphFilter(const MultiCondition& multiCondition)
{
    _multiCondition = std::make_shared<MultiCondition>(multiCondition);
    _mode = FilterMode::MULTI_CONDITION;
}

GraphFilter::GraphFilter(bool (*function)(const Record& record))
{
    _function = function;
    _mode = FilterMode::COMPARE_FUNCTION;
}

GraphFilter::GraphFilter(const GraphFilter& other)
    : _mode { other._mode }
    , _onlyClasses { other._onlyClasses }
    , _onlySubOfClasses { other._onlySubOfClasses }
    , _ignoreClasses { other._ignoreClasses }
    , _ignoreSubOfClasses { other._ignoreSubOfClasses }
{
    switch (_mode) {
    case FilterMode::CONDITION:
        _condition = other._condition;
        break;
    case FilterMode::MULTI_CONDITION:
        _multiCondition = other._multiCondition;
        break;
    case FilterMode::COMPARE_FUNCTION:
        _function = other._function;
        break;
    }
}

GraphFilter& GraphFilter::operator=(const GraphFilter& other)
{
    if (this != &other) {
        _mode = other._mode;
        _onlyClasses = other._onlyClasses;
        _onlySubOfClasses = other._onlySubOfClasses;
        _ignoreClasses = other._ignoreClasses;
        _ignoreSubOfClasses = other._ignoreSubOfClasses;
        switch (_mode) {
        case FilterMode::CONDITION:
            _condition = other._condition;
            break;
        case FilterMode::MULTI_CONDITION:
            _multiCondition = other._multiCondition;
            break;
        case FilterMode::COMPARE_FUNCTION:
            _function = other._function;
            break;
        }
    }
    return *this;
}

GraphFilter::GraphFilter(GraphFilter&& other) noexcept
{
    _mode = other._mode;
    _onlyClasses = std::move(other._onlyClasses);
    _onlySubOfClasses = std::move(other._onlySubOfClasses);
    _ignoreClasses = std::move(other._ignoreClasses);
    _ignoreSubOfClasses = std::move(other._ignoreSubOfClasses);
    switch (_mode) {
    case FilterMode::CONDITION:
        _condition = std::move(other._condition);
        break;
    case FilterMode::MULTI_CONDITION:
        _multiCondition = std::move(other._multiCondition);
        break;
    case FilterMode::COMPARE_FUNCTION:
        _function = other._function;
        other._function = nullptr;
        break;
    }
}

GraphFilter& GraphFilter::operator=(GraphFilter&& other) noexcept
{
    if (this != &other) {
        _mode = other._mode;
        _onlyClasses = std::move(other._onlyClasses);
        _onlySubOfClasses = std::move(other._onlySubOfClasses);
        _ignoreClasses = std::move(other._ignoreClasses);
        _ignoreSubOfClasses = std::move(other._ignoreSubOfClasses);
        switch (_mode) {
        case FilterMode::CONDITION:
            _condition = std::move(other._condition);
            break;
        case FilterMode::MULTI_CONDITION:
            _multiCondition = std::move(other._multiCondition);
            break;
        case FilterMode::COMPARE_FUNCTION:
            _function = other._function;
            other._function = nullptr;
            break;
        }
    }
    return *this;
}

GraphFilter& GraphFilter::only(const std::string& className)
{
    _onlyClasses.insert(className);
    return *this;
}

GraphFilter& GraphFilter::only(const std::vector<std::string>& classNames)
{
    for (const auto& value : classNames) {
        _onlyClasses.insert(value);
    }
    return *this;
}

GraphFilter& GraphFilter::only(const std::list<std::string>& classNames)
{
    for (const auto& value : classNames) {
        _onlyClasses.insert(value);
    }
    return *this;
}

GraphFilter& GraphFilter::only(const std::set<std::string>& classNames)
{
    for (const auto& value : classNames) {
        _onlyClasses.insert(value);
    }
    return *this;
}

GraphFilter& GraphFilter::only(const std::vector<std::string>::const_iterator& begin,
    const std::vector<std::string>::const_iterator& end)
{
    for (auto it = begin; it != end; ++it) {
        _onlyClasses.insert(*it);
    }
    return *this;
}

GraphFilter& GraphFilter::only(const std::list<std::string>::const_iterator& begin,
    const std::list<std::string>::const_iterator& end)
{
    for (auto it = begin; it != end; ++it) {
        _onlyClasses.insert(*it);
    }
    return *this;
}

GraphFilter& GraphFilter::only(const std::set<std::string>::const_iterator& begin,
    const std::set<std::string>::const_iterator& end)
{
    for (auto it = begin; it != end; ++it) {
        _onlyClasses.insert(*it);
    }
    return *this;
}

GraphFilter& GraphFilter::onlySubClassOf(const std::string& className)
{
    _onlySubOfClasses.insert(className);
    return *this;
}

GraphFilter& GraphFilter::onlySubClassOf(const std::vector<std::string>& classNames)
{
    for (const auto& value : classNames) {
        _onlySubOfClasses.insert(value);
    }
    return *this;
}

GraphFilter& GraphFilter::onlySubClassOf(const std::list<std::string>& classNames)
{
    for (const auto& value : classNames) {
        _onlySubOfClasses.insert(value);
    }
    return *this;
}

GraphFilter& GraphFilter::onlySubClassOf(const std::set<std::string>& classNames)
{
    for (const auto& value : classNames) {
        _onlySubOfClasses.insert(value);
    }
    return *this;
}

GraphFilter& GraphFilter::onlySubClassOf(const std::vector<std::string>::const_iterator& begin,
    const std::vector<std::string>::const_iterator& end)
{
    for (auto it = begin; it != end; ++it) {
        _onlySubOfClasses.insert(*it);
    }
    return *this;
}

GraphFilter& GraphFilter::onlySubClassOf(const std::list<std::string>::const_iterator& begin,
    const std::list<std::string>::const_iterator& end)
{
    for (auto it = begin; it != end; ++it) {
        _onlySubOfClasses.insert(*it);
    }
    return *this;
}

GraphFilter& GraphFilter::onlySubClassOf(const std::set<std::string>::const_iterator& begin,
    const std::set<std::string>::const_iterator& end)
{
    for (auto it = begin; it != end; ++it) {
        _onlySubOfClasses.insert(*it);
    }
    return *this;
}

GraphFilter& GraphFilter::exclude(const std::string& className)
{
    _ignoreClasses.insert(className);
    return *this;
}

GraphFilter& GraphFilter::exclude(const std::vector<std::string>& classNames)
{
    for (const auto& value : classNames) {
        _ignoreClasses.insert(value);
    }
    return *this;
}

GraphFilter& GraphFilter::exclude(const std::list<std::string>& classNames)
{
    for (const auto& value : classNames) {
        _ignoreClasses.insert(value);
    }
    return *this;
}

GraphFilter& GraphFilter::exclude(const std::set<std::string>& classNames)
{
    for (const auto& value : classNames) {
        _ignoreClasses.insert(value);
    }
    return *this;
}

GraphFilter& GraphFilter::exclude(const std::vector<std::string>::const_iterator& begin,
    const std::vector<std::string>::const_iterator& end)
{
    for (auto it = begin; it != end; ++it) {
        _ignoreClasses.insert(*it);
    }
    return *this;
}

GraphFilter& GraphFilter::exclude(const std::list<std::string>::const_iterator& begin,
    const std::list<std::string>::const_iterator& end)
{
    for (auto it = begin; it != end; ++it) {
        _ignoreClasses.insert(*it);
    }
    return *this;
}

GraphFilter& GraphFilter::exclude(const std::set<std::string>::const_iterator& begin,
    const std::set<std::string>::const_iterator& end)
{
    for (auto it = begin; it != end; ++it) {
        _ignoreClasses.insert(*it);
    }
    return *this;
}

GraphFilter& GraphFilter::excludeSubClassOf(const std::string& className)
{
    _ignoreSubOfClasses.insert(className);
    return *this;
}

GraphFilter& GraphFilter::excludeSubClassOf(const std::vector<std::string>& classNames)
{
    for (const auto& value : classNames) {
        _ignoreSubOfClasses.insert(value);
    }
    return *this;
}

GraphFilter& GraphFilter::excludeSubClassOf(const std::list<std::string>& classNames)
{
    for (const auto& value : classNames) {
        _ignoreSubOfClasses.insert(value);
    }
    return *this;
}

GraphFilter& GraphFilter::excludeSubClassOf(const std::set<std::string>& classNames)
{
    for (const auto& value : classNames) {
        _ignoreSubOfClasses.insert(value);
    }
    return *this;
}

GraphFilter& GraphFilter::excludeSubClassOf(const std::vector<std::string>::const_iterator& begin,
    const std::vector<std::string>::const_iterator& end)
{
    for (auto it = begin; it != end; ++it) {
        _ignoreSubOfClasses.insert(*it);
    }
    return *this;
}

GraphFilter& GraphFilter::excludeSubClassOf(const std::list<std::string>::const_iterator& begin,
    const std::list<std::string>::const_iterator& end)
{
    for (auto it = begin; it != end; ++it) {
        _ignoreSubOfClasses.insert(*it);
    }
    return *this;
}

GraphFilter& GraphFilter::excludeSubClassOf(const std::set<std::string>::const_iterator& begin,
    const std::set<std::string>::const_iterator& end)
{
    for (auto it = begin; it != end; ++it) {
        _ignoreSubOfClasses.insert(*it);
    }
    return *this;
}

}