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

#ifndef __CONFIG_HPP_INCLUDED_
#define __CONFIG_HPP_INCLUDED_

#include <string>

namespace nogdb {

    constexpr unsigned int MAX_VERSION_CONTROL_SIZE = 128;
    const std::string DB_LOCK_FILE = "/.context.lock";

    constexpr uint16_t INIT_NUM_CLASSES = 4;
    const std::string TB_CLASSES = ".classes";
    const std::string TB_PROPERTIES = ".properties";
    const std::string TB_RELATIONS = ".relations";
    const std::string TB_INDEXES = ".indexes";

    const std::string TB_INDEXING_PREFIX = ".index_";

    constexpr uint16_t INIT_NUM_PROPERTIES = 5;
    constexpr uint16_t CLASS_NAME_PROPERTY_ID = 0;
    const std::string CLASS_NAME_PROPERTY = "@className";
    constexpr uint16_t RECORD_ID_PROPERTY_ID = 1;
    const std::string RECORD_ID_PROPERTY = "@recordId";
    constexpr uint16_t DEPTH_PROPERTY_ID = 2;
    const std::string DEPTH_PROPERTY = "@depth";
    constexpr uint16_t VERSION_PROPERTY_ID = 3;
    const std::string VERSION_PROPERTY = "@version";
    constexpr uint16_t TXN_VERSION_ID = 4;
    const std::string TXN_VERSION = "@txnVersion";

    constexpr uint16_t UINT16_EM_INIT = 0;
    const std::string STRING_EM_INIT = ".init";
    constexpr uint32_t EM_MAXRECNUM = 0;

    const std::string INDEX_POSITIVE_SUFFIX = "_p";
    const std::string INDEX_NEGATIVE_SUFFIX = "_n";

}

#endif
