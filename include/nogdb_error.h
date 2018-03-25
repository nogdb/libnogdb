/*
 *  Copyright (C) 2018, Throughwave (Thailand) Co., Ltd.
 *  <peerawich at throughwave dot co dot th>
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

#ifndef __NOGDB_ERR_H_INCLUDED_
#define __NOGDB_ERR_H_INCLUDED_

#include <exception>
#include "lmdb/lmdb.h"

//*************************************************************
//*  NogDB errors.                                            *
//*************************************************************

#define ALL_OK                            0x0

#define GRAPH_DUP_VERTEX                0x100
#define GRAPH_NOEXST_VERTEX             0x101
#define GRAPH_NOEXST_SRC                0x102
#define GRAPH_NOEXST_DST                0x103
#define GRAPH_DUP_EDGE                  0x200
#define GRAPH_NOEXST_EDGE               0x201
#define GRAPH_UNKNOWN_ERR               0x9ff

#define TXN_INVALID_MODE                0xa00
#define TXN_COMPLETED                   0xa01
#define TXN_VERSION_MAXREACH            0xa02
#define TXN_UNKNOWN_ERR                 0xfff

#define CTX_INVALID_CLASSTYPE           0x1000
#define CTX_DUPLICATE_CLASS             0x1010
#define CTX_NOEXST_CLASS                0x1020
#define CTX_EMPTY_CLASSNAME             0x1030
#define CTX_MISMATCH_CLASSTYPE          0x1990
#define CTX_INVALID_PROPTYPE            0x2000
#define CTX_DUPLICATE_PROPERTY          0x2010
#define CTX_NOEXST_PROPERTY             0x2020
#define CTX_EMPTY_PROPERTYNAME          0x2030
#define CTX_OVERRIDE_PROPERTY           0x2040
#define CTX_CONFLICT_PROPTYPE           0x2050
#define CTX_IN_USED_PROPERTY            0x2060
//#define CTX_EMPTY_RECORD			    0x3000
#define CTX_INVALID_COMPARATOR          0x4000
#define CTX_INVALID_PROPTYPE_INDEX      0x6000
#define CTX_NOEXST_INDEX                0x6010
#define CTX_DUPLICATE_INDEX             0x6020
#define CTX_INVALID_INDEX_CONSTRAINT    0x6030
#define CTX_UNIQUE_CONSTRAINT           0x6040
#define CTX_IS_LOCKED                   0x9fc0
#define CTX_LIMIT_DBSCHEMA              0x9fd0
#define CTX_INTERNAL_ERR                0x9fe0
#define CTX_UNKNOWN_ERR                 0x9ff0
#define CTX_NOT_IMPLEMENTED             0x9fff

namespace nogdb {

//*************************************************************
//*  NogDB error definition.                                  *
//*************************************************************

    class Error : public std::exception {
    public:
        enum class Type {
            DATASTORE = 'd',
            GRAPH = 'g',
            CONTEXT = 'c',
            TRANSACTION = 't'
        };

        Error(int code, Type type) : code_{code}, type_{type} {}

        virtual int code() const {
            return code_;
        }

        virtual const char *what() const throw() {
            switch (type_) {
                case Type::DATASTORE:
                    return mdb_strerror(code_);
                case Type::GRAPH:
                    switch (code_) {
                        case GRAPH_DUP_VERTEX:
                            return "GRAPH_DUP_VERTEX: A duplicated vertex in a graph";
                        case GRAPH_NOEXST_VERTEX:
                            return "GRAPH_NOEXST_VERTEX: A vertex doesn't exist";
                        case GRAPH_NOEXST_SRC:
                            return "GRAPH_NOEXST_SRC: A source vertex doesn't exist";
                        case GRAPH_NOEXST_DST:
                            return "GRAPH_NOEXST_DST: A destination vertex doesn't exist";
                        case GRAPH_DUP_EDGE:
                            return "GRAPH_DUP_EDGE: A duplicated edge in a graph";
                        case GRAPH_NOEXST_EDGE:
                            return "GRAPH_NOEXST_EDGE: An edge doesn't exist";
                        case GRAPH_UNKNOWN_ERR:
                        default:
                            return "GRAPH_UNKNOWN_ERR: Unknown";
                    }
                case Type::CONTEXT:
                    switch (code_) {
                        case CTX_INVALID_CLASSTYPE:
                            return "CTX_INVALID_CLASSTYPE: A type of class is not valid";
                        case CTX_DUPLICATE_CLASS:
                            return "CTX_DUPLICATE_CLASS: A specified class name has already existed";
                        case CTX_NOEXST_CLASS:
                            return "CTX_NOEXST_CLASS: A class does not exist";
                        case CTX_INVALID_PROPTYPE:
                            return "CTX_INVALID_PROPTYPE: A type of property is not valid";
                        case CTX_DUPLICATE_PROPERTY:
                            return "CTX_DUPLICATE_PROPERTY: A specified property name has already existed";
                        case CTX_OVERRIDE_PROPERTY:
                            return "CTX_OVERRIDE_PROPERTY: A specified property name has already existed in some sub-classes";
                        case CTX_NOEXST_PROPERTY:
                            return "CTX_NOEXST_PROPERTY: A property does not exist";
                        case CTX_CONFLICT_PROPTYPE:
                            return "CTX_CONFLICT_PROPTYPE: Some properties do not have the same type";
                        case CTX_IN_USED_PROPERTY:
                            return "CTX_IN_USED_PROPERTY: A property is used by one or more database indexes";
                        case CTX_MISMATCH_CLASSTYPE:
                            return "CTX_MISMATCH_CLASSTYPE: A type of a class does not match as expected";
//            case CTX_EMPTY_RECORD:
//                return "CTX_EMPTY_RECORD: A record has no properties (required at least one)";
                        case CTX_INTERNAL_ERR:
                            return "CTX_INTERNAL_ERROR: Oops! there might be some errors internally";
                        case CTX_INVALID_COMPARATOR:
                            return "CTX_INVALID_COMPARATOR: A comparator is not defined";
                        case CTX_EMPTY_CLASSNAME:
                            return "CTX_EMPTY_CLASSNAME: A length of classname is zero";
                        case CTX_EMPTY_PROPERTYNAME:
                            return "CTX_EMPTY_PROPERTYNAME: A length of property name is zero";
                        case CTX_IS_LOCKED:
                            return "CTX_IS_LOCKED: A context is locked or being used";
                        case CTX_LIMIT_DBSCHEMA:
                            return "CTX_LIMIT_DBSCHEMA: A limitation of a database schema has been reached";
                        case CTX_NOT_IMPLEMENTED:
                            return "CTX_NOT_IMPLEMENTED: A function or class has not been implemented yet";
                        case CTX_INVALID_PROPTYPE_INDEX:
                            return "CTX_INVALID_PROPTYPE_INDEX: A property type doesn't support database indexing";
                        case CTX_NOEXST_INDEX:
                            return "CTX_NOEXST_INDEX: An index doesn't exist on given class and property";
                        case CTX_DUPLICATE_INDEX:
                            return "CTX_DUPLICATE_INDEX: A specified index has already existed";
                        case CTX_INVALID_INDEX_CONSTRAINT:
                            return "CTX_INVALID_INDEX_CONSTRAINT: An index couldn't be created with a unique constraint due to some duplicated values in existing records";
                        case CTX_UNIQUE_CONSTRAINT:
                            return "CTX_UNIQUE_CONSTRAINT: A record has some duplicated values when a unique constraint is applied";
                        case CTX_UNKNOWN_ERR:
                        default:
                            return "CTX_UNKNOWN_ERR: Unknown";
                    }
                case Type::TRANSACTION:
                    switch (code_) {
                        case TXN_INVALID_MODE:
                            return "TXN_INVALID_MODE: An operation couldn't be executed due to an invalid transaction mode";
                        case TXN_COMPLETED:
                            return "TXN_COMPLETED: An operation couldn't be executed due to a completed transaction";
                        case TXN_VERSION_MAXREACH:
                            return "TXN_VERSION_MAXREACH: The transaction version has been reached the maximum value";
                        case TXN_UNKNOWN_ERR:
                        default:
                            return "TXN_UNKNOWN_ERR: Unknown";
                    }
                default:
                    return "UNKNOWN_ERR: Unknown";
            }
        }

    private:
        int code_;
        Type type_;
    };

}
#endif
