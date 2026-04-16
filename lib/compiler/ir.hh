#ifndef TYCHE_IR_HH
#define TYCHE_IR_HH

#include <cstdint>
#include <variant>

#include "../vm/instruction.hh"

namespace compiler {

struct IR {
    struct Variable {
        std::string name;
    };

    struct Function {
        std::vector<vm::Instruction> instructions;
        std::vector<Variable>        local_vars;

        size_t add_variable(std::string const& var_name) { local_vars.emplace_back(var_name); return local_vars.size() - 1; }

        bool operator==(Function const& rhs) const { return instructions==rhs.instructions; }
    };

    std::vector<Function> functions;

    bool operator==(IR const& rhs) const { return functions==rhs.functions; }
};

}

#endif //TYCHE_IR_HH
