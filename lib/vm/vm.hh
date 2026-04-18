#ifndef TYCHE_VM_HH
#define TYCHE_VM_HH

#include "basevm.hh"
#include "bytecode.hh"

namespace vm {

class VM : public BaseVM {
public:
    void load(vm::Bytecode&& bytecode);

    void run();
    void run_debug();

private:
    vm::Bytecode bytecode_;
    Location     loc_ { 0, 0 };

    struct Function {
        FunctionId         id;
        std::vector<Value> vars;
    };
    std::stack<Function> function_;

    bool step();
    bool step_debug();

    void enter_function(FunctionId f_id);
    void exit_function();
};

}

#endif //TYCHE_VM_HH
