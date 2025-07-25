#include "object.h"
#include <string>
#include "memory_node.h"
#include "ops.h"

Object* FindElemInScope(const std::string& name, Scope* scope) {
    while (scope != nullptr) {
        if (scope->IsInScope(name)) {
            return scope->RetObj(name);
        }
        scope = scope->RetParentScope();
    }
    throw NameError("No such name exists");
}

Scope* FindScope(const std::string& name, Scope* scope) {
    while (scope != nullptr) {
    if (scope->IsInScope(name)) {
        return scope;
    }
    scope = scope->RetParentScope();
}
    throw NameError("No such name found in scopes");
}

Scope::Scope() {
    AddName("+", Cleaner::kCleaner->Make<OpHolder<Add>>());
    AddName("-", Cleaner::kCleaner->Make<OpHolder<Sub>>());
    AddName("*", Cleaner::kCleaner->Make<OpHolder<Mul>>());
    AddName("/", Cleaner::kCleaner->Make<OpHolder<Div>>());
    AddName("=", Cleaner::kCleaner->Make<OpHolder<Eq>>());
    AddName(">", Cleaner::kCleaner->Make<OpHolder<Greater>>());
    AddName("<", Cleaner::kCleaner->Make<OpHolder<Less>>());
    AddName("<=", Cleaner::kCleaner->Make<OpHolder<LessOrEq>>());
    AddName(">=", Cleaner::kCleaner->Make<OpHolder<GreaterOrEq>>());
    AddName("min", Cleaner::kCleaner->Make<OpHolder<Min>>());
    AddName("max", Cleaner::kCleaner->Make<OpHolder<Max>>());
    AddName("abs", Cleaner::kCleaner->Make<OpHolder<Abs>>());
    AddName("number?", Cleaner::kCleaner->Make<OpHolder<IsNumber>>());
    AddName("quote", Cleaner::kCleaner->Make<OpHolder<Quote>>());
    AddName("boolean?", Cleaner::kCleaner->Make<OpHolder<IsBool>>());
    AddName("not", Cleaner::kCleaner->Make<OpHolder<Not>>());
    AddName("and", Cleaner::kCleaner->Make<OpHolder<And>>());
    AddName("or", Cleaner::kCleaner->Make<OpHolder<Or>>());
    AddName("pair?", Cleaner::kCleaner->Make<OpHolder<Pair>>());
    AddName("null?", Cleaner::kCleaner->Make<OpHolder<IsNull>>());
    AddName("list?", Cleaner::kCleaner->Make<OpHolder<List>>());
    AddName("cons", Cleaner::kCleaner->Make<OpHolder<Cons>>());
    AddName("car", Cleaner::kCleaner->Make<OpHolder<Car>>());
    AddName("cdr", Cleaner::kCleaner->Make<OpHolder<Cdr>>());
    AddName("list", Cleaner::kCleaner->Make<OpHolder<ConstructList>>());
    AddName("list-ref", Cleaner::kCleaner->Make<OpHolder<ListRef>>());
    AddName("list-tail", Cleaner::kCleaner->Make<OpHolder<ListTail>>());
    AddName("define", Cleaner::kCleaner->Make<OpHolder<Define>>());
    AddName("lambda", Cleaner::kCleaner->Make<OpHolder<LambdaScheme>>());
    AddName("set!", Cleaner::kCleaner->Make<OpHolder<Set>>());
    AddName("if", Cleaner::kCleaner->Make<OpHolder<IfStatement>>());
    AddName("symbol?", Cleaner::kCleaner->Make<OpHolder<IsSymbol>>());
    AddName("set-car!", Cleaner::kCleaner->Make<OpHolder<SetCar>>());
    AddName("set-cdr!", Cleaner::kCleaner->Make<OpHolder<SetCdr>>());
}

Scope::Scope(Scope* parent_scope) : parent_scope_(parent_scope) {
    AddDependency(parent_scope);
}

bool Scope::IsInScope(const std::string& name) {
    return scope_map_.contains(name);
}

 Object* Scope::RetObj(const std::string& name) {
    return scope_map_[name];
 }

Scope* Scope::RetParentScope() {
    return parent_scope_;
}

void Scope::AddName(const std::string& name, Object* obj) {
    if (IsInScope(name)) {
        RemoveDependency(scope_map_[name]);
    }
    scope_map_[name] = obj;
    AddDependency(obj);
}

Number::Number(int val) : val_(val) {}

Object* Number::Exec(Scope* scope) { //NOLINT
    return this;
}
std::string Number::Format() {
    return std::to_string(val_);
}
int Number::GetValue() const {
    return val_;
}

Symbol::Symbol(const std::string& val) : val_(val) {}
Object* Symbol::Exec(Scope* scope) {
    if (val_ == "#t" || val_ == "#f") {
        return this;
    }
    auto val = FindElemInScope(val_, scope);
    return val;
}
std::string Symbol::Format() {
    return val_;
}
const std::string& Symbol::GetName() const {
    return val_;
}

Cell::Cell(Object* first, Object* second) : first_(first), second_(second) {
    AddDependency(first);
    AddDependency(second);
}

Object* Cell::Exec(Scope* scope) {
    
    if (Is<Symbol>(first_)) {
        if (!CheckIfCellIsValid(As<Cell>(this))) {
            throw RuntimeError("Cell structure is not valid");
        }
        std::string op_signature = As<Symbol>(first_)->GetName();
        auto scheme = FindElemInScope(op_signature, scope);
        if (Is<LambdaScheme>(scheme)) {
            auto lambda_func = LambdaFunction(As<LambdaScheme>(scheme), second_, scope);
            return lambda_func.PerformOnArgs();
        } else if (Is<BaseOpHolder>(scheme)) {
            return As<BaseOpHolder>(scheme)->MakeOp(second_, scope);
        }
        throw SyntaxError("Invalid syntax for executed cell");
    } else if (Is<Cell>(first_)) {
        auto lambda_scheme = first_->Exec(scope);
        if (!Is<LambdaScheme>(lambda_scheme)) {
            throw SyntaxError("Bad syntax for executed cell");
        }
        LambdaFunction lambda_func(As<LambdaScheme>(lambda_scheme), second_, scope);
        return lambda_func.PerformOnArgs();
    }
    throw RuntimeError("Can't execute cell");
}

Object* Cell::GetFirst() const {
    return first_;
}

Object* Cell::GetSecond() const {
    return second_;
}

std::string Cell::Format() {
    Object* cell_iter = this;
    std::string formatted_cell("(");
    while (Is<Cell>(cell_iter)) {
        auto current_obj = As<Cell>(cell_iter)->GetFirst();
        auto next_obj = As<Cell>(cell_iter)->GetSecond();

        std::string current_obj_format;
        if (current_obj == nullptr) {
            current_obj_format = "()";
        } else {
            current_obj_format = current_obj->Format();
        }
        formatted_cell += current_obj_format + " ";
                if (!Is<Cell>(next_obj) && next_obj != nullptr) {
            formatted_cell += ". ";
        }

        cell_iter = next_obj;
    }
    if (cell_iter != nullptr) {
        formatted_cell += cell_iter->Format() + " ";
    }

    formatted_cell.pop_back();
    return formatted_cell + ")";
}

void Cell::SetFirst(Object* obj) {
    RemoveDependency(first_);
    first_ = obj;
    AddDependency(first_);
}

void Cell::SetSecond(Object* obj) {
    RemoveDependency(second_);
    second_ = obj;
    AddDependency(second_);
}

void Cell::SetName(const std::string& name) {
    Object* set_name = Cleaner::kCleaner->Make<Symbol>(name);
    AddDependency(set_name);
    name_ = set_name;
}

Object* Cell::GetName() {
    return name_;
}

LambdaScheme::LambdaScheme(Object* arg, Scope* scope, bool is_sugar,  
    bool is_context_capturer) : is_context_capturer_(is_context_capturer) {
    if (!Is<Cell>(arg)) {
            ThrowSyntax();
        }
        Object* lambda_args = As<Cell>(arg)->GetFirst();
        if (!Is<Cell>(lambda_args) && lambda_args != nullptr) {
            ThrowSyntax();
        }
        if (is_sugar) {
            auto symb = As<Cell>(lambda_args)->GetFirst();
            if (!Is<Symbol>(symb)) {
                ThrowSyntax();
            }
            lambda_args = As<Cell>(lambda_args)->GetSecond();
            name_ = As<Symbol>(symb)->GetName();
        }
        while (lambda_args != nullptr) {
            auto symb = As<Cell>(lambda_args)->GetFirst();
            if (!Is<Symbol>(symb)) {
                ThrowSyntax();
            }
            lambda_args_.push_back(As<Symbol>(symb)->GetName());
            lambda_args = As<Cell>(lambda_args)->GetSecond();
        }
        arg = As<Cell>(arg)->GetSecond();
        if (!Is<Cell>(arg)) {
            ThrowSyntax();
        }
        auto body = arg;
        if (!Is<Cell>(body)) {
            ThrowSyntax();
        }
        while (body != nullptr) {
            auto func = As<Cell>(body)->GetFirst();
            AddDependency(func);
            functions_.push_back(As<Cell>(body)->GetFirst());
            body = As<Cell>(body)->GetSecond();
        }

        if (is_context_capturer_ || lambda_args_.empty()) {
            scope_ = scope;
            is_context_capturer_ = true;
        }

        if (is_context_capturer_) {
            AddDependency(scope);
        }
}

bool LambdaScheme::IsCC() {
    return is_context_capturer_;
}

const std::vector<std::string>& LambdaScheme::ReturnArgs() const {
    return lambda_args_;
}

const std::vector<Object*> LambdaScheme::ReturnFuncs() const {
    return functions_;
}

std::string& LambdaScheme::GetName() {
    return name_;
}

Scope* LambdaScheme::GetCCScope() {
    return scope_;
}

std::string LambdaScheme::Format() {
    throw RuntimeError("Kostyl");
}

Object* LambdaScheme::Exec(Scope* scope) {
    if (scope_ != nullptr) {
        scope = scope_;
    }
    if (lambda_args_.empty()) {
        Object* value;
        for (auto& function : functions_) {
            value = function->Exec(scope);
        }
        return value;
    }
    throw RuntimeError("Something went wrong in the internal logic :()");
}

template <typename F>
requires std::is_base_of_v<Operation, F> || std::is_same_v<LambdaScheme, F>

Object* OpHolder<F>::MakeOp(Object* arg_obj, Scope* scope) {
    return F(arg_obj, scope).PerformOnArgs();
}

template<>
Object* OpHolder<Quote>::MakeOp(Object* arg_obj, Scope* scope) { //NOLINT
    return Quote(arg_obj).PerformOnArgs();
}

template<>
Object* OpHolder<LambdaScheme>::MakeOp(Object* arg_obj, Scope* scope) {
    auto scheme = Cleaner::kCleaner->Make<LambdaScheme>(arg_obj, scope);
    return scheme;
}

bool CheckIfCellIsValid(Cell* cell) {
    if (!Is<Symbol>(cell->GetFirst())) {
        return false;
    }
    Object* current_obj = cell;
    while (Is<Cell>(current_obj)) {
        current_obj = As<Cell>(current_obj)->GetSecond();
    }
    return current_obj == nullptr;
}

Object* ConstructBool(bool val) {
    std::string bool_literal = val ? "#t" : "#f";
    return Cleaner::kCleaner->Make<Symbol>(bool_literal);
}