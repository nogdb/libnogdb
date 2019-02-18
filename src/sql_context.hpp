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

#include "lemonxx/lemon_base.h"
#include "sql.hpp"

namespace nogdb {
namespace sql_parser {
    /*
     * An SQL parser context. A reference of this structure is passed through
     * the parser and down into all the parser action routine in order to
     * carry around information that is global to the entire parse.
     */
    class Context : public lemon_base<Token> {
    public:
        friend class Function;

        Context(Transaction& txn_)
            : txn(txn_)
        {
        }

        Transaction& txn;
        enum {
            SQL_OK,
            SQL_ERROR,
        } rc { SQL_OK };
        SQL::Result result;

        // parser error.
        void syntax_error(int tokenType, Token& token)
        {
            cerr << "nogdb::SQL::execute: syntax error near '" << string(token.z, token.n) << "'" << endl;
            rc = SQL_ERROR;
            result = SQL::Result(new NOGDB_SQL_ERROR(NOGDB_SQL_SYNTAX_ERROR));
        }

        void parse_failure()
        {
            cerr << "nogdb::SQL::execute: parse failure." << endl;
            rc = SQL_ERROR;
            result = SQL::Result(new NOGDB_SQL_ERROR(NOGDB_SQL_SYNTAX_ERROR));
        }

        // CLASS operations
        void createClass(const Token& tName, const Token& tExtends, bool checkIfNotExists);

        void alterClass(const Token& tName, const Token& tAttr, const Bytes& value);

        void dropClass(const Token& tName, bool checkIfExists);

        // PROPERTY operations
        void
        createProperty(const Token& tClassName, const Token& tPropName, const Token& tType, bool checkIfNotExists);

        void alterProperty(const Token& tClassName, const Token& tPropName, const Token& tAttr, const Bytes& value);

        void dropProperty(const Token& tClassName, const Token& tPropName, bool checkIfExists);

        // VERTEX operations
        void createVertex(const Token& tClassName, const nogdb::Record& prop);

        // EDGE operations
        void createEdge(const CreateEdgeArgs& args);

        // SELECT operations
        void select(const SelectArgs& args);

        // UPDATE operations
        void update(const UpdateArgs& args);

        // DELETE operations
        void deleteVertex(const DeleteVertexArgs& args);

        void deleteEdge(const DeleteEdgeArgs& args);

        // TRAVERSE operations
        void traverse(const TraverseArgs& args);

        // INDEX operations
        void createIndex(const Token& tClassName, const Token& tPropName, const Token& tIndexType);

        void dropIndex(const Token& tClassName, const Token& tPropName);

    private:
        void newTxnIfRootStmt(bool isRoot, TxnMode mode);

        void commitIfRootStmt(bool isRoot);

        void rollbackIfRootStmt(bool isRoot);

        ResultSet selectPrivate(const SelectArgs& stmt);

        ResultSet select(const Target& target, const Where& where);

        ResultSet select(const Target& target, const Where& where, int skip, int limit);

        ResultSet select(const RecordDescriptorSet& rids);

        ResultSetCursor selectVertex(const string& className, const Where& where);

        ResultSetCursor selectEdge(const string& className, const Where& where);

        ResultSet selectWhere(ResultSet& input, const Where& where);

        ResultSet selectProjection(ResultSet& input, const vector<Projection> projs);

        ResultSet selectGroupBy(ResultSet& input, const string& group);

        ResultSet traversePrivate(const TraverseArgs& stmt);

        static Bytes getProjectionItem(Transaction& txn, const Result& input, const Projection& proj, const PropertyMapType& map);

        static Bytes
        getProjectionItemProperty(Transaction& txn, const Result& input, const string& propName, const PropertyMapType& map);

        static Bytes
        getProjectionItemMethod(Transaction& txn, const Result& input, const Projection& firstProj, const Projection& secondProj,
            const PropertyMapType& map);

        static Bytes
        getProjectionItemArraySelector(Transaction& txn, const Result& input, const Projection& proj, unsigned long index,
            const PropertyMapType& map);

        static Bytes
        getProjectionItemCondition(Transaction& txn, const Result& input, const Function& func, const Condition& cond);

        static ClassType findClassType(Transaction& txn, const string& className);

        static PropertyMapType getPropertyMapTypeFromClassDescriptor(Transaction& txn, ClassId classID);

        static ResultSet executeCondition(Transaction& txn, const ResultSet& input, const MultiCondition& conds);

        /* LEMONXX base */
    public:
        static std::unique_ptr<Context> create(Transaction& txn);
    };
}
}
