#ifndef TYCHE_VALUE_HH
#define TYCHE_VALUE_HH

#include <cstdint>
#include <string>
#include <variant>

namespace vm {

using Nil = std::monostate;

using Value = std::variant<Nil, int32_t>;

}

// debugging

namespace std { string to_string(vm::Value const& value); }

inline std::ostream& operator<<(std::ostream& os, vm::Value const& value) {
    os << std::to_string(value);
    return os;
}

#endif //TYCHE_VALUE_HH
