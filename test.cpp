#include "compiler.h"
#include "parser.h"
#include <gtest/gtest.h>

class CompilerTestFixture: public ::testing::Test {
public:
    parser::Parser parser;
    std::shared_ptr<peg::Ast> ast;
    compiler::Compiler compiler;
};

TEST_F(CompilerTestFixture, TopLevelAtom) {
    parser.parseExpr(ast, "1");
    int result = (int) compiler.compile(ast, false).roundToDouble(true);
    EXPECT_EQ(result, 1);
}

TEST_F(CompilerTestFixture, TopLevelNegativeAtom) {
    parser.parseExpr(ast, "-1");
    int result = (int) compiler.compile(ast, false).roundToDouble(true);
    EXPECT_EQ(result, -1);
}

TEST_F(CompilerTestFixture, IncrExpr) {
    parser.parseExpr(ast, "1++");
    int result = (int) compiler.compile(ast, false).roundToDouble(true);
    EXPECT_EQ(result, 2);
}

TEST_F(CompilerTestFixture, DecrExpr) {
    parser.parseExpr(ast, "1--");
    int result = (int) compiler.compile(ast, false).roundToDouble(true);
    EXPECT_EQ(result, 0);
}

TEST_F(CompilerTestFixture, AddExpr) {
    parser.parseExpr(ast, "1 + 2");
    int result = (int) compiler.compile(ast, false).roundToDouble(true);
    EXPECT_EQ(result, 3);
}

TEST_F(CompilerTestFixture, SubExpr) {
    parser.parseExpr(ast, "1 - 2");
    int result = (int) compiler.compile(ast, false).roundToDouble(true);
    EXPECT_EQ(result, -1);
}

TEST_F(CompilerTestFixture, MultExpr) {
    parser.parseExpr(ast, "2 * 3");
    int result = (int) compiler.compile(ast, false).roundToDouble(true);
    EXPECT_EQ(result, 6);
}

TEST_F(CompilerTestFixture, DivExpr) {
    parser.parseExpr(ast, "5 / 2");
    int result = (int) compiler.compile(ast, false).roundToDouble(true);
    EXPECT_EQ(result, 2);
}


TEST_F(CompilerTestFixture, PowExpr) {
    parser.parseExpr(ast, "2 ^ 3");
    int result = (int) compiler.compile(ast, false).roundToDouble(true);
    EXPECT_EQ(result, 8);
}


TEST_F(CompilerTestFixture, ParenthesizedExpr) {
    parser.parseExpr(ast, "(1 + 2) * 2");
    int result = (int) compiler.compile(ast, false).roundToDouble(true);
    EXPECT_EQ(result, 6);
}

TEST_F(CompilerTestFixture, BinopPrecedence) {
    parser.parseExpr(ast, "1 + 2 * 2");
    int result = (int) compiler.compile(ast, false).roundToDouble(true);
    EXPECT_EQ(result, 5);
}

TEST_F(CompilerTestFixture, PowAssociativity) {
    parser.parseExpr(ast, "2 ^ 2 ^ 3");
    int result = (int) compiler.compile(ast, false).roundToDouble(true);
    EXPECT_EQ(result, 256);
}