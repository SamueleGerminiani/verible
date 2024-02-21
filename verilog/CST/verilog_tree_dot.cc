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

#include "verilog/CST/verilog_tree_dot.h"

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

  std::string GetDotText() {
    FinalizeTree();
    std::string ret = dot_.str();
    dot_.str("");
    return ret;
  }
  void InitTree() {
    dot_ << "digraph verilog_tree {" << std::endl;
    dot_ << "node [shape=ellipse];" << std::endl;
  }
  void FinalizeTree() { dot_ << "}" << std::endl; }

 protected:
  std::stringstream dot_;
  std::stack<size_t> parentIDs_;
  size_t nextNodeID_ = 0;
};

VerilogTreeToDotTextConverter::VerilogTreeToDotTextConverter() { InitTree(); }

void VerilogTreeToDotTextConverter::Visit(const verible::SyntaxTreeLeaf &leaf) {
  std::string txt = std::string(leaf.get().text());
  std::string tag = std::string(verilog::TokenTypeToString(leaf.Tag().tag));

  std::string tag_info = (txt == tag) ? txt : (tag + ": " + txt);

  dot_ << nextNodeID_ << " [label=\"" << tag_info << "\""
       << " shape=box"
       << "];" << std::endl;
  dot_ << parentIDs_.top() << " -> " << nextNodeID_ << ";" << std::endl;
  nextNodeID_++;
}

void VerilogTreeToDotTextConverter::Visit(const verible::SyntaxTreeNode &node) {
  std::string tag_info =
      NodeEnumToString(static_cast<NodeEnum>(node.Tag().tag));

  if (parentIDs_.empty()) {
    dot_ << nextNodeID_ << " [label=\"" << tag_info
         << "\" fontcolor=white style=filled bgcolor=black];" << std::endl;
  } else {
    dot_ << nextNodeID_ << " [label=\"" << tag_info << "\"];" << std::endl;
    dot_ << parentIDs_.top() << " -> " << nextNodeID_ << ";" << std::endl;
  }

  parentIDs_.push(nextNodeID_++);
  for (const auto &child : node.children()) {
    if (child) child->Accept(this);
  }
  parentIDs_.pop();
}

std::string ConvertVerilogTreeToDot(const verible::Symbol &root) {
  VerilogTreeToDotTextConverter converter;
  root.Accept(&converter);
  return converter.GetDotText();
}

}  // namespace verilog
