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

#ifndef __SCHEMA_HPP_INCLUDED_
#define __SCHEMA_HPP_INCLUDED_

#include <map>
#include <memory>

#include "spinlock.hpp"
#include "concurrent.hpp"
#include "graph.hpp"

#include "nogdb_types.h"

namespace nogdb {

    class BaseTxn;

    struct Schema {
        Schema() = default;

        ~Schema() noexcept = default;

        typedef std::multimap<ClassId, std::pair<IndexId, bool>> IndexInfo;

        struct PropertyDescriptor {
            PropertyDescriptor() = default;

            PropertyDescriptor(PropertyId id_, PropertyType type_) : id{id_}, type{type_} {}

            nogdb::PropertyDescriptor transform() const {
                auto toIndexInfo = nogdb::IndexInfo{};
                for (const auto &index: indexInfo) {
                    toIndexInfo.emplace(index.second.first, std::make_pair(index.first, index.second.second));
                }
                return nogdb::PropertyDescriptor{id, type, toIndexInfo};
            }

            PropertyId id{0};
            PropertyType type{PropertyType::UNDEFINED};
            IndexInfo indexInfo{};
        };

        typedef std::map<std::string, PropertyDescriptor> ClassProperty;
        typedef std::vector<std::pair<ClassId, ClassId>> InheritanceInfo;

        template<typename Key, typename T>
        using SchemaElements = std::map<Key, std::shared_ptr<T>>;

        template<typename Key, typename T>
        using ConcurrentSchemaElements = ConcurrentHashMap<SchemaElements<Key, T>, Key, T>;

        struct ClassDescriptor : public TxnObject {
            ClassDescriptor() = default;

            ClassDescriptor(const ClassId id_, const std::string &name_, ClassType type_)
                    : id{id_}, type{type_} {
                name.addLatestVersion(name_);
            }

            nogdb::ClassDescriptor transform(const BaseTxn &txn) const;

            friend bool operator<(const ClassDescriptor &lhs, const ClassDescriptor &rhs) {
                return lhs.id < rhs.id;
            }

            ClassId id{0};
            VersionControl<std::string> name{};
            ClassType type{ClassType::UNDEFINED};
            VersionControl<ClassProperty> properties{};
            VersionControl<std::weak_ptr<ClassDescriptor>> super{};
            VersionControl<std::vector<std::weak_ptr<ClassDescriptor>>> sub{};
        };

        typedef std::shared_ptr<ClassDescriptor> ClassDescriptorPtr;

        ConcurrentSchemaElements<ClassId, ClassDescriptor> schemaInfo;
        ConcurrentDeleteQueue<ClassId> deletedClassId;

        std::map<std::string, std::weak_ptr<ClassDescriptor>> getNameToDescMapping(const BaseTxn &txn);

        void insert(BaseTxn &txn, const ClassDescriptorPtr &classDescriptor);

        void replace(BaseTxn &txn, const ClassDescriptorPtr &classDescriptor, const std::string &newClassName);

        ClassDescriptorPtr find(const BaseTxn &txn, const std::string &className);

        ClassDescriptorPtr find(const BaseTxn &txn, const ClassId &classId);

        void erase(BaseTxn &baseTxn, const ClassId &classId) noexcept;

        void forceDelete(const std::vector<ClassId> &classId) noexcept;

        void clear() noexcept;

        void addProperty(BaseTxn &txn,
                         const ClassId &classId,
                         const std::string &propertyName,
                         const PropertyDescriptor &propertyDescriptor);

        void addProperty(BaseTxn &txn, const ClassId &classId, const ClassProperty &classProperty);

        void deleteProperty(BaseTxn &txn, const ClassId &classId, const std::string &propertyName);

        void updateProperty(BaseTxn &txn, const ClassId &classId, const std::string &oldPropertyName,
                            const std::string &newPropertyName);

        void updateProperty(BaseTxn &txn, const ClassId &classId, const std::string &propertyName,
                            const PropertyDescriptor &propertyDescriptor);

        void apply(BaseTxn &txn, const InheritanceInfo &info);

        inline void clearDeletedElements(TxnId versionId) {
            forceDelete(deletedClassId.pop_front(versionId));
        }
    };

    struct ClassPropertyInfo {
        void insert(PropertyId propertyId, const std::string &propertyName, PropertyType type) {
            idToName.emplace(std::make_pair(propertyId, propertyName));
            nameToDesc.emplace(std::make_pair(propertyName, PropertyDescriptor{propertyId, type}));
        }

        void insert(const std::string &propertyName, const Schema::PropertyDescriptor &propertyDescriptor) {
            idToName.emplace(std::make_pair(propertyDescriptor.id, propertyName));
            nameToDesc.emplace(std::make_pair(propertyName, propertyDescriptor.transform()));
        }

        std::map<PropertyId, std::string> idToName{};
        ClassProperty nameToDesc{};
    };

    struct ClassInfo {
        ClassId id;
        std::string name;
        ClassPropertyInfo propertyInfo;
    };


}

#endif
