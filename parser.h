#pragma once
#include <peglib.h>
#include <llvm/IR/IRBuilder.h>

namespace parser {
    struct Context;

    struct AstAnnotation {
        Context *ctx;
    };

    typedef peg::AstBase<AstAnnotation> AstArithExpr;

    struct Context {
    public:
        std::map<std::string, llvm::Value*> ctx;
        Context(std::map<std::string, llvm::Value*> initCtx) {
            ctx = initCtx;
        }
        Context() {
            std::map<std::string, llvm::Value*> emptyCtx;
            ctx = emptyCtx;
        };
        inline Context *getEnclosingCtx(std::shared_ptr<AstArithExpr> ast) {
            // We only assign contexts only on LET nodes, and the context binds all prior information
            std::shared_ptr<AstArithExpr> tmp = ast->parent.lock();
            while(!tmp->ctx){
                if(!tmp->parent.lock()){
                    Context* emptyCtx;
                    return emptyCtx;
                }
                tmp = tmp->parent.lock();
            }
            return tmp->ctx;
        };
        llvm::Value *getValue(std::string ident) {
            if (ctx.count(ident) != 0) {
                return ctx.find(ident)->second;
            }
            return nullptr;
        };
        llvm::Value *addOrUpdate(std::string ident, llvm::Value *value) {
            ctx.insert({ident, value});
            //return ctx.insert_or_assign(ident, value).first->second;
            return value;
        }
        void initCtx(std::map<std::string, llvm::Value*> otherCtx) {
            auto tmp_ctx = std::map<std::string, llvm::Value*>(otherCtx);
            ctx = tmp_ctx;
        }
    };

    class Parser {
    private:
        const char *grammar = R"(
        EXPRESSION  <- ATOM (OPERATOR ATOM)* {
                         precedence
                           L - +
                           L / *
                       }
        ATOM        <- LET / '(' EXPRESSION ')' / NUMBER
        LET         <- "let" BINDING+ "in" EXPRESSION "end"
        BINDING     <- IDENT "=" EXPRESSION
        NUMBER      <- LITERAL / IDENT
        OPERATOR    <- < [-+/*] >
        LITERAL     <- < '-'? [0-9]+ >
        IDENT       <- < [a-zA-Z]+ >
        %whitespace <- [ \t\r\n]*
    )";
    public:
        peg::parser parser = peg::parser();
         Parser(){
            parser.load_grammar(grammar);
            parser.enable_packrat_parsing();
            parser.enable_ast();
        };

         void parseExpr(std::shared_ptr<AstArithExpr> &ast, const char* expr) const {
             parser.parse(expr, ast);
         }
    };

} // parser
