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
#include "compare.hpp"

#include "nogdb/nogdb_compare.h"

namespace nogdb {

  MultiCondition &MultiCondition::operator&&(const MultiCondition &e) {
    auto newRoot = std::make_shared<CompositeNode>(root, e.root, Operator::AND);
    root = newRoot;
    std::copy(e.conditions.cbegin(), e.conditions.cend(), std::back_inserter(conditions));
    return *this;
  }

  MultiCondition &MultiCondition::operator&&(const Condition &c) {
    auto conditionPtr = std::make_shared<ConditionNode>(c);
    auto newRoot = std::make_shared<CompositeNode>(root, conditionPtr, Operator::AND);
    root = newRoot;
    conditions.push_back(conditionPtr);
    return *this;
  }

  MultiCondition &MultiCondition::operator||(const MultiCondition &e) {
    auto newRoot = std::make_shared<CompositeNode>(root, e.root, Operator::OR);
    root = newRoot;
    std::copy(e.conditions.cbegin(), e.conditions.cend(), std::back_inserter(conditions));
    return *this;
  }

  MultiCondition &MultiCondition::operator||(const Condition &c) {
    auto conditionPtr = std::make_shared<ConditionNode>(c);
    auto newRoot = std::make_shared<CompositeNode>(root, conditionPtr, Operator::OR);
    root = newRoot;
    conditions.push_back(conditionPtr);
    return *this;
  }

  MultiCondition MultiCondition::operator!() const {
    auto tmp(*this);
    tmp.root = std::make_shared<CompositeNode>(root->getLeftNode(),
                                               root->getRightNode(),
                                               root->getOperator(),
                                               !root->getIsNegative());
    return tmp;
  }

  bool MultiCondition::execute(const Record &r, const PropertyMapType &propType) const {
    return root->check(r, propType);
  }

  MultiCondition::MultiCondition(const Condition &c1, const Condition &c2, Operator opt) {
    auto conditionPtr1 = std::make_shared<ConditionNode>(c1);
    auto conditionPtr2 = std::make_shared<ConditionNode>(c2);
    root = std::make_shared<CompositeNode>(conditionPtr1, conditionPtr2, opt);
    conditions.push_back(conditionPtr1);
    conditions.push_back(conditionPtr2);
  }

  MultiCondition::MultiCondition(const Condition &c, const MultiCondition &e, Operator opt) {
    auto conditionPtr = std::make_shared<ConditionNode>(c);
    root = std::make_shared<CompositeNode>(conditionPtr, e.root, opt);
    conditions.push_back(conditionPtr);
    std::copy(e.conditions.cbegin(), e.conditions.cend(), std::back_inserter(conditions));
  }

  MultiCondition::ExprNode::ExprNode(bool isCondition_)
      : isCondition{isCondition_} {}

  bool MultiCondition::ExprNode::checkIfCondition() const {
    return isCondition;
  }

  MultiCondition::CompositeNode::CompositeNode(const std::shared_ptr<ExprNode> &left_,
                                               const std::shared_ptr<ExprNode> &right_,
                                               Operator opt_,
                                               bool isNegative_)
      : ExprNode(false), left{left_}, right{right_}, opt{opt_}, isNegative{isNegative_} {}

  bool MultiCondition::CompositeNode::check(const Record &r, const PropertyMapType &propType) const {
    if (opt == Operator::AND) {
      // check if right is condition, do right first, otherwise, do left
      if (right->checkIfCondition()) {
        if (!right->check(r, propType)) {
          return isNegative;
        } else {
          return left->check(r, propType) ^ isNegative;
        }
      } else {
        if (!left->check(r, propType)) {
          return isNegative;
        } else {
          return right->check(r, propType) ^ isNegative;
        }
      }
    } else if (opt == Operator::OR) {
      // check if right is condition, do right first, otherwise, do left
      if (right->checkIfCondition()) {
        if (right->check(r, propType)) {
          return !isNegative;
        } else {
          return left->check(r, propType) ^ isNegative;
        }
      } else {
        if (left->check(r, propType)) {
          return !isNegative;
        } else {
          return right->check(r, propType) ^ isNegative;
        }
      }
    } else {
      throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_UNKNOWN_ERR);
    }
  }

  const std::shared_ptr<MultiCondition::ExprNode> &MultiCondition::CompositeNode::getLeftNode() const {
    return left;
  }

  const std::shared_ptr<MultiCondition::ExprNode> &MultiCondition::CompositeNode::getRightNode() const {
    return right;
  }

  const MultiCondition::Operator &MultiCondition::CompositeNode::getOperator() const {
    return opt;
  }

  bool MultiCondition::CompositeNode::getIsNegative() const {
    return isNegative;
  }

  MultiCondition::ConditionNode::ConditionNode(const Condition &cond_)
      : ExprNode(true), cond{cond_} {}

  bool MultiCondition::ConditionNode::check(const Record &r, const PropertyMapType &propType) const {
    auto value = r.get(cond.propName);
    auto type = propType.find(cond.propName);
    if (type == propType.cend()) {
      throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_UNKNOWN_ERR);
    }
    if (cond.comp != Condition::Comparator::IS_NULL && cond.comp != Condition::Comparator::NOT_NULL) {
      if (!value.empty()) {
        return compare::RecordCompare::compareBytesValue(value, type->second, cond);
      }
      return false;
    } else {
      switch (cond.comp) {
        case Condition::Comparator::IS_NULL:
          return value.empty() ^ cond.isNegative;
        case Condition::Comparator::NOT_NULL:
          return !value.empty() ^ cond.isNegative;
        default:
          throw NOGDB_CONTEXT_ERROR(NOGDB_CTX_UNKNOWN_ERR);
      }
    }
  }

  const Condition &MultiCondition::ConditionNode::getCondition() const {
    return cond;
  }

}

