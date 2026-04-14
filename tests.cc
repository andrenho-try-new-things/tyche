#include "gtest/gtest.h"
#include "lib/compiler/lexer.hh"

TEST(Lexer, Lexer)
{
    using namespace compiler;
    using V = std::vector<Token>;

    ASSERT_EQ(tokenize("@=42;"), (V{ Symbol("@="), Integer(42), Symbol(";") }));
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
