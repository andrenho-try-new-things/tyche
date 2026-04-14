#include "gtest/gtest.h"

#include "lib/compiler/lexer.hh"
#include "lib/compiler/parser.hh"
#include "lib/compiler/ir.hh"

TEST(Lexer, Lexer)
{
    using namespace compiler;
    using V = std::vector<Token>;
    using T = Token;
    auto t = tokenize;

    ASSERT_EQ(t("42"), (V{ T(Integer(42)), T(EOF_()) }));
    ASSERT_EQ(t("42;"), (V{ T(Integer(42)), T(Symbol(";")), T(EOF_()) }));
    ASSERT_EQ(t("42.8;"), (V{ T(Float(42.8f)), T(Symbol(";")), T(EOF_()) }));
    ASSERT_EQ(t("@= 42;"), (V{ T(Symbol("@=")), T(Integer(42)), T(Symbol(";")), T(EOF_()) }));
    ASSERT_EQ(t("@=;"), (V{ T(Symbol("@=")), T(Symbol(";")), T(EOF_()) }));
    ASSERT_EQ(t("@=id;"), (V{ T(Symbol("@=")), T(Identifier("id")), T(Symbol(";")), T(EOF_()) }));
    ASSERT_EQ(t(R"(hello = "Hello world!")"), (V{ T(Identifier("hello")), T(Symbol("=")), T(String("Hello world!")), T(EOF_()) }));
    ASSERT_EQ(t(R"("a\nb")"), (V{ T(String("a\nb")), T(EOF_()) }));
    ASSERT_EQ(t(R"("a\"b")"), (V{ T(String("a\"b")), T(EOF_()) }));
    ASSERT_EQ(t(R"("a\\b")"), (V{ T(String("a\\b")), T(EOF_()) }));
    ASSERT_EQ(t(R"("a\gb")"), (V{ T(String("a\\gb")), T(EOF_()) }));
}

TEST(Parser, Parser)
{
    using namespace compiler;
    auto CHECK = [](std::string const& code, IR const& expected) {
        ASSERT_EQ(parse(tokenize(code)), expected);
    };
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
