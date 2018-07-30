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

#include <memory>

#include "schema.hpp"
#include "base_txn.hpp"
#include "utils.hpp"

namespace nogdb {

    void Schema::insert(BaseTxn &txn, const ClassDescriptorPtr &classDescriptor) {
        txn.addUncommittedSchema(classDescriptor);
    }

    void Schema::replace(BaseTxn &txn, const ClassDescriptorPtr &classDescriptor, const std::string &newClassName) {
        classDescriptor->name.addLatestVersion(newClassName);
        txn.addUncommittedSchema(classDescriptor);
    }

    Schema::ClassDescriptorPtr Schema::find(const BaseTxn &txn, const ClassId &classId) {
        RWSpinLockGuard<RWSpinLock> _(schemaInfo.splock);
        auto foundClass = schemaInfo.elements.find(classId);
        if (foundClass == schemaInfo.elements.cend()) {
            if (txn.getType() == BaseTxn::TxnType::READ_ONLY) {
                return nullptr;
            } else {
                auto classPtr = txn.findUncommittedSchema(classId);
                if (classPtr != nullptr) {
                    return (classPtr->checkReadWrite()) ? nullptr : classPtr;
                }
                return nullptr;
            }
        } else {
            if ((txn.getType() == BaseTxn::TxnType::READ_ONLY &&
                 foundClass->second->checkReadOnly(txn.getVersionId())) ||
                (txn.getType() == BaseTxn::TxnType::READ_WRITE && foundClass->second->checkReadWrite())) {
                return nullptr;
            }
        }
        return foundClass->second;
    }

    Schema::ClassDescriptorPtr Schema::find(const BaseTxn &txn, const std::string &className) {
        if (txn.getType() == BaseTxn::TxnType::READ_WRITE) {
            for (const auto &element: txn.findUncommittedSchema()) {
                if (auto classPtr = element.second) {
                    if (className == classPtr->name.getUnstableVersion().first) {
                        return (classPtr->checkReadWrite()) ? nullptr : classPtr;
                    }
                }
            }
        }
        RWSpinLockGuard<RWSpinLock> _(schemaInfo.splock);
        for (const auto &element: schemaInfo.elements) {
            if (auto classPtr = element.second) {
                if (className == BaseTxn::getCurrentVersion(txn, classPtr->name).first) {
                    if ((txn.getType() == BaseTxn::TxnType::READ_ONLY && classPtr->checkReadOnly(txn.getVersionId())) ||
                        (txn.getType() == BaseTxn::TxnType::READ_WRITE && classPtr->checkReadWrite())) {
                        return nullptr;
                    }
                    return classPtr;
                }
            }
        }
        return nullptr;

    }

    void Schema::erase(BaseTxn &txn, const ClassId &classId) noexcept {
        if (auto classPtr = find(txn, classId)) {
            if (classPtr->getState().second == TxnObject::StatusFlag::UNCOMMITTED_CREATE) {
                txn.deleteUncommittedSchema(classId);
            } else {
                classPtr->setStatus(TxnObject::StatusFlag::UNCOMMITTED_DELETE);
                txn.addUncommittedSchema(classPtr);
            }
        }
    }

    void Schema::forceDelete(const std::vector<ClassId> &classId) noexcept {
        RWSpinLockGuard<RWSpinLock> _(schemaInfo.splock, RWSpinLockMode::EXCLUSIVE_SPLOCK);
        for (const auto &cid: classId) {
            schemaInfo.elements.erase(cid);
        }
    }

    void Schema::clear() noexcept {
        schemaInfo.lockAndClear();
    }

    void Schema::addProperty(BaseTxn &txn,
                             const ClassId &classId,
                             const std::string &propertyName,
                             const PropertyDescriptor &propertyDescriptor) {
        if (auto classDescriptorPtr = find(txn, classId)) {
            auto properties = classDescriptorPtr->properties.getLatestVersion();
            auto newProperties = properties.first;
            newProperties.emplace(propertyName, propertyDescriptor);
            classDescriptorPtr->properties.addLatestVersion(newProperties);
            txn.addUncommittedSchema(classDescriptorPtr);
        }
    }

    void Schema::addProperty(BaseTxn &txn, const ClassId &classId, const ClassProperty &classProperty) {
        if (auto classDescriptorPtr = find(txn, classId)) {
            classDescriptorPtr->properties.addLatestVersion(classProperty);
            txn.addUncommittedSchema(classDescriptorPtr);
        }
    }

    void Schema::deleteProperty(BaseTxn &txn, const ClassId &classId, const std::string &propertyName) {
        if (auto classDescriptorPtr = find(txn, classId)) {
            auto properties = classDescriptorPtr->properties.getLatestVersion();
            require(properties.second);
            auto newProperties = properties.first;
            newProperties.erase(propertyName);
            classDescriptorPtr->properties.addLatestVersion(newProperties);
            txn.addUncommittedSchema(classDescriptorPtr);
        }
    }

    void Schema::updateProperty(BaseTxn &txn,
                                const ClassId &classId,
                                const std::string &oldPropertyName,
                                const std::string &newPropertyName) {
        if (auto classDescriptorPtr = find(txn, classId)) {
            auto properties = classDescriptorPtr->properties.getLatestVersion();
            require(properties.second);
            auto newProperties = properties.first;
            auto iter = newProperties.find(oldPropertyName);
            require(iter != newProperties.cend());
            auto propertyDescriptor = iter->second;
            newProperties.erase(iter);
            newProperties.emplace(newPropertyName, propertyDescriptor);
            classDescriptorPtr->properties.addLatestVersion(newProperties);
            txn.addUncommittedSchema(classDescriptorPtr);
        }
    }

    void Schema::updateProperty(BaseTxn &txn,
                                const ClassId &classId,
                                const std::string &propertyName,
                                const PropertyDescriptor &propertyDescriptor) {
        if (auto classDescriptorPtr = find(txn, classId)) {
            auto properties = classDescriptorPtr->properties.getLatestVersion();
            require(properties.second);
            auto newProperties = properties.first;
            newProperties.erase(propertyName);
            newProperties.emplace(propertyName, propertyDescriptor);
            classDescriptorPtr->properties.addLatestVersion(newProperties);
            txn.addUncommittedSchema(classDescriptorPtr);
        }
    }

    void Schema::apply(BaseTxn &txn, const InheritanceInfo &info) {
        for (const auto &inheritance: info) {
            auto subClassId = inheritance.first;
            auto superClassId = inheritance.second;
            if (superClassId > 0) {
                auto subClassDescriptorPtr = find(txn, subClassId);
                require(subClassDescriptorPtr != nullptr);
                auto superClassDescriptorPtr = find(txn, superClassId);
                require(superClassDescriptorPtr != nullptr);
                subClassDescriptorPtr->super.addLatestVersion(superClassDescriptorPtr);
                txn.addUncommittedSchema(subClassDescriptorPtr);
                auto subClasses = superClassDescriptorPtr->sub.getLatestVersion().first;
                subClasses.emplace_back(subClassDescriptorPtr);
                superClassDescriptorPtr->sub.addLatestVersion(subClasses);
                txn.addUncommittedSchema(superClassDescriptorPtr);
            }
        }
    }

    nogdb::ClassDescriptor Schema::ClassDescriptor::transform(const BaseTxn &txn) const {
        auto convertProperty = [](const ClassProperty &properties) {
            auto tmpProperties = nogdb::ClassProperty{};
            for (const auto &property: properties) {
                tmpProperties.emplace(property.first, property.second.transform());
            }
            return tmpProperties;
        };
        auto className = BaseTxn::getCurrentVersion(txn, name).first;
        auto classProperties = BaseTxn::getCurrentVersion(txn, properties).first;
        auto classDescriptor = nogdb::ClassDescriptor{id, className, type, convertProperty(classProperties)};
        if (auto superClassDescriptorPtr = BaseTxn::getCurrentVersion(txn, super).first.lock()) {
            classDescriptor.super = BaseTxn::getCurrentVersion(txn, superClassDescriptorPtr->name).first;
            for (const auto &property: convertProperty(
                    BaseTxn::getCurrentVersion(txn, superClassDescriptorPtr->properties).first)) {
                classDescriptor.properties.emplace(property);
            }
            while (auto desc = BaseTxn::getCurrentVersion(txn, superClassDescriptorPtr->super).first.lock()) {
                for (const auto &property: convertProperty(BaseTxn::getCurrentVersion(txn, desc->properties).first)) {
                    classDescriptor.properties.emplace(property);
                }
                superClassDescriptorPtr = desc;
            }
        }
        for (const auto &subClassDescriptor: BaseTxn::getCurrentVersion(txn, sub).first) {
            if (auto subClassDescriptorPtr = subClassDescriptor.lock()) {
                classDescriptor.sub.push_back(BaseTxn::getCurrentVersion(txn, subClassDescriptorPtr->name).first);
            }
        }
        return classDescriptor;
    }

    std::map<std::string, std::weak_ptr<Schema::ClassDescriptor>>
    Schema::getNameToDescMapping(const BaseTxn &txn) {
        auto result = std::map<std::string, std::weak_ptr<ClassDescriptor>>{};
        if (txn.getType() == BaseTxn::TxnType::READ_WRITE) {
            for (const auto &element: txn.findUncommittedSchema()) {
                auto state = element.second->getState();
                if (state.second == TxnObject::StatusFlag::UNCOMMITTED_CREATE) {
                    result.emplace(element.second->name.getUnstableVersion().first, element.second);
                }
            }
        }
        RWSpinLockGuard<RWSpinLock> _(schemaInfo.splock);
        for (const auto &element: schemaInfo.elements) {
            if (auto classDescPtr = element.second) {
                if ((txn.getType() == BaseTxn::TxnType::READ_ONLY && classDescPtr->checkReadOnly(txn.getVersionId())) ||
                    (txn.getType() == BaseTxn::TxnType::READ_WRITE && classDescPtr->checkReadWrite())) {
                    continue;
                }
                auto className = BaseTxn::getCurrentVersion(txn, classDescPtr->name).first;
                if (!className.empty()) {
                    result.emplace(className, classDescPtr);
                }
            }
        }
        return result;
    }


}
