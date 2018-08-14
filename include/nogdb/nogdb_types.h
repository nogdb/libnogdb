/*
 *
 *  Copyright (C) 2018, Throughwave (Thailand) Co., Ltd.
 *  <peerawich at throughwave dot co dot th>
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
#include <set>
#include <type_traits>

//******************************************************************
//*  Forward declarations of NogDB and boost internal classes.     *
//******************************************************************

namespace boost {
    class shared_mutex;
}

namespace nogdb {
    struct Graph;
    struct Validate;
    struct Algorithm;
    struct Compare;
    struct Generic;
    struct Schema;
    struct Class;
    struct Property;
    struct DB;
    struct Vertex;
    struct Edge;
    struct Traverse;

    class Parser;

    class TxnStat;

    class BaseTxn;

    class Condition;

    class MultiCondition;

    class PathFilter;

    class RWSpinLock;

    namespace storage_engine {
        class LMDBEnv;
        class LMDBTxn;
    }

    namespace parser {
        class Parser;
    }
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

    class Txn;

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

        template<typename T>
        T convert() const {
            unsigned char * ptr = getRaw();
            size_t size = this->size();
            return Converter<T>::convert(ptr, size, false);
        }

        template<typename T>
        static Bytes toBytes(const T& data) {
            return Converter<T>::toBytes(data);
        }
    private:
        unsigned char *value_{nullptr};
        size_t size_{0};

        static Bytes merge(const Bytes& bytes1, const Bytes& byte2);
        static Bytes merge(const std::vector<Bytes> &bytes);

        using CollectionSizeType = uint16_t;

        template<typename T>
        class Converter {
        public:

            static const bool special = false;

            static Bytes toBytes(const T &value, bool delimiter = false) {
                return Bytes{static_cast<const unsigned char *>((void *) &value), sizeof(T)};
            };

            static T convert(unsigned char * &ptr, size_t& total_size, bool delimiter = false) {
                if (delimiter) {
                    ptr += sizeof(T);
                    total_size -= sizeof(T);
                    return *reinterpret_cast<T*>(ptr - sizeof(T));
                } else {
                    return *reinterpret_cast<T*>(ptr);
                }
            }
        };

        template<typename T> class Converter<const T> : public Converter<T> {};

        template<typename T>
        class Converter<T*> {
        public:

            static const bool special = true;

            static T* convert(unsigned char * &ptr, size_t& total_size, bool delimiter = false) {
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
                        auto * result = new T[size];
                        for (size_t i = 0; i < size; ++i) {
                            result[i] = std::move(Converter<T>::convert(ptr, total_size, true));
                        }
                        return result;
                    } else {
                        std::vector<T> result;
                        while (total_size > 0) {
                            result.emplace_back(Converter<T>::convert(ptr, total_size, true));
                        }
                        auto * data = new T[result.size()];
                        for (size_t i = 0; i < result.size(); ++i) {
                            data[i] = std::move(result[i]);
                        }
                        return data;
                    }
                }
            }
        };

        template<typename T, CollectionSizeType N>
        class Converter<T[N]> {
        public:

            static const bool special = true;

            static Bytes toBytes(const T (&value)[N], bool delimiter = false) {
                if (!Converter<T>::special) {
                    if (delimiter) {
                        return merge(Bytes(N), toBytes(value, false));
                    } else {
                        return Bytes{reinterpret_cast<const unsigned char *>(value),
                                     (N - std::is_same<typename std::remove_const<T>::type, char>::value) * sizeof(T)};
                    }
                } else {
                    std::vector<Bytes> result (N + delimiter);
                    if (delimiter) {
                        result[0] = Bytes(N);
                    }
                    for (CollectionSizeType i = 0; i < N; ++i) {
                        result[i + delimiter] = Converter<T>::toBytes(value[i], true);
                    }
                    return merge(result);
                }
            };

            static T* convert(unsigned char * &ptr, size_t& total_size, bool delimiter = false) {
                return Converter<T*>::convert(ptr, total_size, delimiter);
            }
        };

        template<typename T1, typename T2>
        class Converter<std::pair<T1, T2>> {
        public:

            static const bool special = Converter<T1>::special || Converter<T2>::special;

            static Bytes toBytes(const std::pair<T1, T2>& value, bool delimiter = false) {
                return merge({Converter<T1>::toBytes(value.first, true), Converter<T2>::toBytes(value.second, delimiter)});
            }

            static std::pair<T1, T2> convert(unsigned char * &ptr, size_t& total_size, bool delimiter = false) {
                const T1 &first = Converter<T1>::convert(ptr, total_size, true);
                const T2 &second = Converter<T2>::convert(ptr, total_size, delimiter);
                return std::pair<T1, T2> (first, second);
            }
        };

        template<typename T>
        class Converter<std::vector<T>> {
        public:

            static const bool special = true;

            static Bytes toBytes(const std::vector<T> &value, bool delimiter = false) {
                if (!Converter<T>::special) {
                    if (delimiter) {
                        return merge(Bytes((CollectionSizeType) value.size()), toBytes(value, false));
                    } else {
                        return Bytes{reinterpret_cast<const unsigned char *>(&value[0]), value.size() * sizeof(T)};
                    }
                } else {
                    std::vector<Bytes> result (value.size() + delimiter);
                    if (delimiter) {
                        result[0] = Bytes((CollectionSizeType) value.size());
                    }
                    for (CollectionSizeType i = 0; i < value.size(); ++i) {
                        result[i + delimiter] = std::move(Converter<T>::toBytes(value[i], true));
                    }
                    return merge(result);
                }
            }

            static std::vector<T> convert(unsigned char * &ptr, size_t& total_size, bool delimiter = false) {
                if (!Converter<T>::special) {
                    if (delimiter) {
                        const CollectionSizeType size = Converter<CollectionSizeType>::convert(ptr, total_size, true);
                        ptr += size * sizeof(T);
                        auto bptr = reinterpret_cast<T*>(ptr - size * sizeof(T));
                        return std::vector<T> (bptr, bptr + size);
                    } else {
                        auto bptr = reinterpret_cast<T*>(ptr);
                        return std::vector<T> (bptr, bptr + total_size / sizeof(T));
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
                            if (current == total_size) break;
                        }
                        return result;
                    }
                }
            }
        };

        template<typename T, size_t N>
        class Converter<std::array<T, N>> {
        public:

            static const bool special = false;

            static Bytes toBytes(const std::array<T, N>& value, bool delimiter = false) {
                return Converter<std::vector<T>>::toBytes(std::vector<T>(value.cbegin(), value.cend()), delimiter);
            }

            static std::array<T, N> convert(unsigned char * &ptr, size_t& total_size, bool delimiter = false) {
                return Converter<std::vector<T>>::convert(ptr, total_size, delimiter);
            }
        };

        template<typename T>
        class Converter<std::set<T>> {
        public:

            static const bool special = true;

            static Bytes toBytes(const std::set<T>& value, bool delimiter = false) {
                return Converter<std::vector<T>>::toBytes(std::vector<T>(value.cbegin(), value.cend()), delimiter);
            }

            static std::set<T> convert(unsigned char * &ptr, size_t& total_size, bool delimiter = false) {
                auto result = Converter<std::vector<T>>::convert(ptr, total_size, delimiter);
                return std::set<T> (result.begin(), result.end());
            }
        };

        template<typename T1, typename T2>
        class Converter<std::map<T1, T2>> {
        public:

            static const bool special = true;

            using Key = std::vector<typename std::map<T1, T2>::value_type>;

            static Bytes toBytes(const std::map<T1, T2>& value, bool delimiter = false) {
                return Converter<Key>::toBytes(Key(value.cbegin(), value.cend()), delimiter);
            }

            static std::map<T1, T2> convert(unsigned char * &ptr, size_t& total_size, bool delimiter = false) {
                auto result = Converter<Key>::convert(ptr, total_size, delimiter);
                return std::map<T1, T2> (result.begin(), result.end());
            }
        };
    };

    // list of converters
    template<>
    class Bytes::Converter<const unsigned char *> {
    public:

        static const bool special = true;

        static Bytes toBytes(const unsigned char * value, bool delimiter = false) {
            if (delimiter) {
                auto result = toBytes(value, false);
                return merge(Bytes((CollectionSizeType) result.size()), result);
            } else {
                return Bytes{value};
            }
        };
    };

    template<>
    class Bytes::Converter<const char *> {
    public:

        static const bool special = true;

        static Bytes toBytes(const char * value, bool delimiter = false) {
            if (delimiter) {
                auto result = toBytes(value, false);
                return merge(Bytes((CollectionSizeType) result.size()), Bytes {value});
            } else {
                return Bytes {value};
            }
        }
    };

    template<>
    class Bytes::Converter<std::string> {
    public:

        static const bool special = true;

        static Bytes toBytes(const std::string &value, bool delimiter = false) {
            if (delimiter) {
                // attach size in front of the data
                return merge(Converter<CollectionSizeType>::toBytes((CollectionSizeType) value.size()), Bytes{value});
            } else {
                return Bytes {value};
            };
        }

        static std::string convert(unsigned char * &ptr, size_t& total_size, bool delimiter = false) {
            if (delimiter) {
                const CollectionSizeType size = Converter<CollectionSizeType>::convert(ptr, total_size, delimiter);
                ptr += size;
                total_size -= size;
                auto bptr = reinterpret_cast<char *>(ptr - size);
                return std::string {bptr, bptr + size};
            } else {
                auto bptr = reinterpret_cast<char *>(ptr);
                return std::string {bptr, bptr + total_size};
            };
        }
    };

    template<>
    class Bytes::Converter<Bytes> {
    public :

        static const bool special = true;

        static Bytes toBytes(const Bytes &bytes, bool delimiter = false) {
            if (delimiter) {
                // attach size in front of the data
                return merge(Converter<CollectionSizeType>::toBytes((CollectionSizeType) bytes.size()), bytes);
            } else {
                return bytes;
            }
        }

        static Bytes convert(unsigned char * &ptr, size_t& total_size, bool delimiter = false) {
            if (delimiter) {
                const CollectionSizeType size = Converter<CollectionSizeType >::convert(ptr, total_size, delimiter);
                ptr += size;
                total_size -= size;
                return Bytes {ptr - size, size};
            } else {
                return Bytes {ptr, total_size};
            }
        }
    };

    class Record {
    public:

        using PropertyToBytesMap = std::map<std::string, Bytes>;

        explicit Record() = default;

        template<typename T>
        Record &set(const std::string &propName, const T &value) {
            if (!propName.empty() && !isBasicInfo(propName)) {
                properties[propName] = Bytes::Converter<T>::toBytes(value);
            }
            return *this;
        }

        template<typename T>
        Record &setIfNotExists(const std::string &propName, const T &value) {
            if (properties.find(propName) == properties.cend()) {
                set(propName, value);
            }
            return *this;
        }

        const PropertyToBytesMap &getAll() const;

        const PropertyToBytesMap &getBasicInfo() const;

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

        friend struct parser::Parser;
        friend struct Generic;
        friend struct Algorithm;
        friend struct Vertex;
        friend struct Edge;

        friend class sql_parser::Record;

        Record(PropertyToBytesMap properties);

        Record(PropertyToBytesMap properties, PropertyToBytesMap basicProperties)
                : properties(std::move(properties)), basicProperties(std::move(basicProperties)) {}

        inline bool isBasicInfo(const std::string &str) const { return str.at(0) == '@'; }

        PropertyToBytesMap properties{};
        mutable PropertyToBytesMap basicProperties{};

        template<typename T>
        const Record &setBasicInfo(const std::string &propName, const T &value) const {
            if (!propName.empty() && isBasicInfo(propName)) {
                basicProperties[propName] = Bytes::Converter<T>::toBytes(value);
            }
            return *this;
        };

        template<typename T>
        const Record &setBasicInfoIfNotExists(const std::string &propName, const T &value) const {
            if (basicProperties.find(propName) == basicProperties.cend()) {
                setBasicInfo(propName, value);
            }
            return *this;
        };

        const Record &updateVersion(const Txn &txn) const;
    };

    struct RecordDescriptor {
        RecordDescriptor() = default;

        RecordDescriptor(ClassId classId, PositionId posId)
                : rid{classId, posId} {}

        RecordDescriptor(const RecordId &rid_)
                : rid{rid_} {}

        virtual ~RecordDescriptor() noexcept = default;

        RecordId rid{0, 0};

    private:
        friend struct Generic;
        friend struct Algorithm;
        unsigned int depth{0};
    };

    struct PropertyDescriptor {
        PropertyId id{0};
        std::string name{""};
        PropertyType type{PropertyType::UNDEFINED};
        bool inherited{false};
    };

    struct ClassDescriptor {
        ClassId id{0};
        std::string name{""};
        ClassId base{0};
        ClassType type{ClassType::UNDEFINED};
    };

    struct DBSchema {

        struct IndexInfo {
            PropertyId propertyId;
            bool unique;
        };

        ClassDescriptor classDescriptor;
        std::vector<PropertyDescriptor> propertyDescriptors;
        std::vector<IndexInfo> indexInfos;
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

    inline bool operator<(const RecordId &lhs, const RecordId &rhs) {
        return (lhs.first == rhs.first) ? lhs.second < rhs.second : lhs.first < rhs.first;
    }

    inline bool operator==(const RecordId &lhs, const RecordId &rhs) {
        return (lhs.first == rhs.first) && (lhs.second == rhs.second);
    }

    inline bool operator!=(const RecordId &lhs, const RecordId &rhs) {
        return !operator==(lhs, rhs);
    }

    inline bool operator<(const RecordDescriptor &lhs, const RecordDescriptor &rhs) {
        return lhs.rid < rhs.rid;
    }

    inline bool operator==(const RecordDescriptor &lhs, const RecordDescriptor &rhs) {
        return lhs.rid == rhs.rid;
    }

    inline bool operator!=(const RecordDescriptor &lhs, const RecordDescriptor &rhs) {
        return !operator==(lhs, rhs);
    }

}

inline std::string rid2str(const RecordId &rid) {
    std::stringstream ss{};
    ss << std::to_string(rid.first) << ":" << std::to_string(rid.second);
    return ss.str();
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

#endif
