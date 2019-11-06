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

#include <iostream>
#include <sstream>

#include "nogdb/nogdb.h"

namespace nogdb {
namespace sql_parser {
    using namespace std;

    /* Forward declaration */
    class Token;

    class Bytes;

    class Record;

    class Result;

    class ResultSet;

    typedef set<RecordDescriptor> RecordDescriptorSet;

    /*
     * Each token coming out of the lexer is an instance of
     * this structure. Tokens are also used as part of an expression.
     *
     * Note if Token.z==NULL then Token.n and Token.t are undefined and
     * may contain random values. Do not make any assumptions about Token.n
     * and Token.t when Token.z==NULL.
     */
    class Token {
    public:
        const char* z; /* Text of the token.  Not NULL-terminated! */
        int n; /* Number of characters in this token */
        int t; /* Token type ID */

        inline string toString() const
        {
            string s = string(this->z, this->n);
            return dequote(s);
        }

        inline string toRawString() const
        {
            return string(this->z, this->n);
        }

        Bytes toBytes() const;

    private:
        bool operator<(const Token& other) const;

        string& dequote(string& z) const;
    };

    class Bytes : public nogdb::Bytes {
    public:
        Bytes();

        template <typename T>
        Bytes(T data, PropertyType type_)
            : nogdb::Bytes(data)
            , t(type_)
        {
        }

        Bytes(const unsigned char* data, size_t len, PropertyType type_);

        Bytes(nogdb::Bytes&& bytes_, PropertyType type_ = PropertyType::UNDEFINED);

        Bytes(PropertyType type_);

        Bytes(ResultSet&& res);

        bool operator<(const Bytes& other) const;

        inline PropertyType type() const { return this->t; }

        inline bool isResults() const { return this->r.get() != nullptr; }

        inline ResultSet& results() const { return *this->r; }

        inline const nogdb::Bytes& getBase() const { return *this; }

    private:
        PropertyType t { PropertyType::UNDEFINED };
        shared_ptr<ResultSet> r { nullptr };
    };

    class Record {
    public:
        Record() = default;

        Record(nogdb::Record&& rec);

        Record& set(const string& propName, const Bytes& value);

        Record& set(const string& propName, Bytes&& value);

        const map<string, Bytes>& getAll() const;

        Bytes get(const string& propName) const;

        bool empty() const;

        nogdb::Record toBaseRecord() const;

    private:
        map<string, Bytes> properties {};
    };

    class Result {
    public:
        Result() = default;

        Result(nogdb::Result&& result)
            : Result(move(result.descriptor), move(result.record))
        {
        }

        Result(RecordDescriptor&& rid, Record&& record_)
            : descriptor(move(rid))
            , record(move(record_))
        {
        }

        RecordDescriptor descriptor {};
        Record record {};

        nogdb::Result toBaseResult() const
        {
            return nogdb::Result(descriptor, record.toBaseRecord());
        }
    };

    class ResultSet : public vector<Result> {
    public:
        using vector<Result>::vector;
        using vector<Result>::insert;

        ResultSet();

        ResultSet(nogdb::ResultSet&& res);

        ResultSet(nogdb::ResultSetCursor& res, int skip, int limit);

        string descriptorsToString() const;

        ResultSet& limit(int skip, int limit);
    };

    class Condition : public nogdb::Condition {
    public:
        Condition(const string& propName = "")
            : nogdb::Condition(propName)
        {
        }

        Condition(const nogdb::Condition& rhs)
            : nogdb::Condition(rhs)
        {
        }

        using nogdb::Condition::eq;

        Condition eq(const Bytes& value) const
        {
            if (!value.empty()) {
                return this->eq(value.getBase());
            } else {
                return this->null();
            }
        }
    };

    /* A class for arguments holder */
    template <class E>
    class Holder {
    public:
        Holder()
            : Holder(static_cast<E>(0))
        {
        }

        Holder(E type_, shared_ptr<void> value_ = nullptr)
            : type(type_)
            , value(value_)
        {
        }

        E type;
        shared_ptr<void> value;

        template <typename T>
        inline T& get() const
        {
            return *static_pointer_cast<T>(this->value);
        }

    protected:
    };

    enum class TargetType {
        NO_TARGET,
        CLASS, // std::string
        RIDS, // sql_parser::RecordDescriptorSet
        NESTED, // sql_parser::SelectArgs
        NESTED_TRAVERSE, // sql_paresr::TraverseArgs
    };
    enum class WhereType {
        NO_COND,
        CONDITION, // nogdb::Condition
        MULTI_COND // nogdb::MultiCondition
    };
    enum class ProjectionType {
        PROPERTY, // std::string
        FUNCTION, // sql_parser::Function
        METHOD, // std::pair<sql_parser::Projection, sql_parser::Projection>
        ARRAY_SELECTOR, // std::pair<sql_parser::Projection, unsigned long>
        CONDITION, // std::pair<sql_parser::Projection, nogdb::Condition>
        ALIAS, // std::pair<sql_parser::Projection, string>
    };
    typedef Holder<TargetType> Target; /* A classname, set of rid or nested select statement */
    typedef Holder<WhereType> Where; /* A condition class, expression class or empty condition */
    typedef Holder<ProjectionType> Projection;

    class Function {
    public:
        enum class Id {
            UNDEFINED,
            COUNT,
            MIN,
            MAX,
            IN,
            IN_E,
            IN_V,
            OUT,
            OUT_E,
            OUT_V,
            BOTH,
            BOTH_E,
            BOTH_V,
            EXPAND,
        };

        Function() = default;

        Function(const string& name, vector<Projection>&& args = {});

        string name;
        Id id;
        vector<Projection> args;

        Bytes execute(Transaction& txn, const Result& input) const;

        Bytes executeAggregateResult(const ResultSet& input) const;

        Bytes executeExpand(Transaction& txn, ResultSet& input) const;

        bool isAggregateResult() const;

        bool isWalkResult() const;

        bool isExpand() const;

    private:
        static Bytes count(const ResultSet& input, const vector<Projection>& args);

        //            static Bytes min(const ResultSet &input, const vector<Projection> &args);
        //            static Bytes max(const ResultSet &input, const vector<Projection> &args);
        static Bytes walkIn(Transaction& txn, const Result& input, const vector<Projection>& args);

        static Bytes walkInEdge(Transaction& txn, const Result& input, const vector<Projection>& args);

        static Bytes walkInVertex(Transaction& txn, const Result& input, const vector<Projection>& args);

        static Bytes walkOut(Transaction& txn, const Result& input, const vector<Projection>& args);

        static Bytes walkOutEdge(Transaction& txn, const Result& input, const vector<Projection>& args);

        static Bytes walkOutVertex(Transaction& txn, const Result& input, const vector<Projection>& args);

        static Bytes walkBoth(Transaction& txn, const Result& input, const vector<Projection>& args);

        static Bytes walkBothEdge(Transaction& txn, const Result& input, const vector<Projection>& args);

        static Bytes walkBothVertex(Transaction& txn, const Result& input, const vector<Projection>& args);

        static Bytes expand(Transaction& txn, ResultSet& input, const vector<Projection>& args);

        static vector<string> argsToClassFilter(const vector<Projection>& args);
    };

    /* An arguments for create edge statement */
    struct CreateEdgeArgs {
        string name;
        Target src;
        Target dest;
        nogdb::Record prop;
    };

    /* An arguments for select statement */
    struct SelectArgs {
        vector<Projection> projections;
        Target from;
        Where where;
        string group;
        void* order;
        int skip; /* Number of records you want to skip from the start of the result-set. */
        int limit; /* Maximum number of records in the result-set. */
    };

    /* An arguments for update statement */
    struct UpdateArgs {
        Target target;
        nogdb::Record prop;
        Where where;
    };

    /* An arguments for delete vertex statement */
    struct DeleteVertexArgs {
        Target target;
        Where where;
    };

    /* An arguments for delete edge statement */
    struct DeleteEdgeArgs {
        Target target;
        Target from;
        Target to;
        Where where;
    };

    /* An arguments for tarverse statement */
    struct TraverseArgs {
        string direction;
        set<string> filter;
        set<RecordDescriptor> root;
        long long minDepth;
        long long maxDepth;
        string strategy;
    };

    string to_string(const Projection& proj);
}
}