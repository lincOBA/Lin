#pragma once
#include <memory>
#include "Lin.hpp"
#include "Parser.h"

class Interpreter {
public:
    explicit Interpreter(const std::string& fileName);
    ~Interpreter();

public:
    void execute();

public:
    static void enterContext(std::deque<lin::Context*>& ctxChain);

    static void leaveContext(std::deque<lin::Context*>& ctxChain);

    static lin::Value callFunction(lin::Runtime* rt, lin::Function* f,
                                   std::deque<lin::Context*> previousCtxChain,
                                   std::vector<Expression*> args);

    static lin::Value calcBinaryExpr(lin::Value lhs, Token opt, Value rhs,
                                     int line, int column);

    static lin::Value calcUnaryExpr(lin::Value& lhs, Token opt, int line,
                                    int column);
    static lin::Value assignSwitch(Token opt, lin::Value lhs, lin::Value rhs);

private:
    void parseCommandOption(int argc, char* argv) {}

private:
    std::deque<lin::Context*> ctxChain;
    lin::Runtime* rt;
    Parser* p;
};
