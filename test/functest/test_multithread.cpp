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

#include <thread>
#include <mutex>
#include <unistd.h>
#include "functest.h"
#include "test_prepare.h"

std::mutex wlock;

void do_job(unsigned int type) {
  if (type == 11) { // create more + commit
    std::unique_lock<std::mutex> _(wlock);
    try {
      nogdb::Transaction txn{*ctx, nogdb::TxnMode::READ_WRITE};
      auto v1 = txn.addVertex("islands", nogdb::Record{}.set("name", "Koh C"));
      auto v2 = txn.addVertex("islands", nogdb::Record{}.set("name", "Koh D"));
      txn.addEdge("bridge", v1, v2, nogdb::Record{}.set("name", "bridge 34"));
      auto res = txn.find("islands", nogdb::Condition("name").eq("Koh D"));
      assert(!res.empty());
      auto resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 34"));
      assert(!res.empty());
      txn.commit();
    } catch (const nogdb::Error &ex) {
      std::cout << "Error: " << ex.what() << std::endl;
      assert(false);
    }
  } else if (type == 21) { // delete some + commit
    std::unique_lock<std::mutex> _(wlock);
    try {
      nogdb::Transaction txn{*ctx, nogdb::TxnMode::READ_WRITE};
      auto res = txn.find("islands", nogdb::Condition("name").eq("Koh C"));
      for (const auto &r: res) txn.remove(r.descriptor);
      res = txn.find("islands", nogdb::Condition("name").eq("Koh C"));
      assert(res.empty());
      auto resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 13"));
      assert(res.empty());
      resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 23"));
      assert(res.empty());
      txn.commit();
    } catch (const nogdb::Error &ex) {
      std::cout << "Error: " << ex.what() << std::endl;
      assert(false);
    }
  } else if (type == 31) { // modify some + commit
    std::unique_lock<std::mutex> _(wlock);
    try {
      nogdb::Transaction txn{*ctx, nogdb::TxnMode::READ_WRITE};
      auto resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 12"));
      auto resV = txn.find("islands", nogdb::Condition("name").eq("Koh A"));
      txn.updateDst(resE[0].descriptor, resV[0].descriptor);
      auto res = txn.findInEdge(resV[0].descriptor);
      ASSERT_SIZE(res, 2);
      txn.commit();
    } catch (const nogdb::Error &ex) {
      std::cout << "Error: " << ex.what() << std::endl;
      assert(false);
    }
  } else if (type == 10) { // create more + rollback
    std::unique_lock<std::mutex> _(wlock);
    try {
      nogdb::Transaction txn{*ctx, nogdb::TxnMode::READ_WRITE};
      auto v1 = txn.addVertex("islands", nogdb::Record{}.set("name", "Koh C"));
      auto v2 = txn.addVertex("islands", nogdb::Record{}.set("name", "Koh D"));
      txn.addEdge("bridge", v1, v2, nogdb::Record{}.set("name", "bridge 34"));
      auto res = txn.find("islands", nogdb::Condition("name").eq("Koh D"));
      assert(!res.empty());
      auto resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 34"));
      assert(!res.empty());
      txn.rollback();
    } catch (const nogdb::Error &ex) {
      std::cout << "Error: " << ex.what() << std::endl;
      assert(false);
    }
  } else if (type == 20) { // delete some + rollback
    std::unique_lock<std::mutex> _(wlock);
    try {
      nogdb::Transaction txn{*ctx, nogdb::TxnMode::READ_WRITE};
      auto res = txn.find("islands", nogdb::Condition("name").eq("Koh C"));
      for (const auto &r: res) txn.remove(r.descriptor);
      res = txn.find("islands", nogdb::Condition("name").eq("Koh C"));
      assert(res.empty());
      auto resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 13"));
      assert(res.empty());
      resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 23"));
      assert(res.empty());
      txn.rollback();
    } catch (const nogdb::Error &ex) {
      std::cout << "Error: " << ex.what() << std::endl;
      assert(false);
    }
  } else if (type == 30) { // modify some + rollback
    std::unique_lock<std::mutex> _(wlock);
    try {
      nogdb::Transaction txn{*ctx, nogdb::TxnMode::READ_WRITE};
      auto resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 12"));
      auto resV = txn.find("islands", nogdb::Condition("name").eq("Koh A"));
      txn.updateDst(resE[0].descriptor, resV[0].descriptor);
      auto res = txn.findInEdge(resV[0].descriptor);
      ASSERT_SIZE(res, 2);
      txn.rollback();
    } catch (const nogdb::Error &ex) {
      std::cout << "Error: " << ex.what() << std::endl;
      assert(false);
    }
  } else if (type == 0) { // read only new version
    try {
      nogdb::Transaction txn{*ctx, nogdb::TxnMode::READ_ONLY};
      auto resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 13"));
      assert(resE.empty());
      resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 23"));
      assert(resE.empty());
      resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 12"));
      assert(!resE.empty());
      resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 21"));
      assert(!resE.empty());

      auto resV = txn.find("islands", nogdb::Condition("name").eq("Koh A"));
      auto res = txn.findOutEdge(resV[0].descriptor);
      ASSERT_SIZE(res, 1);
      resV = txn.find("islands", nogdb::Condition("name").eq("Koh B"));
      res = txn.findOutEdge(resV[0].descriptor);
      ASSERT_SIZE(res, 1);
    } catch (const nogdb::Error &ex) {
      std::cout << "Error: " << ex.what() << std::endl;
      assert(false);
    }
  } else if (type == 1) { // read only old version
    try {
      nogdb::Transaction txn{*ctx, nogdb::TxnMode::READ_ONLY};
      auto resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 13"));
      assert(!resE.empty());
      resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 23"));
      assert(!resE.empty());
      resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 12"));
      assert(!resE.empty());
      resE = txn.find("bridge", nogdb::Condition("name").eq("bridge 21"));
      assert(!resE.empty());

      auto resV = txn.find("islands", nogdb::Condition("name").eq("Koh A"));
      auto res = txn.findOutEdge(resV[0].descriptor);
      ASSERT_SIZE(res, 2);
      resV = txn.find("islands", nogdb::Condition("name").eq("Koh B"));
      res = txn.findOutEdge(resV[0].descriptor);
      ASSERT_SIZE(res, 2);
      resV = txn.find("islands", nogdb::Condition("name").eq("Koh C"));
      res = txn.findInEdge(resV[0].descriptor);
      ASSERT_SIZE(res, 2);
    } catch (const nogdb::Error &ex) {
      std::cout << "Error: " << ex.what() << std::endl;
      assert(false);
    }
  }
}

void test_txn_multithreads() {
  init_vertex_island();
  init_edge_bridge();

  try {
    nogdb::Transaction txn{*ctx, nogdb::TxnMode::READ_WRITE};

    auto v1 = txn.addVertex("islands", nogdb::Record{}.set("name", "Koh A"));
    auto v2 = txn.addVertex("islands", nogdb::Record{}.set("name", "Koh B"));
    auto v3 = txn.addVertex("islands", nogdb::Record{}.set("name", "Koh C"));
    txn.addEdge("bridge", v1, v2, nogdb::Record{}.set("name", "bridge 12"));
    txn.addEdge("bridge", v2, v1, nogdb::Record{}.set("name", "bridge 21"));
    txn.addEdge("bridge", v2, v3, nogdb::Record{}.set("name", "bridge 23"));
    txn.addEdge("bridge", v1, v3, nogdb::Record{}.set("name", "bridge 13"));

    txn.commit();
  } catch (const nogdb::Error &ex) {
    std::cout << "Error: " << ex.what() << std::endl;
    assert(false);
  }

  std::thread t1(do_job, 1);
  std::thread t2(do_job, 10);
  std::thread t3(do_job, 20);
  std::thread t4(do_job, 30);
  usleep(1000000);
  std::thread t5(do_job, 1);
  std::thread tt0(do_job, 1);
  std::thread tt1(do_job, 11);
  std::thread tt2(do_job, 21);
  std::thread tt3(do_job, 31);
  usleep(1000000);
  std::thread tt4(do_job, 0);

  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();
  tt0.join();
  tt1.join();
  tt2.join();
  tt3.join();
  tt4.join();

  destroy_edge_bridge();
  destroy_vertex_island();
}



