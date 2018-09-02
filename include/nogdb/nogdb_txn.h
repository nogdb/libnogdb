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

#ifndef __NOGDB_TXN_H_INCLUDED_
#define __NOGDB_TXN_H_INCLUDED_

#include "nogdb_context.h"

namespace nogdb {

    class Txn {
    public:
        friend struct Compare;
        friend struct Algorithm;
        friend struct Generic;
        friend struct DB;
        friend struct Class;
        friend struct Property;
        friend struct Vertex;
        friend struct Edge;
        friend struct Traverse;

        friend class ResultSetCursor;

        friend class validate::Validator;

        friend class schema::SchemaInterface;

        friend class relation::GraphInterface;

        friend class index::IndexInterface;

        friend class adapter::datarecord::DataRecords;

        enum Mode { READ_ONLY, READ_WRITE };

        Txn(Context &ctx, Mode mode);

        ~Txn() noexcept;

        Txn(Txn &&txn) noexcept;

        Txn &operator=(Txn &&txn) noexcept;

        void commit();

        void rollback() noexcept;

        Mode getTxnMode() const { return _txnMode; }

        bool isCompleted() const { return _completed; }

    private:
        Context &_txnCtx;
        storage_engine::LMDBTxn *_txnBase;
        adapter::metadata::DBInfoAccess *_dbInfo;
        adapter::schema::ClassAccess *_class;
        adapter::schema::PropertyAccess *_property;
        adapter::schema::IndexAccess *_index;
        schema::SchemaInterface *_iSchema;
        index::IndexInterface *_iIndex;
        relation::GraphInterface *_iGraph;

        Mode _txnMode;
        bool _completed; // throw error if working with isCompleted = true
    };

}

#endif
