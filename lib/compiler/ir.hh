#ifndef TYCHE_IR_HH
#define TYCHE_IR_HH

#include <cstdint>
#include <variant>

namespace compiler {

enum class Operation : uint8_t {
    PushNil, PushInt, Pop,
    Return,
};

using Operand = std::variant<std::monostate, int32_t>;

struct Instruction {
    Operation operation;
    Operand operand1 = std::monostate();

    bool operator==(Instruction const& rhs) const { return operation==rhs.operation && operand1==rhs.operand1; }
};

struct IR {
    struct Function {
        std::vector<Instruction> instructions;

        bool operator==(Function const& rhs) const { return instructions==rhs.instructions; }
    };

    std::vector<Function> functions;

    bool operator==(IR const& rhs) const { return functions==rhs.functions; }
};

}

#endif //TYCHE_IR_HH
