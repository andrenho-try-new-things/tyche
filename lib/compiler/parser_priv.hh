#ifndef TYCHE_PARSER_PRIV_HH
#define TYCHE_PARSER_PRIV_HH

#include <list>
#include <stack>
#include <vector>

#include "exceptions.hh"
#include "lexer.hh"
#include "ir.hh"
#include "../vm/instruction.hh"

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
    void add_op(vm::Operation operation, size_t arg) { add_op(operation, (int32_t) arg); }
};

}

#endif //TYCHE_PARSER_PRIV_HH
