#include "bytecode.hh"

namespace vm {

#ifdef DIRECT_IR_ACCESS

Bytecode Bytecode::create_from_ir(compiler::IR const& ir)
{
    Bytecode bytecode;
    bytecode.ir_ = ir;
    return bytecode;
}

NextInstruction Bytecode::next_instruction(Location const& location) const
{
    return { ir_.functions[location.function_id].instructions[location.pc], 1 };
}

#else

// TODO - implement bytecode direct data access

#endif

}