#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H

#include "Exception.hpp"
#include "Python3BaseVisitor.h"
#include "Scope.hpp"
#include "utils.hpp"

Scope scope;

class EvalVisitor : public Python3BaseVisitor {
  // todo:override all methods of Python3BaseVisitor

  virtual antlrcpp::Any visitFile_input(Python3Parser::File_inputContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitFuncdef(Python3Parser::FuncdefContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitParameters(Python3Parser::ParametersContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitTypedargslist(Python3Parser::TypedargslistContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTfpdef(Python3Parser::TfpdefContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitStmt(Python3Parser::StmtContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) override {
    auto testlistArray = ctx->testlist();
    std::string varName = testlistArray[0]->getText();

    int len = testlistArray.size();
    if (ctx->augassign()) {
      std::string op = ctx->augassign()->getText();
      std::string varName2 = testlistArray[1]->getText();
      auto [success, value] = scope.varQuery(varName);
      auto [success2, value2] = scope.varQuery(varName2);
      if (success && success2) {
        if (op == "+=")
          scope.varRegister(varName, value + value2);
        else if (op == "-=")
          scope.varRegister(varName, value - value2);
        else if (op == "*=")
          scope.varRegister(varName, value * value2);
        else if (op == "/=")
          scope.varRegister(varName, value / value2);
        else if (op == "//=")
          scope.varRegister(varName, intDiv(value, value2));
        else if (op == "%=")
          scope.varRegister(varName, value % value2);
      }
    }

    if (len == 1) {
      visitTestlist(testlistArray[0]);
      return 0;
    } else if (len >= 2) {
      AnyValueList varData = visitTestlist(testlistArray[len - 1]);
      int varNum = varData.size();
      for (int i = len - 2; i >= 0; --i) {
        auto tests = testlistArray[i]->test();
        for (int j = 0; j < varNum; ++j) {
          std::string varName = testlistArray[i]->getText();
          scope.varRegister(varName, varData[i]);
        }
      }
      // throw Exception("", UNIMPLEMENTED);
    }

    // int varData = visitTestlist(testlistArray[1]);

    // if (!validateVarName(varName)) {
    //   throw Exception("", INVALID_VARNAME);
    // }
    // scope.varRegister(varName, varData);
    return 0;
  }

  virtual antlrcpp::Any visitAugassign(Python3Parser::AugassignContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIf_stmt(Python3Parser::If_stmtContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitWhile_stmt(Python3Parser::While_stmtContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitSuite(Python3Parser::SuiteContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitTest(Python3Parser::TestContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitOr_test(Python3Parser::Or_testContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitAnd_test(Python3Parser::And_testContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitNot_test(Python3Parser::Not_testContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitComparison(Python3Parser::ComparisonContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitComp_op(Python3Parser::Comp_opContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitArith_expr(Python3Parser::Arith_exprContext *ctx) override {
    auto terms = ctx->term();
    if (terms.size() == 1) return visitTerm(terms[0]).as<AnyValue>();

    auto ops = ctx->addorsub_op();
    AnyValue ans = visitTerm(terms[0]).as<AnyValue>();
    // std::cout << "ans: " << ans << std::endl;

    for (int i = 1; i < terms.size(); ++i) {
      std::string op = ops[i - 1]->getText();
      AnyValue term2 = visitTerm(terms[i]).as<AnyValue>();
      // std::cout << "term2: " << term2 << std::endl;
      if (op == "+")
        ans += term2;
      else if (op == "-")
        ans -= term2;
    }
    // std::cout << "return ans: " << ans << std::endl;
    return ans;
  }

  virtual antlrcpp::Any visitAddorsub_op(Python3Parser::Addorsub_opContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitTerm(Python3Parser::TermContext *ctx) override {
    auto factors = ctx->factor();
    if (factors.size() == 1) return visitFactor(factors[0]).as<AnyValue>();

    auto ops = ctx->muldivmod_op();
    AnyValue ans = visitFactor(factors[0]).as<AnyValue>();

    for (int i = 1; i < factors.size(); ++i) {
      std::string op = ops[i - 1]->getText();
      if (op == "*")
        ans *= visitFactor(factors[i]);
      else if (op == "/")
        ans /= visitFactor(factors[i]);
      else if (op == "//")
        ans = intDiv(ans, visitFactor(factors[i]));
      else if (op == "%")
        ans %= visitFactor(factors[i]);
    }
    return ans;
  }

  virtual antlrcpp::Any visitMuldivmod_op(Python3Parser::Muldivmod_opContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFactor(Python3Parser::FactorContext *ctx) override {
    std::string ctxText = ctx->getText();
    if (ctxText[0] == '-') {
      if (ctx->factor()) {
        return -visitFactor(ctx->factor()).as<AnyValue>();
      } else if (ctx->atom_expr()) {
        return -visitAtom_expr(ctx->atom_expr()).as<AnyValue>();
      }
    } else {
      if (ctx->factor()) {
        return visitFactor(ctx->factor());
      } else if (ctx->atom_expr()) {
        return visitAtom_expr(ctx->atom_expr());
      }
    }
  }

  virtual antlrcpp::Any visitAtom_expr(Python3Parser::Atom_exprContext *ctx) override {
    if (ctx->trailer()) {
      // TODO: 函数调用
      std::string funcText = ctx->atom()->getText();
      // a temporary print
      // TODO: modify it
      if (funcText == "print") {
        // std::cout << "try to print" << std::endl;
        std::cout << visitTest(ctx->trailer()->arglist()->argument()[0]->test()[0]).as<AnyValue>() << std::endl;
        return AnyValue();
      }
    } else {
      return visitAtom(ctx->atom());
    }
  }

  virtual antlrcpp::Any visitTrailer(Python3Parser::TrailerContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitAtom(Python3Parser::AtomContext *ctx) override {
    std::string ctxText = ctx->getText();
    if (ctx->NAME()) {
      std::string varName = ctx->NAME()->getText();
      auto [success, value] = scope.varQuery(varName);
      if (success)
        return value;
      else
        throw Exception(NameError, "name '" + varName + "' is not defined");
    } else if (ctx->NUMBER()) {
      return AnyValue(BigInt(ctxText));
    } else if (ctxText == "None") {
      return AnyValue();
    } else if (ctxText == "True") {
      return AnyValue(true);
    } else if (ctxText == "False") {
      return AnyValue(false);
    }
    auto strings = ctx->STRING();
    if (!strings.empty()) {
      std::string ans;
      for (auto i : strings) {
        ans += i->getText();
      }
      return AnyValue(ans);
    }
  }

  virtual antlrcpp::Any visitTestlist(Python3Parser::TestlistContext *ctx) override {
    // TODO: unnecessary?
    auto test = ctx->test();
    AnyValueList ans;
    for (auto i : test) {
      ans.push_back(visitTest(i).as<AnyValue>());
    }
    return ans;
    // return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArglist(Python3Parser::ArglistContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitArgument(Python3Parser::ArgumentContext *ctx) override { return visitChildren(ctx); }
};

#endif  // PYTHON_INTERPRETER_EVALVISITOR_H
