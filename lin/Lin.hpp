/*
@startuml
Statement <--* Block
Block  <--* Function
Expression <--* Function

Value <--* ExecResult
Value <--* Variable

Context *--> Variable
Context *--> Function

Context <|-- Runtime

Runtime *--> BuiltinFuncType
Runtime *--> Statement

@enduml
 */
#pragma once

#include <any>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

struct Statement;
struct Expression;

namespace lin {
enum ValueType { Int, Double, String, Bool, Char, Null, Array };
enum ExecutionResultType { ExecNormal, ExecReturn, ExecBreak, ExecContinue };

struct Block {
    explicit Block() = default;

    std::vector<Statement*> stmts;
};

struct Function {
    explicit Function() = default;
    ~Function() { delete block; }

    std::string name;
    std::vector<std::string> params;
    Block* block{};
    Expression* retExpr{};
};

struct Value {
    explicit Value() {}
    explicit Value(lin::ValueType type) : type(type) {}
    explicit Value(lin::ValueType type, std::any data)
        : type(type), data(std::move(data)) {}

    template <int _LinType>
    inline bool isType();

    template <typename _CastingType>
    inline _CastingType cast();

    template <typename _DataType>
    inline void set(_DataType data);

    Value operator+(Value rhs);
    Value operator-(Value rhs);
    Value operator*(Value rhs);
    Value operator/(Value rhs);
    Value operator%(Value rhs);

    Value operator&&(Value rhs);
    Value operator||(Value rhs);

    Value operator==(Value rhs);
    Value operator!=(Value rhs);
    Value operator>(Value rhs);
    Value operator>=(Value rhs);
    Value operator<(Value rhs);
    Value operator<=(Value rhs);

    Value operator&(Value rhs);
    Value operator|(Value rhs);

    lin::ValueType type{};
    std::any data;
};

struct ExecResult {
    explicit ExecResult() : execType(ExecNormal) {}
    explicit ExecResult(ExecutionResultType execType) : execType(execType) {}
    explicit ExecResult(ExecutionResultType execType, Value retValue)
        : execType(execType), retValue(retValue) {}

    ExecutionResultType execType;
    Value retValue;
};

struct Variable {
    explicit Variable() = default;

    std::string name;
    Value value;
};

class Context {
public:
    explicit Context() = default;
    virtual ~Context();

    bool hasVariable(const std::string& identName);
    void createVariable(const std::string& identName, Value value);
    Variable* getVariable(const std::string& identName);

    void addFunction(const std::string& name, Function* f);
    bool hasFunction(const std::string& name);
    Function* getFunction(const std::string& name);

private:
    std::unordered_map<std::string, Variable*> vars;
    std::unordered_map<std::string, Function*> funcs;
};

class Runtime : public Context {
    using BuiltinFuncType = Value (*)(Runtime*, std::deque<Context*>,
                                      std::vector<Value>);

public:
    explicit Runtime();

    bool hasBuiltinFunction(const std::string& name);
    BuiltinFuncType getBuiltinFunction(const std::string& name);

    void addStatement(Statement* stmt);
    std::vector<Statement*> getStatements();

private:
    std::unordered_map<std::string, BuiltinFuncType> builtin;
    std::vector<Statement*> stmts;
};

template <int _LinType>
inline bool Value::isType() {
    return this->type == _LinType;
}

template <typename _CastingType>
inline _CastingType Value::cast() {
    return std::any_cast<_CastingType>(data);
}

template <typename _DataType>
inline void Value::set(_DataType data) {
    this->data = std::make_any<_DataType>(std::move(data));
}
}  // namespace lin
