#include "value.hh"

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };

std::string std::to_string(vm::Value const& value)
{
    return std::visit(overloaded{
        [](std::monostate) { return std::string("nil"); },
        [](auto const& arg) { return std::to_string(arg); }
    }, value);
}
