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

// Implementation of VerilogTreeCollector

#include "verilog/CST/verilog_tree_collector.h"

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

VerilogTreeCollector::VerilogTreeCollector(
    const std::unordered_set<verilog::NodeEnum> &wantedNodes,
    const std::unordered_set<verilog_tokentype> &wantedLeaves)
    : wantedNodes_(wantedNodes), wantedLeaves_(wantedLeaves) {}

void VerilogTreeCollector::Visit(const verible::SyntaxTreeLeaf &leaf) {
  if (wantedLeaves_.count((verilog_tokentype)leaf.Tag().tag)) {
    collected_.push_back(&leaf);
  }
}

void VerilogTreeCollector::Visit(const verible::SyntaxTreeNode &node) {
  if (wantedNodes_.count(static_cast<verilog::NodeEnum>(node.Tag().tag))) {
    collected_.push_back(&node);
  }
  for (const auto &child : node.children()) {
    if (child) {
      child->Accept(this);
    }
  }
}

std::vector<const verible::Symbol *> CollectSymbolsVerilogTree(
    const verible::Symbol &root,
    const std::unordered_set<verilog::NodeEnum> &wantedNodes,
    const std::unordered_set<verilog_tokentype> &wantedLeaves) {
  VerilogTreeCollector collector(wantedNodes, wantedLeaves);
  root.Accept(&collector);
  return collector.GetCollected();
}

}  // namespace verilog
