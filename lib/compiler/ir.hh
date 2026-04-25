#ifndef TYCHE_IR_HH
#define TYCHE_IR_HH

#include <cstdint>
#include <variant>
#include <unordered_map>

#include "../vm/instruction.hh"

namespace compiler {

struct IR {
    struct Variable {
        std::string name;

        bool operator==(Variable const& var) const = default;
    };

    struct Function {
        std::vector<vm::Instruction> instructions {};
        std::vector<Variable>        local_vars {};
        size_t                       n_parameters = 0;

        size_t add_variable(std::string const& var_name) {
            local_vars.push_back({ var_name });
            return local_vars.size() - 1;
        }

        bool operator==(Function const& rhs) const
        {
            return std::tie(instructions) == std::tie(rhs.instructions)
                && std::tie(local_vars) == std::tie(rhs.local_vars);
        }
    };

    std::vector<Function> functions;

    bool operator==(IR const& rhs) const { return functions==rhs.functions; }
};

}

#endif //TYCHE_IR_HH
