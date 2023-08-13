#include <iostream>
#include <peglib.h>
#include <llvm/IR/IRBuilder.h>
#include "parser.h"
#include "compiler.h"

int main() {
    parser::Parser parser;
    std::shared_ptr<parser::AstArithExpr> ast;
    const char* expr = "3 * (2 + 3)";
    parser.parseExpr(ast, expr);
    auto nodes = ast->nodes;
    auto lastNode = *nodes[0];
    std::cout << "AST:" << std::endl << peg::ast_to_s(ast) << std::endl; //print(peg::ast_to_s(ast));
    compiler::Compiler compiler;
    std::cerr << "Results:" << std::endl;
    compiler.compile(ast)->print(llvm::errs());
    std::cerr << std::endl << "Errors:" << std::endl;
    compiler.getModule()->print(llvm::errs(), nullptr);
    return 0;
}

