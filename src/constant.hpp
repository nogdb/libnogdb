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
#include <string>

namespace nogdb {

constexpr uint16_t INIT_NUM_CLASSES = 6;
const std::string DB_SETTING_NAME = "/.settings.nogdb";
const std::string TB_DBINFO = ".dbinfo";
const std::string TB_CLASSES = ".classes";
const std::string TB_PROPERTIES = ".properties";
const std::string TB_RELATIONS_IN = ".relations#in";
const std::string TB_RELATIONS_OUT = ".relations#out";
const std::string TB_INDEXES = ".indexes";

const std::string TB_INDEXING_PREFIX = ".index_";

constexpr uint16_t INIT_NUM_PROPERTIES = 4;
constexpr uint16_t CLASS_NAME_PROPERTY_ID = 0;
const std::string CLASS_NAME_PROPERTY = "@className";
constexpr uint16_t RECORD_ID_PROPERTY_ID = 1;
const std::string RECORD_ID_PROPERTY = "@recordId";
constexpr uint16_t DEPTH_PROPERTY_ID = 2;
const std::string DEPTH_PROPERTY = "@depth";
constexpr uint16_t VERSION_PROPERTY_ID = 3;
const std::string VERSION_PROPERTY = "@version";

constexpr uint32_t MAX_RECORD_NUM_EM = 0;

constexpr size_t MAX_CLASS_NAME_LEN = 128;
constexpr size_t MAX_PROPERTY_NAME_LEN = 128;
const std::string MAX_CLASS_ID_KEY = "?max_class_id";
const std::string NUM_CLASS_KEY = "?num_class_id";
const std::string MAX_PROPERTY_ID_KEY = "?max_property_id";
const std::string NUM_PROPERTY_KEY = "?num_property_id";
const std::string MAX_INDEX_ID_KEY = "?max_index_id";
const std::string NUM_INDEX_KEY = "?num_index_id";

const std::regex GLOBAL_VALID_NAME_PATTERN = std::regex("^[A-Za-z_][A-Za-z0-9_]*$");

}
