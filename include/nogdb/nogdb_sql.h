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

#include <memory>
#include <string>

#include "nogdb_errors.h"
#include "nogdb_types.h"

namespace nogdb {

namespace sql_parser {
    class Context;
}

struct SQL {
    SQL() = delete;

    ~SQL() noexcept = delete;

    SQL& operator=(const SQL& _) = delete;

    class Result {
    public:
        friend class sql_parser::Context;

        Result()
            : t(NO_RESULT)
            , value(nullptr)
        {
        }

        enum Type {
            NO_RESULT,
            ERROR,
            CLASS_DESCRIPTOR,
            PROPERTY_DESCRIPTOR,
            RECORD_DESCRIPTORS,
            RESULT_SET
        };

        inline Type type() const
        {
            return this->t;
        }

        template <typename T>
        inline T& get() const
        {
            return *std::static_pointer_cast<T>(this->value);
        }

    protected:
        Result(Type type_, std::shared_ptr<void> value_)
            : t(type_)
            , value(value_)
        {
        }

        Result(Error* error)
            : t(ERROR)
            , value(error)
        {
        }

        Result(ClassDescriptor* classDescriptor)
            : t(CLASS_DESCRIPTOR)
            , value(classDescriptor)
        {
        }

        Result(PropertyDescriptor* propertyDescriptor)
            : t(PROPERTY_DESCRIPTOR)
            , value(propertyDescriptor)
        {
        }

        Result(std::vector<RecordDescriptor>* recordDescriptor)
            : t(RECORD_DESCRIPTORS)
            , value(recordDescriptor)
        {
        }

        Result(ResultSet* resultSet)
            : t(RESULT_SET)
            , value(resultSet)
        {
        }

        Type t;
        std::shared_ptr<void> value;
    };

    static const Result execute(Transaction& txn, const std::string& sql);
};
}
