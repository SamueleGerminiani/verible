#!/bin/bash
bazel-6.5.0 build -c opt //... && bazel-bin/verilog/tools/syntax/verible-verilog-syntax work/test.sv --printtree
