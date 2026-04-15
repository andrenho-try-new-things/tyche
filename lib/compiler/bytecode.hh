#ifndef TYCHE_BYTECODE_HH
#define TYCHE_BYTECODE_HH

#include <cstdint>
#include <vector>

#include "ir.hh"

#define DIRECT_IR_ACCESS 1

namespace compiler {

struct Location {
    size_t function_id;
    size_t pc;
};

struct NextInstruction {
    Instruction instruction;
    size_t      size;

    NextInstruction(Instruction const& instruction, size_t size) :instruction(instruction), size(size) {}
};

class Bytecode {
public:
    static Bytecode create_from_ir(IR const& ir);

    [[nodiscard]] NextInstruction next_instruction(Location const& location) const;

private:
#ifdef DIRECT_IR_ACCESS
    IR ir_;
#else
    std::vector<uint8_t> data_;
#endif
};

}

#endif //TYCHE_BYTECODE_HH
