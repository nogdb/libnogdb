/*
 *  Copyright (C) 2018, Throughwave (Thailand) Co., Ltd.
 *  <kasidej dot bu at throughwave dot co dot th>
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

#include <functional>

#include "constant.hpp"
#include "sql.hpp"
#include "sql_parser.h"
#include "sql_context.hpp"
#include "utils.hpp"

#include "nogdb.h"

using namespace std;
using namespace nogdb::sql_parser;
using namespace nogdb::utils::assertion;

#define CLASS_DESCDRIPTOR_TEMPORARY     -2
#define PROPERTY_DESCRIPTOR_TEMPORARY   -2


#pragma mark - Helper

typedef function<bool(const string &, const string &)> StringCaseCompare;

static bool stringcasecmp(const string &a, const string &b) {
  return strcasecmp(a.c_str(), b.c_str()) < 0;
}


#pragma mark - Context

void Context::createClass(const Token &tName, const Token &tExtend, bool checkIfNotExists) {
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
    if (checkIfNotExists && e.code() == NOGDB_CTX_DUPLICATE_CLASS) {
      result = DB::getSchema(this->txn, name);
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
  static const auto attrMap = map<string, AlterAttr, StringCaseCompare>(
      {
          {"NAME", ALTER_NAME}
      },
      stringcasecmp
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
        throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_ALTER_ATTR);
    }
  } catch (const Error &e) {
    this->rc = SQL_ERROR;
    this->result = SQL::Result(new Error(e));
  }
}

void Context::dropClass(const Token &tName, bool checkIfExists) {
  try {
    Class::drop(this->txn, tName.toString());
    this->rc = SQL_OK;
    this->result = SQL::Result();
  } catch (const Error &e) {
    if (checkIfExists && e.code() == NOGDB_CTX_NOEXST_CLASS) {
      this->rc = SQL_OK;
      this->result = SQL::Result();
    } else {
      this->rc = SQL_ERROR;
      this->result = SQL::Result(new Error(e));
    }
  }
}

void
Context::createProperty(const Token &tClassName, const Token &tPropName, const Token &tType, bool checkIfNotExists) {
  static const auto mapType = map<std::string, nogdb::PropertyType, StringCaseCompare>(
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
      stringcasecmp
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
    if (checkIfNotExists && e.code() == NOGDB_CTX_DUPLICATE_PROPERTY) {
      result = DB::getSchema(this->txn, tClassName.toString()).properties.at(tPropName.toString());
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
  static const auto attrMap = map<string, AlterAttr, StringCaseCompare>(
      {
          {"NAME", ALTER_NAME}
      },
      stringcasecmp
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
        throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_ALTER_ATTR);
    }
  } catch (const Error &e) {
    this->rc = SQL_ERROR;
    this->result = SQL::Result(new Error(e));
  }
}

void Context::dropProperty(const Token &tClassName, const Token &tPropName, bool checkIfExists) {
  try {
    Property::remove(this->txn, tClassName.toString(), tPropName.toString());
    this->rc = SQL_OK;
    this->result = SQL::Result();
  } catch (const Error &e) {
    if (checkIfExists && e.code() == NOGDB_CTX_NOEXST_PROPERTY) {
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
    vector<nogdb::RecordDescriptor> result{};
    ResultSet targets = this->select(args.target, args.where);
    for (auto &target: targets) {
      nogdb::Record r = target.record.toBaseRecord();
      for (const auto &prop: args.prop.getAll()) {
        r.set(prop.first, prop.second);
      }
      ClassType type = DB::getSchema(this->txn, target.descriptor.rid.first).type;
      switch (type) {
        case ClassType::VERTEX:
          Vertex::update(this->txn, target.descriptor, r);
          break;
        case ClassType::EDGE:
          Edge::update(this->txn, target.descriptor, r);
          break;
        case ClassType::UNDEFINED:
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_CLASSTYPE);
      }
      result.push_back(target.descriptor);
    }
    this->rc = SQL_OK;
    this->result = SQL::Result(new vector<nogdb::RecordDescriptor>(move(result)));
  } catch (const Error &e) {
    this->rc = SQL_ERROR;
    this->result = SQL::Result(new Error(e));
  }
}

void Context::deleteVertex(const DeleteVertexArgs &args) {
  try {
    vector<nogdb::RecordDescriptor> result{};
    ResultSet targets = select(args.target, args.where);
    for (const auto &target: targets) {
      Vertex::destroy(this->txn, target.descriptor);
      result.push_back(target.descriptor);
    }
    this->rc = SQL_OK;
    this->result = SQL::Result(new vector<nogdb::RecordDescriptor>(move(result)));
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
      RecordDescriptorSet outEdgeRids{}, inEdgeRids{};

      string className = args.target.get<string>();

      // get outEdge from these sources.
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
          outEdgeRids.insert(move(edge.descriptor));
        }
      }

      // get inEdge from these desination. (skip if can't find source)
      if (!outEdgeRids.empty() || args.from.type == TargetType::NO_TARGET) {
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
            inEdgeRids.insert(move(edge.descriptor));
          }
        }
      }

      // process target.
      if (args.from.type != TargetType::NO_TARGET
          && args.to.type != TargetType::NO_TARGET) {
        set_intersection(outEdgeRids.begin(), outEdgeRids.end(),
                         inEdgeRids.begin(), inEdgeRids.end(),
                         inserter(targets, targets.begin()));
      } else if (args.from.type != TargetType::NO_TARGET) {
        targets = move(inEdgeRids);
      } else if (args.to.type != TargetType::NO_TARGET) {
        targets = move(outEdgeRids);
      } else /* if (from == NO_TARGET && to == NO_TARGET) */ {
        ResultSetCursor edges = this->selectEdge(className, args.where);
        while (edges.next()) {
          targets.insert(move(edges->descriptor));
        }
      }
    } else if (args.target.type == TargetType::RIDS) {
      targets = args.target.get<RecordDescriptorSet>();
    }

    // delete.
    for (const auto &target: targets) {
      Edge::destroy(this->txn, target);
    }

    this->rc = SQL_OK;
    this->result = SQL::Result(new vector<RecordDescriptor>(targets.begin(), targets.end()));
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

void Context::createIndex(const Token &tClassName, const Token &tPropName, const Token &tIndexType) {
  try {
    bool unique = stringcasecmp(tIndexType.toString(), "UNIQUE") == 0 ? true : false;
    Property::createIndex(this->txn, tClassName.toString(), tPropName.toString(), unique);

    this->rc = SQL_OK;
    this->result = SQL::Result();
  } catch (const Error &e) {
    this->rc = SQL_ERROR;
    this->result = SQL::Result(new Error(e));
  }
}

void Context::dropIndex(const Token &tClassName, const Token &tPropName) {
  try {
    Property::dropIndex(this->txn, tClassName.toString(), tPropName.toString());

    this->rc = SQL_OK;
    this->result = SQL::Result();
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
      ClassType type = Context::findClassType(this->txn, className);
      if (type == ClassType::VERTEX) {
        ResultSetCursor res = this->selectVertex(className, where);
        return ResultSet(res, skip, limit);
      } else if (type == ClassType::EDGE) {
        ResultSetCursor res = this->selectEdge(className, where);
        return ResultSet(res, skip, limit);
      } else {
        throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_INVALID_CLASSTYPE);
      }
    }

    case TargetType::RIDS: {
      ResultSet result = this->select(target.get<RecordDescriptorSet>());
      result = this->selectWhere(result, where);
      return result.limit(skip, limit);
    }

    case TargetType::NESTED: {
      auto result = this->selectPrivate(target.get<SelectArgs>());
      result = this->selectWhere(result, where);
      return result.limit(skip, limit);
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
    nogdb::Record r = nogdb::DB::getRecord(this->txn, rid);
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
    return Context::executeCondition(this->txn, input, exp);
  }
}

ResultSet Context::selectProjection(ResultSet &input, const vector<Projection> projs) {
  if (projs.empty()) {
    return move(input);
  }

  // expand function.
  if (projs.size() == 1
      && projs[0].type == ProjectionType::FUNCTION
      && projs[0].get<Function>().isExpand()) {
    projs[0].get<Function>().executeExpand(this->txn, input);
    return move(input);
  }

  bool aggregated = false;
  Record tmpRec{};
  for (const Projection &proj: projs) {
    if (proj.type == ProjectionType::FUNCTION) {
      const Function &func = proj.get<Function>();
      if (func.isAggregateResult()) {
        tmpRec.set(func.name, func.executeAggregateResult(input));
        aggregated = true;
      } else if (func.isExpand()) {
        throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_PROJECTION);
      }
    }
  }

  if (aggregated) {
    // if input is not empty, use last record for other projection.
    if (!input.empty()) {
      const Result &last = input.back();
      PropertyMapType mapProps = Context::getPropertyMapTypeFromClassDescriptor(this->txn, last.descriptor.rid.first);
      for (const Projection &proj: projs) {
        if (proj.type != ProjectionType::FUNCTION || proj.get<Function>().isAggregateResult() == false) {
          tmpRec.set(to_string(proj), Context::getProjectionItem(this->txn, last, proj, mapProps));
        }
      }
    }
    return ResultSet({Result(RecordDescriptor(CLASS_DESCDRIPTOR_TEMPORARY, 0), move(tmpRec))});
  } else {
    ResultSet results{};
    for (const Result &in: input) {
      Record record{};
      PropertyMapType mapProps = Context::getPropertyMapTypeFromClassDescriptor(this->txn, in.descriptor.rid.first);
      for (const Projection &proj: projs) {
        record.set(to_string(proj), Context::getProjectionItem(this->txn, in, proj, mapProps));
      }
      if (!record.empty()) {
        results.emplace_back(nogdb::RecordDescriptor(CLASS_DESCDRIPTOR_TEMPORARY, results.size()),
                             move(record));
      }
    }
    return results;
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
  static const auto mapFunc = map<string, TraverseFunction, StringCaseCompare>(
      {
          {"INDEPTH_FIRST",    Traverse::inEdgeDfs},
          {"OUTDEPTH_FIRST",   Traverse::outEdgeDfs},
          {"ALLDEPTH_FIRST",   Traverse::allEdgeDfs},
          {"INBREADTH_FIRST",  Traverse::inEdgeBfs},
          {"OUTBREADTH_FIRST", Traverse::outEdgeBfs},
          {"ALLBREADTH_FIRST", Traverse::allEdgeBfs}
      },
      stringcasecmp
  );

  if (args.minDepth<0 || args.minDepth>UINT_MAX) {
    throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_TRAVERSE_MIN_DEPTH);
  }
  if (args.maxDepth<0 || args.maxDepth>UINT_MAX) {
    throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_TRAVERSE_MAX_DEPTH);
  }

  TraverseFunction func;
  try {
    func = mapFunc.at(args.direction + args.strategy);
  } catch (...) {
    if (strcasecmp("IN", args.direction.c_str()) != 0
        && strcasecmp("OUT", args.direction.c_str()) != 0
        && strcasecmp("ALL", args.direction.c_str()) != 0) {
      throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_TRAVERSE_DIRECTION);
    } else /*if (strcasecmp("DEPTH_FIRST", strategy.c_str()) != 0
                && strcasecmp("BREADTH_FIRST", strategy.c_str()) != 0) */
    {
      throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_TRAVERSE_STRATEGY);
    }
  }

  return func(this->txn, args.root, args.minDepth, args.maxDepth, ClassFilter(args.filter));
}

Bytes Context::getProjectionItem(Txn &txn, const Result &input, const Projection &proj, const PropertyMapType &map) {
  switch (proj.type) {
    case ProjectionType::PROPERTY:
      return Context::getProjectionItemProperty(txn, input, proj.get<string>(), map);
    case ProjectionType::FUNCTION: {
      Function func = proj.get<Function>();
      if (func.isAggregateResult() || func.isExpand()) {
        throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_PROJECTION);
      }
      return func.execute(txn, input);
    }
    case ProjectionType::METHOD: {
      const auto &method = proj.get<pair<Projection, Projection>>();
      return Context::getProjectionItemMethod(txn, input, method.first, method.second, map);
    }
    case ProjectionType::ARRAY_SELECTOR: {
      const auto &arrSel = proj.get<pair<Projection, unsigned long>>();
      return Context::getProjectionItemArraySelector(txn, input, arrSel.first, arrSel.second, map);
    }
    case ProjectionType::CONDITION: {
      const auto &cond = proj.get<pair<Projection, Condition>>();
      if (cond.first.type != ProjectionType::FUNCTION) {
        throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_PROJECTION);
      }
      return Context::getProjectionItemCondition(txn, input, cond.first.get<Function>(), cond.second);
    }
    case ProjectionType::ALIAS:
      return Context::getProjectionItem(txn, input, proj.get<pair<Projection, string>>().first, map);
    default:
      require(false);
      abort();
  }
}

Bytes
Context::getProjectionItemProperty(Txn &txn, const Result &input, const string &propName, const PropertyMapType &map) {
  Bytes b = input.record.get(propName);
  if (b.empty() || b.type() != nogdb::PropertyType::UNDEFINED) {
    return b;
  } else {
    return Bytes(b.getRaw(), b.size(), map.at(propName));
  }
}

Bytes Context::getProjectionItemMethod(Txn &txn, const Result &input, const Projection &firstProj,
                                       const Projection &secondProj, const PropertyMapType &map) {
  Bytes resA = Context::getProjectionItem(txn, input, firstProj, map);
  if (resA.isResults()) {
    ResultSet &inputB = resA.results();
    if (inputB.size() == 1) {
      const Result &in = inputB.front();
      PropertyMapType mapProps = Context::getPropertyMapTypeFromClassDescriptor(txn, in.descriptor.rid.first);
      return Context::getProjectionItem(txn, in, secondProj, mapProps);
    } else {
      ResultSet results;
      PropertyMapType mapProps{};
      ClassId previousClassID = -1;
      for (const Result &in: inputB) {
        if (in.descriptor.rid.first != previousClassID) {
          mapProps = Context::getPropertyMapTypeFromClassDescriptor(txn, in.descriptor.rid.first);
        }
        Bytes resB = Context::getProjectionItem(txn, in, secondProj, mapProps);
        if (!resB.isResults()) {
          throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_PROJECTION_METHOD);
        }
        results.insert(results.end(), make_move_iterator(resB.results().begin()),
                       make_move_iterator(resB.results().end()));
      }
      return Bytes(move(results));
    }
  } else if (resA.empty()) {
    return Bytes();
  } else {
    throw NOGDB_SQL_ERROR(NOGDB_SQL_NOT_IMPLEMENTED);
  }
}

Bytes
Context::getProjectionItemArraySelector(Txn &txn, const Result &input, const Projection &proj, unsigned long index,
                                        const PropertyMapType &map) {
  Bytes resA = Context::getProjectionItem(txn, input, proj, map);
  if (resA.isResults()) {
    ResultSet &inputB = resA.results();
    if (index < inputB.size()) {
      return Bytes(ResultSet({inputB[index]}));
    } else {
      return Bytes();
    }
  } else {
    throw NOGDB_SQL_ERROR(NOGDB_SQL_NOT_IMPLEMENTED);
  }
}

Bytes Context::getProjectionItemCondition(Txn &txn, const Result &input, const Function &func, const Condition &cond) {
  if (!func.isWalkResult()) {
    throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_PROJECTION);
  }

  Bytes resA = func.execute(txn, input);
  if (resA.isResults()) {
    static MultiCondition alwaysTrue = Condition(RECORD_ID_PROPERTY) || !Condition(RECORD_ID_PROPERTY);
    ResultSet result = Context::executeCondition(txn, resA.results(), cond && alwaysTrue);
    if (!result.empty()) {
      return Bytes(move(result));
    } else {
      return Bytes();
    }
  } else {
    throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_PROJECTION);
  }
}

nogdb::ClassType Context::findClassType(Txn &txn, const string &className) {
  auto classD = DB::getSchema(txn, className);
  return classD.type;
}

nogdb::PropertyMapType Context::getPropertyMapTypeFromClassDescriptor(Txn &txn, ClassId classID) {
  if (classID != (ClassId) CLASS_DESCDRIPTOR_TEMPORARY) {
    const ClassProperty &classProp = DB::getSchema(txn, classID).properties;
    PropertyMapType map{
        {CLASS_NAME_PROPERTY, PropertyType::TEXT},
        {RECORD_ID_PROPERTY,  PropertyType::TEXT},
        {VERSION_PROPERTY,    PropertyType::UNSIGNED_BIGINT}
    };
    for (const auto &p: classProp) {
      map[p.first] = p.second.type;
    }
    return map;
  } else {
    return PropertyMapType{};
  }
}

ResultSet Context::executeCondition(Txn &txn, const ResultSet &input, const MultiCondition &conds) {
  ResultSet result{};
  PropertyMapType mapProp{};
  ClassId previousClassID = -1;
  for (auto in = input.begin(); in != input.end(); in++) {
    ClassId classID = in->descriptor.rid.first;
    if (classID == (ClassId) CLASS_DESCDRIPTOR_TEMPORARY) {
      mapProp.clear();
      mapProp[RECORD_ID_PROPERTY] = PropertyType::TEXT;
      mapProp[CLASS_NAME_PROPERTY] = PropertyType::TEXT;
      mapProp[VERSION_PROPERTY] = PropertyType::UNSIGNED_BIGINT;
      for (const auto &prop: in->record.getAll()) {
        mapProp[prop.first] = prop.second.type();
      }
    } else if (classID != previousClassID) {
      mapProp.clear();
      mapProp[RECORD_ID_PROPERTY] = PropertyType::TEXT;
      mapProp[CLASS_NAME_PROPERTY] = PropertyType::TEXT;
      mapProp[VERSION_PROPERTY] = PropertyType::UNSIGNED_BIGINT;
      const ClassProperty &classProp = DB::getSchema(txn, classID).properties;
      for (const auto &p: classProp) {
        mapProp[p.first] = p.second.type;
      }
    } else /* if (classID == previousClassID) */ {
      // no-op;
    }

    if (conds.execute(in->record.toBaseRecord(), mapProp)) {
      result.push_back(move(*in));
    }

    previousClassID = classID;
  }
  return result;
}
