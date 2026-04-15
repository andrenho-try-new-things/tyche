#include "bytecode.hh"

namespace compiler {

#ifdef DIRECT_IR_ACCESS

Bytecode Bytecode::create_from_ir(IR const& ir)
{
    Bytecode bytecode;
    bytecode.ir_ = ir;
    return bytecode;
}

NextInstruction Bytecode::next_instruction(Location const& location) const
{
    return { ir_.functions[location.function_id].instructions[location.pc], 1 };
}

#endif

}