#include "parser.hh"

#include <list>
#include <stack>

#include "exceptions.hh"

/* Grammar:

<statements> ::= <statement>
             |   <statement> <statements>
             |   "{" <statements> "}"

<statement>  ::= "return" <expr> ";"
             |   IDENTIFIER ":=" <expr> ";"
             |   <EOF>

<expr> ::= "func" "(" ")" "{" statements "}"
       |   INTEGER
       |   IDENTIFIER

*/

using namespace vm;

namespace compiler {

class Parser {
public:
    explicit Parser(std::vector<Token> const& tks)
        : tokens({ tks.begin(), tks.end() }),
          ir({ .functions = { {} } }),
          current_function_id(0),
          functions({ Function { .n_local_vars = 0, .scope_stack = { {} } } }) {}

    IR parse();

private:
    struct LocalVar {
        std::string name;
        size_t      idx;
    };

    struct Scope {
        std::vector<LocalVar> local_vars {};
    };

    struct Function {
        size_t             n_local_vars = 0;
        std::vector<Scope> scope_stack {};
    };

    std::list<Token>      tokens;
    IR                    ir;
    size_t                current_function_id;
    std::vector<Function> functions;

    // parsing
    void statements(int scope_level = 0);
    bool statement();
    void get_local_variable(std::string const& identifier, Token const& t);
    void declare_local_variable(std::string const& identifier);
    void assign_local_variable(std::string const& identifier, Token const& t);
    void variable(std::string const& identifier);
    void expr();
    void return_();

    // scope/function management
    void push_scope();
    void pop_scope();
    void end_function();

    // token management
    [[nodiscard]] Token peek_token() const;
    Token ingest_token();
    void expect_symbol(std::string const& symbol);

    // utils
    [[nodiscard]] std::optional<int32_t> find_local_variable(std::string const& identifier) const;
    [[nodiscard]] Function& current_function() { return functions.at(current_function_id); }
    [[nodiscard]] Function const& current_function() const { return functions.at(current_function_id); }

    template <typename... Args> void add_op(Args... args);
};

//
// PARSER
//

IR Parser::parse()
{
    statements();
    end_function();

    if (!tokens.empty())
        throw CompilationError("Additional text in the end of the file", 0, 0);

    return ir;
}

void Parser::statements(int scope_level)
{
    for (;;) {
        Token t = peek_token();

        // open/close brackes
        if (t.is_symbol("{")) {
            ingest_token();
            push_scope();
            statements(scope_level + 1);
        } else if (t.is_symbol("}")) {
            ingest_token();
            if (scope_level == 0)
                throw CompilationError("Unexpected '}' in statement", t.line, t.column);
            pop_scope();
            return;
        }

        // normal statement
        if (statement())
            break;
    }
}

void Parser::return_()
{
    expr();
    expect_symbol(";");
    add_op(Operation::Return);
}

void Parser::get_local_variable(std::string const& identifier, Token const& t)
{
    if (auto o_idx = find_local_variable(identifier); o_idx)
        add_op(Operation::GetLocal, *o_idx);
    else
        throw CompilationError("Could not find local variable '" + identifier + "'", t.line, t.column);
}

void Parser::declare_local_variable(std::string const& identifier)
{
    expr();
    expect_symbol(";");
    current_function().scope_stack.back().local_vars.push_back({
            .name = identifier,
            .idx = current_function().n_local_vars++,
    });
    add_op(Operation::SetLocal, (int32_t) ir.functions.at(current_function_id).add_variable(identifier));
}

void Parser::assign_local_variable(std::string const& identifier, Token const& t)
{
    expr();
    expect_symbol(";");

    if (auto o_idx = find_local_variable(identifier); o_idx)
        add_op(Operation::SetLocal, *o_idx);
    else
        throw CompilationError("Could not find local variable '" + identifier + "'", t.line, t.column);
}

void Parser::variable(std::string const& identifier)
{
    Token t = ingest_token();
    if (t.is_symbol(":="))
        declare_local_variable(identifier);
    else if (t.is_symbol("="))
        assign_local_variable(identifier, t);
    else
        throw CompilationError("Expected ':=' or '='", t.line, t.column);
}

void Parser::expr()
{
    Token t = ingest_token();
    if (auto o_int = t.integer(); o_int)
        add_op(Operation::PushInt, *o_int);
    else if (auto o_id = t.identifier(); o_id)
        get_local_variable(*o_id, t);
    else
        throw CompilationError("Invalid expression", t.line, t.column);
}

bool Parser::statement()
{
    Token t = ingest_token();

    if (t.is_identifier("return"))
        return_();
    else if (auto o_id = t.identifier(); o_id)  // any other identifier
        variable(*o_id);
    else if (t.is_eof())
        return true;
    else
        throw CompilationError("Invalid statement", t.line, t.column);

    return false;
}

//
// UTILS
//

template <typename... Args>
void Parser::add_op(Args... args)
{
    ir.functions.at(current_function_id).instructions.emplace_back(args...);
}

std::optional<int32_t> Parser::find_local_variable(std::string const& identifier) const
{
    auto const& scopes = current_function().scope_stack;
    for (auto const& scope : scopes | std::views::reverse) {
        for (auto const& local_var: scope.local_vars) {
            if (local_var.name == identifier) {
                return (int32_t) local_var.idx;
            }
        }
    }

    return {};
}

//
// SCOPE/FUNCTION MANAGEMENT
//

void Parser::push_scope()
{
    current_function().scope_stack.emplace_back();
}

void Parser::pop_scope()
{
    current_function().scope_stack.pop_back();
}


void Parser::end_function()
{
    add_op(Operation::ReturnNil);
}

//
// TOKEN MANAGEMENT
//

Token Parser::peek_token() const
{
    return tokens.front();
}

Token Parser::ingest_token()
{
    Token t = std::move(tokens.front());
    tokens.pop_front();
    return t;
}

void Parser::expect_symbol(std::string const& symbol)
{
    Token t = ingest_token();
    if (!t.is_symbol(symbol))
        throw CompilationError("Expected '" + symbol + "'", t.line, t.column);
}


//
// PUBLIC FUNCTION
//

IR parse(std::vector<Token> const& tks)
{
    return Parser(tks).parse();
}

}