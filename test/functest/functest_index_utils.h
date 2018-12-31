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

#include <iostream>
#include <cassert>
#include <type_traits>
#include <sstream>
#include <string>
#include <functional>
#include <set>

#include "functest_config.h"
#include "functest_cursor_utils.h"

#include "nogdb/nogdb.h"

template<typename T>
void indexConditionTester(nogdb::Context *ctx, const std::string &className, const std::string &propertyName,
                          const nogdb::RecordDescriptor &rdescMin, const T &min,
                          const nogdb::RecordDescriptor &rdescFirstMid, const T &firstMid,
                          const nogdb::RecordDescriptor &rdescSecondMid, const T &secondMid,
                          const nogdb::RecordDescriptor &rdescMax, const T &max) {

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(min)).get();
    assert(rdescCompare(propertyName, res, {rdescMin}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(firstMid)).get();
    assert(rdescCompare(propertyName, res, {rdescFirstMid}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(secondMid)).get();
    assert(rdescCompare(propertyName, res, {rdescSecondMid}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(max)).get();
    assert(rdescCompare(propertyName, res, {rdescMax}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(min)).get();
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(firstMid)).get();
    assert(rdescCompare(propertyName, res, {rdescMin}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(secondMid)).get();
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(max)).get();
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(min)).get();
    assert(rdescCompare(propertyName, res, {rdescMin}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(firstMid)).get();
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(secondMid)).get();
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(max)).get();
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid, rdescMax}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(min)).get();
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid, rdescMax}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(firstMid)).get();
    assert(rdescCompare(propertyName, res, {rdescMax, rdescFirstMid, rdescSecondMid}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(secondMid)).get();
    assert(rdescCompare(propertyName, res, {rdescMax, rdescSecondMid}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(max)).get();
    assert(rdescCompare(propertyName, res, {rdescMax}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(min)).get();
    assert(rdescCompare(propertyName, res, {rdescMax, rdescFirstMid, rdescSecondMid}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(firstMid)).get();
    assert(rdescCompare(propertyName, res, {rdescMax, rdescSecondMid}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(secondMid)).get();
    assert(rdescCompare(propertyName, res, {rdescMax}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(max)).get();
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, max)).get();
    assert(rdescCompare(propertyName, res, {rdescMin, rdescMax, rdescFirstMid, rdescSecondMid}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, max, {false, true})).get();
    assert(rdescCompare(propertyName, res, {rdescMax, rdescFirstMid, rdescSecondMid}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, max, {true, false})).get();
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, max, {false, false})).get();
    assert(rdescCompare(propertyName, res, {rdescFirstMid, rdescSecondMid}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, firstMid)).get();
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, firstMid, {false, true})).get();
    assert(rdescCompare(propertyName, res, {rdescFirstMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, firstMid, {true, false})).get();
    assert(rdescCompare(propertyName, res, {rdescMin}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, firstMid, {false, false})).get();
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, secondMid));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid}));

    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(min, secondMid, {false, true}));
    assert(rdescCompare(propertyName, res, {rdescFirstMid, rdescSecondMid}));

    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(min, secondMid, {true, false}));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid}));

    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(min, secondMid, {false, false}));
    assert(rdescCompare(propertyName, res, {rdescFirstMid}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, secondMid));
    assert(rdescCompare(propertyName, res, {rdescFirstMid, rdescSecondMid}));

    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(firstMid, secondMid, {false, true}));
    assert(rdescCompare(propertyName, res, {rdescSecondMid}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(firstMid, secondMid, {true, false}));
    assert(rdescCompare(propertyName, res, {rdescFirstMid}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(firstMid, secondMid, {false, false}));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, max));
    assert(rdescCompare(propertyName, res, {rdescFirstMid, rdescSecondMid, rdescMax}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, max, {false, true}));
    assert(rdescCompare(propertyName, res, {rdescSecondMid, rdescMax}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, max, {true, false}));
    assert(rdescCompare(propertyName, res, {rdescFirstMid, rdescSecondMid}));

    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(firstMid, max, {false, false}));
    assert(rdescCompare(propertyName, res, {rdescSecondMid}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid, max));
    assert(rdescCompare(propertyName, res, {rdescSecondMid, rdescMax}));

    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(secondMid, max, {false, true}));
    assert(rdescCompare(propertyName, res, {rdescMax}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(secondMid, max, {true, false}));
    assert(rdescCompare(propertyName, res, {rdescSecondMid}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(secondMid, max, {false, false}));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

}

template<typename T>
void indexAdjacentConditionTester(nogdb::Context *ctx, const std::string &className, const std::string &propertyName,
                                  const nogdb::RecordDescriptor &rdescMin, const T &min,
                                  const nogdb::RecordDescriptor &rdescFirstMid, const T &firstMid,
                                  const nogdb::RecordDescriptor &rdescSecondMid, const T &secondMid,
                                  const nogdb::RecordDescriptor &rdescMax, const T &max) {
  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(firstMid - 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(secondMid + 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(min + 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(max - 1));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(min - 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(firstMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMin}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(secondMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(max - 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(min + 1));
    assert(rdescCompare(propertyName, res, {rdescMin}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(firstMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(secondMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(max + 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid, rdescMax}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(min + 1));
    assert(rdescCompare(propertyName, res, {rdescMin}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(firstMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(secondMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(max + 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid, rdescMax}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(min - 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(firstMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMin}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(secondMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(max - 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(min + 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid, rdescSecondMid, rdescMax}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(firstMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMax, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(secondMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMax}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(max + 1));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(min - 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescMax, rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(firstMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMax, rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(secondMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMax, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(max - 1));
    assert(rdescCompare(propertyName, res, {rdescMax}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(min + 1));
    assert(rdescCompare(propertyName, res, {rdescMax, rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(firstMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMax, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(secondMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMax}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(max + 1));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(min - 1));
    assert(rdescCompare(propertyName, res, {rdescMax, rdescSecondMid, rdescFirstMid, rdescMin}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(firstMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMax, rdescSecondMid, rdescFirstMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(secondMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMax, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(max - 1));
    assert(rdescCompare(propertyName, res, {rdescMax}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min + 1, max - 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min - 1, max - 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min + 1, max + 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid, rdescSecondMid, rdescMax}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min - 1, max + 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid, rdescMax}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min + 1, firstMid - 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min + 1, firstMid + 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min - 1, firstMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMin}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min - 1, firstMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min + 1, secondMid + 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min + 1, secondMid - 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min - 1, secondMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min - 1, secondMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMin, rdescFirstMid}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(
                                       nogdb::Condition(propertyName).between(firstMid + 1, secondMid + 1));
    assert(rdescCompare(propertyName, res, {rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid + 1, secondMid - 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid - 1, secondMid + 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid - 1, secondMid - 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid + 1, max - 1));
    assert(rdescCompare(propertyName, res, {rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid - 1, max - 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid + 1, max + 1));
    assert(rdescCompare(propertyName, res, {rdescSecondMid, rdescMax}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid - 1, max + 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid, rdescSecondMid, rdescMax}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid + 1, max - 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid - 1, max - 1));
    assert(rdescCompare(propertyName, res, {rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid + 1, max + 1));
    assert(rdescCompare(propertyName, res, {rdescMax}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid - 1, max + 1));
    assert(rdescCompare(propertyName, res, {rdescSecondMid, rdescMax}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }
}

template<typename T>
void emptyIndexConditionTester(nogdb::Context *ctx, const std::string &className, const std::string &propertyName,
                               const nogdb::RecordDescriptor &rdescMin, const T &min,
                               const nogdb::RecordDescriptor &rdescFirstMid, const T &firstMid,
                               const nogdb::RecordDescriptor &rdescSecondMid, const T &secondMid,
                               const nogdb::RecordDescriptor &rdescMax, const T &max) {

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(min));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(firstMid));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(secondMid));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(max));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(min));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(firstMid));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(secondMid));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(max));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(min));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(firstMid));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(secondMid));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(max));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(min));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(firstMid));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(secondMid));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(max));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(min));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(firstMid));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(secondMid));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(max));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, max));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, max, {false, true}));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, max, {true, false}));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, max, {false, false}));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, firstMid));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, firstMid, {false, true}));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, firstMid, {true, false}));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(min, firstMid, {false, false}));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, secondMid));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(min, secondMid, {false, true}));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(min, secondMid, {true, false}));
    assert(rdescCompare(propertyName, res, {}));

    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(min, secondMid, {false, false}));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, secondMid));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(firstMid, secondMid, {false, true}));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(firstMid, secondMid, {true, false}));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(firstMid, secondMid, {false, false}));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, max));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, max, {false, true}));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, max, {true, false}));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(firstMid, max, {false, false}));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid, max));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(secondMid, max, {false, true}));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(secondMid, max, {true, false}));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(secondMid, max, {false, false}));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

}

template<typename T>
void indexCursorConditionTester(nogdb::Context *ctx, const std::string &className, const std::string &propertyName,
                                const nogdb::RecordDescriptor &rdescMin, const T &min,
                                const nogdb::RecordDescriptor &rdescFirstMid, const T &firstMid,
                                const nogdb::RecordDescriptor &rdescSecondMid, const T &secondMid,
                                const nogdb::RecordDescriptor &rdescMax, const T &max) {

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(min));
    assert(rdescCursorCompare(propertyName, res, {rdescMin}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(firstMid));
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(secondMid));
    assert(rdescCursorCompare(propertyName, res, {rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(max));
    assert(rdescCursorCompare(propertyName, res, {rdescMax}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(min));
    assert(rdescCursorCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(firstMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMin}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(secondMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMin, rdescFirstMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(max));
    assert(rdescCursorCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(min));
    assert(rdescCursorCompare(propertyName, res, {rdescMin}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(firstMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMin, rdescFirstMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(secondMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(max));
    assert(rdescCursorCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid, rdescMax}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(min));
    assert(rdescCursorCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid, rdescMax}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(firstMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMax, rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(secondMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMax, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(max));
    assert(rdescCursorCompare(propertyName, res, {rdescMax}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(min));
    assert(rdescCursorCompare(propertyName, res, {rdescMax, rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(firstMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMax, rdescSecondMid}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(secondMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMax}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(max));
    assert(rdescCursorCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, max));
    assert(rdescCursorCompare(propertyName, res, {rdescMin, rdescMax, rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, max, {false, true}));
    assert(rdescCursorCompare(propertyName, res, {rdescMax, rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, max, {true, false}));
    assert(rdescCursorCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, max, {false, false}));
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid, rdescSecondMid}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, firstMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMin, rdescFirstMid}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, firstMid, {false, true}));
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, firstMid, {true, false}));
    assert(rdescCursorCompare(propertyName, res, {rdescMin}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, firstMid, {false, false}));
    assert(rdescCursorCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, secondMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMin, rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, secondMid, {false, true}));
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, secondMid, {true, false}));
    assert(rdescCursorCompare(propertyName, res, {rdescMin, rdescFirstMid}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, secondMid, {false, false}));
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(
                                             nogdb::Condition(propertyName).between(firstMid, secondMid));
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(firstMid, secondMid, {false, true}));
    assert(rdescCursorCompare(propertyName, res, {rdescSecondMid}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(firstMid, secondMid, {true, false}));
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(firstMid, secondMid, {false, false}));
    assert(rdescCursorCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, max));
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid, rdescSecondMid, rdescMax}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(firstMid, max, {false, true}));
    assert(rdescCursorCompare(propertyName, res, {rdescSecondMid, rdescMax}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(firstMid, max, {true, false}));
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid, rdescSecondMid}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(firstMid, max, {false, false}));
    assert(rdescCursorCompare(propertyName, res, {rdescSecondMid}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid, max));
    assert(rdescCursorCompare(propertyName, res, {rdescSecondMid, rdescMax}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(secondMid, max, {false, true}));
    assert(rdescCursorCompare(propertyName, res, {rdescMax}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(secondMid, max, {true, false}));
    assert(rdescCursorCompare(propertyName, res, {rdescSecondMid}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(secondMid, max, {false, false}));
    assert(rdescCursorCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

}

template<typename T>
void nonUniqueIndexConditionTester(nogdb::Context *ctx, const std::string &className, const std::string &propertyName,
                                   const nogdb::RecordDescriptor &rdescMin1, const nogdb::RecordDescriptor &rdescMin2,
                                   const T &min,
                                   const nogdb::RecordDescriptor &rdescFirstMid1,
                                   const nogdb::RecordDescriptor &rdescFirstMid2, const T &firstMid,
                                   const nogdb::RecordDescriptor &rdescSecondMid1,
                                   const nogdb::RecordDescriptor &rdescSecondMid2, const T &secondMid,
                                   const nogdb::RecordDescriptor &rdescMax1, const nogdb::RecordDescriptor &rdescMax2,
                                   const T &max) {

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(min));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescMin2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(firstMid));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(secondMid));
    assert(rdescCompare(propertyName, res, {rdescSecondMid1, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(max));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescMax2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(min));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(firstMid));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescMin2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(secondMid));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(max));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(min));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescMin2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(firstMid));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(secondMid));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(max));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescMin2, rdescFirstMid2,
                         rdescSecondMid2, rdescMax2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(min));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescMin2, rdescFirstMid2,
                         rdescSecondMid2, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(firstMid));
    assert(rdescCompare(propertyName, res,
                        {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(secondMid));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescSecondMid1, rdescMax2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(max));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescMax2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(min));
    assert(rdescCompare(propertyName, res,
                        {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(firstMid));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescSecondMid1, rdescMax2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(secondMid));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(max));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, max));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescMax2, rdescFirstMid2,
                         rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, max, {false, true}));
    assert(rdescCompare(propertyName, res,
                        {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, max, {true, false}));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, max, {false, false}));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, firstMid));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, firstMid, {false, true}));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, firstMid, {true, false}));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescMin2}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(min, firstMid, {false, false}));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, secondMid));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(min, secondMid, {false, true}));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(min, secondMid, {true, false}));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(min, secondMid, {false, false}));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescFirstMid2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, secondMid));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(firstMid, secondMid, {false, true}));
    assert(rdescCompare(propertyName, res, {rdescSecondMid1, rdescSecondMid2}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(firstMid, secondMid, {true, false}));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescFirstMid2}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(firstMid, secondMid, {false, false}));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, max));
    assert(rdescCompare(propertyName, res,
                        {rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescFirstMid2, rdescSecondMid2, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, max, {false, true}));
    assert(rdescCompare(propertyName, res, {rdescSecondMid1, rdescMax1, rdescSecondMid2, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, max, {true, false}));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(firstMid, max, {false, false}));
    assert(rdescCompare(propertyName, res, {rdescSecondMid1, rdescSecondMid2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid, max));
    assert(rdescCompare(propertyName, res, {rdescSecondMid1, rdescMax1, rdescSecondMid2, rdescMax2}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(secondMid, max, {false, true}));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescMax2}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(secondMid, max, {true, false}));
    assert(rdescCompare(propertyName, res, {rdescSecondMid1, rdescSecondMid2}));
    res = txn.find(className).indexed().where(
                                  nogdb::Condition(propertyName).between(secondMid, max, {false, false}));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

}

template<typename T>
void nonUniqueIndexAdjacentConditionTester(nogdb::Context *ctx, const std::string &className,
                                           const std::string &propertyName,
                                           const nogdb::RecordDescriptor &rdescMin1,
                                           const nogdb::RecordDescriptor &rdescMin2, const T &min,
                                           const nogdb::RecordDescriptor &rdescFirstMid1,
                                           const nogdb::RecordDescriptor &rdescFirstMid2, const T &firstMid,
                                           const nogdb::RecordDescriptor &rdescSecondMid1,
                                           const nogdb::RecordDescriptor &rdescSecondMid2, const T &secondMid,
                                           const nogdb::RecordDescriptor &rdescMax1,
                                           const nogdb::RecordDescriptor &rdescMax2, const T &max) {

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(min + 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(firstMid + 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(secondMid + 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(max + 1));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(min + 1));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescMin2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(firstMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescMin2, rdescFirstMid1, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(secondMid + 1));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2, rdescSecondMid1, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(max + 1));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2, rdescSecondMid1, rdescSecondMid2,
                         rdescMax1, rdescMax2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(min - 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(firstMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescMin2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(secondMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(max - 1));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(min + 1));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescMin2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(firstMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(secondMid + 1));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(max + 1));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2,
                         rdescMax1, rdescMax2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(min - 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(firstMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescMin2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(secondMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(max - 1));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(min + 1));
    assert(rdescCompare(propertyName, res,
                        {rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescFirstMid2, rdescSecondMid2, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(firstMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescSecondMid1, rdescMax2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(secondMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(max + 1));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(min - 1));
    assert(rdescCompare(propertyName, res,
                        {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMin1, rdescMax2, rdescFirstMid2,
                         rdescSecondMid2, rdescMin2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(firstMid - 1));
    assert(rdescCompare(propertyName, res,
                        {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(secondMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescSecondMid1, rdescMax2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(max - 1));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescMax2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(min + 1));
    assert(rdescCompare(propertyName, res,
                        {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(firstMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescSecondMid1, rdescMax2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(secondMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(max + 1));
    assert(rdescCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(max - 1));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(firstMid - 1));
    assert(rdescCompare(propertyName, res,
                        {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(secondMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescMax2, rdescSecondMid1, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(min - 1));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescMax2, rdescFirstMid2,
                         rdescSecondMid2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min + 1, max - 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min - 1, max - 1));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min + 1, max + 1));
    assert(rdescCompare(propertyName, res,
                        {rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescFirstMid2, rdescSecondMid2, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min - 1, max + 1));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescMin2, rdescFirstMid2,
                         rdescSecondMid2, rdescMax2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min + 1, firstMid - 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min + 1, firstMid + 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min - 1, firstMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescMin2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min - 1, firstMid + 1));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescMin2, rdescFirstMid1, rdescFirstMid2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min + 1, secondMid - 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min + 1, secondMid + 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min - 1, secondMid - 1));
    assert(rdescCompare(propertyName, res, {rdescMin1, rdescMin2, rdescFirstMid1, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min - 1, secondMid + 1));
    assert(rdescCompare(propertyName, res,
                        {rdescMin1, rdescMin2, rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(
                                       nogdb::Condition(propertyName).between(firstMid - 1, secondMid - 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid - 1, secondMid + 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid + 1, secondMid - 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid + 1, secondMid + 1));
    assert(rdescCompare(propertyName, res, {rdescSecondMid1, rdescSecondMid2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid + 1, max - 1));
    assert(rdescCompare(propertyName, res, {rdescSecondMid1, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid - 1, max - 1));
    assert(rdescCompare(propertyName, res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid + 1, max + 1));
    assert(rdescCompare(propertyName, res, {rdescSecondMid1, rdescSecondMid2, rdescMax1, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid - 1, max + 1));
    assert(rdescCompare(propertyName, res,
                        {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2, rdescMax1, rdescMax2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid + 1, max - 1));
    assert(rdescCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid - 1, max - 1));
    assert(rdescCompare(propertyName, res, {rdescSecondMid1, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid + 1, max + 1));
    assert(rdescCompare(propertyName, res, {rdescMax1, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid - 1, max + 1));
    assert(rdescCompare(propertyName, res, {rdescSecondMid1, rdescSecondMid2, rdescMax1, rdescMax2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

}

template<typename T>
void
nonUniqueIndexCursorConditionTester(nogdb::Context *ctx, const std::string &className, const std::string &propertyName,
                                    const nogdb::RecordDescriptor &rdescMin1, const nogdb::RecordDescriptor &rdescMin2,
                                    const T &min,
                                    const nogdb::RecordDescriptor &rdescFirstMid1,
                                    const nogdb::RecordDescriptor &rdescFirstMid2, const T &firstMid,
                                    const nogdb::RecordDescriptor &rdescSecondMid1,
                                    const nogdb::RecordDescriptor &rdescSecondMid2, const T &secondMid,
                                    const nogdb::RecordDescriptor &rdescMax1, const nogdb::RecordDescriptor &rdescMax2,
                                    const T &max) {

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(min));
    assert(rdescCursorCompare(propertyName, res, {rdescMin1, rdescMin2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(firstMid));
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid1, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(secondMid));
    assert(rdescCursorCompare(propertyName, res, {rdescSecondMid1, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).eq(max));
    assert(rdescCursorCompare(propertyName, res, {rdescMax1, rdescMax2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(min));
    assert(rdescCursorCompare(propertyName, res, {}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(firstMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMin1, rdescMin2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(secondMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).lt(max));
    assert(rdescCursorCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2,
                                                  rdescSecondMid2}));

  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(min));
    assert(rdescCursorCompare(propertyName, res, {rdescMin1, rdescMin2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(firstMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(secondMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2,
                                                  rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).le(max));
    assert(rdescCursorCompare(propertyName, res,
                              {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescMin2, rdescFirstMid2,
                               rdescSecondMid2, rdescMax2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(min));
    assert(rdescCursorCompare(propertyName, res,
                              {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescMin2, rdescFirstMid2,
                               rdescSecondMid2, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(firstMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2,
                                                  rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(secondMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMax1, rdescSecondMid1, rdescMax2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).ge(max));
    assert(rdescCursorCompare(propertyName, res, {rdescMax1, rdescMax2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(min));
    assert(rdescCursorCompare(propertyName, res, {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2,
                                                  rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(firstMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMax1, rdescSecondMid1, rdescMax2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(secondMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMax1, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).gt(max));
    assert(rdescCursorCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, max));
    assert(rdescCursorCompare(propertyName, res,
                              {rdescMin1, rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescMax2,
                               rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, max, {false, true}));
    assert(rdescCursorCompare(propertyName, res, {rdescMax1, rdescFirstMid1, rdescSecondMid1, rdescMax2, rdescFirstMid2,
                                                  rdescSecondMid2}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, max, {true, false}));
    assert(rdescCursorCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2,
                                                  rdescSecondMid2}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, max, {false, false}));
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, firstMid));
    assert(rdescCursorCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, firstMid, {false, true}));
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid1, rdescFirstMid2}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, firstMid, {true, false}));
    assert(rdescCursorCompare(propertyName, res, {rdescMin1, rdescMin2}));
    res = txn.find(className).indexed().where(
                                        nogdb::Condition(propertyName).between(min, firstMid, {false, false}));
    assert(rdescCursorCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, secondMid)).getCursor();
    assert(rdescCursorCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescSecondMid1, rdescMin2, rdescFirstMid2,
                                                  rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, secondMid, {false, true})).getCursor();
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, secondMid, {true, false})).getCursor();
    assert(rdescCursorCompare(propertyName, res, {rdescMin1, rdescFirstMid1, rdescMin2, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(min, secondMid, {false, false})).getCursor();
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid1, rdescFirstMid2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, secondMid)).getCursor();
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, secondMid, {false, true})).getCursor();
    assert(rdescCursorCompare(propertyName, res, {rdescSecondMid1, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, secondMid, {true, false})).getCursor();
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid1, rdescFirstMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, secondMid, {false, false})).getCursor();
    assert(rdescCursorCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, max)).getCursor();
    assert(rdescCursorCompare(propertyName, res,
                              {rdescFirstMid1, rdescSecondMid1, rdescMax1, rdescFirstMid2, rdescSecondMid2, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, max, {false, true})).getCursor();
    assert(rdescCursorCompare(propertyName, res, {rdescSecondMid1, rdescMax1, rdescSecondMid2, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, max, {true, false})).getCursor();
    assert(rdescCursorCompare(propertyName, res, {rdescFirstMid1, rdescSecondMid1, rdescFirstMid2, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(firstMid, max, {false, false})).getCursor();
    assert(rdescCursorCompare(propertyName, res, {rdescSecondMid1, rdescSecondMid2}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

  try {
    auto txn = ctx->beginTxn(nogdb::TxnMode::READ_ONLY);
    auto res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid, max)).getCursor();
    assert(rdescCursorCompare(propertyName, res, {rdescSecondMid1, rdescMax1, rdescSecondMid2, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid, max, {false, true})).getCursor();
    assert(rdescCursorCompare(propertyName, res, {rdescMax1, rdescMax2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid, max, {true, false})).getCursor();
    assert(rdescCursorCompare(propertyName, res, {rdescSecondMid1, rdescSecondMid2}));
    res = txn.find(className).indexed().where(nogdb::Condition(propertyName).between(secondMid, max, {false, false})).getCursor();
    assert(rdescCursorCompare(propertyName, res, {}));
  } catch (const nogdb::Error &ex) {
    std::cout << "\nError: " << ex.what() << std::endl;
    assert(false);
  }

}