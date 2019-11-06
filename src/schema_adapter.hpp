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

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "constant.hpp"
#include "dbinfo_adapter.hpp"
#include "storage_adapter.hpp"
#include "utils.hpp"

namespace nogdb {
namespace adapter {
namespace schema {
    using namespace internal_data_type;
    using namespace utils::string;
    using namespace utils::assertion;

    /**
     * Raw record format in lmdb data storage:
     * {name<string>} -> {id<uint16>}{superClassId<uint16>}{type<char>}
     */
    struct ClassAccessInfo {
        ClassAccessInfo() = default;

        ClassAccessInfo(const std::string& _name, const ClassId& _id, const ClassId& _superClassId,
            const ClassType& _type)
            : name { _name }
            , id { _id }
            , superClassId { _superClassId }
            , type { _type }
        {
        }

        std::string name { "" };
        ClassId id { 0 };
        ClassId superClassId { 0 };
        ClassType type { ClassType::UNDEFINED };
    };

    class ClassAccess : public storage_engine::adapter::LMDBKeyValAccess {
    public:
        ClassAccess() = default;

        ClassAccess(const storage_engine::LMDBTxn* const txn)
            : LMDBKeyValAccess(txn, TB_CLASSES, false, true, false, true)
        {
        }

        virtual ~ClassAccess() noexcept = default;

        ClassAccess(ClassAccess&& other) noexcept
        {
            *this = std::move(other);
        }

        ClassAccess& operator=(ClassAccess&& other) noexcept
        {
            if (this != &other) {
                using std::swap;
                swap(_classCache, other._classCache);
                other._classCache.clear();
            }
            return *this;
        }

        void create(const ClassAccessInfo& props)
        {
            auto result = get(props.name);
            if (result.empty) {
                createOrUpdate(props);
                _classCache.set(props.id, props);
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_CLASS);
            }
        }

        void update(const ClassAccessInfo& props)
        {
            auto result = get(props.name);
            if (!result.empty) {
                createOrUpdate(props);
                _classCache.set(props.id, props);
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_CLASS);
            }
        }

        void remove(const std::string& className)
        {
            auto result = get(className);
            if (!result.empty) {
                del(className);
                auto classId = parseClassId(result.data.blob());
                _classCache.unset(classId);
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_CLASS);
            }
        }

        void alterClassName(const std::string& oldName, const std::string& newName)
        {
            auto result = get(oldName);
            if (!result.empty) {
                auto newResult = get(newName);
                if (newResult.empty) {
                    auto blob = result.data.blob();
                    del(oldName);
                    put(newName, blob);
                    auto info = parse(newName, blob);
                    _classCache.set(info.id, info);
                } else {
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_CLASS);
                }
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_CLASS);
            }
        }

        ClassAccessInfo getInfo(const std::string& className) const
        {
            auto result = get(className);
            if (result.empty) {
                return ClassAccessInfo {};
            } else {
                return parse(className, result.data.blob());
            }
        }

        ClassAccessInfo getInfo(const ClassId& classId) const
        {
            std::function<ClassAccessInfo(void)> callback = [&]() {
                auto cursorHandler = cursor();
                for (auto keyValue = cursorHandler.getNext();
                     !keyValue.empty();
                     keyValue = cursorHandler.getNext()) {
                    auto classIdValue = parseClassId(keyValue.val.data.blob());
                    if (classId != classIdValue)
                        continue;
                    auto key = keyValue.key.data.string();
                    return parse(key, keyValue.val.data.blob());
                }
                return ClassAccessInfo {};
            };
            return _classCache.get(classId, callback);
        }

        std::vector<ClassAccessInfo> getAllInfos() const
        {
            auto result = std::vector<ClassAccessInfo> {};
            auto cursorHandler = cursor();
            for (auto keyValue = cursorHandler.getNext();
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto info = parse(keyValue.key.data.string(), keyValue.val.data.blob());
                result.emplace_back(info);
            }
            return result;
        }

        ClassId getId(const std::string& className) const
        {
            auto result = get(className);
            if (result.empty) {
                return ClassId {};
            } else {
                return parseClassId(result.data.blob());
            }
        }

        ClassId getSuperClassId(const ClassId& classId) const
        {
            return getInfo(classId).superClassId;
        }

        std::set<ClassId> getSubClassIds(const ClassId& classId) const
        {
            //TODO: can we improve the performance for this?
            auto result = std::set<ClassId> {};
            auto cursorHandler = cursor();
            for (auto keyValue = cursorHandler.getNext();
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto superClassIdValue = parseSuperClassId(keyValue.val.data.blob());
                if (classId != superClassIdValue)
                    continue;
                result.insert(parseClassId(keyValue.val.data.blob()));
            }
            return result;
        }

        std::vector<ClassAccessInfo> getSubClassInfos(const ClassId& classId) const
        {
            //TODO: can we improve the performance for this?
            auto result = std::vector<ClassAccessInfo> {};
            auto cursorHandler = cursor();
            for (auto keyValue = cursorHandler.getNext();
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto superClassIdValue = parseSuperClassId(keyValue.val.data.blob());
                if (classId != superClassIdValue)
                    continue;
                auto key = keyValue.key.data.string();
                result.emplace_back(parse(key, keyValue.val.data.blob()));
            }
            return result;
        }

    protected:
        static ClassAccessInfo parse(const std::string& className, const Blob& blob)
        {
            return ClassAccessInfo {
                className,
                parseClassId(blob),
                parseSuperClassId(blob),
                parseClassType(blob)
            };
        }

        static ClassId parseClassId(const Blob& blob)
        {
            auto classId = ClassId {};
            blob.retrieve(&classId, 0, sizeof(classId));
            return classId;
        }

        static ClassId parseSuperClassId(const Blob& blob)
        {
            auto superClassId = ClassId {};
            blob.retrieve(&superClassId, sizeof(ClassId), sizeof(superClassId));
            return superClassId;
        }

        static ClassType parseClassType(const Blob& blob)
        {
            auto classType = ClassType::UNDEFINED;
            blob.retrieve(&classType, 2 * sizeof(ClassId), sizeof(ClassType));
            return classType;
        }

    private:
        using InternalCache = utils::caching::UnorderedCache<ClassId, ClassAccessInfo>;
        InternalCache _classCache {};

        void createOrUpdate(const ClassAccessInfo& props)
        {
            auto totalLength = 2 * sizeof(ClassId) + sizeof(props.type);
            auto value = Blob(totalLength);
            value.append(&props.id, sizeof(ClassId));
            value.append(&props.superClassId, sizeof(ClassId));
            value.append(&props.type, sizeof(props.type));
            put(props.name, value);
        }
    };

    /**
   * Raw record format in lmdb data storage:
   * {classId<string>:name<string+padding>} -> {id<uint16>}{type<char>}
   */
    struct PropertyAccessInfo {
        PropertyAccessInfo() = default;

        PropertyAccessInfo(const ClassId& _classId, const std::string& _name, const PropertyId& _id,
            const PropertyType& _type)
            : classId { _classId }
            , name { _name }
            , id { _id }
            , type { _type }
        {
        }

        ClassId classId { 0 };
        std::string name { "" };
        PropertyId id { 0 };
        PropertyType type { PropertyType::UNDEFINED };
    };

    typedef std::map<std::string, PropertyAccessInfo> PropertyNameMapInfo;
    typedef std::map<PropertyId, PropertyAccessInfo> PropertyIdMapInfo;

    constexpr char KEY_SEPARATOR = ':';
    constexpr char KEY_PADDING = ' ';
    const std::string KEY_SEARCH_BEGIN = frontPadding("", MAX_PROPERTY_NAME_LEN, KEY_PADDING);

    class PropertyAccess : public storage_engine::adapter::LMDBKeyValAccess {
    public:
        PropertyAccess() = default;

        PropertyAccess(const storage_engine::LMDBTxn* const txn)
            : LMDBKeyValAccess(txn, TB_PROPERTIES, false, true, false, false)
        {
        }

        virtual ~PropertyAccess() noexcept = default;

        PropertyAccess(PropertyAccess&& other) noexcept = default;

        PropertyAccess& operator=(PropertyAccess&& other) noexcept = default;

        void create(const PropertyAccessInfo& props)
        {
            auto propertyKey = buildKey(props.classId, props.name);
            auto result = get(propertyKey);
            if (result.empty) {
                createOrUpdate(props);
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_PROPERTY);
            }
        }

        void remove(const ClassId& classId, const std::string& propertyName)
        {
            auto propertyKey = buildKey(classId, propertyName);
            auto result = get(propertyKey);
            if (!result.empty) {
                del(propertyKey);
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
            }
        }

        void alterPropertyName(const ClassId& classId, const std::string& oldName, const std::string& newName)
        {
            auto propertyKey = buildKey(classId, oldName);
            auto result = get(propertyKey);
            if (!result.empty) {
                auto newPropertyKey = buildKey(classId, newName);
                auto newResult = get(newPropertyKey);
                if (newResult.empty) {
                    auto props = parse(classId, newName, result.data.blob());
                    del(propertyKey);
                    createOrUpdate(props);
                } else {
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_PROPERTY);
                }
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
            }
        }

        PropertyAccessInfo getInfo(const ClassId& classId, const std::string& propertyName) const
        {
            auto propertyKey = buildKey(classId, propertyName);
            auto result = get(propertyKey);
            if (result.empty) {
                return PropertyAccessInfo {};
            } else {
                return parse(classId, propertyName, result.data.blob());
            }
        }

        std::vector<PropertyAccessInfo> getInfos(const ClassId& classId) const
        {
            auto result = std::vector<PropertyAccessInfo> {};
            auto cursorHandler = cursor();
            for (auto keyValue = cursorHandler.findRange(buildSearchKeyBegin(classId));
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto keyPair = splitKey(keyValue.key.data.string());
                auto& classIdKey = keyPair.first;
                auto& propertyNameKey = keyPair.second;
                if (classId != classIdKey)
                    break;
                result.emplace_back(parse(classIdKey, propertyNameKey, keyValue.val.data.blob()));
            }
            return result;
        }

        PropertyId getId(const ClassId& classId, const std::string& propertyName) const
        {
            auto propertyKey = buildKey(classId, propertyName);
            auto result = get(propertyKey);
            if (result.empty) {
                return PropertyId {};
            } else {
                return parsePropertyId(result.data.blob());
            }
        }

        PropertyNameMapInfo getNameMapInfo(const ClassId& classId) const
        {
            auto result = PropertyNameMapInfo {};
            for (const auto& property : getInfos(classId)) {
                result[property.name] = property;
            }
            return result;
        }

        PropertyIdMapInfo getIdMapInfo(const ClassId& classId) const
        {
            auto result = PropertyIdMapInfo {};
            for (const auto& property : getInfos(classId)) {
                result[property.id] = property;
            }
            return result;
        }

    protected:
        static PropertyAccessInfo parse(const ClassId& classId, const std::string& propertyName, const Blob& blob)
        {
            return PropertyAccessInfo {
                classId,
                propertyName,
                parsePropertyId(blob),
                parsePropertyType(blob)
            };
        }

        static PropertyId parsePropertyId(const Blob& blob)
        {
            auto propertyId = PropertyId {};
            blob.retrieve(&propertyId, 0, sizeof(PropertyId));
            return propertyId;
        }

        static PropertyType parsePropertyType(const Blob& blob)
        {
            auto propertyType = PropertyType::UNDEFINED;
            blob.retrieve(&propertyType, sizeof(PropertyId), sizeof(PropertyType));
            return propertyType;
        }

    private:
        void createOrUpdate(const PropertyAccessInfo& props)
        {
            auto totalLength = sizeof(PropertyId) + sizeof(PropertyType);
            auto value = Blob(totalLength);
            value.append(&props.id, sizeof(PropertyId));
            value.append(&props.type, sizeof(PropertyType));
            put(buildKey(props.classId, props.name), value);
        }

        std::string buildKey(const ClassId& classId, const std::string& propertyName) const
        {
            return std::to_string(classId) + std::string { KEY_SEPARATOR } + frontPadding(propertyName, MAX_PROPERTY_NAME_LEN, KEY_PADDING);
        }

        std::string buildSearchKeyBegin(const ClassId& classId) const
        {
            return std::to_string(classId) + std::string { KEY_SEPARATOR } + KEY_SEARCH_BEGIN;
        }

        std::pair<ClassId, std::string> splitKey(const std::string& key) const
        {
            auto splitKey = utils::string::split(key, KEY_SEPARATOR);
            require(splitKey.size() == 2);
            auto classId = static_cast<ClassId>(std::strtoul(splitKey[0].c_str(), nullptr, 0));
            auto propertyName = splitKey[1];
            ltrim(propertyName);
            return std::make_pair(classId, propertyName);
        };

        inline void ltrim(std::string& str) const
        {
            str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
                return !std::isspace(ch);
            }));
        }
    };

    /**
   * Raw record format in lmdb data storage:
   * {classId<uint16>}{propertyId<uint16>} -> {id<uint16>}{isUnique<uint8>}
   */
    struct IndexAccessInfo {
        IndexAccessInfo() = default;

        IndexAccessInfo(const ClassId& _classId, const PropertyId& _propertyId, const IndexId& _id, bool _isUnique)
            : classId { _classId }
            , propertyId { _propertyId }
            , id { _id }
            , isUnique { _isUnique }
        {
        }

        ClassId classId { 0 };
        PropertyId propertyId { 0 };
        IndexId id { 0 };
        bool isUnique { true };
    };

    class IndexAccess : public storage_engine::adapter::LMDBKeyValAccess {
    public:
        IndexAccess() = default;

        IndexAccess(const storage_engine::LMDBTxn* const txn)
            : LMDBKeyValAccess(txn, TB_INDEXES, true, true, false, false)
        {
        }

        virtual ~IndexAccess() noexcept = default;

        IndexAccess(IndexAccess&& other) noexcept = default;

        IndexAccess& operator=(IndexAccess&& other) noexcept = default;

        void create(const IndexAccessInfo& props)
        {
            auto indexKey = buildKey(props.classId, props.propertyId);
            auto result = get(indexKey);
            if (result.empty) {
                createOrUpdate(props);
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_DUPLICATE_INDEX);
            }
        }

        void remove(const ClassId& classId, const PropertyId& propertyId)
        {
            auto indexKey = buildKey(classId, propertyId);
            auto result = get(indexKey);
            if (!result.empty) {
                del(indexKey);
            } else {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_INDEX);
            }
        }

        IndexAccessInfo getInfo(const ClassId& classId, const PropertyId& propertyId) const
        {
            auto indexKey = buildKey(classId, propertyId);
            auto result = get(indexKey);
            if (result.empty) {
                return IndexAccessInfo {};
            } else {
                return parse(classId, propertyId, result.data.blob());
            }
        }

        std::vector<IndexAccessInfo> getInfos(const ClassId& classId) const
        {
            auto result = std::vector<IndexAccessInfo> {};
            auto cursorHandler = cursor();
            for (auto keyValue = cursorHandler.findRange(buildSearchKeyBegin(classId));
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto keyPair = keyValue.key.data.numeric<IndexKey>();
                auto classIdKey = getClassIdFromKey(keyPair);
                auto propertyIdKey = getPropertyIdFromKey(keyPair);
                if (classId != classIdKey)
                    break;
                result.emplace_back(parse(classIdKey, propertyIdKey, keyValue.val.data.blob()));
            }
            return result;
        }

    protected:
        using IndexKey = uint32_t;

        static IndexAccessInfo parse(const ClassId& classId, const PropertyId& propertyId, const Blob& blob)
        {
            return IndexAccessInfo {
                classId,
                propertyId,
                parseIndexId(blob),
                parseIsUnique(blob)
            };
        }

        static IndexId parseIndexId(const Blob& blob)
        {
            auto indexId = IndexId {};
            blob.retrieve(&indexId, 0, sizeof(IndexId));
            return indexId;
        }

        static bool parseIsUnique(const Blob& blob)
        {
            auto isUnique = uint8_t {};
            blob.retrieve(&isUnique, sizeof(IndexId), sizeof(uint8_t));
            return isUnique == 1;
        }

    private:
        void createOrUpdate(const IndexAccessInfo& props)
        {
            auto totalLength = sizeof(IndexId) + sizeof(uint8_t);
            auto value = Blob(totalLength);
            value.append(&props.id, sizeof(IndexId));
            auto isUnique = (props.isUnique) ? uint8_t { 1 } : uint8_t { 0 };
            value.append(&isUnique, sizeof(isUnique));
            put(buildKey(props.classId, props.propertyId), value);
        }

        IndexKey buildKey(const ClassId& classId, const PropertyId& propertyId) const
        {
            return classId << 16 | propertyId;
        }

        IndexKey buildSearchKeyBegin(const ClassId& classId) const
        {
            return classId << 16;
        }

        ClassId getClassIdFromKey(const IndexKey& indexKey) const
        {
            return static_cast<ClassId>(indexKey >> 16);
        }

        PropertyId getPropertyIdFromKey(const IndexKey& indexKey) const
        {
            return static_cast<PropertyId>(indexKey & 0xffff);
        }
    };

}
}
}