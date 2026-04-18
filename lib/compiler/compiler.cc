#include "compiler.hh"

#include <format>

#include "lexer.hh"
#include "parser.hh"
#include "linker.hh"
#include "exceptions.hh"

namespace compiler {

vm::Bytecode compile(std::string const& source, bool add_debugging_info)
{
    try {
        auto tokens = tokenize(source);
        auto ir1 = parse(tokens);
        auto ir2 = link({ ir1 });
        auto bytecode = vm::Bytecode::create_from_ir(ir2, add_debugging_info);
        return bytecode;
    } catch (CompilationError& e) {
        throw std::runtime_error(std::format("Compilation error in line {}, column {}: {}", e.line, e.column, e.what()));
    }
}

}
