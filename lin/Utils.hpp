#pragma once
#include <any>
#include <deque>
#include <string>
#include "Lin.hpp"

std::string valueToStdString(lin::Value v);

std::string repeatString(int count, const std::string& str);

std::vector<lin::Value> repeatArray(int count, std::vector<lin::Value>&& arr);

template <typename _DesireType, typename... _ArgumentType>
inline bool anyone(_DesireType k, _ArgumentType... args) {
    return ((args == k) || ...);
}

[[noreturn]] void panic(char const* const format, ...);
