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

#ifndef __DATARECORD_ADAPTER_HPP_INCLUDED_
#define __DATARECORD_ADAPTER_HPP_INCLUDED_

#include "storage_adapter.hpp"
#include "schema_adapter.hpp"

namespace nogdb {
    namespace datarecord {

        class DataRecord : public storage_engine::adapter::LMDBKeyValAccess {
        public:
            DataRecord(const storage_engine::LMDBTxn *const txn, const ClassId& classId, const ClassType& classType)
                    : _classId{classId},
                      _classType{classType},
                      LMDBKeyValAccess(txn, std::to_string(classId), true, true, true, true) {}

            virtual ~DataRecord() noexcept = default;

        private:
            ClassId _classId{};
            ClassType _classType{ClassType::UNDEFINED};

        };

    }
}

#endif //__DATARECORD_ADAPTER_HPP_INCLUDED_
