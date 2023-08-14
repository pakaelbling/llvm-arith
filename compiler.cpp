#include "compiler.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>

namespace compiler {
        int Compiler::buildObjectCode() {
            std::string targetTriple = llvm::sys::getDefaultTargetTriple();
            std::string err;
            const llvm::Target *target = llvm::TargetRegistry::lookupTarget(targetTriple, err);
            if(!target){
                llvm::errs() << err;
            }
            const std::string cpu = "generic";
            const std::string features;
            llvm::TargetOptions llvmOpts;
            auto RM = std::optional<llvm::Reloc::Model>();
            llvm::TargetMachine *targetMachine = target->createTargetMachine(targetTriple, cpu, features, llvmOpts, RM);
            module->setDataLayout(targetMachine->createDataLayout());
            std::error_code errorCode;
            llvm::raw_fd_ostream dest(OBJECTCODEFILENAME, errorCode, llvm::sys::fs::OF_None);
            if (errorCode) {
                llvm::errs() << "Couldn't open file: " << errorCode.message();
                return 1;
            }
            llvm::legacy::PassManager pass;

            llvm::CodeGenFileType fileType = llvm::CGFT_ObjectFile;
            if(targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)){
                llvm::errs() << "Target Machine can't emit a file of this type";
                return 1;
            }
            pass.run(*module);
            dest.flush();
            llvm::outs() << "Wrote " << OBJECTCODEFILENAME << "\n";
            return 0;
        }

        void Compiler::buildLL() {
            std::error_code errorCode;
            llvm::raw_fd_ostream dest(LLFILENAME, errorCode, llvm::sys::fs::OF_None);
            module->print(dest, nullptr);
        }

        llvm::Value *Compiler::compileExpr(std::shared_ptr<peg::Ast> &ast) {
            llvm::Value *result = nullptr;
            if (ast->name == "EXPRESSION" || ast->name == "ATOM") {
                // These node types are syntactic sugar for unions, so we don't need to do any work on them
                // Instead we compileExpr their child
                result = compileExpr(ast->nodes[0]);
            } else if (ast->name == "BINARYEXPR") {
                result = handleBinaryExpr(ast);
            } else if (ast->name == "UNARYEXPR") {
                result = handleUnaryExpr(ast);
            } else if (ast->name == "LITERAL") {
                result = handleLiteral(ast);
            }
            return result;
        }

        llvm::Value *Compiler::handleBinaryExpr(std::shared_ptr<peg::Ast> &ast) {
            if (ast->nodes.size() == 3) {
                // Matches a binary expression
                auto *lhs = compileExpr(ast->nodes[0]);
                auto *rhs = compileExpr(ast->nodes[2]);
                // The second node is a BINOP, we can just access the token directly
                // instead of doing more recursion
                switch (ast->nodes[1]->token[0]) {
                    case '+':
                        return builder->CreateAdd(lhs, rhs);
                    case '-':
                        return builder->CreateSub(lhs, rhs);
                    case '*':
                        return builder->CreateMul(lhs, rhs);
                    case '/':
                        return builder->CreateSDiv(lhs, rhs);
                    case '^': {
                        llvm::Value *floatLhs = builder->CreateSIToFP(lhs, llvm::Type::getFloatTy(*context));
                        llvm::Value *floatPow = builder->CreateIntrinsic(
                                llvm::Type::getFloatTy(*context),
                                llvm::Intrinsic::powi,
                                {floatLhs, rhs});
                        return builder->CreateFPToSI(floatPow, llvm::Type::getIntNTy(*context, INT_SIZE));
                    }
                    default:
                        // This is unreachable; a symbol isn't matched iff it's not in [+-*/], which would throw
                        // an error during parsing
                        return nullptr;
                }
            } else {
                // Matches an atom
                return compileExpr(ast->nodes[0]);
            }
        }

        llvm::Value *Compiler::handleUnaryExpr(std::shared_ptr<peg::Ast> &ast) {
            auto *lhs = compileExpr(ast->nodes[0]);
            llvm::Value *ONE = llvm::ConstantInt::get(*context, llvm::APInt(INT_SIZE, 1));
            if (ast->nodes[1]->token == "++") {
                return builder->CreateAdd(lhs, ONE);
            } else {
                // If we're not incrementing, we're decrementing
                // We don't need to worry about throwing errors, as these two branches cover all possibilities
                // for the current set of unary operators
                return builder->CreateSub(lhs, ONE);
            }
        }

        llvm::Value *Compiler::handleLiteral(std::shared_ptr<peg::Ast> &ast) {
            return llvm::ConstantInt::get(
                    *context,
                    llvm::APInt(INT_SIZE, std::stoi(ast->token))
            );
        }

        llvm::Function *Compiler::initMainFunc(std::shared_ptr<peg::Ast> &ast){
            llvm::FunctionType *funcType = llvm::FunctionType::get(
                    llvm::Type::getIntNTy(*context,INT_SIZE),
                    {},
                    false);
            llvm::Function *main = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", *module);
            llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(*context, "entry", main);
            builder->SetInsertPoint(basicBlock);
            llvm::Value *retVal = compileExpr(ast);
            builder->CreateRet(retVal);
            llvm::verifyFunction(*main);
            return main;
        }

        int Compiler::emitObjectCode(){
            return buildObjectCode();
        }

        void Compiler::emitLL() {
            buildLL();
        }

        llvm::APInt Compiler::compile(std::shared_ptr<peg::Ast> &ast, bool printIR) {
            llvm::InitializeNativeTarget();
            llvm::InitializeNativeTargetAsmParser();
            llvm::InitializeNativeTargetAsmPrinter();
            llvm::Function *main = initMainFunc(ast);
            if (printIR) {
                std::cout << "GENERATED IR:" << "\n";
                main->print(llvm::outs());
            }
            std::string err;
            llvm::ExecutionEngine *execEngine = llvm::EngineBuilder(std::move(module))
                    .setErrorStr(&err)
                    .create();
            if (!execEngine){
                llvm::errs() << "Failed constructing ExecutionEngine: " << err;
            }
            llvm::GenericValue result = execEngine->runFunction(main, {});
            return result.IntVal;
        }
} // namespace compiler