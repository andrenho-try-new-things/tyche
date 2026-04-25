#ifndef TYCHE_VM_HH
#define TYCHE_VM_HH

#include "bytecode.hh"
#include "value.hh"

#include <stack>
#include <vector>

namespace vm {

class VM {
public:
    void load(vm::Bytecode&& bytecode);

    // execution
    void run();
    void run_debug();

    // information
    [[nodiscard]] std::vector<Value> const& stack() const { return stack_; }
    [[nodiscard]] std::string debug_stack() const;

private:
    vm::Bytecode bytecode_;
    Location     loc_ { 0, 0 };

    std::vector<Value> stack_;

    struct Function {
        FunctionId         id;
        std::vector<Value> vars;
        Location           return_loc;
    };
    std::stack<Function> function_;

    bool step();
    bool step_debug();

    void enter_function(FunctionId f_id, size_t return_pc, size_t n_parameters);
    void exit_function();

    Value pop_value();
    void  push_value(Value const& val);
};

}

#endif //TYCHE_VM_HH
