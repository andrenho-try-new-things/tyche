#ifndef TYCHE_LEXER_HH
#define TYCHE_LEXER_HH

#include <iostream>
#include <string>
#include <variant>
#include <vector>

namespace compiler {

struct Symbol {
    std::string symbol;

    explicit Symbol(std::string const& symbol_) : symbol(symbol_) {}
    bool operator==(Symbol const& other) const { return symbol == other.symbol; }

    friend std::ostream& operator<<(std::ostream& os, const Symbol& t) {
        return os << "Symbol { " << t.symbol << " }";
    }
};

struct Integer {
    int32_t value;

    explicit Integer(int32_t value_) : value(value_) {}
    bool operator==(Integer const& other) const { return value == other.value; }

    friend std::ostream& operator<<(std::ostream& os, const Integer& t) {
        return os << "Integer { " << t.value << " }";
    }
};

struct Identifier {
    std::string identifier;

    explicit Identifier(std::string const& identifier_) : identifier(identifier_) {}
    bool operator==(Identifier const& other) const { return identifier == other.identifier; }

    friend std::ostream& operator<<(std::ostream& os, const Identifier& t) {
        return os << "Identifier { " << t.identifier << " }";
    }
};

struct EOF_ {
    bool operator==(EOF_ const&) const { return true; }

    friend std::ostream& operator<<(std::ostream& os, EOF_ const&) {
        return os << "EOF";
    }
};

struct Token {
    std::variant<Symbol, Integer, Identifier, EOF_> token;
    size_t line;
    size_t column;

    bool operator==(Token const& other) const { return token == other.token; }

    friend std::ostream& operator<<(std::ostream& os, Token const& t) {
        std::visit([&os](auto const& v) { os << v; }, t.token);
        return os;
    }
};

std::vector<Token> tokenize(const std::string& source);

}

#endif //TYCHE_LEXER_HH
