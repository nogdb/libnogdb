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

#pragma once

#include <map>
#include <vector>
#include <utility>

#include "datatype.hpp"
#include "lmdb_engine.hpp"
#include "schema.hpp"
#include "schema_adapter.hpp"

#include "nogdb/nogdb_types.h"

namespace nogdb {

  namespace parser {

    constexpr size_t UINT8_BITS_COUNT = 8 * sizeof(uint8_t);
    constexpr size_t UINT16_BITS_COUNT = 8 * sizeof(uint16_t);
    constexpr size_t UINT32_BITS_COUNT = 8 * sizeof(uint32_t);

    const std::string EMPTY_STRING = std::string{"\n"};
    const size_t SIZE_OF_EMPTY_STRING = strlen(EMPTY_STRING.c_str());

    constexpr size_t VERTEX_SRC_DST_RAW_DATA_LENGTH = 2 * (sizeof(ClassId) + sizeof(PositionId));

    class RecordParser {
    public:
      RecordParser() = delete;

      ~RecordParser() noexcept = delete;

      //-------------------------
      // Common parsers
      //-------------------------
      static Blob parseRecord(const Record &record,
                              const adapter::schema::PropertyNameMapInfo &properties);

      static Record parseRawData(const storage_engine::lmdb::Result &rawData,
                                 const adapter::schema::PropertyIdMapInfo &propertyInfos,
                                 bool isEdge = false);

      static Record parseRawData(const storage_engine::lmdb::Result &rawData,
                                 const adapter::schema::PropertyIdMapInfo &propertyInfos,
                                 const ClassType &classType);

      static Record parseRawDataWithBasicInfo(const std::string &className,
                                              const RecordId &rid,
                                              const storage_engine::lmdb::Result &rawData,
                                              const adapter::schema::PropertyIdMapInfo &propertyInfos,
                                              const ClassType &classType);
      //-------------------------
      // Edge only parsers
      //-------------------------
      static Blob parseEdgeVertexSrcDst(const RecordId &srcRid, const RecordId &dstRid);

      static std::pair<RecordId, RecordId> parseEdgeRawDataVertexSrcDst(const Blob &blob);

      static Blob parseEdgeRawDataVertexSrcDstAsBlob(const Blob &blob);

      static Blob parseEdgeRawDataAsBlob(const Blob &blob);

    private:

      static void buildRawData(Blob &blob, const PropertyId &propertyId, const Bytes &rawData);

      static Blob parseRecord(const Record &record,
                              const size_t dataSize,
                              const adapter::schema::PropertyNameMapInfo &properties);

      inline static size_t getRawDataSize(size_t size) {
        return sizeof(PropertyId) + size +
               ((size >= std::pow(2, UINT8_BITS_COUNT - 1)) ? sizeof(uint32_t) : sizeof(uint8_t));
      };

      inline static bool isNameValid(const std::string &name) {
        return std::regex_match(name, GLOBAL_VALID_NAME_PATTERN);
      }
    };
  }

}
