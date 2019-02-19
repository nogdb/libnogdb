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

#include <cassert>
#include <cstring>
#include <functional>
#include <numeric>

#include "constant.hpp"
#include "sql.hpp"
#include "sql_context.hpp"
#include "sql_parser.h"
#include "utils.hpp"

#include "nogdb/nogdb.h"

using namespace std;
using namespace nogdb::sql_parser;
using namespace nogdb::utils::assertion;

#pragma mark - Helper

/*
 ** Translate a single byte of Hex into an integer.
 ** This routine only works if h really is a valid hexadecimal
 ** character:  0..9a..fA..F
 */
static inline uint8_t HexToInt(int h)
{
    assert((h >= '0' && h <= '9')
        || (h >= 'a' && h <= 'f')
        || (h >= 'A' && h <= 'F'));
    return (uint8_t)((h + 9 * (1 & (h >> 6))) & 0xf);
}

/*
 * Convert a BLOB literal of the form "x'hhhhhh'" into its binary
 * value. Return a pointer to its binary value. Space to hold the
 * binary value has been obtained from malloc and must be freed by
 * the calling routine.
 */
static unsigned char* HexToBlob(const char* z, int n)
{
    unsigned char* blob = (unsigned char*)malloc(n / 2 + 1);
    n--;
    if (blob) {
        int i;
        for (i = 0; i < n; i += 2) {
            blob[i / 2] = (HexToInt(z[i]) << 4) | HexToInt(z[i + 1]);
        }
        blob[i / 2] = 0;
    }
    return blob;
}

typedef function<bool(const string&, const string&)> StringCaseCompare;

static bool stringcasecmp(const string& a, const string& b)
{
    return strcasecmp(a.c_str(), b.c_str()) < 0;
}

string nogdb::sql_parser::to_string(const Projection& proj)
{
    switch (proj.type) {
    case ProjectionType::PROPERTY:
        return proj.get<string>();
    case ProjectionType::FUNCTION:
        return proj.get<Function>().name;
    case ProjectionType::METHOD:
        return to_string(proj.get<pair<Projection, Projection>>().first);
    case ProjectionType::ARRAY_SELECTOR:
        return to_string(proj.get<pair<Projection, unsigned long>>().first);
    case ProjectionType::CONDITION:
        return to_string(proj.get<pair<Projection, Condition>>().first);
    case ProjectionType::ALIAS:
        return proj.get<pair<Projection, string>>().second;
    default:
        assert(false);
        abort();
    }
}

#pragma mark - Token

Bytes Token::toBytes() const
{
    switch (this->t) {
    case TK_NULL:
        return Bytes();
    case TK_FLOAT:
        return Bytes(stod(string(this->z, this->n)), nogdb::PropertyType::REAL);
    case TK_STRING:
        return Bytes(nogdb::Bytes(this->toString()), nogdb::PropertyType::TEXT);
    case TK_SIGNED:
        return Bytes(stoll(string(this->z, this->n)), nogdb::PropertyType::BIGINT);
    case TK_UNSIGNED:
        return Bytes(stoull(string(this->z, this->n)), nogdb::PropertyType::UNSIGNED_BIGINT);
    case TK_BLOB: {
        // "x'hhhh' or X'hhhh'"
        const char* z = this->z + 2;
        int n = this->n - 3; // -3 is remove X and two quote.
        auto blob = unique_ptr<unsigned char[]>(HexToBlob(z, n));
        return Bytes(blob.get(), n / 2, nogdb::PropertyType::BLOB);
    }
    default:
        require(false);
        return Bytes();
    }
}

#pragma mark-- private

bool Token::operator<(const Token& other) const
{
    return this->n < other.n && strncmp(this->z, other.z, this->n) < 0;
}

/*
 * Convert an SQL-style quoted string into a normal string by removing
 * the quote characters. The conversion is done in-place.  If the
 * input does not begin with a quote character, then this routine
 * is a no-op.
 */
string& Token::dequote(string& z) const
{
    char quote;
    int i, j;
    quote = z[0];
    if (quote == '"'
        || quote == '\''
        || quote == '`'
        || quote == '[') {
        if (quote == '[') {
            quote = ']';
        }
        for (i = 1, j = 0;; i++) {
            require(z[i]);
            if (z[i] == quote) {
                break;
            } else if (z[i] == '\\') {
                if (z[i + 1] == quote) {
                    // if backslash quote (escape quote), remove backslash.
                    z[j++] = quote;
                } else if (z[i + 1] == '\\') {
                    // if (backslash backslash (escape backslash, remove one backslash.
                    z[j++] = '\\';
                } else {
                    // else don't remove anything.
                    z[j++] = z[i];
                    z[j++] = z[i + 1];
                }
                i++;
            } else {
                z[j++] = z[i];
            }
        }
        z[j] = '\0';
        z.resize(j);
    }
    return z;
}

#pragma mark - Bytes

Bytes::Bytes()
    : Bytes(nogdb::PropertyType::UNDEFINED)
{
}

Bytes::Bytes(const unsigned char* data, size_t len, PropertyType type_)
    : nogdb::Bytes(data, len)
    , t(type_)
{
}

Bytes::Bytes(nogdb::Bytes&& bytes_, PropertyType type_)
    : nogdb::Bytes(move(bytes_))
    , t(type_)
{
}

Bytes::Bytes(PropertyType type_)
    : nogdb::Bytes()
    , t(type_)
{
}

Bytes::Bytes(ResultSet&& res)
    : nogdb::Bytes(res.descriptorsToString())
    , r(make_shared<ResultSet>(move(res)))
{
}

bool Bytes::operator<(const Bytes& other) const
{
    if (this->size() != other.size()) {
        return this->size() < other.size();
    } else {
        return memcmp(this->getRaw(), other.getRaw(), this->size());
    }
}

#pragma mark - Record

Record::Record(nogdb::Record&& rec)
{
    for (auto p : rec.getAll()) {
        properties.insert(move(p));
    }
    for (auto p : rec.getBasicInfo()) {
        properties.insert(move(p));
    }
}

Record& Record::set(const string& propName, const Bytes& value)
{
    return this->set(propName, Bytes(value));
}

Record& Record::set(const string& propName, Bytes&& value)
{
    if (properties.find(propName) == properties.end()) {
        properties[propName] = move(value);
    } else {
        int num = 2;
        string name = propName + ::to_string(num);
        while (properties.find(name) != properties.end()) {
            name = propName + ::to_string(++num);
        }
        properties[name] = move(value);
    }
    return *this;
}

const map<string, Bytes>& Record::getAll() const
{
    return this->properties;
}

Bytes Record::get(const string& propName) const
{
    try {
        return this->properties.at(propName);
    } catch (...) {
        return Bytes();
    }
}

bool Record::empty() const
{
    if (!this->properties.empty()) {
        for (auto p : this->properties) {
            if (!p.second.empty()) {
                return false;
            }
        }
    }
    return true;
}

nogdb::Record Record::toBaseRecord() const
{
    std::map<string, nogdb::Bytes> baseProperty;
    for (auto& p : this->properties) {
        baseProperty[p.first] = move(p.second);
    }
    return nogdb::Record(baseProperty);
}

#pragma mark - ResultSet

ResultSet::ResultSet()
    : vector<Result>()
{
}

ResultSet::ResultSet(nogdb::ResultSet&& res)
    : vector<Result>()
{
    this->insert(this->begin(),
        make_move_iterator(res.begin()),
        make_move_iterator(res.end()));
}

ResultSet::ResultSet(nogdb::ResultSetCursor& res, int skip, int limit)
    : vector<Result>()
{
    if (skip < 0) {
        skip = 0;
    }
    if (res.to(skip)) {
        if (limit >= 0) {
            while (limit-- > 0) {
                this->push_back(nogdb::Result(*res));
                if (!res.next()) {
                    break;
                }
            }
        } else {
            do {
                this->push_back(nogdb::Result(*res));
            } while (res.next());
        }
    }
}

string ResultSet::descriptorsToString() const
{
    stringstream buff;
    buff << this->size();
    for (const Result& r : *this) {
        buff << "," << r.descriptor.rid;
    }
    return buff.str();
}

ResultSet& ResultSet::limit(int skip, int limit)
{
    if (skip > 0) {
        if ((unsigned long)skip < this->size()) {
            this->erase(this->begin(), this->begin() + skip);
        } else {
            this->clear();
        }
    }
    if (limit >= 0 && (unsigned)limit < this->size()) {
        this->resize(limit);
    }
    return *this;
}

#pragma mark - Function

Function::Function(const string& name_, vector<Projection>&& args_)
    : name(name_)
    , args(move(args_))
{
    static const auto nameMap = map<string, Id, StringCaseCompare>(
        {
            { "COUNT", Id::COUNT },
            { "MIN", Id::MIN },
            { "MAX", Id::MAX },
            { "IN", Id::IN },
            { "INE", Id::IN_E },
            { "INV", Id::IN_V },
            { "OUT", Id::OUT },
            { "OUTE", Id::OUT_E },
            { "OUTV", Id::OUT_V },
            { "BOTH", Id::BOTH },
            { "BOTHE", Id::BOTH_E },
            { "BOTHV", Id::BOTH_V },
            { "EXPAND", Id::EXPAND },
        },
        stringcasecmp);

    auto id_ = nameMap.find(name);
    if (id_ != nameMap.end()) {
        this->id = id_->second;
    } else {
        this->id = Id::UNDEFINED;
    }
}

Bytes Function::execute(Transaction& txn, const Result& input) const
{
    require(this->isAggregateResult() == false);
    require(this->isExpand() == false);

    function<Bytes(Transaction&, const Result&, const vector<Projection>&)> func;
    switch (this->id) {
    case Id::IN:
        func = walkIn;
        break;
    case Id::IN_E:
        func = walkInEdge;
        break;
    case Id::IN_V:
        func = walkInVertex;
        break;
    case Id::OUT:
        func = walkOut;
        break;
    case Id::OUT_E:
        func = walkOutEdge;
        break;
    case Id::OUT_V:
        func = walkOutVertex;
        break;
    case Id::BOTH:
        func = walkBoth;
        break;
    case Id::BOTH_E:
        func = walkBothEdge;
        break;
    case Id::BOTH_V:
        func = walkBothVertex;
        break;
    default:
        throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_FUNCTION_NAME);
    }
    return func(txn, input, this->args);
}

Bytes Function::executeAggregateResult(const ResultSet& input) const
{
    function<Bytes(const ResultSet&, const vector<Projection>&)> func;
    switch (this->id) {
    case Id::COUNT:
        func = count;
        break;
    case Id::MAX:
        //            func = max; break;
    case Id::MIN:
        //            func = min; break;
    default:
        throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_FUNCTION_NAME);
    }
    return func(input, this->args);
}

Bytes Function::executeExpand(Transaction& txn, ResultSet& input) const
{
    return expand(txn, input, args);
}

bool Function::isAggregateResult() const
{
    switch (this->id) {
    case Id::COUNT:
    case Id::MIN:
    case Id::MAX:
        return true;
    default:
        return false;
    }
}

bool Function::isWalkResult() const
{
    switch (this->id) {
    case Id::IN:
    case Id::IN_E:
    case Id::IN_V:
    case Id::OUT:
    case Id::OUT_E:
    case Id::OUT_V:
    case Id::BOTH:
    case Id::BOTH_E:
        return true;
    default:
        return false;
    }
}

bool Function::isExpand() const
{
    return this->id == Id::EXPAND;
}

#pragma mark-- private

Bytes Function::count(const ResultSet& input, const vector<Projection>& args)
{
    if (args.empty()) {
        return Bytes(input.size(), PropertyType(nogdb::PropertyType::UNSIGNED_BIGINT));
    } else if (args.size() == 1 && args[0].type == ProjectionType::PROPERTY) {
        string propName = args[0].get<string>();
        size_t result = 0;
        for (const auto& row : input) {
            if (!row.record.get(propName).empty()) {
                result++;
            }
        }
        return Bytes(result, PropertyType(nogdb::PropertyType::UNSIGNED_BIGINT));
    } else {
        throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_FUNCTION_ARGS);
    }
}

//Bytes Function::min(Transaction &txn, const ResultSet &input, const vector<Projection> &args) {
//}
//
//Bytes Function::max(Transaction &txn, const ResultSet &input, const vector<Projection> &args) {
//}

Bytes Function::walkIn(nogdb::Transaction& txn, const Result& input, const vector<Projection>& args)
{
    ResultSet results {};
    Bytes rTmp = walkInEdge(txn, input, args);
    for (const Result& input : rTmp.results()) {
        results.push_back(txn.fetchSrc(input.descriptor));
    }
    return Bytes(move(results));
}

Bytes Function::walkInEdge(Transaction& txn, const Result& input, const vector<Projection>& args)
{
    return Bytes(txn.findInEdge(input.descriptor).where(GraphFilter {}.only(argsToClassFilter(args))).get());
}

Bytes Function::walkInVertex(Transaction& txn, const Result& input, const vector<Projection>& args)
{
    if (args.size() == 0) {
        return Bytes(ResultSet { txn.fetchDst(input.descriptor) });
    } else {
        throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_FUNCTION_ARGS);
    }
}

Bytes Function::walkOut(nogdb::Transaction& txn, const Result& input, const vector<Projection>& args)
{
    ResultSet results {};
    Bytes rTmp = walkOutEdge(txn, input, args);
    for (const Result& input : rTmp.results()) {
        results.push_back(txn.fetchDst(input.descriptor));
    }
    return Bytes(move(results));
}

Bytes Function::walkOutEdge(Transaction& txn, const Result& input, const vector<Projection>& args)
{
    return Bytes(txn.findOutEdge(input.descriptor).where(GraphFilter {}.only(argsToClassFilter(args))).get());
}

Bytes Function::walkOutVertex(Transaction& txn, const Result& input, const vector<Projection>& args)
{
    if (args.size() == 0) {
        return Bytes(ResultSet { txn.fetchSrc(input.descriptor) });
    } else {
        throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_FUNCTION_ARGS);
    }
}

Bytes Function::walkBoth(Transaction& txn, const Result& input, const vector<Projection>& args)
{
    ResultSet results {};
    Bytes rTmp = walkInEdge(txn, input, args);
    for (const Result& input : rTmp.results()) {
        results.push_back(txn.fetchSrc(input.descriptor));
    }
    rTmp = walkOutEdge(txn, input, args);
    for (const Result& input : rTmp.results()) {
        results.push_back(txn.fetchDst(input.descriptor));
    }
    return Bytes(move(results));
}

Bytes Function::walkBothEdge(Transaction& txn, const Result& input, const vector<Projection>& args)
{
    return Bytes(txn.findEdge(input.descriptor).where(GraphFilter {}.only(argsToClassFilter(args))).get());
}

Bytes Function::walkBothVertex(Transaction& txn, const Result& input, const vector<Projection>& args)
{
    if (args.size() == 0) {
        return Bytes(txn.fetchSrcDst(input.descriptor));
    } else {
        throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_FUNCTION_ARGS);
    }
}

Bytes Function::expand(Transaction& txn, ResultSet& input, const vector<Projection>& args)
{
    if (args.size() != 1) {
        throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_FUNCTION_ARGS);
    }

    ResultSet results {};
    const Projection& arg = args[0];
    for (const Result& in : input) {
        Bytes out = Context::getProjectionItem(txn, in, arg, {});
        if (out.isResults()) {
            results.insert(results.end(), make_move_iterator(out.results().begin()), make_move_iterator(out.results().end()));
        } else if (out.empty()) {
            // no-op.
        } else {
            throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_FUNCTION_ARGS);
        }
    }

    input = move(results);
    return Bytes();
}

vector<string> Function::argsToClassFilter(const vector<Projection>& args)
{
    vector<string> filter {};
    for (const Projection& arg : args) {
        if (arg.type != ProjectionType::PROPERTY) {
            throw NOGDB_SQL_ERROR(NOGDB_SQL_INVALID_FUNCTION_ARGS);
        }
        filter.emplace_back(arg.get<string>());
    }
    return filter;
}

#pragma mark - Parser process

// define token space and illegal follow sqlite3
#define TK_SPACE 0xFFFE
#define TK_ILLEGAL 0xFFFF

/* Character classes for tokenizing
 **
 ** In the sqlite3GetToken() function, a switch() on aiClass[c] is implemented
 ** using a lookup table, whereas a switch() directly on c uses a binary search.
 ** The lookup table is much faster.  To maximize speed, and to ensure that
 ** a lookup table is used, all of the classes need to be small integers and
 ** all of them need to be used within the switch.
 */
#define CC_X 0 /* The letter 'x', or start of BLOB literal */
#define CC_KYWD 1 /* Alphabetics or '_'.  Usable in a keyword */
#define CC_ID 2 /* unicode characters usable in IDs */
#define CC_DIGIT 3 /* Digits */
#define CC_DOLLAR 4 /* '$' */
#define CC_VARALPHA 5 /* '@', '#', ':'.  Alphabetic SQL variables */
#define CC_VARNUM 6 /* '?'.  Numeric SQL variables */
#define CC_SPACE 7 /* Space characters */
#define CC_QUOTE 8 /* '"', '\'', or '`'.  String literals, quoted ids */
//#define CC_QUOTE2     9    /* '['.   [...] style quoted ids */
#define CC_BRACKET 9 /* '[', ']' */
#define CC_PIPE 10 /* '|'.   Bitwise OR or concatenate */
#define CC_MINUS 11 /* '-'.  Minus or SQL-style comment */
#define CC_LT 12 /* '<'.  Part of < or <= or <> */
#define CC_GT 13 /* '>'.  Part of > or >= */
#define CC_EQ 14 /* '='.  Part of = or == */
#define CC_BANG 15 /* '!'.  Part of != */
#define CC_SLASH 16 /* '/'.  / or c-style comment */
#define CC_LP 17 /* '(' */
#define CC_RP 18 /* ')' */
#define CC_SEMI 19 /* ';' */
#define CC_PLUS 20 /* '+' */
#define CC_STAR 21 /* '*' */
#define CC_PERCENT 22 /* '%' */
#define CC_COMMA 23 /* ',' */
#define CC_AND 24 /* '&' */
#define CC_TILDA 25 /* '~' */
#define CC_DOT 26 /* '.' */
#define CC_ILLEGAL 27 /* Illegal character */

static const unsigned char aiClass[] = {
    /*         x0  x1  x2  x3  x4  x5  x6  x7  x8  x9  xa  xb  xc  xd  xe  xf */
    /* 0x */ 27, 27, 27, 27, 27, 27, 27, 27, 27, 7, 7, 27, 7, 7, 27, 27,
    /* 1x */ 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    /* 2x */ 7, 15, 8, 5, 4, 22, 24, 8, 17, 18, 21, 20, 23, 11, 26, 16,
    /* 3x */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 19, 12, 14, 13, 6,
    /* 4x */ 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 5x */ 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 9, 27, 9, 27, 1,
    /* 6x */ 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 7x */ 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 27, 10, 27, 25, 27,
    /* 8x */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* 9x */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* Ax */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* Bx */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* Cx */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* Dx */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* Ex */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* Fx */ 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

/*
 * If X is a character that can be used in an identifier then
 * IdChar(X) will be true.  Otherwise it is false.
 *
 * For ASCII, any character with the high-order bit set is
 * allowed in an identifier.  For 7-bit characters,
 * isalnum(X) must be true.
 */
#define IdChar(C) (((unsigned char)C) & 0x80 || isalnum(C))

static int keywordCode(const char* z, int n, int* pType)
{
    static const auto kw = map<string, int, StringCaseCompare>(
        {
            { "ALTER", TK_ALTER },
            { "AND", TK_AND },
            { "AS", TK_AS },
            { "ASC", TK_ASC },
            { "BEGIN", TK_BEGIN },
            { "BY", TK_BY },
            { "CASE", TK_CASE },
            { "CLASS", TK_CLASS },
            { "CONTAIN", TK_CONTAIN },
            { "CREATE", TK_CREATE },
            { "DELETE", TK_DELETE },
            { "DESC", TK_DESC },
            { "DROP", TK_DROP },
            { "EDGE", TK_EDGE },
            { "END", TK_END },
            { "EXISTS", TK_EXISTS },
            { "EXTENDS", TK_EXTENDS },
            { "FROM", TK_FROM },
            { "GROUP", TK_GROUP },
            { "IF", TK_IF },
            { "INDEX", TK_INDEX },
            { "IS", TK_IS },
            { "LIKE", TK_LIKE },
            { "LIMIT", TK_LIMIT },
            { "MAXDEPTH", TK_MAXDEPTH },
            { "MINDEPTH", TK_MINDEPTH },
            { "NOT", TK_NOT },
            { "NULL", TK_NULL },
            { "OR", TK_OR },
            { "ORDER", TK_ORDER },
            { "PROPERTY", TK_PROPERTY },
            { "SELECT", TK_SELECT },
            { "SET", TK_SET },
            { "SKIP", TK_SKIP },
            { "STRATEGY", TK_STRATEGY },
            { "TO", TK_TO },
            { "TRAVERSE", TK_TRAVERSE },
            { "UPDATE", TK_UPDATE },
            { "VERTEX", TK_VERTEX },
            { "WHERE", TK_WHERE },
            { "WITH", TK_WITH },
        },
        stringcasecmp);
    try {
        *pType = kw.at(string(z, n));
    } catch (...) {
        *pType = TK_IDENTITY;
    }
    return n;
}

/*
 * Return the length (in bytes) of the token that begins at z[0].
 * Store the token type in *tokenType before returning.
 */
static int getTokenID(const unsigned char* z, int* tokenType)
{
    int i;
    switch (aiClass[*z]) { /* Switch on the character-class of the first byte
                              * of the token. See the comment on the CC_ defines
                              * above. */
    case CC_SPACE:
        for (i = 1; aiClass[z[i]] == CC_SPACE; i++) {
        }
        *tokenType = TK_SPACE;
        return i;
    case CC_MINUS:
        if (aiClass[z[1]] != CC_DIGIT) {
            *tokenType = TK_ILLEGAL;
            return -1;
        }
        i = 1 + getTokenID(z + 1, tokenType);
        if (*tokenType == TK_UNSIGNED) {
            *tokenType = TK_SIGNED;
        }
        return i;
    case CC_LP:
        *tokenType = TK_LP;
        return 1;
    case CC_RP:
        *tokenType = TK_RP;
        return 1;
    case CC_SEMI:
        *tokenType = TK_SEMI;
        return 1;
    case CC_PLUS:
        return -1;
    case CC_STAR:
        *tokenType = TK_STAR;
        return 1;
    case CC_SLASH:
        return -1;
    case CC_PERCENT:
        return -1;
    case CC_EQ:
        *tokenType = TK_EQ;
        return 1 + (z[1] == '=');
    case CC_LT:
        if (z[1] == '=') {
            *tokenType = TK_LE;
            return 2;
        } else if (z[1] == '>') {
            *tokenType = TK_NE;
            return 2;
        } else {
            *tokenType = TK_LT;
            return 1;
        }
    case CC_GT:
        if (z[1] == '=') {
            *tokenType = TK_GE;
            return 2;
        } else {
            *tokenType = TK_GT;
            return 1;
        }
    case CC_BANG:
        if (z[1] == '=') {
            *tokenType = TK_NE;
            return 2;
        } else {
            *tokenType = TK_ILLEGAL;
            return 1;
        }
    case CC_PIPE:
        return -1;
    case CC_COMMA:
        *tokenType = TK_COMMA;
        return 1;
    case CC_AND:
        return -1;
    case CC_TILDA:
        return -1;
    case CC_QUOTE: {
        int delim = z[0];
        for (i = 1; z[i] != '\0'; i++) {
            if (z[i] == '\\') {
                i++;
            } else if (z[i] == delim) {
                break;
            }
        }
        if (z[i] == '\'' || z[i] == '"') {
            *tokenType = TK_STRING;
            return i + 1;
        } else if (z[i] != '\0') {
            *tokenType = TK_IDENTITY;
            return i + 1;
        } else {
            *tokenType = TK_ILLEGAL;
            return i;
        }
    }
    case CC_DOT:
        *tokenType = TK_DOT;
        return 1;
    case CC_DIGIT:
        *tokenType = TK_UNSIGNED;
        if (z[0] == '0' && (z[1] == 'x' || z[1] == 'X') && isxdigit(z[2])) {
            // 0x12347890abcdef
            for (i = 3; isxdigit(z[i]); i++) {
            }
            return i;
        }
        for (i = 0; isdigit(z[i]); i++) {
        }
        if (z[i] == '.') {
            // 12.34
            i++;
            while (isdigit(z[i])) {
                i++;
            }
            *tokenType = TK_FLOAT;
        }
        if ((z[i] == 'e' || z[i] == 'E')
            && (isdigit(z[i + 1])
                || ((z[i + 1] == '+' || z[i + 1] == '-') && isdigit(z[i + 2])))) {
            // 12e34
            i += 2;
            while (isdigit(z[i])) {
                i++;
            }
            *tokenType = TK_FLOAT;
        }
        while (IdChar(z[i])) {
            *tokenType = TK_ILLEGAL;
            i++;
        }
        return i;
        //        case CC_QUOTE2:
        //            return -1;
    case CC_BRACKET:
        if (z[0] == '[') {
            *tokenType = TK_LB;
        } else /* if (z[0] == ']') */ {
            *tokenType = TK_RB;
        }
        return 1;
    case CC_VARNUM:
        return -1;
    case CC_DOLLAR:
        return -1;
    case CC_VARALPHA:
        if (z[0] == '#') {
            *tokenType = TK_SHARP;
        } else if (z[0] == ':') {
            *tokenType = TK_COLON;
        } else if (z[0] == '@') {
            *tokenType = TK_AT;
        } else {
            *tokenType = TK_ILLEGAL;
            return -1;
        }
        return 1;
    case CC_KYWD:
        for (i = 1; aiClass[z[i]] <= CC_KYWD; i++) {
        }
        if (IdChar(z[i])) {
            /* This token started out using characters that can appear in keywords,
         * but z[i] is a character not allowed within keywords, so this must
         * be an identifier instead */
            i++;
            break;
        }
        *tokenType = TK_IDENTITY;
        return keywordCode((char*)z, i, tokenType);
    case CC_X:
        if (z[1] == '\'') {
            *tokenType = TK_BLOB;
            for (i = 2; isxdigit(z[i]); i++) {
            }
            if (z[i] != '\'' || i % 2) {
                *tokenType = TK_ILLEGAL;
                while (z[i] && z[i] != '\'') {
                    i++;
                }
            }
            if (z[i]) {
                i++;
            }
            return i;
        }
        /* If it is not a BLOB literal, then it must be an ID, since no
       * SQL keywords start with the letter 'x'.  Fall through */
    case CC_ID:
        i = 1;
        break;
    default:
        return -1;
    }
    while (IdChar(z[i])) {
        i++;
    }
    *tokenType = TK_IDENTITY;
    return i;
}

const nogdb::SQL::Result nogdb::SQL::execute(Transaction& txn, const std::string& sql)
{
    auto parser = sql_parser::Context::create(txn);

    const char* zSql = sql.c_str();
    int n = 0; /* Length of the next token token */
    int tokenType; /* type of the next token */
    int lastTokenParsed = -1; /* type of the previous token */

    while (1) {
        if (zSql[0] != '\0') {
            n = getTokenID((unsigned char*)zSql, &tokenType);
        } else {
            /* Upon reaching the end of input, call the parser two more times
       * with tokens TK_SEMI and 0, in that order. */
            if (lastTokenParsed == TK_SEMI) {
                tokenType = 0;
            } else if (lastTokenParsed == 0) {
                break;
            } else {
                tokenType = TK_SEMI;
            }
            zSql -= n;
        }
        if (tokenType >= TK_SPACE || n == -1) {
            if (tokenType == TK_ILLEGAL || n == -1) {
                throw NOGDB_SQL_ERROR(NOGDB_SQL_UNRECOGNIZED_TOKEN);
            }
            zSql += n;
        } else {
            parser->parse(tokenType, { zSql, n, tokenType });
            lastTokenParsed = tokenType;
            zSql += n;
            if (parser->rc != sql_parser::Context::SQL_OK) {
                throw parser->result.get<Error>();
            }
        }
    }

    return parser->result;
}
