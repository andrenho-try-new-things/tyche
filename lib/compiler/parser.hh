#ifndef TYCHE_PARSER_HH
#define TYCHE_PARSER_HH

#include "lexer.hh"
#include "ir.hh"

namespace compiler {

IR parse(std::vector<Token> const& tokens);

}

#endif //TYCHE_PARSER_HH
