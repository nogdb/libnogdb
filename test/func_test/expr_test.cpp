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

#include "func_test.h"
#include <map>
#include <string>

void test_expression()
{
    nogdb::PropertyMapType propTypes;
    propTypes.emplace("firstname", nogdb::PropertyType::TEXT);
    propTypes.emplace("lastname", nogdb::PropertyType::TEXT);
    propTypes.emplace("age", nogdb::PropertyType::UNSIGNED_INTEGER);
    propTypes.emplace("gpa", nogdb::PropertyType::REAL);
    propTypes.emplace("#awards", nogdb::PropertyType::UNSIGNED_INTEGER);
    propTypes.emplace("balance", nogdb::PropertyType::INTEGER);
    propTypes.emplace("status", nogdb::PropertyType::TEXT);

    nogdb::Record r {};
    r.set("firstname", "hello");
    r.set("lastname", "world");
    r.set("age", 26U);
    r.set("gpa", 3.67);
    r.set("#awards", 3U);
    r.set("balance", -200);
    r.set("invalid", 0);

    auto c1 = nogdb::Condition("age").gt(24U); // true
    auto c2 = nogdb::Condition("age").le(24U); // false
    auto c3 = nogdb::Condition("gpa").ge(3.00); // true
    auto c4 = nogdb::Condition("gpa").lt(3.00); // false
    auto c5 = nogdb::Condition("firstname").eq("hello"); // true
    auto c6 = !nogdb::Condition("firstname").endWith("lo"); // false
    auto c7 = !nogdb::Condition("lastname").eq("world!"); // true
    auto c8 = nogdb::Condition("lastname").beginWith("so"); // false
    auto c9 = !nogdb::Condition("#awards").null(); // true
    auto c10 = nogdb::Condition("status").null(); // true

    try {
        // true & true
        assert((c1 && c3).execute(r, propTypes) == true);
        assert((c1 || c3).execute(r, propTypes) == true);
        // true & false
        assert((c1 && c4).execute(r, propTypes) == false);
        assert((c1 || c4).execute(r, propTypes) == true);
        // false & true
        assert((c4 && c1).execute(r, propTypes) == false);
        assert((c4 || c1).execute(r, propTypes) == true);
        // false & false
        assert((c2 && c4).execute(r, propTypes) == false);
        assert((c2 || c4).execute(r, propTypes) == false);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        // (true AND false) OR (true) = true
        assert(((c1 && c2) || (c5)).execute(r, propTypes) == true);
        // (true AND true) OR (true AND true) = true
        assert(((c9 && c10) || (c5 && c7)).execute(r, propTypes) == true);
        // (false AND true) AND (false OR (true AND true)) = false
        assert(((c4 && c5) && (c8 || (c1 && c3))).execute(r, propTypes) == false);
        // ((false AND true) AND (false OR (true AND true))) AND false = false
        assert((((c4 && c5) && (c8 || (c1 && c3))) && c2).execute(r, propTypes) == false);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto tmp = !nogdb::Condition("invalid"); // equivalent to null()
        auto res = (tmp && c10).execute(r, propTypes);
        assert(res == false);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto tmp = nogdb::Condition("gpa").contain("my grade");
        (tmp && c10).execute(r, propTypes);
        assert(false);
    } catch (const nogdb::Error& ex) {
        REQUIRE(ex, NOGDB_CTX_INVALID_COMPARATOR, "NOGDB_CTX_INVALID_COMPARATOR");
    }
}

void test_range_expression()
{
    nogdb::PropertyMapType propTypes;
    propTypes.emplace("firstname", nogdb::PropertyType::TEXT);
    propTypes.emplace("lastname", nogdb::PropertyType::TEXT);
    propTypes.emplace("age", nogdb::PropertyType::UNSIGNED_INTEGER);
    propTypes.emplace("gpa", nogdb::PropertyType::REAL);
    propTypes.emplace("#awards", nogdb::PropertyType::UNSIGNED_INTEGER);
    propTypes.emplace("balance", nogdb::PropertyType::INTEGER);
    propTypes.emplace("status", nogdb::PropertyType::TEXT);

    nogdb::Record r1 {}, r2 {}, r3 {};
    r1.set("firstname", "hello")
        .set("lastname", "world")
        .set("age", 26U)
        .set("gpa", 3.67)
        .set("#awards", 3U)
        .set("balance", -200);
    r2.set("firstname", "james")
        .set("lastname", "cookie")
        .set("age", 56U)
        .set("gpa", 2.89)
        .set("#awards", 0U)
        .set("balance", 100000);
    r3.set("firstname", "jessica")
        .set("lastname", "apollo")
        .set("age", 18U)
        .set("gpa", 3.24)
        .set("#awards", 10U)
        .set("balance", 5000);

    auto baseCondition = nogdb::Condition("status").null();

    auto in1 = nogdb::Condition("firstname").in("hello", "james");
    auto lastNames = std::vector<std::string> { "ApoLLo", "cOOkie", "koLTaI" };
    auto in2 = nogdb::Condition("lastname").in(lastNames).ignoreCase();
    auto in3 = nogdb::Condition("age").in(std::list<unsigned int> { 17, 18, 25, 26, 50 });

    try {
        assert((baseCondition && in1).execute(r1, propTypes) == true);
        assert((baseCondition && in1).execute(r2, propTypes) == true);
        assert((baseCondition && in1).execute(r3, propTypes) == false);

        assert((baseCondition && in2).execute(r1, propTypes) == false);
        assert((baseCondition && in2).execute(r2, propTypes) == true);
        assert((baseCondition && in2).execute(r3, propTypes) == true);

        assert((baseCondition && in3).execute(r1, propTypes) == true);
        assert((baseCondition && in3).execute(r2, propTypes) == false);
        assert((baseCondition && in3).execute(r3, propTypes) == true);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    auto between1 = nogdb::Condition("lastname").between("ant", "dog");
    auto between2 = nogdb::Condition("lastname").between("ANT", "DOG").ignoreCase();
    auto between3 = nogdb::Condition("gpa").between(3.00, 4.00);
    auto between4 = nogdb::Condition("balance").between(-200, 100000, { true, true });
    auto between5 = nogdb::Condition("balance").between(-200, 100000, { false, true });
    auto between6 = nogdb::Condition("balance").between(-200, 100000, { true, false });
    auto between7 = nogdb::Condition("balance").between(-200, 100000, { false, false });

    try {
        assert((baseCondition && between1).execute(r1, propTypes) == false);
        assert((baseCondition && between1).execute(r2, propTypes) == true);
        assert((baseCondition && between1).execute(r3, propTypes) == true);

        assert((baseCondition && between2).execute(r1, propTypes) == false);
        assert((baseCondition && between2).execute(r2, propTypes) == true);
        assert((baseCondition && between2).execute(r3, propTypes) == true);

        assert((baseCondition && between3).execute(r1, propTypes) == true);
        assert((baseCondition && between3).execute(r2, propTypes) == false);
        assert((baseCondition && between3).execute(r3, propTypes) == true);

        assert((baseCondition && between4).execute(r1, propTypes) == true);
        assert((baseCondition && between4).execute(r2, propTypes) == true);
        assert((baseCondition && between4).execute(r3, propTypes) == true);

        assert((baseCondition && between5).execute(r1, propTypes) == false);
        assert((baseCondition && between5).execute(r2, propTypes) == true);
        assert((baseCondition && between5).execute(r3, propTypes) == true);

        assert((baseCondition && between6).execute(r1, propTypes) == true);
        assert((baseCondition && between6).execute(r2, propTypes) == false);
        assert((baseCondition && between6).execute(r3, propTypes) == true);

        assert((baseCondition && between7).execute(r1, propTypes) == false);
        assert((baseCondition && between7).execute(r2, propTypes) == false);
        assert((baseCondition && between7).execute(r3, propTypes) == true);

    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_extra_string_expression()
{
    nogdb::PropertyMapType propTypes;
    propTypes.emplace("firstname", nogdb::PropertyType::TEXT);
    propTypes.emplace("lastname", nogdb::PropertyType::TEXT);
    propTypes.emplace("status", nogdb::PropertyType::TEXT);

    nogdb::Record r1 {}, r2 {}, r3 {};
    r1.set("firstname", "Jonathan").set("lastname", "Potter");
    r2.set("firstname", "Hermione").set("lastname", "PoLYsister");
    r3.set("firstname", "Hermes").set("lastname", "Apolly");

    auto like1 = nogdb::Condition("firstname").like("Herm%e%");
    auto like2 = nogdb::Condition("lastname").like("pO%ter").ignoreCase();
    auto like3 = nogdb::Condition("lastname").like("%ly%");
    auto like4 = nogdb::Condition("firstname").like("herm__").ignoreCase();

    auto baseCondition = nogdb::Condition("status").null();

    try {
        assert((baseCondition && like1).execute(r1, propTypes) == false);
        assert((baseCondition && like2).execute(r1, propTypes) == true);
        assert((baseCondition && like3).execute(r1, propTypes) == false);
        assert((baseCondition && like4).execute(r1, propTypes) == false);

        assert((baseCondition && like1).execute(r2, propTypes) == true);
        assert((baseCondition && like2).execute(r2, propTypes) == true);
        assert((baseCondition && like3).execute(r2, propTypes) == false);
        assert((baseCondition && like4).execute(r2, propTypes) == false);

        assert((baseCondition && like1).execute(r3, propTypes) == true);
        assert((baseCondition && like2).execute(r3, propTypes) == false);
        assert((baseCondition && like3).execute(r3, propTypes) == true);
        assert((baseCondition && like4).execute(r3, propTypes) == true);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    auto regex1 = nogdb::Condition("firstname").regex("Herm(.*)e(.*)");
    auto regex2 = nogdb::Condition("lastname").regex("pO(.*)ter").ignoreCase();
    auto regex3 = nogdb::Condition("lastname").regex("(.*)ly(.*)");
    auto regex4 = nogdb::Condition("firstname").regex("herm(.)(.)").ignoreCase();

    try {
        assert((baseCondition && regex1).execute(r1, propTypes) == false);
        assert((baseCondition && regex2).execute(r1, propTypes) == true);
        assert((baseCondition && regex3).execute(r1, propTypes) == false);
        assert((baseCondition && regex4).execute(r1, propTypes) == false);

        assert((baseCondition && regex1).execute(r2, propTypes) == true);
        assert((baseCondition && regex2).execute(r2, propTypes) == true);
        assert((baseCondition && regex3).execute(r2, propTypes) == false);
        assert((baseCondition && regex4).execute(r2, propTypes) == false);

        assert((baseCondition && regex1).execute(r3, propTypes) == true);
        assert((baseCondition && regex2).execute(r3, propTypes) == false);
        assert((baseCondition && regex3).execute(r3, propTypes) == true);
        assert((baseCondition && regex4).execute(r3, propTypes) == true);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_negative_expression()
{
    nogdb::PropertyMapType propTypes;
    propTypes.emplace("firstname", nogdb::PropertyType::TEXT);
    propTypes.emplace("lastname", nogdb::PropertyType::TEXT);
    propTypes.emplace("age", nogdb::PropertyType::UNSIGNED_INTEGER);
    propTypes.emplace("gpa", nogdb::PropertyType::REAL);
    propTypes.emplace("#awards", nogdb::PropertyType::UNSIGNED_INTEGER);
    propTypes.emplace("balance", nogdb::PropertyType::INTEGER);
    propTypes.emplace("status", nogdb::PropertyType::TEXT);

    nogdb::Record r {};
    r.set("firstname", "hello");
    r.set("lastname", "world");
    r.set("age", 26U);
    r.set("gpa", 3.67);
    r.set("#awards", 3U);
    r.set("balance", -200);
    r.set("invalid", 0);

    auto c1 = nogdb::Condition("age").gt(24U); // true
    auto c2 = nogdb::Condition("age").le(24U); // false
    auto c3 = nogdb::Condition("gpa").ge(3.00); // true
    auto c4 = nogdb::Condition("gpa").lt(3.00); // false
    auto c5 = nogdb::Condition("firstname").eq("hello"); // true
    auto c6 = !nogdb::Condition("firstname").endWith("lo"); // false
    auto c7 = !nogdb::Condition("lastname").eq("world!"); // true
    auto c8 = nogdb::Condition("lastname").beginWith("so"); // false
    auto c9 = !nogdb::Condition("#awards").null(); // true
    auto c10 = nogdb::Condition("status").null(); // true

    try {
        // true & true
        assert((!c1 && !c3).execute(r, propTypes) == false);
        assert((!c1 || !c3).execute(r, propTypes) == false);
        assert((!c1 || c3).execute(r, propTypes) == true);
        assert((c1 || !c3).execute(r, propTypes) == true);
        // true & false
        assert((c1 && !c4).execute(r, propTypes) == true);
        assert((!c1 || c4).execute(r, propTypes) == false);
        // false & true
        assert((!c4 && c1).execute(r, propTypes) == true);
        assert((c4 || !c1).execute(r, propTypes) == false);
        // false & false
        assert((!c2 && !c4).execute(r, propTypes) == true);
        assert((!c2 || !c4).execute(r, propTypes) == true);
        assert((!c2 || c4).execute(r, propTypes) == true);
        assert((c2 || !c4).execute(r, propTypes) == true);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        // (true AND false) OR NOT(true) = false
        assert(((c1 && c2) || !(c5)).execute(r, propTypes) == false);
        // NOT(true AND false) OR NOT(true) = true
        assert((!(c1 && c2) || !(c5)).execute(r, propTypes) == true);
        // NOT(NOT(true AND false) OR (true)) = false
        assert(!(!(c1 && c2) || !(c5)).execute(r, propTypes) == false);

        // NOT(true AND true) OR NOT(true AND true) = true
        assert((!(c9 && c10) || !(c5 && c7)).execute(r, propTypes) == false);

        // NOT(false AND true) AND (false OR (true AND true)) = true
        assert((!(c4 && c5) && (c8 || (c1 && c3))).execute(r, propTypes) == true);
        // NOT(false AND true) AND NOT(false OR (true AND true)) = false
        assert((!(c4 && c5) && !(c8 || (c1 && c3))).execute(r, propTypes) == false);
        // NOT(false AND true) AND (false OR NOT(true AND true)) = false
        assert((!(c4 && c5) && (c8 || !(c1 && c3))).execute(r, propTypes) == false);

        // NOT((false AND true) AND (false OR (true AND true))) AND NOT(false) = true
        assert((!((c4 && c5) && (c8 || (c1 && c3))) && !c2).execute(r, propTypes) == true);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto tmp1 = !c1;
        assert((c1 || !c3).execute(r, propTypes) == true);
        assert((tmp1 || !c3).execute(r, propTypes) == false);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

    try {
        auto exp = ((c1 && c2) || (c5));
        auto tmp2 = !exp;
        assert(exp.execute(r, propTypes) == true);
        assert(tmp2.execute(r, propTypes) == false);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }
}

void test_cmp_function_expression()
{
    nogdb::PropertyMapType propTypes;
    propTypes.emplace("firstname", nogdb::PropertyType::TEXT);
    propTypes.emplace("lastname", nogdb::PropertyType::TEXT);
    propTypes.emplace("age", nogdb::PropertyType::UNSIGNED_INTEGER);
    propTypes.emplace("gpa", nogdb::PropertyType::REAL);
    propTypes.emplace("#awards", nogdb::PropertyType::UNSIGNED_INTEGER);
    propTypes.emplace("balance", nogdb::PropertyType::INTEGER);
    propTypes.emplace("status", nogdb::PropertyType::TEXT);

    nogdb::Record r1 {}, r2 {}, r3 {};
    r1.set("firstname", "hello")
        .set("lastname", "world")
        .set("age", 26U)
        .set("gpa", 3.67)
        .set("#awards", 3U)
        .set("balance", -200);
    r2.set("firstname", "james")
        .set("lastname", "cookie")
        .set("age", 56U)
        .set("gpa", 2.89)
        .set("#awards", 0U)
        .set("balance", 100000);
    r3.set("firstname", "jessica")
        .set("lastname", "apollo")
        .set("age", 18U)
        .set("gpa", 3.24)
        .set("#awards", 10U)
        .set("balance", 5000);

    auto baseCondition = nogdb::Condition("status").null();
    auto multiCondition = baseCondition && nogdb::Condition("firstname").eq("test");
    auto cmp1 = [](const nogdb::Record& record) {
      return record.getIntU("age") > 30U && record.getInt("balance") > 0;
    };
    auto cmp2 = [](const nogdb::Record& record) {
      return record.getIntU("age") <= 30U && record.getInt("balance") <= 0;
    };

    try {
        assert((baseCondition && cmp1).execute(r1, propTypes) == false);
        assert((baseCondition && cmp1).execute(r2, propTypes) == true);
        assert((baseCondition && cmp1).execute(r3, propTypes) == false);
        assert((baseCondition && cmp2).execute(r1, propTypes) == true);
        assert((baseCondition && cmp2).execute(r2, propTypes) == false);
        assert((baseCondition && cmp2).execute(r3, propTypes) == false);

        assert((multiCondition || cmp1).execute(r1, propTypes) == false);
        assert((multiCondition || cmp1).execute(r2, propTypes) == true);
        assert((multiCondition || cmp1).execute(r3, propTypes) == false);
        assert((multiCondition || cmp2).execute(r1, propTypes) == true);
        assert((multiCondition || cmp2).execute(r2, propTypes) == false);
        assert((multiCondition || cmp2).execute(r3, propTypes) == false);
    } catch (const nogdb::Error& ex) {
        std::cout << "\nError: " << ex.what() << std::endl;
        assert(false);
    }

}
