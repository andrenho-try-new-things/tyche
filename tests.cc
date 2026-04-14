#include "gtest/gtest.h"
#include "lib/compiler/lexer.hh"

TEST(Lexer, Lexer)
{
    using namespace compiler;
    using V = std::vector<Token>;
#define CHECK(a, b) ASSERT_EQ(tokenize(a), b);

    CHECK("42", (V{ Integer(42), EOF_() }))

#undef CHECK
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
