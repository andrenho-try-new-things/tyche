#include "bytecode.hh"

namespace vm {

#ifdef DIRECT_IR_ACCESS

Bytecode Bytecode::create_from_ir(compiler::IR const& ir)
{
    Bytecode bytecode;
    bytecode.ir_ = ir;
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

#else

// TODO - implement bytecode direct data access

#endif

std::ostream& operator<<(std::ostream& os, Bytecode const& b)
{

    for (size_t i = 0; i < b.n_functions(); ++i) {
        os << ".function " << i << "\n";
        size_t pc = 0;

        while (auto next = b.next_instruction({ i, pc })) {
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

}