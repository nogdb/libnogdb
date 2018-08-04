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

#ifndef __SCHEMA_ADAPTER_HPP_INCLUDED_
#define __SCHEMA_ADAPTER_HPP_INCLUDED_

#include <unordered_map>
#include <map>
#include <utility>

#include "storage_adapter.hpp"
#include "constant.hpp"
#include "dbinfo_adapter.hpp"

namespace nogdb {

    namespace adapter {

        namespace schema {

            struct ClassAccessInfo {
                ClassId id{0};
                std::string name{""};
                ClassId superClassId{0};
                ClassType type{ClassType::UNDEFINED};
            };

            class ClassAccess : public storage_engine::adapter::LMDBKeyValAccess {
            public:
                ClassAccess(const storage_engine::LMDBTxn * const txn)
                        : LMDBKeyValAccess(txn, TB_CLASSES, true, true, true, true) {}

                virtual ~ClassAccess() noexcept = default;

                void createOrUpdate(const ClassAccessInfo& props) {
                    auto totalLength = sizeof(props.type) + sizeof(ClassId) + props.name.length();
                    auto value = Blob(totalLength);
                    value.append(&props.type, sizeof(props.type));
                    value.append(&props.superClassId, sizeof(ClassId));
                    value.append(props.name.c_str(), props.name.length());
                    put(props.id, value);
                    // update cache
                    _classNameMapping.emplace({props.name, props.id});
                }

                void remove(const ClassId& classId) {
                    del(classId);
                }

                void remove(const std::string& className) {
                    auto classIdIter = _classNameMapping.find(className);
                    if (classIdIter != _classNameMapping.cend()) {
                        // found in cache, then
                        remove(classIdIter->second);
                        // update cache
                        _classNameMapping.erase(className);
                    } else {
                        auto classId = getId(className);
                        if (classId != ClassId{}) {
                            remove(classId);
                        }
                    }
                }

                ClassAccessInfo getInfo(const ClassId& classId) const {
                    auto result = get(classId);
                    if (result.empty) {
                        return ClassAccessInfo{};
                    } else {
                        return parse(classId, result.data.blob());
                    }
                }

                ClassAccessInfo getInfo(const std::string& className) const {
                    auto classIdIter = _classNameMapping.find(className);
                    if (classIdIter != _classNameMapping.cend()) {
                        // found in cache, then
                        return getInfo(classIdIter->second);
                    } else {
                        // not found in cache, then
                        auto cursorHandler = cursor();
                        for(auto keyValue = cursorHandler.getNext();
                            !keyValue.empty();
                            keyValue = cursorHandler.getNext()) {
                            auto classId = keyValue.key.data.numeric<ClassId>();
                            if (classId == UINT16_EM_INIT) continue;
                            auto blob = keyValue.val.data.blob();
                            if (className == parseClassName(blob)) {
                                auto superClassId = parseSuperClassId(blob);
                                auto type = parseClassType(blob);
                                // update cache
                                _classNameMapping.emplace(className, classId);
                                return ClassAccessInfo{classId, className, superClassId, type};
                            }
                        }
                        return ClassAccessInfo{};
                    }
                }

                std::string getName(const ClassId& classId) const {
                    auto result = get(classId);
                    if (!result.empty) {
                        return parseClassName(result.data.blob());
                    } else {
                        return std::string{};
                    }
                }

                ClassId getId(const std::string& className) const {
                    auto classIdIter = _classNameMapping.find(className);
                    if (classIdIter != _classNameMapping.cend()) {
                        // found in cache, then
                        return classIdIter->second;
                    } else {
                        // not found in cache, then
                        auto cursorHandler = cursor();
                        for(auto keyValue = cursorHandler.getNext();
                            !keyValue.empty();
                            keyValue = cursorHandler.getNext()) {
                            auto classId = keyValue.key.data.numeric<ClassId>();
                            if (classId == UINT16_EM_INIT) continue;
                            if (className == parseClassName(keyValue.val.data.blob())) {
                                // update cache
                                _classNameMapping.emplace(className, classId);
                                return classId;
                            }
                        }
                        return ClassId{};
                    }
                }

            private:
                mutable std::unordered_map<std::string, ClassId> _classNameMapping{};

                static ClassAccessInfo parse(const ClassId& classId, const Blob& blob) {
                    return ClassAccessInfo{
                        classId,
                        parseClassName(blob),
                        parseSuperClassId(blob),
                        parseClassType(blob)
                    };
                }

                static ClassType parseClassType(const Blob& blob) {
                    auto classType = ClassType::UNDEFINED;
                    blob.retrieve(&classType, 0, sizeof(ClassType));
                    return classType;
                }

                static ClassId parseSuperClassId(const Blob& blob) {
                    auto superClassId = ClassId{};
                    blob.retrieve(&superClassId, sizeof(ClassType), sizeof(superClassId));
                    return superClassId;
                }

                static std::string parseClassName(const Blob& blob) {
                    auto offset = sizeof(ClassType) + sizeof(ClassId);
                    auto nameLength = blob.size() - offset;
                    require(nameLength > 0);
                    Blob::Byte nameBytes[nameLength];
                    blob.retrieve(nameBytes, offset, nameLength);
                    return std::string(reinterpret_cast<char *>(nameBytes), nameLength);
                }

            };

            struct PropertyAccessInfo {
                PropertyId  id{0};
                std::string name{""};
                ClassId classId{0};
                PropertyType type{PropertyType::UNDEFINED};
            };

            class PropertyAccess : public storage_engine::adapter::LMDBKeyValAccess {
            public:
                PropertyAccess(const storage_engine::LMDBTxn * const txn)
                        : LMDBKeyValAccess(txn, TB_CLASSES, true, true, true, true) {}

                virtual ~PropertyAccess() noexcept = default;

                void createOrUpdate(const PropertyAccessInfo& props) {
                    auto totalLength = sizeof(PropertyType) + sizeof(ClassId) + props.name.length();
                    auto value = Blob(totalLength);
                    value.append(&props.type, sizeof(PropertyType));
                    value.append(&props.classId, sizeof(ClassId));
                    value.append(props.name.c_str(), props.name.length());
                    put(props.id, value);
                    // update cache
                    _propertyNameMapping.emplace({PropertyInfoKey{props.classId, props.name}, props.id});
                }

                void remove(const PropertyId& propertyId) {
                    del(propertyId);
                }

                void remove(const ClassId& classId, const std::string& propertyName) {
                    auto propertyInfoKey = PropertyInfoKey{classId, propertyName};
                    auto propertyIdIter = _propertyNameMapping.find(propertyInfoKey);
                    if (propertyIdIter != _propertyNameMapping.cend()) {
                        // found in cache, then
                        remove(propertyIdIter->second);
                        // update cache
                        _propertyNameMapping.erase(propertyInfoKey);
                    } else {
                        auto propertyId = getId(classId, propertyName);
                        if (propertyId != PropertyId{}) {
                            remove(propertyId);
                        }
                    }
                }

                PropertyAccessInfo getInfo(const PropertyId& propertyId) const {
                    auto result = get(propertyId);
                    if (result.empty) {
                        return PropertyAccessInfo{};
                    } else {
                        return parse(propertyId, result.data.blob());
                    }
                }

                PropertyAccessInfo getInfo(const ClassId& classId, const std::string& propertyName) const {
                    auto propertyInfoKey = PropertyInfoKey{classId, propertyName};
                    auto propertyIdIter = _propertyNameMapping.find(propertyInfoKey);
                    if (propertyIdIter != _propertyNameMapping.cend()) {
                        // found in cache, then
                        return getInfo(propertyIdIter->second);
                    } else {
                        // not found in cache, then
                        auto cursorHandler = cursor();
                        for(auto keyValue = cursorHandler.getNext();
                            !keyValue.empty();
                            keyValue = cursorHandler.getNext()) {
                            auto propertyId = keyValue.key.data.numeric<PropertyId>();
                            if (propertyId == UINT16_EM_INIT) continue;
                            auto blob = keyValue.val.data.blob();
                            if (propertyName == parsePropertyName(blob) &&
                                classId == parseClassId(blob)) {
                                auto type = parsePropertyType(blob);
                                // update cache
                                _propertyNameMapping.emplace(propertyInfoKey, propertyId);
                                return PropertyAccessInfo{propertyId, propertyName, classId, type};
                            }
                        }
                        return PropertyAccessInfo{};
                    }
                }

                std::string getName(const ClassId& classId, const std::string& propertyName) const {
                    auto result = get(classId);
                    if (!result.empty) {
                        return parsePropertyName(result.data.blob());
                    } else {
                        return std::string{};
                    }
                }

                PropertyId getId(const ClassId& classId, const std::string& propertyName) const {
                    auto propertyInfoKey = PropertyInfoKey{classId, propertyName};
                    auto propertyIdIter = _propertyNameMapping.find(propertyInfoKey);
                    if (propertyIdIter != _propertyNameMapping.cend()) {
                        // found in cache, then
                        return propertyIdIter->second;
                    } else {
                        // not found in cache, then
                        auto cursorHandler = cursor();
                        for(auto keyValue = cursorHandler.getNext();
                            !keyValue.empty();
                            keyValue = cursorHandler.getNext()) {
                            auto propertyId = keyValue.key.data.numeric<PropertyId>();
                            if (propertyId == UINT16_EM_INIT) continue;
                            auto blob = keyValue.val.data.blob();
                            if (propertyName == parsePropertyName(blob) &&
                                classId == parseClassId(blob)) {
                                // update cache
                                _propertyNameMapping.emplace(propertyInfoKey, propertyId);
                                return propertyId;
                            }
                        }
                        return PropertyId{};
                    }
                }

            private:

                typedef std::pair<ClassId, std::string>    PropertyInfoKey;

                mutable std::map<PropertyInfoKey, PropertyId> _propertyNameMapping{};

                static PropertyAccessInfo parse(const PropertyId& propertyId, const Blob& blob) {
                    return PropertyAccessInfo{
                            propertyId,
                            parsePropertyName(blob),
                            parseClassId(blob),
                            parsePropertyType(blob)
                    };
                }

                static PropertyType parsePropertyType(const Blob& blob) {
                    auto propertyType = PropertyType::UNDEFINED;
                    blob.retrieve(&propertyType, 0, sizeof(PropertyType));
                    return propertyType;
                }

                static ClassId parseClassId(const Blob& blob) {
                    auto classId = ClassId{};
                    blob.retrieve(&classId, sizeof(PropertyType), sizeof(ClassId));
                    return classId;
                }

                static std::string parsePropertyName(const Blob& blob) {
                    auto offset = sizeof(PropertyType) + sizeof(ClassId);
                    auto nameLength = blob.size() - offset;
                    require(nameLength > 0);
                    Blob::Byte nameBytes[nameLength];
                    blob.retrieve(nameBytes, offset, nameLength);
                    return std::string(reinterpret_cast<char *>(nameBytes), nameLength);
                }

            };

            class IndexAccess : public storage_engine::adapter::LMDBKeyValAccess {
                //TODO
            };

            class SchemaAccess {
                //TODO
            };

        }
    }
}


#endif //__SCHEMA_ADAPTER_HPP_INCLUDED_
