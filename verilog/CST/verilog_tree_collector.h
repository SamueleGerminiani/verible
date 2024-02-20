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
#include <unordered_set>

#include "common/text/concrete_syntax_leaf.h"
#include "common/text/concrete_syntax_tree.h"
#include "common/text/symbol.h"
#include "common/text/tree_utils.h"
#include "verilog/parser/verilog_token_enum.h"
#include "verilog_nonterminals.h"

namespace verilog {

class VerilogTreeCollector : public verible::SymbolVisitor {
 public:
  explicit VerilogTreeCollector(
      const std::unordered_set<verilog::NodeEnum> &wantedNodes,
      const std::unordered_set<verilog_tokentype> &wantedLeaves);

  void Visit(const verible::SyntaxTreeLeaf &) final;
  void Visit(const verible::SyntaxTreeNode &) final;

  std::vector<const verible::Symbol *> &GetCollected() { return collected_; }

 private:
  std::vector<const verible::Symbol *> collected_;
  const std::unordered_set<verilog::NodeEnum> &wantedNodes_;
  const std::unordered_set<verilog_tokentype> &wantedLeaves_;
};

// Collect symbols of specific types from a syntax tree.
std::vector<const verible::Symbol *> CollectSymbolsVerilogTree(
    const verible::Symbol &root,
    const std::unordered_set<verilog::NodeEnum> &wantedNodes,
    const std::unordered_set<verilog_tokentype> &wantedLeaves);

}  // namespace verilog

