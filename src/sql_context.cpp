/*
 *  Copyright (C) 2018, Throughwave (Thailand) Co., Ltd.
 *  <kasidej dot bu at throughwave dot co dot th>
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

#include "constant.hpp"
#include "sql.hpp"
#include "sql_parser.h"
#include "sql_context.hpp"
#include "utils.hpp"

#include "nogdb.h"

using namespace std;
using namespace nogdb::sql_parser;

#define CLASS_DESCDRIPTOR_TEMPORARY     -2
#define PROPERTY_DESCRIPTOR_TEMPORARY   -2

#pragma mark - Context

void Context::createClass(const Token &tName, const Token &tExtend, char checkIfNotExists) {
    ClassDescriptor result;
    string name = tName.toString();
    try {
        ClassType classType;
        if (tExtend.t == TK_VERTEX) {
            result = Class::create(this->txn, name, ClassType::VERTEX);
        } else if (tExtend.t == TK_EDGE) {
            result = Class::create(this->txn, name, ClassType::EDGE);
        } else {
            result = Class::createExtend(this->txn, name, tExtend.toString());
        }

        this->rc = SQL_OK;
        this->result = SQL::Result(new ClassDescriptor(move(result)));
    } catch (const Error &e) {
        if (checkIfNotExists && e.code() == CTX_DUPLICATE_CLASS) {
            result = Db::getSchema(this->txn, name);
            this->rc = SQL_OK;
            this->result = SQL::Result(new ClassDescriptor(move(result)));
        } else {
            this->rc = SQL_ERROR;
            this->result = SQL::Result(new Error(e));
        }
    }
}

void Context::alterClass(const Token &tName, const Token &tAttr, const Bytes &value) {
    enum AlterAttr {
        ALTER_NAME,
        UNDEFINED
    };
    static const map<string, AlterAttr, function<bool(const string &, const string &)>> attrMap(
            {
                    {"NAME", ALTER_NAME}
            },
            [](const string &a, const string &b) { return strcasecmp(a.c_str(), b.c_str()) < 0; }
    );

    try {
        string attrStr = tAttr.toString();
        AlterAttr attr;
        try {
            attr = attrMap.at(attrStr);
        } catch (...) {
            attr = UNDEFINED;
        }

        switch (attr) {
            case ALTER_NAME:
                nogdb::Class::alter(this->txn, tName.toString(), value.toText());
                this->rc = SQL_OK;
                this->result = SQL::Result();
                break;

            case UNDEFINED:
            default:
                throw Error(SQL_INVALID_ALTER_ATTR, Error::Type::SQL);
        }
    } catch (const Error &e) {
        this->rc = SQL_ERROR;
        this->result = SQL::Result(new Error(e));
    }
}

void Context::dropClass(const Token &tName, char checkIfExists) {
    try {
        Class::drop(this->txn, tName.toString());
        this->rc = SQL_OK;
        this->result = SQL::Result();
    } catch (const Error &e) {
        if (checkIfExists && e.code() == CTX_NOEXST_CLASS) {
            this->rc = SQL_OK;
            this->result = SQL::Result();
        } else {
            this->rc = SQL_ERROR;
            this->result = SQL::Result(new Error(e));
        }
    }
}

void
Context::createProperty(const Token &tClassName, const Token &tPropName, const Token &tType, char checkIfNotExists) {
    map<std::string, nogdb::PropertyType, std::function<bool(const std::string &, const std::string &)>> mapType(
            {
                    {"TINYINT",           nogdb::PropertyType::TINYINT},
                    {"UNSIGNED_TINYINT",  nogdb::PropertyType::UNSIGNED_TINYINT},
                    {"SMALLINT",          nogdb::PropertyType::SMALLINT},
                    {"UNSIGNED_SMALLINT", nogdb::PropertyType::UNSIGNED_SMALLINT},
                    {"INTEGER",           nogdb::PropertyType::INTEGER},
                    {"UNSIGNED_INTEGER",  nogdb::PropertyType::UNSIGNED_INTEGER},
                    {"BIGINT",            nogdb::PropertyType::BIGINT},
                    {"UNSIGNED_BIGINT",   nogdb::PropertyType::UNSIGNED_BIGINT},
                    {"TEXT",              nogdb::PropertyType::TEXT},
                    {"REAL",              nogdb::PropertyType::REAL},
                    {"BLOB",              nogdb::PropertyType::BLOB},
            },
            [](const std::string &a, const std::string &b) { return strcasecmp(a.c_str(), b.c_str()) < 0; }
    );
    PropertyDescriptor result;
    try {
        nogdb::PropertyType t;
        try {
            t = mapType.at(tType.toString());
        } catch (...) {
            t = nogdb::PropertyType::UNDEFINED;
        }

        result = Property::add(this->txn, tClassName.toString(), tPropName.toString(), t);

        this->rc = SQL_OK;
        this->result = SQL::Result(new PropertyDescriptor(move(result)));
    } catch (const Error &e) {
        if (checkIfNotExists && e.code() == CTX_DUPLICATE_PROPERTY) {
            result = Db::getSchema(this->txn, tClassName.toString()).properties.at(tPropName.toString());
            this->rc = SQL_OK;
            this->result = SQL::Result(new PropertyDescriptor(move(result)));
        } else {
            this->rc = SQL_ERROR;
            this->result = SQL::Result(new Error(e));
        }
    }
}

void Context::alterProperty(const Token &tClassName, const Token &tPropName, const Token &tAttr, const Bytes &value) {
    enum AlterAttr {
        ALTER_NAME,
        UNDEFINED
    };
    static const map<string, AlterAttr, function<bool(const string &, const string &)>> attrMap(
            {
                    {"NAME", ALTER_NAME}
            },
            [](const string &a, const string &b) { return strcasecmp(a.c_str(), b.c_str()) < 0; }
    );

    try {
        string attrStr = tAttr.toString();
        AlterAttr attr;
        try {
            attr = attrMap.at(attrStr);
        } catch (...) {
            attr = UNDEFINED;
        }

        switch (attr) {
            case ALTER_NAME:
                nogdb::Property::alter(this->txn, tClassName.toString(), tPropName.toString(), value.toText());
                this->rc = SQL_OK;
                this->result = SQL::Result();
                break;

            case UNDEFINED:
            default:
                throw Error(SQL_INVALID_ALTER_ATTR, Error::Type::SQL);
        }
    } catch (const Error &e) {
        this->rc = SQL_ERROR;
        this->result = SQL::Result(new Error(e));
    }
}

void Context::dropProperty(const Token &tClassName, const Token &tPropName, char checkIfExists) {
    try {
        Property::remove(this->txn, tClassName.toString(), tPropName.toString());
        this->rc = SQL_OK;
        this->result = SQL::Result();
    } catch (const Error &e) {
        if (checkIfExists && e.code() == CTX_NOEXST_PROPERTY) {
            this->rc = SQL_OK;
            this->result = SQL::Result();
        } else {
            this->rc = SQL_ERROR;
            this->result = SQL::Result(new Error(e));
        }
    }
}

void Context::createVertex(const Token &tClassName, const nogdb::Record &prop) {
    try {
        const nogdb::RecordDescriptor result = Vertex::create(this->txn, tClassName.toString(), prop);
        this->rc = SQL_OK;
        this->result = SQL::Result(new vector<nogdb::RecordDescriptor>{result});
    } catch (const Error &e) {
        this->rc = SQL_ERROR;
        this->result = SQL::Result(new Error(e));
    }
}

void Context::createEdge(const CreateEdgeArgs &args) {
    try {
        auto srcVertex = this->select(args.src, Where());
        auto destVertex = this->select(args.dest, Where());

        vector<nogdb::RecordDescriptor> result{};
        for (const auto &src: srcVertex) {
            for (const auto &dest: destVertex) {
                nogdb::RecordDescriptor r = Edge::create(this->txn, args.name, src.descriptor, dest.descriptor,
                                                         args.prop);
                result.push_back(move(r));
            }
        }
        this->rc = SQL_OK;
        this->result = SQL::Result(new vector<nogdb::RecordDescriptor>(move(result)));
    } catch (const Error &e) {
        this->rc = SQL_ERROR;
        this->result = SQL::Result(new Error(e));
    }
}

void Context::select(const SelectArgs &args) {
    try {
        ResultSet result = this->selectPrivate(args);
        this->rc = SQL_OK;
        nogdb::ResultSet *tmp = new nogdb::ResultSet(result.size());
        transform(result.cbegin(), result.cend(), tmp->begin(), [](const Result &r) { return r.toBaseResult(); });
        this->result = SQL::Result(tmp);
    } catch (const Error &e) {
        this->rc = SQL_ERROR;
        this->result = SQL::Result(new Error(e));
    }
}

void Context::update(const UpdateArgs &args) {
    try {
        ResultSet targets = this->select(args.target, args.where);
        for (auto &target: targets) {
            nogdb::Record r = target.record.toBaseRecord();
            for (const auto &prop: args.prop.getAll()) {
                r.set(prop.first, prop.second);
            }
            ClassType type = Db::getSchema(this->txn, target.descriptor.rid.first).type;
            switch (type) {
                case ClassType::VERTEX:
                    Vertex::update(this->txn, target.descriptor, r);
                    break;
                case ClassType::EDGE:
                    Edge::update(this->txn, target.descriptor, r);
                    break;
                case ClassType::UNDEFINED:
                    throw Error(CTX_INVALID_CLASSTYPE, Error::Type::CONTEXT);
            }
        }
        this->rc = SQL_OK;
        this->result = SQL::Result();
    } catch (const Error &e) {
        this->rc = SQL_ERROR;
        this->result = SQL::Result(new Error(e));
    }
}

void Context::deleteVertex(const DeleteVertexArgs &args) {
    try {
        ResultSet targets = select(args.target, args.where);
        for (const auto &target: targets) {
            Vertex::destroy(this->txn, target.descriptor);
        }

        this->rc = SQL_OK;
        this->result = SQL::Result();
    } catch (const Error &e) {
        this->rc = SQL_ERROR;
        this->result = SQL::Result(new Error(e));
    }
}

void Context::deleteEdge(const DeleteEdgeArgs &args) {
    try {
        RecordDescriptorSet targets{};

        if (args.target.type == TargetType::CLASS) {
            ResultSet srcs, dests;
            RecordDescriptorSet outEdgeDescs{}, inEdgeDescs{};

            string className = args.target.get<string>();
            srcs = this->select(args.from, Where());
            const auto whereType = args.where.type;
            for (const auto &src: srcs) {
                ResultSet edges;
                switch (whereType) {
                    case WhereType::NO_COND:
                        edges = Vertex::getOutEdge(this->txn, src.descriptor, ClassFilter({className}));
                        break;
                    case WhereType::CONDITION:
                        edges = Vertex::getOutEdge(this->txn, src.descriptor, args.where.get<Condition>(),
                                                   ClassFilter({className}));
                        break;
                    case WhereType::MULTI_COND:
                        edges = Vertex::getOutEdge(this->txn, src.descriptor, args.where.get<MultiCondition>(),
                                                   ClassFilter({className}));
                        break;
                }
                // edgeDescs += edges;
                for (auto &edge: edges) {
                    outEdgeDescs.insert(move(edge.descriptor));
                }
            }

            if (!outEdgeDescs.empty() || args.from.type == TargetType::NO_TARGET) {
                ResultSet dests = this->select(args.to, Where());
                for (const auto &dest: dests) {
                    ResultSet edges;
                    switch (whereType) {
                        case WhereType::NO_COND:
                            edges = Vertex::getInEdge(this->txn, dest.descriptor, ClassFilter({className}));
                            break;
                        case WhereType::CONDITION:
                            edges = Vertex::getInEdge(this->txn, dest.descriptor, args.where.get<Condition>(),
                                                      ClassFilter({className}));
                            break;
                        case WhereType::MULTI_COND:
                            edges = Vertex::getInEdge(this->txn, dest.descriptor, args.where.get<MultiCondition>(),
                                                      ClassFilter({className}));
                            break;
                    }
                    // inEdgeDescs += edges
                    for (auto &edge: edges) {
                        inEdgeDescs.insert(move(edge.descriptor));
                    }
                }
            }

            if (args.from.type != TargetType::NO_TARGET
                && args.to.type != TargetType::NO_TARGET) {
                set_intersection(outEdgeDescs.begin(), outEdgeDescs.end(),
                                 inEdgeDescs.begin(), inEdgeDescs.end(),
                                 inserter(targets, targets.begin()));
            } else if (args.from.type != TargetType::NO_TARGET) {
                targets = move(inEdgeDescs);
            } else if (args.to.type != TargetType::NO_TARGET) {
                targets = move(outEdgeDescs);
            } else /* if (from == NO_TARGET && to == NO_TARGET) */ {
                ResultSetCursor edges = this->selectEdge(className, args.where);
                while (edges.next()) {
                    targets.insert(move(edges->descriptor));
                }
            }
        } else if (args.target.type == TargetType::RIDS) {
            auto edges = this->select(args.target.get<RecordDescriptorSet>());
            for (auto &edge: edges) {
                targets.insert(move(edge.descriptor));
            }
        }

        for (const auto &target: targets) {
            Edge::destroy(this->txn, target);
        }

        this->rc = SQL_OK;
        this->result = SQL::Result();
    } catch (const Error &e) {
        this->rc = SQL_ERROR;
        this->result = SQL::Result(new Error(e));
    }
}

void Context::traverse(const TraverseArgs &args) {
    try {
        ResultSet result = this->traversePrivate(args);
        this->rc = SQL_OK;
        nogdb::ResultSet *tmp = new nogdb::ResultSet(result.size());
        transform(result.cbegin(), result.cend(), tmp->begin(), [](const Result &r) { return r.toBaseResult(); });
        this->result = SQL::Result(tmp);
    } catch (const Error &e) {
        this->rc = SQL_ERROR;
        this->result = SQL::Result(new Error(e));
    }
}


#pragma mark -- private

ResultSet Context::selectPrivate(const SelectArgs &stmt) {
    ResultSet result = this->select(stmt.from, stmt.where, stmt.skip, stmt.limit);
    result = this->selectProjection(result, stmt.projections);
    return this->selectGroupBy(result, stmt.group);
}

ResultSet Context::select(const Target &target, const Where &where) {
    return this->select(target, where, -1, -1);
}

ResultSet Context::select(const Target &target, const Where &where, int skip, int limit) {
    switch (target.type) {
        case TargetType::NO_TARGET:
            return ResultSet{};

        case TargetType::CLASS: {
            string &className = target.get<string>();
            ClassType type = this->findClassType(className);
            if (type == ClassType::VERTEX) {
                ResultSetCursor res = this->selectVertex(className, where);
                return ResultSet(res, skip, limit);
            } else if (type == ClassType::EDGE) {
                ResultSetCursor res = this->selectEdge(className, where);
                return ResultSet(res, skip, limit);
            } else {
                throw Error(CTX_INVALID_CLASSTYPE, Error::Type::CONTEXT);
            }
        }

        case TargetType::RIDS: {
            ResultSet result = this->select(target.get<RecordDescriptorSet>());
            result = this->selectWhere(result, where);
            if (skip > 0) {
                result.erase(result.begin(), result.begin() + skip);
            }
            if (limit >= 0 && (unsigned) limit < result.size()) {
                result.resize(limit);
            }
            return result;
        }

        case TargetType::NESTED: {
            auto result = this->selectPrivate(target.get<SelectArgs>());
            result = this->selectWhere(result, where);
            if (skip > 0) {
                result.erase(result.begin(), result.begin() + skip);
            }
            if (limit >= 0 && (unsigned) limit < result.size()) {
                result.resize(limit);
            }
            return result;
        }

        case TargetType::NESTED_TRAVERSE: {
            auto result = this->traversePrivate(target.get<TraverseArgs>());
            result = this->selectWhere(result, where);
            if (skip > 0) {
                result.erase(result.begin(), result.begin() + skip);
            }
            if (limit >= 0 && (unsigned) limit < result.size()) {
                result.resize(limit);
            }
            return result;
        }

        default:
            return ResultSet{};
    }
}

ResultSet Context::select(const RecordDescriptorSet &rids) {
    ResultSet result{};
    for (RecordDescriptor rid: rids) {
        nogdb::Record r = nogdb::Db::getRecord(this->txn, rid);
        auto res = Result(move(rid), move(r));
        result.push_back(move(res));
    }
    return result;
}

nogdb::ResultSetCursor Context::selectVertex(const string &className, const Where &where) {
    switch (where.type) {
        case WhereType::CONDITION:
            return nogdb::Vertex::getCursor(this->txn, className, where.get<Condition>());
        case WhereType::MULTI_COND:
            return nogdb::Vertex::getCursor(this->txn, className, where.get<MultiCondition>());
        case WhereType::NO_COND:
        default:
            return Vertex::getCursor(this->txn, className);
    }
}

nogdb::ResultSetCursor Context::selectEdge(const string &className, const Where &where) {
    switch (where.type) {
        case WhereType::CONDITION:
            return nogdb::Edge::getCursor(this->txn, className, where.get<Condition>());
        case WhereType::MULTI_COND:
            return nogdb::Edge::getCursor(this->txn, className, where.get<MultiCondition>());
        case WhereType::NO_COND:
        default:
            return nogdb::Edge::getCursor(this->txn, className);
    }
}

ResultSet Context::selectWhere(ResultSet &input, const Where &where) {
    if (where.type == WhereType::NO_COND || input.size() == 0) {
        return move(input);
    } else /* if (where.type == WhereType::CONDITION || where.type == WhereType::MULTI_COND) */ {
        static MultiCondition alwaysTrue = Condition(RECORD_ID_PROPERTY) || !Condition(RECORD_ID_PROPERTY);
        MultiCondition exp = (where.type == WhereType::MULTI_COND
                              ? where.get<MultiCondition>()
                              : where.get<Condition>() && alwaysTrue);
        ResultSet result{};

        PropertyMapType map{};
        ClassId previousClassID = -1;
        for (ResultSet::const_iterator in = input.begin(); in != input.end(); in++) {
            ClassId classID = in->descriptor.rid.first;
            if (classID == (ClassId) CLASS_DESCDRIPTOR_TEMPORARY) {
                map.clear();
                map[RECORD_ID_PROPERTY] = nogdb::PropertyType::TEXT;
                map[CLASS_NAME_PROPERTY] = nogdb::PropertyType::TEXT;
                for (const auto &prop: in->record.getAll()) {
                    map[prop.first] = prop.second.type().toBase();
                }
            } else if (classID != previousClassID) {
                map.clear();
                map[RECORD_ID_PROPERTY] = nogdb::PropertyType::TEXT;
                map[CLASS_NAME_PROPERTY] = nogdb::PropertyType::TEXT;
                const ClassProperty &classProp = Db::getSchema(this->txn, classID).properties;
                for (const auto &p: classProp) {
                    map[p.first] = p.second.type;
                }
            }

            if (exp.execute(in->record.toBaseRecord(), map)) {
                result.push_back(move(*in));
            }

            previousClassID = classID;
        }
        return result;
    }
}

ResultSet Context::selectProjection(ResultSet &input, const vector<Projection> projs) {
    if (projs.empty()) {
        return move(input);
    }

    if (projs.size() == 1
        && projs[0].type == ProjectionType::FUNCTION
        && projs[0].get<Function>().isExpand()) {
        projs[0].get<Function>().executeExpand(this->txn, input);
        return move(input);
    }

    bool grouped = false;
    Record tmpRec{};
    for (const Projection &proj: projs) {
        if (proj.type == ProjectionType::FUNCTION) {
            const Function &func = proj.get<Function>();
            if (func.isGroupResult()) {
                tmpRec.set(func.toString(), func.executeGroupResult(input));
                grouped = true;
            } else if (func.isExpand()) {
                throw Error(SQL_INVALID_PROJECTION, Error::Type::SQL);
            }
        }
    }

    if (grouped) {
        if (!input.empty()) {
            const Result &last = input.back();
            PropertyMapType mapProps = this->getPropertyMapTypeFromClassDescriptor(last.descriptor.rid.first);
            for (const Projection &proj: projs) {
                if (proj.type != ProjectionType::FUNCTION || proj.get<Function>().isGroupResult() == false) {
                    tmpRec.set(this->selectProjectionItem(last, proj, mapProps));
                }
            }
        }
        return ResultSet({Result(RecordDescriptor(CLASS_DESCDRIPTOR_TEMPORARY, 0), move(tmpRec))});
    } else {
        ResultSet results{};
        for (const Result &in: input) {
            Record record{};
            PropertyMapType mapProps = this->getPropertyMapTypeFromClassDescriptor(in.descriptor.rid.first);
            for (const Projection &proj: projs) {
                record.set(this->selectProjectionItem(in, proj, mapProps));
            }
            if (!record.empty()) {
                results.emplace_back(nogdb::RecordDescriptor(CLASS_DESCDRIPTOR_TEMPORARY, results.size()),
                                     move(record));
            }
        }
        return results;
    }
}

pair<string, Bytes>
Context::selectProjectionItem(const Result &input, const Projection &proj, const PropertyMapType &map) {
    switch (proj.type) {
        case ProjectionType::PROPERTY: {
            string name = proj.get<string>();
            if (name == CLASS_NAME_PROPERTY) {
                string className = Db::getSchema(this->txn, input.descriptor.rid.first).name;
                return make_pair(move(name), Bytes(className, nogdb::PropertyType::TEXT));
            } else if (name == RECORD_ID_PROPERTY) {
                return make_pair(move(name), Bytes(input.descriptor, nogdb::PropertyType::BLOB));
            } else {
                Bytes b = input.record.get(name);
                if (b.empty() || b.type() != nogdb::PropertyType::UNDEFINED) {
                    return make_pair(move(name), move(b));
                } else {
                    return make_pair(move(name), Bytes(b.getRaw(), b.size(), map.at(name)));
                }
            }
        }
        case ProjectionType::FUNCTION: {
            Function func = proj.get<Function>();
            if (func.isGroupResult() || func.isExpand()) {
                throw Error(SQL_INVALID_PROJECTION, Error::Type::SQL);
            }
            return make_pair(func.toString(), func.execute(this->txn, input));
        }
        case ProjectionType::METHOD: {
            pair<Projection, Projection> mProj = proj.get<pair<Projection, Projection>>();
            pair<string, Bytes> resA = this->selectProjectionItem(input, mProj.first, map);
            if (resA.second.type() == PropertyTypeExt::RESULT_SET) {
                ResultSet &inputB = resA.second.results();
                if (inputB.size() == 1) {
                    const Result &in = inputB.front();
                    PropertyMapType mapProps = this->getPropertyMapTypeFromClassDescriptor(in.descriptor.rid.first);
                    pair<string, Bytes> resB = this->selectProjectionItem(in, mProj.second, mapProps);
                    return make_pair(resA.first + "." + resB.first, move(resB.second));
                } else {
                    throw Error(SQL_NOT_IMPLEMENTED, Error::Type::SQL);
                }
            } else {
                throw Error(SQL_NOT_IMPLEMENTED, Error::Type::SQL);
            }
        }
        case ProjectionType::ARRAY_SELECTOR: {
            pair<Projection, unsigned long> arrSelProj = proj.get<pair<Projection, unsigned long>>();
            pair<string, Bytes> resA = this->selectProjectionItem(input, arrSelProj.first, map);
            if (resA.second.type() == PropertyTypeExt::RESULT_SET) {
                ResultSet &inputB = resA.second.results();
                string outName = resA.first + "[" + to_string(arrSelProj.second) + "]";
                if (arrSelProj.second < inputB.size()) {
                    return make_pair(move(outName), Bytes(ResultSet({inputB[arrSelProj.second]})));
                } else {
                    Result emptyResult{RecordDescriptor(CLASS_DESCDRIPTOR_TEMPORARY, 0), Record()};
                    return make_pair(move(outName), Bytes(ResultSet{emptyResult}));
                }
            } else {
                throw Error(SQL_NOT_IMPLEMENTED, Error::Type::SQL);
            }
        }
        case ProjectionType::ALIAS: {
            pair<Projection, string> aliasProj = proj.get<pair<Projection, string>>();
            pair<string, Bytes> resA = this->selectProjectionItem(input, aliasProj.first, map);
            resA.first = aliasProj.second;
            return resA;
        }
        default:
            require(false);
            abort();
    }
}

ResultSet Context::selectGroupBy(ResultSet &input, const string &group) {
    if (group.empty()) {
        return move(input);
    }

    set<Bytes> grouped;
    for (ResultSet::reverse_iterator in = input.rbegin(); in != input.rend(); in++) {
        if (grouped.find(in->record.get(group)) == grouped.end()) {
            grouped.insert(in->record.get(group));
        } else {
            input.erase(next(in).base());
        }
    }
    return move(input);
}

ResultSet Context::traversePrivate(const TraverseArgs &args) {
    typedef nogdb::ResultSet (*TraverseFunction)(const Txn &, const nogdb::RecordDescriptor &, unsigned int,
                                                 unsigned int, const ClassFilter &);
    static const map<string, TraverseFunction, function<bool(const string &, const string &)>> mapFunc(
       {
           {"INDEPTH_FIRST",    Traverse::inEdgeDfs},
           {"OUTDEPTH_FIRST",   Traverse::outEdgeDfs},
           {"ALLDEPTH_FIRST",   Traverse::allEdgeDfs},
           {"INBREADTH_FIRST",  Traverse::inEdgeBfs},
           {"OUTBREADTH_FIRST", Traverse::outEdgeBfs},
           {"ALLBREADTH_FIRST", Traverse::allEdgeBfs}
       },
       [](const string &a, const string &b) { return strcasecmp(a.c_str(), b.c_str()) < 0; }
       );

    if (args.minDepth < 0 || args.minDepth > UINT_MAX) {
        throw Error(SQL_INVALID_TRAVERSE_MIN_DEPTH, Error::Type::SQL);
    }
    if (args.maxDepth < 0 || args.maxDepth > UINT_MAX) {
        throw Error(SQL_INVALID_TRAVERSE_MAX_DEPTH, Error::Type::SQL);
    }

    TraverseFunction func;
    try {
        func = mapFunc.at(args.direction + args.strategy);
    } catch (...) {
        if (strcasecmp("IN", args.direction.c_str()) != 0
            && strcasecmp("OUT", args.direction.c_str()) != 0
            && strcasecmp("ALL", args.direction.c_str()) != 0) {
            throw Error(SQL_INVALID_TRAVERSE_DIRECTION, Error::Type::SQL);
        } else /*if (strcasecmp("DEPTH_FIRST", strategy.c_str()) != 0
                && strcasecmp("BREADTH_FIRST", strategy.c_str()) != 0) */
        {
            throw Error(SQL_INVALID_TRAVERSE_STRATEGY, Error::Type::SQL);
        }
    }

    return func(this->txn, args.root, args.minDepth, args.maxDepth, ClassFilter(args.filter));
}


nogdb::ClassType Context::findClassType(const string &className) {
    auto classD = Db::getSchema(this->txn, className);
    return classD.type;
}

nogdb::PropertyMapType Context::getPropertyMapTypeFromClassDescriptor(ClassId classID) {
    if (classID != (ClassId) CLASS_DESCDRIPTOR_TEMPORARY) {
        const ClassProperty &classProp = Db::getSchema(this->txn, classID).properties;
        PropertyMapType map{};
        for (const auto &p: classProp) {
            map[p.first] = p.second.type;
        }
        return map;
    } else {
        return PropertyMapType{};
    }
}
