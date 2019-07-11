#include <cstdarg>
#include "Lin.hpp"
#include "Utils.hpp"

std::string valueToStdString(lin::Value v) {
    switch (v.type) {
        case lin::Bool:
            return v.cast<bool>() ? "true" : "false";
        case lin::Double:
            return std::to_string(v.cast<double>());
        case lin::Int:
            return std::to_string(v.cast<int>());
        case lin::Null:
            return "null";
        case lin::Char: {
            std::string str;
            str += v.cast<char>();
            return str;
        }
        case lin::Array: {
            std::string str = "[";
            auto elements = v.cast<std::vector<lin::Value>>();
            for (int i = 0; i < elements.size(); i++) {
                str += valueToStdString(elements[i]);

                if (i != elements.size() - 1) {
                    str += ",";
                }
            }
            str += "]";
            return str;
        }
        case lin::String:
            return v.cast<std::string>();
    }
    return "unknown";
}

std::string repeatString(int count, const std::string& str) {
    std::string result;
    for (int i = 0; i < count; i++) {
        result += str;
    }
    return result;
}

std::vector<lin::Value> repeatArray(int count, std::vector<lin::Value>&& arr) {
    std::vector<lin::Value> result;
    for (int i = 0; i < count; i++) {
        result.insert(result.begin(), arr.begin(), arr.end());
    }
    return result;
}

[[noreturn]] void panic(char const* const format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    exit(EXIT_FAILURE);
}
