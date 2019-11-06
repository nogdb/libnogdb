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

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <utility>

#include "constant.hpp"
#include "storage_adapter.hpp"
#include "utils.hpp"

namespace nogdb {
namespace adapter {
namespace relation {
    using namespace internal_data_type;
    using namespace utils::assertion;

    enum class Direction {
        IN,
        OUT,
        ALL
    };

    /**
     * Raw record format in lmdb data storage:
     * {vertexId<string>} -> {edgeId<RecordId>}{neighborId<RecordId>}
     */
    struct RelationAccessInfo {
        RelationAccessInfo() = default;

        RelationAccessInfo(const RecordId& _vertexId, const RecordId& _edgeId, const RecordId& _neighborId)
            : vertexId { _vertexId }
            , edgeId { _edgeId }
            , neighborId { _neighborId }
        {
        }

        RecordId vertexId {};
        RecordId edgeId {};
        RecordId neighborId {};
    };

    constexpr char KEY_SEPARATOR = ':';

    class RelationAccess : public storage_engine::adapter::LMDBKeyValAccess {
    public:
        RelationAccess() = default;

        RelationAccess(const storage_engine::LMDBTxn* const txn, const Direction& direction)
            : LMDBKeyValAccess(txn, (direction == Direction::IN) ? TB_RELATIONS_IN : TB_RELATIONS_OUT,
                false, false, false, true)
            , _direction { direction }
        {
        }

        virtual ~RelationAccess() noexcept = default;

        void create(const RelationAccessInfo& props)
        {
            put(rid2str(props.vertexId), convertToBlob(props));
        }

        void remove(const RecordId& vertexId)
        {
            del(rid2str(vertexId));
        }

        //TODO: doesn't work as expected
        void remove(const RelationAccessInfo& props)
        {
            del(rid2str(props.vertexId), convertToBlob(props));
        }

        //TODO: this method was created for a temporary fix of the above method (but having worse performance)
        void removeByCursor(const RelationAccessInfo& props)
        {
            auto result = std::vector<RecordId> {};
            auto cursorHandler = cursor();
            for (auto keyValue = cursorHandler.find(rid2str(props.vertexId));
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto key = str2rid(keyValue.key.data.string());
                if (key != props.vertexId)
                    break;
                auto neighbor = parseNeighborId(keyValue.val.data.blob());
                if (neighbor != props.neighborId)
                    continue;
                auto edgeId = parseEdgeId(keyValue.val.data.blob());
                if (edgeId != props.edgeId)
                    continue;
                cursorHandler.del();
                break;
            }
        }

        std::vector<RelationAccessInfo> getInfos(const RecordId& vertexId) const
        {
            auto result = std::vector<RelationAccessInfo> {};
            auto cursorHandler = cursor();
            for (auto keyValue = cursorHandler.find(rid2str(vertexId));
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto key = str2rid(keyValue.key.data.string());
                if (key != vertexId)
                    break;
                result.emplace_back(parse(vertexId, keyValue.val.data.blob()));
            }
            return result;
        }

        std::vector<RecordId> getEdges(const RecordId& vertexId, const RecordId& neighborId) const
        {
            auto result = std::vector<RecordId> {};
            auto cursorHandler = cursor();
            for (auto keyValue = cursorHandler.find(rid2str(vertexId));
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto key = str2rid(keyValue.key.data.string());
                if (key != vertexId)
                    break;
                auto neighbor = parseNeighborId(keyValue.val.data.blob());
                if (neighbor != neighborId)
                    continue;
                result.emplace_back(parseEdgeId(keyValue.val.data.blob()));
            }
            return result;
        }

        std::vector<RecordId> getEdges(const RecordId& vertexId) const
        {
            auto result = std::vector<RecordId> {};
            auto cursorHandler = cursor();
            for (auto keyValue = cursorHandler.find(rid2str(vertexId));
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto key = str2rid(keyValue.key.data.string());
                if (key != vertexId)
                    break;
                result.emplace_back(parseEdgeId(keyValue.val.data.blob()));
            }
            return result;
        }

        std::vector<std::pair<RecordId, RecordId>> getEdgeAndNeighbours(const RecordId& vertexId) const
        {
            auto result = std::vector<std::pair<RecordId, RecordId>> {};
            auto cursorHandler = cursor();
            for (auto keyValue = cursorHandler.find(rid2str(vertexId));
                 !keyValue.empty();
                 keyValue = cursorHandler.getNext()) {
                auto key = str2rid(keyValue.key.data.string());
                if (key != vertexId)
                    break;
                auto blob = keyValue.val.data.blob();
                result.emplace_back(std::make_pair(parseEdgeId(blob), parseNeighborId(blob)));
            }
            return result;
        }

        Direction getDirection() const
        {
            return _direction;
        };

    protected:
        static Blob convertToBlob(const RelationAccessInfo& props)
        {
            auto totalLength = 2 * (sizeof(ClassId) + sizeof(PositionId));
            auto value = Blob(totalLength);
            value.append(&props.edgeId.first, sizeof(ClassId));
            value.append(&props.edgeId.second, sizeof(PositionId));
            value.append(&props.neighborId.first, sizeof(ClassId));
            value.append(&props.neighborId.second, sizeof(PositionId));
            return value;
        }

        RelationAccessInfo parse(const RecordId& vertexId, const Blob& blob) const
        {
            return RelationAccessInfo {
                vertexId,
                parseEdgeId(blob),
                parseNeighborId(blob)
            };
        }

        static RecordId parseEdgeId(const Blob& blob)
        {
            auto edgeId = RecordId {};
            blob.retrieve(&edgeId.first, 0, sizeof(ClassId));
            blob.retrieve(&edgeId.second, sizeof(ClassId), sizeof(PositionId));
            return edgeId;
        }

        static RecordId parseNeighborId(const Blob& blob)
        {
            auto neighborId = RecordId {};
            auto sizeOfRecordId = sizeof(ClassId) + sizeof(PositionId);
            blob.retrieve(&neighborId.first, sizeOfRecordId, sizeof(ClassId));
            blob.retrieve(&neighborId.second, sizeOfRecordId + sizeof(ClassId), sizeof(PositionId));
            return neighborId;
        }

    private:
        const Direction _direction;

        RecordId str2rid(const std::string& key) const
        {
            auto splitKey = utils::string::split(key, KEY_SEPARATOR);
            require(splitKey.size() == 2);
            auto classId = static_cast<ClassId>(std::strtoul(splitKey[0].c_str(), nullptr, 0));
            auto positionId = static_cast<PositionId>(std::strtoul(splitKey[1].c_str(), nullptr, 0));
            return RecordId { classId, positionId };
        };
    };
}

}
}