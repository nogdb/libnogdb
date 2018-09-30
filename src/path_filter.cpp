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

#include "nogdb_compare.h"

namespace nogdb {

  PathFilter::PathFilter(bool (*vertexFunc)(const Record &record), bool (*edgeFunc)(const Record &record)) :
      vertexFilter(vertexFunc), edgeFilter(edgeFunc) {}

  PathFilter &PathFilter::setVertex(bool (*function)(const Record &record)) {
    vertexFilter = function;
    return *this;
  }

  PathFilter &PathFilter::setEdge(bool (*function)(const Record &record)) {
    edgeFilter = function;
    return *this;
  }

  bool PathFilter::isEnable() const {
    return isSetVertex() || isSetEdge();
  }

  bool PathFilter::isSetVertex() const {
    return vertexFilter != nullptr;
  }

  bool PathFilter::isSetEdge() const {
    return edgeFilter != nullptr;
  }

}