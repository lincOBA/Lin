#pragma once

#include <deque>
#include <vector>
#include "Lin.hpp"

lin::Value lin_builtin_print(lin::Runtime* rt,
                             std::deque<lin::Context*> ctxChain,
                             std::vector<lin::Value> args);

lin::Value lin_builtin_println(lin::Runtime* rt,
                               std::deque<lin::Context*> ctxChain,
                               std::vector<lin::Value> args);

lin::Value lin_builtin_input(lin::Runtime* rt,
                             std::deque<lin::Context*> ctxChain,
                             std::vector<lin::Value> args);

lin::Value lin_builtin_typeof(lin::Runtime* rt,
                              std::deque<lin::Context*> ctxChain,
                              std::vector<lin::Value> args);

lin::Value lin_builtin_length(lin::Runtime* rt,
                              std::deque<lin::Context*> ctxChain,
                              std::vector<lin::Value> args);
