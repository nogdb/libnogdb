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

Condition::Condition(const std::string& _propName)
    : propName { _propName }
    , comp { Comparator::NOT_NULL }
{
    valueBytes = Bytes {};
    valueSet = std::vector<Bytes> {};
}

Condition Condition::operator!() const
{
    auto tmp(*this);
    if (tmp.comp == Comparator::NOT_NULL) {
        tmp.comp = Comparator::IS_NULL;
    } else if (tmp.comp == Comparator::IS_NULL) {
        tmp.comp = Comparator::NOT_NULL;
    } else {
        tmp.isNegative = !isNegative;
    }
    return tmp;
}

MultiCondition Condition::operator&&(const Condition& c) const
{
    return MultiCondition { *this, c, MultiCondition::Operator::AND };
}

MultiCondition Condition::operator&&(const MultiCondition& e) const
{
    return MultiCondition { *this, e, MultiCondition::Operator::AND };
}

MultiCondition Condition::operator||(const Condition& c) const
{
    return MultiCondition { *this, c, MultiCondition::Operator::OR };
}

MultiCondition Condition::operator||(const MultiCondition& e) const
{
    return MultiCondition { *this, e, MultiCondition::Operator::OR };
}

MultiCondition Condition::operator&&(bool (*cmpFunc)(const Record& r)) const
{
    return MultiCondition { *this, cmpFunc, MultiCondition::Operator::AND };
}

MultiCondition Condition::operator||(bool (*cmpFunc)(const Record& r)) const
{
    return MultiCondition { *this, cmpFunc, MultiCondition::Operator::OR };
}

}