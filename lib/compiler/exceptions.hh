#ifndef TYCHE_COMPILER_EXCEPTIONS_HH
#define TYCHE_COMPILER_EXCEPTIONS_HH

#include <stdexcept>
#include <format>

using namespace std::string_literals;

namespace compiler {

class CompilationError : public std::runtime_error {
public:
    CompilationError(const char* description, size_t line_, size_t column_)
        : std::runtime_error(description), line(line_), column(column_) {}

    const size_t line;
    const size_t column;
};

}


#endif //TYCHE_EXCEPTIONS_HH
