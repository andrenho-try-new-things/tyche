#include "basevm.hh"

namespace vm {

void BaseVM::push_nil()
{
    stack_.emplace_back(std::monostate());
}

void BaseVM::push_integer(int32_t value)
{
    stack_.emplace_back(value);
}

void BaseVM::pop()
{
    stack_.pop_back();
}

std::string BaseVM::debug_stack() const
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

Value BaseVM::pop_value()
{
    Value v = std::move(stack_.back());
    stack_.pop_back();
    return v;
}

}