#include "vm.hh"
#include "exception.hh"

#include <iostream>
#include <ranges>

using namespace std::string_view_literals;

namespace vm {

void VM::load(vm::Bytecode&& bytecode)
{
    bytecode_ = std::move(bytecode);
}

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
            exit_function();
            return function_.empty();
        case Operation::ReturnNil:
            push_nil();
            exit_function();
            return function_.empty();
        case Operation::SetLocal:
            function_.top().vars.at(std::get<int32_t>(next->instruction.operand1)) = pop_value();
            break;
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

    std::cout << next->instruction << "  ";
    if (bytecode_.has_debugging_info()) {
        if (next->instruction.operation == Operation::SetLocal) {
            std::cout << "{" << bytecode_.debug_variable_name(function_.top().id, std::get<int32_t>(next->instruction.operand1))
                      << "=" << function_.top().vars.at(std::get<int32_t>(next->instruction.operand1)) << "}";
        }
    }
    std::cout << "\n";
    std::cout << "  -> " << debug_stack() << "\n";

    return b;
}

void VM::run()
{
    enter_function(0);
    while (!step());
}

void VM::run_debug()
{
    enter_function(0);
    while (!step_debug());
}

void VM::enter_function(FunctionId f_id)
{
    Function f = { .id = f_id, .vars = {} };
    f.vars.resize(bytecode_.n_local_vars(f_id), vm::Value());
    function_.emplace(std::move(f));
}

void VM::exit_function()
{
    function_.pop();
}

}