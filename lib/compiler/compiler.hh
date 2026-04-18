#ifndef TYCHE_COMPILER_HH
#define TYCHE_COMPILER_HH

#include <string>

#include "../vm/bytecode.hh"

namespace compiler {

vm::Bytecode compile(std::string const& source, bool add_debugging_info=false);

}

#endif //TYCHE_COMPILER_HH
