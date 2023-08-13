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

        llvm::Value *compile(std::shared_ptr<parser::AstArithExpr> ast) {
            llvm::Value *result = nullptr;
            if(ast->name == "LITERAL") {
                result = handleLiteral(ast);
            } else if (ast->name == "NUMBER") {
                //NUMBER is syntactic sugar for LITERAL | IDENT, we don't emit anything
                result = compile(ast->nodes[0]);
            } else if (ast->name == "EXPRESSION") {
                result = handleExpr(ast);
            } else if (ast->name == "ATOM") {
                //Syntactic sugar, see above
                result = compile(ast->nodes[0]);
            } else if (ast->name == "IDENT"){
                result = handleIdent(ast);
            } else if (ast->name == "LET") {
                result = handleLet(ast);
            }
            return result;
        }

        llvm::Value *handleLiteral(std::shared_ptr<parser::AstArithExpr> ast) {
            return llvm::ConstantInt::get(
                    *context,
                    llvm::APInt(32, std::stoi(ast->token))
            );
        }

        llvm::Value *handleExpr(std::shared_ptr<parser::AstArithExpr> ast) {
            if (ast->nodes.size() == 3){
                // Matches a binary expression
                auto *lhs = compile(ast->nodes[0]);
                auto *rhs = compile(ast->nodes[2]);
                switch(ast->nodes[1]->token[0]) {
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

        llvm::Value* handleLet(std::shared_ptr<parser::AstArithExpr> ast) {
            ast->ctx = ast->ctx->getEnclosingCtx(ast);
            // ast->ctx->initCtx(ast->ctx->getEnclosingCtx(ast)->ctx);
            for (std::shared_ptr<parser::AstArithExpr> node: ast->nodes) {
                if (node->name == "BINDING") {
                    std::pair<std::string, llvm::Value*> bindingResult = handleBinding(node);
                    ast->ctx->ctx.insert({bindingResult.first, bindingResult.second});
                } else {
                    return compile(node);
                }
            }
        }

        std::pair<std::string, llvm::Value*> handleBinding(std::shared_ptr<parser::AstArithExpr> ast) {
            return std::make_pair(ast->nodes[0]->token, compile(ast->nodes[1]));
        }

        // Note: We're only handling IDENTs used as variables here.
        // IDENTs present in assignments are handled (without calling this method)
        // in handleBinding
        llvm::Value* handleIdent(std::shared_ptr<parser::AstArithExpr> ast){
            parser::Context *ctx = ast->ctx->getEnclosingCtx(ast);
            return ctx->getValue(ast->token);
        }
    };
}; // namespace compiler