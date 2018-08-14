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

#ifndef __SCHEMA_HPP_INCLUDED_
#define __SCHEMA_HPP_INCLUDED_

#include <vector>

#include "schema_adapter.hpp"

#include "nogdb_txn.h"
#include "nogdb_types.h"

namespace nogdb {

    namespace schema {

        using adapter::schema::PropertyAccessInfo;
        using adapter::schema::PropertyNameMapInfo;

        class SchemaInterface {
        public:
            SchemaInterface(const Txn* txn): _txn{txn} {}

            virtual ~SchemaInterface() noexcept = default;

            std::vector<PropertyAccessInfo>
            getNativePropertyInfo(const ClassId& classId) {
                auto result = std::vector<PropertyAccessInfo>{};
                for (const auto &propertyInfo: _txn->_property->getInfos(classId)) {
                    result.emplace_back(propertyInfo);
                }
                return result;
            }

            std::vector<PropertyAccessInfo>
            getInheritPropertyInfo(const ClassId& superClassId) {
                auto result = std::vector<PropertyAccessInfo>{};
                if (superClassId != ClassId{}) {
                    auto nativeProperties = _txn->_property->getInfos(superClassId);
                    result.insert(result.cend(), nativeProperties.cbegin(), nativeProperties.cend());
                    auto inheritProperties = getInheritPropertyInfo(_txn->_class->getSuperClassId(superClassId));
                    result.insert(result.cend(), inheritProperties.cbegin(), inheritProperties.cend());
                }
                return result;
            }

            PropertyNameMapInfo
            getPropertyNameMapInfo(const ClassId& classId, const ClassId& superClassId) {
                auto result = PropertyNameMapInfo{};
                for(const auto& property: getNativePropertyInfo(classId)) {
                    result[property.name] = property;
                }
                for(const auto& property: getInheritPropertyInfo(superClassId)) {
                    result[property.name] = property;
                }
                return result;
            }

        private:
            const Txn *_txn;

        };

    }

}

#endif
