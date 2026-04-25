#ifndef TYCHE_INSTRUCTION_HH
#define TYCHE_INSTRUCTION_HH

#include <ostream>

namespace vm {

enum class Operation : uint8_t {
    PushNil, PushTrue, PushFalse, PushInt, PushFunction, Pop,
    Return, ReturnNil,
    GetLocal, SetLocal,
    Call,
    BranchFalse,
};

struct Label {
    size_t instruction_idx;
};

using Operand = std::variant<std::monostate, int32_t, Label*>;

struct Instruction {
    Operation operation;
    Operand operand1 = std::monostate();

    bool operator==(Instruction const& rhs) const { return operation==rhs.operation && operand1==rhs.operand1; }

    friend std::ostream& operator<<(std::ostream& os, Instruction const& i) {
        switch (i.operation) {
            case Operation::PushNil:        os << "PUSHNIL"; break;
            case Operation::PushInt:        os << "PUSHINT " << std::get<int32_t>(i.operand1); break;
            case Operation::PushTrue:       os << "PUSHTRUE"; break;
            case Operation::PushFalse:      os << "PUSHFALSE"; break;
            case Operation::PushFunction:   os << "PUSHFUNC @" << std::get<int32_t>(i.operand1); break;
            case Operation::Pop:            os << "POP"; break;
            case Operation::Return:         os << "RET"; break;
            case Operation::ReturnNil:      os << "RETNIL"; break;
            case Operation::SetLocal:       os << "SETLOCAL " << std::get<int32_t>(i.operand1); break;
            case Operation::GetLocal:       os << "GETLOCAL " << std::get<int32_t>(i.operand1); break;
            case Operation::Call:           os << "CALL " << std::get<int32_t>(i.operand1); break;
            case Operation::BranchFalse:    os << "BRANCH_F &" << std::format("{:03x}", (size_t) std::get<int32_t>(i.operand1)); break;
            default:                        os << "???"; break;
        }
        return os;
    }
};

}

#endif //TYCHE_INSTRUCTION_HH
