#include <deque>
#include <memory>
#include <vector>
#include "Ast.h"
#include "Builtin.h"
#include "Interpreter.h"
#include "Lin.hpp"
#include "Utils.hpp"

//===----------------------------------------------------------------------===//
// Lin interpreter, as its name described, will interpret all statements within
// top-level source file. This part defines internal functions of interpreter
// and leaves actually statement performing later.
//===----------------------------------------------------------------------===//
Interpreter::Interpreter(const std::string& fileName)
    : p(new Parser(fileName)), rt(new lin::Runtime) {}

Interpreter::~Interpreter() {
    delete p;
    delete rt;
}

lin::Value Expression::eval(lin::Runtime* rt,
                            std::deque<lin::Context*> ctxChain) {
    panic(
        "RuntimeError: can not evaluate abstract expression at line %d, column "
        "%d\n",
        line, column);
}

lin::ExecResult Statement::interpret(lin::Runtime* rt,
                                     std::deque<lin::Context*> ctxChain) {
    panic(
        "RuntimeError: can not interpret abstract statement at line %d, column "
        "%d\n",
        line, column);
}

void Interpreter::execute() {
    this->p->parse(this->rt);
    this->ctxChain.push_back(new lin::Context);

    auto stmts = rt->getStatements();
    for (auto stmt : stmts) {
        // std::cout << stmt->astString() << "\n";
        stmt->interpret(rt, ctxChain);
    }
}

void Interpreter::enterContext(std::deque<lin::Context*>& ctxChain) {
    auto* tempContext = new Context;
    ctxChain.push_back(tempContext);
}

void Interpreter::leaveContext(std::deque<lin::Context*>& ctxChain) {
    auto* tempContext = ctxChain.back();
    ctxChain.pop_back();
    delete tempContext;
}

lin::Value Interpreter::callFunction(lin::Runtime* rt, lin::Function* f,
                                     std::deque<lin::Context*> previousCtxChain,
                                     std::vector<Expression*> args) {
    // Execute user defined function
    std::deque<lin::Context*> funcCtxChain;
    Interpreter::enterContext(funcCtxChain);

    auto* funcCtx = funcCtxChain.back();
    for (int i = 0; i < f->params.size(); i++) {
        std::string paramName = f->params[i];
        // Evaluate argument values from previouse context chain
        lin::Value argValue = args[i]->eval(rt, previousCtxChain);
        funcCtx->createVariable(f->params[i], argValue);
    }

    lin::ExecResult ret(lin::ExecNormal);
    for (auto& stmt : f->block->stmts) {
        ret = stmt->interpret(rt, funcCtxChain);
        if (ret.execType == lin::ExecReturn) {
            break;
        }
    }
    Interpreter::leaveContext(funcCtxChain);

    return ret.retValue;
}

lin::Value Interpreter::calcUnaryExpr(lin::Value& lhs, Token opt, int line,
                                      int column) {
    switch (opt) {
        case TK_MINUS:
            switch (lhs.type) {
                case lin::Int:
                    return lin::Value(lin::Int, -std::any_cast<int>(lhs.data));
                case lin::Double:
                    return lin::Value(lin::Double,
                                      -std::any_cast<double>(lhs.data));
                default:
                    panic(
                        "TypeError: invalid operand type for operator "
                        "-(negative) at line %d, col %d\n",
                        line, column);
            }
            break;
        case TK_LOGNOT:
            if (lhs.type == lin::Bool) {
                return lin::Value(lin::Bool, !std::any_cast<bool>(lhs.data));
            } else {
                panic(
                    "TypeError: invalid operand type for operator "
                    "!(logical not) at line %d, col %d\n",
                    line, column);
            }
            break;
        case TK_BITNOT:
            if (lhs.type == lin::Int) {
                return lin::Value(lin::Int, ~std::any_cast<int>(lhs.data));
            } else {
                panic(
                    "TypeError: invalid operand type for operator "
                    "~(bit not) at line %d, col %d\n",
                    line, column);
            }
            break;
    }

    return lhs;
}

lin::Value Interpreter::calcBinaryExpr(lin::Value lhs, Token opt, Value rhs,
                                       int line, int column) {
    lin::Value result{lin::Null};

    switch (opt) {
        case TK_PLUS:
            result = (lhs + rhs);
            break;
        case TK_MINUS:
            result = (lhs - rhs);
            break;
        case TK_TIMES:
            result = (lhs * rhs);
            break;
        case TK_DIV:
            result = (lhs / rhs);
            break;
        case TK_MOD:
            result = (lhs % rhs);
            break;
        case TK_LOGAND:
            result = (lhs && rhs);
            break;
        case TK_LOGOR:
            result = (lhs || rhs);
            break;
        case TK_EQ:
            result = (lhs == rhs);
            break;
        case TK_NE:
            result = (lhs != rhs);
            break;
        case TK_GT:
            result = (lhs > rhs);
            break;
        case TK_GE:
            result = (lhs >= rhs);
            break;
        case TK_LT:
            result = (lhs < rhs);
            break;
        case TK_LE:
            result = (lhs <= rhs);
            break;
        case TK_BITAND:
            result = (lhs & rhs);
            break;
        case TK_BITOR:
            result = (lhs | rhs);
            break;
    }

    return result;
}

lin::Value Interpreter::assignSwitch(Token opt, lin::Value lhs,
                                     lin::Value rhs) {
    switch (opt) {
        case TK_ASSIGN:
            return rhs;
        case TK_PLUS_AGN:
            return lhs + rhs;
        case TK_MINUS_AGN:
            return lhs - rhs;
        case TK_TIMES_AGN:
            return lhs * rhs;
        case TK_DIV_AGN:
            return lhs / rhs;
        case TK_MOD_AGN:
            return lhs % rhs;
        default:
            panic("InteralError: unexpects branch reached");
    }
}

//===----------------------------------------------------------------------===//
// Interpret various statements within given runtime and context chain. Runtime
// holds all necessary data that widely used in every context. Context chain
// saves a linked contexts of current execution flow.
//===----------------------------------------------------------------------===//
lin::ExecResult IfStmt::interpret(lin::Runtime* rt,
                                  std::deque<lin::Context*> ctxChain) {
    lin::ExecResult ret(lin::ExecNormal);
    Value cond = this->cond->eval(rt, ctxChain);
    if (!cond.isType<lin::Bool>()) {
        panic(
            "TypeError: expects bool type in while condition at line %d, "
            "col %d\n",
            line, column);
    }
    if (true == cond.cast<bool>()) {
        Interpreter::enterContext(ctxChain);
        for (auto& stmt : block->stmts) {
            // std::cout << stmt->astString() << "\n";
            ret = stmt->interpret(rt, ctxChain);
            if (ret.execType == lin::ExecReturn) {
                break;
            } else if (ret.execType == lin::ExecBreak) {
                break;
            } else if (ret.execType == lin::ExecContinue) {
                break;
            }
        }
        Interpreter::leaveContext(ctxChain);
    } else {
        if (elseBlock != nullptr) {
            Interpreter::enterContext(ctxChain);
            for (auto& elseStmt : elseBlock->stmts) {
                // std::cout << stmt->astString() << "\n";
                ret = elseStmt->interpret(rt, ctxChain);
                if (ret.execType == lin::ExecReturn) {
                    break;
                } else if (ret.execType == lin::ExecBreak) {
                    break;
                } else if (ret.execType == lin::ExecContinue) {
                    break;
                }
            }
            Interpreter::leaveContext(ctxChain);
        }
    }
    return ret;
}

lin::ExecResult WhileStmt::interpret(lin::Runtime* rt,
                                     std::deque<lin::Context*> ctxChain) {
    lin::ExecResult ret;
    Value cond = this->cond->eval(rt, ctxChain);

    Interpreter::enterContext(ctxChain);
    while (true == cond.cast<bool>()) {
        for (auto& stmt : block->stmts) {
            // std::cout << stmt->astString() << "\n";
            ret = stmt->interpret(rt, ctxChain);
            if (ret.execType == lin::ExecReturn) {
                goto outside;
            } else if (ret.execType == lin::ExecBreak) {
                // Disable propagating through the whole chain
                ret.execType = lin::ExecNormal;
                goto outside;
            } else if (ret.execType == lin::ExecContinue) {
                // Disable propagating through the whole chain
                ret.execType = lin::ExecNormal;
                break;
            }
        }
        cond = this->cond->eval(rt, ctxChain);
        if (!cond.isType<lin::Bool>()) {
            panic(
                "TypeError: expects bool type in while condition at line %d, "
                "col %d\n",
                line, column);
        }
    }

outside:
    Interpreter::leaveContext(ctxChain);
    return ret;
}

lin::ExecResult ExpressionStmt::interpret(lin::Runtime* rt,
                                          std::deque<lin::Context*> ctxChain) {
    // std::cout << this->expr->astString() << "\n";
    this->expr->eval(rt, ctxChain);
    return lin::ExecResult(lin::ExecNormal);
}

lin::ExecResult ReturnStmt::interpret(lin::Runtime* rt,
                                      std::deque<lin::Context*> ctxChain) {
    Value retVal = this->ret->eval(rt, ctxChain);
    return lin::ExecResult(lin::ExecReturn, retVal);
}

lin::ExecResult BreakStmt::interpret(lin::Runtime* rt,
                                     std::deque<lin::Context*> ctxChain) {
    return lin::ExecResult(lin::ExecBreak);
}

lin::ExecResult ContinueStmt::interpret(lin::Runtime* rt,
                                        std::deque<lin::Context*> ctxChain) {
    return lin::ExecResult(lin::ExecContinue);
}

//===----------------------------------------------------------------------===//
// Evaulate all expressions and return a lin::Value structure, this object
// contains evaulated data and corresponding data type, it represents sorts
// of(also all) data type in lin and can get value by interpreter directly.
//===----------------------------------------------------------------------===//
lin::Value NullExpr::eval(lin::Runtime* rt,
                          std::deque<lin::Context*> ctxChain) {
    return lin::Value(lin::Null);
}

lin::Value BoolExpr::eval(lin::Runtime* rt,
                          std::deque<lin::Context*> ctxChain) {
    return lin::Value(lin::Bool, this->literal);
}

lin::Value CharExpr::eval(lin::Runtime* rt,
                          std::deque<lin::Context*> ctxChain) {
    return lin::Value(lin::Char, this->literal);
}

lin::Value IntExpr::eval(lin::Runtime* rt, std::deque<lin::Context*> ctxChain) {
    return lin::Value(lin::Int, this->literal);
}

lin::Value DoubleExpr::eval(lin::Runtime* rt,
                            std::deque<lin::Context*> ctxChain) {
    return lin::Value(lin::Double, this->literal);
}

lin::Value StringExpr::eval(lin::Runtime* rt,
                            std::deque<lin::Context*> ctxChain) {
    return lin::Value(lin::String, this->literal);
}

lin::Value ArrayExpr::eval(lin::Runtime* rt,
                           std::deque<lin::Context*> ctxChain) {
    std::vector<lin::Value> elements;
    for (auto& e : this->literal) {
        elements.push_back(e->eval(rt, ctxChain));
    }

    return lin::Value(lin::Array, elements);
}

lin::Value IdentExpr::eval(lin::Runtime* rt,
                           std::deque<lin::Context*> ctxChain) {
    for (auto p = ctxChain.crbegin(); p != ctxChain.crend(); ++p) {
        auto* ctx = *p;
        if (auto* var = ctx->getVariable(this->identName); var != nullptr) {
            return var->value;
        }
    }
    panic("RuntimeError: use of undefined variable \"%s\" at line %d, col %d\n",
          identName.c_str(), this->line, this->column);
}

lin::Value IndexExpr::eval(lin::Runtime* rt,
                           std::deque<lin::Context*> ctxChain) {
    for (auto p = ctxChain.crbegin(); p != ctxChain.crend(); ++p) {
        auto* ctx = *p;
        if (auto* var = ctx->getVariable(this->identName); var != nullptr) {
            auto idx = this->index->eval(rt, ctxChain);
            if (!idx.isType<lin::Int>()) {
                panic(
                    "TypeError: expects int type within indexing expression at "
                    "line %d, col %d\n",
                    line, column);
            }
            if (idx.cast<int>() >=
                var->value.cast<std::vector<lin::Value>>().size()) {
                panic("IndexError: index %d out of range at line %d, col %d\n",
                      idx.cast<int>(), line, column);
            }
            return var->value.cast<std::vector<lin::Value>>()[idx.cast<int>()];
        }
    }
    panic("RuntimeError: use of undefined variable \"%s\" at line %d, col %d\n",
          identName.c_str(), this->line, this->column);
}

lin::Value AssignExpr::eval(lin::Runtime* rt,
                            std::deque<lin::Context*> ctxChain) {
    lin::Value rhs = this->rhs->eval(rt, ctxChain);

    if (typeid(*lhs) == typeid(IdentExpr)) {
        std::string identName = dynamic_cast<IdentExpr*>(lhs)->identName;

        for (auto p = ctxChain.crbegin(); p != ctxChain.crend(); ++p) {
            if (auto* var = (*p)->getVariable(identName); var != nullptr) {
                var->value =
                    Interpreter::assignSwitch(this->opt, var->value, rhs);
                return rhs;
            }
        }

        (ctxChain.back())->createVariable(identName, rhs);
    } else if (typeid(*lhs) == typeid(IndexExpr)) {
        std::string identName = dynamic_cast<IndexExpr*>(lhs)->identName;
        lin::Value index =
            dynamic_cast<IndexExpr*>(lhs)->index->eval(rt, ctxChain);
        if (!index.isType<lin::Int>()) {
            panic(
                "TypeError: expects int type when applying indexing "
                "to variable %s at line %d, col %d\n",
                identName.c_str(), line, column);
        }
        for (auto p = ctxChain.crbegin(); p != ctxChain.crend(); ++p) {
            if (auto* var = (*p)->getVariable(identName); var != nullptr) {
                if (!var->value.isType<lin::Array>()) {
                    panic(
                        "TypeError: expects array type of variable %s "
                        "at line %d, col %d\n",
                        identName.c_str(), line, column);
                }
                auto&& temp = var->value.cast<std::vector<lin::Value>>();
                temp[index.cast<int>()] = Interpreter::assignSwitch(
                    this->opt, temp[index.cast<int>()], rhs);
                var->value.data = std::move(temp);
                return rhs;
            }
        }

        (ctxChain.back())->createVariable(identName, rhs);
    } else {
        panic("SyntaxError: can not assign to %s at line %d, col %d\n",
              typeid(lhs).name(), line, column);
    }
    return rhs;
}

lin::Value FunCallExpr::eval(lin::Runtime* rt,
                             std::deque<lin::Context*> ctxChain) {
    if (auto* builtinFunc = rt->getBuiltinFunction(this->funcName);
        builtinFunc != nullptr) {
        std::vector<Value> arguments;
        for (auto e : this->args) {
            arguments.push_back(e->eval(rt, ctxChain));
        }
        return builtinFunc(rt, ctxChain, arguments);
    }
    if (auto* func = rt->getFunction(this->funcName); func != nullptr) {
        if (func->params.size() != this->args.size()) {
            panic("ArgumentError: expects %d arguments but got %d",
                  func->params.size(), this->args.size());
        }
        return Interpreter::callFunction(rt, func, ctxChain, this->args);
    }

    panic(
        "RuntimeError: can not find function definition of %s in both "
        "built-in "
        "functions and user defined functions",
        this->funcName.c_str());
}

lin::Value BinaryExpr::eval(lin::Runtime* rt,
                            std::deque<lin::Context*> ctxChain) {
    lin::Value lhs =
        this->lhs ? this->lhs->eval(rt, ctxChain) : lin::Value(lin::Null);
    lin::Value rhs =
        this->rhs ? this->rhs->eval(rt, ctxChain) : lin::Value(lin::Null);
    Token opt = this->opt;

    if (!lhs.isType<lin::Null>() && rhs.isType<lin::Null>()) {
        return Interpreter::calcUnaryExpr(lhs, opt, line, column);
    }

    return Interpreter::calcBinaryExpr(lhs, opt, rhs, line, column);
}
