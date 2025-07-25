#include "tokenizer.h"
#include "error.h"

bool Tokenizer::IsDot(const char& ch) {
    return ch == '.';
}
bool Tokenizer::IsQuote(const char& ch) {
    return ch == 39;
}
bool Tokenizer::IsPlusMinus(const char& ch) {
    return (ch == '+') || (ch == '-');
}
bool Tokenizer::IsDigit(const char& ch) {
    return (ch >= 48 && ch <= 57);
}
bool Tokenizer::IsOpenBracket(const char& ch) {
    return ch == 40;
}
bool Tokenizer::IsCloseBracket(const char& ch) {
    return ch == 41;
}

bool Tokenizer::IsOpeningSymbol(const char& ch) {
    return (ch >= 65 && ch <= 90) || (ch >= 97 && ch <= 122) || (ch == '*') || (ch == '/') ||
           (ch == '#') || (ch == '=') || (ch == '>') || (ch == '<');
}

bool Tokenizer::IsInnerSymbol(const char& ch) {
    return IsOpeningSymbol(ch) || IsDigit(ch) || ch == '?' || ch == '!' || ch == '-';
}

int Tokenizer::ReadNum(char& ch) {
    std::string str_num = {ch};
    while (IsDigit(in_->peek())) {
        str_num += in_->get();
    }

    return std::stoi(str_num);
}

std::string Tokenizer::ReadSymbols(char& ch) {
    std::string str = {ch};
    while (IsInnerSymbol(in_->peek())) {
        str += in_->get();
    }

    return str;
}

bool Tokenizer::IsValid(const char& ch) {
    return (IsDot(ch) || IsQuote(ch) || IsDigit(ch) || IsPlusMinus(ch) || IsOpeningSymbol(ch) ||
            IsOpenBracket(ch) || IsCloseBracket(ch));
}

Tokenizer::Tokenizer(std::istream* in) {
    in_ = in;
    is_end_ = false;
    Next();
}

bool Tokenizer::IsEnd() {
    return is_end_;
}

void Tokenizer::Next() {
    char curr_char = ' ';
    while (in_->peek() != EOF) {
        curr_char = in_->get();
        if (IsValid(curr_char)) {
            break;
        } else {
            if (curr_char != '\n' && curr_char != ' ') {
                throw SyntaxError("Syntax Error");
            }
        }
    }
    if (curr_char == ' ') {
        is_end_ = true;
        return;
    }

    if (IsQuote(curr_char)) {
        curr_token_ = QuoteToken();
        return;
    }
    if (IsDot(curr_char)) {
        curr_token_ = DotToken();
        return;
    }
    if (IsOpenBracket(curr_char)) {
        curr_token_ = BracketToken::OPEN;
        return;
    }
    if (IsCloseBracket(curr_char)) {
        curr_token_ = BracketToken::CLOSE;
        return;
    }
    if (IsDigit(curr_char)) {
        curr_token_ = ConstantToken{.value = ReadNum(curr_char)};
        return;
    }

    if (IsPlusMinus(curr_char)) {
        const char sign_ch = curr_char;
        if (IsDigit(in_->peek())) {
            curr_char = in_->get();
            int value = ReadNum(curr_char);
            if (sign_ch == '-') {
                value = -value;
            }
            curr_token_ = ConstantToken{.value = value};
        } else {
            std::string name{curr_char};
            curr_token_ = SymbolToken{.name = name};
        }

        return;
    }

    if (IsOpeningSymbol(curr_char)) {
        if (curr_char == '#') {
            if (in_->peek() == 't' || in_->peek() == 'f') {
                char bool_ch = in_->get();
                std::string name = "#";
                name += bool_ch;
                curr_token_ = SymbolToken{.name = name};
                return;
            } else {
                throw SyntaxError("1");
            }
        }

        std::string name = ReadSymbols(curr_char);
        curr_token_ = SymbolToken{.name = name};
    }
}

Token Tokenizer::GetToken() {
    return curr_token_;
}

bool SymbolToken::operator==(const SymbolToken& other) const {
    return (name == other.name);
}
bool QuoteToken::operator==(const QuoteToken&) const {
    return true;
}
bool DotToken::operator==(const DotToken&) const {
    return true;
}

bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
}

bool BooleanToken::operator==(const BooleanToken& other) const {
    return state == other.state;
}