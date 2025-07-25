#pragma once

#include <type_traits>
#include <unordered_map>
#include "error.h"
#include "memory_node.h"
#include <string>
#include <vector>

class Scope;
class Operation;

class Object : public MemoryNode {
    friend Scope;
public:
    virtual Object* Exec(Scope* scope) = 0;
    virtual std::string Format() = 0;
};

Object* FindElemInScope(const std::string& name, Scope* scope);
Scope* FindScope(const std::string& name, Scope* scope);

class Scope : public MemoryNode{
public:
    Scope();
    Scope(Scope* parent_scope);
    bool IsInScope(const std::string& name);
    Object* RetObj(const std::string& name);
    Scope* RetParentScope();
    void AddName(const std::string& name, Object* obj);

private:
    Scope* parent_scope_;
    std::unordered_map<std::string, Object*> scope_map_;
};

class Number : public Object {
public:
    Number(int val);
    int GetValue() const;
    Object* Exec(Scope* scope) override;
    std::string Format() override;
private:
    int val_;
};

class Symbol : public Object {
public:
    Symbol(const std::string& val);
    const std::string& GetName() const;
    Object* Exec(Scope* scope) override;
    std::string Format() override;
private:
    std::string val_;
};

class Cell : public Object {
public:
    Cell(Object* first, Object* second);
    Object* Exec(Scope* scope) override;
    std::string Format() override;

    Object* GetFirst() const;
    Object* GetSecond() const;

    void SetFirst(Object*);
    void SetSecond(Object*);

    void SetName(const std::string&);
    Object* GetName();
private:
    Object* first_;
    Object* second_;
    Object* name_;
};


class LambdaScheme : public Object {
public:
    LambdaScheme(Object* arg, Scope* scope,
                    bool is_sugar = false,
                    bool is_context_capturer = true);
    bool IsCC();
    const std::vector<std::string>& ReturnArgs() const;
    const std::vector<Object*> ReturnFuncs() const;
    std::string& GetName();
    Scope* GetCCScope();

    std::string Format() override;
    Object* Exec(Scope* scope) override;
private:
    inline void ThrowSyntax() {
        throw SyntaxError("Incorrect args for lambda definition");
    }

    bool is_context_capturer_;
    std::string name_;
    Scope* scope_;
    std::vector<std::string> lambda_args_;
    std::vector<Object*> functions_;
};



class BaseOpHolder : public Object {
public:
    BaseOpHolder() = default;

    virtual Object* MakeOp(Object* arg_obj, Scope* scope) = 0;

    std::string Format() final {
        throw RuntimeError("Uncallable");
    }
    Object* Exec(Scope* scope) final { //NOLINT
        throw RuntimeError("Uncallable");
    }
};

template<typename F>
requires std::is_base_of_v<Operation, F> || std::is_same_v<LambdaScheme, F>

class OpHolder : public BaseOpHolder {
public:
    OpHolder() = default;

    Object* MakeOp(Object* arg_obj, Scope* scope) override;
};


///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
T* As(Object* obj) {
    return dynamic_cast<T*>(obj);
}

template <class T>
bool Is(Object* obj) {
    return dynamic_cast<T*>(obj) != nullptr;
}

bool CheckIfCellIsValid(Cell* cell);
Object* ConstructBool(bool val);