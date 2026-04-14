#ifndef TYCHE_COMPILER_EXCEPTIONS_HH
#define TYCHE_COMPILER_EXCEPTIONS_HH

#include <stdexcept>
#include <format>

using namespace std::string_literals;

namespace compiler {

struct LexerInvalidCharacter : public std::runtime_error {
    LexerInvalidCharacter(char c, int line, int column)
        : std::runtime_error(std::format("Invalid character '%c' in line %d, column %d.", c, line, column)) {}
};

}


#endif //TYCHE_EXCEPTIONS_HH
