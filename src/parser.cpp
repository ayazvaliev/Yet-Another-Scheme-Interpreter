#include "parser.h"
#include <vector>
#include "error.h"

static Object* PackProperList(std::vector<Object*>& vector, size_t ind = 0) {
    Object* res = nullptr;
    if (ind == vector.size() - 1) {
        res = Cleaner::kCleaner->Make<Cell>(vector[ind], nullptr);
        return res;
    }
    res = Cleaner::kCleaner->Make<Cell>(vector[ind], PackProperList(vector, ind + 1));
    return res;
}

static Object* PackUnproperList(std::vector<Object*>& vector, size_t ind = 0) {
    Object* res;
    if (ind == vector.size() - 1) {
        res = vector[ind];
        return res;
    }
    res = Cleaner::kCleaner->Make<Cell>(vector[ind], PackUnproperList(vector, ind + 1));
    return res;
}

Object* ReadList(Tokenizer* tokenizer);

Object* Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("1");
    }

    Object* obj_ptr = nullptr;
    Token curr_token = tokenizer->GetToken();

    if (BracketToken* brace_token = std::get_if<BracketToken>(&curr_token)) {
        if (*brace_token == BracketToken::OPEN) {
            obj_ptr = ReadList(tokenizer);
        }

    } else {
        if (ConstantToken* const_token = std::get_if<ConstantToken>(&curr_token)) {
            obj_ptr = Cleaner::kCleaner->Make<Number>(const_token->value);
        } else if (SymbolToken* symb_token = std::get_if<SymbolToken>(&curr_token)) {
            obj_ptr = Cleaner::kCleaner->Make<Symbol>(symb_token->name);
        } else if (QuoteToken* quote_token = std::get_if<QuoteToken>(&curr_token)) {
            obj_ptr = Cleaner::kCleaner->Make<Symbol>("quote");
        }
    }
    tokenizer->Next();
    if (tokenizer->IsEnd()) {
        if (Is<Symbol>(obj_ptr)) {
            if (As<Symbol>(obj_ptr)->GetName() == "quote") {
                throw SyntaxError("1");
            }
        }
    }
    return obj_ptr;
}

Object* ReadList(Tokenizer* tokenizer) {
    tokenizer->Next();
    std::vector<Object*> objs;

    int dot_pos = -1;
    int counter = 0;
    while (true) {
        if (tokenizer->IsEnd()) {
            throw SyntaxError("1");
        }
        Token curr_token = tokenizer->GetToken();
        if (BracketToken* brace_token = std::get_if<BracketToken>(&curr_token)) {
            if ((*brace_token) == BracketToken::CLOSE) {
                break;
            }
        }
        if (DotToken* dot_token = std::get_if<DotToken>(&curr_token)) {
            if (dot_pos != -1) {
                throw SyntaxError("1");
            }
            dot_pos = counter;
            tokenizer->Next();
            continue;
        }

        objs.push_back(Read(tokenizer));

        ++counter;
    }
    Object* ret_object = nullptr;

    if (objs.empty()) {
        if (dot_pos != -1) {
            throw SyntaxError("1");
        }
        return ret_object;
    }
    if (objs.size() == 2) {
        if (dot_pos == -1) {
            ret_object = PackProperList(objs);
        } else {
            if (dot_pos != 1) {
                throw SyntaxError("1");
            }
            ret_object = Cleaner::kCleaner->Make<Cell>(objs.front(), objs.back());
        }
    } else if (dot_pos != -1) {
        if (static_cast<size_t>(dot_pos) != objs.size() - 1 || objs.size() == 1) {
            throw SyntaxError("1");
        }
        ret_object = PackUnproperList(objs);
    } else {
        ret_object = PackProperList(objs);
    }

    return ret_object;
}