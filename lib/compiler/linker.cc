#include "linker.hh"

#include "../vm/instruction.hh"
#include "exceptions.hh"

compiler::IR compiler::link(std::vector<IR> const& ir)
{
    IR out = ir.at(0);

    // replace labels by integers
    for (auto& function: out.functions) {
        for (auto& instruction : function.instructions) {
            auto *key = std::get_if<UnresolvedKey>(&instruction.operand1);
            if (key) {
                auto it = function.unresolved_map.find(*key);
                if (it == function.unresolved_map.end() || !it->second.has_value())
                    throw LinkerError(std::string("Key '") + it->first.str() + "' could not be resolved.");
                instruction.operand1 = *it->second;
            }
        }
    }

    return out;
}
