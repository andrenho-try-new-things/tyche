#ifndef TYCHE_IR_HH
#define TYCHE_IR_HH

#include <cstdint>
#include <variant>

#include "../vm/instruction.hh"

namespace compiler {

struct IR {
    struct Function {
        std::vector<vm::Instruction> instructions;

        bool operator==(Function const& rhs) const { return instructions==rhs.instructions; }
    };

    std::vector<Function> functions;

    bool operator==(IR const& rhs) const { return functions==rhs.functions; }
};

}

#endif //TYCHE_IR_HH
