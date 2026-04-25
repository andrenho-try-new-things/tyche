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
            stack_push(std::monostate());
            break;
        case Operation::PushInt:
            stack_push(std::get<int32_t>(next->instruction.operand1));
            break;
        case Operation::PushFunction:
            stack_push(ValueFunction{.id = (size_t) std::get<int32_t>(next->instruction.operand1)});
            break;
        case Operation::Pop:
            stack_pop();
            break;
        case Operation::Return:
            exit_function();
            return function_.empty();
        case Operation::ReturnNil:
            stack_.emplace_back(std::monostate());
            exit_function();
            return function_.empty();
        case Operation::SetLocal:
            function_.top().vars.at(std::get<int32_t>(next->instruction.operand1)) = stack_pop();
            break;
        case Operation::GetLocal:
            stack_push(function_.top().vars.at(std::get<int32_t>(next->instruction.operand1)));
            break;
        case Operation::Call:
            function_call((size_t) std::get<int32_t>(next->instruction.operand1), next->size);
            return false;
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

void VM::function_call(size_t n_params, size_t instruction_size)
{
    // pop parameters
    std::vector<Value> parameters;
    for (size_t i = 0; i < n_params; ++i)
        parameters.push_back(stack_pop());

    // get function object
    Value f = stack_pop();
    auto *function = std::get_if<ValueFunction>(&f);
    if (!function)
        throw ExecutionException("Expected function type.");

    // enter function
    enter_function(function->id, loc_.pc + instruction_size, n_params);

    // pass function parameters as variables
    for (size_t i = 0; i < n_params; ++i)
        function_.top().vars.at(i) = parameters.at(n_params - i - 1);
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

Value VM::stack_pop()
{
    Value v = std::move(stack_.back());
    stack_.pop_back();
    return v;
}

void VM::stack_push(Value const& val)
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