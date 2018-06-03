/*
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

#ifndef TEST_PREPARE_H_
#define TEST_PREPARE_H_

#include "runtest.h"

inline void init_vertex_book() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "books", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "books", "title", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "books", "words", nogdb::PropertyType::UNSIGNED_BIGINT);
        nogdb::Property::add(txn, "books", "pages", nogdb::PropertyType::INTEGER);
        nogdb::Property::add(txn, "books", "price", nogdb::PropertyType::REAL);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_book() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "books");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_person() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "persons", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "persons", "name", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "persons", "age", nogdb::PropertyType::INTEGER);
        nogdb::Property::add(txn, "persons", "address", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "persons", "salary", nogdb::PropertyType::REAL);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_person() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "persons");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_author() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "authors", nogdb::ClassType::EDGE);
        nogdb::Property::add(txn, "authors", "profit", nogdb::PropertyType::REAL);
        nogdb::Property::add(txn, "authors", "time_used", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_author() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "authors");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_teachers() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "teachers", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "teachers", "name", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "teachers", "age", nogdb::PropertyType::UNSIGNED_INTEGER);
        nogdb::Property::add(txn, "teachers", "salary", nogdb::PropertyType::UNSIGNED_INTEGER);
        nogdb::Property::add(txn, "teachers", "level", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_teachers() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "teachers");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_students() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "students", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "students", "name", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "students", "age", nogdb::PropertyType::UNSIGNED_INTEGER);
        nogdb::Property::add(txn, "students", "grade", nogdb::PropertyType::REAL);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_students() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "students");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_departments() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "departments", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "departments", "name", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_departments() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "departments");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_subjects() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "subjects", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "subjects", "name", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_subjects() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "subjects");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_teach() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "teach", nogdb::ClassType::EDGE);
        nogdb::Property::add(txn, "teach", "semester", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_teach() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "teach");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_enrol() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "enrol", nogdb::ClassType::EDGE);
        nogdb::Property::add(txn, "enrol", "semester", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_enrol() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "enrol");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_know() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "know", nogdb::ClassType::EDGE);
        nogdb::Property::add(txn, "know", "relationship", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_know() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "know");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_workfor() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "workfor", nogdb::ClassType::EDGE);
        nogdb::Property::add(txn, "workfor", "position", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_workfor() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "workfor");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_belongto() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "belongto", nogdb::ClassType::EDGE);
        nogdb::Property::add(txn, "belongto", "null", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_belongto() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "belongto");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_folders() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "folders", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "folders", "name", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_folders() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "folders");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_files() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "files", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "files", "name", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_files() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "files");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_link() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "link", nogdb::ClassType::EDGE);
        nogdb::Property::add(txn, "link", "null", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_link() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "link");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_symbolic() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "symbolic", nogdb::ClassType::EDGE);
        nogdb::Property::add(txn, "symbolic", "null", nogdb::PropertyType::TEXT);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_symbolic() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "symbolic");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_mountain() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "mountains", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "mountains", "name", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "mountains", "temperature", nogdb::PropertyType::INTEGER);
        nogdb::Property::add(txn, "mountains", "height", nogdb::PropertyType::UNSIGNED_BIGINT);
        nogdb::Property::add(txn, "mountains", "rating", nogdb::PropertyType::REAL);
        nogdb::Property::add(txn, "mountains", "coordinates", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_mountain() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "mountains");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_location() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "locations", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "locations", "name", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "locations", "temperature", nogdb::PropertyType::INTEGER);
        nogdb::Property::add(txn, "locations", "postcode", nogdb::PropertyType::UNSIGNED_INTEGER);
        nogdb::Property::add(txn, "locations", "price", nogdb::PropertyType::BIGINT);
        nogdb::Property::add(txn, "locations", "population", nogdb::PropertyType::UNSIGNED_BIGINT);
        nogdb::Property::add(txn, "locations", "rating", nogdb::PropertyType::REAL);
        nogdb::Property::add(txn, "locations", "coordinates", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_location() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "locations");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_street() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "street", nogdb::ClassType::EDGE);
        nogdb::Property::add(txn, "street", "name", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "street", "temperature", nogdb::PropertyType::INTEGER);
        nogdb::Property::add(txn, "street", "capacity", nogdb::PropertyType::UNSIGNED_INTEGER);
        nogdb::Property::add(txn, "street", "distance", nogdb::PropertyType::REAL);
        nogdb::Property::add(txn, "street", "coordinates", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_street() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "street");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_highway() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "highway", nogdb::ClassType::EDGE);
        nogdb::Property::add(txn, "highway", "name", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "highway", "temperature", nogdb::PropertyType::INTEGER);
        nogdb::Property::add(txn, "highway", "capacity", nogdb::PropertyType::UNSIGNED_INTEGER);
        nogdb::Property::add(txn, "highway", "distance", nogdb::PropertyType::REAL);
        nogdb::Property::add(txn, "highway", "coordinates", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_highway() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "highway");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_railway() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "railway", nogdb::ClassType::EDGE);
        nogdb::Property::add(txn, "railway", "name", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "railway", "temperature", nogdb::PropertyType::INTEGER);
        nogdb::Property::add(txn, "railway", "distance", nogdb::PropertyType::REAL);
        nogdb::Property::add(txn, "railway", "coordinates", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_railway() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "railway");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_country() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "country", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "country", "name", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "country", "population", nogdb::PropertyType::UNSIGNED_BIGINT);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_country() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "country");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_path() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "path", nogdb::ClassType::EDGE);
        nogdb::Property::add(txn, "path", "distance", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_path() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "path");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_island() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "islands", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "islands", "name", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "islands", "area", nogdb::PropertyType::REAL);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_island() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "islands");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_city() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "cities", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "cities", "name", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "cities", "area", nogdb::PropertyType::REAL);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_city() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "cities");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_bridge() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "bridge", nogdb::ClassType::EDGE);
        nogdb::Property::add(txn, "bridge", "name", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "bridge", "length", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_bridge() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "bridge");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_edge_flight() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "flight", nogdb::ClassType::EDGE);
        nogdb::Property::add(txn, "flight", "name", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "flight", "distance", nogdb::PropertyType::UNSIGNED_INTEGER);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_edge_flight() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "flight");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void init_vertex_index_test() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::create(txn, "index_test", nogdb::ClassType::VERTEX);
        nogdb::Property::add(txn, "index_test", "index_text", nogdb::PropertyType::TEXT);
        nogdb::Property::add(txn, "index_test", "index_tinyint_u", nogdb::PropertyType::UNSIGNED_TINYINT);
        nogdb::Property::add(txn, "index_test", "index_tinyint", nogdb::PropertyType::TINYINT);
        nogdb::Property::add(txn, "index_test", "index_smallint_u", nogdb::PropertyType::UNSIGNED_SMALLINT);
        nogdb::Property::add(txn, "index_test", "index_smallint", nogdb::PropertyType::SMALLINT);
        nogdb::Property::add(txn, "index_test", "index_int_u", nogdb::PropertyType::UNSIGNED_INTEGER);
        nogdb::Property::add(txn, "index_test", "index_int", nogdb::PropertyType::INTEGER);
        nogdb::Property::add(txn, "index_test", "index_bigint_u", nogdb::PropertyType::UNSIGNED_BIGINT);
        nogdb::Property::add(txn, "index_test", "index_bigint", nogdb::PropertyType::BIGINT);
        nogdb::Property::add(txn, "index_test", "index_real", nogdb::PropertyType::REAL);
        nogdb::Property::add(txn, "index_test", "index_blob", nogdb::PropertyType::BLOB);
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

inline void destroy_vertex_index_test() {
    try {
        auto txn = nogdb::Txn{*ctx, nogdb::Txn::Mode::READ_WRITE};
        nogdb::Class::drop(txn, "index_test");
        txn.commit();
    } catch (const nogdb::Error &ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        assert(false);
    }
}

#endif
