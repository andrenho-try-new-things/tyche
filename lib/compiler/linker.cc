#include "linker.hh"

#include "../vm/instruction.hh"

compiler::IR compiler::link(std::vector<IR> const& ir)
{
    IR out = ir.at(0);

    // replace labels by integers
    for (auto& function: out.functions) {
        for (auto& instruction : function.instructions) {
            auto *label = std::get_if<vm::Label*>(&instruction.operand1);
            if (label)
                instruction.operand1 = (int32_t) (*label)->instruction_idx;
        }
    }

    return out;
}
