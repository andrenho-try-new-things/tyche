#include "parser.hh"

#include <list>

#include "exceptions.hh"

using namespace vm;

/* Grammar:

<statements> ::= <statement> | <statement> <statements>

<statement>  ::= "return" <expr> ";"
             |   IDENTIFIER ":=" <expr> ";"
             |   <EOF>

<expr> ::= INTEGER

*/

#define H std::holds_alternative

namespace compiler {

struct Context {
    std::list<Token> tokens;
    IR               ir;
    size_t           function_id;
};

static Token ingest_token(Context& ctx)
{
    Token t = std::move(ctx.tokens.front());
    ctx.tokens.pop_front();
    return t;
}

static void expect_symbol(Context& ctx, std::string const& symbol)
{
    Token t = ingest_token(ctx);
    if (!H<Symbol>(t.token) || std::get<Symbol>(t.token).symbol != symbol)
        throw CompilationError("Expected '" + symbol + "'", t.line, t.column);
}

template <typename... Args>
static void add_op(Context& ctx, Args... args)
{
    ctx.ir.functions.at(ctx.function_id).instructions.emplace_back(args...);
}

static void expr(Context& ctx)
{
    Token t = ingest_token(ctx);
    if (auto *i = std::get_if<Integer>(&t.token)) {
        add_op(ctx, Operation::PushInt, i->value);
    } else {
        throw CompilationError("Invalid expression", t.line, t.column);
    }
}

static void statements(Context& ctx)
{
    for (;;) {
        Token t = ingest_token(ctx);

        if (auto *id = std::get_if<Identifier>(&t.token)) {
            if (id->identifier == "return") {
                expr(ctx);
                expect_symbol(ctx, ";");
                add_op(ctx, Operation::Return);
            } else {
                expect_symbol(ctx, ":=");
                expr(ctx);
                expect_symbol(ctx, ";");
                add_op(ctx, Operation::SetLocal, (int32_t) ctx.ir.functions.at(ctx.function_id).add_variable(id->identifier));
            }

        } else if (H<EOF_>(t.token)) {
            return;

        } else {
            throw CompilationError("Invalid statement", t.line, t.column);
        }
    }
}

static void end_function(Context& ctx)
{
    add_op(ctx, Operation::ReturnNil);
}

IR parse(std::vector<Token> const& tks)
{
    Context ctx = {
        .tokens = { tks.begin(), tks.end() },
        .ir = { .functions = { {} } },
        .function_id = 0,
    };

    statements(ctx);
    end_function(ctx);

    if (!ctx.tokens.empty())
        throw CompilationError("Additional text in the end of the file", 0, 0);

    return ctx.ir;
}

}