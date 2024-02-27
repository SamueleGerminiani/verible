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

#include "verilog/CST/verilog_tree_print_terminal.h"

#include <ostream>
#include <sstream>
#include <stack>
#include <utility>

#include "absl/strings/string_view.h"
#include "common/text/concrete_syntax_leaf.h"
#include "common/text/concrete_syntax_tree.h"
#include "common/text/symbol.h"
#include "common/text/token_info.h"
#include "common/util/value_saver.h"
#include "verilog/CST/verilog_nonterminals.h"  // for NodeEnumToString
#include "verilog/parser/verilog_token.h"
#include "verilog/parser/verilog_token_classifications.h"

namespace verilog {

class VerilogTreeToTerminalTextConverter : public verible::SymbolVisitor {
 public:
  explicit VerilogTreeToTerminalTextConverter();

  void Visit(const verible::SyntaxTreeLeaf &) final;
  void Visit(const verible::SyntaxTreeNode &) final;

  std::string GetTerminalText() {
    std::string ret = terminal_.str();
    terminal_.str("");
    return ret;
  }

 protected:
  std::stringstream terminal_;
  // the edges string is used to keep track of the edges of the tree
  std::string edgesStr_;

  // configurable parameters
  // Warning: the vertical connector must be a single character
  const char verticalConnector = '|';
  const std::string horizontalConnector = "`--> ";
  size_t verticalSpaceBetweenNodes = 1;
  const std::string horizontalWhiteSpace = std::string(10, ' ');
};

VerilogTreeToTerminalTextConverter::VerilogTreeToTerminalTextConverter() =
    default;

void VerilogTreeToTerminalTextConverter::Visit(
    const verible::SyntaxTreeLeaf &leaf) {
  std::string txt = std::string(leaf.get().text());
  std::string tag = std::string(verilog::TokenTypeToString(leaf.Tag().tag));

  std::string tag_info = (txt == tag) ? txt : (tag + ": " + txt);

  // print the leaf
  terminal_ << edgesStr_ << horizontalConnector << tag_info << "\n";
  // print the edges
  for (size_t i = 0; i < verticalSpaceBetweenNodes; i++) {
    terminal_ << edgesStr_ << "\n";
  }
}

void VerilogTreeToTerminalTextConverter::Visit(
    const verible::SyntaxTreeNode &node) {
  std::string tag_info =
      NodeEnumToString(static_cast<NodeEnum>(node.Tag().tag));

  // print the node
  terminal_ << edgesStr_ << horizontalConnector << tag_info << "\n";
  if (node.empty()) {
    return;
  }
  // append a new vertical connector for the children of this node
  edgesStr_ += horizontalWhiteSpace + verticalConnector;
  // print the edges, the space between nodes is dictated by the
  // verticalSpaceBetweenNodes parameter
  for (size_t i = 0; i < verticalSpaceBetweenNodes; i++) {
    terminal_ << edgesStr_ << "\n";
  }

  // gather non-null children: we need this because the last child is a special
  // case during printing
  std::vector<const verible::Symbol *> safe_children;
  for (const auto &child : node.children()) {
    if (child) safe_children.push_back(child.get());
  }

  // print the children
  for (size_t i = 1; i <= safe_children.size(); ++i) {
    //  if this is the last child, we need to replace the last appended edge
    //  with white space
    if (i == safe_children.size()) {
      edgesStr_.replace(edgesStr_.size() - (horizontalWhiteSpace.size() + 1),
                        horizontalWhiteSpace.size() + 1,
                        horizontalWhiteSpace.size() + 1, ' ');
    }

    safe_children[i - 1]->Accept(this);
  }

  // reduce the edges string to match the parent
  edgesStr_ =
      edgesStr_.substr(0, edgesStr_.size() - (horizontalWhiteSpace.size() + 1));
}

std::string ConvertVerilogTreeToTerminalText(const verible::Symbol &root) {
  VerilogTreeToTerminalTextConverter converter;
  root.Accept(&converter);
  return converter.GetTerminalText();
}

}  // namespace verilog
