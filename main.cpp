#include <cstdio>
#include <peglib.h>
#include "parser.h"
#include "compiler.h"
#include "utils.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Handle arguments
    int c;
    char *fileName = nullptr;
    int printAst = 0, printIR = 0, emitObj = 0, emitLL = 0;
    while ((c = getopt(argc, argv, "afoil:")) != -1) {
        switch (c) {
            case 'a':
                printAst = 1;
                break;
            case 'f':
                fileName = optarg;
                break;
            case 'o':
                emitObj = 1;
                break;
            case 'l':
                emitLL = 1;
                break;
            case 'i':
                printIR = 1;
                break;
            case '?':
                if (optopt == 'f') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                }
                return 1;
            default:
                abort();
        }
    }
    // Declare some things we'll use later
    char *expr;
    parser::Parser parser;
    std::shared_ptr<peg::Ast> ast;
    if (fileName) {
        // If there's a user-supplied filename, read the whole thing into memory and parse into an AST
        expr = utils::readFile(fileName);
        parser.parseExpr(ast, expr);
        free(expr);
    } else {
        // Set expr to the first non-option argument, assumes an expression is supplied
        expr = argv[optind];
        parser.parseExpr(ast, expr);
    }
    compiler::Compiler compiler;
    if(printAst) {
        std::cout << "AST:" << std::endl << peg::ast_to_s(ast) << std::endl;
    }
    llvm::APInt result = compiler.compile(ast, /*printIR=*/printIR);
    llvm::outs() << "RESULT: " << result << "\n";
    int retVal = 0;
    if (emitObj) {
        retVal = compiler.emitObjectCode();
    }
    if (emitLL) {
        compiler.emitLL();
    }
    return retVal;
}

