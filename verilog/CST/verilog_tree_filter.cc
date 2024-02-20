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

VerilogTreeFilter::VerilogTreeFilter(
    const std::vector<const verible::Symbol *> &toKeep)
    : toKeep_(toKeep) {}

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
  size_t currentSubTreeSize = subTrees_.size();

  for (const auto &child : node.children()) {
    if (child) {
      child->Accept(this);
    }
  }

  int increment = subTrees_.size() - currentSubTreeSize;

  if (increment >= 2 || MustKeep(node)) {
    verible::SymbolPtr thisSubstreeRoot =
        verible::MakeTaggedNode(node.Tag().tag);

    for (size_t i = 0; i < increment; i++) {
      assert(!subTrees_.empty() && "subTrees_ stack is empty");
      thisSubstreeRoot = verible::ExtendNode(thisSubstreeRoot, subTrees_.top());
      subTrees_.pop();
    }

    subTrees_.push(std::move(thisSubstreeRoot));

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
}

verible::SymbolPtr FilterSymbolsVerilogTree(
    const verible::Symbol &root,
    const std::vector<const verible::Symbol *> &toKeep) {
  // VerilogTreeFilter *filter = new VerilogTreeFilter(toKeep);
  // root.Accept(filter);
  // return filter->GetFilteredTree();

  VerilogTreeFilter filter(toKeep);
  root.Accept(&filter);
  return filter.GetFilteredTree();
}

}  // namespace verilog
