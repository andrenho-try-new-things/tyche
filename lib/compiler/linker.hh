#ifndef TYCHE_LINKER_HH
#define TYCHE_LINKER_HH

#include <vector>

#include "ir.hh"

namespace compiler {

IR link(std::vector<IR> const& ir);

}

#endif //TYCHE_LINKER_HH
