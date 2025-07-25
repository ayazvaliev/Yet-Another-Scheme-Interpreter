#include "scheme.h"
#include "error.h"
#include "parser.h"
#include <sstream>

std::string Interpreter::Run(const std::string& str) {
    std::stringstream input_stream(PreprocessInputStr(str));
    Tokenizer tokenizer(&input_stream);
    Object* parsed_obj = Read(&tokenizer);
    if (!tokenizer.IsEnd()) {
        throw SyntaxError("Syntax error occured, input string processing didn't reach and end");
    }
    if (!parsed_obj) {
        throw RuntimeError("Given object is empty, nothing to execute");
    }

    auto main_scope = new Scope();

    Object* result_after_execution = parsed_obj->Exec(main_scope);
    auto res_str = result_after_execution == nullptr ? "()" : result_after_execution->Format();

    Cleaner::kCleaner->Sweep(main_scope);
    ++Cleaner::counter;

    return res_str;
}

std::string Interpreter::PreprocessInputStr(std::string str) {
    if (str[0] == '\'') {
        str = "(" + str + ")";
    }
    return str;
}

Interpreter::~Interpreter() {
    Cleaner::kCleaner->DeleteAll();
}
