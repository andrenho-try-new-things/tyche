#ifndef TYCHE_INSTRUCTION_HH
#define TYCHE_INSTRUCTION_HH

#include <ostream>

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

    friend std::ostream& operator<<(std::ostream& os, Instruction const& i) {
        switch (i.operation) {
            case Operation::PushNil:    os << "PUSHNIL"; break;
            case Operation::PushInt:    os << "PUSHINT " << std::get<int32_t>(i.operand1); break;
            case Operation::Pop:        os << "POP"; break;
            case Operation::Return:     os << "RET"; break;
            case Operation::ReturnNil:  os << "RETNIL"; break;
            default:                    os << "???"; break;
        }
        return os;
    }
};

}

#endif //TYCHE_INSTRUCTION_HH
