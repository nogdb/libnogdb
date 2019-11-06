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

#include <cmath>
#include <map>
#include <utility>
#include <vector>

#include "datatype.hpp"
#include "lmdb_engine.hpp"
#include "schema.hpp"
#include "schema_adapter.hpp"

#include "nogdb/nogdb_types.h"

namespace nogdb {
namespace parser {
    using namespace adapter::schema;

    constexpr size_t UINT8_BITS_COUNT = 8 * sizeof(uint8_t);
    constexpr size_t UINT16_BITS_COUNT = 8 * sizeof(uint16_t);
    constexpr size_t UINT32_BITS_COUNT = 8 * sizeof(uint32_t);

    const std::string EMPTY_STRING = std::string { "\n" };
    const size_t SIZE_OF_EMPTY_STRING = strlen(EMPTY_STRING.c_str());

    constexpr size_t VERTEX_SRC_DST_RAW_DATA_LENGTH = 2 * (sizeof(ClassId) + sizeof(PositionId));
    constexpr size_t RECORD_VERSION_DATA_LENGTH = sizeof(uint64_t);

    class RecordParser {
    public:
        RecordParser() = delete;

        ~RecordParser() noexcept = delete;

        //-------------------------
        // Common parsers
        //-------------------------
        static Blob parseRecord(const Record& record, const PropertyNameMapInfo& properties);

        static Record parseRawData(const storage_engine::lmdb::Result& rawData,
            const PropertyIdMapInfo& propertyInfos,
            bool isEdge,
            bool enableVersion);

        static Record parseRawData(const storage_engine::lmdb::Result& rawData,
            const PropertyIdMapInfo& propertyInfos,
            const ClassType& classType,
            bool enableVersion);

        static Blob& parseOnlyUpdateVersion(Blob& blob, VersionId versionId);

        static Blob parseOnlyUpdateVersion(const storage_engine::lmdb::Result& rawData, VersionId versionId);

        static Blob parseOnlyUpdateSrcVertex(const storage_engine::lmdb::Result& rawData,
            const RecordId& srcVertex,
            bool enableVersion);

        static Blob parseOnlyUpdateDstVertex(const storage_engine::lmdb::Result& rawData,
            const RecordId& dstVertex,
            bool enableVersion);

        static Blob parseOnlyUpdateRecord(const storage_engine::lmdb::Result& rawData,
            const Blob& newRecordBlob,
            bool isEdge,
            bool enableVersion);

        static Record parseRawDataWithBasicInfo(const std::string& className,
            const RecordId& rid,
            const storage_engine::lmdb::Result& rawData,
            const PropertyIdMapInfo& propertyInfos,
            const ClassType& classType,
            bool enableVersion);
        //-------------------------
        // Version Id parsers
        //-------------------------
        static Blob parseVertexRecordWithVersion(const Blob& recordBlob, VersionId versionId);

        static Blob parseEdgeRecordWithVersion(const Blob& srcDstBlob, const Blob& recordBlob, VersionId versionId);

        static VersionId parseRawDataVersionId(const storage_engine::lmdb::Result& rawData);

        //-------------------------
        // Edge only parsers
        //-------------------------
        static Blob parseEdgeVertexSrcDst(const RecordId& srcRid, const RecordId& dstRid);

        static std::pair<RecordId, RecordId>
        parseEdgeRawDataVertexSrcDst(const storage_engine::lmdb::Result& rawData, bool enableVersion);

        static Blob parseEdgeRawDataVertexSrcDstAsBlob(const storage_engine::lmdb::Result& rawData, bool enableVersion);

        static Blob parseEdgeRawDataAsBlob(const storage_engine::lmdb::Result& rawData, bool enableVersion);

    private:
        static void buildRawData(Blob& blob, const PropertyId& propertyId, const Bytes& rawData);

        static Blob parseRecord(const Record& record,
            const size_t dataSize,
            const PropertyNameMapInfo& properties);

        inline static size_t getRawDataSize(size_t size)
        {
            return sizeof(PropertyId) + size + ((size >= std::pow(2, UINT8_BITS_COUNT - 1)) ? sizeof(uint32_t) : sizeof(uint8_t));
        };

        inline static bool isNameValid(const std::string& name)
        {
            return std::regex_match(name, GLOBAL_VALID_NAME_PATTERN);
        }
    };
}

}
