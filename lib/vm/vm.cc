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
            push_value(std::monostate());
            break;
        case Operation::PushInt:
            push_value(std::get<int32_t>(next->instruction.operand1));
            break;
        case Operation::PushFunction:
            push_value(ValueFunction { .id = (size_t) std::get<int32_t>(next->instruction.operand1) });
            break;
        case Operation::Pop:
            pop_value();
            break;
        case Operation::Return:
            exit_function();
            return function_.empty();
        case Operation::ReturnNil:
            stack_.emplace_back(std::monostate());
            exit_function();
            return function_.empty();
        case Operation::SetLocal:
            function_.top().vars.at(std::get<int32_t>(next->instruction.operand1)) = pop_value();
            break;
        case Operation::GetLocal:
            push_value(function_.top().vars.at(std::get<int32_t>(next->instruction.operand1)));
            break;
        case Operation::Call: {
            Value v = pop_value();
            if (auto* f = std::get_if<ValueFunction>(&v); f) {
                enter_function(f->id, loc_.pc + next->size, (size_t) std::get<int32_t>(next->instruction.operand1));
                return false;
            } else {
                throw ExecutionException("Expected function type.");
            }
            break;
        }
        default:
            throw ExecutionException("Invalid opcode");
    }

    loc_.pc += next->size;
    return false;
}

bool VM::step_debug()
{
    Location loc = loc_;
    auto next = bytecode_.next_instruction(loc);
    bool b = step();

    std::cout << "(" << loc.function_id << ":" << loc.pc << ") " << next->instruction << "  ";
    if (bytecode_.has_debugging_info()) {
        if (next->instruction.operation == Operation::SetLocal) {
            std::cout << "; " << bytecode_.debug_variable_name(function_.top().id, std::get<int32_t>(next->instruction.operand1))
                      << "=" << function_.top().vars.at(std::get<int32_t>(next->instruction.operand1));
        } else if (next->instruction.operation == Operation::GetLocal) {
            std::cout << "; " << bytecode_.debug_variable_name(function_.top().id, std::get<int32_t>(next->instruction.operand1));
        }
    }
    std::cout << "\n";
    std::cout << "  -> " << debug_stack() << "\n";

    return b;
}

void VM::run()
{
    enter_function(0, 0, 0);
    while (!step());
}

void VM::run_debug()
{
    enter_function(0, 0, 0);
    while (!step_debug());
}

void VM::enter_function(FunctionId f_id, size_t return_pc, size_t n_pars)
{
    Function f = {
        .id = f_id,
        .vars = {},
        .return_loc = function_.empty() ? (Location) { 0, 0 } : (Location) { function_.top().id, return_pc }
    };
    f.vars.resize(bytecode_.n_local_vars(f_id), vm::Value());
    function_.emplace(std::move(f));
    loc_ = { function_.top().id, 0 };
}

void VM::exit_function()
{
    loc_ = function_.top().return_loc;
    function_.pop();
}

Value VM::pop_value()
{
    Value v = std::move(stack_.back());
    stack_.pop_back();
    return v;
}

void VM::push_value(Value const& val)
{
    stack_.emplace_back(val);
}

std::string VM::debug_stack() const
{
    if (stack_.empty()) {
        return "empty";
    } else {
        std::string out;
        for (auto const& item: stack_)
            out += "[" + std::to_string(item) + "] ";
        return out;
    }
}

}