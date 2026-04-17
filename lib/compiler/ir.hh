#ifndef TYCHE_IR_HH
#define TYCHE_IR_HH

#include <cstdint>
#include <variant>
#include <unordered_map>

#include "../vm/instruction.hh"

namespace compiler {

struct IR {
    struct Variable {
        size_t index;
    };

    struct Function {
        std::vector<vm::Instruction>              instructions;
        std::unordered_map<std::string, Variable> local_vars;

        size_t add_variable(std::string const& var_name) {
            size_t idx = local_vars.size();
            local_vars[var_name] = { .index = idx };
            return idx;
        }

        bool operator==(Function const& rhs) const
        {
            return std::tie(instructions)==std::tie(rhs.instructions);  // TODO - add local vars comparison
        }
    };

    std::vector<Function> functions;

    bool operator==(IR const& rhs) const { return functions==rhs.functions; }
};

}

#endif //TYCHE_IR_HH
