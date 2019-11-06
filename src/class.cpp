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

#include <memory>

#include "constant.hpp"
#include "datarecord_adapter.hpp"
#include "lmdb_engine.hpp"
#include "parser.hpp"
#include "relation.hpp"
#include "schema.hpp"
#include "validate.hpp"

#include "nogdb/nogdb.h"

namespace nogdb {
using namespace utils::assertion;
using namespace adapter::schema;
using namespace adapter::datarecord;
using namespace schema;
using parser::RecordParser;

const ClassDescriptor Transaction::addClass(const std::string& className, ClassType type)
{
    BEGIN_VALIDATION(this)
        .isTxnValid()
        .isTxnCompleted()
        .isClassNameValid(className)
        .isClassTypeValid(type)
        .isNotDuplicatedClass(className)
        .isClassIdMaxReach();

    try {
        auto classId = _adapter->dbInfo()->getMaxClassId() + ClassId { 1 };
        _adapter->dbClass()->create(ClassAccessInfo { className, classId, ClassId { 0 }, type });
        _adapter->dbInfo()->setMaxClassId(classId);
        _adapter->dbInfo()->setNumClassId(_adapter->dbInfo()->getNumClassId() + ClassId { 1 });
        DataRecord(_txnBase, classId, type).init();
        return ClassDescriptor { classId, className, ClassId { 0 }, type };
    } catch (const Error& err) {
        rollback();
        throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
        rollback();
        std::rethrow_exception(std::current_exception());
    }
}

const ClassDescriptor Transaction::addSubClassOf(const std::string& superClass, const std::string& className)
{
    BEGIN_VALIDATION(this)
        .isTxnValid()
        .isTxnCompleted()
        .isClassNameValid(className)
        .isClassNameValid(superClass)
        .isNotDuplicatedClass(className)
        .isClassIdMaxReach();

    auto superClassInfo = SchemaUtils::getExistingClass(this, superClass);
    try {
        auto classId = _adapter->dbInfo()->getMaxClassId() + ClassId { 1 };
        _adapter->dbClass()->create(ClassAccessInfo { className, classId, superClassInfo.id, superClassInfo.type });
        _adapter->dbInfo()->setMaxClassId(classId);
        _adapter->dbInfo()->setNumClassId(_adapter->dbInfo()->getNumClassId() + ClassId { 1 });
        DataRecord(_txnBase, classId, superClassInfo.type).init();
        return ClassDescriptor { classId, className, superClassInfo.id, superClassInfo.type };
    } catch (const Error& err) {
        rollback();
        throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
        rollback();
        std::rethrow_exception(std::current_exception());
    }
}

void Transaction::dropClass(const std::string& className)
{
    BEGIN_VALIDATION(this)
        .isTxnValid()
        .isTxnCompleted()
        .isClassNameValid(className);

    auto foundClass = SchemaUtils::getExistingClass(this, className);
    // retrieve relevant properties information
    auto propertyInfos = _adapter->dbProperty()->getInfos(foundClass.id);
    for (const auto& property : propertyInfos) {
        // check if all index tables associated with the column have been removed beforehand
        auto foundIndex = _adapter->dbIndex()->getInfo(foundClass.id, property.id);
        if (foundIndex.id != IndexId { 0 }) {
            throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_IN_USED_PROPERTY);
        }
    }
    try {
        auto rids = std::vector<RecordId> {};
        // delete class from schema
        _adapter->dbClass()->remove(className);
        // delete properties from schema
        for (const auto& property : propertyInfos) {
            _adapter->dbProperty()->remove(property.classId, property.name);
            //TODO: implement existing index deletion if needed
        }
        // delete all associated relations
        auto table = DataRecord(_txnBase, foundClass.id, foundClass.type);
        std::function<void(const PositionId&, const storage_engine::lmdb::Result&)> callback =
            [&](const PositionId& positionId, const storage_engine::lmdb::Result& result) {
                auto recordId = RecordId { foundClass.id, positionId };
                if (foundClass.type == ClassType::EDGE) {
                    auto vertices = RecordParser::parseEdgeRawDataVertexSrcDst(result, _txnCtx->isVersionEnabled());
                    _graph->removeRelFromEdge(recordId, vertices.first, vertices.second);
                    if (_txnCtx->isVersionEnabled()) {
                        // update version of src vertex
                        if (_updatedRecords.find(vertices.first) == _updatedRecords.cend()) {
                            auto srcVertexDataRecord = DataRecord(_txnBase, vertices.first.first, ClassType::VERTEX);
                            auto srcVertexRecordResult = srcVertexDataRecord.getResult(vertices.first.second);
                            auto versionId = RecordParser::parseRawDataVersionId(srcVertexRecordResult);
                            auto updateRecordBlob = RecordParser::parseOnlyUpdateVersion(
                                srcVertexRecordResult, versionId + 1);
                            srcVertexDataRecord.update(vertices.first.second, updateRecordBlob);
                            _updatedRecords.insert(vertices.first);
                        }
                        // update version of dst vertex
                        if (_updatedRecords.find(vertices.second) == _updatedRecords.cend()) {
                            auto dstVertexDataRecord = DataRecord(_txnBase, vertices.second.first, ClassType::VERTEX);
                            auto dstVertexRecordResult = dstVertexDataRecord.getResult(vertices.second.second);
                            auto versionId = RecordParser::parseRawDataVersionId(dstVertexRecordResult);
                            auto updateRecordBlob = RecordParser::parseOnlyUpdateVersion(
                                dstVertexRecordResult, versionId + 1);
                            dstVertexDataRecord.update(vertices.second.second, updateRecordBlob);
                            _updatedRecords.insert(vertices.second);
                        }
                    }
                } else {
                    auto neighbours = _graph->removeRelFromVertex(recordId);
                    if (_txnCtx->isVersionEnabled()) {
                        for (const auto& neighbour : neighbours) {
                            if (_updatedRecords.find(neighbour) == _updatedRecords.cend()) {
                                auto neighbourDataRecord = DataRecord(_txnBase, neighbour.first, ClassType::VERTEX);
                                auto neighbourRecordResult = neighbourDataRecord.getResult(neighbour.second);
                                auto versionId = RecordParser::parseRawDataVersionId(neighbourRecordResult);
                                auto updateRecordBlob = RecordParser::parseOnlyUpdateVersion(
                                    neighbourRecordResult, versionId + 1);
                                neighbourDataRecord.update(neighbour.second, updateRecordBlob);
                                _updatedRecords.insert(neighbour);
                            }
                        }
                    }
                }
            };
        table.resultSetIter(callback);
        // drop the actual table
        table.destroy();
        // update a superclass of subclasses if existing
        for (const auto& subClassInfo : _adapter->dbClass()->getSubClassInfos(foundClass.id)) {
            _adapter->dbClass()->update(
                ClassAccessInfo {
                    subClassInfo.name,
                    subClassInfo.id,
                    foundClass.superClassId,
                    subClassInfo.type });
        }
        // update database info
        _adapter->dbInfo()->setNumClassId(_adapter->dbInfo()->getNumClassId() - ClassId { 1 });
        _adapter->dbInfo()->setNumPropertyId(
            _adapter->dbInfo()->getNumPropertyId() - PropertyId { static_cast<uint16_t>(propertyInfos.size()) });
    } catch (const Error& err) {
        rollback();
        throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
        rollback();
        std::rethrow_exception(std::current_exception());
    }
}

void Transaction::renameClass(const std::string& oldClassName, const std::string& newClassName)
{
    BEGIN_VALIDATION(this)
        .isTxnValid()
        .isTxnCompleted()
        .isClassNameValid(oldClassName)
        .isClassNameValid(newClassName)
        .isNotDuplicatedClass(newClassName);

    auto foundClass = SchemaUtils::getExistingClass(this, oldClassName);
    try {
        _adapter->dbClass()->alterClassName(oldClassName, newClassName);
    } catch (const Error& err) {
        rollback();
        throw NOGDB_FATAL_ERROR(err);
    } catch (...) {
        rollback();
        std::rethrow_exception(std::current_exception());
    }
}
}
