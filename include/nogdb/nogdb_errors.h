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

#include <exception>
#include <stdexcept>
#include <string>

#include "lmdb/lmdb.h"

//*************************************************************
//*  NogDB errors.                                            *
//*************************************************************

#define NOGDB_SUCCESS 0x0

#define NOGDB_GRAPH_DUP_VERTEX 0x100
#define NOGDB_GRAPH_NOEXST_VERTEX 0x101
#define NOGDB_GRAPH_NOEXST_SRC 0x102
#define NOGDB_GRAPH_NOEXST_DST 0x103
#define NOGDB_GRAPH_DUP_EDGE 0x200
#define NOGDB_GRAPH_NOEXST_EDGE 0x201
#define NOGDB_GRAPH_UNKNOWN_ERR 0x9ff

#define NOGDB_INTERNAL_NULL_TXN 0xa00
#define NOGDB_INTERNAL_EMPTY_DBI 0xa01
#define NOGDB_INTERNAL_UNKNOWN_ERROR 0xcff

#define NOGDB_TXN_INVALID_MODE 0xd00
#define NOGDB_TXN_COMPLETED 0xd01
#define NOGDB_TXN_UNKNOWN_ERR 0xfff

#define NOGDB_CTX_INVALID_CLASSTYPE 0x1000
#define NOGDB_CTX_DUPLICATE_CLASS 0x1010
#define NOGDB_CTX_NOEXST_CLASS 0x1020
#define NOGDB_CTX_INVALID_CLASSNAME 0x1030
#define NOGDB_CTX_MISMATCH_CLASSTYPE 0x1990
#define NOGDB_CTX_INVALID_PROPTYPE 0x2000
#define NOGDB_CTX_DUPLICATE_PROPERTY 0x2010
#define NOGDB_CTX_NOEXST_PROPERTY 0x2020
#define NOGDB_CTX_INVALID_PROPERTYNAME 0x2030
#define NOGDB_CTX_OVERRIDE_PROPERTY 0x2040
#define NOGDB_CTX_CONFLICT_PROPTYPE 0x2050
#define NOGDB_CTX_IN_USED_PROPERTY 0x2060
#define NOGDB_CTX_NOEXST_RECORD 0x3000
#define NOGDB_CTX_INVALID_COMPARATOR 0x4000
#define NOGDB_CTX_INVALID_PROPTYPE_INDEX 0x6000
#define NOGDB_CTX_NOEXST_INDEX 0x6010
#define NOGDB_CTX_DUPLICATE_INDEX 0x6020
#define NOGDB_CTX_INVALID_INDEX_CONSTRAINT 0x6030
#define NOGDB_CTX_UNIQUE_CONSTRAINT 0x6040
#define NOGDB_CTX_UNINITIALIZED 0x7000
#define NOGDB_CTX_ALREADY_INITIALIZED 0x7010
#define NOGDB_CTX_DBSETTING_MISSING 0x7020
#define NOGDB_CTX_MAXCLASS_REACH 0x9fd0
#define NOGDB_CTX_MAXPROPERTY_REACH 0x9fd1
#define NOGDB_CTX_MAXINDEX_REACH 0x9fd2
#define NOGDB_CTX_INTERNAL_ERR 0x9fe0
#define NOGDB_CTX_UNKNOWN_ERR 0x9ff0
#define NOGDB_CTX_NOT_IMPLEMENTED 0x9fff

#define NOGDB_SQL_UNRECOGNIZED_TOKEN 0xa001
#define NOGDB_SQL_SYNTAX_ERROR 0xa002
#define NOGDB_SQL_STACK_OVERFLOW 0xa003
#define NOGDB_SQL_NUMBER_FORMAT_EXCEPTION 0xa004
#define NOGDB_SQL_INVALID_ALTER_ATTR 0xa005
#define NOGDB_SQL_INVALID_COMPARATOR 0xa006
#define NOGDB_SQL_INVALID_FUNCTION_NAME 0xa007
#define NOGDB_SQL_INVALID_FUNCTION_ARGS 0xa008
#define NOGDB_SQL_INVALID_PROJECTION 0xa009
#define NOGDB_SQL_INVALID_TRAVERSE_DIRECTION 0xa00a
#define NOGDB_SQL_INVALID_TRAVERSE_MIN_DEPTH 0xa00b
#define NOGDB_SQL_INVALID_TRAVERSE_MAX_DEPTH 0xa00c
#define NOGDB_SQL_INVALID_TRAVERSE_STRATEGY 0xa00d
#define NOGDB_SQL_INVALID_PROJECTION_METHOD 0xa00e
#define NOGDB_SQL_NOT_IMPLEMENTED 0xaf01
#define NOGDB_SQL_UNKNOWN_ERR 0xafff

namespace nogdb {

/**
   *  NogDB generic error definition
   */

enum class ErrorType {
    INTERNAL_ERROR,
    STORAGE_ERROR,
    GRAPH_ERROR,
    CONTEXT_ERROR,
    TXN_ERROR,
    SQL_ERROR
};

class Error : public std::runtime_error {
public:
    /**
     * Constructor
     */
    explicit Error(const int code, const std::string& func, const std::string& file, const int line, const ErrorType& type)
        : std::runtime_error { func + " in " + file + ":" + std::to_string(line) }
        , _code { code }
        , _func { func }
        , _file { file }
        , _line { line }
        , _type { type }
    {
    }

    Error(const Error& error) noexcept
        : std::runtime_error { error._func + " in " + error._file + ":" + std::to_string(error._line) }
        , _code { error._code }
        , _func { error._func }
        , _file { error._file }
        , _line { error._line }
        , _type { error._type }
    {
    }

    Error& operator=(const Error& error) noexcept
    {
        if (this != &error) {
            auto tmp = Error(error);
            using std::swap;
            swap(tmp, *this);
        }
        return *this;
    }

    virtual ~Error() noexcept = default;

    virtual int code() const noexcept
    {
        return _code;
    }

    virtual std::string origin() const noexcept
    {
        return runtime_error::what();
    }

    virtual const char* what() const noexcept
    {
        return "NOGDB_UNKNOWN_ERR: Unknown";
    }

    const ErrorType& type() const noexcept
    {
        return _type;
    }

    const std::string func() const noexcept
    {
        return _func;
    }

    const std::string file() const noexcept
    {
        return _file;
    }

    int line() const noexcept
    {
        return _line;
    }

protected:
    const int _code;
    const std::string _func {};
    const std::string _file {};
    const int _line;
    const ErrorType _type;
};

/**
   * NogDB internal error definition
   */
class InternalError : public Error {
public:
    using Error::Error;

    virtual const char* what() const noexcept
    {
        switch (_code) {
        case NOGDB_INTERNAL_NULL_TXN:
            return "NOGDB_INTERNAL_NULL_TXN: An underlying txn is NULL";
        case NOGDB_INTERNAL_EMPTY_DBI:
            return "NOGDB_INTERNAL_EMPTY_DBI: An underlying database interface is empty";
        case NOGDB_INTERNAL_UNKNOWN_ERROR:
        default:
            return "NOGDB_INTERNAL_UNKNOWN_ERR: Unknown";
        }
    }
};

/**
   * NogDB storage error definition
   */
class StorageError : public Error {
public:
    using Error::Error;

    virtual const char* what() const noexcept
    {
        return mdb_strerror(_code);
    }
};

/**
   * NogDB graph error definition
   */
class GraphError : public Error {
public:
    using Error::Error;

    virtual const char* what() const noexcept
    {
        switch (_code) {
        case NOGDB_GRAPH_DUP_VERTEX:
            return "NOGDB_GRAPH_DUP_VERTEX: A duplicated vertex in a graph";
        case NOGDB_GRAPH_NOEXST_VERTEX:
            return "NOGDB_GRAPH_NOEXST_VERTEX: A vertex doesn't exist";
        case NOGDB_GRAPH_NOEXST_SRC:
            return "NOGDB_GRAPH_NOEXST_SRC: A source vertex doesn't exist";
        case NOGDB_GRAPH_NOEXST_DST:
            return "NOGDB_GRAPH_NOEXST_DST: A destination vertex doesn't exist";
        case NOGDB_GRAPH_DUP_EDGE:
            return "NOGDB_GRAPH_DUP_EDGE: A duplicated edge in a graph";
        case NOGDB_GRAPH_NOEXST_EDGE:
            return "NOGDB_GRAPH_NOEXST_EDGE: An edge doesn't exist";
        case NOGDB_GRAPH_UNKNOWN_ERR:
        default:
            return "NOGDB_GRAPH_UNKNOWN_ERR: Unknown";
        }
    }
};

/**
   * NogDB context error definition
   */
class ContextError : public Error {
public:
    using Error::Error;

    virtual const char* what() const noexcept
    {
        switch (_code) {
        case NOGDB_CTX_INVALID_CLASSTYPE:
            return "NOGDB_CTX_INVALID_CLASSTYPE: A type of class is not valid";
        case NOGDB_CTX_DUPLICATE_CLASS:
            return "NOGDB_CTX_DUPLICATE_CLASS: A specified class name has already existed";
        case NOGDB_CTX_NOEXST_CLASS:
            return "NOGDB_CTX_NOEXST_CLASS: A class does not exist";
        case NOGDB_CTX_INVALID_PROPTYPE:
            return "NOGDB_CTX_INVALID_PROPTYPE: A type of property is not valid";
        case NOGDB_CTX_DUPLICATE_PROPERTY:
            return "NOGDB_CTX_DUPLICATE_PROPERTY: A specified property name has already existed";
        case NOGDB_CTX_OVERRIDE_PROPERTY:
            return "NOGDB_CTX_OVERRIDE_PROPERTY: A specified property name has already existed in some sub-classes";
        case NOGDB_CTX_NOEXST_PROPERTY:
            return "NOGDB_CTX_NOEXST_PROPERTY: A property does not exist";
        case NOGDB_CTX_CONFLICT_PROPTYPE:
            return "NOGDB_CTX_CONFLICT_PROPTYPE: Some properties do not have the same type";
        case NOGDB_CTX_IN_USED_PROPERTY:
            return "NOGDB_CTX_IN_USED_PROPERTY: A property is used by one or more database indexes";
        case NOGDB_CTX_NOEXST_RECORD:
            return "NOGDB_CTX_NOEXST_RECORD: A record with the given descriptor doesn't exist";
        case NOGDB_CTX_MISMATCH_CLASSTYPE:
            return "NOGDB_CTX_MISMATCH_CLASSTYPE: A type of a class does not match as expected";
        case NOGDB_CTX_INTERNAL_ERR:
            return "NOGDB_CTX_INTERNAL_ERROR: Oops! there might be some internal errors";
        case NOGDB_CTX_INVALID_COMPARATOR:
            return "NOGDB_CTX_INVALID_COMPARATOR: A comparator is not defined";
        case NOGDB_CTX_INVALID_CLASSNAME:
            return "NOGDB_CTX_INVALID_CLASSNAME: A class name is empty or contains invalid characters";
        case NOGDB_CTX_INVALID_PROPERTYNAME:
            return "NOGDB_CTX_INVALID_PROPERTYNAME: A property name is empty or contains invalid characters";
        case NOGDB_CTX_MAXCLASS_REACH:
            return "NOGDB_CTX_MAXCLASS_REACH: A limitation of class number has been reached";
        case NOGDB_CTX_MAXPROPERTY_REACH:
            return "NOGDB_CTX_MAXPROPERTY_REACH: A limitation of property number has been reached";
        case NOGDB_CTX_MAXINDEX_REACH:
            return "NOGDB_CTX_MAXINDEX_REACH: A limitation of index number has been reached";
        case NOGDB_CTX_NOT_IMPLEMENTED:
            return "NOGDB_CTX_NOT_IMPLEMENTED: A function or class has not been implemented yet";
        case NOGDB_CTX_INVALID_PROPTYPE_INDEX:
            return "NOGDB_CTX_INVALID_PROPTYPE_INDEX: A property type doesn't support database indexing";
        case NOGDB_CTX_NOEXST_INDEX:
            return "NOGDB_CTX_NOEXST_INDEX: An index doesn't exist on given class and property";
        case NOGDB_CTX_DUPLICATE_INDEX:
            return "NOGDB_CTX_DUPLICATE_INDEX: A specified index has already existed";
        case NOGDB_CTX_INVALID_INDEX_CONSTRAINT:
            return "NOGDB_CTX_INVALID_INDEX_CONSTRAINT: An index couldn't be created with a unique constraint due to some duplicated values in existing records";
        case NOGDB_CTX_UNIQUE_CONSTRAINT:
            return "NOGDB_CTX_UNIQUE_CONSTRAINT: A record has some duplicated values when a unique constraint is applied";
        case NOGDB_CTX_UNINITIALIZED:
            return "NOGDB_CTX_UNINITIALIZED: A database is not initialized";
        case NOGDB_CTX_ALREADY_INITIALIZED:
            return "NOGDB_CTX_ALREADY_INITIALIZED: A database already exists";
        case NOGDB_CTX_DBSETTING_MISSING:
            return "NOGDB_CTX_DBSETTING_MISSING: A database setting is missing";
        case NOGDB_CTX_UNKNOWN_ERR:
        default:
            return "NOGDB_CTX_UNKNOWN_ERR: Unknown";
        }
    }
};

/**
   * NogDB transaction error definition
   */
class TxnError : public Error {
public:
    using Error::Error;

    virtual const char* what() const noexcept
    {
        switch (_code) {
        case NOGDB_TXN_INVALID_MODE:
            return "NOGDB_TXN_INVALID_MODE: An operation couldn't be executed due to an invalid transaction mode";
        case NOGDB_TXN_COMPLETED:
            return "NOGDB_TXN_COMPLETED: An operation couldn't be executed due to a completed transaction";
        case NOGDB_TXN_UNKNOWN_ERR:
        default:
            return "NOGDB_TXN_UNKNOWN_ERR: Unknown";
        }
    }
};

/**
   * NogDB SQL error definition
   */
class SQLError : public Error {
public:
    using Error::Error;

    virtual const char* what() const noexcept
    {
        switch (_code) {
        case NOGDB_SQL_UNRECOGNIZED_TOKEN:
            return "NOGDB_SQL_UNRECOGNIZED_TOKEN: A SQL has some word or keyword that can't recognize.";
        case NOGDB_SQL_SYNTAX_ERROR:
            return "NOGDB_SQL_SYNTAX_ERROR: A SQL syntax error.";
        case NOGDB_SQL_STACK_OVERFLOW:
            return "NOGDB_SQL_STACK_OVERFLOW: A parser stack overflow.";
        case NOGDB_SQL_NUMBER_FORMAT_EXCEPTION:
            return "NOGDB_SQL_NUMBER_FORMAT_EXCEPTION: A number is incorrect format or over limits.";
        case NOGDB_SQL_INVALID_ALTER_ATTR:
            return "NOGDB_SQL_INVALID_ALTER_ATTR: A attribute of alter is invalid (or unknown).";
        case NOGDB_SQL_INVALID_COMPARATOR:
            return "NOGDB_SQL_INVALID_COMPARATOR: A comparator is invalid for this function.";
        case NOGDB_SQL_INVALID_FUNCTION_NAME:
            return "NOGDB_SQL_INVALID_FUNCTION_NAME: A function name is invalid (or unknown).";
        case NOGDB_SQL_INVALID_FUNCTION_ARGS:
            return "NOGDB_SQL_INVALID_FUNCTION_ARGS: A arguments of function is invalid (invalid args).";
        case NOGDB_SQL_INVALID_PROJECTION:
            return "NOGDB_SQL_INVALID_PROJECTION: Projection(s) of select statement is invalid.";
        case NOGDB_SQL_INVALID_TRAVERSE_DIRECTION:
            return "NOGDB_SQL_INVALID_TRAVERSE_DIRECTION: Traverse direction must be in, out or all.";
        case NOGDB_SQL_INVALID_TRAVERSE_MIN_DEPTH:
            return "NOGDB_SQL_INVALID_TRAVERSE_MIN_DEPTH: Traverse minimum depth must be unsigned integer.";
        case NOGDB_SQL_INVALID_TRAVERSE_MAX_DEPTH:
            return "NOGDB_SQL_INVALID_TRAVERSE_MAX_DEPTH: Traverse maximum depth must be unsigned integer.";
        case NOGDB_SQL_INVALID_TRAVERSE_STRATEGY:
            return "NOGDB_SQL_INVALID_TRAVERSE_STRATEGY: Traverse strategy must be DEPTH_FIRST or BREADTH_FIRST.";
        case NOGDB_SQL_INVALID_PROJECTION_METHOD:
            return "NOGDB_SQL_INVALID_PROJECTION_METHOD: Projection method has some problem (invalid results).";
        case NOGDB_SQL_NOT_IMPLEMENTED:
            return "NOGDB_SQL_NOT_IMPLEMENTED: A function has not been implemented yet.";
        case NOGDB_SQL_UNKNOWN_ERR:
        default:
            return "NOGDB_SQL_UNKNOWN_ERROR: Unknown";
        }
    }
};

/**
   * NogDB Fatal error definition
   */
class FatalError : public std::runtime_error {
public:
    explicit FatalError(const Error& error)
        : std::runtime_error { error.func() + " in " + error.file() + ":" + std::to_string(error.line()) }
        , _code{error.code()}
        , _func{error.func()}
        , _file{error.file()}
        , _line{error.line()}
        , _type{error.type()}
        , _what{error.what()}
    {

    }

    FatalError(const FatalError& error) noexcept
        : std::runtime_error { error._func + " in " + error._file + ":" + std::to_string(error._line) }
        , _code{error._code}
        , _func{error._func}
        , _file{error._file}
        , _line{error._line}
        , _type{error._type}
        , _what{error._what}
    {

    }

    FatalError& operator=(const FatalError& error) noexcept
    {
        if (this != &error) {
            auto tmp = FatalError(error);
            using std::swap;
            swap(tmp, *this);
        }
        return *this;
    }

    virtual ~FatalError() noexcept = default;

    virtual const char* what() const noexcept
    {
        return ("(FATAL) " + _what).c_str();
    }

private:
  const int _code;
  const std::string _func {};
  const std::string _file {};
  const int _line;
  const ErrorType _type;
  const std::string _what {};
};

#define NOGDB_INTERNAL_ERROR(error) nogdb::InternalError(error, __FUNCTION__, __FILE__, __LINE__, nogdb::ErrorType::INTERNAL_ERROR)
#define NOGDB_STORAGE_ERROR(error) nogdb::StorageError(error, __FUNCTION__, __FILE__, __LINE__, nogdb::ErrorType::STORAGE_ERROR)
#define NOGDB_GRAPH_ERROR(error) nogdb::GraphError(error, __FUNCTION__, __FILE__, __LINE__, nogdb::ErrorType::GRAPH_ERROR)
#define NOGDB_CONTEXT_ERROR(error) nogdb::ContextError(error, __FUNCTION__, __FILE__, __LINE__, nogdb::ErrorType::CONTEXT_ERROR)
#define NOGDB_TXN_ERROR(error) nogdb::TxnError(error, __FUNCTION__, __FILE__, __LINE__, nogdb::ErrorType::TXN_ERROR)
#define NOGDB_SQL_ERROR(error) nogdb::SQLError(error, __FUNCTION__, __FILE__, __LINE__, nogdb::ErrorType::SQL_ERROR)
#define NOGDB_FATAL_ERROR(error) nogdb::FatalError(error)

}
