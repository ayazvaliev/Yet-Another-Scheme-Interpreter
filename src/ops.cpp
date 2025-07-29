#include "ops.h"
#include <sys/types.h>
#include <cstddef>
#include <memory>
#include "error.h"
#include "memory_node.h"
#include "object.h"

Operation::Operation(Object* arg_obj, Scope* scope) : scope_(scope) {
    if (arg_obj != nullptr && !Is<Cell>(arg_obj)) {
        std::string what = (arg_obj == nullptr ? "()" : arg_obj->Format());
        throw RuntimeError("Type of arguments of operation is not Cell: " + what);
    }
    
    while (arg_obj != nullptr) {
        Cell* cell_obj = As<Cell>(arg_obj);
        Object* first_cell_arg = cell_obj->GetFirst();
        Object* sec_cell_arg = cell_obj->GetSecond();
        Object* obj_to_push = nullptr;
        if (!first_cell_arg) {
            obj_to_push = nullptr;
        } else if (Is<Symbol>(first_cell_arg)) {
            std::string symbol_val = As<Symbol>(first_cell_arg)->GetName();
            if (symbol_val == "#t" || symbol_val == "#f") {
                obj_to_push = first_cell_arg;
            } else {
                obj_to_push = FindElemInScope(symbol_val, scope);
                if(Is<OpHolder<Quote>>(obj_to_push)) {
                    obj_to_push = Quote(sec_cell_arg).PerformOnArgs();
                    arg_obj = sec_cell_arg;
                }
            }
        } else {
            obj_to_push = first_cell_arg->Exec(scope);
        }
        arguments_.push_back(obj_to_push);
        arg_obj = As<Cell>(arg_obj)->GetSecond();
    }
}
Object* Operation::PerformOnArgs() {
    throw RuntimeError("Inevitable Kostyl :)");
}

ArithmeticModule::ArithmeticModule(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* ArithmeticModule::PerformArithmeticOp(ArithmeticModule::ArithmeticType type) {
    int val = 0;
    size_t iter = 0;
    if (type == ArithmeticType::MUL) {
        val = 1;
    } else if (type == ArithmeticType::SUB || type == ArithmeticType::DIV) {
        if (arguments_.empty()) {
            throw RuntimeError("Abscence of arguments for arithmetic operation");
        }
        if (!Is<Number>(arguments_.front())) {
            throw RuntimeError("Invalid arguments for arithmetic operation");
        }
        val = As<Number>(arguments_.front())->GetValue();
        iter = 1;
    }
    for (;iter < arguments_.size(); ++iter) {
        Object* current_arg = arguments_[iter];
        if (!Is<Number>(current_arg)) {
            throw RuntimeError("Invalid arguments for arithmetic operation");
        }
        int current_value = As<Number>(current_arg)->GetValue();
        if (type == ArithmeticType::SUM) {
            val += current_value;
        } else if (type == ArithmeticType::SUB) {
            val -= current_value;
        } else if (type == ArithmeticType::DIV) {
            val /= current_value;
        } else if (type == ArithmeticType::MUL) {
            val *= current_value;
        }
    }
    return Cleaner::kCleaner->Make<Number>(val);
}

Add::Add(Object* arg_obj, Scope* scope) : ArithmeticModule(arg_obj, scope) {}
Object* Add::PerformOnArgs() {
    return PerformArithmeticOp(ArithmeticType::SUM);
}

Sub::Sub(Object* arg_obj, Scope* scope) : ArithmeticModule(arg_obj, scope) {}
Object* Sub::PerformOnArgs() {
    return PerformArithmeticOp(ArithmeticType::SUB);
}

Mul::Mul(Object* arg_obj, Scope* scope) : ArithmeticModule(arg_obj, scope) {}
Object* Mul::PerformOnArgs() {
    return PerformArithmeticOp(ArithmeticType::MUL);
}

Div::Div(Object* arg_obj, Scope* scope) : ArithmeticModule(arg_obj, scope) {}
Object* Div::PerformOnArgs() {
    return PerformArithmeticOp(ArithmeticType::DIV);
}

CompareModule::CompareModule(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* CompareModule::Compare(CompareModule::CompareType type) {
    if (arguments_.empty()) {
        return ConstructBool(true);
    }
    if (!Is<Number>(arguments_.front())) {
        throw RuntimeError("Invalid arguments for Compare operation");
    }
    bool answer = true;
    int val = As<Number>(arguments_.front())->GetValue();
    for (size_t i = 1; i < arguments_.size(); ++i) {
        Object* current_arg = arguments_[i];
        if (!Is<Number>(current_arg)) {
            throw RuntimeError("Invalid arguments for Compare operation");
        }
        int current_val = As<Number>(current_arg)->GetValue();
        bool result = false;
        if (type == CompareType::EQ) {
            result = (val == current_val);
        } else if (type == CompareType::GREATER) {
            result = (val > current_val);
        } else if (type == CompareType::LESS) {
            result = (val < current_val);
        } else if (type == CompareType::EQGREATER) {
            result = (val >= current_val);
        } else if (type == CompareType::EQLESS) {
            result = (val <= current_val);
        }
        if (!result) {
            answer = false;
            break;
        }
    }
    return ConstructBool(answer);
}

Eq::Eq(Object* arg_obj, Scope* scope) : CompareModule(arg_obj, scope) {}
Object* Eq::PerformOnArgs() {
    return Compare(CompareType::EQ);
}

Greater::Greater(Object* arg_obj, Scope* scope) : CompareModule(arg_obj, scope) {}
Object* Greater::PerformOnArgs() {
    return Compare(CompareType::GREATER);
}

Less::Less(Object* arg_obj, Scope* scope) : CompareModule(arg_obj, scope) {}
Object* Less::PerformOnArgs() {
    return Compare(CompareType::LESS);
}

LessOrEq::LessOrEq(Object* arg_obj, Scope* scope) : CompareModule(arg_obj, scope) {}
Object* LessOrEq::PerformOnArgs() {
    return Compare(CompareType::EQLESS);
}

GreaterOrEq::GreaterOrEq(Object* arg_obj, Scope* scope) : CompareModule(arg_obj, scope) {}
Object* GreaterOrEq::PerformOnArgs() {
    return Compare(CompareType::EQGREATER);
}

Min::Min(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* Min::PerformOnArgs() {
    if (arguments_.empty()) {
        throw RuntimeError("Abscence of arguments for Min operation");
    }
    if (!Is<Number>(arguments_.front())) {
        throw RuntimeError("Invalid arguments for Min operation");
    }
    int val = As<Number>(arguments_.front())->GetValue();
    for (size_t i = 1; i < arguments_.size(); ++i) {
        Object* current_arg = arguments_[i];
        if (!Is<Number>(current_arg)) {
            throw RuntimeError("Invalid arguments for Min operation");
        }
        val = std::min(As<Number>(current_arg)->GetValue(), val);
    }
    return Cleaner::kCleaner->Make<Number>(val);
}

Max::Max(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* Max::PerformOnArgs() {
    if (arguments_.empty()) {
        throw RuntimeError("Abscence of arguments for Max operation");
    }
    if (!Is<Number>(arguments_.front())) {
        throw RuntimeError("Invalid arguments for Max operation");
    }
    int val = As<Number>(arguments_.front())->GetValue();
    for (size_t i = 1; i < arguments_.size(); ++i) {
        Object* current_arg = arguments_[i];
        if (!Is<Number>(current_arg)) {
            throw RuntimeError("Invalid arguments for Max operation");
        }
        val = std::max(As<Number>(current_arg)->GetValue(), val);
    }
    return Cleaner::kCleaner->Make<Number>(val);
}

Abs::Abs(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {};
Object* Abs::PerformOnArgs() {
    if (arguments_.empty()) {
        throw RuntimeError("Abscence of arguments for Abs operation");
    }
    if (arguments_.size() > 1 || !Is<Number>(arguments_.front())) {
        throw RuntimeError("Invalid arguments for Abs operation");
    }
    
    int val = As<Number>(arguments_.front())->GetValue();
    return Cleaner::kCleaner->Make<Number>(std::abs(val));
}

IsNumber::IsNumber(Object* arg_obj, Scope* scope) : TypeChecker<Number>(arg_obj, scope) {}
Object* IsNumber::PerformOnArgs() {
    return ConstructBool(CheckType());
}

IsBool::IsBool(Object* arg_obj, Scope* scope) : TypeChecker<Symbol>(arg_obj, scope) {}
Object* IsBool::PerformOnArgs() {
    bool answer = CheckType();
    if (answer) {
        std::string symbol = As<Symbol>(arguments_.front())->GetName();
        answer = (symbol == "#t") || (symbol == "#f");
    }
    return ConstructBool(answer);
}

Not::Not(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* Not::PerformOnArgs() {
    if (arguments_.empty() || arguments_.size() > 1) {
        throw RuntimeError("Invalid arguments for Not operation");
    }
    bool answer = false;
    if (Is<Symbol>(arguments_.front()) && (As<Symbol>(arguments_.front()))->GetName() == "#f") {
        answer = true;
    }
    return ConstructBool(answer);
}

And::And(Object* arg_obj, Scope* scope) : LogicModule<false>(arg_obj, scope) {}
Object* And::PerformOnArgs() {
    if (last_evalulated_obj_ == nullptr) {
        return ConstructBool(true);
    }
    return last_evalulated_obj_;
}

Or::Or(Object* arg_obj, Scope* scope) : LogicModule<true>(arg_obj, scope) {}
Object* Or::PerformOnArgs() {
    if (last_evalulated_obj_ == nullptr) {
        return ConstructBool(false);
    }
    return last_evalulated_obj_;
}

IsNull::IsNull(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* IsNull::PerformOnArgs() {
    if (arguments_.empty() || arguments_.size() > 1) {
        throw RuntimeError("Wrong arguments for IsNull operation");
    }
    return ConstructBool(arguments_.front() == nullptr);
}

Pair::Pair(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* Pair::PerformOnArgs() {
    if (arguments_.empty() || arguments_.size() > 1) {
        throw RuntimeError("Invalid arguments for IsPair operation");
    }
    bool answer = false;
    if (Is<Cell>(arguments_.front())) {
        auto arg_cell = As<Cell>(arguments_.front());
        if (Is<Cell>(arg_cell->GetSecond())) {
            if (As<Cell>(arg_cell->GetSecond())->GetSecond() == nullptr) {
                answer = true;
            }
        } else if (arg_cell->GetSecond() != nullptr) {
            answer = true;
        }
    }
    return ConstructBool(answer);
}

List::List(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* List::PerformOnArgs() {
    if (arguments_.empty() || arguments_.size() > 1) {
        throw RuntimeError("Invalid arguments for IsList operation");
    }
    Object* list_iter = arguments_.front();
    while (Is<Cell>(list_iter)) {
        list_iter = As<Cell>(list_iter)->GetSecond();
    }
    return ConstructBool(list_iter == nullptr);
}

Cons::Cons(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* Cons::PerformOnArgs() {
    if (arguments_.size() != 2) {
        throw RuntimeError("Invalid arguments for Cons operation");
    }
    return Cleaner::kCleaner->Make<Cell>(arguments_[0], arguments_[1]);
}

Car::Car(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* Car::PerformOnArgs() {
    if ((arguments_.size() != 1) || !Is<Cell>(arguments_.front())) {
        throw RuntimeError("Invalid arguments for Car operation");
    }
    return As<Cell>(arguments_.front())->GetFirst();
}

Cdr::Cdr(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* Cdr::PerformOnArgs() {
    if ((arguments_.size() != 1) || !Is<Cell>(arguments_.front())) {
        throw RuntimeError("Invalid arguments for Cdr operation");
    }
    Arguments list_elements;
    Object* list_iter = arguments_.front();
    if (!Is<Cell>(list_iter)) {
        throw RuntimeError("Bad args for Cdr");
    }
    if (QCheckIsPair(list_iter)) {
        return As<Cell>(list_iter)->GetSecond();
    }
    while (Is<Cell>(list_iter)) {
        list_elements.push_back(As<Cell>(list_iter)->GetFirst());
        list_iter = As<Cell>(list_iter)->GetSecond();
    }
    if (list_iter != nullptr) {
        list_elements.push_back(list_iter);
    }
    if (list_elements.size() == 1) {
        return nullptr;
    }
    return (list_iter == nullptr) ? Operation::MakeList<true>(list_elements, 1) :
                                    Operation::MakeList<false>(list_elements, 1);
}

ConstructList::ConstructList(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* ConstructList::PerformOnArgs() {
    return Operation::MakeList<true>(arguments_, 0);
}

ListRef::ListRef(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* ListRef::PerformOnArgs() {
    if (arguments_.size() != 2 || !Is<Cell>(arguments_.front()) || 
            !Is<Number>(arguments_.back())) {
        throw RuntimeError("Invalid arguments for ListRef operation");
    }

    Object* list_iter = arguments_.front();
    size_t queried_index = static_cast<size_t>(As<Number>(arguments_.back())->GetValue());
    Object* queried_element = nullptr;

    size_t current_ind = 0;
    while (Is<Cell>(list_iter)) {
        if (current_ind == queried_index) {
            queried_element = As<Cell>(list_iter)->GetFirst();
        }
        list_iter = As<Cell>(list_iter)->GetSecond();
        ++current_ind;
    }
    if (list_iter != nullptr) {
        throw RuntimeError("Argument is not a correct list in ListRef operation");
    }
    if (current_ind <= queried_index) {
        throw RuntimeError("Index out of bounds");
    }
    return queried_element;

}

ListTail::ListTail(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* ListTail::PerformOnArgs() {
   if (arguments_.size() != 2 || !Is<Cell>(arguments_.front()) || 
            !Is<Number>(arguments_.back())) {
        throw RuntimeError("Invalid arguments for ListTail operation");
    }
    size_t queried_index = static_cast<size_t>(As<Number>(arguments_.back())->GetValue());
    Arguments list_elements;
    Object* list_iter = arguments_.front();
    while (Is<Cell>(list_iter)) {
        list_elements.push_back(As<Cell>(list_iter)->GetFirst());
        list_iter = As<Cell>(list_iter)->GetSecond();
    }
    if (list_iter != nullptr) {
        throw RuntimeError("Argument is not a correct list in ListTail operation");
    }
    if (list_elements.size() < queried_index) {
        throw RuntimeError("Index out of bounds");
    }
    return MakeList<true>(list_elements, queried_index);
}

Quote::Quote(Object* arg_obj) {
    if (!Is<Cell>(arg_obj)) {
        throw RuntimeError("Invalid arguments for Quote");
    }
    if (Is<Cell>(As<Cell>(arg_obj)->GetSecond())) {
        auto second_arg = As<Cell>(As<Cell>(arg_obj)->GetSecond());
        if (second_arg->GetFirst() == nullptr && second_arg->GetSecond() == nullptr) {
            throw RuntimeError("Invalid arguments for Quote");
        }
    }
    quoted_object_ = As<Cell>(arg_obj)->GetFirst();
}
Object* Quote::PerformOnArgs() {
    return quoted_object_;
}

IfStatement::IfStatement(Object* arg_obj, Scope* scope) : condition_(nullptr), true_branch_(nullptr), false_branch_(nullptr) {
    scope_ = scope;

    if (!Is<Cell>(arg_obj)) {
        ThrowSyntax();
    }
    auto cond = As<Cell>(arg_obj)->GetFirst();
    condition_ = cond->Exec(scope);

    arg_obj = As<Cell>(arg_obj)->GetSecond();
    if (!Is<Cell>(arg_obj)) {
        ThrowSyntax();
    }
    true_branch_ = As<Cell>(arg_obj)->GetFirst();

    arg_obj = As<Cell>(arg_obj)->GetSecond();
    if (arg_obj != nullptr) {
        if (!Is<Cell>(arg_obj)) {
            ThrowSyntax();
        }
        false_branch_ = As<Cell>(arg_obj)->GetFirst();
        arg_obj = As<Cell>(arg_obj)->GetSecond();
        if (arg_obj != nullptr) {
            ThrowSyntax();
        }
    }
}



Object* IfStatement::PerformOnArgs() {
    bool state = true;

    if (Is<Symbol>(condition_)) {
        if (As<Symbol>(condition_)->GetName() != "#f") {
            state = true;
        } else {
            state = false;
        }
    } else {
        state = true;
    }
    if (state) {
        return true_branch_->Exec(scope_);
    } else {
        if (false_branch_ != nullptr) {
            return false_branch_->Exec(scope_);
        } else {
            return false_branch_;
        }
    }
}

LambdaFunction::LambdaFunction(
    LambdaScheme* scheme,
    Object* arg_obj,
    Scope* scope) : scheme_(scheme) {
    if (scheme_->IsCC()) {
        scope_ = Cleaner::kCleaner->MakeScope(scheme_->GetCCScope());
    } else {
        scope_ = Cleaner::kCleaner->MakeScope(scope);
    }

    std::vector<Object*> input_args;
    while (arg_obj != nullptr) {
        auto elem = As<Cell>(arg_obj)->GetFirst();
        if (Is<Symbol>(elem)) {
            elem = FindElemInScope(As<Symbol>(elem)->GetName(), scope_);
        }
        input_args.push_back(elem->Exec(scope_));
        arg_obj = As<Cell>(arg_obj)->GetSecond();
    }

    const std::vector<std::string>& names = scheme->ReturnArgs();
    if (names.size() != input_args.size()) {
        throw RuntimeError("Incorrect num of args");
    }
    for (size_t i = 0; i < input_args.size(); ++i) {
        scope_->AddName(names[i], input_args[i]);
    }    
}

Object* LambdaFunction::PerformOnArgs() {
    Object* value;
    for (const auto& function : scheme_->ReturnFuncs()) {
        value = function->Exec(scope_);
        if (Is<Symbol>(value)) {
            value = FindElemInScope(As<Symbol>(value)->GetName(), scope_);
        }
    }
    return value;
}

Define::Define(Object* arg_obj, Scope* scope) {
    scope_ = scope;

    if (!Is<Cell>(arg_obj)) {
        ThrowSyntax();
    }
    auto header = As<Cell>(arg_obj)->GetFirst();
    if (Is<Cell>(header)) {
        Object* lambda_scheme = Cleaner::kCleaner->Make<LambdaScheme>(arg_obj, scope, true, false);
        new_elem_ = lambda_scheme;
        name_ = As<LambdaScheme>(lambda_scheme)->GetName();
    } else {
        if (!Is<Symbol>(header)) {
            ThrowSyntax();
        }
        name_ = As<Symbol>(header)->GetName();
        arg_obj = As<Cell>(arg_obj)->GetSecond();
        if (!Is<Cell>(arg_obj)) {
            ThrowSyntax();
        }
        auto elem = As<Cell>(arg_obj)->GetFirst();
        if(Is<Symbol>(elem)) {
            auto elem_from_scope = FindElemInScope(As<Symbol>(elem)->GetName(), scope);
            if (Is<OpHolder<Quote>>(elem_from_scope)) {
                pure_obj_stored_ = true;
                arg_obj = As<Cell>(arg_obj)->GetSecond();
                new_elem_ = As<Cell>(arg_obj)->GetFirst();
                return;
            }
        }

        if (As<Cell>(arg_obj)->GetSecond() != nullptr) {
            ThrowSyntax();
        }

        arg_obj = As<Cell>(arg_obj)->GetFirst();
        if(!Is<Cell>(arg_obj)) {
            if (Is<Symbol>(As<Cell>(arg_obj)->GetFirst())) {
                auto symb = As<Symbol>(As<Cell>(arg_obj)->GetFirst());
                auto elem_from_scope = FindElemInScope(symb->GetName(), scope);
                if (Is<OpHolder<LambdaScheme>>(elem_from_scope)) {
                    new_elem_ = Cleaner::kCleaner->Make<LambdaScheme>(As<Cell>(arg_obj)->GetSecond(), scope, false, false);
                    As<LambdaScheme>(new_elem_)->GetName() = name_;
                    return;
                }
            }
        }
        new_elem_ = arg_obj->Exec(scope);
    }
}

Object* Define::PerformOnArgs() {
    if (pure_obj_stored_) {
        if (Is<Cell>(new_elem_)) {
            As<Cell>(new_elem_)->SetName(name_);
        }
    }
    scope_->AddName(name_, new_elem_);
    return nullptr;
}

Set::Set(Object* arg_obj, Scope* scope) {
    scope_ = scope;

    if (!Is<Cell>(arg_obj)) {
            ThrowSyntax();
        }
        auto elem = As<Cell>(arg_obj)->GetFirst();
        if (!Is<Symbol>(elem)) {
            ThrowSyntax();
        }
        name_ = As<Symbol>(elem)->GetName();
        arg_obj = As<Cell>(arg_obj)->GetSecond();
        if (!As<Cell>(arg_obj)) {
            ThrowSyntax();
        }
        if (As<Cell>(arg_obj)->GetSecond() != nullptr) {
            ThrowSyntax();
        }
        new_elem_ = As<Cell>(arg_obj)->GetFirst()->Exec(scope);
}

Object* Set::PerformOnArgs() {
    scope_ = FindScope(name_, scope_);
    scope_->AddName(name_, new_elem_);
    return nullptr;
}

IsSymbol::IsSymbol(Object* arg_obj, Scope* scope) : Operation(arg_obj, scope) {}
Object* IsSymbol::PerformOnArgs() {
    if (arguments_.empty() || arguments_.size() > 1) {
        throw RuntimeError("Invalid args for symbol?");
    }
    bool res_state = true;
    auto arg = arguments_.front();
    if (!Is<Symbol>(arg)) {
        res_state = false;
    }

    return ConstructBool(res_state);
}

bool QCheckIsPair(Object* obj) {
    uint counter = 0;
    while (Is<Cell>(obj)) {
        ++counter;
        if (counter >= 2) {
            return false;
        }
        obj = As<Cell>(obj)->GetSecond();
        if (Is<Cell>(obj)) {
            if (As<Cell>(obj)->GetName() != nullptr) {
                return true;
            }
        }
    }
    return false;
}

SetCar::SetCar(Object* arg_obj, Scope* scope) {
    scope_ = scope;

     if (!Is<Cell>(arg_obj)) {
        ThrowSyntax();
    }
    Object* elem = As<Cell>(arg_obj)->GetFirst();
    if (Is<Cell>(elem)) {
        elem = elem->Exec(scope);
        if (Is<Cell>(elem)) {
            elem = As<Cell>(elem)->GetName();
        }
    }
    name_ = As<Symbol>(elem)->GetName();
    arg_obj = As<Cell>(arg_obj)->GetSecond();
    if (!As<Cell>(arg_obj)) {
        ThrowSyntax();
    }
    if (As<Cell>(arg_obj)->GetSecond() != nullptr) {
        ThrowSyntax();
    }
    new_elem_ = As<Cell>(arg_obj)->GetFirst()->Exec(scope);
}

Object* SetCar::PerformOnArgs() {
    auto pair = FindElemInScope(name_, scope_);
    if (!QCheckIsPair(pair)) {
        throw RuntimeError("Wrong args for set-car");
    }
    As<Cell>(pair)->SetFirst((new_elem_));
    return nullptr;
}

SetCdr::SetCdr(Object* arg_obj, Scope* scope) {
    scope_ = scope;

    if (!Is<Cell>(arg_obj)) {
            ThrowSyntax();
    }
    auto elem = As<Cell>(arg_obj)->GetFirst();
    if (Is<Cell>(elem)) {
        elem = elem->Exec(scope);
        if (Is<Cell>(elem)) {
            elem = As<Cell>(elem)->GetName();
        }
    }
    name_ = As<Symbol>(elem)->GetName();
    arg_obj = As<Cell>(arg_obj)->GetSecond();
    if (!As<Cell>(arg_obj)) {
        ThrowSyntax();
    }
    if (As<Cell>(arg_obj)->GetSecond() != nullptr) {
        ThrowSyntax();
    }
    new_elem_ = As<Cell>(arg_obj)->GetFirst()->Exec(scope);
}

Object* SetCdr::PerformOnArgs() {
    auto pair = FindElemInScope(name_, scope_);
    if (!QCheckIsPair(pair)) {
        throw RuntimeError("Wrong arg for set-cdr");
    }
    As<Cell>(pair)->SetSecond(new_elem_);
    return nullptr;
}