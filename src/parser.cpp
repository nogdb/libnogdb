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

#include "parser.hpp"
#include "utils.hpp"

namespace nogdb {
namespace parser {
    using namespace internal_data_type;
    using namespace adapter::schema;
    using namespace utils::assertion;

    Blob RecordParser::parseRecord(const Record& record, const PropertyNameMapInfo& properties)
    {
        auto dataSize = size_t { 0 };
        // calculate a raw data size of properties in a record
        for (const auto& property : record.getAll()) {
            auto foundProperty = properties.find(property.first);
            if (foundProperty == properties.cend()) {
                throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_NOEXST_PROPERTY);
            }
            dataSize += getRawDataSize(property.second.size());
            //TODO: check if having any index?
        }
        return parseRecord(record, dataSize, properties);
    }

    Blob RecordParser::parseVertexRecordWithVersion(const Blob& recordBlob, VersionId versionId)
    {
        if (versionId > 0) {
            auto versionIdBlob = Blob(sizeof(VersionId)).append(&versionId, sizeof(VersionId));
            return versionIdBlob + recordBlob;
        } else {
            return recordBlob;
        }
    }

    Blob RecordParser::parseEdgeRecordWithVersion(const Blob& srcDstBlob,
        const Blob& recordBlob,
        VersionId versionId)
    {
        if (versionId > 0) {
            auto versionIdBlob = Blob(sizeof(VersionId)).append(&versionId, sizeof(VersionId));
            return versionIdBlob + srcDstBlob + recordBlob;
        } else {
            return srcDstBlob + recordBlob;
        }
    }

    Blob& RecordParser::parseOnlyUpdateVersion(Blob& blob, VersionId versionId)
    {
        blob.update(&versionId, 0, sizeof(VersionId));
        return blob;
    }

    Blob RecordParser::parseOnlyUpdateVersion(const storage_engine::lmdb::Result& rawData, VersionId versionId)
    {
        require(!rawData.empty);
        auto blob = rawData.data.blob();
        blob.update(&versionId, 0, sizeof(VersionId));
        return blob;
    }

    Blob RecordParser::parseOnlyUpdateSrcVertex(const storage_engine::lmdb::Result& rawData,
        const RecordId& srcVertex,
        bool enableVersion)
    {
        require(!rawData.empty);
        auto blob = rawData.data.blob();
        auto offset = (enableVersion) ? RECORD_VERSION_DATA_LENGTH : size_t { 0 };
        blob.update(&srcVertex.first, offset, sizeof(ClassId));
        blob.update(&srcVertex.second, offset + sizeof(ClassId), sizeof(PositionId));
        return blob;
    }

    Blob RecordParser::parseOnlyUpdateDstVertex(const storage_engine::lmdb::Result& rawData,
        const RecordId& dstVertex,
        bool enableVersion)
    {
        require(!rawData.empty);
        auto blob = rawData.data.blob();
        auto offset = (enableVersion) ? RECORD_VERSION_DATA_LENGTH : size_t { 0 };
        offset += sizeof(ClassId) + sizeof(PositionId);
        blob.update(&dstVertex.first, offset, sizeof(ClassId));
        blob.update(&dstVertex.second, offset + sizeof(ClassId), sizeof(PositionId));
        return blob;
    }

    Blob RecordParser::parseOnlyUpdateRecord(const storage_engine::lmdb::Result& rawData,
        const Blob& newRecordBlob,
        bool isEdge,
        bool enableVersion)
    {
        require(!(rawData.empty && (isEdge || enableVersion)));
        auto blob = (rawData.empty) ? Blob {} : rawData.data.blob();
        auto offset = size_t { 0 };
        offset += (isEdge) ? VERTEX_SRC_DST_RAW_DATA_LENGTH : size_t { 0 };
        offset += (enableVersion) ? RECORD_VERSION_DATA_LENGTH : size_t { 0 };
        return blob.overwrite(newRecordBlob.bytes(), offset, newRecordBlob.size());
    }

    Record RecordParser::parseRawData(const storage_engine::lmdb::Result& rawData,
        const PropertyIdMapInfo& propertyInfos,
        bool isEdge,
        bool enableVersion)
    {
        if (rawData.empty) {
            return Record {};
        }
        Record::PropertyToBytesMap properties {};
        auto rawDataBlob = rawData.data.blob();
        auto offset = size_t { 0 };
        offset += (isEdge) ? VERTEX_SRC_DST_RAW_DATA_LENGTH : size_t { 0 };
        offset += (enableVersion) ? RECORD_VERSION_DATA_LENGTH : size_t { 0 };
        if (rawDataBlob.capacity() == 0 || rawDataBlob.size() - offset == 1) {
            return Record {};
        } else if (rawDataBlob.capacity() >= 2 * sizeof(uint16_t)) {
            //TODO: should be concerned about ENDIAN?
            /**
             * NOTE: each property block consists of property id, flag, size, and value
             * when option flag = 0
             * +----------------------+--------------------+-----------------------+-----------+
             * | propertyId (16bits)  | option flag (1bit) | propertySize (7bits)  |   value   | (next block) ...
             * +----------------------+--------------------+-----------------------+-----------+
             * when option flag = 1 (for extra large size of value)
             * +----------------------+--------------------+------------------------+-----------+
             * | propertyId (16bits)  | option flag (1bit) | propertySize (31bits)  |   value   | (next block) ...
             * +----------------------+--------------------+------------------------+-----------+
             */
            while (offset < rawDataBlob.size()) {
                auto propertyId = PropertyId {};
                auto optionFlag = uint8_t {};
                offset = rawDataBlob.retrieve(&propertyId, offset, sizeof(PropertyId));
                rawDataBlob.retrieve(&optionFlag, offset, sizeof(optionFlag));
                auto propertySize = size_t {};
                if ((optionFlag & 0x1) == 1) {
                    //extra large size of value (exceed 127 bytes)
                    auto tmpSize = uint32_t {};
                    offset = rawDataBlob.retrieve(&tmpSize, offset, sizeof(uint32_t));
                    propertySize = static_cast<size_t>(tmpSize >> 1);
                } else {
                    //normal size of value (not exceed 127 bytes)
                    auto tmpSize = uint8_t {};
                    offset = rawDataBlob.retrieve(&tmpSize, offset, sizeof(uint8_t));
                    propertySize = static_cast<size_t>(tmpSize >> 1);
                }
                auto foundInfo = propertyInfos.find(propertyId);
                if (foundInfo != propertyInfos.cend()) {
                    if (propertySize > 0) {
                        Blob::Byte byteData[propertySize];
                        offset = rawDataBlob.retrieve(byteData, offset, propertySize);
                        properties[foundInfo->second.name] = Bytes { byteData, propertySize };
                    } else {
                        properties[foundInfo->second.name] = Bytes {};
                    }
                } else {
                    offset += propertySize;
                }
            }
        }
        return Record(properties);
    }

    Record RecordParser::parseRawData(const storage_engine::lmdb::Result& rawData,
        const PropertyIdMapInfo& propertyInfos,
        const ClassType& classType,
        bool enableVersion)
    {
        return parseRawData(rawData, propertyInfos, classType == ClassType::EDGE, enableVersion);
    }

    Record RecordParser::parseRawDataWithBasicInfo(const std::string& className,
        const RecordId& rid,
        const storage_engine::lmdb::Result& rawData,
        const PropertyIdMapInfo& propertyInfos,
        const ClassType& classType,
        bool enableVersion)
    {
        auto versionId = (enableVersion) ? parseRawDataVersionId(rawData) : VersionId { 0 };
        return parseRawData(rawData, propertyInfos, classType == ClassType::EDGE, versionId > 0)
            .setBasicInfoIfNotExists(CLASS_NAME_PROPERTY, className)
            .setBasicInfoIfNotExists(RECORD_ID_PROPERTY, rid2str(rid))
            .setBasicInfoIfNotExists(DEPTH_PROPERTY, 0U)
            .setBasicInfoIfNotExists(VERSION_PROPERTY, versionId);
    }

    VersionId RecordParser::parseRawDataVersionId(const storage_engine::lmdb::Result& rawData)
    {
        require(!rawData.data.empty());
        auto blob = rawData.data.blob();
        auto versionId = VersionId { 0 };
        blob.retrieve(&versionId, 0, RECORD_VERSION_DATA_LENGTH);
        return versionId;
    }

    Blob RecordParser::parseEdgeVertexSrcDst(const RecordId& srcRid, const RecordId& dstRid)
    {
        auto value = Blob(VERTEX_SRC_DST_RAW_DATA_LENGTH);
        value.append(&srcRid.first, sizeof(ClassId));
        value.append(&srcRid.second, sizeof(PositionId));
        value.append(&dstRid.first, sizeof(ClassId));
        value.append(&dstRid.second, sizeof(PositionId));
        return value;
    }

    std::pair<RecordId, RecordId> RecordParser::
        parseEdgeRawDataVertexSrcDst(const storage_engine::lmdb::Result& rawData, bool enableVersion)
    {
        require(!rawData.data.empty());
        auto blob = rawData.data.blob();
        auto offset = (enableVersion) ? RECORD_VERSION_DATA_LENGTH : size_t { 0 };
        require(blob.size() >= offset + VERTEX_SRC_DST_RAW_DATA_LENGTH);
        auto srcVertexRid = RecordId {};
        auto dstVertexRid = RecordId {};
        offset = blob.retrieve(&srcVertexRid.first, offset, sizeof(ClassId));
        offset = blob.retrieve(&srcVertexRid.second, offset, sizeof(PositionId));
        offset = blob.retrieve(&dstVertexRid.first, offset, sizeof(ClassId));
        offset = blob.retrieve(&dstVertexRid.second, offset, sizeof(PositionId));
        return std::make_pair(srcVertexRid, dstVertexRid);
    }

    Blob RecordParser::parseEdgeRawDataVertexSrcDstAsBlob(const storage_engine::lmdb::Result& rawData, bool enableVersion)
    {
        require(!rawData.data.empty());
        auto blob = rawData.data.blob();
        auto offset = (enableVersion) ? RECORD_VERSION_DATA_LENGTH : size_t { 0 };
        require(blob.size() >= offset + VERTEX_SRC_DST_RAW_DATA_LENGTH);
        Blob::Byte byteData[VERTEX_SRC_DST_RAW_DATA_LENGTH];
        blob.retrieve(byteData, offset, VERTEX_SRC_DST_RAW_DATA_LENGTH);
        return Blob(byteData, VERTEX_SRC_DST_RAW_DATA_LENGTH);
    }

    Blob RecordParser::parseEdgeRawDataAsBlob(const storage_engine::lmdb::Result& rawData, bool enableVersion)
    {
        require(!rawData.data.empty());
        auto blob = rawData.data.blob();
        auto offset = VERTEX_SRC_DST_RAW_DATA_LENGTH + ((enableVersion) ? RECORD_VERSION_DATA_LENGTH : size_t { 0 });
        if (blob.size() > offset) {
            auto rawDataSize = blob.size() - offset;
            Blob::Byte byteData[rawDataSize];
            blob.retrieve(byteData, offset, sizeof(byteData));
            return Blob(byteData, rawDataSize);
        } else {
            return Blob();
        }
    }

    void RecordParser::buildRawData(Blob& blob, const PropertyId& propertyId, const Bytes& rawData)
    {
        if (rawData.size() < std::pow(2, UINT8_BITS_COUNT - 1)) {
            auto size = static_cast<uint8_t>(rawData.size()) << 1;
            blob.append(&propertyId, sizeof(PropertyId));
            blob.append(&size, sizeof(uint8_t));
            blob.append(static_cast<void*>(rawData.getRaw()), rawData.size());
        } else {
            auto size = (static_cast<uint32_t>(rawData.size()) << 1) + 0x1;
            blob.append(&propertyId, sizeof(PropertyId));
            blob.append(&size, sizeof(uint32_t));
            blob.append(static_cast<void*>(rawData.getRaw()), rawData.size());
        }
    }

    Blob RecordParser::parseRecord(const Record& record,
        const size_t dataSize,
        const PropertyNameMapInfo& properties)
    {
        if (dataSize <= 0) {
            // create an empty property as a raw data for a class
            auto value = Blob(SIZE_OF_EMPTY_STRING);
            value.append(EMPTY_STRING.c_str(), SIZE_OF_EMPTY_STRING);
            return value;
        } else {
            // create properties as a raw data for a class
            auto value = Blob(dataSize);
            for (const auto& property : properties) {
                if (!isNameValid(property.first))
                    continue;
                auto propertyId = static_cast<PropertyId>(property.second.id);
                auto rawData = record.get(property.first);
                if (rawData.empty())
                    continue;
                require(propertyId < std::pow(2, UINT16_BITS_COUNT));
                require(rawData.size() < std::pow(2, UINT32_BITS_COUNT - 1));
                buildRawData(value, propertyId, rawData);
            }
            return value;
        }
    }

}
}
