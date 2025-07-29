#include "scheme_test.h"

TEST_CASE_METHOD(SchemeTest, "IfReturnValue") {
    ExpectEq("(if #t 0)", "0");
    ExpectEq("(if #f 0)", "()");
    ExpectEq("(if (= 2 2) (+ 1 10))", "11");
    ExpectEq("(if (= 2 3) (+ 1 10) 5)", "5");
}

TEST_CASE_METHOD(SchemeTest, "IfSyntax") {
    ExpectSyntaxError("(if)");
    ExpectSyntaxError("(if 1 2 3 4)");
}
