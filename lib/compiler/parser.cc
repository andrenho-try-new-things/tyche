#include "parser.hh"

#include "parser_priv.hh"

/* Grammar:

<statements> ::= <statement>
             |   <statement> <statements>
             |   "{" <statements> "}"

<statement>  ::= "return" <expr> ";"
             |   IDENTIFIER ":=" <expr> ";"
             |   "if" <expr> "{" <statements> "}" ["elseif" <expr> "{" <statements> "}"...] ["else" "{" <statements> "}"]
             |   <EOF>

<expr> ::= <expr> ["(" function_call_parameters ")"...]
       |   "func" "(" function_parameters ")" "{" statements "}"
       |   "true" | "false"
       |   <expr> "?" <expr> ":" <expr>
       |   INTEGER
       |   IDENTIFIER

<function_parameter> ::= IDENTIFIER "," <function_parameter>
                     |   IDENTIFIER

<function_call_parameters> ::= <expr> "," <expr>
                           |   <expr>

*/

using namespace vm;

namespace compiler {

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
    ir_add_op(Operation::Return);
}

void Parser::local_variable_retrieval(std::string const& identifier)
{
    if (auto o_idx = find_local_variable(identifier); o_idx)
        ir_add_op(Operation::GetLocal, *o_idx);
    else
        throw CompilationError("Could not find local variable '" + identifier + "'", latest_token_.line, latest_token_.column);
}

void Parser::local_variable_declaration(std::string const& identifier)
{
    expr();
    expect_symbol(";");
    add_local_variable(identifier);
    ir_add_op(Operation::SetLocal, (int32_t) ir_.functions.at(current_function_id()).add_variable(identifier));
}

void Parser::local_variable_assignment(std::string const& identifier)
{
    expr();
    expect_symbol(";");

    if (auto o_idx = find_local_variable(identifier); o_idx)
        ir_add_op(Operation::SetLocal, *o_idx);
    else
        throw CompilationError("Could not find local variable '" + identifier + "'", latest_token_.line, latest_token_.column);
}

void Parser::variable(std::string const& identifier)
{
    Token t = ingest_token();
    if (t.is_symbol(":="))
        local_variable_declaration(identifier);
    else if (t.is_symbol("="))
        local_variable_assignment(identifier);
    else
        throw CompilationError("Expected ':=' or '='", t.line, t.column);
}

void Parser::expr(int min_bp)
{
    // supporting functions

    struct BindingPower { int left; int right; };
    auto infix_bp = [](Token const& t) -> BindingPower {
        if (t.is_symbol("+"))
            return { 10, 11 };
        if (t.is_symbol("*"))
            return { 20, 21 };
        return { 0, 0 };
    };

    auto prefix_bp = [](Token const& t) -> int {
        return 0;
    };

    Token t = ingest_token();

    // prefix

    if (t.is_symbol("(")) {   // parenthesis
        expr(min_bp);
        expect_symbol(")");
    } else {                  // numbers or identifiers
        if (auto o_int = t.integer(); o_int)
            ir_add_op(Operation::PushInt, *o_int);
        else if (t.is_identifier("func"))
            function();
        else if (t.is_identifier("true"))
            ir_add_op(Operation::PushTrue);
        else if (t.is_identifier("false"))
            ir_add_op(Operation::PushFalse);
        else if (auto o_id = t.identifier(); o_id)   // any other identifier
            local_variable_retrieval(*o_id);
        else
            throw CompilationError("Unexpected token in expression", latest_token_.line, latest_token_.column);
    }

    // infix

    for (;;) {
        Token tt = peek_token();
        BindingPower bp = infix_bp(tt);

        if (bp.left == 0 || bp.left < min_bp)
            break;

        ingest_token();
        expr(bp.right);

        // parse expression
        if (tt.is_symbol("+"))
            ir_add_op(Operation::Sum);
        else if (tt.is_symbol("*"))
            ir_add_op(Operation::Multiplication);
    }

    // ternary expressions (TODO)
    if (peek_symbol("?"))
        ternary_expr();

    // function calls (TODO)
    while (peek_symbol("(")) {
        function_call();
    }
}

void Parser::function_call()
{
    expect_symbol("(");

    size_t n_pars = 0;
    if (!peek_symbol(")")) {
        for (;;) {
            expr();
            ++n_pars;
            if (peek_symbol(")"))
                break;
            else if (!ingest_token().is_symbol(","))
                throw CompilationError("On function call, expected ')' or ','", latest_token_.line, latest_token_.column);
        }
    }

    expect_symbol(")");
    ir_add_op(Operation::Call, n_pars);
}

bool Parser::statement()
{
    Token t = ingest_token();

    if (t.is_identifier("return"))
        return_();
    else if (t.is_identifier("if"))
        if_();
    else if (auto o_id = t.identifier(); o_id)  // any other identifier
        variable(*o_id);
    else if (t.is_eof())
        return true;
    else
        throw CompilationError("Invalid statement", t.line, t.column);

    return false;
}

void Parser::if_()
{
    auto skip_to_end = ir_create_unknown_key();

    // if
    expr();
    auto skip_to_next = ir_create_unknown_key();
    ir_add_op(Operation::BranchFalse, skip_to_next);
    expect_symbol("{");
    push_scope();
    statements(1);
    if (peek_identifier("else") || peek_identifier("elseif"))
        ir_add_op(Operation::Jump, skip_to_end);

    // elseif
    while (peek_identifier("elseif")) {
        ingest_token();
        ir_resolve_to_instruction_idx(skip_to_next);
        skip_to_next = ir_create_unknown_key();
        expr();
        ir_add_op(Operation::BranchFalse, skip_to_next);
        expect_symbol("{");
        push_scope();
        statements(1);
        ir_add_op(Operation::Jump, skip_to_end);
    }

    ir_resolve_to_instruction_idx(skip_to_next);

    // else
    if (peek_identifier("else")) {
        ingest_token();
        expect_symbol("{");
        push_scope();
        statements(1);
    }

    ir_resolve_to_instruction_idx(skip_to_end);
}

void Parser::ternary_expr()
{
    expect_symbol("?");

    auto skip_to_false = ir_create_unknown_key();
    auto skip_to_end = ir_create_unknown_key();

    ir_add_op(Operation::BranchFalse, skip_to_false);
    expr();
    ir_add_op(Operation::Jump, skip_to_end);

    expect_symbol(":");

    ir_resolve_to_instruction_idx(skip_to_false);
    expr();
    ir_resolve_to_instruction_idx(skip_to_end);
}


void Parser::function()
{
    // function parameters
    expect_symbol("(");
    auto parameters = function_parameters();

    // add function
    expect_symbol(")");
    size_t function_id = start_function();
    push_scope();

    // add parameters as local variables to parser
    for (auto const& parameter: parameters)
        add_local_variable(parameter);
    current_function().n_parameters = parameters.size();

    // add parameters as local variables to IR
    ir_.functions.back().n_parameters = parameters.size();
    for (auto const& parameter: parameters)
        ir_.functions.back().local_vars.emplace_back(parameter);

    // parse rest of the function
    expect_symbol("{");
    statements(1);       // scope level is 1 because we already ingested the "{"
    end_function();
    ir_add_op(Operation::PushFunction, function_id);
}

std::vector<std::string> Parser::function_parameters()
{
    // function parameters
    std::vector<std::string> parameters;
    if (!peek_symbol(")")) {
        for (;;) {
            if (!peek_token().is_identifier())
                throw CompilationError("Expected identifier or ')' for function parameter", latest_token_.line, latest_token_.column);

            Token t = ingest_token();
            if (!t.is_identifier())
                throw CompilationError("Expected parameter name", latest_token_.line, latest_token_.column);
            parameters.push_back(*t.identifier());

            if (peek_symbol(")"))
                break;
            else if (!ingest_token().is_symbol(","))
                throw CompilationError("Expected ')' or ',' in function parameter list.", latest_token_.line, latest_token_.column);
        }
    }

    return parameters;
}

//
// UTILS
//

template <typename... Args>
void Parser::ir_add_op(Args... args)
{
    ir_.functions.at(current_function_id()).add_instruction(args...);
}

void Parser::add_local_variable(std::string const& identifier)
{
    current_function().scope_stack.back().local_vars.push_back({
            .name = identifier,
            .idx = current_function().n_local_vars++,
    });
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
    ir_add_op(Operation::ReturnNil);
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

bool Parser::peek_identifier(const std::string& symbol) const
{
    return peek_token().is_identifier(symbol);
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