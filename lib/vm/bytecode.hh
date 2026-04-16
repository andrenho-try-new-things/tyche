#ifndef TYCHE_BYTECODE_HH
#define TYCHE_BYTECODE_HH

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "../compiler/ir.hh"

#define DIRECT_IR_ACCESS 1
  // DIRECT_IR_ACCESS undefined - the bytecode is generated in the byte array. This is the default method.
  // DIRECT_IR_ACCESS 1 - the bytecode is a copy of the IR. This is used temporarily until the final bytecode is built.

namespace vm {

using FunctionId = size_t;

struct Location {
    FunctionId function_id;
    size_t     pc;
};

struct NextInstruction {
    Instruction instruction;
    size_t      size;

    NextInstruction(Instruction const& instruction_, size_t size_) : instruction(instruction_), size(size_) {}
};

class Bytecode {
public:
    static Bytecode create_from_ir(compiler::IR const& ir);

    [[nodiscard]] std::optional<NextInstruction> next_instruction(Location const& location) const;
    [[nodiscard]] size_t n_functions() const;
    [[nodiscard]] size_t n_local_vars(FunctionId f_id) const;

    friend std::ostream& operator<<(std::ostream& os, Bytecode const& b);

private:
#ifdef DIRECT_IR_ACCESS
    compiler::IR ir_;
#else
    std::vector<uint8_t> data_;
#endif
};

}

#endif //TYCHE_BYTECODE_HH
