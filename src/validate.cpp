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

#include <algorithm>
#include <regex>

#include "validate.hpp"

#include "nogdb_txn.h"

namespace nogdb {

    void Validate::isTransactionValid(const Txn &txn) {
        if (txn.getTxnMode() == Txn::Mode::READ_ONLY) {
            throw Error(NOGDB_TXN_INVALID_MODE, Error::Type::TRANSACTION);
        }
        if (!txn.txnBase->isNotCompleted()) {
            throw Error(NOGDB_TXN_COMPLETED, Error::Type::TRANSACTION);
        }
    }

    void Validate::isClassNameValid(const std::string &className) {
        if (!isNameValid(className)) {
            throw Error(NOGDB_CTX_INVALID_CLASSNAME, Error::Type::CONTEXT);
        }
    }

    void Validate::isPropertyNameValid(const std::string &propName) {
        if (!isNameValid(propName)) {
            throw Error(NOGDB_CTX_INVALID_PROPERTYNAME, Error::Type::CONTEXT);
        }
    }

    bool Validate::isNameValid(const std::string &name) {
        return std::regex_match(name, std::regex("^[A-Za-z_@][A-Za-z0-9_]*$"));
    }

    void Validate::isClassTypeValid(const ClassType &type) {
        switch (type) {
            case ClassType::VERTEX:
            case ClassType::EDGE:
                return;
            default:
                throw Error(NOGDB_CTX_INVALID_CLASSTYPE, Error::Type::CONTEXT);
        }
    }

    void Validate::isPropertyTypeValid(const PropertyType &type) {
        switch (type) {
            case PropertyType::TINYINT:
            case PropertyType::UNSIGNED_TINYINT:
            case PropertyType::SMALLINT:
            case PropertyType::UNSIGNED_SMALLINT:
            case PropertyType::INTEGER:
            case PropertyType::UNSIGNED_INTEGER:
            case PropertyType::BIGINT:
            case PropertyType::UNSIGNED_BIGINT:
            case PropertyType::TEXT:
            case PropertyType::REAL:
            case PropertyType::BLOB:
                return;
            default:
                throw Error(NOGDB_CTX_INVALID_PROPTYPE, Error::Type::CONTEXT);
        }
    }

    void Validate::isNotDuplicatedClass(const Txn &txn, const std::string &className) {
        auto foundClass = txn.txnCtx.dbSchema->find(*txn.txnBase, className);
        if (foundClass != nullptr) {
            throw Error(NOGDB_CTX_DUPLICATE_CLASS, Error::Type::CONTEXT);
        }
    }

    std::shared_ptr<Schema::ClassDescriptor> Validate::isExistingClass(const Txn &txn, const std::string &className) {
        auto foundClass = txn.txnCtx.dbSchema->find(*txn.txnBase, className);
        if (foundClass == nullptr) {
            throw Error(NOGDB_CTX_NOEXST_CLASS, Error::Type::CONTEXT);
        }
        return foundClass;
    }

    std::shared_ptr<Schema::ClassDescriptor> Validate::isExistingClass(const Txn &txn, const ClassId &classId) {
        auto foundClass = txn.txnCtx.dbSchema->find(*txn.txnBase, classId);
        if (foundClass == nullptr) {
            throw Error(NOGDB_CTX_NOEXST_CLASS, Error::Type::CONTEXT);
        }
        return foundClass;
    }

    Schema::PropertyDescriptor Validate::isExistingProperty(const BaseTxn &txn,
                                                            const std::shared_ptr<Schema::ClassDescriptor> &classDescriptor,
                                                            const std::string &propertyName) {
        auto properties = BaseTxn::getCurrentVersion(txn, classDescriptor->properties).first;
        auto foundProperty = properties.find(propertyName);
        if (foundProperty == properties.cend()) {
            throw Error(NOGDB_CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
        }
        return foundProperty->second;
    }

    std::pair<ClassId, Schema::PropertyDescriptor>
    Validate::isExistingPropertyExtend(const BaseTxn &txn,
                                       const std::shared_ptr<Schema::ClassDescriptor> &classDescriptor,
                                       const std::string &propertyName) {
        auto properties = BaseTxn::getCurrentVersion(txn, classDescriptor->properties).first;
        auto foundProperty = properties.find(propertyName);
        if (foundProperty == properties.cend()) {
            if (auto superClassDescriptor = BaseTxn::getCurrentVersion(txn, classDescriptor->super).first.lock()) {
                return isExistingPropertyExtend(txn, superClassDescriptor, propertyName);
            } else {
                throw Error(NOGDB_CTX_NOEXST_PROPERTY, Error::Type::CONTEXT);
            }
        } else {
            return std::make_pair(classDescriptor->id, foundProperty->second);
        }
    }

    void Validate::isNotDuplicatedProperty(const BaseTxn &txn,
                                           const std::shared_ptr<Schema::ClassDescriptor> &classDescriptor,
                                           const std::string &propertyName) {
        auto properties = BaseTxn::getCurrentVersion(txn, classDescriptor->properties).first;
        auto foundProperty = properties.find(propertyName);
        if (foundProperty != properties.cend()) {
            throw Error(NOGDB_CTX_DUPLICATE_PROPERTY, Error::Type::CONTEXT);
        }
        if (auto superClassDescriptor = BaseTxn::getCurrentVersion(txn, classDescriptor->super).first.lock()) {
            isNotDuplicatedProperty(txn, superClassDescriptor, propertyName);
        }
    }

    void Validate::isNotOverridenProperty(const BaseTxn &txn,
                                          const std::shared_ptr<Schema::ClassDescriptor> &classDescriptor,
                                          const std::string &propertyName) {
        auto properties = BaseTxn::getCurrentVersion(txn, classDescriptor->properties).first;
        auto foundProperty = properties.find(propertyName);
        if (foundProperty != properties.cend()) {
            throw Error(NOGDB_CTX_OVERRIDE_PROPERTY, Error::Type::CONTEXT);
        }
        for (const auto &subClassDescriptor: BaseTxn::getCurrentVersion(txn, classDescriptor->sub).first) {
            if (auto subClassDescriptorPtr = subClassDescriptor.lock()) {
                isNotOverridenProperty(txn, subClassDescriptorPtr, propertyName);
            }
        }
    }
}



