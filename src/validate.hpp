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

#pragma once

#include "constant.hpp"
#include <regex>

#include "nogdb/nogdb.h"
#include "nogdb/nogdb_errors.h"

#define BEGIN_VALIDATION(_txn) nogdb::validate::Validator(_txn)
#define CLASS_ID_UPPER_LIMIT UINT16_MAX - 1

namespace nogdb {
namespace validate {
    using namespace adapter::schema;

    class Validator {
    public:
        Validator(const Transaction* txn)
            : _txn { txn }
        {
        }

        virtual ~Validator() noexcept = default;

        Validator& isTxnValid();

        Validator& isTxnCompleted();

        Validator& isClassIdMaxReach();

        Validator& isPropertyIdMaxReach();

        Validator& isIndexIdMaxReach();

        Validator& isClassNameValid(const std::string& className);

        Validator& isPropertyNameValid(const std::string& propName);

        Validator& isClassTypeValid(const ClassType& type);

        Validator& isPropertyTypeValid(const PropertyType& type);

        Validator& isNotDuplicatedClass(const std::string& className);

        Validator& isNotDuplicatedProperty(const ClassId& classId, const std::string& propertyName);

        Validator& isNotOverriddenProperty(const ClassId& classId, const std::string& propertyName);

        Validator& isExistingSrcVertex(const RecordDescriptor& vertex);

        Validator& isExistingDstVertex(const RecordDescriptor& vertex);

        Validator& isExistingVertex(const RecordDescriptor& vertex);

        Validator& isExistingVertices(const std::set<RecordDescriptor>& vertices);

    private:
        const Transaction* _txn;

        inline static bool isNameValid(const std::string& name)
        {
            return std::regex_match(name, GLOBAL_VALID_NAME_PATTERN);
        }
    };
}
}
