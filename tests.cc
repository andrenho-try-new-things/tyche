#include "gtest/gtest.h"
#include "lib/compiler/lexer.hh"

TEST(Lexer, Lexer)
{
    using namespace compiler;
    using V = std::vector<Token>;
    using T = Token;
    auto t = tokenize;

    ASSERT_EQ(t("42"), (V{ T(Integer(42)), T(EOF_()) }));
    ASSERT_EQ(t("42;"), (V{ T(Integer(42)), T(Symbol(";")), T(EOF_()) }));
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
