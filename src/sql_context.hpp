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

#ifndef __SQL_CONTEXT_HPP_INCLUDED_
#define __SQL_CONTEXT_HPP_INCLUDED_

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
            Context(Txn &txn_) : txn(txn_) {}

            Txn &txn;
            enum {
                SQL_OK,
                SQL_ERROR,
            } rc{SQL_OK};
            SQL::Result result;


            // parser error.
            void syntax_error(int tokenType, Token &token) {
                cerr << "nogdb::SQL::execute: syntax error near '" << string(token.z, token.n) << "'" << endl;
                rc = SQL_ERROR;
                result = SQL::Result(new Error(SQL_SYNTAX_ERROR, Error::Type::SQL));
            }

            void parse_failure() {
                cerr << "nogdb::SQL::execute: parse failure." << endl;
                rc = SQL_ERROR;
                result = SQL::Result(new Error(SQL_SYNTAX_ERROR, Error::Type::SQL));
            }


            // CLASS operations
            void createClass(const Token &tName, const Token &tExtends, char checkIfNotExists);

            void alterClass(const Token &tName, const Token &tAttr, const Bytes &value);

            void dropClass(const Token &tName, char checkIfExists);

            // PROPERTY operations
            void
            createProperty(const Token &tClassName, const Token &tPropName, const Token &tType, char checkIfNotExists);

            void alterProperty(const Token &tClassName, const Token &tPropName, const Token &tAttr, const Bytes &value);

            void dropProperty(const Token &tClassName, const Token &tPropName, char checkIfExists);

            // VERTEX operations
            void createVertex(const Token &tClassName, const nogdb::Record &prop);

            // EDGE operations
            void createEdge(const CreateEdgeArgs &args);

            // SELECT operations
            void select(const SelectArgs &args);

            // UPDATE operations
            void update(const UpdateArgs &args);

            // DELETE operations
            void deleteVertex(const DeleteVertexArgs &args);

            void deleteEdge(const DeleteEdgeArgs &args);

            // TRAVERSE operations
            void traverse(const TraverseArgs &args);

        private:
            void newTxnIfRootStmt(bool isRoot, Txn::Mode mode);

            void commitIfRootStmt(bool isRoot);

            void rollbackIfRootStmt(bool isRoot);

            ResultSet selectPrivate(const SelectArgs &stmt);

            ResultSet select(const Target &target, const Where &where);

            ResultSet select(const Target &target, const Where &where, int skip, int limit);

            ResultSet select(const RecordDescriptorSet &rids);

            ResultSetCursor selectVertex(const string &className, const Where &where);

            ResultSetCursor selectEdge(const string &className, const Where &where);

            ResultSet selectWhere(ResultSet &input, const Where &where);

            ResultSet selectProjection(ResultSet &input, const vector <Projection> projs);

            pair <string, Bytes>
            selectProjectionItem(const Result &input, const Projection &proj, const PropertyMapType &map);

            ResultSet selectGroupBy(ResultSet &input, const string &group);

            ResultSet traversePrivate(const TraverseArgs &stmt);

            ClassType findClassType(const string &className);

            PropertyMapType getPropertyMapTypeFromClassDescriptor(ClassId classID);

            /* LEMONXX base */
        public:
            static std::unique_ptr<Context> create(Txn &txn);
        };
    }
}

#endif
