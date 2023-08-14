#pragma once
#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include "peglib.h"

namespace compiler {
    struct Compiler {
    private:
        std::unique_ptr<llvm::LLVMContext> context;
        std::unique_ptr<llvm::IRBuilder<>> builder;
        std::unique_ptr<llvm::Module> module;
        const uint INT_SIZE = 32;
        const std::string OBJECT_CODE_FILE_NAME = "../objectCodeOut.o";
        const std::string LL_FILE_NAME = "llOut.ll";
        // generate object code and print to file
        int buildObjectCode();
        //print IR to file
        void buildLL();
        // Compile an expression by delegating work to corresponding function based on node type
        llvm::Value *compileAst(std::shared_ptr<peg::Ast> &ast);
        // build a value out of a parsed binary expression (add, subtract, multiply, divide, pow)
        // expects ast.name == BINARYEXPR
        llvm::Value *handleBinaryExpr(std::shared_ptr<peg::Ast> &ast);
        // build a value out of a parsed unary expression (increment, decrement)
        // expects ast.name == UNARYEXPR
        llvm::Value *handleUnaryExpr(std::shared_ptr<peg::Ast> &ast);
        // create an IR constant integer out of a parsed literal
        // expects ast.name == LITERAL
        llvm::Value *handleLiteral(std::shared_ptr<peg::Ast> &ast);
        // Initializes the main function to assert compiled code into
        // Then, calls compileAst to compile the provided AST
        llvm::Function *initMainFunc(std::shared_ptr<peg::Ast> &ast);
    public:
        // Construct a Compiler instance, just initializes the llvm fields
        Compiler();
        // use buildObjectCode to print object code to file
        int emitObjectCode();
        // use buildLL to print IR to file
        void emitLL();
        // Compile a provided AST using , optionally printing generated IR
        // Then, use default ExecutionEngine to execute the generated IR, returning
        // the resulting value
        llvm::APInt compile(std::shared_ptr<peg::Ast> &ast, bool printIR);
    };
}