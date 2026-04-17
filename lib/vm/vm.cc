#include "vm.hh"
#include "exception.hh"

#include <iostream>

namespace vm {

bool VM::step()
{
    auto next = bytecode_.next_instruction(loc_);
    if (!next)
        throw ExecutionException("Out of code area bounds");
    switch (next->instruction.operation) {
        case Operation::PushNil:
            push_nil();
            break;
        case Operation::PushInt:
            push_integer(std::get<int32_t>(next->instruction.operand1));
            break;
        case Operation::Pop:
            pop();
            break;
        case Operation::Return:
            return true;  // TODO - return from function
        case Operation::ReturnNil:
            push_nil();
            return true;  // TODO - return from function
        default:
            throw ExecutionException("Invalid opcode");
    }

    loc_.pc += next->size;
    return false;
}

bool VM::step_debug()
{
    auto next = bytecode_.next_instruction(loc_);
    bool b = step();

    std::cout << next->instruction << "\n";
    std::cout << "  -> " << debug_stack() << "\n";

    return b;
}

void VM::run()
{
    while (!step());
}

void VM::run_debug()
{
    while (!step_debug());
}

}