/*
 *
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

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace nogdb {

namespace storage_engine {
    class LMDBEnv;

    class LMDBTxn;
}

namespace parser {
    class RecordParser;
}

namespace compare {
    class RecordCompare;
}

namespace adapter {
    namespace metadata {
        class DBInfoAccess;
    }
    namespace schema {
        class ClassAccess;

        class PropertyAccess;

        class IndexAccess;
    }
}

namespace schema {
    struct SchemaUtils;
}

namespace datarecord {
    struct DataRecordUtils;
}

namespace relation {
    class GraphUtils;
}

namespace index {
    struct IndexUtils;
}

namespace validate {
    class Validator;
}

namespace algorithm {
    class GraphTraversal;
}

namespace sql_parser {
    class Record;
}

class Condition;

class MultiCondition;

class GraphFilter;

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

enum class TxnMode {
    READ_ONLY,
    READ_WRITE
};

typedef uint16_t ClassId;
typedef uint16_t PropertyId;
typedef uint32_t PositionId;
typedef uint32_t ClusterId;
typedef std::pair<ClassId, PositionId> RecordId;
typedef uint32_t IndexId;
typedef uint64_t VersionId;
typedef std::map<std::string, PropertyType> PropertyMapType;

struct DBInfo {
    std::string dbPath;
    ClassId maxClassId;
    ClassId numClass;
    PropertyId maxPropertyId;
    PropertyId numProperty;
    IndexId maxIndexId;
    IndexId numIndex;
};

class Transaction;

class Bytes {
public:
    friend class Record;

    Bytes() = default;

    Bytes(const unsigned char* data, size_t len, bool copy = true);

    template <typename T>
    explicit Bytes(T data)
        : Bytes { static_cast<const unsigned char*>((void*)&data), sizeof(data) }
    {
    }

    explicit Bytes(const unsigned char* data);

    explicit Bytes(const char* data);

    explicit Bytes(const std::string& data);

    ~Bytes() noexcept;

    Bytes(const Bytes& binaryObject);

    Bytes& operator=(const Bytes& binaryObject);

    Bytes(Bytes&& binaryObject) noexcept;

    Bytes& operator=(Bytes&& binaryObject) noexcept;

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

    explicit operator unsigned char*() const;

    unsigned char* getRaw() const;

    size_t size() const;

    bool empty() const;

    template <typename T>
    void convertTo(T& object)
    {
        memcpy(&object, _value, _size);
    }

    template <typename T>
    T convert() const
    {
        unsigned char* ptr = getRaw();
        size_t size = this->size();
        return Converter<T>::convert(ptr, size, false);
    }

    template <typename T>
    static Bytes toBytes(const T& data)
    {
        return Converter<T>::toBytes(data);
    }

private:
    unsigned char* _value { nullptr };
    size_t _size { 0 };

    static Bytes merge(const Bytes& bytes1, const Bytes& byte2);

    static Bytes merge(const std::vector<Bytes>& bytes);

    using CollectionSizeType = uint16_t;

    template <typename T>
    class Converter {
    public:
        static const bool special = false;

        static Bytes toBytes(const T& value, bool delimiter = false)
        {
            return Bytes { static_cast<const unsigned char*>((void*)&value), sizeof(T) };
        };

        static T convert(unsigned char*& ptr, size_t& total_size, bool delimiter = false)
        {
            if (delimiter) {
                ptr += sizeof(T);
                total_size -= sizeof(T);
                return *reinterpret_cast<T*>(ptr - sizeof(T));
            } else {
                return *reinterpret_cast<T*>(ptr);
            }
        }
    };

    template <typename T>
    class Converter<const T> : public Converter<T> {
    };

    template <typename T>
    class Converter<T*> {
    public:
        static const bool special = true;

        static T* convert(unsigned char*& ptr, size_t& total_size, bool delimiter = false)
        {
            if (!Converter<T>::special) {
                if (delimiter) {
                    const CollectionSizeType size = Converter<CollectionSizeType>::convert(ptr, total_size, true);
                    ptr += sizeof(CollectionSizeType) + size * sizeof(T);
                    total_size -= sizeof(CollectionSizeType);
                    return reinterpret_cast<T*>(ptr - size * sizeof(T));
                } else {
                    return reinterpret_cast<T*>(ptr);
                }
            } else {
                if (delimiter) {
                    const CollectionSizeType size = Converter<CollectionSizeType>::convert(ptr, total_size, true);
                    auto* result = new T[size];
                    for (size_t i = 0; i < size; ++i) {
                        result[i] = std::move(Converter<T>::convert(ptr, total_size, true));
                    }
                    return result;
                } else {
                    std::vector<T> result;
                    while (total_size > 0) {
                        result.emplace_back(Converter<T>::convert(ptr, total_size, true));
                    }
                    auto* data = new T[result.size()];
                    for (size_t i = 0; i < result.size(); ++i) {
                        data[i] = std::move(result[i]);
                    }
                    return data;
                }
            }
        }
    };

    template <typename T, CollectionSizeType N>
    class Converter<T[N]> {
    public:
        static const bool special = true;

        static Bytes toBytes(const T (&value)[N], bool delimiter = false)
        {
            if (!Converter<T>::special) {
                if (delimiter) {
                    return merge(Bytes(N), toBytes(value, false));
                } else {
                    return Bytes { reinterpret_cast<const unsigned char*>(value),
                        (N - std::is_same<typename std::remove_const<T>::type, char>::value) * sizeof(T) };
                }
            } else {
                std::vector<Bytes> result(N + delimiter);
                if (delimiter) {
                    result[0] = Bytes(N);
                }
                for (CollectionSizeType i = 0; i < N; ++i) {
                    result[i + delimiter] = Converter<T>::toBytes(value[i], true);
                }
                return merge(result);
            }
        };

        static T* convert(unsigned char*& ptr, size_t& total_size, bool delimiter = false)
        {
            return Converter<T*>::convert(ptr, total_size, delimiter);
        }
    };

    template <typename T1, typename T2>
    class Converter<std::pair<T1, T2>> {
    public:
        static const bool special = Converter<T1>::special || Converter<T2>::special;

        static Bytes toBytes(const std::pair<T1, T2>& value, bool delimiter = false)
        {
            return merge({ Converter<T1>::toBytes(value.first, true), Converter<T2>::toBytes(value.second, delimiter) });
        }

        static std::pair<T1, T2> convert(unsigned char*& ptr, size_t& total_size, bool delimiter = false)
        {
            const T1& first = Converter<T1>::convert(ptr, total_size, true);
            const T2& second = Converter<T2>::convert(ptr, total_size, delimiter);
            return std::pair<T1, T2>(first, second);
        }
    };

    template <typename T>
    class Converter<std::vector<T>> {
    public:
        static const bool special = true;

        static Bytes toBytes(const std::vector<T>& value, bool delimiter = false)
        {
            if (!Converter<T>::special) {
                if (delimiter) {
                    return merge(Bytes((CollectionSizeType)value.size()), toBytes(value, false));
                } else {
                    return Bytes { reinterpret_cast<const unsigned char*>(&value[0]), value.size() * sizeof(T) };
                }
            } else {
                std::vector<Bytes> result(value.size() + delimiter);
                if (delimiter) {
                    result[0] = Bytes((CollectionSizeType)value.size());
                }
                for (CollectionSizeType i = 0; i < value.size(); ++i) {
                    result[i + delimiter] = std::move(Converter<T>::toBytes(value[i], true));
                }
                return merge(result);
            }
        }

        static std::vector<T> convert(unsigned char*& ptr, size_t& total_size, bool delimiter = false)
        {
            if (!Converter<T>::special) {
                if (delimiter) {
                    const CollectionSizeType size = Converter<CollectionSizeType>::convert(ptr, total_size, true);
                    ptr += size * sizeof(T);
                    auto bptr = reinterpret_cast<T*>(ptr - size * sizeof(T));
                    return std::vector<T>(bptr, bptr + size);
                } else {
                    auto bptr = reinterpret_cast<T*>(ptr);
                    return std::vector<T>(bptr, bptr + total_size / sizeof(T));
                }
            } else {
                if (delimiter) {
                    const CollectionSizeType size = Converter<CollectionSizeType>::convert(ptr, total_size, true);
                    std::vector<T> result;
                    for (size_t i = 0; i < size; ++i) {
                        result.emplace_back(Converter<T>::convert(ptr, total_size, true));
                    }
                    return result;
                } else {
                    std::vector<T> result;
                    while (total_size > 0) {
                        const auto current = total_size;
                        result.emplace_back(Converter<T>::convert(ptr, total_size, true));
                        if (current == total_size)
                            break;
                    }
                    return result;
                }
            }
        }
    };

    template <typename T, size_t N>
    class Converter<std::array<T, N>> {
    public:
        static const bool special = false;

        static Bytes toBytes(const std::array<T, N>& value, bool delimiter = false)
        {
            return Converter<std::vector<T>>::toBytes(std::vector<T>(value.cbegin(), value.cend()), delimiter);
        }

        static std::array<T, N> convert(unsigned char*& ptr, size_t& total_size, bool delimiter = false)
        {
            return Converter<std::vector<T>>::convert(ptr, total_size, delimiter);
        }
    };

    template <typename T>
    class Converter<std::set<T>> {
    public:
        static const bool special = true;

        static Bytes toBytes(const std::set<T>& value, bool delimiter = false)
        {
            return Converter<std::vector<T>>::toBytes(std::vector<T>(value.cbegin(), value.cend()), delimiter);
        }

        static std::set<T> convert(unsigned char*& ptr, size_t& total_size, bool delimiter = false)
        {
            auto result = Converter<std::vector<T>>::convert(ptr, total_size, delimiter);
            return std::set<T>(result.begin(), result.end());
        }
    };

    template <typename T1, typename T2>
    class Converter<std::map<T1, T2>> {
    public:
        static const bool special = true;

        using Key = std::vector<typename std::map<T1, T2>::value_type>;

        static Bytes toBytes(const std::map<T1, T2>& value, bool delimiter = false)
        {
            return Converter<Key>::toBytes(Key(value.cbegin(), value.cend()), delimiter);
        }

        static std::map<T1, T2> convert(unsigned char*& ptr, size_t& total_size, bool delimiter = false)
        {
            auto result = Converter<Key>::convert(ptr, total_size, delimiter);
            return std::map<T1, T2>(result.begin(), result.end());
        }
    };
};

// list of converters
template <>
class Bytes::Converter<const unsigned char*> {
public:
    static const bool special = true;

    static Bytes toBytes(const unsigned char* value, bool delimiter = false)
    {
        if (delimiter) {
            auto result = toBytes(value, false);
            return merge(Bytes((CollectionSizeType)result.size()), result);
        } else {
            return Bytes { value };
        }
    };
};

template <>
class Bytes::Converter<const char*> {
public:
    static const bool special = true;

    static Bytes toBytes(const char* value, bool delimiter = false)
    {
        if (delimiter) {
            auto result = toBytes(value, false);
            return merge(Bytes((CollectionSizeType)result.size()), Bytes { value });
        } else {
            return Bytes { value };
        }
    }
};

template <>
class Bytes::Converter<std::string> {
public:
    static const bool special = true;

    static Bytes toBytes(const std::string& value, bool delimiter = false)
    {
        if (delimiter) {
            // attach size in front of the data
            return merge(Converter<CollectionSizeType>::toBytes((CollectionSizeType)value.size()), Bytes { value });
        } else {
            return Bytes { value };
        };
    }

    static std::string convert(unsigned char*& ptr, size_t& total_size, bool delimiter = false)
    {
        if (delimiter) {
            const CollectionSizeType size = Converter<CollectionSizeType>::convert(ptr, total_size, delimiter);
            ptr += size;
            total_size -= size;
            auto bptr = reinterpret_cast<char*>(ptr - size);
            return std::string { bptr, bptr + size };
        } else {
            auto bptr = reinterpret_cast<char*>(ptr);
            return std::string { bptr, bptr + total_size };
        };
    }
};

template <>
class Bytes::Converter<Bytes> {
public:
    static const bool special = true;

    static Bytes toBytes(const Bytes& bytes, bool delimiter = false)
    {
        if (delimiter) {
            // attach size in front of the data
            return merge(Converter<CollectionSizeType>::toBytes((CollectionSizeType)bytes.size()), bytes);
        } else {
            return bytes;
        }
    }

    static Bytes convert(unsigned char*& ptr, size_t& total_size, bool delimiter = false)
    {
        if (delimiter) {
            const CollectionSizeType size = Converter<CollectionSizeType>::convert(ptr, total_size, delimiter);
            ptr += size;
            total_size -= size;
            return Bytes { ptr - size, size };
        } else {
            return Bytes { ptr, total_size };
        }
    }
};

class ResultSetCursor;

class Record {
public:
    using PropertyToBytesMap = std::map<std::string, Bytes>;

    explicit Record() = default;

    template <typename T>
    Record& set(const std::string& propName, const T& value)
    {
        if (!propName.empty() && !isBasicInfo(propName)) {
            properties[propName] = Bytes::Converter<T>::toBytes(value);
        }
        return *this;
    }

    template <typename T>
    Record& setIfNotExists(const std::string& propName, const T& value)
    {
        if (properties.find(propName) == properties.cend()) {
            set(propName, value);
        }
        return *this;
    }

    const PropertyToBytesMap& getAll() const;

    const PropertyToBytesMap& getBasicInfo() const;

    std::vector<std::string> getProperties() const;

    Bytes get(const std::string& propName) const;

    uint8_t getTinyIntU(const std::string& propName) const;

    int8_t getTinyInt(const std::string& propName) const;

    uint16_t getSmallIntU(const std::string& propName) const;

    int16_t getSmallInt(const std::string& propName) const;

    uint32_t getIntU(const std::string& propName) const;

    int32_t getInt(const std::string& propName) const;

    uint64_t getBigIntU(const std::string& propName) const;

    int64_t getBigInt(const std::string& propName) const;

    double getReal(const std::string& propName) const;

    std::string getText(const std::string& propName) const;

    std::string getClassName() const;

    RecordId getRecordId() const;

    uint32_t getDepth() const;

    uint64_t getVersion() const;

    void unset(const std::string& className);

    size_t size() const;

    bool empty() const;

    void clear();

private:
    friend class parser::RecordParser;
    friend class algorithm::GraphTraversal;
    friend class sql_parser::Record;
    friend class ResultSetCursor;

    Record(PropertyToBytesMap properties);

    Record(PropertyToBytesMap properties, PropertyToBytesMap basicProperties)
        : properties(std::move(properties))
        , basicProperties(std::move(basicProperties))
    {
    }

    inline bool isBasicInfo(const std::string& str) const { return str.at(0) == '@'; }

    PropertyToBytesMap properties {};
    mutable PropertyToBytesMap basicProperties {};

    template <typename T>
    const Record& setBasicInfo(const std::string& propName, const T& value) const
    {
        if (!propName.empty() && isBasicInfo(propName)) {
            basicProperties[propName] = Bytes::Converter<T>::toBytes(value);
        }
        return *this;
    };

    template <typename T>
    const Record& setBasicInfoIfNotExists(const std::string& propName, const T& value) const
    {
        if (basicProperties.find(propName) == basicProperties.cend()) {
            setBasicInfo(propName, value);
        }
        return *this;
    };
};

struct RecordDescriptor {
    RecordDescriptor() = default;

    RecordDescriptor(const ClusterId& clusterId, const ClassId& classId, const PositionId& posId)
        : cid { clusterId }
        , rid { classId, posId }
    {
    }

    RecordDescriptor(const ClusterId& clusterId, const RecordId& recordId)
        : cid { clusterId }
        , rid { recordId }
    {
    }

    RecordDescriptor(const ClassId& classId, const PositionId& posId)
        : cid { 0 }
        , rid { classId, posId }
    {
    }

    RecordDescriptor(const RecordId& recordId)
        : cid { 0 }
        , rid { recordId }
    {
    }

    virtual ~RecordDescriptor() noexcept = default;

    ClusterId cid { 0 };
    RecordId rid { 0, 0 };

private:
    friend class algorithm::GraphTraversal;
    friend class ResultSetCursor;

    unsigned int _depth { 0 };
};

struct IndexDescriptor {
    IndexDescriptor() = default;

    IndexDescriptor(const IndexId& _id, const ClassId& _classId, const PropertyId& _propertyId, bool _isUnique)
        : id { _id }
        , classId { _classId }
        , propertyId { _propertyId }
        , unique { _isUnique }
    {
    }

    IndexId id { 0 };
    ClassId classId { 0 };
    PropertyId propertyId { 0 };
    bool unique { true };
};

struct PropertyDescriptor {
    PropertyDescriptor() = default;

    PropertyDescriptor(const PropertyId& _id, const std::string& _name, const PropertyType& _type, bool _inherited)
        : id { _id }
        , name { _name }
        , type { _type }
        , inherited { _inherited }
    {
    }

    PropertyId id { 0 };
    std::string name { "" };
    PropertyType type { PropertyType::UNDEFINED };
    bool inherited { false };
};

struct ClassDescriptor {
    ClassDescriptor() = default;

    ClassDescriptor(const ClassId& _id, const std::string& _name, const ClassId& _base, const ClassType& _type)
        : id { _id }
        , name { _name }
        , base { _base }
        , type { _type }
    {
    }

    ClassId id { 0 };
    std::string name { "" };
    ClassId base { 0 };
    ClassType type { ClassType::UNDEFINED };
};

struct Result {
    Result() = default;

    Result(const RecordDescriptor& recordDescriptor_, const Record& record_)
        : descriptor { recordDescriptor_ }
        , record { record_ }
    {
    }

    RecordDescriptor descriptor {};
    Record record {};
};

typedef std::vector<Result> ResultSet;

class ResultSetCursor {
public:
    friend class FindOperationBuilder;
    friend class FindEdgeOperationBuilder;
    friend class TraverseOperationBuilder;
    friend class ShortestPathOperationBuilder;

    ResultSetCursor(const Transaction& _txn);

    ~ResultSetCursor() noexcept;

    ResultSetCursor(ResultSetCursor&& rc) noexcept;

    ResultSetCursor& operator=(ResultSetCursor&& rc) noexcept;

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

    const Result& operator*() const;

    const Result* operator->() const;

private:
    friend struct datarecord::DataRecordUtils;

    const Transaction* txn;
    std::vector<RecordDescriptor> metadata {};
    long long currentIndex;
    Result result;

    ResultSetCursor& addMetadata(const RecordDescriptor& recordDescriptor)
    {
        metadata.emplace_back(recordDescriptor);
        return *this;
    }

    ResultSetCursor& addMetadata(const std::vector<RecordDescriptor>& recordDescriptors)
    {
        metadata.insert(metadata.cend(), recordDescriptors.cbegin(), recordDescriptors.cend());
        return *this;
    }

    ResultSetCursor& addMetadata(const ResultSetCursor& resultSetCursor)
    {
        metadata.insert(metadata.cend(), resultSetCursor.metadata.cbegin(), resultSetCursor.metadata.cend());
        return *this;
    }
};

class GraphFilter {
public:
    friend class compare::RecordCompare;

    GraphFilter();

    explicit GraphFilter(const Condition& condition);

    explicit GraphFilter(const MultiCondition& multiCondition);

    explicit GraphFilter(bool (*function)(const Record& record));

    ~GraphFilter() noexcept = default;

    GraphFilter(const GraphFilter& other);

    GraphFilter& operator=(const GraphFilter& other);

    GraphFilter(GraphFilter&& other) noexcept;

    GraphFilter& operator=(GraphFilter&& other) noexcept;

    GraphFilter& only(const std::string& className);

    template <typename... T>
    GraphFilter& only(const std::string& className, const T&... classNames)
    {
        only(className);
        only(classNames...);
        return *this;
    }

    GraphFilter& only(const std::vector<std::string>& classNames);

    GraphFilter& only(const std::list<std::string>& classNames);

    GraphFilter& only(const std::set<std::string>& classNames);

    GraphFilter& only(const std::vector<std::string>::const_iterator& begin,
        const std::vector<std::string>::const_iterator& end);

    GraphFilter& only(const std::list<std::string>::const_iterator& begin,
        const std::list<std::string>::const_iterator& end);

    GraphFilter& only(const std::set<std::string>::const_iterator& begin,
        const std::set<std::string>::const_iterator& end);

    GraphFilter& onlySubClassOf(const std::string& className);

    template <typename... T>
    GraphFilter& onlySubClassOf(const std::string& className, const T&... classNames)
    {
        onlySubClassOf(className);
        onlySubClassOf(classNames...);
        return *this;
    }

    GraphFilter& onlySubClassOf(const std::vector<std::string>& classNames);

    GraphFilter& onlySubClassOf(const std::list<std::string>& classNames);

    GraphFilter& onlySubClassOf(const std::set<std::string>& classNames);

    GraphFilter& onlySubClassOf(const std::vector<std::string>::const_iterator& begin,
        const std::vector<std::string>::const_iterator& end);

    GraphFilter& onlySubClassOf(const std::list<std::string>::const_iterator& begin,
        const std::list<std::string>::const_iterator& end);

    GraphFilter& onlySubClassOf(const std::set<std::string>::const_iterator& begin,
        const std::set<std::string>::const_iterator& end);

    GraphFilter& exclude(const std::string& className);

    template <typename... T>
    GraphFilter& exclude(const std::string& className, const T&... classNames)
    {
        exclude(className);
        exclude(classNames...);
        return *this;
    }

    GraphFilter& exclude(const std::vector<std::string>& classNames);

    GraphFilter& exclude(const std::list<std::string>& classNames);

    GraphFilter& exclude(const std::set<std::string>& classNames);

    GraphFilter& exclude(const std::vector<std::string>::const_iterator& begin,
        const std::vector<std::string>::const_iterator& end);

    GraphFilter& exclude(const std::list<std::string>::const_iterator& begin,
        const std::list<std::string>::const_iterator& end);

    GraphFilter& exclude(const std::set<std::string>::const_iterator& begin,
        const std::set<std::string>::const_iterator& end);

    GraphFilter& excludeSubClassOf(const std::string& className);

    template <typename... T>
    GraphFilter& excludeSubClassOf(const std::string& className, const T&... classNames)
    {
        excludeSubClassOf(className);
        excludeSubClassOf(classNames...);
        return *this;
    }

    GraphFilter& excludeSubClassOf(const std::vector<std::string>& classNames);

    GraphFilter& excludeSubClassOf(const std::list<std::string>& classNames);

    GraphFilter& excludeSubClassOf(const std::set<std::string>& classNames);

    GraphFilter& excludeSubClassOf(const std::vector<std::string>::const_iterator& begin,
        const std::vector<std::string>::const_iterator& end);

    GraphFilter& excludeSubClassOf(const std::list<std::string>::const_iterator& begin,
        const std::list<std::string>::const_iterator& end);

    GraphFilter& excludeSubClassOf(const std::set<std::string>::const_iterator& begin,
        const std::set<std::string>::const_iterator& end);

private:
    enum class FilterMode {
        CONDITION,
        MULTI_CONDITION,
        COMPARE_FUNCTION
    };

    FilterMode _mode;

    //TODO: can be improved by using std::varient in c++17
    std::shared_ptr<Condition> _condition {};
    std::shared_ptr<MultiCondition> _multiCondition {};
    bool (*_function)(const Record& record);

    std::set<std::string> _onlyClasses {};
    std::set<std::string> _onlySubOfClasses {};
    std::set<std::string> _ignoreClasses {};
    std::set<std::string> _ignoreSubOfClasses {};
};

class Condition {
private:
    enum class Comparator;

public:
    friend class MultiCondition;
    friend class compare::RecordCompare;

    Condition(const std::string& _propName);

    ~Condition() noexcept = default;

    operator GraphFilter() { return GraphFilter(*this); }

    template <typename T>
    Condition eq(T value_) const
    {
        auto tmp(*this);
        tmp.valueBytes = Bytes { value_ };
        tmp.comp = Comparator::EQUAL;
        return tmp;
    }

    template <typename T>
    Condition gt(T value_) const
    {
        auto tmp(*this);
        tmp.valueBytes = Bytes { value_ };
        tmp.comp = Comparator::GREATER;
        return tmp;
    }

    template <typename T>
    Condition lt(T value_) const
    {
        auto tmp(*this);
        tmp.valueBytes = Bytes { value_ };
        tmp.comp = Comparator::LESS;
        return tmp;
    }

    template <typename T>
    Condition ge(T value_) const
    {
        auto tmp(*this);
        tmp.valueBytes = Bytes { value_ };
        tmp.comp = Comparator::GREATER_EQUAL;
        return tmp;
    }

    template <typename T>
    Condition le(T value_) const
    {
        auto tmp(*this);
        tmp.valueBytes = Bytes { value_ };
        tmp.comp = Comparator::LESS_EQUAL;
        return tmp;
    }

    template <typename T>
    Condition contain(T value_) const
    {
        auto tmp(*this);
        tmp.valueBytes = Bytes { value_ };
        tmp.comp = Comparator::CONTAIN;
        return tmp;
    }

    template <typename T>
    Condition beginWith(T value_) const
    {
        auto tmp(*this);
        tmp.valueBytes = Bytes { value_ };
        tmp.comp = Comparator::BEGIN_WITH;
        return tmp;
    }

    template <typename T>
    Condition endWith(T value_) const
    {
        auto tmp(*this);
        tmp.valueBytes = Bytes { value_ };
        tmp.comp = Comparator::END_WITH;
        return tmp;
    }

    template <typename T>
    Condition like(T value_) const
    {
        auto tmp(*this);
        tmp.valueBytes = Bytes { value_ };
        tmp.comp = Comparator::LIKE;
        return tmp;
    }

    template <typename T>
    Condition regex(T value_) const
    {
        auto tmp(*this);
        tmp.valueBytes = Bytes { value_ };
        tmp.comp = Comparator::REGEX;
        return tmp;
    }

    Condition ignoreCase() const
    {
        auto tmp(*this);
        tmp.isIgnoreCase = true;
        return tmp;
    }

    Condition null() const
    {
        auto tmp(*this);
        tmp.valueBytes = Bytes {};
        tmp.comp = Comparator::IS_NULL;
        return tmp;
    }

    template <typename T>
    Condition between(T lower_, T upper_, const std::pair<bool, bool> isIncludeBound = { true, true }) const
    {
        auto tmp(*this);
        tmp.valueSet = std::vector<Bytes> { Bytes { lower_ }, Bytes { upper_ } };
        if (!isIncludeBound.first && !isIncludeBound.second) {
            tmp.comp = Comparator::BETWEEN_NO_BOUND;
        } else if (!isIncludeBound.first && isIncludeBound.second) {
            tmp.comp = Comparator::BETWEEN_NO_LOWER;
        } else if (isIncludeBound.first && !isIncludeBound.second) {
            tmp.comp = Comparator::BETWEEN_NO_UPPER;
        } else {
            tmp.comp = Comparator::BETWEEN;
        }
        return tmp;
    }

    template <typename T>
    Condition in(const std::vector<T>& values) const
    {
        auto tmp(*this);
        tmp.valueSet = std::vector<Bytes> {};
        for (auto iter = values.cbegin(); iter != values.cend(); ++iter) {
            tmp.valueSet.emplace_back(Bytes { *iter });
        }
        tmp.comp = Comparator::IN;
        return tmp;
    }

    template <typename T>
    Condition in(const std::list<T>& values) const
    {
        auto tmp(*this);
        tmp.valueSet = std::vector<Bytes> {};
        for (auto iter = values.cbegin(); iter != values.cend(); ++iter) {
            tmp.valueSet.emplace_back(Bytes { *iter });
        }
        tmp.comp = Comparator::IN;
        return tmp;
    }

    template <typename T>
    Condition in(const std::set<T>& values) const
    {
        auto tmp(*this);
        tmp.valueSet = std::vector<Bytes> {};
        for (auto iter = values.cbegin(); iter != values.cend(); ++iter) {
            tmp.valueSet.emplace_back(Bytes { *iter });
        }
        tmp.comp = Comparator::IN;
        return tmp;
    }

    template <typename T>
    Condition in(const T& value) const
    {
        auto tmp(*this);
        tmp.valueSet = std::vector<Bytes> { Bytes { value } };
        tmp.comp = Comparator::IN;
        return tmp;
    }

    template <typename T, typename... U>
    Condition in(const T& head, const U&... tail) const
    {
        auto tmp = in(head);
        for (const auto& value : in(tail...).valueSet) {
            tmp.valueSet.emplace_back(value);
        }
        return tmp;
    }

    MultiCondition operator&&(const Condition& c) const;

    MultiCondition operator&&(const MultiCondition& e) const;

    MultiCondition operator||(const Condition& c) const;

    MultiCondition operator||(const MultiCondition& e) const;

    MultiCondition operator&&(bool (*cmpFunc)(const Record& r)) const;

    MultiCondition operator||(bool (*cmpFunc)(const Record& r)) const;

    Condition operator!() const;

private:
    friend struct datarecord::DataRecordUtils;
    friend struct index::IndexUtils;

    std::string propName;
    Bytes valueBytes;
    std::vector<Bytes> valueSet;
    Comparator comp;
    bool isIgnoreCase { false };
    bool isNegative { false };

    enum class Comparator {
        IS_NULL,
        NOT_NULL,
        EQUAL,
        GREATER,
        LESS,
        GREATER_EQUAL,
        LESS_EQUAL,
        CONTAIN,
        BEGIN_WITH,
        END_WITH,
        LIKE,
        REGEX,
        IN,
        BETWEEN,
        BETWEEN_NO_UPPER,
        BETWEEN_NO_LOWER,
        BETWEEN_NO_BOUND
    };
};

class MultiCondition {
private:
    enum Operator {
        AND,
        OR
    };

    class CompositeNode;

    class ConditionNode;

    class CmpFunctionNode;

public:
    friend class Condition;
    friend class compare::RecordCompare;
    friend struct index::IndexUtils;

    MultiCondition() = delete;

    MultiCondition operator&&(const MultiCondition& e) const;

    MultiCondition operator&&(const Condition& c) const;

    MultiCondition operator||(const MultiCondition& e) const;

    MultiCondition operator||(const Condition& c) const;

    MultiCondition operator&&(bool (*cmpFunc)(const Record& r)) const;

    MultiCondition operator||(bool (*cmpFunc)(const Record& r)) const;

    MultiCondition operator!() const;

    operator GraphFilter() { return GraphFilter(*this); }

    bool execute(const Record& r, const PropertyMapType& propType) const;

private:
    std::shared_ptr<CompositeNode> root;
    std::vector<std::weak_ptr<ConditionNode>> conditions;
    std::vector<std::weak_ptr<CmpFunctionNode>> cmpFunctions;

    MultiCondition(const Condition& c1, const Condition& c2, Operator opt);

    MultiCondition(const Condition& c, const MultiCondition& e, Operator opt);

    MultiCondition(const Condition& c, bool (*cmpFunc)(const Record& r), Operator opt);

    enum ExprNodeType {
        CONDITION,
        MULTI_CONDITION,
        CMP_FUNCTION
    };

    class ExprNode {
    public:
        explicit ExprNode(const ExprNodeType type);

        virtual ~ExprNode() noexcept = default;

        virtual bool check(const Record& r, const PropertyMapType& propType) const = 0;

        bool checkIfCondition() const;

        bool checkIfCmpFunction() const;

    private:
      ExprNodeType nodeType;
    };

    class ConditionNode : public ExprNode {
    public:
        explicit ConditionNode(const Condition& cond_);

        ~ConditionNode() noexcept override = default;

        virtual bool check(const Record& r, const PropertyMapType& propType) const override;

        const Condition& getCondition() const;

    private:
        Condition cond;
    };

    class CmpFunctionNode : public ExprNode {
    public:
        explicit CmpFunctionNode(bool (*cmpFunc_)(const Record& record));

        ~CmpFunctionNode() noexcept override = default;

        virtual bool check(const Record& r, const PropertyMapType& propType) const override;

    private:
        bool (*cmpFunc)(const Record& record);
    };

    class CompositeNode : public ExprNode {
    public:
        CompositeNode(const std::shared_ptr<ExprNode>& left_,
            const std::shared_ptr<ExprNode>& right_,
            Operator opt_,
            bool isNegative_ = false);

        ~CompositeNode() noexcept override = default;

        virtual bool check(const Record& r, const PropertyMapType& propType) const override;

        const std::shared_ptr<ExprNode>& getLeftNode() const;

        const std::shared_ptr<ExprNode>& getRightNode() const;

        const Operator& getOperator() const;

        bool getIsNegative() const;

    private:
        std::shared_ptr<ExprNode> left;
        std::shared_ptr<ExprNode> right;
        Operator opt;
        bool isNegative;
    };
};

class OperationBuilder {
public:
    enum class ConditionType {
        CONDITION,
        MULTI_CONDITION,
        COMPARE_FUNCTION,
        UNDEFINED
    };

    enum class EdgeDirection {
        IN,
        OUT,
        UNDIRECTED
    };

protected:
    OperationBuilder(const Transaction* txn);

    virtual ~OperationBuilder() noexcept = default;

    //TODO: improve the performance of record retrieval by having get(parallel = n);
    virtual ResultSet get() const = 0;

    //TODO: improve the performance of record retrieval by having getCursor(parallel = n);
    virtual ResultSetCursor getCursor() const = 0;

    //TODO: improve the performance of record retrieval by having count(parallel = n);
    virtual unsigned long count() const = 0;

    const Transaction* _txn;
};

class FindOperationBuilder : public OperationBuilder {
public:
    FindOperationBuilder() = delete;

    virtual ~FindOperationBuilder() noexcept = default;

    virtual FindOperationBuilder& where(const Condition& condition);

    virtual FindOperationBuilder& where(const MultiCondition& multiCondition);

    virtual FindOperationBuilder& where(bool (*condition)(const Record& record));

    virtual FindOperationBuilder& indexed(bool onlyIndex = true);

    //    virtual FindOperationBuilder& limit(unsigned int size);
    //
    //    virtual FindOperationBuilder& limit(unsigned int from, unsigned int to);
    //
    //    virtual FindOperationBuilder& orderBy(const std::string &propName);
    //
    //    template<typename ...T>
    //    FindOperationBuilder& orderBy(const std::string &propName, const T &... propNames) {
    //      orderBy(propName);
    //      orderBy(propNames...);
    //      return *this;
    //    }
    //
    //    virtual FindOperationBuilder& orderBy(const std::vector<std::string> &propNames);
    //
    //    virtual FindOperationBuilder& orderBy(const std::list<std::string> &propNames);
    //
    //    virtual FindOperationBuilder& orderBy(const std::set<std::string> &propNames);
    //
    //    virtual FindOperationBuilder& orderBy(const std::vector<std::string>::const_iterator &begin,
    //                                          const std::vector<std::string>::const_iterator &end);
    //
    //    virtual FindOperationBuilder& orderBy(const std::list<std::string>::const_iterator &begin,
    //                                          const std::list<std::string>::const_iterator &end);
    //
    //    virtual FindOperationBuilder& orderBy(const std::set<std::string>::const_iterator &begin,
    //                                          const std::set<std::string>::const_iterator &end);

    ResultSet get() const;

    ResultSetCursor getCursor() const;

    unsigned long count() const;

private:
    friend class Transaction;

    FindOperationBuilder(const Transaction* txn, const std::string& className, bool includeSubClassOf);

    std::string _className;
    ConditionType _conditionType;
    bool _includeSubClassOf;
    bool _indexed { false };
    std::vector<std::string> _orderBy {};

    //TODO: can be improved by using std::varient in c++17
    std::shared_ptr<Condition> _condition {};
    std::shared_ptr<MultiCondition> _multiCondition {};
    bool (*_function)(const Record& record);
};

class FindEdgeOperationBuilder : public OperationBuilder {
public:
    FindEdgeOperationBuilder() = delete;

    virtual ~FindEdgeOperationBuilder() noexcept = default;

    virtual FindEdgeOperationBuilder& where(const GraphFilter& edgeFilter);

    //    virtual FindEdgeOperationBuilder& limit(unsigned int size);
    //
    //    virtual FindEdgeOperationBuilder& limit(unsigned int from, unsigned int to);
    //
    //    virtual FindEdgeOperationBuilder& orderBy(const std::string &propName);
    //
    //    template<typename ...T>
    //    FindEdgeOperationBuilder& orderBy(const std::string &propName, const T &... propNames) {
    //      orderBy(propName);
    //      orderBy(propNames...);
    //      return *this;
    //    }
    //
    //    virtual FindEdgeOperationBuilder& orderBy(const std::vector<std::string> &propNames);
    //
    //    virtual FindEdgeOperationBuilder& orderBy(const std::list<std::string> &propNames);
    //
    //    virtual FindEdgeOperationBuilder& orderBy(const std::set<std::string> &propNames);
    //
    //    virtual FindEdgeOperationBuilder& orderBy(const std::vector<std::string>::const_iterator &begin,
    //                                              const std::vector<std::string>::const_iterator &end);
    //
    //    virtual FindEdgeOperationBuilder& orderBy(const std::list<std::string>::const_iterator &begin,
    //                                              const std::list<std::string>::const_iterator &end);
    //
    //    virtual FindEdgeOperationBuilder& orderBy(const std::set<std::string>::const_iterator &begin,
    //                                              const std::set<std::string>::const_iterator &end);

    ResultSet get() const;

    ResultSetCursor getCursor() const;

    unsigned long count() const;

private:
    friend class Transaction;

    FindEdgeOperationBuilder(const Transaction* txn,
        const RecordDescriptor& recordDescriptor,
        const EdgeDirection& direction);

    RecordDescriptor _rdesc;
    EdgeDirection _direction;
    GraphFilter _filter {};
    std::vector<std::string> _orderBy {};
};

class TraverseOperationBuilder : public OperationBuilder {
public:
    TraverseOperationBuilder() = delete;

    virtual ~TraverseOperationBuilder() noexcept = default;

    virtual TraverseOperationBuilder& addSource(const RecordDescriptor& recordDescriptor);

    virtual TraverseOperationBuilder& whereV(const GraphFilter& filter);

    virtual TraverseOperationBuilder& whereE(const GraphFilter& filter);

    virtual TraverseOperationBuilder& minDepth(unsigned int depth);

    virtual TraverseOperationBuilder& maxDepth(unsigned int depth);

    virtual TraverseOperationBuilder& depth(unsigned int minDepth, unsigned int maxDepth);

    //    virtual TraverseOperationBuilder& orderBy(const std::string &propName);
    //
    //    template<typename ...T>
    //    TraverseOperationBuilder& orderBy(const std::string &propName, const T &... propNames) {
    //      orderBy(propName);
    //      orderBy(propNames...);
    //      return *this;
    //    }
    //
    //    virtual TraverseOperationBuilder& orderBy(const std::vector<std::string> &propNames);
    //
    //    virtual TraverseOperationBuilder& orderBy(const std::list<std::string> &propNames);
    //
    //    virtual TraverseOperationBuilder& orderBy(const std::set<std::string> &propNames);
    //
    //    virtual TraverseOperationBuilder& orderBy(const std::vector<std::string>::const_iterator &begin,
    //                                              const std::vector<std::string>::const_iterator &end);
    //
    //    virtual TraverseOperationBuilder& orderBy(const std::list<std::string>::const_iterator &begin,
    //                                              const std::list<std::string>::const_iterator &end);
    //
    //    virtual TraverseOperationBuilder& orderBy(const std::set<std::string>::const_iterator &begin,
    //                                              const std::set<std::string>::const_iterator &end);

    ResultSet get() const;

    ResultSetCursor getCursor() const;

    unsigned long count() const;

private:
    friend class Transaction;

    TraverseOperationBuilder(const Transaction* txn,
        const RecordDescriptor& recordDescriptor,
        const EdgeDirection& direction);

    std::set<RecordDescriptor> _rdescs {};
    EdgeDirection _direction;
    unsigned int _minDepth { 0 };
    unsigned int _maxDepth { std::numeric_limits<unsigned int>::max() };
    GraphFilter _edgeFilter {};
    GraphFilter _vertexFilter {};
    std::vector<std::string> _orderBy {};
};

class ShortestPathOperationBuilder : public OperationBuilder {
public:
    ShortestPathOperationBuilder() = delete;

    virtual ~ShortestPathOperationBuilder() noexcept = default;

    virtual ShortestPathOperationBuilder& whereV(const GraphFilter& filter);

    virtual ShortestPathOperationBuilder& whereE(const GraphFilter& filter);

    //    virtual ShortestPathOperationBuilder& minDepth(unsigned int depth);
    //
    //    virtual ShortestPathOperationBuilder& maxDepth(unsigned int depth);
    //
    //    virtual ShortestPathOperationBuilder& depth(unsigned int minDepth, unsigned int maxDepth);
    //
    //    virtual ShortestPathOperationBuilder& orderBy(const std::string &propName);
    //
    //    template<typename ...T>
    //    ShortestPathOperationBuilder& orderBy(const std::string &propName, const T &... propNames) {
    //      orderBy(propName);
    //      orderBy(propNames...);
    //      return *this;
    //    }
    //
    //    virtual ShortestPathOperationBuilder& orderBy(const std::vector<std::string> &propNames);
    //
    //    virtual ShortestPathOperationBuilder& orderBy(const std::list<std::string> &propNames);
    //
    //    virtual ShortestPathOperationBuilder& orderBy(const std::set<std::string> &propNames);
    //
    //    virtual ShortestPathOperationBuilder& orderBy(const std::vector<std::string>::const_iterator &begin,
    //                                                  const std::vector<std::string>::const_iterator &end);
    //
    //    virtual ShortestPathOperationBuilder& orderBy(const std::list<std::string>::const_iterator &begin,
    //                                                  const std::list<std::string>::const_iterator &end);
    //
    //    virtual ShortestPathOperationBuilder& orderBy(const std::set<std::string>::const_iterator &begin,
    //                                                  const std::set<std::string>::const_iterator &end);

    ResultSet get() const;

    ResultSetCursor getCursor() const;

    unsigned long count() const;

private:
    friend class Transaction;

    ShortestPathOperationBuilder(const Transaction* txn,
        const RecordDescriptor& srcVertexRecordDescriptor,
        const RecordDescriptor& dstVertexRecordDescriptor);

    RecordDescriptor _srcRdesc;
    RecordDescriptor _dstRdesc;
    unsigned int _minDepth { 0 };
    unsigned int _maxDepth { std::numeric_limits<unsigned int>::max() };
    GraphFilter _edgeFilter {};
    GraphFilter _vertexFilter {};
    std::vector<std::string> _orderBy {};
};

inline bool operator<(const RecordId& lhs, const RecordId& rhs)
{
    return (lhs.first == rhs.first) ? lhs.second < rhs.second : lhs.first < rhs.first;
}

inline bool operator==(const RecordId& lhs, const RecordId& rhs)
{
    return (lhs.first == rhs.first) && (lhs.second == rhs.second);
}

inline bool operator!=(const RecordId& lhs, const RecordId& rhs)
{
    return !operator==(lhs, rhs);
}

inline bool operator<(const RecordDescriptor& lhs, const RecordDescriptor& rhs)
{
    return lhs.rid < rhs.rid;
}

inline bool operator==(const RecordDescriptor& lhs, const RecordDescriptor& rhs)
{
    return lhs.rid == rhs.rid;
}

inline bool operator!=(const RecordDescriptor& lhs, const RecordDescriptor& rhs)
{
    return !operator==(lhs, rhs);
}

inline bool operator==(const ClassDescriptor& lhs, const ClassDescriptor& rhs)
{
    return (lhs.id == rhs.id) && (lhs.name == rhs.name) && (lhs.type == rhs.type) && (lhs.base == rhs.base);
}

inline bool operator==(const PropertyDescriptor& lhs, const PropertyDescriptor& rhs)
{
    return (lhs.id == rhs.id) && (lhs.name == rhs.name) && (lhs.type == rhs.type) && (lhs.inherited == rhs.inherited);
}

inline bool operator==(const IndexDescriptor& lhs, const IndexDescriptor& rhs)
{
    return (lhs.id == rhs.id) && (lhs.classId == rhs.classId) && (lhs.propertyId == rhs.propertyId) && (lhs.unique == rhs.unique);
}

inline std::string rid2str(const nogdb::RecordId& rid)
{
    std::stringstream ss {};
    ss << std::to_string(rid.first) << ":" << std::to_string(rid.second);
    return ss.str();
}

struct RecordIdHash {
    inline uint64_t operator()(const std::pair<ClassId, PositionId>& rid) const
    {
        return (static_cast<uint64_t>(rid.first) << 32) + static_cast<uint64_t>(rid.second);
    }
};

}

inline std::ostream& operator<<(std::ostream& os, const nogdb::RecordId& rid)
{
    os << "#" << std::to_string(rid.first) << ":" << std::to_string(rid.second);
    return os;
}

inline std::ostream& operator<<(std::ostream& os, nogdb::PropertyType type)
{
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

inline std::ostream& operator<<(std::ostream& os, nogdb::ClassType type)
{
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
