#include <llvm/IR/IRBuilder.h>
#include "parser.h"

namespace compiler {
    struct Compiler {
    private:
        std::unique_ptr<llvm::LLVMContext> context;
        std::unique_ptr<llvm::IRBuilder<>> builder;
        std::unique_ptr<llvm::Module> module;
    public:
        Compiler() {
            context = std::make_unique<llvm::LLVMContext>();
            module = std::make_unique<llvm::Module>("ModuleId", *context);
            builder = std::make_unique<llvm::IRBuilder<>>(*context);
        }

        std::unique_ptr<llvm::Module> &getModule() {
            return module;
        }

        llvm::Value *compile(std::shared_ptr<peg::Ast> ast) {
            llvm::Value *result = nullptr;
            if (ast->name == "LITERAL") {
                result = handleLiteral(ast);
            } else if (ast->name == "NUMBER") {
                //NUMBER is syntactic sugar for LITERAL | IDENT, we don't emit anything
                result = compile(ast->nodes[0]);
            } else if (ast->name == "EXPRESSION") {
                result = handleExpr(ast);
            } else if (ast->name == "ATOM") {
                //Syntactic sugar, see above
                result = compile(ast->nodes[0]);
            }
            return result;
        }

        llvm::Value *handleLiteral(std::shared_ptr<peg::Ast> ast) {
            return llvm::ConstantInt::get(
                    *context,
                    llvm::APInt(32, std::stoi(ast->token))
            );
        }

        llvm::Value *handleExpr(std::shared_ptr<peg::Ast> ast) {
            if (ast->nodes.size() == 3) {
                // Matches a binary expression
                auto *lhs = compile(ast->nodes[0]);
                auto *rhs = compile(ast->nodes[2]);
                switch (ast->nodes[1]->token[0]) {
                    case '+':
                        return builder->CreateAdd(lhs, rhs);
                    case '-':
                        return builder->CreateSub(lhs, rhs);
                    case '*':
                        return builder->CreateMul(lhs, rhs);
                    case '/':
                        return builder->CreateSDiv(lhs, rhs);
                    default:
                        return nullptr;
                }
            } else {
                //Matches a top level atom
                return compile(ast->nodes[0]);
            }
        }
    };
} // namespace compiler