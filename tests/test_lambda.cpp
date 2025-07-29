#include <string>

#include "scheme_test.h"


TEST_CASE_METHOD(SchemeTest, "SimpleLambda") {
    ExpectEq("((lambda (x) (+ 1 x)) 5)", "6");
}

TEST_CASE_METHOD(SchemeTest, "LambdaSyntax") {
    ExpectSyntaxError("(lambda)");
    ExpectSyntaxError("(lambda x)");
    ExpectSyntaxError("(lambda (x))");
}
