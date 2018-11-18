/*
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

#pragma once

#include "nogdb_context.h"

namespace nogdb {

  class Txn {
  public:
    friend struct DB;
    friend struct Class;
    friend struct Property;
    friend struct Vertex;
    friend struct Edge;
    friend struct Traverse;

    friend class ResultSetCursor;

    friend class compare::RecordCompare;

    friend class validate::Validator;

    friend class schema::SchemaInterface;

    friend class relation::GraphInterface;

    friend class index::IndexInterface;

    friend class datarecord::DataRecordInterface;

    friend class algorithm::GraphTraversal;

    enum Mode {
      READ_ONLY, READ_WRITE
    };

    Txn(Context &ctx, Mode mode);

    ~Txn() noexcept;

    Txn(const Txn &txn) = delete;

    Txn(Txn &&txn) noexcept;

    Txn &operator=(const Txn &txn) = delete;

    Txn &operator=(Txn &&txn) noexcept;

    void commit();

    void rollback() noexcept;

    Mode getTxnMode() const { return _txnMode; }

    bool isCompleted() const { return _txnBase == nullptr; }

  private:

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

      Interface(const Txn *txn);

      ~Interface() noexcept;

      Interface(Interface &&other) noexcept = delete;

      Interface& operator=(Interface &&other) noexcept = delete;

      schema::SchemaInterface *schema() const { return _schema; }

      index::IndexInterface *index() const { return _index; }

      relation::GraphInterface *graph() const { return _graph; }

      datarecord::DataRecordInterface *record() const { return _record; }

      void destroy();

    private:
      const Txn* _txn;
      schema::SchemaInterface *_schema;
      datarecord::DataRecordInterface *_record;
      relation::GraphInterface *_graph;
      index::IndexInterface *_index;
    };

    Mode _txnMode;
    const Context *_txnCtx;
    storage_engine::LMDBTxn *_txnBase;
    Adapter *_adapter;
    Interface *_interface;

  };

}
