#ifndef TYCHE_EXCEPTION_HH
#define TYCHE_EXCEPTION_HH

#include <stdexcept>
#include <string>

namespace vm {

class ExecutionException : std::runtime_error {
public:
    explicit ExecutionException(std::string const& desc) : std::runtime_error(desc) {}
};

}

#endif //TYCHE_EXCEPTION_HH
