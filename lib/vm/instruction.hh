#ifndef TYCHE_INSTRUCTION_HH
#define TYCHE_INSTRUCTION_HH

namespace vm {

enum class Operation : uint8_t {
    PushNil, PushInt, Pop,
    Return, ReturnNil,
};

using Operand = std::variant<std::monostate, int32_t>;

struct Instruction {
    Operation operation;
    Operand operand1 = std::monostate();

    bool operator==(Instruction const& rhs) const { return operation==rhs.operation && operand1==rhs.operand1; }
};

}

#endif //TYCHE_INSTRUCTION_HH
