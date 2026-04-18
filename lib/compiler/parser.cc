#include "parser.hh"

#include <list>

#include "exceptions.hh"

using namespace vm;

/* Grammar:

<statements> ::= <statement> | <statement> <statements>

<statement>  ::= "return" <expr> ";"
             |   IDENTIFIER ":=" <expr> ";"
             |   <EOF>

<expr> ::= "func" "(" ")" "{" statements "}"
       |   INTEGER
       |   IDENTIFIER

*/

#define H std::holds_alternative

namespace compiler {

struct Context {
    std::list<Token> tokens;
    IR               ir;
    size_t           function_id;
};

static void expr(Context& ctx);
static bool statement(Context& ctx);

static Token peek_token(Context& ctx)
{
    return ctx.tokens.front();
}

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

static void return_(Context& ctx)
{
    expr(ctx);
    expect_symbol(ctx, ";");
    add_op(ctx, Operation::Return);
}

/*
static void function(Context& ctx)
{
    expect_symbol(ctx, "(");
    // TODO - function parameters
    expect_symbol(ctx, ")");
    expect_symbol(ctx, "{");
    statement(ctx);
    expect_symbol(ctx, "}");
}
*/

static void get_local_variable(std::string const& identifier, Context& ctx, Token const& t)
{
    auto const& vars = ctx.ir.functions.at(ctx.function_id).local_vars;
    auto it = vars.find(identifier);
    if (it == vars.end())
        throw CompilationError("Could not find local variable '" + identifier + "'", t.line, t.column);
    else
        add_op(ctx, Operation::GetLocal, (int32_t) it->second.index);
}

static void set_local_variable(std::string const& identifier, Context& ctx)
{
    expect_symbol(ctx, ":=");
    expr(ctx);
    expect_symbol(ctx, ";");
    add_op(ctx, Operation::SetLocal, (int32_t) ctx.ir.functions.at(ctx.function_id).add_variable(identifier));
}

static void expr(Context& ctx)
{
    Token t = ingest_token(ctx);
    if (auto *i = std::get_if<Integer>(&t.token)) {
        add_op(ctx, Operation::PushInt, i->value);
    } else if (auto *id = std::get_if<Identifier>(&t.token)) {
        get_local_variable(id->identifier, ctx, t);
    } else {
        throw CompilationError("Invalid expression", t.line, t.column);
    }
}

static bool statement(Context& ctx)
{
    Token t = ingest_token(ctx);

    if (auto *id = std::get_if<Identifier>(&t.token)) {
        if (id->identifier == "return")
            return_(ctx);
        else
            set_local_variable(id->identifier, ctx);

    } else if (H<EOF_>(t.token)) {
        return true;

    } else {
        throw CompilationError("Invalid statement", t.line, t.column);
    }

    return false;
}

static void statements(Context& ctx, int scope_level = 0)
{
    for (;;) {
        Token t = peek_token(ctx);

        // open/close brackes
        if (auto *s = std::get_if<Symbol>(&t.token)) {
            if (s->symbol == "{") {
                ingest_token(ctx);
                // TODO - enter scope
                statements(ctx, scope_level + 1);
            } else if (s->symbol == "}") {
                ingest_token(ctx);
                if (scope_level == 0)
                    throw CompilationError("Unexpected '}' in statement", t.line, t.column);
                // TODO - exit scope
                return;
            }
        }

        // normal statement
        if (statement(ctx))
            break;
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