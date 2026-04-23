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
        :tokens_({tks.begin(), tks.end() }),
         ir_({ .functions = {{} } }),
         function_id_stack_({ 0 }),
         functions_({Function { .n_local_vars = 0, .scope_stack = {{} } } }),
         latest_token_(Token(Integer {0}, 0, 0))
     {}

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

    std::list<Token>      tokens_;
    IR                    ir_;
    std::stack<size_t>    function_id_stack_;
    std::vector<Function> functions_;
    mutable Token         latest_token_;

    // parsing
    void statements(int scope_level = 0);
    bool statement();
    void get_local_variable(std::string const& identifier);
    void declare_local_variable(std::string const& identifier);
    void assign_local_variable(std::string const& identifier);
    void variable(std::string const& identifier);
    void expr();
    void return_();
    void function();
    void function_call();

    // scope/function management
    void push_scope();
    void pop_scope();
    size_t start_function();
    void   end_function();

    // token management
    [[nodiscard]] Token peek_token() const;
    [[nodiscard]] bool peek_symbol(std::string const& symbol) const;
    Token ingest_token();
    void expect_symbol(std::string const& symbol);

    // utils
    [[nodiscard]] std::optional<int32_t> find_local_variable(std::string const& identifier) const;
    [[nodiscard]] Function& current_function() { return functions_.at(current_function_id()); }
    [[nodiscard]] Function const& current_function() const { return functions_.at(current_function_id()); }
    [[nodiscard]] size_t current_function_id() const { return function_id_stack_.top(); }

    template <typename... Args> void add_op(Args... args);
};

//
// PARSER
//

IR Parser::parse()
{
    statements();
    end_function();

    if (!tokens_.empty())
        throw CompilationError("Additional text in the end of the file", 0, 0);

    return ir_;
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

void Parser::get_local_variable(std::string const& identifier)
{
    if (auto o_idx = find_local_variable(identifier); o_idx)
        add_op(Operation::GetLocal, *o_idx);
    else
        throw CompilationError("Could not find local variable '" + identifier + "'", latest_token_.line, latest_token_.column);
}

void Parser::declare_local_variable(std::string const& identifier)
{
    expr();
    expect_symbol(";");
    current_function().scope_stack.back().local_vars.push_back({
            .name = identifier,
            .idx = current_function().n_local_vars++,
    });
    add_op(Operation::SetLocal, (int32_t) ir_.functions.at(current_function_id()).add_variable(identifier));
}

void Parser::assign_local_variable(std::string const& identifier)
{
    expr();
    expect_symbol(";");

    if (auto o_idx = find_local_variable(identifier); o_idx)
        add_op(Operation::SetLocal, *o_idx);
    else
        throw CompilationError("Could not find local variable '" + identifier + "'", latest_token_.line, latest_token_.column);
}

void Parser::variable(std::string const& identifier)
{
    Token t = ingest_token();
    if (t.is_symbol(":="))
        declare_local_variable(identifier);
    else if (t.is_symbol("="))
        assign_local_variable(identifier);
    else
        throw CompilationError("Expected ':=' or '='", t.line, t.column);
}

void Parser::expr()
{
    Token t = ingest_token();
    if (auto o_int = t.integer(); o_int)
        add_op(Operation::PushInt, *o_int);
    else if (t.is_identifier("func"))
        function();
    else if (auto o_id = t.identifier(); o_id)   // any other identifier
        get_local_variable(*o_id);
    else
        throw CompilationError("Invalid expression", t.line, t.column);

    if (peek_symbol("(")) {
        function_call();
    }
}

void Parser::function_call()
{
    expect_symbol("(");
    // TODO - parse parameters
    expect_symbol(")");
    add_op(Operation::Call, 0);
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

void Parser::function()
{
    expect_symbol("(");
    // TODO - function parameters
    expect_symbol(")");
    start_function();
    push_scope();
    expect_symbol("{");
    statements(1);       // scope level is 1 because we already ingested the "{"
    end_function();
}

//
// UTILS
//

template <typename... Args>
void Parser::add_op(Args... args)
{
    ir_.functions.at(current_function_id()).instructions.emplace_back(args...);
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

size_t Parser::start_function()
{
    size_t id = functions_.size();
    functions_.emplace_back();
    function_id_stack_.push(id);
    ir_.functions.emplace_back();
    return id;
}

void Parser::end_function()
{
    add_op(Operation::ReturnNil);
    function_id_stack_.pop();
}

//
// TOKEN MANAGEMENT
//

Token Parser::peek_token() const
{
    latest_token_ = tokens_.front();
    return tokens_.front();
}

Token Parser::ingest_token()
{
    Token t = std::move(tokens_.front());
    tokens_.pop_front();
    latest_token_ = t;
    return t;
}

bool Parser::peek_symbol(std::string const& symbol) const
{
    return peek_token().is_symbol(symbol);
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