#ifndef TYCHE_BASEVM_HH
#define TYCHE_BASEVM_HH

#include <vector>

#include "value.hh"

namespace vm {

class BaseVM {
public:
    [[nodiscard]] std::vector<Value> const& stack() const { return stack_; }

    // stack operations
    void push_nil();
    void push_integer(int32_t value);

    void pop();

protected:
    std::vector<Value> stack_;
};

}

#endif //TYCHE_BASEVM_HH
