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

#include "nogdb/nogdb.h"

namespace nogdb {

OperationBuilder::OperationBuilder(const Transaction* txn)
    : _txn { txn }
{
}

FindOperationBuilder::FindOperationBuilder(const Transaction* txn,
    const std::string& className,
    bool includeSubClassOf)
    : OperationBuilder { txn }
    , _className { className }
    , _conditionType { ConditionType::UNDEFINED }
    , _includeSubClassOf { includeSubClassOf }
{
}

FindOperationBuilder& FindOperationBuilder::where(const Condition& condition)
{
    _conditionType = ConditionType::CONDITION;
    _condition = std::make_shared<Condition>(condition);
    return *this;
}

FindOperationBuilder& FindOperationBuilder::where(const MultiCondition& multiCondition)
{
    _conditionType = ConditionType::MULTI_CONDITION;
    _multiCondition = std::make_shared<MultiCondition>(multiCondition);
    return *this;
}

FindOperationBuilder& FindOperationBuilder::where(bool (*condition)(const Record& record))
{
    _conditionType = ConditionType::COMPARE_FUNCTION;
    _function = condition;
    return *this;
}

FindOperationBuilder& FindOperationBuilder::indexed(bool onlyIndex)
{
    _indexed = onlyIndex;
    return *this;
}

FindEdgeOperationBuilder::FindEdgeOperationBuilder(const Transaction* txn,
    const RecordDescriptor& recordDescriptor,
    const EdgeDirection& direction)
    : OperationBuilder { txn }
    , _rdesc { recordDescriptor }
    , _direction { direction }
{
}

FindEdgeOperationBuilder& FindEdgeOperationBuilder::where(const GraphFilter& edgeFilter)
{
    _filter = edgeFilter;
    return *this;
}

TraverseOperationBuilder::TraverseOperationBuilder(const Transaction* txn,
    const RecordDescriptor& recordDescriptor,
    const EdgeDirection& direction)
    : OperationBuilder { txn }
    , _direction { direction }
{
    _rdescs.insert(recordDescriptor);
}

TraverseOperationBuilder& TraverseOperationBuilder::addSource(const RecordDescriptor& recordDescriptor)
{
    _rdescs.insert(recordDescriptor);
    return *this;
}

TraverseOperationBuilder& TraverseOperationBuilder::whereV(const GraphFilter& filter)
{
    _vertexFilter = filter;
    return *this;
}

TraverseOperationBuilder& TraverseOperationBuilder::whereE(const GraphFilter& filter)
{
    _edgeFilter = filter;
    return *this;
}

TraverseOperationBuilder& TraverseOperationBuilder::minDepth(unsigned int depth)
{
    _minDepth = depth;
    return *this;
}

TraverseOperationBuilder& TraverseOperationBuilder::maxDepth(unsigned int depth)
{
    _maxDepth = depth;
    return *this;
}

TraverseOperationBuilder& TraverseOperationBuilder::depth(unsigned int minDepth, unsigned int maxDepth)
{
    _minDepth = minDepth;
    _maxDepth = maxDepth;
    return *this;
}

ShortestPathOperationBuilder::ShortestPathOperationBuilder(const Transaction* txn,
    const RecordDescriptor& srcVertexRecordDescriptor,
    const RecordDescriptor& dstVertexRecordDescriptor)
    : OperationBuilder { txn }
    , _srcRdesc { srcVertexRecordDescriptor }
    , _dstRdesc { dstVertexRecordDescriptor }
{
}

ShortestPathOperationBuilder& ShortestPathOperationBuilder::whereV(const GraphFilter& filter)
{
    _vertexFilter = filter;
    return *this;
}

ShortestPathOperationBuilder& ShortestPathOperationBuilder::whereE(const GraphFilter& filter)
{
    _edgeFilter = filter;
    return *this;
}

}