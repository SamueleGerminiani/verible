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

#include "verilog/CST/constraints.h"

#include <vector>

#include "common/analysis/syntax_tree_search.h"
#include "common/text/concrete_syntax_leaf.h"
#include "common/text/concrete_syntax_tree.h"
#include "common/text/symbol.h"
#include "common/text/token_info.h"
#include "common/text/tree_utils.h"
#include "common/util/casts.h"
#include "verilog/CST/identifier.h"
#include "verilog/CST/verilog_matchers.h"  // IWYU pragma: keep
#include "verilog/CST/verilog_nonterminals.h"

namespace verilog {

std::vector<verible::TreeSearchMatch> FindAllConstraintDeclarations(
    const verible::Symbol &root) {
  return verible::SearchSyntaxTree(root, NodekConstraintDeclaration());
}

bool IsOutOfLineConstraintDefinition(const verible::Symbol &symbol) {
  const auto *identifier_symbol =
      verible::GetSubtreeAsSymbol(symbol, NodeEnum::kConstraintDeclaration, 2);

  return IdIsQualified(*identifier_symbol);
}

const verible::TokenInfo *GetSymbolIdentifierFromConstraintDeclaration(
    const verible::Symbol &symbol) {
  const auto *identifier_symbol =
      verible::GetSubtreeAsSymbol(symbol, NodeEnum::kConstraintDeclaration, 2);
  if (!identifier_symbol) return nullptr;
  return &AutoUnwrapIdentifier(*identifier_symbol)->get();
}

}  // namespace verilog
