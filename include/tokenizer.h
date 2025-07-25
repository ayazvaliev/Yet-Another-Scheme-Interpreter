#pragma once

#include <variant>
#include <istream>

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    bool operator==(const ConstantToken& other) const;
};

struct BooleanToken {
    bool state;

    bool operator==(const BooleanToken& other) const;
};

using Token =
    std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken, BooleanToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in);

    bool IsEnd();

    void Next();

    Token GetToken();

private:
    bool IsDot(const char& ch);
    bool IsQuote(const char& ch);
    bool IsOpenBracket(const char& ch);
    bool IsCloseBracket(const char& ch);
    bool IsPlusMinus(const char& ch);
    bool IsDigit(const char& ch);
    bool IsOpeningSymbol(const char& ch);
    bool IsInnerSymbol(const char& ch);
    bool IsValid(const char& ch);

    int ReadNum(char& ch);
    std::string ReadSymbols(char& ch);

    Token curr_token_;
    std::istream* in_;
    bool is_end_;
};