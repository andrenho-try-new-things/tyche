#ifndef TYCHE_VALUE_HH
#define TYCHE_VALUE_HH

#include <cstdint>
#include <string>
#include <variant>

namespace vm {

using Nil = std::monostate;

using Value = std::variant<Nil, int32_t>;

}

namespace std { string to_string(vm::Value const& value); }


#endif //TYCHE_VALUE_HH
