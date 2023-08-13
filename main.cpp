#include <iostream>
#include <peglib.h>
#include <llvm/IR/IRBuilder.h>
#include "parser.h"
#include "compiler.h"
#include "utils.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
    parser::Parser parser;
    std::shared_ptr<peg::Ast> ast;
    int c;
    char *fileName = nullptr;
    int printAst = 0;
    while ((c = getopt (argc, argv, "af:")) != -1)
        switch (c)
        {
            case 'a':
                printAst = 1;
                break;
            case 'f':
                fileName = optarg;
                break;
            case '?':
                if (optopt == 'f')
                    fprintf (stderr, "Option -%f requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr,
                             "Unknown option character `\\x%x'.\n",
                             optopt);
                return 1;
            default:
                abort ();
        }
    char *expr;
    if (fileName){
        expr = utils::readFile(fileName);
        parser.parseExpr(ast, expr);
        free(expr);
    } else {
        expr = argv[optind];
        parser.parseExpr(ast, expr);
    }
    if(printAst){
        std::cout << "AST:" << std::endl << peg::ast_to_s(ast) << std::endl;
    }
    compiler::Compiler compiler;
    std::cerr << "Results:" << std::endl;
    compiler.compile(ast)->print(llvm::errs());
    std::cerr << std::endl << "Errors:" << std::endl;
    compiler.getModule()->print(llvm::errs(), nullptr);
    return 0;
}

