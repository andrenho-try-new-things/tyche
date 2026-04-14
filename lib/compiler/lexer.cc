#include "lexer.hh"
#include "exceptions.hh"

static const std::string SYMBOLS = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
static const std::string IGNORE = "\n \t";

namespace compiler {

std::vector<Token> tokenize(std::string const& source)
{
    enum class State { NOTHING, SYMBOL, INTEGER, IDENTIFIER };

    std::vector<Token> tokens;
    State current_state = State::NOTHING;
    std::string current_token;
    int line = 1;
    int column = 1;

    State new_state = State::NOTHING;

    const char* src = source.data();
    while (*src) {
        char lookahead = *src++;

        if (isalpha(lookahead))
            new_state = State::IDENTIFIER;
        else if (isdigit(lookahead))
            new_state = State::INTEGER;
        else if (SYMBOLS.contains(lookahead))
            new_state = State::SYMBOL;
        else if (!IGNORE.contains(lookahead))
            throw LexerInvalidCharacter(lookahead, line, column);

        if (new_state != current_state) {
            switch (current_state) {
                case State::NOTHING:
                    break;
                case State::SYMBOL:
                    tokens.emplace_back(Symbol(current_token));
                    break;
                case State::INTEGER:
                    tokens.emplace_back(Integer(std::atoi(current_token.c_str())));
                    break;
                case State::IDENTIFIER:
                    tokens.emplace_back(Identifier(current_token));
                    break;
            }
            current_state = new_state;
        }

        current_token += lookahead;
        src++;
    }

    tokens.emplace_back(EOF_ {});
    return tokens;
}

}