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

#ifndef __VALIDATE_HPP_INCLUDED_
#define __VALIDATE_HPP_INCLUDED_

#include "schema.hpp"
#include "base_txn.hpp"

#include "nogdb_error.h"
#include "nogdb_context.h"
#include "nogdb_types.h"
#include "nogdb_txn.h"

namespace nogdb {
    struct Validate {
        Validate() = delete;

        ~Validate() noexcept = delete;

        static void isTransactionValid(const Txn &txn);

        static void isClassNameValid(const std::string &className);

        static void isPropertyNameValid(const std::string &propName);

        static bool isNameValid(const std::string& name);

        static void isClassTypeValid(const ClassType &type);

        static void isPropertyTypeValid(const PropertyType &type);

        static void isNotDuplicatedClass(const Txn &txn, const std::string &className);

        static std::shared_ptr<Schema::ClassDescriptor> isExistingClass(const Txn &txn, const std::string &className);

        static std::shared_ptr<Schema::ClassDescriptor> isExistingClass(const Txn &txn, const ClassId &classId);

        static void isNotDuplicatedProperty(const BaseTxn &txn,
                                            const std::shared_ptr<Schema::ClassDescriptor> &classDescriptor,
                                            const std::string &propertyName);

        static Schema::PropertyDescriptor isExistingProperty(const BaseTxn &txn,
                                                             const std::shared_ptr<Schema::ClassDescriptor> &classDescriptor,
                                                             const std::string &propertyName);

        static std::pair<ClassId, Schema::PropertyDescriptor>
        isExistingPropertyExtend(const BaseTxn &txn,
                                 const std::shared_ptr<Schema::ClassDescriptor> &classDescriptor,
                                 const std::string &propertyName);

        static void isNotOverridenProperty(const BaseTxn &txn,
                                           const std::shared_ptr<Schema::ClassDescriptor> &classDescriptor,
                                           const std::string &propertyName);
    };
}

#endif
