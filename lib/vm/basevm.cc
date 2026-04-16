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

}