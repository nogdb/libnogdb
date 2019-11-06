/*
 *  runtest_config.h - A configuration of runtest software
 *
 *  Copyright (C) 2019, NogDB <https://nogdb.org>
 *  <nogdb at throughwave dot co dot th>
 *
 *  This program is free software: you can redistribute it and/or modify
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

#include <string>

const std::string DATABASE_PATH { "./func_test.db" };
constexpr unsigned int COLUMN_ID_OFFSET = 13;
constexpr unsigned int COLUMN_NAME_OFFSET = 25;
constexpr unsigned int COLUMN_TYPE_OFFSET = 13;
