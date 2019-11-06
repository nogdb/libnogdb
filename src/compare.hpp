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

#include <regex>
#include <utility>

#include "datarecord.hpp"
#include "index.hpp"
#include "relation.hpp"
#include "schema.hpp"

#include "nogdb/nogdb.h"
#include "nogdb/nogdb_types.h"

namespace nogdb {
namespace compare {
    using namespace utils::assertion;
    using namespace adapter::schema;
    using namespace adapter::relation;

    struct ClassFilter {
        std::set<std::string> onlyClasses;
        std::set<std::string> ignoreClasses;
    };

    class RecordCompare {
    public:
        RecordCompare() = delete;

        ~RecordCompare() noexcept = delete;

        static bool compareBytesValue(const Bytes& value, PropertyType type, const Condition& condition);

        static bool compareRecordByCondition(const Record& record,
            const PropertyType& propertyType,
            const Condition& condition);

        static bool compareRecordByCondition(const Record& record,
            const PropertyNameMapInfo& propertyNameMapInfo,
            const Condition& condition);

        static bool compareRecordByMultiCondition(const Record& record,
            const PropertyNameMapInfo& propertyNameMapInfo,
            const MultiCondition& multiCondition);

        static ClassFilter getFilterClasses(const Transaction& txn, const GraphFilter& filter);

        static RecordDescriptor filterRecord(const Transaction& txn,
            const RecordDescriptor& recordDescriptor,
            const GraphFilter& filter,
            const ClassFilter& classFilter);

        static Result filterResult(const Transaction& txn,
            const RecordDescriptor& recordDescriptor,
            const GraphFilter& filter,
            const ClassFilter& classFilter);

        static std::vector<std::pair<RecordDescriptor, RecordDescriptor>> filterIncidentEdges(const Transaction& txn,
            const RecordId& vertex,
            const Direction& direction,
            const GraphFilter& filter,
            const ClassFilter& classFilter);

        static std::vector<RecordId> resolveEdgeRecordIds(const Transaction& txn,
            const RecordId& recordId,
            const Direction& direction);

        static ResultSet compareCondition(const Transaction& txn,
            const ClassAccessInfo& classInfo,
            const PropertyNameMapInfo& propertyNameMapInfo,
            const Condition& condition,
            bool searchIndexOnly = false);

        static ResultSet compareMultiCondition(const Transaction& txn,
            const ClassAccessInfo& classInfo,
            const PropertyNameMapInfo& propertyNameMapInfo,
            const MultiCondition& conditions,
            bool searchIndexOnly = false);

        static std::vector<RecordDescriptor> compareConditionRdesc(const Transaction& txn,
            const ClassAccessInfo& classInfo,
            const PropertyNameMapInfo& propertyNameMapInfo,
            const Condition& condition,
            bool searchIndexOnly = false);

        static std::vector<RecordDescriptor> compareMultiConditionRdesc(const Transaction& txn,
            const ClassAccessInfo& classInfo,
            const PropertyNameMapInfo& propertyNameMapInfo,
            const MultiCondition& conditions,
            bool searchIndexOnly = false);

        static unsigned int compareConditionCount(const Transaction& txn,
            const ClassAccessInfo& classInfo,
            const PropertyNameMapInfo& propertyNameMapInfo,
            const Condition& condition,
            bool searchIndexOnly = false);

        static unsigned int compareMultiConditionCount(const Transaction& txn,
            const ClassAccessInfo& classInfo,
            const PropertyNameMapInfo& propertyNameMapInfo,
            const MultiCondition& conditions,
            bool searchIndexOnly = false);

        static ResultSet compareEdgeCondition(const Transaction& txn,
            const RecordDescriptor& recordDescriptor,
            const Direction& direction,
            const Condition& condition);

        static ResultSet compareEdgeCondition(const Transaction& txn,
            const RecordDescriptor& recordDescriptor,
            const Direction& direction,
            bool (*condition)(const Record&));

        static ResultSet compareEdgeMultiCondition(const Transaction& txn,
            const RecordDescriptor& recordDescriptor,
            const Direction& direction,
            const MultiCondition& multiCondition);

        static std::vector<RecordDescriptor> compareEdgeConditionRdesc(const Transaction& txn,
            const RecordDescriptor& recordDescriptor,
            const Direction& direction,
            const Condition& condition);

        static std::vector<RecordDescriptor> compareEdgeConditionRdesc(const Transaction& txn,
            const RecordDescriptor& recordDescriptor,
            const Direction& direction,
            bool (*condition)(const Record&));

        static std::vector<RecordDescriptor> compareEdgeMultiConditionRdesc(const Transaction& txn,
            const RecordDescriptor& recordDescriptor,
            const Direction& direction,
            const MultiCondition& multiCondition);

    private:
        inline static std::string toLower(const std::string& text)
        {
            auto tmp = std::string {};
            std::transform(text.cbegin(), text.cend(), std::back_inserter(tmp), ::tolower);
            return tmp;
        };

        inline static bool genericCompareFunc(const Bytes& value,
            const PropertyType& type,
            const Bytes& cmpValue1,
            const Bytes& cmpValue2,
            const Condition::Comparator& cmp,
            bool isIgnoreCase)
        {
            switch (type) {
            case PropertyType::TINYINT:
                switch (cmp) {
                case Condition::Comparator::EQUAL:
                    return value.toTinyInt() == cmpValue1.toTinyInt();
                case Condition::Comparator::GREATER:
                    return value.toTinyInt() > cmpValue1.toTinyInt();
                case Condition::Comparator::GREATER_EQUAL:
                    return value.toTinyInt() >= cmpValue1.toTinyInt();
                case Condition::Comparator::LESS:
                    return value.toTinyInt() < cmpValue1.toTinyInt();
                case Condition::Comparator::LESS_EQUAL:
                    return value.toTinyInt() <= cmpValue1.toTinyInt();
                case Condition::Comparator::BETWEEN:
                    return (cmpValue1.toTinyInt() <= value.toTinyInt()) && (value.toTinyInt() <= cmpValue2.toTinyInt());
                case Condition::Comparator::BETWEEN_NO_LOWER:
                    return (cmpValue1.toTinyInt() < value.toTinyInt()) && (value.toTinyInt() <= cmpValue2.toTinyInt());
                case Condition::Comparator::BETWEEN_NO_UPPER:
                    return (cmpValue1.toTinyInt() <= value.toTinyInt()) && (value.toTinyInt() < cmpValue2.toTinyInt());
                case Condition::Comparator::BETWEEN_NO_BOUND:
                    return (cmpValue1.toTinyInt() < value.toTinyInt()) && (value.toTinyInt() < cmpValue2.toTinyInt());
                default:
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                }
            case PropertyType::UNSIGNED_TINYINT:
                switch (cmp) {
                case Condition::Comparator::EQUAL:
                    return value.toTinyIntU() == cmpValue1.toTinyIntU();
                case Condition::Comparator::GREATER:
                    return value.toTinyIntU() > cmpValue1.toTinyIntU();
                case Condition::Comparator::GREATER_EQUAL:
                    return value.toTinyIntU() >= cmpValue1.toTinyIntU();
                case Condition::Comparator::LESS:
                    return value.toTinyIntU() < cmpValue1.toTinyIntU();
                case Condition::Comparator::LESS_EQUAL:
                    return value.toTinyIntU() <= cmpValue1.toTinyIntU();
                case Condition::Comparator::BETWEEN:
                    return (cmpValue1.toTinyIntU() <= value.toTinyIntU()) && (value.toTinyIntU() <= cmpValue2.toTinyIntU());
                case Condition::Comparator::BETWEEN_NO_LOWER:
                    return (cmpValue1.toTinyIntU() < value.toTinyIntU()) && (value.toTinyIntU() <= cmpValue2.toTinyIntU());
                case Condition::Comparator::BETWEEN_NO_UPPER:
                    return (cmpValue1.toTinyIntU() <= value.toTinyIntU()) && (value.toTinyIntU() < cmpValue2.toTinyIntU());
                case Condition::Comparator::BETWEEN_NO_BOUND:
                    return (cmpValue1.toTinyIntU() < value.toTinyIntU()) && (value.toTinyIntU() < cmpValue2.toTinyIntU());
                default:
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                }
            case PropertyType::SMALLINT:
                switch (cmp) {
                case Condition::Comparator::EQUAL:
                    return value.toSmallInt() == cmpValue1.toSmallInt();
                case Condition::Comparator::GREATER:
                    return value.toSmallInt() > cmpValue1.toSmallInt();
                case Condition::Comparator::GREATER_EQUAL:
                    return value.toSmallInt() >= cmpValue1.toSmallInt();
                case Condition::Comparator::LESS:
                    return value.toSmallInt() < cmpValue1.toSmallInt();
                case Condition::Comparator::LESS_EQUAL:
                    return value.toSmallInt() <= cmpValue1.toSmallInt();
                case Condition::Comparator::BETWEEN:
                    return (cmpValue1.toSmallInt() <= value.toSmallInt()) && (value.toSmallInt() <= cmpValue2.toSmallInt());
                case Condition::Comparator::BETWEEN_NO_LOWER:
                    return (cmpValue1.toSmallInt() < value.toSmallInt()) && (value.toSmallInt() <= cmpValue2.toSmallInt());
                case Condition::Comparator::BETWEEN_NO_UPPER:
                    return (cmpValue1.toSmallInt() <= value.toSmallInt()) && (value.toSmallInt() < cmpValue2.toSmallInt());
                case Condition::Comparator::BETWEEN_NO_BOUND:
                    return (cmpValue1.toSmallInt() < value.toSmallInt()) && (value.toSmallInt() < cmpValue2.toSmallInt());
                default:
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                }
            case PropertyType::UNSIGNED_SMALLINT:
                switch (cmp) {
                case Condition::Comparator::EQUAL:
                    return value.toSmallIntU() == cmpValue1.toSmallIntU();
                case Condition::Comparator::GREATER:
                    return value.toSmallIntU() > cmpValue1.toSmallIntU();
                case Condition::Comparator::GREATER_EQUAL:
                    return value.toSmallIntU() >= cmpValue1.toSmallIntU();
                case Condition::Comparator::LESS:
                    return value.toSmallIntU() < cmpValue1.toSmallIntU();
                case Condition::Comparator::LESS_EQUAL:
                    return value.toSmallIntU() <= cmpValue1.toSmallIntU();
                case Condition::Comparator::BETWEEN:
                    return (cmpValue1.toSmallIntU() <= value.toSmallIntU()) && (value.toSmallIntU() <= cmpValue2.toSmallIntU());
                case Condition::Comparator::BETWEEN_NO_LOWER:
                    return (cmpValue1.toSmallIntU() < value.toSmallIntU()) && (value.toSmallIntU() <= cmpValue2.toSmallIntU());
                case Condition::Comparator::BETWEEN_NO_UPPER:
                    return (cmpValue1.toSmallIntU() <= value.toSmallIntU()) && (value.toSmallIntU() < cmpValue2.toSmallIntU());
                case Condition::Comparator::BETWEEN_NO_BOUND:
                    return (cmpValue1.toSmallIntU() < value.toSmallIntU()) && (value.toSmallIntU() < cmpValue2.toSmallIntU());
                default:
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                }
            case PropertyType::INTEGER:
                switch (cmp) {
                case Condition::Comparator::EQUAL:
                    return value.toInt() == cmpValue1.toInt();
                case Condition::Comparator::GREATER:
                    return value.toInt() > cmpValue1.toInt();
                case Condition::Comparator::GREATER_EQUAL:
                    return value.toInt() >= cmpValue1.toInt();
                case Condition::Comparator::LESS:
                    return value.toInt() < cmpValue1.toInt();
                case Condition::Comparator::LESS_EQUAL:
                    return value.toInt() <= cmpValue1.toInt();
                case Condition::Comparator::BETWEEN:
                    return (cmpValue1.toInt() <= value.toInt()) && (value.toInt() <= cmpValue2.toInt());
                case Condition::Comparator::BETWEEN_NO_LOWER:
                    return (cmpValue1.toInt() < value.toInt()) && (value.toInt() <= cmpValue2.toInt());
                case Condition::Comparator::BETWEEN_NO_UPPER:
                    return (cmpValue1.toInt() <= value.toInt()) && (value.toInt() < cmpValue2.toInt());
                case Condition::Comparator::BETWEEN_NO_BOUND:
                    return (cmpValue1.toInt() < value.toInt()) && (value.toInt() < cmpValue2.toInt());
                default:
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                }
            case PropertyType::UNSIGNED_INTEGER:
                switch (cmp) {
                case Condition::Comparator::EQUAL:
                    return value.toIntU() == cmpValue1.toIntU();
                case Condition::Comparator::GREATER:
                    return value.toIntU() > cmpValue1.toIntU();
                case Condition::Comparator::GREATER_EQUAL:
                    return value.toIntU() >= cmpValue1.toIntU();
                case Condition::Comparator::LESS:
                    return value.toIntU() < cmpValue1.toIntU();
                case Condition::Comparator::LESS_EQUAL:
                    return value.toIntU() <= cmpValue1.toIntU();
                case Condition::Comparator::BETWEEN:
                    return (cmpValue1.toIntU() <= value.toIntU()) && (value.toIntU() <= cmpValue2.toIntU());
                case Condition::Comparator::BETWEEN_NO_LOWER:
                    return (cmpValue1.toIntU() < value.toIntU()) && (value.toIntU() <= cmpValue2.toIntU());
                case Condition::Comparator::BETWEEN_NO_UPPER:
                    return (cmpValue1.toIntU() <= value.toIntU()) && (value.toIntU() < cmpValue2.toIntU());
                case Condition::Comparator::BETWEEN_NO_BOUND:
                    return (cmpValue1.toIntU() < value.toIntU()) && (value.toIntU() < cmpValue2.toIntU());
                default:
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                }
            case PropertyType::BIGINT:
                switch (cmp) {
                case Condition::Comparator::EQUAL:
                    return value.toBigInt() == cmpValue1.toBigInt();
                case Condition::Comparator::GREATER:
                    return value.toBigInt() > cmpValue1.toBigInt();
                case Condition::Comparator::GREATER_EQUAL:
                    return value.toBigInt() >= cmpValue1.toBigInt();
                case Condition::Comparator::LESS:
                    return value.toBigInt() < cmpValue1.toBigInt();
                case Condition::Comparator::LESS_EQUAL:
                    return value.toBigInt() <= cmpValue1.toBigInt();
                case Condition::Comparator::BETWEEN:
                    return (cmpValue1.toBigInt() <= value.toBigInt()) && (value.toBigInt() <= cmpValue2.toBigInt());
                case Condition::Comparator::BETWEEN_NO_LOWER:
                    return (cmpValue1.toBigInt() < value.toBigInt()) && (value.toBigInt() <= cmpValue2.toBigInt());
                case Condition::Comparator::BETWEEN_NO_UPPER:
                    return (cmpValue1.toBigInt() <= value.toBigInt()) && (value.toBigInt() < cmpValue2.toBigInt());
                case Condition::Comparator::BETWEEN_NO_BOUND:
                    return (cmpValue1.toBigInt() < value.toBigInt()) && (value.toBigInt() < cmpValue2.toBigInt());
                default:
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                }
            case PropertyType::UNSIGNED_BIGINT:
                switch (cmp) {
                case Condition::Comparator::EQUAL:
                    return value.toBigIntU() == cmpValue1.toBigIntU();
                case Condition::Comparator::GREATER:
                    return value.toBigIntU() > cmpValue1.toBigIntU();
                case Condition::Comparator::GREATER_EQUAL:
                    return value.toBigIntU() >= cmpValue1.toBigIntU();
                case Condition::Comparator::LESS:
                    return value.toBigIntU() < cmpValue1.toBigIntU();
                case Condition::Comparator::LESS_EQUAL:
                    return value.toBigIntU() <= cmpValue1.toBigIntU();
                case Condition::Comparator::BETWEEN:
                    return (cmpValue1.toBigIntU() <= value.toBigIntU()) && (value.toBigIntU() <= cmpValue2.toBigIntU());
                case Condition::Comparator::BETWEEN_NO_LOWER:
                    return (cmpValue1.toBigIntU() < value.toBigIntU()) && (value.toBigIntU() <= cmpValue2.toBigIntU());
                case Condition::Comparator::BETWEEN_NO_UPPER:
                    return (cmpValue1.toBigIntU() <= value.toBigIntU()) && (value.toBigIntU() < cmpValue2.toBigIntU());
                case Condition::Comparator::BETWEEN_NO_BOUND:
                    return (cmpValue1.toBigIntU() < value.toBigIntU()) && (value.toBigIntU() < cmpValue2.toBigIntU());
                default:
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                }
            case PropertyType::REAL:
                switch (cmp) {
                case Condition::Comparator::EQUAL:
                    return value.toReal() == cmpValue1.toReal();
                case Condition::Comparator::GREATER:
                    return value.toReal() > cmpValue1.toReal();
                case Condition::Comparator::GREATER_EQUAL:
                    return value.toReal() >= cmpValue1.toReal();
                case Condition::Comparator::LESS:
                    return value.toReal() < cmpValue1.toReal();
                case Condition::Comparator::LESS_EQUAL:
                    return value.toReal() <= cmpValue1.toReal();
                case Condition::Comparator::BETWEEN:
                    return (cmpValue1.toReal() <= value.toReal()) && (value.toReal() <= cmpValue2.toReal());
                case Condition::Comparator::BETWEEN_NO_LOWER:
                    return (cmpValue1.toReal() < value.toReal()) && (value.toReal() <= cmpValue2.toReal());
                case Condition::Comparator::BETWEEN_NO_UPPER:
                    return (cmpValue1.toReal() <= value.toReal()) && (value.toReal() < cmpValue2.toReal());
                case Condition::Comparator::BETWEEN_NO_BOUND:
                    return (cmpValue1.toReal() < value.toReal()) && (value.toReal() < cmpValue2.toReal());
                default:
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                }
            case PropertyType::TEXT: {
                auto textValue = (isIgnoreCase) ? toLower(value.toText()) : value.toText();
                auto textCmpValue1 = (isIgnoreCase) ? toLower(cmpValue1.toText()) : cmpValue1.toText();
                auto textCmpValue2 = (cmpValue2.empty()) ? "" : ((isIgnoreCase) ? toLower(cmpValue2.toText()) : cmpValue2.toText());
                switch (cmp) {
                case Condition::Comparator::EQUAL:
                    return textValue == textCmpValue1;
                case Condition::Comparator::GREATER:
                    return textValue > textCmpValue1;
                case Condition::Comparator::GREATER_EQUAL:
                    return textValue >= textCmpValue1;
                case Condition::Comparator::LESS:
                    return textValue < textCmpValue1;
                case Condition::Comparator::LESS_EQUAL:
                    return textValue <= textCmpValue1;
                case Condition::Comparator::CONTAIN:
                    return textValue.find(textCmpValue1) < strlen(textValue.c_str());
                case Condition::Comparator::BEGIN_WITH:
                    return textValue.find(textCmpValue1) == 0;
                case Condition::Comparator::END_WITH:
                    std::reverse(textValue.begin(), textValue.end());
                    std::reverse(textCmpValue1.begin(), textCmpValue1.end());
                    return textValue.find(textCmpValue1) == 0;
                case Condition::Comparator::LIKE:
                    utils::string::replaceAll(textCmpValue1, "%", "(.*)");
                    utils::string::replaceAll(textCmpValue1, "_", "(.)");
                    return std::regex_match(textValue, std::regex(textCmpValue1));
                case Condition::Comparator::REGEX: {
                    return std::regex_match(textValue, std::regex(textCmpValue1));
                }
                case Condition::Comparator::BETWEEN:
                    return (textCmpValue1 <= textValue) && (textValue <= textCmpValue2);
                case Condition::Comparator::BETWEEN_NO_LOWER:
                    return (textCmpValue1 < textValue) && (textValue <= textCmpValue2);
                case Condition::Comparator::BETWEEN_NO_UPPER:
                    return (textCmpValue1 <= textValue) && (textValue < textCmpValue2);
                case Condition::Comparator::BETWEEN_NO_BOUND:
                    return (textCmpValue1 < textValue) && (textValue < textCmpValue2);
                default:
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                }
            }
            case PropertyType::BLOB:
                switch (cmp) {
                case Condition::Comparator::EQUAL:
                    return memcmp(value.getRaw(), cmpValue1.getRaw(), value.size()) == 0;
                default:
                    throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_COMPARATOR);
                }
            default:
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_PROPTYPE);
            }
        }
    };
}
}
