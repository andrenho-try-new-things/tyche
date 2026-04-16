#ifndef TYCHE_VALUE_HH
#define TYCHE_VALUE_HH

#include <cstdint>
#include <variant>

namespace vm {

using Value = std::variant<std::monostate, int32_t>;

}

#endif //TYCHE_VALUE_HH
