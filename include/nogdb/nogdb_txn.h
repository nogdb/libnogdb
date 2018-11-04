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

    ~Txn() noexcept = default;

    Txn(const Txn &txn);

    Txn(Txn &&txn) noexcept;

    Txn &operator=(const Txn &txn);

    Txn &operator=(Txn &&txn) noexcept;

    void commit() const;

    void rollback() const noexcept;

    Mode getTxnMode() const { return _txnMode; }

    bool isCompleted() const { return _completed; }

  private:
    Context &_txnCtx;
    std::shared_ptr<storage_engine::LMDBTxn> _txnBase;
    std::shared_ptr<adapter::metadata::DBInfoAccess> _dbInfo;
    std::shared_ptr<adapter::schema::ClassAccess> _class;
    std::shared_ptr<adapter::schema::PropertyAccess> _property;
    std::shared_ptr<adapter::schema::IndexAccess> _index;
    std::shared_ptr<schema::SchemaInterface> _iSchema;
    std::shared_ptr<index::IndexInterface> _iIndex;
    std::shared_ptr<relation::GraphInterface> _iGraph;
    std::shared_ptr<datarecord::DataRecordInterface> _iRecord;

    Mode _txnMode;
    mutable bool _completed; // throw error if working with isCompleted = true
  };

}
