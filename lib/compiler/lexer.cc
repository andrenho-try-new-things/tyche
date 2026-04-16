#include "lexer.hh"
#include "exceptions.hh"

static const std::string COMBINED_SYMBOLS_L2[] = { "@=", ":=" };
static const std::string SYMBOLS = "!#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
static const std::string IGNORED = " \t";

namespace compiler {

static bool is_letter(char c)
{
    return c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

std::vector<Token> tokenize(std::string const& source)
{
    std::vector<Token> tokens;

    size_t line = 1;
    size_t column = 1;

    for (size_t i = 0; i < source.size(); ++i) {
next_token:
        size_t start = i;

        // Integer

        if (isdigit(source.at(i))) {
            bool is_float = false;
            while (i < source.size() && (isdigit(source.at(i)) || source.at(i) == '.')) {
                if (source.at(i) == '.')
                    is_float = true;
                ++i;
            }
            if (is_float)
                tokens.emplace_back(Float(std::atof(source.substr(start, i - start).c_str())), line, column);
            else
                tokens.emplace_back(Integer(std::atoi(source.substr(start, i - start).c_str())), line, column);
            column += i;
            --i;
        }

        // Symbol

        else if (SYMBOLS.contains(source.at(i))) {
            if (source.length() >= i + 1) {
                std::string next_2 = source.substr(i, 2);
                for (auto const& combined: COMBINED_SYMBOLS_L2) {
                    if (combined == next_2) {
                        tokens.emplace_back(Symbol(combined), line, column);
                        column += 2;
                        i += 2;
                        goto next_token;
                    }
                }
            }
            tokens.emplace_back(Symbol(std::string(1, source.at(i))), line, column);
            ++column;
        }

        // String

        else if (source.at(i) == '"') {
            ++i;
            std::string str;
            while (i < source.size() && source.at(i) != '"') {
                if (source.at(i) == '\\') {
                    if (source.at(i+1) == 'n')
                        str += "\n";
                    else if (source.at(i+1) == '\\')
                        str += "\\";
                    else if (source.at(i+1) == '\"')
                        str += "\"";
                    else
                        str += source.substr(i, 2);
                    i += 2;
                } else {
                    str += source.at(i);
                    ++i;
                }
            }
            if (i >= source.size())
                throw CompilationError("Unterminated string", line, column);
            tokens.emplace_back(String(str), line, column);
            column += i;
        }

        // Identifier

        else if (is_letter(source.at(i))) {
            while (i < source.size() && is_letter(source.at(i)))
                ++i;
            tokens.emplace_back(Identifier(source.substr(start, i - start)), line, column);
            column += i;
            --i;
        }

        // Ignored characters

        else if (IGNORED.contains(source.at(i))) {
            ++column;
        } else if (source.at(i) == '\n') {
            ++line;
            column = 1;
        }

        // other invalid character

        else {
            throw CompilationError(std::string("Invalid character '") + source.at(i) + "'.", line, column);
        }

    }

    tokens.emplace_back(EOF_(), line, column);
    return tokens;
}

}