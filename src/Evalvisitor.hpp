#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H

#include <functional>
#include <unordered_map>

#include "Exception.hpp"
#include "Python3BaseVisitor.h"
#include "Scope.hpp"
#include "utils.hpp"

ScopeStack scope;

// Return Value Definition
// 0: Statement finished normally
// AnyValue(BREAK): break a loop
// AnyValue(CONTINUE): continue
// AnyValue (Not above): value of a node, or return value of a function
//                       "void" function returns None

class EvalVisitor : public Python3BaseVisitor {
  virtual antlrcpp::Any visitFile_input(Python3Parser::File_inputContext *ctx) override { return visitChildren(ctx); }

  // Returns 0
  virtual antlrcpp::Any visitFuncdef(Python3Parser::FuncdefContext *ctx) override {
    Func fun;
    fun.suite = ctx->suite();
    auto args = ctx->parameters()->typedargslist();
    if (args) {
      auto paraNames = args->tfpdef();
      auto tests = args->test();
      auto paraNum = paraNames.size();
      auto testNum = tests.size();
      auto shift = paraNum - testNum;
      for (std::size_t i = 0; i < paraNum; ++i) {
        fun.paras.push_back(
            Func::Para(paraNames[i]->getText(), i >= shift ? visitTest(tests[i - shift]).as<AnyValue>() : AnyValue()));
      }
    }
    scope.funcRegister(ctx->NAME()->getText(), fun);
    return 0;
  }

  // Not used
  virtual antlrcpp::Any visitParameters(Python3Parser::ParametersContext *ctx) override { return visitChildren(ctx); }

  // Not used
  virtual antlrcpp::Any visitTypedargslist(Python3Parser::TypedargslistContext *ctx) override {
    return visitChildren(ctx);
  }

  // Not used
  virtual antlrcpp::Any visitTfpdef(Python3Parser::TfpdefContext *ctx) override { return visitChildren(ctx); }

  // Returns 0 when finished normally, AnyValue when encountering BREAK, CONTINUE or RETURN
  virtual antlrcpp::Any visitStmt(Python3Parser::StmtContext *ctx) override {
    if (ctx->simple_stmt()) return visitSimple_stmt(ctx->simple_stmt());
    return visitCompound_stmt(ctx->compound_stmt());
  }

  // Returns 0 when finished normally, AnyValue when encountering BREAK, CONTINUE or RETURN
  virtual antlrcpp::Any visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) override {
    return visitSmall_stmt(ctx->small_stmt());
  }

  // Returns 0 when finished normally, AnyValue when encountering BREAK, CONTINUE or RETURN
  virtual antlrcpp::Any visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) override {
    if (ctx->expr_stmt()) {
      return visitExpr_stmt(ctx->expr_stmt());
    }
    return visitFlow_stmt(ctx->flow_stmt());
  }

  // Returns 0
  virtual antlrcpp::Any visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) override {
    auto testlists = ctx->testlist();

    int len = testlists.size();
    if (ctx->augassign()) {
      std::string varName = testlists[0]->getText();
      // auto op = ctx->augassign()->getText();
      auto augassign = ctx->augassign();
      auto &value = scope.varQuery(varName);

      const AnyValue &value2 = visitTest(testlists[1]->test()[0]);

      // if (op == "+=")
      if (augassign->ADD_ASSIGN()) value += value2;
      // else if (op == "-=")
      else if (augassign->SUB_ASSIGN())
        value -= value2;
      // else if (op == "*=")
      else if (augassign->MULT_ASSIGN())
        value *= value2;
      // else if (op == "/=")
      else if (augassign->DIV_ASSIGN())
        value /= value2;
      // else if (op == "//=")
      else if (augassign->IDIV_ASSIGN())
        value = intDiv(value, value2);
      // else if (op == "%=")
      else if (augassign->MOD_ASSIGN())
        value %= value2;
      return 0;
    }

    if (len == 1) {
      visitTestlist(testlists[0]);
      return 0;  // TODO: check
    } else if (len >= 2) {
      AnyValueList varData = visitTestlist(testlists[len - 1]);
      int varNum = varData.size();
      for (int i = len - 2; ~i; --i) {
        auto tests = testlists[i]->test();
        for (int j = 0; j < varNum; ++j) {
          std::string varName = tests[j]->getText();
          scope.varRegister(varName, varData[j]);
        }
      }
      return 0;
    }
    return 0;
  }

  // Not used
  virtual antlrcpp::Any visitAugassign(Python3Parser::AugassignContext *ctx) override { return visitChildren(ctx); }

  // Returns AnyValue or AnyValueList
  virtual antlrcpp::Any visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) override {
    if (ctx->return_stmt()) {
      return visitReturn_stmt(ctx->return_stmt());
    }
    if (ctx->break_stmt()) return AnyValue(BREAK);
    // if (ctx->continue_stmt()) return AnyValue(CONTINUE);
    return AnyValue(CONTINUE);
  }

  // Not used
  virtual antlrcpp::Any visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) override { return AnyValue(BREAK); }

  // Not used
  virtual antlrcpp::Any visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) override {
    return AnyValue(CONTINUE);
  }

  // Returns AnyValue or AnyValueList
  virtual antlrcpp::Any visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) override {
    auto testlist = ctx->testlist();
    if (testlist) {
      auto result = visitTestlist(testlist);
      if (result.is<AnyValueList>()) {
        AnyValueList resultVal = result;
        if (resultVal.size() == 1) return resultVal[0];
      }
      return result;
    }
    return AnyValue();  // "void" function returns None
  }

  // Returns 0 when finished normally, AnyValue when encountering BREAK, CONTINUE or RETURN
  virtual antlrcpp::Any visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  // Returns 0 when finished normally, AnyValue when encountering BREAK, CONTINUE or RETURN
  virtual antlrcpp::Any visitIf_stmt(Python3Parser::If_stmtContext *ctx) override {
    auto tests = ctx->test();
    auto testNum = tests.size();
    auto suites = ctx->suite();
    // go through if and elif(s)
    for (std::size_t i = 0; i < testNum; ++i) {
      if (visitTest(tests[i]).as<AnyValue>().toBool()) {
        return visitSuite(suites[i]);
      }
    }
    // go into else (if exists)
    if (ctx->ELSE()) {
      return visitSuite(suites.back());
    }
    return 0;
  }

  // Returns 0 when finished normally, AnyValue when encountering RETURN
  virtual antlrcpp::Any visitWhile_stmt(Python3Parser::While_stmtContext *ctx) override {
    auto test = ctx->test();
    auto suite = ctx->suite();
    while (visitTest(test).as<AnyValue>().toBool()) {
      auto result = visitSuite(suite);
      if (result.is<AnyValue>()) {
        AnyValue resultVal = result;
        if (resultVal.isBREAK()) break;
        if (resultVal.isCONTINUE()) continue;
        return result;  // Then result will only be return value of a function
      }
    }
    return 0;
  }

  // Returns 0 when finished normally,
  // AnyValue when encountering BREAK or CONTINUE,
  // AnyValue or AnyValueList when encountering RETURN
  virtual antlrcpp::Any visitSuite(Python3Parser::SuiteContext *ctx) override {
    // std::cout << "visitSuite" << std::endl;
    if (ctx->simple_stmt()) {
      // std::cout << "simpe stmt: " << ctx->simple_stmt()->getText() << std::endl;
      return visitSimple_stmt(ctx->simple_stmt());
    }
    auto stmts = ctx->stmt();
    for (auto i : stmts) {
      // std::cout << "Now run: " << i->getText() << std::endl;
      auto result = visitStmt(i);
      // if (result.is<AnyValueList>()) return result;  // return value of a function
      if (result.is<AnyValue>()) return result;
      if (result.is<AnyValueList>()) {
        // AnyValueList resultVal = result;
        // if (resultVal.size() == 1) return resultVal[0];
        // BREAK, CONTINUE or RETURN
        // All will stop current suite. So return it directly.
        return result;
      }
    }
    return 0;
  }

  // Returns AnyValue or AnyValueList
  virtual antlrcpp::Any visitTest(Python3Parser::TestContext *ctx) override { return visitOr_test(ctx->or_test()); }

  // Returns AnyValue or AnyValueList
  virtual antlrcpp::Any visitOr_test(Python3Parser::Or_testContext *ctx) override {
    auto and_tests = ctx->and_test();
    if (and_tests.size() > 1) {
      for (auto i : and_tests) {
        if (visitAnd_test(i).as<AnyValue>().toBool()) return AnyValue(true);
      }
      return AnyValue(false);
    } else {
      return visitAnd_test(and_tests[0]);
    }
  }

  // Returns AnyValue or AnyValueList
  virtual antlrcpp::Any visitAnd_test(Python3Parser::And_testContext *ctx) override {
    auto not_tests = ctx->not_test();
    if (not_tests.size() > 1) {
      for (auto i : not_tests) {
        if (!visitNot_test(i).as<AnyValue>().toBool()) return AnyValue(false);
      }
      return AnyValue(true);
    } else {
      return visitNot_test(not_tests[0]);
    }
  }

  // Returns AnyValue or AnyValueList
  virtual antlrcpp::Any visitNot_test(Python3Parser::Not_testContext *ctx) override {
    if (ctx->not_test())
      return AnyValue(!visitNot_test(ctx->not_test()).as<AnyValue>().toBool());
    else
      return visitComparison(ctx->comparison());
  }

  // Returns AnyValue or AnyValueList
  virtual antlrcpp::Any visitComparison(Python3Parser::ComparisonContext *ctx) override {
    auto arith_exprs = ctx->arith_expr();
    auto arith_exprNum = arith_exprs.size();
    auto ops = ctx->comp_op();
    if (ops.empty()) {
      return visitArith_expr(arith_exprs[0]);
    }
    AnyValue last_val = visitArith_expr(arith_exprs[0]);
    // for (auto i : arith_exprs) {
    //   vals.push_back(visitArith_expr(i));
    // }
    bool ans = true;
    for (int i = 1; i < arith_exprNum; ++i) {
      AnyValue val = visitArith_expr(arith_exprs[i]);
      std::string op = ops[i - 1]->getText();
      if (op == "<") {
        ans = last_val < val;
      } else if (op == ">") {
        ans = last_val > val;
      } else if (op == "<=") {
        ans = last_val <= val;
      } else if (op == ">=") {
        ans = last_val >= val;
      } else if (op == "==") {
        ans = last_val == val;
      } else if (op == "!=") {
        ans = last_val != val;
      }
      if (!ans) return AnyValue(false);
      last_val = val;
    }
    return AnyValue(true);
  }

  // Not used
  virtual antlrcpp::Any visitComp_op(Python3Parser::Comp_opContext *ctx) override { return visitChildren(ctx); }

  // Returns AnyValue or AnyValueList
  virtual antlrcpp::Any visitArith_expr(Python3Parser::Arith_exprContext *ctx) override {
    auto terms = ctx->term();
    auto termNum = terms.size();
    if (termNum == 1) return visitTerm(terms[0]);

    auto ops = ctx->addorsub_op();
    AnyValue ans = visitTerm(terms[0]);
    // std::cout << "ans: " << ans << std::endl;

    for (int i = 1; i < termNum; ++i) {
      // std::string op = ops[i - 1] ->getText();
      AnyValue term2 = visitTerm(terms[i]);
      // std::cout << "term2: " << term2 << std::endl;
      if (ops[i - 1]->ADD())
        ans += term2;
      else /*if (op == "-")*/
        ans -= term2;
    }
    // std::cout << "return ans: " << ans << std::endl;
    return ans;
  }

  // Not used
  virtual antlrcpp::Any visitAddorsub_op(Python3Parser::Addorsub_opContext *ctx) override { return visitChildren(ctx); }

  // Returns AnyValue or AnyValueList
  virtual antlrcpp::Any visitTerm(Python3Parser::TermContext *ctx) override {
    auto factors = ctx->factor();
    auto factorNum = factors.size();
    if (factorNum == 1) return visitFactor(factors[0]);

    auto ops = ctx->muldivmod_op();
    AnyValue ans = visitFactor(factors[0]);

    for (int i = 1; i < factorNum; ++i) {
      // std::string op = ops[i - 1]->getText();
      // if (op == "*")
      if (ops[i - 1]->STAR()) ans *= visitFactor(factors[i]);
      // else if (op == "/")
      else if (ops[i - 1]->DIV())
        ans /= visitFactor(factors[i]);
      // else if (op == "//")
      else if (ops[i - 1]->IDIV())
        ans = intDiv(ans, visitFactor(factors[i]));
      // else if (op == "%")
      else if (ops[i - 1]->MOD())
        ans %= visitFactor(factors[i]);
    }
    return ans;
  }

  // Not used
  virtual antlrcpp::Any visitMuldivmod_op(Python3Parser::Muldivmod_opContext *ctx) override {
    return visitChildren(ctx);
  }

  // Returns AnyValue or AnyValueList
  virtual antlrcpp::Any visitFactor(Python3Parser::FactorContext *ctx) override {
    if (ctx->MINUS()) {
      if (ctx->factor()) {
        return -visitFactor(ctx->factor()).as<AnyValue>();
      }
      return -visitAtom_expr(ctx->atom_expr()).as<AnyValue>();
    } else {
      if (ctx->factor()) {
        return visitFactor(ctx->factor());
      }
      return visitAtom_expr(ctx->atom_expr());
    }
  }

  // Returns AnyValue or AnyValueList
  virtual antlrcpp::Any visitAtom_expr(Python3Parser::Atom_exprContext *ctx) override {
    if (ctx->trailer()) {
      std::string funcName = ctx->atom()->getText();
      std::vector<Python3Parser::ArgumentContext *> args;
      if (ctx->trailer()->arglist()) args = ctx->trailer()->arglist()->argument();
      auto argNum = args.size();
      // Built-in functions
      // std::cout << "funcName: " << funcName << std::endl;
      if (funcName == "print") {
        // std::cout << "try to print" << std::endl;
        AnyValueList visitedArgs;
        for (auto arg : args) {
          visitedArgs.push_back(visitTest(arg->test()[0]));
        }
        bool second = false;
        for (auto arg : visitedArgs) {
          if (second) std::cout << " ";
          second = true;
          std::cout << arg;
        }
        std::cout << std::endl;
        return AnyValue();
      } else if (funcName == "int") {
        return AnyValue(visitTest(args[0]->test()[0]).as<AnyValue>().toBigInt());
      } else if (funcName == "float") {
        return AnyValue(visitTest(args[0]->test()[0]).as<AnyValue>().toDouble());
      } else if (funcName == "str") {
        return AnyValue(visitTest(args[0]->test()[0]).as<AnyValue>().toString());
      } else if (funcName == "bool") {
        return AnyValue(visitTest(args[0]->test()[0]).as<AnyValue>().toBool());
      }

      // 函数调用
      auto &func = scope.funcQuery(funcName);
      Scope funcScope;
      // arguments -> parameters
      auto &paras = func.paras;
      auto paraNum = paras.size();
      std::size_t i;
      for (i = 0; i < argNum; ++i) {  // Positional argument
        auto tests = args[i]->test();
        if (tests.size() == 1) {
          funcScope.varRegister(paras[i].name, visitTest(tests[0]));
        } else {
          break;
        }
      }
      for (std::size_t j = i; j < argNum; ++j) {  // Keyword argument
        auto tests = args[j]->test();
        funcScope.varRegister(tests[0]->getText(), visitTest(tests[1]));
      }
      for (; i < paraNum; ++i) {
        if (funcScope.hasVar(paras[i].name)) continue;
        funcScope.varRegister(paras[i].name, paras[i].defaultVal);
      }
      // call the function
      scope.enterFunc(funcScope);  // directly enter this scope before processing arguments will cause error
      // auto result = visitSuite(func.suite).as<AnyValueList>();
      auto result = visitSuite(func.suite);
      scope.quitFunc();
      if (result.is<AnyValue>() || result.is<AnyValueList>()) return result;
      return AnyValue();  // "void" function returns None
#ifdef DEBUG
      std::cout << "Function " << funcName << " returns: " << returnVal << std::endl;
#endif  // DEBUG
    } else {
      return visitAtom(ctx->atom());
    }
  }

  // Not used
  virtual antlrcpp::Any visitTrailer(Python3Parser::TrailerContext *ctx) override { return visitChildren(ctx); }

  // Returns AnyValue or AnyValueList
  virtual antlrcpp::Any visitAtom(Python3Parser::AtomContext *ctx) override {
    if (ctx->test()) {
      return visitTest(ctx->test());
    }

    if (ctx->NAME()) {
      return scope.varQuery(ctx->getText());
    } else if (ctx->NUMBER()) {
      std::string ctxText = ctx->getText();
      if (ctxText.find(".") == std::string::npos)
        return AnyValue(BigInt(ctxText));
      else {
        return AnyValue(std::stod(ctxText));
      }
    } else if (ctx->NONE()) {
      return AnyValue();
    } else if (ctx->TRUE()) {
      return AnyValue(true);
    } else if (ctx->FALSE()) {
      return AnyValue(false);
    }
    auto strings = ctx->STRING();
    if (!strings.empty()) {
      std::string ans;
      for (auto i : strings) {
        std::string s(i->getText());
        auto len = s.size();
        auto front = s.substr(0, 3);
        if (front == "\"\"\"" || front == "'''") {
          ans += s.substr(3, len - 6);
        } else {
          ans += s.substr(1, len - 2);
        }
      }
      return AnyValue(ans);
    }
    return AnyValue();
  }

  // Returns AnyValueList
  virtual antlrcpp::Any visitTestlist(Python3Parser::TestlistContext *ctx) override {
    auto tests = ctx->test();
    AnyValueList ans;
    for (auto i : tests) {
      auto result = visitTest(i);
      if (result.is<AnyValueList>()) {
        // unpack return value of function
        AnyValueList resultVal = result;
        for (auto i : resultVal) {
          ans.push_back(i);
        }
      } else {
        ans.push_back(result);
      }
    }
    return ans;
  }

  // Not used
  virtual antlrcpp::Any visitArglist(Python3Parser::ArglistContext *ctx) override { return visitChildren(ctx); }

  // Not used
  virtual antlrcpp::Any visitArgument(Python3Parser::ArgumentContext *ctx) override { return visitChildren(ctx); }
};

#endif  // PYTHON_INT