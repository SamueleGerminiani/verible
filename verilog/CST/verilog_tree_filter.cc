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

// Implementation of VerilogTreeFilter

#include "verilog/CST/verilog_tree_filter.h"

#include <iostream>
#include <memory>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "common/text/concrete_syntax_leaf.h"
#include "common/text/concrete_syntax_tree.h"
#include "common/text/symbol.h"
#include "common/text/token_info.h"
#include "common/util/value_saver.h"
#include "verilog/CST/verilog_nonterminals.h"  // for NodeEnumToString
#include "verilog/parser/verilog_parser.h"     // for verilog_symbol_name

namespace verilog {

VerilogTreeFilter::VerilogTreeFilter(const std::vector<FilteringRulePtr> &rules)
    : rules_(rules) {}

void VerilogTreeFilter::Visit(const verible::SyntaxTreeLeaf &leaf) {
  if (MustKeep(leaf)) {
    verible::SymbolPtr newLeaf = std::make_unique<verible::SyntaxTreeLeaf>(
        verible::SyntaxTreeLeaf(leaf));
    subTrees_.push(std::move(newLeaf));

    // DEBUG
    // std::cout << "Must keep leaf: "
    //           << static_cast<const verible::SyntaxTreeLeaf *>(
    //                  subTrees_.top().get())
    //                  ->get()
    //                  .text()
    //           << std::endl;
  }
}

void VerilogTreeFilter::Visit(const verible::SyntaxTreeNode &node) {
  // How many subtrees before visiting the children
  size_t currentNumberOfSubtrees = subTrees_.size();

  // Visit all children
  for (const auto &child : node.children()) {
    if (child) {
      child->Accept(this);
    }
  }

  // How many subtrees were added by the children
  int increment = subTrees_.size() - currentNumberOfSubtrees;
  CHECK_GE(increment, 0);

  auto canKeepResult = CanKeep(node);
  if (canKeepResult == CanKeepResult::Yes) {
    // If there are more than 2 subtrees generated by the children, then must
    // also keep the current node as a "separator" between the subtrees
    if (MustKeep(node) || increment >= 2) {
      // copy the current node
      verible::SymbolPtr thisSubstreeRoot =
          verible::MakeTaggedNode(node.Tag().tag);

      // add the subtrees to the current node
      for (size_t i = 0; i < (size_t)increment; i++) {
        CHECK(!subTrees_.empty());
        thisSubstreeRoot =
            verible::ExtendNode(thisSubstreeRoot, subTrees_.top());
        subTrees_.pop();
      }

      // push the current node to the stack
      subTrees_.push(std::move(thisSubstreeRoot));
    }
  } else if (canKeepResult == CanKeepResult::No_DeleteSubtree) {
    // throw away the current node and its subtrees
    for (size_t i = 0; i < (size_t)increment; i++) {
      CHECK(!subTrees_.empty());
      subTrees_.pop();
    }
  }

  // Debug print
  // std::cout << "Must keep node: "
  //          << verilog::NodeEnumToString(static_cast<verilog::NodeEnum>(
  //                 subTrees_.top().get()->Tag().tag))
  //          << std::endl;
  // for (const auto &child :
  //     static_cast<const verible::SyntaxTreeNode *>(subTrees_.top().get())
  //         ->children()) {
  //  std::cout << "\t\t\t"
  //            << verilog::NodeEnumToString(
  //                   static_cast<verilog::NodeEnum>(child->Tag().tag))
  //            << std::endl;
  //}
}

verible::SymbolPtr FilterSymbolsVerilogTree(
    const verible::Symbol &root, const std::vector<FilteringRulePtr> &rules) {
  VerilogTreeFilter filter(rules);
  root.Accept(&filter);
  return filter.GetFilteredTree();
}

}  // namespace verilog
