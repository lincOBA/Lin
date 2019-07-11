#include "Builtin.h"
#include "Lin.hpp"
#include "Utils.hpp"

namespace lin {

Context::~Context() {
    for (auto v : vars) {
        delete v.second;
    }
}

Runtime::Runtime() {
    builtin["print"] = &lin_builtin_print;
    builtin["println"] = &lin_builtin_println;
    builtin["typeof"] = &lin_builtin_typeof;
    builtin["input"] = &lin_builtin_input;
    builtin["length"] = &lin_builtin_length;
}

bool Runtime::hasBuiltinFunction(const std::string& name) {
    return builtin.count(name) == 1;
}

Runtime::BuiltinFuncType Runtime::getBuiltinFunction(const std::string& name) {
    if (auto res = builtin.find(name); res != builtin.end()) {
        return res->second;
    }
    return builtin[name];
}

void Runtime::addStatement(Statement* stmt) { stmts.push_back(stmt); }

std::vector<Statement*> Runtime::getStatements() { return stmts; }

bool Context::hasVariable(const std::string& identName) {
    return vars.count(identName) == 1;
}

void Context::createVariable(const std::string& identName, Value value) {
    auto* var = new Variable;
    var->name = identName;
    var->value = value;
    vars.emplace(identName, var);
}

Variable* Context::getVariable(const std::string& identName) {
    if (auto res = vars.find(identName); res != vars.end()) {
        return res->second;
    }
    return nullptr;
}

void Context::addFunction(const std::string& name, Function* f) {
    funcs.insert(std::make_pair(name, f));
}

bool Context::hasFunction(const std::string& name) {
    return funcs.count(name) == 1;
}

Function* Context::getFunction(const std::string& name) {
    if (auto f = funcs.find(name); f != funcs.end()) {
        return f->second;
    }
    return nullptr;
}

Value Value::operator+(Value rhs) {
    Value result;
    // Basic
    if (isType<lin::Int>() && rhs.isType<lin::Int>()) {
        result.type = lin::Int;
        result.data = cast<int>() + rhs.cast<int>();
    } else if (isType<lin::Double>() && rhs.isType<lin::Double>()) {
        result.type = lin::Double;
        result.data = cast<double>() + rhs.cast<double>();
    } else if (isType<lin::Int>() && rhs.isType<lin::Double>()) {
        result.type = lin::Double;
        result.data = cast<int>() + rhs.cast<double>();
    } else if (isType<lin::Double>() && rhs.isType<lin::Int>()) {
        result.type = lin::Double;
        result.data = cast<double>() + rhs.cast<int>();
    } else if (isType<lin::Char>() && rhs.isType<lin::Int>()) {
        result.type = lin::Char;
        result.data = static_cast<char>(cast<char>() + rhs.cast<int>());
    } else if (isType<lin::Int>() && rhs.isType<lin::Char>()) {
        result.type = lin::Char;
        result.data = static_cast<char>(cast<int>() + rhs.cast<char>());
    } else if (isType<lin::Char>() && rhs.isType<lin::Char>()) {
        result.type = lin::Char;
        result.data = static_cast<char>(cast<char>() + rhs.cast<char>());
    }
    // String
    // One of operands has string type, we say the result value was a string
    else if (isType<lin::String>() || rhs.isType<lin::String>()) {
        result.type = lin::String;
        result.data = valueToStdString(*this) + valueToStdString(rhs);
    }
    // Array
    else if (isType<lin::Array>()) {
        result.type = lin::Array;
        auto resultArr = this->cast<std::vector<lin::Value>>();
        resultArr.push_back(rhs);
        result.data = resultArr;
    } else if (rhs.isType<lin::Array>()) {
        result.type = lin::Array;
        auto resultArr = rhs.cast<std::vector<lin::Value>>();
        resultArr.push_back(*this);
        result.data = resultArr;
    }
    // Invalid
    else {
        panic("TypeError: unexpected arguments of operator +");
    }
    return result;
}

Value Value::operator-(Value rhs) {
    Value result;
    if (isType<lin::Int>() && rhs.isType<lin::Int>()) {
        result.type = lin::Int;
        result.data = cast<int>() - rhs.cast<int>();
    } else if (isType<lin::Double>() && rhs.isType<lin::Double>()) {
        result.type = lin::Double;
        result.data = cast<double>() - rhs.cast<double>();
    } else if (isType<lin::Int>() && rhs.isType<lin::Double>()) {
        result.type = lin::Double;
        result.data = cast<int>() - rhs.cast<double>();
    } else if (isType<lin::Double>() && rhs.isType<lin::Int>()) {
        result.type = lin::Double;
        result.data = cast<double>() - rhs.cast<int>();
    } else if (isType<lin::Char>() && rhs.isType<lin::Int>()) {
        result.type = lin::Char;
        result.data = static_cast<char>(cast<char>() - rhs.cast<int>());
    } else if (isType<lin::Int>() && rhs.isType<lin::Char>()) {
        result.type = lin::Char;
        result.data = static_cast<char>(cast<int>() - rhs.cast<char>());
    } else if (isType<lin::Char>() && rhs.isType<lin::Char>()) {
        result.type = lin::Char;
        result.data = static_cast<char>(cast<char>() - rhs.cast<char>());
    } else {
        panic("TypeError: unexpected arguments of operator -");
    }

    return result;
}

Value Value::operator*(Value rhs) {
    Value result;
    // Basic
    if (isType<lin::Int>() && rhs.isType<lin::Int>()) {
        result.type = lin::Int;
        result.data = cast<int>() * rhs.cast<int>();
    } else if (isType<lin::Double>() && rhs.isType<lin::Double>()) {
        result.type = lin::Double;
        result.data = cast<double>() * rhs.cast<double>();
    } else if (isType<lin::Int>() && rhs.isType<lin::Double>()) {
        result.type = lin::Double;
        result.data = cast<int>() * rhs.cast<double>();
    } else if (isType<lin::Double>() && rhs.isType<lin::Int>()) {
        result.type = lin::Double;
        result.data = cast<double>() * rhs.cast<int>();
    }
    // String
    else if (isType<lin::String>() && rhs.isType<lin::Int>()) {
        result.type = lin::String;
        result.data = repeatString(rhs.cast<int>(), cast<std::string>());
    } else if (isType<lin::Int>() && rhs.isType<lin::String>()) {
        result.type = lin::String;
        result.data = repeatString(cast<int>(), rhs.cast<std::string>());
    }
    // Array
    else if (isType<lin::Int>() && rhs.isType<lin::Array>()) {
        result.type = lin::Array;
        result.data = repeatArray(
            cast<int>(), std::move(rhs.cast<std::vector<lin::Value>>()));
    } else if (isType<lin::Array>() && rhs.isType<lin::Int>()) {
        result.type = lin::Array;
        result.data = repeatArray(rhs.cast<int>(),
                                  std::move(cast<std::vector<lin::Value>>()));
    } else {
        panic("TypeError: unexpected arguments of operator *");
    }
    return result;
}

Value Value::operator/(Value rhs) {
    Value result;
    if (isType<lin::Int>() && rhs.isType<lin::Int>()) {
        result.type = lin::Int;
        result.data = cast<int>() / rhs.cast<int>();
    } else if (isType<lin::Double>() && rhs.isType<lin::Double>()) {
        result.type = lin::Double;
        result.data = cast<double>() / rhs.cast<double>();
    } else if (isType<lin::Int>() && rhs.isType<lin::Double>()) {
        result.type = lin::Double;
        result.data = cast<int>() / rhs.cast<double>();
    } else if (isType<lin::Double>() && rhs.isType<lin::Int>()) {
        result.type = lin::Double;
        result.data = cast<double>() / rhs.cast<int>();
    } else {
        panic("TypeError: unexpected arguments of operator /");
    }
    return result;
}

Value Value::operator%(Value rhs) {
    Value result;
    if (isType<lin::Int>() && rhs.isType<lin::Int>()) {
        result.type = lin::Int;
        result.data = (int)cast<int>() % rhs.cast<int>();
    } else {
        panic("TypeError: unexpected arguments of operator %");
    }
    return result;
}

Value Value::operator&&(Value rhs) {
    Value result;
    if (isType<lin::Bool>() && rhs.isType<lin::Bool>()) {
        result.type = lin::Bool;
        result.data = (cast<bool>() && rhs.cast<bool>());
    } else {
        panic("TypeError: unexpected arguments of operator &&");
    }
    return result;
}

Value Value::operator||(Value rhs) {
    Value result;
    if (isType<lin::Bool>() && rhs.isType<lin::Bool>()) {
        result.type = lin::Bool;
        result.data = (cast<bool>() || rhs.cast<bool>());
    } else {
        panic("TypeError: unexpected arguments of operator ||");
    }
    return result;
}

Value Value::operator==(Value rhs) {
    Value result;
    if (isType<lin::Int>() && rhs.isType<lin::Int>()) {
        result.type = lin::Bool;
        result.data = (cast<int>() == rhs.cast<int>());
    } else if (isType<lin::Double>() && rhs.isType<lin::Double>()) {
        result.type = lin::Bool;
        result.data = (cast<double>() == rhs.cast<double>());
    } else if (isType<lin::String>() && rhs.isType<lin::String>()) {
        result.type = lin::Bool;
        std::string lhsStr, rhsStr;
        lhsStr = valueToStdString(*this);
        rhsStr = valueToStdString(rhs);
        result.data = (lhsStr == rhsStr);
    } else if (isType<lin::Bool>() && rhs.isType<lin::Bool>()) {
        result.type = lin::Bool;
        result.data = (cast<bool>() == rhs.cast<bool>());
    } else if (this->type == lin::Null && rhs.type == lin::Null) {
        result.type = lin::Bool;
        result.data = std::make_any<bool>(true);
    } else if (isType<lin::Char>() && rhs.isType<lin::Char>()) {
        result.type = lin::Bool;
        result.data = (cast<char>() == rhs.cast<char>());
    } else {
        panic("TypeError: unexpected arguments of operator ==");
    }
    return result;
}

Value Value::operator!=(Value rhs) {
    Value result;
    if (isType<lin::Int>() && rhs.isType<lin::Int>()) {
        result.type = lin::Bool;
        result.data = (cast<int>() != rhs.cast<int>());
    } else if (isType<lin::Double>() && rhs.isType<lin::Double>()) {
        result.type = lin::Bool;
        result.data = (cast<double>() != rhs.cast<double>());
    } else if (isType<lin::String>() && rhs.isType<lin::String>()) {
        result.type = lin::Bool;
        std::string lhsStr, rhsStr;
        lhsStr = valueToStdString(*this);
        rhsStr = valueToStdString(rhs);
        result.data = (lhsStr != rhsStr);
    } else if (isType<lin::Bool>() && rhs.isType<lin::Bool>()) {
        result.type = lin::Bool;
        result.data = (cast<bool>() != rhs.cast<bool>());
    } else if (this->type == lin::Null && rhs.type == lin::Null) {
        result.type = lin::Bool;
        result.data = std::make_any<bool>(false);
    } else if (isType<lin::Char>() && rhs.isType<lin::Char>()) {
        result.type = lin::Bool;
        result.data = (cast<char>() != rhs.cast<char>());
    } else {
        panic("TypeError: unexpected arguments of operator !=");
    }
    return result;
}

Value Value::operator>(Value rhs) {
    Value result;
    if (isType<lin::Int>() && rhs.isType<lin::Int>()) {
        result.type = lin::Bool;
        result.data = (cast<int>() > rhs.cast<int>());
    } else if (isType<lin::Double>() && rhs.isType<lin::Double>()) {
        result.type = lin::Bool;
        result.data = (cast<double>() > rhs.cast<double>());
    } else if (isType<lin::String>() && rhs.isType<lin::String>()) {
        result.type = lin::Bool;
        std::string lhsStr, rhsStr;
        lhsStr = valueToStdString(*this);
        rhsStr = valueToStdString(rhs);
        result.data = (lhsStr > rhsStr);
    } else if (isType<lin::Char>() && rhs.isType<lin::Char>()) {
        result.type = lin::Bool;
        result.data = (cast<char>() > rhs.cast<char>());
    } else {
        panic("TypeError: unexpected arguments of operator >");
    }
    return result;
}

Value Value::operator>=(Value rhs) {
    Value result;
    if (isType<lin::Int>() && rhs.isType<lin::Int>()) {
        result.type = lin::Bool;
        result.data = (cast<int>() >= rhs.cast<int>());
    } else if (isType<lin::Double>() && rhs.isType<lin::Double>()) {
        result.type = lin::Bool;
        result.data = (cast<double>() >= rhs.cast<double>());
    } else if (isType<lin::String>() && rhs.isType<lin::String>()) {
        result.type = lin::Bool;
        std::string lhsStr, rhsStr;
        lhsStr = valueToStdString(*this);
        rhsStr = valueToStdString(rhs);
        result.data = (lhsStr >= rhsStr);
    } else if (isType<lin::Char>() && rhs.isType<lin::Char>()) {
        result.type = lin::Bool;
        result.data = (cast<char>() >= rhs.cast<char>());
    } else {
        panic("TypeError: unexpected arguments of operator >=");
    }
    return result;
}

Value Value::operator<(Value rhs) {
    Value result;
    if (isType<lin::Int>() && rhs.isType<lin::Int>()) {
        result.type = lin::Bool;
        result.data = (cast<int>() < rhs.cast<int>());
    } else if (isType<lin::Double>() && rhs.isType<lin::Double>()) {
        result.type = lin::Bool;
        result.data = (cast<double>() < rhs.cast<double>());
    } else if (isType<lin::String>() && rhs.isType<lin::String>()) {
        result.type = lin::Bool;
        std::string lhsStr, rhsStr;
        lhsStr = valueToStdString(*this);
        rhsStr = valueToStdString(rhs);
        result.data = (lhsStr < rhsStr);
    } else if (isType<lin::Char>() && rhs.isType<lin::Char>()) {
        result.type = lin::Bool;
        result.data = (cast<char>() < rhs.cast<char>());
    } else {
        panic("TypeError: unexpected arguments of operator <");
    }
    return result;
}

Value Value::operator<=(Value rhs) {
    Value result;
    if (isType<lin::Int>() && rhs.isType<lin::Int>()) {
        result.type = lin::Bool;
        result.data = (cast<int>() <= rhs.cast<int>());
    } else if (isType<lin::Double>() && rhs.isType<lin::Double>()) {
        result.type = lin::Bool;
        result.data = (cast<double>() <= rhs.cast<double>());
    } else if (isType<lin::String>() && rhs.isType<lin::String>()) {
        result.type = lin::Bool;
        std::string lhsStr, rhsStr;
        lhsStr = valueToStdString(*this);
        rhsStr = valueToStdString(rhs);
        result.data = (lhsStr <= rhsStr);
    } else if (isType<lin::Char>() && rhs.isType<lin::Char>()) {
        result.type = lin::Bool;
        result.data = (cast<char>() <= rhs.cast<char>());
    } else {
        panic("TypeError: unexpected arguments of operator <=");
    }
    return result;
}

Value Value::operator&(Value rhs) {
    Value result;
    if (isType<lin::Int>() && rhs.isType<lin::Int>()) {
        result.type = lin::Int;
        result.data = (cast<int>() & rhs.cast<int>());
    } else {
        panic("TypeError: unexpected arguments of operator &");
    }
    return result;
}

Value Value::operator|(Value rhs) {
    Value result;
    if (isType<lin::Int>() && rhs.isType<lin::Int>()) {
        result.type = lin::Int;
        result.data = (cast<int>() | rhs.cast<int>());
    } else {
        panic("TypeError: unexpected arguments of operator |");
    }
    return result;
}

}  // namespace lin
