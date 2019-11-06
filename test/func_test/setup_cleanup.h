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

#include "func_test.h"

inline void init_vertex_book()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("books", nogdb::ClassType::VERTEX);
        txn.addProperty("books", "title", nogdb::PropertyType::TEXT);
        txn.addProperty("books", "words", nogdb::PropertyType::UNSIGNED_BIGINT);
        txn.addProperty("books", "pages", nogdb::PropertyType::INTEGER);
        txn.addProperty("books", "price", nogdb::PropertyType::REAL);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_book()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("books");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_person()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("persons", nogdb::ClassType::VERTEX);
        txn.addProperty("persons", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("persons", "age", nogdb::PropertyType::INTEGER);
        txn.addProperty("persons", "address", nogdb::PropertyType::TEXT);
        txn.addProperty("persons", "salary", nogdb::PropertyType::REAL);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_person()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("persons");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_author()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("authors", nogdb::ClassType::EDGE);
        txn.addProperty("authors", "profit", nogdb::PropertyType::REAL);
        txn.addProperty("authors", "time_used", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_author()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("authors");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_teachers()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("teachers", nogdb::ClassType::VERTEX);
        txn.addProperty("teachers", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("teachers", "age", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addProperty("teachers", "salary", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addProperty("teachers", "level", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_teachers()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("teachers");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_students()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("students", nogdb::ClassType::VERTEX);
        txn.addProperty("students", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("students", "age", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addProperty("students", "grade", nogdb::PropertyType::REAL);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_students()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("students");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_departments()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("departments", nogdb::ClassType::VERTEX);
        txn.addProperty("departments", "name", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_departments()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("departments");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_subjects()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("subjects", nogdb::ClassType::VERTEX);
        txn.addProperty("subjects", "name", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_subjects()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("subjects");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_teach()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("teach", nogdb::ClassType::EDGE);
        txn.addProperty("teach", "semester", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_teach()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("teach");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_enrol()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("enrol", nogdb::ClassType::EDGE);
        txn.addProperty("enrol", "semester", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_enrol()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("enrol");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_know()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("know", nogdb::ClassType::EDGE);
        txn.addProperty("know", "relationship", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_know()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("know");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_workfor()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("workfor", nogdb::ClassType::EDGE);
        txn.addProperty("workfor", "position", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_workfor()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("workfor");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_belongto()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("belongto", nogdb::ClassType::EDGE);
        txn.addProperty("belongto", "null", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_belongto()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("belongto");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_folders()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("folders", nogdb::ClassType::VERTEX);
        txn.addProperty("folders", "name", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_folders()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("folders");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_files()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("files", nogdb::ClassType::VERTEX);
        txn.addProperty("files", "name", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_files()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("files");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_link()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("link", nogdb::ClassType::EDGE);
        txn.addProperty("link", "null", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_link()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("link");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_symbolic()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("symbolic", nogdb::ClassType::EDGE);
        txn.addProperty("symbolic", "null", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_symbolic()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("symbolic");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_mountain()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("mountains", nogdb::ClassType::VERTEX);
        txn.addProperty("mountains", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("mountains", "temperature", nogdb::PropertyType::INTEGER);
        txn.addProperty("mountains", "height", nogdb::PropertyType::UNSIGNED_BIGINT);
        txn.addProperty("mountains", "rating", nogdb::PropertyType::REAL);
        txn.addProperty("mountains", "coordinates", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_mountain()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("mountains");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_location()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("locations", nogdb::ClassType::VERTEX);
        txn.addProperty("locations", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("locations", "temperature", nogdb::PropertyType::INTEGER);
        txn.addProperty("locations", "postcode", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addProperty("locations", "price", nogdb::PropertyType::BIGINT);
        txn.addProperty("locations", "population", nogdb::PropertyType::UNSIGNED_BIGINT);
        txn.addProperty("locations", "rating", nogdb::PropertyType::REAL);
        txn.addProperty("locations", "coordinates", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_location()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("locations");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_street()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("street", nogdb::ClassType::EDGE);
        txn.addProperty("street", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("street", "temperature", nogdb::PropertyType::INTEGER);
        txn.addProperty("street", "capacity", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addProperty("street", "distance", nogdb::PropertyType::REAL);
        txn.addProperty("street", "coordinates", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_street()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("street");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_highway()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("highway", nogdb::ClassType::EDGE);
        txn.addProperty("highway", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("highway", "temperature", nogdb::PropertyType::INTEGER);
        txn.addProperty("highway", "capacity", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addProperty("highway", "distance", nogdb::PropertyType::REAL);
        txn.addProperty("highway", "coordinates", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_highway()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("highway");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_railway()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("railway", nogdb::ClassType::EDGE);
        txn.addProperty("railway", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("railway", "temperature", nogdb::PropertyType::INTEGER);
        txn.addProperty("railway", "distance", nogdb::PropertyType::REAL);
        txn.addProperty("railway", "coordinates", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_railway()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("railway");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_country()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("country", nogdb::ClassType::VERTEX);
        txn.addProperty("country", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("country", "population", nogdb::PropertyType::UNSIGNED_BIGINT);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_country()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("country");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_path()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("path", nogdb::ClassType::EDGE);
        txn.addProperty("path", "distance", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_path()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("path");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_island()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("islands", nogdb::ClassType::VERTEX);
        txn.addProperty("islands", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("islands", "area", nogdb::PropertyType::REAL);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_island()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("islands");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_city()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("cities", nogdb::ClassType::VERTEX);
        txn.addProperty("cities", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("cities", "area", nogdb::PropertyType::REAL);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_city()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("cities");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_bridge()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("bridge", nogdb::ClassType::EDGE);
        txn.addProperty("bridge", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("bridge", "length", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_bridge()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("bridge");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_flight()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("flight", nogdb::ClassType::EDGE);
        txn.addProperty("flight", "name", nogdb::PropertyType::TEXT);
        txn.addProperty("flight", "distance", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_flight()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("flight");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_index_test()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.addClass("index_test", nogdb::ClassType::VERTEX);
        txn.addProperty("index_test", "index_text", nogdb::PropertyType::TEXT);
        txn.addProperty("index_test", "index_tinyint_u", nogdb::PropertyType::UNSIGNED_TINYINT);
        txn.addProperty("index_test", "index_tinyint", nogdb::PropertyType::TINYINT);
        txn.addProperty("index_test", "index_smallint_u", nogdb::PropertyType::UNSIGNED_SMALLINT);
        txn.addProperty("index_test", "index_smallint", nogdb::PropertyType::SMALLINT);
        txn.addProperty("index_test", "index_int_u", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.addProperty("index_test", "index_int", nogdb::PropertyType::INTEGER);
        txn.addProperty("index_test", "index_bigint_u", nogdb::PropertyType::UNSIGNED_BIGINT);
        txn.addProperty("index_test", "index_bigint", nogdb::PropertyType::BIGINT);
        txn.addProperty("index_test", "index_real", nogdb::PropertyType::REAL);
        txn.addProperty("index_test", "index_blob", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_index_test()
{
    try {
        auto txn = ctx->beginTxn(nogdb::TxnMode::READ_WRITE);
        txn.dropClass("index_test");
        txn.commit();
    } catch (const nogdb::Error& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}
