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
    bool operator!=(Symbol const& other) const { return !(*this == other); }

    friend std::ostream& operator<<(std::ostream& os, const Symbol& t) {
        return os << "Symbol {" << t.symbol << "}";
    }
};

struct Integer {
    int32_t value;

    explicit Integer(int32_t value_) : value(value_) {}
    bool operator==(Integer const& other) const { return value == other.value; }
    bool operator!=(Integer const& other) const { return !(*this == other); }

    friend std::ostream& operator<<(std::ostream& os, const Integer& t) {
        return os << "Integer {" << t.value << "}";
    }
};

struct Float {
    float value;

    explicit Float(float value_) : value(value_) {}
    bool operator==(Float const& other) const {
        return std::abs(value - other.value) <= 1e-6f * std::max(std::abs(value), std::abs(other.value));
    }
    bool operator!=(Float const& other) const { return !(*this == other); }

    friend std::ostream& operator<<(std::ostream& os, const Float& t) {
        return os << "Float {" << t.value << "}";
    }
};

struct Identifier {
    std::string identifier;

    explicit Identifier(std::string const& identifier_) : identifier(identifier_) {}
    bool operator==(Identifier const& other) const { return identifier == other.identifier; }
    bool operator!=(Identifier const& other) const { return !(*this == other); }

    friend std::ostream& operator<<(std::ostream& os, const Identifier& t) {
        return os << "Identifier {" << t.identifier << "}";
    }
};

struct String {
    std::string str;

    explicit String(std::string const& str_) : str(str_) {}
    bool operator==(String const& other) const { return str == other.str; }
    bool operator!=(String const& other) const { return !(*this == other); }

    friend std::ostream& operator<<(std::ostream& os, const String& t) {
        return os << "String {" << t.str << "}";
    }
};

struct EOF_ {
    bool operator==(EOF_ const&) const { return true; }
    bool operator!=(EOF_ const& other) const { return false; }

    friend std::ostream& operator<<(std::ostream& os, EOF_ const&) {
        return os << "EOF";
    }
};

struct Token {
    std::variant<Symbol, Integer, Float, Identifier, String, EOF_> token;
    size_t line;
    size_t column;

    bool operator==(Token const& other) const { return token == other.token; }

    [[nodiscard]] bool is_symbol(std::string const& symbol) const { auto const* s = std::get_if<Symbol>(&token); return s && s->symbol == symbol; }
    [[nodiscard]] bool is_identifier(std::string const& identifier) const { auto const* s = std::get_if<Identifier>(&token); return s && s->identifier == identifier; }
    [[nodiscard]] bool is_eof() const { return std::holds_alternative<EOF_>(token); }

    [[nodiscard]] std::optional<std::string> identifier() const { auto const* s = std::get_if<Identifier>(&token); return s ? s->identifier : std::optional<std::string>{}; }
    [[nodiscard]] std::optional<int32_t>     integer() const { auto const* s = std::get_if<Integer>(&token); return s ? s->value : std::optional<int32_t>{}; }

    friend std::ostream& operator<<(std::ostream& os, Token const& t) {
        std::visit([&os](auto const& v) { os << v; }, t.token);
        return os;
    }
};

std::vector<Token> tokenize(const std::string& source);

}

#endif //TYCHE_LEXER_HH
