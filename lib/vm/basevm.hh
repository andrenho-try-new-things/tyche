#ifndef TYCHE_BASEVM_HH
#define TYCHE_BASEVM_HH

#include <stack>
#include <string>
#include <vector>

#include "value.hh"

namespace vm {

class BaseVM {
public:
    // stack operations
    void push_nil();
    void push_integer(int32_t value);

    void pop();

    [[nodiscard]] std::vector<Value> const& stack() const { return stack_; }
    [[nodiscard]] std::string debug_stack() const;

protected:
    std::vector<Value> stack_;

    Value pop_value();
    void  push_value(Value const& val);
};

}

#endif //TYCHE_BASEVM_HH
