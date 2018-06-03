/*
 *
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

#ifndef __NOGDB_TYPES_H_INCLUDED_
#define __NOGDB_TYPES_H_INCLUDED_

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <ostream>

//******************************************************************
//*  Forward declarations of NogDB and boost internal classes.     *
//******************************************************************

namespace boost {
    class shared_mutex;
}

namespace nogdb {
    struct Datastore;
    struct Graph;
    struct Validate;
    struct Algorithm;
    struct Compare;
    struct TxnStat;
    struct Generic;
    struct Parser;
    struct Schema;
    struct Class;
    struct Property;
    struct Db;
    struct Vertex;
    struct Edge;
    struct Traverse;

    class EnvHandlerPtr;

    class BaseTxn;

    class Condition;

    class MultiCondition;

    class PathFilter;

    class RWSpinLock;

    namespace sql_parser {
        class Record;
    }
}

//*************************************************************
//*  NogDB type definitions.                                  *
//*************************************************************
namespace nogdb {

    enum class PropertyType {
        TINYINT = 'i',
        UNSIGNED_TINYINT = 'I',
        SMALLINT = 's',
        UNSIGNED_SMALLINT = 'S',
        INTEGER = 'd',
        UNSIGNED_INTEGER = 'D',
        BIGINT = 'l',
        UNSIGNED_BIGINT = 'L',
        TEXT = 't',
        REAL = 'f',
        BLOB = 'b',
        UNDEFINED = 'n'
    };

    enum class ClassType {
        VERTEX = 'v',
        EDGE = 'e',
        UNDEFINED = 'n'
    };

    typedef uint16_t ClassId;
    typedef uint16_t PropertyId;
    typedef uint32_t PositionId;
    typedef std::pair<ClassId, PositionId> RecordId;
    typedef uint64_t TxnId;
    typedef uint32_t IndexId;
    typedef std::map<std::string, PropertyType> PropertyMapType;

    struct DBInfo {
        DBInfo() = default;

        std::string dbPath{""};        // a path to the database folder.
        unsigned int maxDB{0};         // a maximum number of databases that can be handled.
        unsigned long maxDBSize{0};    // the largest size of a database.
        PropertyId maxPropertyId{0};   // the largest property number(id) in the entire database.
        PropertyId numProperty{0};     // a number of properties in the database.
        ClassId maxClassId{0};         // the largest class number(id) in the entire database.
        ClassId numClass{0};           // a number of classes in the database.
        IndexId maxIndexId{0};         // the largest index number(id) in the entire database.
        IndexId numIndex{0};           // a number of indexes in the database.
    };

    class Bytes {
    public:
        friend class Record;

        Bytes() = default;

        Bytes(const unsigned char *data, size_t len);

        template<typename T>
        explicit Bytes(T data)
                : Bytes{static_cast<const unsigned char *>((void *) &data), sizeof(data)} {}

        explicit Bytes(const unsigned char *data);

        explicit Bytes(const char *data);

        explicit Bytes(const std::string &data);

        ~Bytes() noexcept;

        Bytes(const Bytes &binaryObject);

        Bytes &operator=(const Bytes &binaryObject);

        Bytes(Bytes &&binaryObject) noexcept;

        Bytes &operator=(Bytes &&binaryObject) noexcept;

        uint8_t toTinyIntU() const;

        int8_t toTinyInt() const;

        uint16_t toSmallIntU() const;

        int16_t toSmallInt() const;

        uint32_t toIntU() const;

        int32_t toInt() const;

        uint64_t toBigIntU() const;

        int64_t toBigInt() const;

        double toReal() const;

        std::string toText() const;

        explicit operator unsigned char *() const;

        unsigned char *getRaw() const;

        size_t size() const;

        bool empty() const;

        template<typename T>
        void convertTo(T &object) {
            memcpy(&object, value_, size_);
        }

    private:
        unsigned char *value_{nullptr};
        size_t size_{0};

        template<typename T>
        T convert() const {
            T result = 0;
            memcpy(static_cast<void *>(&result), static_cast<const void *>(value_), sizeof(T));
            return result;
        }
    };

    class Record {
    public:

        using RecordPropertyType = std::map<std::string, Bytes>;

        Record() = default;

        template<typename T>
        Record &set(const std::string &propName, const T &value) {
            if (!propName.empty() && !isBasicInfo(propName)) {
                properties[propName] = Bytes{static_cast<const unsigned char *>((void *) &value), sizeof(T)};
            }
            return *this;
        }

        Record &set(const std::string &propName, const unsigned char *value);

        Record &set(const std::string &propName, const char *value);

        Record &set(const std::string &propName, const std::string &value);

        Record &set(const std::string &propName, const nogdb::Bytes &b);

        template<typename T>
        Record &setIfNotExists(const std::string &propName, const T &value) {
            if (properties.find(propName) == properties.cend()) {
                set(propName, value);
            }
            return *this;
        }

        const RecordPropertyType &getAll() const;

        const RecordPropertyType &getBasicInfo() const;

        std::vector<std::string> getProperties() const;

        Bytes get(const std::string &propName) const;

        uint8_t getTinyIntU(const std::string &propName) const;

        int8_t getTinyInt(const std::string &propName) const;

        uint16_t getSmallIntU(const std::string &propName) const;

        int16_t getSmallInt(const std::string &propName) const;

        uint32_t getIntU(const std::string &propName) const;

        int32_t getInt(const std::string &propName) const;

        uint64_t getBigIntU(const std::string &propName) const;

        int64_t getBigInt(const std::string &propName) const;

        double getReal(const std::string &propName) const;

        std::string getText(const std::string &propName) const;

        std::string getClassName() const;

        RecordId getRecordId() const;

        uint32_t getDepth() const;

        uint64_t getVersion() const;

        void unset(const std::string &className);

        size_t size() const;

        bool empty() const;

        void clear();

    private:

        friend struct Parser;
        friend struct Generic;
        friend class sql_parser::Record;

        friend struct Vertex;
        friend struct Edge;

        Record(RecordPropertyType properties);

        Record(RecordPropertyType properties, RecordPropertyType basicProperties)
                : properties(std::move(properties)), basicProperties(std::move(basicProperties)) {}

        inline bool isBasicInfo(const std::string &str) const { return str.at(0) == '@'; }

        RecordPropertyType properties{};
        mutable RecordPropertyType basicProperties{};

        template<typename T>
        const Record &setBasicInfo(const std::string &propName, const T &value) const {
            if (!propName.empty() && isBasicInfo(propName)) {
                basicProperties[propName] = Bytes{static_cast<const unsigned char *>((void *) &value), sizeof(T)};
            }
            return *this;
        };

        const Record &setBasicInfo(const std::string &propName, const unsigned char *value) const;

        const Record &setBasicInfo(const std::string &propName, const char *value) const;

        const Record &setBasicInfo(const std::string &propName, const std::string &value) const;

        const Record &setBasicInfo(const std::string &propName, const Bytes& b) const;

        template<typename T>
        const Record &setBasicInfoIfNotExists(const std::string &propName, const T &value) const {
            if (basicProperties.find(propName) == basicProperties.cend()) {
                setBasicInfo(propName, value);
            }
            return *this;
        };

    };

    struct RecordDescriptor {
        RecordDescriptor() = default;

        RecordDescriptor(ClassId classId, PositionId posId)
                : rid{classId, posId} {}

        RecordDescriptor(const RecordId &rid_)
                : rid{rid_} {}

        ~RecordDescriptor() noexcept = default;

        RecordId rid{0, 0};

    private:
        friend struct Generic;
        friend struct Algorithm;
        unsigned int depth{0};
    };

    typedef std::map<IndexId, std::pair<ClassId, bool>> IndexInfo;

    struct PropertyDescriptor {
        PropertyDescriptor() = default;

        PropertyDescriptor(PropertyId id_, PropertyType type_)
                : id{id_}, type{type_} {}

        PropertyDescriptor(PropertyId id_, PropertyType type_, const IndexInfo &indexInfo_)
                : id{id_}, type{type_}, indexInfo{indexInfo_} {}

        ~PropertyDescriptor() noexcept = default;

        PropertyId id{0};
        PropertyType type{PropertyType::UNDEFINED};
        IndexInfo indexInfo{};
    };

    typedef std::map<std::string, PropertyDescriptor> ClassProperty;

    struct ClassDescriptor {
        ClassDescriptor() = default;

        ClassDescriptor(ClassId id_,
                        const std::string &name_,
                        ClassType type_,
                        ClassProperty properties_)
                : id{id_}, name{name_}, type{type_}, properties{properties_} {}

        ClassDescriptor(ClassId id_,
                        const std::string &name_,
                        ClassType type_,
                        ClassProperty properties_,
                        const std::string &super_,
                        const std::vector<std::string> &sub_)
                : id{id_}, name{name_}, type{type_}, properties{properties_}, super{super_}, sub{sub_} {}

        ~ClassDescriptor() noexcept = default;

        ClassId id{0};
        std::string name{""};
        ClassType type{ClassType::UNDEFINED};
        ClassProperty properties{};
        std::string super{""};
        std::vector<std::string> sub{};
    };

    struct Result {
        Result() = default;

        Result(const RecordDescriptor &recordDescriptor_, const Record &record_)
                : descriptor{recordDescriptor_}, record{record_} {}

        RecordDescriptor descriptor{};
        Record record{};
    };

    typedef std::vector<Result> ResultSet;

    class Txn;

    struct ClassPropertyInfo;

    class ResultSetCursor {
    public:
        friend struct Generic;
        friend struct Vertex;
        friend struct Edge;
        friend struct Traverse;

        ResultSetCursor(Txn &txn_);

        ~ResultSetCursor() noexcept;

        ResultSetCursor(const ResultSetCursor &rc);

        ResultSetCursor &operator=(const ResultSetCursor &rc);

        ResultSetCursor(ResultSetCursor &&rc) noexcept;

        ResultSetCursor &operator=(ResultSetCursor &&rc) noexcept;

        bool hasNext() const;

        bool hasPrevious() const;

        bool hasAt(unsigned long index) const;

        bool next();

        bool previous();

        void first();

        void last();

        bool to(unsigned long index);

        bool empty() const;

        size_t size() const;

        size_t count() const;

        const Result &operator*() const;

        const Result *operator->() const;

    private:
        typedef std::unordered_map<ClassId, ClassPropertyInfo> ClassPropertyCache;

        Txn &txn;
        std::unique_ptr<ClassPropertyCache> classPropertyInfos;
        std::vector<RecordDescriptor> metadata{};
        long long currentIndex;
        Result result;

        const ClassPropertyInfo resolveClassPropertyInfo(ClassId classId);
    };

}

inline std::ostream &operator<<(std::ostream &os, const nogdb::RecordId &rid) {
    os << "#" << std::to_string(rid.first) << ":" << std::to_string(rid.second);
    return os;
}

inline std::ostream &operator<<(std::ostream &os, nogdb::PropertyType type) {
    switch (type) {
        case nogdb::PropertyType::TINYINT:
            os << "tinyint";
            break;
        case nogdb::PropertyType::UNSIGNED_TINYINT:
            os << "unsigned tinyint";
            break;
        case nogdb::PropertyType::SMALLINT:
            os << "smallint";
            break;
        case nogdb::PropertyType::UNSIGNED_SMALLINT:
            os << "unsigned smallint";
            break;
        case nogdb::PropertyType::INTEGER:
            os << "integer";
            break;
        case nogdb::PropertyType::UNSIGNED_INTEGER:
            os << "unsigned integer";
            break;
        case nogdb::PropertyType::BIGINT:
            os << "bigint";
            break;
        case nogdb::PropertyType::UNSIGNED_BIGINT:
            os << "unsigned bigint";
            break;
        case nogdb::PropertyType::TEXT:
            os << "text";
            break;
        case nogdb::PropertyType::REAL:
            os << "real";
            break;
        case nogdb::PropertyType::BLOB:
            os << "blob";
            break;
        default:
            os << "undefined";
            break;
    }
    return os;
}

inline std::ostream &operator<<(std::ostream &os, nogdb::ClassType type) {
    switch (type) {
        case nogdb::ClassType::EDGE:
            os << "edge";
            break;
        case nogdb::ClassType::VERTEX:
            os << "vertex";
            break;
        default:
            os << "";
            break;
    }
    return os;
}

inline bool operator<(const nogdb::RecordId &lhs, const nogdb::RecordId &rhs) {
    return (lhs.first == rhs.first) ? lhs.second < rhs.second : lhs.first < rhs.first;
}

inline bool operator==(const nogdb::RecordId &lhs, const nogdb::RecordId &rhs) {
    return (lhs.first == rhs.first) && (lhs.second == rhs.second);
}

inline bool operator!=(const nogdb::RecordId &lhs, const nogdb::RecordId &rhs) {
    return !operator==(lhs, rhs);
}

inline bool operator<(const nogdb::RecordDescriptor &lhs, const nogdb::RecordDescriptor &rhs) {
    return lhs.rid < rhs.rid;
}

inline bool operator==(const nogdb::RecordDescriptor &lhs, const nogdb::RecordDescriptor &rhs) {
    return lhs.rid == rhs.rid;
}

inline bool operator!=(const nogdb::RecordDescriptor &lhs, const nogdb::RecordDescriptor &rhs) {
    return !operator==(lhs, rhs);
}

#endif
