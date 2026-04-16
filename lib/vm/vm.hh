#ifndef TYCHE_VM_HH
#define TYCHE_VM_HH

#include "basevm.hh"
#include "bytecode.hh"

namespace vm {

class VM : public BaseVM {
public:
    void load(vm::Bytecode&& bytecode) { bytecode_ = std::move(bytecode); }

    void run();
    void run_debug();

private:
    vm::Bytecode bytecode_;
    Location     loc_ { 0, 0 };

    bool step();
    bool step_debug();
};

}

#endif //TYCHE_VM_HH
