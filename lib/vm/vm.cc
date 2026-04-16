#include "vm.hh"
#include "exception.hh"

namespace vm {

bool VM::step()
{
    NextInstruction next = bytecode_.next_instruction(loc_);
    switch (next.instruction.operation) {
        case Operation::PushNil:
            push_nil();
            break;
        case Operation::PushInt:
            push_integer(std::get<int32_t>(next.instruction.operand1));
            break;
        case Operation::Pop:
            pop();
            break;
        case Operation::Return:
            return true;  // TODO
        case Operation::ReturnNil:
            push_nil();
            return true;  // TODO
        default:
            throw ExecutionException("Invalid opcode");
    }

    loc_.pc += next.size;
    return false;
}

void VM::run()
{
    while (step());
}

}