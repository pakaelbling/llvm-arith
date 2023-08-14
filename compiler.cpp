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
        // Generate objectCode and print to file
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
            llvm::raw_fd_ostream dest(OBJECT_CODE_FILE_NAME, errorCode, llvm::sys::fs::OF_None);
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
            llvm::outs() << "Wrote " << OBJECT_CODE_FILE_NAME << "\n";
            return 0;
        }

        void Compiler::buildLL() {
            std::error_code errorCode;
            llvm::raw_fd_ostream dest(LL_FILE_NAME, errorCode, llvm::sys::fs::OF_None);
            module->print(dest, nullptr);
        }

        llvm::Value *Compiler::compileAst(std::shared_ptr<peg::Ast> &ast) {
            llvm::Value *result = nullptr;
            if (ast->name == "EXPRESSION" || ast->name == "ATOM") {
                // These node types are syntactic sugar for unions, so we don't need to do any work on them
                // Instead we compileAst their child
                result = compileAst(ast->nodes[0]);
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
                auto *lhs = compileAst(ast->nodes[0]);
                auto *rhs = compileAst(ast->nodes[2]);
                // The second node is a BINOP, we can just access the token directly
                // instead of doing more recursion
                // For most operators we just call the corresponding IRBuilder method
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
                        // The llvm intrinsic works only on llvm floats, so we need to:
                        // cast the first arg to a llvm float, call the intrinsic, cast the result back to an llvm int
                        llvm::Type *llvmFloatType = llvm::Type::getFloatTy(*context);
                        llvm::Value *floatLhs = builder->CreateSIToFP(lhs, llvmFloatType);
                        llvm::Value *floatPow = builder->CreateIntrinsic(
                                llvmFloatType,
                                llvm::Intrinsic::powi,
                                {floatLhs, rhs});
                        return builder->CreateFPToSI(floatPow, llvm::Type::getIntNTy(*context, INT_SIZE));
                    }
                    default:
                        // This is unreachable; a symbol isn't matched iff it's not in [+-*/], which would throw
                        // an error during parsing
                        std::cerr << "Failed compiling a binary expression with operator " << ast->nodes[1]->token
                            << "\n" << "AST is " << peg::ast_to_s(ast) << "\n";
                        return nullptr;
                }
            } else {
                // Matches an atom
                return compileAst(ast->nodes[0]);
            }
        }

        llvm::Value *Compiler::handleUnaryExpr(std::shared_ptr<peg::Ast> &ast) {
            auto *lhs = compileAst(ast->nodes[0]);
            llvm::Value *ONE = llvm::ConstantInt::get(*context, llvm::APInt(INT_SIZE, 1));
            if (ast->nodes[1]->token == "++") {
                return builder->CreateAdd(lhs, ONE);
            } else if (ast->nodes[1]->token == "--") {
                return builder->CreateSub(lhs, ONE);
            } else {
                // This is unreachable; a symbol isn't matched iff it's not ++ or --, which would throw
                // an error during parsing
                std::cerr << "Failed compiling a unary expression with operator " << ast->nodes[1]->token
                    << "\n" << "AST is " << peg::ast_to_s(ast) << "\n";
                return nullptr;
            }
        }

        llvm::Value *Compiler::handleLiteral(std::shared_ptr<peg::Ast> &ast) {
            // Create an IR constant int with the token of the LITERAL node
            return llvm::ConstantInt::get(
                    *context,
                    llvm::APInt(INT_SIZE, std::stoi(ast->token))
            );
        }

        llvm::Function *Compiler::initMainFunc(std::shared_ptr<peg::Ast> &ast){
            // constructs type main:() -> Int
            llvm::FunctionType *funcType = llvm::FunctionType::get(
                    llvm::Type::getIntNTy(*context,INT_SIZE),
                    {},
                    false);
            // Create the main function and a basic block into which the body will be inserted
            llvm::Function *main = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", *module);
            llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(*context, "entry", main);
            builder->SetInsertPoint(basicBlock);
            // Compile the provided AST
            llvm::Value *retVal = compileAst(ast);
            builder->CreateRet(retVal);
            llvm::verifyFunction(*main);
            return main;
        }

        Compiler::Compiler()  {
            context = std::make_unique<llvm::LLVMContext>();
            module = std::make_unique<llvm::Module>("ArithComp", *context);
            builder = std::make_unique<llvm::IRBuilder<>>(*context);
        }

        int Compiler::emitObjectCode(){
            return buildObjectCode();
        }

        void Compiler::emitLL() {
            buildLL();
        }

        llvm::APInt Compiler::compile(std::shared_ptr<peg::Ast> &ast, bool printIR) {
            // Initialize a bunch of LLVM stuff, build main func
            llvm::InitializeNativeTarget();
            llvm::InitializeNativeTargetAsmParser();
            llvm::InitializeNativeTargetAsmPrinter();
            llvm::Function *main = initMainFunc(ast);
            if (printIR) {
                std::cout << "GENERATED IR:" << "\n";
                main->print(llvm::outs());
            }
            std::string err;
            // Run main and return the value
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