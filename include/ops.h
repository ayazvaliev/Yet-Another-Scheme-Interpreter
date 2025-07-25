#pragma once

#include "memory_node.h"
#include "object.h"
#include <memory>
#include <vector>
#include "error.h"

class Operation {
public:
    using Arguments = std::vector<Object*>;

    Operation() = default;
    Operation(Object* arg_obj, Scope* scope);
    virtual ~Operation() = default;
    virtual Object* PerformOnArgs() = 0;

    template <bool IsGood>
    static Object* MakeList(Arguments& arguments, size_t current_ind) {
        if (arguments.size() == 0 || current_ind >= arguments.size()) {
            return nullptr;
        }
        if (current_ind == arguments.size() - 1) {
            if (IsGood) {
                return Cleaner::kCleaner->Make<Cell>(arguments[current_ind], nullptr);
            } else {
                return arguments[current_ind];
            }
        }
        return Cleaner::kCleaner->Make<Cell>(arguments[current_ind], MakeList<IsGood>(arguments, current_ind + 1));
    }
protected:
    Scope* scope_;
    Arguments arguments_;
};

class ArithmeticModule : public Operation {
public:
    ArithmeticModule(Object* arg_obj, Scope* scope);
protected:
    enum class ArithmeticType {SUM, SUB, MUL, DIV};
    Object* PerformArithmeticOp(ArithmeticType type);
};

class Add : public ArithmeticModule {
public:
    Add(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Sub : public ArithmeticModule {
public:
    Sub(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Mul : public ArithmeticModule {
public:
    Mul(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Div : public ArithmeticModule {
public:
    Div(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class CompareModule : public Operation {
public:
    CompareModule(Object* arg_obj, Scope* scope);
protected:
    enum class CompareType {EQ, GREATER, LESS, EQGREATER, EQLESS};
    Object* Compare(CompareType type);
};

class Eq : public CompareModule {
public:
    Eq(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Greater : public CompareModule {
public:
    Greater(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Less : public CompareModule {
public:
    Less(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class GreaterOrEq : public CompareModule {
public:
    GreaterOrEq(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class LessOrEq : public CompareModule {
public:
    LessOrEq(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Min : public Operation {
public:
    Min(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Max : public Operation {
public:
    Max(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Abs : public Operation {
public:
    Abs(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

template <class ObjectType>
class TypeChecker : public Operation {
public:
    TypeChecker(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
protected:
    bool CheckType() {
        if (arguments_.empty() || arguments_.size() > 1) {
        throw RuntimeError("Invalid arguments for IsType operation");
        }
        return Is<ObjectType>(arguments_.front());
    }
};

class IsNumber : public TypeChecker<Number> {
public:
    IsNumber(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class IsBool : public TypeChecker<Symbol> {
public:
    IsBool(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class IsNull : public Operation {
public:
    IsNull(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Not : public Operation {
public:
    Not(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Quote : public Operation {
public:
    Quote(Object* arg_obj);
    Object* PerformOnArgs() override;
private:
    Object* quoted_object_;
};

template <bool StopOnTrue>
class LogicModule : public Operation {
public:
    LogicModule(Object* arg_obj, Scope* scope) {
        Object* obj_to_ret = nullptr;

        while (arg_obj != nullptr) {
            Cell* cell_obj = As<Cell>(arg_obj);
            Object* first_cell_arg = cell_obj->GetFirst();
            Object* sec_cell_arg = cell_obj->GetSecond();
            if (!first_cell_arg) {
                obj_to_ret = nullptr;
                if (StopOnTrue) {
                    break;
                }
            } else if (Is<Symbol>(first_cell_arg)) {
                std::string symbol_val = As<Symbol>(first_cell_arg)->GetName();
                if (symbol_val == "quote") {
                    obj_to_ret = Quote(sec_cell_arg).PerformOnArgs();
                    arg_obj = sec_cell_arg;
                    if (StopOnTrue) {
                        break;
                    }
                } else if (symbol_val == "#t" || symbol_val == "#f") {
                    obj_to_ret = first_cell_arg;
                    if (StopOnTrue && symbol_val == "#t") {
                        break;
                    }
                    if (!StopOnTrue && symbol_val == "#f") {
                        break;
                    }
                } else {
                    auto elem = FindElemInScope(symbol_val, scope);
                    obj_to_ret = elem;
                    if (Is<Symbol>(elem)) {
                        if (StopOnTrue && As<Symbol>(elem)->GetName() != "#f") {
                            break;
                        }
                        if (!StopOnTrue && As<Symbol>(elem)->GetName() == "#f") {
                            break;
                        }
                    } else if (StopOnTrue) {
                        break;
                    }
                }
            } else {
                Object* result = first_cell_arg->Exec(scope);
                obj_to_ret = result;
                if (Is<Symbol>(result)) {
                    if (StopOnTrue && As<Symbol>(result)->GetName() != "#f") {
                        break;
                    }
                    if (!StopOnTrue && As<Symbol>(result)->GetName() == "#f") {
                        break;
                    }
                } else if (StopOnTrue) {
                    break;
                }
            }
            arg_obj = As<Cell>(arg_obj)->GetSecond();
        }
        last_evalulated_obj_ = obj_to_ret;
    }
protected:
    Object* last_evalulated_obj_;
};

class And : public LogicModule<false> {
public:
    And(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Or : public LogicModule<true> {
public:
    Or(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Pair : public Operation {
public:
    Pair(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class List : public Operation {
public:
    List(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Cons : public Operation {
public:
    Cons(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Car : public Operation {
public:
    Car(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class Cdr : public Operation {
public:
    Cdr(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class ConstructList: public Operation {
public:
    ConstructList(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class ListRef: public Operation {
public:
    ListRef(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class ListTail: public Operation {
public:
    ListTail(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class IfStatement : public Operation {
public:
    IfStatement(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
private:
    inline void ThrowSyntax() {
        throw SyntaxError("Wrong syntax for if statement");
    }
    Object* condition_;
    Object* true_branch_;
    Object* false_branch_;
};

class LambdaFunction : public Operation {
public:
    LambdaFunction(LambdaScheme* scheme, Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
private:
    Scope* scope_;
    LambdaScheme* scheme_;
};

class Define : public Operation {
public:
    Define(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
private:
    void ThrowSyntax() {
        throw SyntaxError("Wrong syntax for Define");
    }
    bool pure_obj_stored_;
    std::string name_;
    Object* new_elem_;
};

class Set : public Operation {
public:
    Set(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
private:
    void ThrowSyntax() {
        throw SyntaxError("Wrong syntax for Set");
    }
    std::string name_;
    Object* new_elem_;
};

class IsSymbol : public Operation {
public:
    IsSymbol(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
};

class SetCar : public Operation {
public:
    SetCar(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;
private:
    void ThrowSyntax() {
        throw SyntaxError("Wrong syntax for Set-car");
    }
    std::string name_;
    Object* new_elem_;
};

class SetCdr : public Operation {
public:
    SetCdr(Object* arg_obj, Scope* scope);
    Object* PerformOnArgs() override;

private:
    void ThrowSyntax() {
        throw SyntaxError("Wrong syntax for set-car");
    }
    std::string name_;
    Object* new_elem_;
};

bool QCheckIsPair(Object* obj);