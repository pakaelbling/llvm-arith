#include <peglib.h>
#include "parser.h"

namespace parser {
        Parser::Parser() {
            parser.load_grammar(grammar);
            parser.enable_packrat_parsing();
            parser.enable_ast();
        };
         void Parser::parseExpr(std::shared_ptr<peg::Ast> &ast, const char* expr) const {
             parser.parse(expr, ast);
         }

} // parser
