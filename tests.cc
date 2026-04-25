#include "gtest/gtest.h"

#include "lib/compiler/lexer.hh"
#include "lib/compiler/parser.hh"
#include "lib/compiler/compiler.hh"
#include "lib/compiler/ir.hh"
#include "lib/vm/vm.hh"
#include "lib/compiler/exceptions.hh"
#include "lib/vm/exception.hh"
#include "lib/vm/value.hh"

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

constexpr int32_t ExpectCompilationError = 432840923;
constexpr int32_t ExpectRuntimeError     = 432840924;
constexpr int32_t ExpectRuntimeSuccess   = 432840925;

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

    if (auto *i = std::get_if<int32_t>(&value_expected); !i || *i != ExpectRuntimeSuccess) {
        ASSERT_EQ(vm.stack().size(), 1);
        ASSERT_EQ(vm.stack().back(), vm::Value(expected));
    }
}

TEST(VM, GeneralCode)
{
    vm_test("return 42;", 42);
    vm_test("42;", ExpectCompilationError);
}

TEST(VM, LocalVariables)
{
    vm_test("a := 52; return a;", 52);
    vm_test("a := 52; b := 13; return a;", 52);
    vm_test("a := 52; b := 13; return b;", 13);
}

TEST(VM, Scopes)
{
    vm_test("a := 52; { b := 12; } return a;", 52);
    vm_test("a := 52; { b := 12; } return b;", ExpectCompilationError);
    vm_test("a := 52; { a := 12; } return a;", 52);
}

TEST(VM, VariableAssignment)
{
    vm_test("a := 12; a = 13; return a;", 13);
    vm_test("a := 12; { a := 0; a = 13; } return a;", 12);
}

TEST(VM, SimpleFunctions)
{
    vm_test("return func() { return 42; };", vm::ValueFunction(1));
    vm_test("return func() { return 42; }();", 42);
    vm_test("a := func() { return 42; }; return a();", 42);
    vm_test("a := func() { return 42; }; b := func() { return 24; }; return a();", 42);
    vm_test("a := func() { return 42; }; b := func() { return 24; }; return b();", 24);
}

TEST(VM, NestedFunctions)
{
    vm_test("a := func() { return func() { return 42; }; }; return a;", vm::ValueFunction(1));
    vm_test("a := func() { return func() { return 42; }; }; return a();", vm::ValueFunction(2));
    vm_test("a := func() { return func() { return 42; }; }; return a()();", 42);
    vm_test("a := func() { return func() { return 42; }; }; b := a(); return b();", 42);
}

TEST(VM, LocalVarsInFunctions)
{
    vm_test("a := 42; return func() { a := 12; return a; }();", 12);
}

TEST(VM, FunctionParameters)
{
    vm_test("return func(a) { return a; }(42);", 42);
    vm_test("return func(a, b) { return a; }(42, 12);", 42);
    vm_test("return func(a, b) { return b; }(42, 12);", 12);
    vm_test("return func(a, b) { c := 91; return b; }(42, 12);", 12);
    vm_test("return func(a, b) { c := 91; return c; }(42, 12);", 91);
    vm_test("return func(a, b) { c := 91; { c := 33; } return c; }(42, 12);", 91);
    vm_test("a := func(b) { return b(); }; return a(func() { return 32; });", 32);  // pass function as parameter
}

TEST(VM, Boolean)
{
    vm_test("return true;", true);
    vm_test("return false;", false);
}

TEST(VM, FlowConditional)
{
    // vm_test("if true { return 1; } return 2;", 1);
    // vm_test("if false { return 1; } return 2;", 2);
    // vm_test("if true { return 1; } else { return 2; } return 3;", 1);
    vm_test("if false { return 1; } else { return 2; } return 3;", 2);
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
