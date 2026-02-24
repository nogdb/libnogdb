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

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#include "nogdb/nogdb.h"

static const char* BENCH_DB_PATH = "/tmp/nogdb_bench_db";

// ---------------------------------------------------------------------------
// Timer helpers
// ---------------------------------------------------------------------------

using Clock = std::chrono::high_resolution_clock;
using Ns = std::chrono::nanoseconds;

struct BenchResult {
    std::string name;
    unsigned long iterations;
    double totalMs;
    double perIterUs;
};

static double nsToMs(long long ns) { return static_cast<double>(ns) / 1e6; }
static double nsToUs(long long ns) { return static_cast<double>(ns) / 1e3; }

static BenchResult runBench(const std::string& name,
    unsigned long iterations,
    std::function<void()> fn)
{
    auto t0 = Clock::now();
    for (unsigned long i = 0; i < iterations; ++i) {
        fn();
    }
    auto t1 = Clock::now();
    auto totalNs = std::chrono::duration_cast<Ns>(t1 - t0).count();
    BenchResult r;
    r.name = name;
    r.iterations = iterations;
    r.totalMs = nsToMs(totalNs);
    r.perIterUs = nsToUs(totalNs / static_cast<long long>(iterations));
    return r;
}

static void printResult(const BenchResult& r)
{
    std::printf("  %-55s  %6lu iters  %8.2f ms total  %8.3f us/iter\n",
        r.name.c_str(), r.iterations, r.totalMs, r.perIterUs);
}

// ---------------------------------------------------------------------------
// Setup / teardown helpers
// ---------------------------------------------------------------------------

static void removeDBDir(const char* path)
{
    std::string cmd = std::string("rm -rf ") + path;
    (void)std::system(cmd.c_str());
}

static nogdb::Context* createFreshContext()
{
    removeDBDir(BENCH_DB_PATH);
    nogdb::ContextInitializer(BENCH_DB_PATH)
        .setMaxDBSize(256UL * 1024 * 1024)
        .init();
    return new nogdb::Context(BENCH_DB_PATH);
}

// ---------------------------------------------------------------------------
// Benchmarks
// ---------------------------------------------------------------------------

static void bench_vertex_insert(nogdb::Context& ctx, std::vector<BenchResult>& results)
{
    // Schema setup
    {
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("Person", nogdb::ClassType::VERTEX);
        txn.addProperty("Person", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("Person", "age", nogdb::PropertyType::INTEGER);
        txn.commit();
    }

    const unsigned long N = 10000;
    unsigned long counter = 0;
    auto r = runBench("vertex insert (10k, individual txn each)", N, [&] {
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addVertex("Person",
            nogdb::Record {}
                .set("name", std::string("user_") + std::to_string(counter++))
                .set("age", int32_t(counter % 100)));
        txn.commit();
    });
    results.push_back(r);

    // Batch insert
    unsigned long batch_counter = 0;
    const unsigned long BATCH = 1000;
    const unsigned long REPS = 10;
    auto r2 = runBench("vertex insert (10k, 1000-per-txn batch)", REPS, [&] {
        auto txn = ctx.beginBatchTxn();
        for (unsigned long i = 0; i < BATCH; ++i) {
            txn.addVertex("Person",
                nogdb::Record {}
                    .set("name", std::string("batch_") + std::to_string(batch_counter++))
                    .set("age", int32_t(batch_counter % 100)));
        }
        txn.commit();
    });
    r2.iterations = REPS * BATCH;
    r2.perIterUs = r2.totalMs * 1e3 / static_cast<double>(r2.iterations);
    r2.name = "vertex insert (10k, 1000-per-txn batch)";
    results.push_back(r2);
}

static void bench_edge_insert(nogdb::Context& ctx, std::vector<BenchResult>& results)
{
    {
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("Friend", nogdb::ClassType::EDGE);
        txn.addProperty("Friend", "weight", nogdb::PropertyType::REAL);
        txn.commit();
    }

    // Fetch existing vertices
    std::vector<nogdb::RecordDescriptor> vertices;
    {
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
        auto cursor = txn.find("Person").getCursor();
        while (cursor.next()) {
            vertices.push_back((*cursor).descriptor);
        }
        txn.rollback();
    }
    if (vertices.size() < 2) {
        return;
    }

    const unsigned long N = std::min(static_cast<unsigned long>(vertices.size() / 2), 1000UL);
    unsigned long ei = 0;
    auto r = runBench("edge insert (N edges, individual txn)", N, [&] {
        auto src = vertices[ei % vertices.size()];
        auto dst = vertices[(ei + 1) % vertices.size()];
        ++ei;
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addEdge("Friend", src, dst, nogdb::Record {}.set("weight", 1.0));
        txn.commit();
    });
    results.push_back(r);
}

static void bench_find_full_scan(nogdb::Context& ctx, std::vector<BenchResult>& results)
{
    const unsigned long N = 100;
    auto r = runBench("find().get() full scan (Person)", N, [&] {
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
        auto rs = txn.find("Person").get();
        (void)rs.size();
        txn.rollback();
    });
    results.push_back(r);

    auto r2 = runBench("find().getCursor() full scan (Person)", N, [&] {
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
        auto cursor = txn.find("Person").getCursor();
        unsigned long count = 0;
        while (cursor.next()) { ++count; }
        txn.rollback();
    });
    results.push_back(r2);
}

static void bench_find_condition(nogdb::Context& ctx, std::vector<BenchResult>& results)
{
    const unsigned long N = 200;
    auto r = runBench("find().where(Condition) no index", N, [&] {
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
        auto rs = txn.find("Person")
                      .where(nogdb::Condition("age").eq(int32_t { 42 }))
                      .get();
        (void)rs.size();
        txn.rollback();
    });
    results.push_back(r);
}

static void bench_find_indexed(nogdb::Context& ctx, std::vector<BenchResult>& results)
{
    {
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addIndex("Person", "age", false);
        txn.commit();
    }

    const unsigned long N = 500;
    auto r = runBench("find().indexed().where(Condition) with index", N, [&] {
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
        auto rs = txn.find("Person")
                      .indexed()
                      .where(nogdb::Condition("age").eq(int32_t { 42 }))
                      .get();
        (void)rs.size();
        txn.rollback();
    });
    results.push_back(r);

    {
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropIndex("Person", "age");
        txn.commit();
    }
}

static void bench_traversal(nogdb::Context& ctx, std::vector<BenchResult>& results)
{
    std::vector<nogdb::RecordDescriptor> vertices;
    {
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
        auto cursor = txn.find("Person").getCursor();
        while (cursor.next()) {
            vertices.push_back((*cursor).descriptor);
            if (vertices.size() >= 10) break;
        }
        txn.rollback();
    }
    if (vertices.empty()) return;

    const unsigned long N = 200;
    unsigned long vi = 0;
    auto r = runBench("traverseOut BFS depth 1-3", N, [&] {
        auto src = vertices[vi % vertices.size()];
        ++vi;
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
        auto rs = txn.traverseOut(src).depth(1, 3).get();
        (void)rs.size();
        txn.rollback();
    });
    results.push_back(r);

    unsigned long vi2 = 0;
    auto r2 = runBench("traverseOutDFS depth 1-3", N, [&] {
        auto src = vertices[vi2 % vertices.size()];
        ++vi2;
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
        auto rs = txn.traverseOutDFS(src).depth(1, 3).get();
        (void)rs.size();
        txn.rollback();
    });
    results.push_back(r2);
}

static void bench_shortest_path(nogdb::Context& ctx, std::vector<BenchResult>& results)
{
    std::vector<nogdb::RecordDescriptor> vertices;
    {
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
        auto cursor = txn.find("Person").getCursor();
        while (cursor.next()) {
            vertices.push_back((*cursor).descriptor);
            if (vertices.size() >= 20) break;
        }
        txn.rollback();
    }
    if (vertices.size() < 2) return;

    const unsigned long N = 100;
    unsigned long vi = 0;
    auto r = runBench("shortestPath BFS (random src/dst pairs)", N, [&] {
        auto src = vertices[vi % vertices.size()];
        auto dst = vertices[(vi + 7) % vertices.size()];
        ++vi;
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
        auto rs = txn.shortestPath(src, dst).get();
        (void)rs.size();
        txn.rollback();
    });
    results.push_back(r);

    unsigned long vi2 = 0;
    auto r2 = runBench("shortestPath Dijkstra withWeight (random src/dst pairs)", N, [&] {
        auto src = vertices[vi2 % vertices.size()];
        auto dst = vertices[(vi2 + 7) % vertices.size()];
        ++vi2;
        auto txn = ctx.beginTxn(nogdb::TxnMode::READ_ONLY);
        auto rs = txn.shortestPath(src, dst).withWeight("weight").get();
        (void)rs.size();
        txn.rollback();
    });
    results.push_back(r2);
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main()
{
    std::printf("NogDB Micro-Benchmark Suite\n");
    std::printf("===========================\n\n");

    nogdb::Context* ctx = nullptr;
    std::vector<BenchResult> results;

    try {
        ctx = createFreshContext();

        std::printf("[ Insert ]\n");
        bench_vertex_insert(*ctx, results);
        bench_edge_insert(*ctx, results);
        for (const auto& r : results) printResult(r);
        results.clear();

        std::printf("\n[ Find / Query ]\n");
        bench_find_full_scan(*ctx, results);
        bench_find_condition(*ctx, results);
        bench_find_indexed(*ctx, results);
        for (const auto& r : results) printResult(r);
        results.clear();

        std::printf("\n[ Traversal ]\n");
        bench_traversal(*ctx, results);
        bench_shortest_path(*ctx, results);
        for (const auto& r : results) printResult(r);
        results.clear();

    } catch (const nogdb::Error& e) {
        std::fprintf(stderr, "nogdb::Error: %s (code %d)\n", e.what(), e.code());
        delete ctx;
        removeDBDir(BENCH_DB_PATH);
        return 1;
    } catch (const std::exception& e) {
        std::fprintf(stderr, "std::exception: %s\n", e.what());
        delete ctx;
        removeDBDir(BENCH_DB_PATH);
        return 1;
    }

    delete ctx;
    removeDBDir(BENCH_DB_PATH);
    std::printf("\nDone.\n");
    return 0;
}
