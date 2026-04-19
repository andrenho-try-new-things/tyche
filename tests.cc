#include "gtest/gtest.h"

#include "lib/compiler/lexer.hh"
#include "lib/compiler/parser.hh"
#include "lib/compiler/compiler.hh"
#include "lib/compiler/ir.hh"
#include "lib/vm/vm.hh"
#include "lib/compiler/exceptions.hh"
#include "lib/vm/exception.hh"

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
    using namespace vm;
    auto compile_to_ir = [](std::string const& code) { return parse(tokenize(code)); };

    ASSERT_EQ(compile_to_ir("return 42;"), (IR {
        .functions = {
           { .instructions = {
               { Operation::PushInt, 42 },
               { Operation::Return },
               { Operation::ReturnNil },
           } }
        }
    }));
}

TEST(BaseVM, StackOperations)
{
    using namespace vm;
    vm::BaseVM vm;

    ASSERT_TRUE(vm.stack().empty());

    vm.push_integer(542);
    ASSERT_EQ(vm.stack().size(), 1);
    ASSERT_EQ(vm.stack().back(), Value(542));
}

constexpr int32_t ExpectCompilationError = 432840923;
constexpr int32_t ExpectRuntimeError = 432840924;

template <typename T>
void vm_test(std::string const& code, T const& expected)
{
    vm::Value value_expected(expected);

    static const char* RULER = "-------------------\n";
    std::cout << code << "\n" << RULER;

    auto tokens = compiler::tokenize(code);
    std::cout << "Tokens:\n";
    for (auto const& token: tokens) std::cout << "[" << token << "] ";
    std::cout << "\n" << RULER;

    if (auto *i = std::get_if<int32_t>(&value_expected); i && *i == ExpectCompilationError) {
        ASSERT_THROW(compiler::compile(code, true), compiler::CompilationError);
        return;
    }

    vm::Bytecode bytecode = compiler::compile(code, true);
    std::cout << "Bytecode output:\n" << bytecode << RULER;

    vm::VM vm;
    vm.load(std::move(bytecode));

    std::cout << "VM execution:\n";
    if (auto *i = std::get_if<int32_t>(&value_expected); i && *i == ExpectRuntimeError) {
        ASSERT_THROW(vm.run_debug(), vm::ExecutionException);
        return;
    }

    vm.run_debug();
    std::cout << RULER;

    ASSERT_EQ(vm.stack().size(), 1);
    ASSERT_EQ(vm.stack().back(), vm::Value(expected));
}

TEST(VM, GeneralCode)
{
    vm_test("return 42;", 42);
    vm_test("42;", ExpectCompilationError);
}

TEST(VM, LocalVariables) {
    vm_test("a := 52; return a;", 52);
    vm_test("a := 52; b := 13; return a;", 52);
    vm_test("a := 52; b := 13; return b;", 13);
}

TEST(VM, Scopes) {
    vm_test("a := 52; { b := 12; } return a;", 52);
    vm_test("a := 52; { b := 12; } return b;", ExpectCompilationError);
    vm_test("a := 52; { a := 12; } return a;", 52);
}

TEST(VM, VariableAssignment) {
    vm_test("a := 12; a = 13; return a;", 13);
}

/*
TEST(VM, Functions) {
    vm_test("return func() { return 42; }();", 42);
}
 */

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
