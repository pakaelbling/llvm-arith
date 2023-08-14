#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include "peglib.h"

#pragma once
namespace compiler {
    struct Compiler {
    private:
        std::unique_ptr<llvm::LLVMContext> context;
        std::unique_ptr<llvm::IRBuilder<>> builder;
        std::unique_ptr<llvm::Module> module;
        const uint INT_SIZE = 32;
        const std::string OBJECTCODEFILENAME = "../objectCodeOut.o";
        const std::string LLFILENAME = "llOut.ll";
        int buildObjectCode();
        void buildLL();
        llvm::Value *compileExpr(std::shared_ptr<peg::Ast> &ast);
        llvm::Value *handleBinaryExpr(std::shared_ptr<peg::Ast> &ast);
        llvm::Value *handleUnaryExpr(std::shared_ptr<peg::Ast> &ast);
        llvm::Value *handleLiteral(std::shared_ptr<peg::Ast> &ast);
        llvm::Function *initMainFunc(std::shared_ptr<peg::Ast> &ast);
    public:
        Compiler() {
            context = std::make_unique<llvm::LLVMContext>();
            module = std::make_unique<llvm::Module>("ModuleId", *context);
            builder = std::make_unique<llvm::IRBuilder<>>(*context);
        }
        int emitObjectCode();
        void emitLL();
        llvm::APInt compile(std::shared_ptr<peg::Ast> &ast, bool printIR);
    };
}