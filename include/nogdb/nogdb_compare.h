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

#include <iostream> // for debugging
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <list>
#include <set>
#include <memory>

#include "nogdb_errors.h"
#include "nogdb_types.h"

namespace nogdb {

  namespace index {
    class IndexInterface;
  }

  class Condition;

  class MultiCondition;

  class GraphFilter {
  private:

    enum FilterMode {
      CONDITION,
      MULTI_CONDITION,
      COMPARE_FUNCTION
    };

  public:
    friend class compare::RecordCompare;

    GraphFilter();

    explicit GraphFilter(const Condition &condition);

    explicit GraphFilter(const MultiCondition &multiCondition);

    explicit GraphFilter(bool (*function)(const Record &record));

    ~GraphFilter() noexcept = default;

    GraphFilter(const GraphFilter &other);

    GraphFilter &operator=(const GraphFilter &other);

    GraphFilter(GraphFilter &&other) noexcept;

    GraphFilter &operator=(GraphFilter &&other) noexcept;

    GraphFilter &only(const std::string &className);

    template<typename ...T>
    GraphFilter &only(const std::string &className, const T &... classNames) {
      only(className);
      only(classNames...);
      return *this;
    }

    GraphFilter &only(const std::vector<std::string> &classNames);

    GraphFilter &only(const std::list<std::string> &classNames);

    GraphFilter &only(const std::set<std::string> &classNames);

    GraphFilter &only(const std::vector<std::string>::const_iterator &begin,
                      const std::vector<std::string>::const_iterator &end);

    GraphFilter &only(const std::list<std::string>::const_iterator &begin,
                      const std::list<std::string>::const_iterator &end);

    GraphFilter &only(const std::set<std::string>::const_iterator &begin,
                      const std::set<std::string>::const_iterator &end);

    GraphFilter &onlySubClassOf(const std::string &className);

    template<typename ...T>
    GraphFilter &onlySubClassOf(const std::string &className, const T &... classNames) {
      onlySubClassOf(className);
      onlySubClassOf(classNames...);
      return *this;
    }

    GraphFilter &onlySubClassOf(const std::vector<std::string> &classNames);

    GraphFilter &onlySubClassOf(const std::list<std::string> &classNames);

    GraphFilter &onlySubClassOf(const std::set<std::string> &classNames);

    GraphFilter &onlySubClassOf(const std::vector<std::string>::const_iterator &begin,
                                const std::vector<std::string>::const_iterator &end);

    GraphFilter &onlySubClassOf(const std::list<std::string>::const_iterator &begin,
                                const std::list<std::string>::const_iterator &end);

    GraphFilter &onlySubClassOf(const std::set<std::string>::const_iterator &begin,
                                const std::set<std::string>::const_iterator &end);

    GraphFilter &exclude(const std::string &className);

    template<typename ...T>
    GraphFilter &exclude(const std::string &className, const T &... classNames) {
      exclude(className);
      exclude(classNames...);
    }

    GraphFilter &exclude(const std::vector<std::string> &classNames);

    GraphFilter &exclude(const std::list<std::string> &classNames);

    GraphFilter &exclude(const std::set<std::string> &classNames);

    GraphFilter &exclude(const std::vector<std::string>::const_iterator &begin,
                         const std::vector<std::string>::const_iterator &end);

    GraphFilter &exclude(const std::list<std::string>::const_iterator &begin,
                         const std::list<std::string>::const_iterator &end);

    GraphFilter &exclude(const std::set<std::string>::const_iterator &begin,
                         const std::set<std::string>::const_iterator &end);

    GraphFilter &excludeSubClassOf(const std::string &className);

    template<typename ...T>
    GraphFilter &excludeSubClassOf(const std::string &className, const T &... classNames) {
      excludeSubClassOf(className);
      excludeSubClassOf(classNames...);
    }

    GraphFilter &excludeSubClassOf(const std::vector<std::string> &classNames);

    GraphFilter &excludeSubClassOf(const std::list<std::string> &classNames);

    GraphFilter &excludeSubClassOf(const std::set<std::string> &classNames);

    GraphFilter &excludeSubClassOf(const std::vector<std::string>::const_iterator &begin,
                                   const std::vector<std::string>::const_iterator &end);

    GraphFilter &excludeSubClassOf(const std::list<std::string>::const_iterator &begin,
                                   const std::list<std::string>::const_iterator &end);

    GraphFilter &excludeSubClassOf(const std::set<std::string>::const_iterator &begin,
                                   const std::set<std::string>::const_iterator &end);

  private:

    FilterMode _mode;

    //TODO: can be improved by using std::varient in c++17
    std::shared_ptr<Condition> _condition{};
    std::shared_ptr<MultiCondition> _multiCondition{};
    bool (*_function)(const Record &record);

    std::set<std::string> _onlyClasses{};
    std::set<std::string> _onlySubOfClasses{};
    std::set<std::string> _ignoreClasses{};
    std::set<std::string> _ignoreSubOfClasses{};

  };

  class Condition {
  private:
    enum class Comparator;

  public:

    friend class MultiCondition;

    friend class compare::RecordCompare;

    friend class datarecord::DataRecordInterface;

    friend class index::IndexInterface;

    Condition(const std::string &propName_);

    ~Condition() noexcept = default;

    operator GraphFilter() { return GraphFilter(*this); }

    template<typename T>
    Condition eq(T value_) const {
      auto tmp(*this);
      tmp.valueBytes = Bytes{value_};
      tmp.comp = Comparator::EQUAL;
      return tmp;
    }

    template<typename T>
    Condition gt(T value_) const {
      auto tmp(*this);
      tmp.valueBytes = Bytes{value_};
      tmp.comp = Comparator::GREATER;
      return tmp;
    }

    template<typename T>
    Condition lt(T value_) const {
      auto tmp(*this);
      tmp.valueBytes = Bytes{value_};
      tmp.comp = Comparator::LESS;
      return tmp;
    }

    template<typename T>
    Condition ge(T value_) const {
      auto tmp(*this);
      tmp.valueBytes = Bytes{value_};
      tmp.comp = Comparator::GREATER_EQUAL;
      return tmp;
    }

    template<typename T>
    Condition le(T value_) const {
      auto tmp(*this);
      tmp.valueBytes = Bytes{value_};
      tmp.comp = Comparator::LESS_EQUAL;
      return tmp;
    }

    template<typename T>
    Condition contain(T value_) const {
      auto tmp(*this);
      tmp.valueBytes = Bytes{value_};
      tmp.comp = Comparator::CONTAIN;
      return tmp;
    }

    template<typename T>
    Condition beginWith(T value_) const {
      auto tmp(*this);
      tmp.valueBytes = Bytes{value_};
      tmp.comp = Comparator::BEGIN_WITH;
      return tmp;
    }

    template<typename T>
    Condition endWith(T value_) const {
      auto tmp(*this);
      tmp.valueBytes = Bytes{value_};
      tmp.comp = Comparator::END_WITH;
      return tmp;
    }

    template<typename T>
    Condition like(T value_) const {
      auto tmp(*this);
      tmp.valueBytes = Bytes{value_};
      tmp.comp = Comparator::LIKE;
      return tmp;
    }

    template<typename T>
    Condition regex(T value_) const {
      auto tmp(*this);
      tmp.valueBytes = Bytes{value_};
      tmp.comp = Comparator::REGEX;
      return tmp;
    }

    Condition ignoreCase() const {
      auto tmp(*this);
      tmp.isIgnoreCase = true;
      return tmp;
    }

    Condition null() const {
      auto tmp(*this);
      tmp.valueBytes = Bytes{};
      tmp.comp = Comparator::IS_NULL;
      return tmp;
    }

    template<typename T>
    Condition between(T lower_, T upper_, const std::pair<bool, bool> isIncludeBound = {true, true}) const {
      auto tmp(*this);
      tmp.valueSet = std::vector<Bytes>{Bytes{lower_}, Bytes{upper_}};
      if (!isIncludeBound.first && !isIncludeBound.second) {
        tmp.comp = Comparator::BETWEEN_NO_BOUND;
      } else if (!isIncludeBound.first && isIncludeBound.second) {
        tmp.comp = Comparator::BETWEEN_NO_LOWER;
      } else if (isIncludeBound.first && !isIncludeBound.second) {
        tmp.comp = Comparator::BETWEEN_NO_UPPER;
      } else {
        tmp.comp = Comparator::BETWEEN;
      }
      return tmp;
    }

    template<typename T>
    Condition in(const std::vector<T> &values) const {
      auto tmp(*this);
      tmp.valueSet = std::vector<Bytes>{};
      for (auto iter = values.cbegin(); iter != values.cend(); ++iter) {
        tmp.valueSet.emplace_back(Bytes{*iter});
      }
      tmp.comp = Comparator::IN;
      return tmp;
    }

    template<typename T>
    Condition in(const std::list<T> &values) const {
      auto tmp(*this);
      tmp.valueSet = std::vector<Bytes>{};
      for (auto iter = values.cbegin(); iter != values.cend(); ++iter) {
        tmp.valueSet.emplace_back(Bytes{*iter});
      }
      tmp.comp = Comparator::IN;
      return tmp;
    }

    template<typename T>
    Condition in(const std::set<T> &values) const {
      auto tmp(*this);
      tmp.valueSet = std::vector<Bytes>{};
      for (auto iter = values.cbegin(); iter != values.cend(); ++iter) {
        tmp.valueSet.emplace_back(Bytes{*iter});
      }
      tmp.comp = Comparator::IN;
      return tmp;
    }

    template<typename T>
    Condition in(const T &value) const {
      auto tmp(*this);
      tmp.valueSet = std::vector<Bytes>{Bytes{value}};
      tmp.comp = Comparator::IN;
      return tmp;
    }

    template<typename T, typename... U>
    Condition in(const T &head, const U &... tail) const {
      auto tmp = in(head);
      for (const auto &value: in(tail...).valueSet) {
        tmp.valueSet.emplace_back(value);
      }
      return tmp;
    }

    MultiCondition operator&&(const Condition &c) const;

    MultiCondition operator&&(const MultiCondition &e) const;

    MultiCondition operator||(const Condition &c) const;

    MultiCondition operator||(const MultiCondition &e) const;

    Condition operator!() const;

  private:
    std::string propName;
    Bytes valueBytes;
    std::vector<Bytes> valueSet;
    Comparator comp;
    bool isIgnoreCase{false};
    bool isNegative{false};

    enum class Comparator {
      IS_NULL,
      NOT_NULL,
      EQUAL,
      GREATER,
      LESS,
      GREATER_EQUAL,
      LESS_EQUAL,
      CONTAIN,
      BEGIN_WITH,
      END_WITH,
      LIKE,
      REGEX,
      IN,
      BETWEEN,
      BETWEEN_NO_UPPER,
      BETWEEN_NO_LOWER,
      BETWEEN_NO_BOUND
    };
  };

  class MultiCondition {
  private:
    enum Operator {
      AND, OR
    };

    class CompositeNode;

    class ConditionNode;

  public:

    friend class Condition;

    friend class compare::RecordCompare;

    friend class datarecord::DataRecordInterface;

    friend class index::IndexInterface;

    MultiCondition() = delete;

    MultiCondition &operator&&(const MultiCondition &e);

    MultiCondition &operator&&(const Condition &c);

    MultiCondition &operator||(const MultiCondition &e);

    MultiCondition &operator||(const Condition &c);

    MultiCondition operator!() const;

    operator GraphFilter() { return GraphFilter(*this); }

    bool execute(const Record &r, const PropertyMapType &propType) const;

  private:
    std::shared_ptr<CompositeNode> root;
    std::vector<std::weak_ptr<ConditionNode>> conditions;

    MultiCondition(const Condition &c1, const Condition &c2, Operator opt);

    MultiCondition(const Condition &c, const MultiCondition &e, Operator opt);

    class ExprNode {
    public:
      explicit ExprNode(bool isCondition_);

      virtual ~ExprNode() noexcept = default;

      virtual bool check(const Record &r, const PropertyMapType &propType) const = 0;

      bool checkIfCondition() const;

    private:
      bool isCondition;
    };

    class ConditionNode : public ExprNode {
    public:
      explicit ConditionNode(const Condition &cond_);

      ~ConditionNode() noexcept override = default;

      virtual bool check(const Record &r, const PropertyMapType &propType) const override;

      const Condition &getCondition() const;

    private:
      Condition cond;
    };

    class CompositeNode : public ExprNode {
    public:
      CompositeNode(const std::shared_ptr<ExprNode> &left_,
                    const std::shared_ptr<ExprNode> &right_,
                    Operator opt_,
                    bool isNegative_ = false);

      ~CompositeNode() noexcept override = default;

      virtual bool check(const Record &r, const PropertyMapType &propType) const override;

      const std::shared_ptr<ExprNode> &getLeftNode() const;

      const std::shared_ptr<ExprNode> &getRightNode() const;

      const Operator &getOperator() const;

      bool getIsNegative() const;

    private:
      std::shared_ptr<ExprNode> left;
      std::shared_ptr<ExprNode> right;
      Operator opt;
      bool isNegative;
    };
  };

}
