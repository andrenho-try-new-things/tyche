#ifndef TYCHE_LEXER_HH
#define TYCHE_LEXER_HH

#include <string>
#include <variant>
#include <vector>

namespace compiler {

struct Symbol {
    std::string symbol;

    explicit Symbol(std::string const& symbol) : symbol(symbol) {}
    bool operator==(Symbol const& other) const { return symbol == other.symbol; }
};

struct Integer {
    int32_t value;

    explicit Integer(int32_t value) : value(value) {}
    bool operator==(Integer const& other) const { return value == other.value; }
};

using Token = std::variant<Symbol, Integer>;

std::vector<Token> tokenize(const std::string& source);

}

#endif //TYCHE_LEXER_HH
