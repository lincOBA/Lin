#!/bin/sh
g++ -std=c++17 Main.cpp Parser.cpp Utils.cpp Interpreter.cpp Lin.cpp Builtin.cpp Lin.hpp Utils.hpp Ast.cpp -o lin
