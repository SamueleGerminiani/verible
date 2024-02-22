// Copyright 2017-2020 The Verible Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// VerilogCollector is a specialized printer for Verilog syntax trees.

#pragma once

#include <iosfwd>
#include <stack>
#include <unordered_set>

#include "common/text/concrete_syntax_leaf.h"
#include "common/text/concrete_syntax_tree.h"
#include "common/text/symbol.h"
#include "common/text/tree_utils.h"
#include "verilog/parser/verilog_token_enum.h"
#include "verilog_nonterminals.h"

namespace verilog {

// ========================Filtering rules========================
using TreeContext = std::unordered_set<verilog::NodeEnum>;

enum class RuleType { Inclusion, Exclusion };

class FilteringRule {
 public:
  FilteringRule() = default;
  virtual ~FilteringRule() = default;

  // If true, the rule is satisfied on the symbol, context pair
  virtual bool Evaluate(const verible::Symbol &symbol,
                        const TreeContext &context) = 0;

  RuleType GetType() const { return type_; }
  bool IsInclusion() const { return type_ == RuleType::Inclusion; }
  bool IsExclusion() const { return type_ == RuleType::Exclusion; }
  virtual bool RequiresSubtreeDeletion() const { return false; }

 protected:
  TreeContext context_;
  RuleType type_;
};

using FilteringRulePtr = std::unique_ptr<FilteringRule>;

class TagSelection : public FilteringRule {
  using FilteringRule::context_;
  using FilteringRule::type_;

 public:
  TagSelection(const std::unordered_set<verilog::NodeEnum> &wantedNodeTags,
               const std::unordered_set<verilog_tokentype> &wantedLeafTags)
      : nodes_(wantedNodeTags), leaves_(wantedLeafTags) {
    type_ = RuleType::Inclusion;
  }

  ~TagSelection() override = default;
  bool Evaluate(const verible::Symbol &symbol,
                const TreeContext &context) override {
    if (context.empty() || context_.empty()) {
      return symbol.Kind() == verible::SymbolKind::kLeaf
                 ? leaves_.count(
                       static_cast<verilog_tokentype>(symbol.Tag().tag))
                 : nodes_.count(
                       static_cast<verilog::NodeEnum>(symbol.Tag().tag));
    }
    return false;
  }

  const std::unordered_set<verilog::NodeEnum> &nodes_;
  const std::unordered_set<verilog_tokentype> &leaves_;
};

class SelectAll : public FilteringRule {
  using FilteringRule::type_;

 public:
  SelectAll() { type_ = RuleType::Inclusion; }

  ~SelectAll() override = default;
  bool Evaluate(const verible::Symbol &symbol,
                const TreeContext &context) override {
    return true;
  }
};

class TagRectification : public FilteringRule {
  using FilteringRule::context_;
  using FilteringRule::type_;

 public:
  TagRectification(const std::unordered_set<verilog::NodeEnum> &wantedNodeTags,
                   const std::unordered_set<verilog_tokentype> &wantedLeafTags,
                   bool deleteSubtree = false)
      : nodes_(wantedNodeTags),
        leaves_(wantedLeafTags),
        deleteSubtree_(deleteSubtree) {
    type_ = RuleType::Exclusion;
  }
  ~TagRectification() override = default;

  bool Evaluate(const verible::Symbol &symbol,
                const TreeContext &context) override {
    if (context.empty() || context_.empty()) {
      return symbol.Kind() == verible::SymbolKind::kLeaf
                 ? leaves_.count(
                       static_cast<verilog_tokentype>(symbol.Tag().tag))
                 : nodes_.count(
                       static_cast<verilog::NodeEnum>(symbol.Tag().tag));
    }
    return false;
  }
  bool RequiresSubtreeDeletion() const override { return deleteSubtree_; }

  const std::unordered_set<verilog::NodeEnum> &nodes_;
  const std::unordered_set<verilog_tokentype> &leaves_;
  // If true, the subtree associated with the symbol must be deleted
  bool deleteSubtree_;
};

//========================VerilogTreeFilter========================
class VerilogTreeFilter : public verible::SymbolVisitor {
 public:
  explicit VerilogTreeFilter(const std::vector<FilteringRulePtr> &rules);

  void Visit(const verible::SyntaxTreeLeaf &) final;
  void Visit(const verible::SyntaxTreeNode &) final;

  // Returns the filtered tree
  verible::SymbolPtr &&GetFilteredTree() {
    CHECK_EQ(subTrees_.size(), 1);
    return std::move(subTrees_.top());
  }

  // Check if the symbol must be kept: if it is included by any rule and not
  // excluded by all rules
  bool MustKeep(const verible::Symbol &symbol) {
    bool inclusionTruthValue = false;
    bool exclusionTruthValue = false;

    for (const auto &rule : rules_) {
      if (rule->IsInclusion()) {
        inclusionTruthValue |= rule->Evaluate(symbol, TreeContext());
      } else {
        exclusionTruthValue |= rule->Evaluate(symbol, TreeContext());
      }
    }

    return inclusionTruthValue && !exclusionTruthValue;
  }

  // Yes: keep the symbol
  // No: do not keep the symbol
  // No_DeleteSubtree: do not keep the symbol and delete the subtreesassociated
  // with the symbol
  enum class CanKeepResult { Yes, No, No_DeleteSubtree };

  // Check if the symbol can be kept: if it is not excluded by any rule
  CanKeepResult CanKeep(const verible::Symbol &symbol) {
    bool exclusionTruthValue = false;
    bool deleteSubtree = false;

    for (const auto &rule : rules_) {
      if (rule->IsExclusion()) {
        if (rule->Evaluate(symbol, TreeContext())) {
          exclusionTruthValue = true;
          if (rule->RequiresSubtreeDeletion()) {
            deleteSubtree = true;
            break;
          }
        }
      }
    }

    return deleteSubtree         ? CanKeepResult::No_DeleteSubtree
           : exclusionTruthValue ? CanKeepResult::No
                                 : CanKeepResult::Yes;
  }

 private:
  // accumulates subtrees during the visit
  std::stack<verible::SymbolPtr> subTrees_;
  // rules to filter the CST tree
  const std::vector<FilteringRulePtr> &rules_;
};

// Filter the CST tree accrding to the rules
// root: the root of the CST tree
// rules: the rules to filter the CST tree
verible::SymbolPtr FilterSymbolsVerilogTree(
    const verible::Symbol &root, const std::vector<FilteringRulePtr> &rules);

}  // namespace verilog

