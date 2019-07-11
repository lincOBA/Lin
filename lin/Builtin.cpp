#include <iostream>
#include <vector>
#include "Ast.h"
#include "Builtin.h"
#include "Lin.hpp"
#include "Utils.hpp"

lin::Value lin_builtin_print(lin::Runtime* rt,
                             std::deque<lin::Context*> ctxChain,
                             std::vector<lin::Value> args) {
    for (auto arg : args) {
        std::cout << valueToStdString(arg);
    }
    return lin::Value(lin::Int, (int)args.size());
}

lin::Value lin_builtin_println(lin::Runtime* rt,
                               std::deque<lin::Context*> ctxChain,
                               std::vector<lin::Value> args) {
    if (args.size() != 0) {
        for (auto arg : args) {
            std::cout << valueToStdString(arg) << "\n";
        }
    } else {
        std::cout << "\n";
    }

    return lin::Value(lin::Int, (int)args.size());
}

lin::Value lin_builtin_input(lin::Runtime* rt,
                             std::deque<lin::Context*> ctxChain,
                             std::vector<lin::Value> args) {
    lin::Value result{lin::String};

    std::string str;
    std::cin >> str;
    result.data = std::make_any<std::string>(std::move(str));
    return result;
}

lin::Value lin_builtin_typeof(lin::Runtime* rt,
                              std::deque<lin::Context*> ctxChain,
                              std::vector<lin::Value> args) {
    if (args.size() != 1) {
        panic("ArgumentError: expects one argument but got %d", args.size());
    }
    lin::Value result(lin::String);
    switch (args[0].type) {
        case lin::Bool:
            result.set<std::string>("bool");
            break;
        case lin::Double:
            result.set<std::string>("double");
            break;
        case lin::Int:
            result.set<std::string>("int");
            break;
        case lin::String:
            result.set<std::string>("string");
            break;
        case lin::Null:
            result.set<std::string>("null");
            break;
        case lin::Char:
            result.set<std::string>("char");
            break;
        case lin::Array:
            result.set<std::string>("array");
            break;
        default:
            panic("TypeError: unknown type!");
    }
    return result;
}

lin::Value lin_builtin_length(lin::Runtime* rt,
                              std::deque<lin::Context*> ctxChain,
                              std::vector<lin::Value> args) {
    if (args.size() != 1) {
        panic("ArgumentError: expects one argument but got %d", args.size());
    }

    if (args[0].isType<lin::String>()) {
        return lin::Value(
            lin::Int, std::make_any<int>(args[0].cast<std::string>().length()));
    }
    if (args[0].isType<lin::Array>()) {
        return lin::Value(
            lin::Int,
            std::make_any<int>(args[0].cast<std::vector<lin::Value>>().size()));
    }

    panic(
        "TypeError: unexpected type of arguments, requires string type or "
        "array type");
}
