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

/*
** This file contains NogDB's grammar for SQL. Process this file
** using the lemon parser generator to generate C code that runs
** the parser. Lemon will also generate a header file containing
** numeric codes for all of the tokens.
*/

// All token codes are small integers with #defines that begin with "TK_"
%token_prefix TK_

// The type of the data attached to each token is Token.  This is also the
// default type for non-termials.
%token_type { Token }

%wildcard ANY.

%include {
#include <stdio.h>
#include <assert.h>
#include <set>
#include "sql.hpp"
#include "sql_context.hpp"

using namespace std;
using namespace nogdb::sql_parser;

using nogdb::RecordDescriptor;
using nogdb::MultiCondition;

#define LEMON_SUPER Context

} // %include

%code {
unique_ptr<Context> Context::create(Transaction &txn) {
    return unique_ptr<yypParser>(new yypParser(txn));
}

} // %code

%token_class name IDENTITY|STRING.
%token_class typename IDENTITY|STRING.
%token_class integer SIGNED|UNSIGNED.


//////////////////// Input is a single SQL command
input ::= cmd.


//////////////////// The CLASS operations ////////////////////
// CREATE
cmd ::= CREATE CLASS name(name) if_not_exists_opt(checkIfNotExists) EXTENDS VERTEX|EDGE(type) SEMI. {
    this->createClass(name, type, checkIfNotExists);
}
cmd ::= CREATE CLASS name(name) if_not_exists_opt(checkIfNotExists) EXTENDS name(extend) SEMI. {
    this->createClass(name, extend, checkIfNotExists);
}

// ALTER
cmd ::= ALTER CLASS name(name) IDENTITY(attr) term(value) SEMI. {
    this->alterClass(name, attr, value);
}

// DROP
cmd ::= DROP CLASS name(name) if_exists_opt(checkIfExists) SEMI. {
    this->dropClass(name, checkIfExists);
}


//////////////////// The PROPERTY operations ////////////////////
// CREATE
cmd ::= CREATE PROPERTY name(className) DOT name(propName) if_not_exists_opt(checkIfNotExists) typename(type) SEMI. {
    this->createProperty(className, propName, type, checkIfNotExists);
}

// ALTER
cmd ::= ALTER PROPERTY name(className) DOT name(propName) IDENTITY(attr) term(value) SEMI. {
    this->alterProperty(className, propName, attr, value);
}

// DROP
cmd ::= DROP PROPERTY name(className) DOT name(propName) if_exists_opt(checkIfExists) SEMI. {
    this->dropProperty(className, propName, checkIfExists);
}


//////////////////// The Database operations ////////////////////


//////////////////// The CREATE VERTEX command ////////////////////
cmd ::= CREATE VERTEX name(name) props_opt(prop) SEMI. {
    this->createVertex(name, prop);
}


//////////////////// The CREATE EDGE command ////////////////////
cmd ::= create_edge_stmt(s) SEMI. {
    this->createEdge(s);
}

%type create_edge_stmt { CreateEdgeArgs }
create_edge_stmt(A) ::= CREATE EDGE name(name) FROM select_target_without_class(src) TO select_target_without_class(dest) props_opt(prop). {
    A = CreateEdgeArgs{name.toString(), move(src), move(dest), move(prop)};
}


//////////////////// The SELECT command ////////////////////
cmd ::= select_stmt(stmt) SEMI. {
    this->select(stmt);
}

%type select_stmt { SelectArgs }
select_stmt(A) ::= SELECT projections(proj) from_opt(from) where_opt(where) group_by(group) order_by(order) skip(skip) limit(limit). {
    A = SelectArgs{move(proj), move(from), move(where), group, order, skip, limit};
}

// projections
%type projections { vector<Projection> }
projections(A) ::= . { A = vector<Projection>(); }
projections(A) ::= STAR. { A = vector<Projection>(); }
projections(A) ::= projections(A) COMMA proj_alias(X). { A.push_back(move(X)); }
projections(A) ::= proj_alias(X). { A = vector<Projection>{X}; }

%type proj_alias { Projection }
proj_alias(A) ::= proj_item(A).
proj_alias(A) ::= proj_item(X) AS name(Y). {
    A = Projection(ProjectionType::ALIAS, make_shared<pair<Projection, string>>(move(X), Y.toString()));
}

%type proj_item { Projection }
proj_item(A) ::= LP proj_item(X) RP. { A = move(X); }
proj_item(A) ::= IDENTITY(X). {
    A = Projection(ProjectionType::PROPERTY, make_shared<string>(X.toString()));
}
proj_item(A) ::= STRING(X). {
    A = Projection(ProjectionType::PROPERTY, make_shared<string>(X.toString()));
}
proj_item(A) ::= AT(X) IDENTITY(Y). {
    Token atProp{X.z, static_cast<int>(Y.z + Y.n - X.z), X.t};
    A = Projection(ProjectionType::PROPERTY, make_shared<string>(atProp.toString()));
}
proj_item(A) ::= IDENTITY(fName) LP projections(args) RP. {
    A = Projection(ProjectionType::FUNCTION, make_shared<Function>(fName.toString(), move(args)));
}
%left DOT.
proj_item(A) ::= proj_item(X) DOT proj_item(Y). {
    A = Projection(ProjectionType::METHOD, make_shared<pair<Projection, Projection>>(move(X), move(Y)));
}
proj_item(A) ::= IDENTITY(fName) LP projections(fArgs) RP LB integer(index) RB. {
    A = Projection(
            ProjectionType::ARRAY_SELECTOR,
            make_shared<pair<Projection, unsigned long>>(
                Projection(ProjectionType::FUNCTION, make_shared<Function>(fName.toString(), move(fArgs))),
                stoull(string(index.z, index.n))));
}
proj_item(A) ::= IDENTITY(X) LB integer(index) RB. {
    A = Projection(
            ProjectionType::ARRAY_SELECTOR,
            make_shared<pair<Projection, unsigned long>>(
                Projection(ProjectionType::PROPERTY, make_shared<string>(X.toString())),
                stoull(string(index.z, index.n))));
}
proj_item(A) ::= STRING(X) LB integer(index) RB. {
    A = Projection(
            ProjectionType::ARRAY_SELECTOR,
            make_shared<pair<Projection, unsigned long>>(
                Projection(ProjectionType::PROPERTY, make_shared<string>(X.toString())),
                stoull(string(index.z, index.n))));
}
proj_item(A) ::= IDENTITY(fName) LP projections(fArgs) RP LB cond(c) RB. {
    A = Projection(
        ProjectionType::CONDITION,
        make_shared<pair<Projection, Condition>>(
            Projection(ProjectionType::FUNCTION, make_shared<Function>(fName.toString(), move(fArgs))),
            c));
}

// from_opt
%type from_opt { Target }
from_opt(A) ::= . { A = Target(); }
from_opt(A) ::= FROM select_target(X). { A = X; }

%type select_target { Target }
select_target(A) ::= name(class_). {
    A = Target(TargetType::CLASS, make_shared<string>(class_.toString()));
}
select_target(A) ::= select_target_without_class(X). { A = X; }

%type select_target_without_class { Target }
select_target_without_class(A) ::= select_target_rids(rids). {
    A = Target(TargetType::RIDS, make_shared<RecordDescriptorSet>(move(rids)));
}
select_target_without_class(A) ::= LP select_stmt(stmt) RP. {
    A = Target(TargetType::NESTED, make_shared<SelectArgs>(move(stmt)));
}
select_target_without_class(A) ::= LP traverse_stmt(stmt) RP. {
    A = Target(TargetType::NESTED_TRAVERSE, make_shared<TraverseArgs>(move(stmt)));
}

%type select_target_rids { RecordDescriptorSet }
select_target_rids(A) ::= rid(X). { A = RecordDescriptorSet{X}; }
select_target_rids(A) ::= LP rid_set(X) RP. { A = X; }

// where
%type where_opt { Where }
where_opt(A) ::= . { A = Where(); }
where_opt(A) ::= WHERE multi_cond(X). {
    A = Where(WhereType::MULTI_COND, X);
}
where_opt(A) ::= WHERE cond(X). {
    A = Where(WhereType::CONDITION, make_shared<Condition>(move(X)));
}

// gropu_by
%type group_by { string }
group_by(A) ::= . { A = string(); }
group_by(A) ::= GROUP BY prop_name(X). { A = X; }

// order_by
%type order_by { void * }
order_by ::= .
order_by ::= ORDER BY name_set sort_order.
sort_order ::= .
sort_order ::= ASC.
sort_order ::= DESC.

// skip
%type skip { int }
skip(A) ::= . { A = -1; }
skip(A) ::= SKIP integer(X). { A = stoi(string(X.z, X.n)); }

// limit
%type limit { int }
limit(A) ::= . { A = -1; }
limit(A) ::= LIMIT integer(X). { A = stoi(string(X.z, X.n)); }


//////////////////// The UPDATE command ////////////////////
cmd ::= update_stmt(stmt) SEMI. {
    this->update(stmt);
}

%type update_stmt { UpdateArgs }
update_stmt(A) ::= UPDATE select_target(target) props_opt(prop) where_opt(where). {
    A = UpdateArgs{move(target), move(prop), move(where)};
}


//////////////////// The DELETE VERTEX command ////////////////////
cmd ::= delete_vertex_stmt(stmt) SEMI. {
    this->deleteVertex(stmt);
}

%type delete_vertex_stmt { DeleteVertexArgs }
delete_vertex_stmt(A) ::= DELETE VERTEX select_target(target) where_opt(where). {
    A = DeleteVertexArgs{move(target), move(where)};
}


//////////////////// The DELETE EDGE command ////////////////////
cmd ::= delete_edge_stmt(stmt) SEMI. {
    this->deleteEdge(stmt);
}

%type delete_edge_stmt  { DeleteEdgeArgs }
delete_edge_stmt(A) ::= DELETE EDGE select_target_rids(rids). {
    auto target = Target(TargetType::RIDS, make_shared<RecordDescriptorSet>(move(rids)));
    A = DeleteEdgeArgs{move(target), Target(), Target(), Where()};
}
delete_edge_stmt(A) ::= DELETE EDGE name(name) from_edge_opt(from) to_edge_opt(to) where_opt(where). {
    auto target = Target(TargetType::CLASS, make_shared<string>(name.toString()));
    A = DeleteEdgeArgs{move(target), move(from), move(to), move(where)};
}

%type from_edge_opt { Target }
from_edge_opt(A) ::= . { A = Target(); }
from_edge_opt(A) ::= FROM select_target_without_class(X). { A = X; }

%type to_edge_opt { Target }
to_edge_opt(A) ::= . { A = Target(); }
to_edge_opt(A) ::= TO select_target_without_class(X). { A = X; }


//////////////////// The TRAVERSE command ////////////////////
cmd ::= traverse_stmt(stmt) SEMI. {
    this->traverse(stmt);
}

%type traverse_stmt { TraverseArgs }
traverse_stmt(A) ::= TRAVERSE
    IDENTITY(direction) LP class_filter(filter) RP
    FROM rid_set(root)
    min_depth_opt(min_depth) max_depth_opt(max_depth)
    strategy_opt(strategy).
{
    A = TraverseArgs{direction.toString(), filter, root, min_depth, max_depth, strategy};
}

%type class_filter { set<string> }
class_filter(A) ::= . { A = set<string>(); }
class_filter(A) ::= name_set(X). { A = move(X); }

%type min_depth_opt { long long }
min_depth_opt(A) ::= . { A = 0; }
min_depth_opt(A) ::= MINDEPTH integer(X). { A = stoll(string(X.z, X.n)); }

%type max_depth_opt { long long }
max_depth_opt(A) ::= . { A = UINT_MAX; }
max_depth_opt(A) ::= MAXDEPTH integer(X). { A = stoll(string(X.z, X.n)); }

%type strategy_opt { string }
strategy_opt(A) ::= . { A = "DEPTH_FIRST"; }
strategy_opt(A) ::= STRATEGY IDENTITY(X). { A = X.toString(); }

//////////////////// The INDEX command ////////////////////
// CREATE
cmd ::= CREATE INDEX name(className) DOT name(propName) index_type(type) SEMI. {
    this->createIndex(className, propName, type);
}

// DROP
cmd ::= DROP INDEX name(className) DOT name(propName) SEMI. {
    this->dropIndex(className, propName);
}


index_type ::= .
index_type(A) ::= IDENTITY(X). { A = X; }


//////////////////// Other options ////////////////////
// if (not) exists
%type if_not_exists_opt { bool }
%type if_exists_opt { bool }
if_not_exists_opt(A) ::= . { A = false; }
if_not_exists_opt(A) ::= IF NOT EXISTS. { A = true; }
if_exists_opt(A) ::= . { A = false; }
if_exists_opt(A) ::= IF EXISTS. { A = true; }

// properties
%type props_opt { nogdb::Record }
%type props_list { nogdb::Record }
props_opt(A) ::= . { A = nogdb::Record(); }
props_opt(A) ::= SET props_list(X). { A = move(X); }
props_list(A) ::= props_list(A) COMMA prop_name(prop) EQ term(value). {
    A.set(prop, value.getBase());
}
props_list(A) ::= prop_name(prop) EQ term(value). {
    A = nogdb::Record().set(prop, value.getBase());
}

%type prop_name { string }
prop_name(A) ::= name(X). { A = X.toString(); }
prop_name(A) ::= AT(X) IDENTITY(Y). { A = string(X.z, (Y.z + Y.n) - X.z); }

// RID
%type rid { RecordDescriptor }
rid(A) ::= SHARP integer(class_id) COLON integer(pos_id). {
    A = RecordDescriptor(stoi(string(class_id.z, class_id.n)), stoi(string(pos_id.z, pos_id.n)));
}


//////////////////// List/Set ////////////////////
// rid_set
%type rid_set { RecordDescriptorSet }
rid_set(A) ::= rid_set(A) COMMA rid(X). { A.insert(move(X)); }
rid_set(A) ::= rid(X). { A = RecordDescriptorSet{X}; }

// name_set
%type name_set { set<string> }
name_set(A) ::= name_set(A) COMMA name(X). { A.insert(X.toString()); }
name_set(A) ::= name(X). { A = set<string>{X.toString()}; }

// term_list
%type term_list { vector<Bytes> }
term_list(A) ::= term_list(A) COMMA term(X). { A.push_back(move(X)); }
term_list(A) ::= term(X). { A = vector<Bytes>{move(X)}; }


//////////////////// Condition Processing ////////////////////
%left OR.
%left AND.
%left LT GT GE LE.
%left EQ NE.
%left NOT.

%type multi_cond { shared_ptr<MultiCondition> } // use shared_ptr because constructor of MultiCondition is already deleted
multi_cond(A) ::= LP multi_cond(X) RP. { A = X; }
multi_cond(A) ::= multi_cond(X) AND multi_cond(Y). { A = make_shared<MultiCondition>(*X && *Y); }
multi_cond(A) ::= multi_cond(X) OR multi_cond(Y). { A = make_shared<MultiCondition>(*X || *Y); }
multi_cond(A) ::= multi_cond(X) AND cond(Y). { A = make_shared<MultiCondition>(*X && Y); }
multi_cond(A) ::= multi_cond(X) OR cond(Y). { A = make_shared<MultiCondition>(*X || Y); }
multi_cond(A) ::= cond(X) AND multi_cond(Y). { A = make_shared<MultiCondition>(X && *Y); }
multi_cond(A) ::= cond(X) OR multi_cond(Y). { A = make_shared<MultiCondition>(X || *Y); }
multi_cond(A) ::= cond(X) AND cond(Y). { A = make_shared<MultiCondition>(X && Y); }
multi_cond(A) ::= cond(X) OR cond(Y). { A = make_shared<MultiCondition>(X || Y); }
multi_cond(A) ::= NOT multi_cond(X). { A = make_shared<MultiCondition>(!(*X)); }

%type cond { Condition }
cond(A) ::= LP cond(X) RP. { A = move(X); }
cond(A) ::= NOT cond(X). { A = !X; }
cond(A) ::= prop_name(prop) EQ term(value). { A = Condition(prop).eq(value); }
cond(A) ::= prop_name(prop) NE term(value). { A = !Condition(prop).eq(value); }
cond(A) ::= prop_name(prop) GT term(value). { A = Condition(prop).gt(value.getBase()); }
cond(A) ::= prop_name(prop) LT term(value). { A = Condition(prop).lt(value.getBase()); }
cond(A) ::= prop_name(prop) GE term(value). { A = Condition(prop).ge(value.getBase()); }
cond(A) ::= prop_name(prop) LE term(value). { A = Condition(prop).le(value.getBase()); }
cond(A) ::= prop_name(prop) IS term(value). { A = Condition(prop).eq(value); }
cond(A) ::= prop_name(prop) IS NOT term(value). { A = !Condition(prop).eq(value); }
cond(A) ::= prop_name(prop) CONTAIN CASE term(value). { A = Condition(prop).contain(value.getBase()); }
cond(A) ::= prop_name(prop) CONTAIN term(value). { A = Condition(prop).contain(value.getBase()).ignoreCase(); }
cond(A) ::= prop_name(prop) BEGIN WITH CASE term(value). { A = Condition(prop).beginWith(value.getBase()); }
cond(A) ::= prop_name(prop) BEGIN WITH term(value). { A = Condition(prop).beginWith(value.getBase()).ignoreCase(); }
cond(A) ::= prop_name(prop) END WITH CASE term(value). { A = Condition(prop).endWith(value.getBase()); }
cond(A) ::= prop_name(prop) END WITH term(value). { A = Condition(prop).endWith(value.getBase()).ignoreCase(); }
cond(A) ::= prop_name(prop) LIKE CASE term(value). { A = Condition(prop).like(value.getBase()); }
cond(A) ::= prop_name(prop) LIKE term(value). { A = Condition(prop).like(value.getBase()).ignoreCase(); }
cond(A) ::= prop_name(prop) REGEX CASE term(value). { A = Condition(prop).regex(value.getBase()); }
cond(A) ::= prop_name(prop) REGEX term(value). { A = Condition(prop).regex(value.getBase()).ignoreCase(); }
cond(A) ::= prop_name(prop) BETWEEN term(value1) AND term(value2). { A = Condition(prop).between(value1.getBase(), value2.getBase()); }
cond(A) ::= prop_name(prop) IDENTITY(cmp) LB term_list(values) RB. {
    if (strncasecmp(cmp.z, "IN", cmp.n) == 0) {
        vector<nogdb::Bytes> baseValues(values.size());
        transform(values.begin(), values.end(), baseValues.begin(), [](const Bytes& v){ return v.getBase(); });
        A = Condition(prop).in(baseValues);
    } else {
        this->syntax_error(-1, cmp);
    }
}
//cond(A) ::= IDENTITY(propA) cmp IDENTITY(propB). NOT_IMPLEMENTED

%type term { Bytes }
term(A) ::= term_token(X). { A = X.toBytes(); }
term_token(A) ::= NULL|FLOAT|STRING|SIGNED|UNSIGNED|BLOB(X). { A = X; }
