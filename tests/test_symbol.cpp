#include "scheme_test.h"

TEST_CASE_METHOD(SchemeTest, "SymbolPredicate") {
    ExpectEq("(symbol? 'x)", "#t");
    ExpectEq("(symbol? 1)", "#f");
}

TEST_CASE_METHOD(SchemeTest, "DefineInvalidSyntax") {
    ExpectSyntaxError("(define)");
    ExpectSyntaxError("(define 1)");
    ExpectSyntaxError("(define x 1 2)");
}

TEST_CASE_METHOD(SchemeTest, "SetInvalidSyntax") {
    ExpectSyntaxError("(set!)");
    ExpectSyntaxError("(set! 1)");
    ExpectSyntaxError("(set! x 1 2)");
}