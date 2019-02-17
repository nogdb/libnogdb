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

#include <map>
#include <vector>
#include <set>
#include <utility>
#include <memory>
#include <unordered_set>

#include "nogdb_errors.h"
#include "nogdb_types.h"
#include "nogdb_sql.h"

namespace nogdb {

  struct ContextSetting {
    unsigned int _maxDB{};
    unsigned long _maxDBSize{};
    bool _enableVersion{};
  };

  class Context;

  class ContextInitializer {
  public:

    ContextInitializer(const std::string& dbPath);

    ~ContextInitializer() = default;

    ContextInitializer& setMaxDB(unsigned int maxDbNum) noexcept;

    ContextInitializer& setMaxDBSize(unsigned long maxDbSize) noexcept;

    ContextInitializer& enableVersion() noexcept;

    Context init();

  private:
    std::string _dbPath{};
    ContextSetting _settings{};

  };

  class Context {
  public:

    friend class ContextInitializer;

    friend class Transaction;

    Context() = default;

    ~Context() noexcept;

    Context(const std::string &dbPath);

    Context(const Context &ctx);

    Context(Context &&ctx) noexcept;

    Context &operator=(const Context &ctx);

    Context &operator=(Context &&ctx) noexcept;

    std::string getDBPath() const { return _dbPath; }

    unsigned int getMaxDB() const { return _settings._maxDB; }

    unsigned long getMaxDBSize() const { return _settings._maxDBSize; }

    bool isEnableVersion() const { return _settings._enableVersion; }

    Transaction beginTxn(const TxnMode &txnMode = TxnMode::READ_WRITE);

  private:

    Context(const std::string &dbPath, const ContextSetting& settings);

    std::string _dbPath{};
    ContextSetting _settings{};
    storage_engine::LMDBEnv* _envHandler;

    struct LMDBInstance {
      storage_engine::LMDBEnv* _handler;
      unsigned int _refCount;
    };

    static std::unordered_map<std::string, LMDBInstance> _underlying;

  };



  class Transaction {
  public:
    friend class ResultSetCursor;

    friend class compare::RecordCompare;

    friend class validate::Validator;

    friend class schema::SchemaInterface;

    friend class relation::GraphInterface;

    friend class index::IndexInterface;

    friend class datarecord::DataRecordInterface;

    friend class algorithm::GraphTraversal;

    Transaction(Context &ctx, const TxnMode& mode);

    ~Transaction() noexcept;

    Transaction(const Transaction &txn) = delete;

    Transaction(Transaction &&txn) noexcept;

    Transaction &operator=(const Transaction &txn) = delete;

    Transaction &operator=(Transaction &&txn) noexcept;

    void commit();

    void rollback() noexcept;

    TxnMode getTxnMode() const { return _txnMode; }

    bool isCompleted() const { return _txnBase == nullptr; }

    const ClassDescriptor addClass(const std::string &className, ClassType type);

    const ClassDescriptor addSubClassOf(const std::string &superClass, const std::string &className);

    void dropClass(const std::string &className);

    void renameClass(const std::string &oldClassName, const std::string &newClassName);

    const PropertyDescriptor addProperty(const std::string &className,
                                         const std::string &propertyName,
                                         PropertyType type);

    void renameProperty(const std::string &className,
                        const std::string &oldPropertyName,
                        const std::string &newPropertyName);

    void dropProperty(const std::string &className, const std::string &propertyName);

    const IndexDescriptor addIndex(const std::string &className,
                                   const std::string &propertyName,
                                   bool isUnique = false);

    void dropIndex(const std::string &className, const std::string &propertyName);

    const DbInfo getDbInfo() const;

    const std::vector<ClassDescriptor> getClasses() const;

    const std::vector<PropertyDescriptor> getProperties(const std::string &className) const;

    const std::vector<PropertyDescriptor> getProperties(const ClassDescriptor &classDescriptor) const;

    const std::vector<IndexDescriptor> getIndexes(const ClassDescriptor &classDescriptor) const;

    const ClassDescriptor getClass(const std::string &className) const;

    const ClassDescriptor getClass(const ClassId &classId) const;

    const PropertyDescriptor getProperty(const std::string &className, const std::string &propertyName) const;

    const IndexDescriptor getIndex(const std::string &className, const std::string &propertyName) const;

    Record fetchRecord(const RecordDescriptor &recordDescriptor) const;

    const RecordDescriptor addVertex(const std::string &className, const Record &record = Record{});

    const RecordDescriptor addEdge(const std::string &className,
                                   const RecordDescriptor &srcVertexRecordDescriptor,
                                   const RecordDescriptor &dstVertexRecordDescriptor,
                                   const Record &record = Record{});

    void update(const RecordDescriptor &recordDescriptor, const Record &record);

    void updateSrc(const RecordDescriptor &recordDescriptor, const RecordDescriptor &newSrcVertexRecordDescriptor);

    void updateDst(const RecordDescriptor &recordDescriptor, const RecordDescriptor &newDstVertexRecordDescriptor);

    void remove(const RecordDescriptor &recordDescriptor);

    void removeAll(const std::string &className);

    FindOperationBuilder find(const std::string &className) const;

    FindOperationBuilder findSubClassOf(const std::string &className) const;

    FindEdgeOperationBuilder findInEdge(const RecordDescriptor &recordDescriptor) const;

    FindEdgeOperationBuilder findOutEdge(const RecordDescriptor &recordDescriptor) const;

    FindEdgeOperationBuilder findEdge(const RecordDescriptor &recordDescriptor) const;

    Result fetchSrc(const RecordDescriptor &recordDescriptor) const;

    Result fetchDst(const RecordDescriptor &recordDescriptor) const;

    ResultSet fetchSrcDst(const RecordDescriptor &recordDescriptor) const;

    TraverseOperationBuilder traverseIn(const RecordDescriptor &recordDescriptor) const;

    TraverseOperationBuilder traverseOut(const RecordDescriptor &recordDescriptor) const;

    TraverseOperationBuilder traverse(const RecordDescriptor &recordDescriptor) const;

    ShortestPathOperationBuilder shortestPath(const RecordDescriptor &srcVertexRecordDescriptor,
                                              const RecordDescriptor &dstVertexRecordDescriptor) const;

  private:

    friend class FindOperationBuilder;

    friend class FindEdgeOperationBuilder;

    friend class TraverseOperationBuilder;

    friend class ShortestPathOperationBuilder;

    class Adapter {
    public:
      Adapter();

      Adapter(const storage_engine::LMDBTxn *txn);

      ~Adapter() noexcept;

      Adapter(Adapter &&other) noexcept = delete;

      Adapter& operator=(Adapter &&other) noexcept = delete;

      adapter::metadata::DBInfoAccess *dbInfo() const { return _dbInfo; }

      adapter::schema::ClassAccess *dbClass() const { return _class; }

      adapter::schema::PropertyAccess *dbProperty() const { return _property; }

      adapter::schema::IndexAccess *dbIndex() const { return _index; }

    private:
      adapter::metadata::DBInfoAccess *_dbInfo;
      adapter::schema::ClassAccess *_class;
      adapter::schema::PropertyAccess *_property;
      adapter::schema::IndexAccess *_index;
    };

    class Interface {
    public:
      Interface();

      Interface(const Transaction *txn);

      ~Interface() noexcept;

      Interface(Interface &&other) noexcept = delete;

      Interface& operator=(Interface &&other) noexcept = delete;

      schema::SchemaInterface *schema() const { return _schema; }

      index::IndexInterface *index() const { return _index; }

      relation::GraphInterface *graph() const { return _graph; }

      datarecord::DataRecordInterface *record() const { return _record; }

      void destroy();

    private:
      const Transaction* _txn;
      schema::SchemaInterface *_schema;
      datarecord::DataRecordInterface *_record;
      relation::GraphInterface *_graph;
      index::IndexInterface *_index;
    };

    TxnMode _txnMode;
    const Context *_txnCtx;
    storage_engine::LMDBTxn *_txnBase;
    Adapter *_adapter;
    Interface *_interface;

    std::unordered_set<RecordId, RecordIdHash> _updatedRecords{};

  };

}
