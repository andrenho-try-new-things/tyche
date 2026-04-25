#include "bytecode.hh"

using namespace std::string_literals;

namespace vm {

#ifdef DIRECT_IR_ACCESS

Bytecode Bytecode::create_from_ir(compiler::IR const& ir, bool add_debugging_info)
{
    Bytecode bytecode;
    bytecode.ir_ = ir;
    bytecode.has_debugging_info_ = add_debugging_info;
    return bytecode;
}

std::optional<NextInstruction> Bytecode::next_instruction(Location const& location) const
{
    if (location.function_id >= ir_.functions.size() || location.pc >= ir_.functions.at(location.function_id).instructions.size())
        return {};
    return NextInstruction { ir_.functions.at(location.function_id).instructions.at(location.pc), 1 };
}

size_t Bytecode::n_functions() const
{
    return ir_.functions.size();
}

std::string Bytecode::debug_variable_name(FunctionId f_id, size_t var_idx) const
{
    if (has_debugging_info_) {
        return ir_.functions.at(f_id).local_vars.at(var_idx).name;
    } else {
        return "#"s + std::to_string(f_id) + ":" + std::to_string(var_idx);
    }
}


#else

// TODO - implement bytecode direct data access

#endif

std::ostream& operator<<(std::ostream& os, Bytecode const& b)
{

    for (size_t f_id = 0; f_id < b.n_functions(); ++f_id) {
        os << ".function " << f_id << " (pars: " << b.n_function_parameters(f_id) << ", local vars: " << b.n_local_vars(f_id) << ")\n";
        size_t pc = 0;

        while (auto next = b.next_instruction({f_id, pc })) {
            if (!next)
                break;
            os << "\t" << next->instruction << "\n";
            pc += next->size;
        }
    }

    return os;
}

size_t Bytecode::n_local_vars(FunctionId f_id) const
{
    return ir_.functions.at(f_id).local_vars.size();
}

size_t Bytecode::n_function_parameters(FunctionId f_id) const
{
    return ir_.functions.at(f_id).n_parameters;
}

}