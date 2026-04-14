#include "lexer.hh"
#include "exceptions.hh"

static const std::string COMBINED_SYMBOLS_L2[] = { "@=" };
static const std::string SYMBOLS = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
static const std::string IGNORE = "\n \t";

namespace compiler {

std::vector<Token> tokenize(std::string const& source)
{
    std::vector<Token> tokens;

    size_t line = 1;
    size_t column = 1;

    for (size_t i = 0; i < source.size(); ++i) {
        size_t start = i;

        if (isdigit(source.at(i))) {
            while (i < source.size() && isdigit(source.at(i)))
                ++i;
            tokens.emplace_back(Integer(std::atoi(source.substr(start, i - start).c_str())), line, column);
            column += i;
            --i;
        }

        else if (SYMBOLS.contains(source.at(i))) {
            tokens.emplace_back(Symbol(std::string(1, source.at(i))), line, column);
            ++column;
        }

        else if (source.at(i) == '\n') {
            ++line;
            column = 1;
        }
    }

    tokens.emplace_back(EOF_(), line, column);
    return tokens;
}

}