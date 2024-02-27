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

#include "verilog/CST/verilog_tree_print_dot.h"

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

class VerilogTreeToDotTextConverter : public verible::SymbolVisitor {
 public:
  explicit VerilogTreeToDotTextConverter();

  void Visit(const verible::SyntaxTreeLeaf &) final;
  void Visit(const verible::SyntaxTreeNode &) final;

  // Returns the DOT text of the tree.
  std::string GetDotText() {
    FinalizeTree();
    std::string ret = dot_.str();
    dot_.str("");
    return ret;
  }
  // Initializes the dot string
  void InitTree() {
    dot_ << "digraph SystemVerilog_tree {" << std::endl;
    dot_ << "node [shape=ellipse];" << std::endl;
  }
  // finalizes the dot string
  void FinalizeTree() { dot_ << "}" << std::endl; }

 protected:
  // buffer for the dot text
  std::stringstream dot_;
  // stack to keep track of ids of the parent nodes
  std::stack<size_t> parent_ids_;
  // id of the next node to be used in the dot text
  size_t next_node_id_ = 0;
};

VerilogTreeToDotTextConverter::VerilogTreeToDotTextConverter() { InitTree(); }

void VerilogTreeToDotTextConverter::Visit(const verible::SyntaxTreeLeaf &leaf) {
  std::string txt = std::string(leaf.get().text());
  std::string tag = std::string(verilog::TokenTypeToString(leaf.Tag().tag));

  std::string tag_info = (txt == tag) ? txt : (tag + ": " + txt);

  // depth of the node in the tree, used for indentation
  size_t depth = parent_ids_.size();

  dot_ << std::string(depth, '\t') << next_node_id_ << " [label=\"" << tag_info
       << "\""
       << " shape=box"
       << "];" << std::endl;
  dot_ << std::string(depth, '\t') << parent_ids_.top() << " -> " << next_node_id_
       << ";" << std::endl;
  next_node_id_++;
}

void VerilogTreeToDotTextConverter::Visit(const verible::SyntaxTreeNode &node) {
  std::string tag_info =
      NodeEnumToString(static_cast<NodeEnum>(node.Tag().tag));

  // depth of the node in the tree, used for indentation
  size_t depth = parent_ids_.size();

  if (parent_ids_.empty()) {
    // this is the root node
    dot_ << next_node_id_ << " [label=\"" << tag_info
         << "\" fontcolor=white style=filled bgcolor=black];" << std::endl;
  } else {
    // this is a non-root node
    dot_ << std::string(depth, '\t') << next_node_id_ << " [label=\"" << tag_info
         << "\"];" << std::endl;
    dot_ << std::string(depth, '\t') << parent_ids_.top() << " -> "
         << next_node_id_ << ";" << std::endl;
  }
  parent_ids_.push(next_node_id_++);
  for (const auto &child : node.children()) {
    if (child) child->Accept(this);
  }
  parent_ids_.pop();
}

std::string ConvertVerilogTreeToDotText(const verible::Symbol &root) {
  VerilogTreeToDotTextConverter converter;
  root.Accept(&converter);
  return converter.GetDotText();
}

}  // namespace verilog
