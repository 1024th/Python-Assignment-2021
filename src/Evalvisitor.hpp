#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H

#include <functional>
// #include <regex>
#include <unordered_map>

#include "Exception.hpp"
#include "Python3BaseVisitor.h"
#include "Scope.hpp"
#include "utils.hpp"

ScopeStack scope;
// const AnyValue True(true), False(false), None;

class EvalVisitor : public Python3BaseVisitor {
  // todo:override all methods of Python3BaseVisitor

  virtual antlrcpp::Any visitFile_input(Python3Parser::File_inputContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitFuncdef(Python3Parser::FuncdefContext *ctx) override {
    // scope.funcRegister(ctx);
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
            Func::Para(paraNames[i]->getText(), i >= shift ? visitTest(tests[i - shift]).as<AnyValue>() : None));
      }
    }
    scope.funcRegister(ctx->NAME()->getText(), fun);
    return 0;
    // return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitParameters(Python3Parser::ParametersContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitTypedargslist(Python3Parser::TypedargslistContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTfpdef(Python3Parser::TfpdefContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitStmt(Python3Parser::StmtContext *ctx) override {
    // std::cout << "in visitStmt: ";
    // return
    // auto result = visitChildren(ctx);
    // if (result.is<AnyValue>()) {
    //   std::cout << "visitStmt's returnVal is: " << result.as<AnyValue>() << std::endl;
    // } else {
    //   std::cout << "visitStmt's returnVal is not AnyValue" << std::endl;
    // }
    // return result;
    if (ctx->simple_stmt()) return visitSimple_stmt(ctx->simple_stmt());
    return visitCompound_stmt(ctx->compound_stmt());
    // return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) override {
    // return visitChildren(ctx);
    return visitSmall_stmt(ctx->small_stmt());
    // std::cout << "in visitSimple_stmt: ";
    // // return
    // auto result = visitChildren(ctx);
    // if (result.is<AnyValue>()) {
    //   std::cout << "visitSimple_stmt's returnVal is: " << result.as<AnyValue>() << std::endl;
    // } else {
    //   std::cout << "visitSimple_stmt's returnVal is not AnyValue" << std::endl;
    // }
    // return result;
  }

  virtual antlrcpp::Any visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) override {
    if (ctx->expr_stmt()) {
      return visitExpr_stmt(ctx->expr_stmt());
    }
    return visitFlow_stmt(ctx->flow_stmt());
    // return visitChildren(ctx);

    // std::cout << "in visitSmall_stmt: ";
    // return
    // auto result = visitChildren(ctx);
    // if (result.is<AnyValue>()) {
    //   std::cout << "visitSmall_stmt's returnVal is: " << result.as<AnyValue>() << std::endl;
    // } else {
    //   std::cout << "visitSmall_stmt's returnVal is not AnyValue" << std::endl;
    // }
    // return result;
  }

  virtual antlrcpp::Any visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) override {
    auto testlists = ctx->testlist();

    int len = testlists.size();
    if (ctx->augassign()) {
      std::string varName = testlists[0]->getText();
      // auto op = ctx->augassign()->getText();
      auto augassign = ctx->augassign();
      // std::string varName2 = testlists[1]->getText();
      // auto [success, value] = scope.varQuery(varName);
      auto& value = scope.varQuery(varName);

      // auto [success2, value2] = scope.varQuery(varName2);
      // AnyValue value2 = visitTestlist(testlists[1]).as<AnyValueList>()[0];
      AnyValue value2 = visitTest(testlists[1]->test()[0]).as<AnyValue>();
      // if (success && success2) {
      // if (success) {
      // if (op == "+=")
      if (augassign->ADD_ASSIGN())
        // scope.varRegister(varName, value + value2);
        value += value2;
      // else if (op == "-=")
      else if(augassign->SUB_ASSIGN())
        // scope.varRegister(varName, value - value2);
        value -= value2;
      // else if (op == "*=")
      else if (augassign->MULT_ASSIGN())
        // scope.varRegister(varName, value * value2);
        value *= value2;
      // else if (op == "/=")
      else if (augassign->DIV_ASSIGN())
        // scope.varRegister(varName, value / value2);
        value /= value2;
      // else if (op == "//=")
      else if (augassign->IDIV_ASSIGN())
        // scope.varRegister(varName, intDiv(value, value2));
        value = intDiv(value, value2);
      // else if (op == "%=")
      else if (augassign->MOD_ASSIGN())
        // scope.varRegister(varName, value % value2);
        value %= value2;
      // }
      return 0;
    }

    if (len == 1) {
      // TODO: check
      // No need to 实现 多返回值
      visitTestlist(testlists[0]);
      // std::cout << "visitExpr_stmt returns: " << returnVal << std::endl;
      return 0;  // TODO: check
    } else if (len >= 2) {
      // TODO: modify
      AnyValueList varData = visitTestlist(testlists[len - 1]).as<AnyValueList>();
      int varNum = varData.size();
      for (int i = len - 2; i >= 0; --i) {
        auto tests = testlists[i]->test();  // ?
        for (int j = 0; j < varNum; ++j) {
          // std::string varName = testlistArray[i]->getText();
          std::string varName = tests[j]->getText();
          scope.varRegister(varName, varData[j]);
        }
      }
      return 0;
    }
    // throw Exception(UndefinedBehavior);

    // int varData = visitTestlist(testlistArray[1]);

    // if (!validateVarName(varName)) {
    //   throw Exception("", INVALID_VARNAME);
    // }
    // scope.varRegister(varName, varData);
    return 0;
  }

  virtual antlrcpp::Any visitAugassign(Python3Parser::AugassignContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) override {
    if (ctx->return_stmt()) {
      return visitReturn_stmt(ctx->return_stmt());
      // return visitTestlist(ctx->return_stmt()->testlist()).as<AnyValueList>()[0];
      // auto testlist = ctx->return_stmt()->testlist();
      // if (testlist) return visitTest(testlist->test()[0]);
      // std::cout << "(in visitFlow_stmt) returnVal is: " << returnVal << std::endl;
      // return None;  // None
    }
    if (ctx->break_stmt()) return AnyValue(BREAK);
    if (ctx->continue_stmt()) return AnyValue(CONTINUE);
    return 0;
    // switch (scope.currentStatus()) {
    //   case FUNCTION:
    //     /* code */
    //     break;
    //   case WHILE:
    //     /* code */
    //     break;
    //   case GLOBAL:
    //     /* code */
    //     break;

    //   default:
    //     break;
    // }
    // return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) override { return AnyValue(BREAK); }

  virtual antlrcpp::Any visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) override {
    return AnyValue(CONTINUE);
  }

  virtual antlrcpp::Any visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) override {
    auto testlist = ctx->testlist();
    if (testlist) return visitTest(testlist->test()[0]);
    return None;
  }

  virtual antlrcpp::Any visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIf_stmt(Python3Parser::If_stmtContext *ctx) override {
    auto tests = ctx->test();
    auto testNum = tests.size();
    auto suites = ctx->suite();
    // go through if and elif(s)
    for (std::size_t i = 0; i < testNum; ++i) {
      if (visitTest(tests[i]).as<AnyValue>().toBool()) {
        return visitSuite(suites[i]);
        // auto result = visitSuite(suites[i]);
        // if (result.is<AnyValue>()) {
        //   // auto resultVal = result.as<AnyValue>();
        //   // if (resultVal.isValue()) return resultVal;
        //   return result;  // RETURN, BREAK, CONTINUE
        // } else {
        //   return 0;
        // }
      }
    }
    // go into else (if exists)
    auto suiteNum = suites.size();
    if (suiteNum > testNum) {
      return visitSuite(suites.back());
    }
    return 0;
    // return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhile_stmt(Python3Parser::While_stmtContext *ctx) override {
    auto test = ctx->test();
    auto suite = ctx->suite();
    // scope.enterWhile();
    while (visitTest(test).as<AnyValue>().toBool()) {
      auto result = visitSuite(suite);
      // if (result.is<AnyValueList>()) {
      //   // result is the return value of a function, return it to the funciton.
      //   scope.quitWhile();
      //   return result;
      // }
      if (result.is<AnyValue>()) {
        AnyValue resultVal = result;
        if (resultVal.isBREAK()) break;
        if (resultVal.isValue()) {
          // scope.quitWhile();
          return result;
        }
        // if (resultVal.isCONTINUE()) continue;  // unnecessary
      }
    }
    // scope.quitWhile();
    return 0;
    // return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSuite(Python3Parser::SuiteContext *ctx) override {
    // std::cout << "visitSuite" << std::endl;
    if (ctx->simple_stmt()) {
      // std::cout << "simpe stmt: " << ctx->simple_stmt()->getText() << std::endl;
      return visitSimple_stmt(ctx->simple_stmt());
    }
    auto stmts = ctx->stmt();
    // TODO
    for (auto i : stmts) {
      // std::cout << "Now run: " << i->getText() << std::endl;
      auto result = visitStmt(i);
      // if (result.is<AnyValueList>()) return result;  // return value of a function
      if (result.is<AnyValue>()) {
        // AnyValue resultVal = result;
        // std::cout << "(in visitSuite) returnVal is: " << resultVal << std::endl;
        // if (resultVal.isValue()) {
        //   return resultVal;
        // }

        // BREAK, CONTINUE or RETURN
        // All will stop current suite. So return it directly.
        return result;
      }
    }
    return 0;
    // return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTest(Python3Parser::TestContext *ctx) override { return visitOr_test(ctx->or_test()); }

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
  virtual antlrcpp::Any visitNot_test(Python3Parser::Not_testContext *ctx) override {
    if (ctx->not_test())
      return AnyValue(!visitNot_test(ctx->not_test()).as<AnyValue>().toBool());
    else
      return visitComparison(ctx->comparison());
  }

  virtual antlrcpp::Any visitComparison(Python3Parser::ComparisonContext *ctx) override {
    auto arith_exprs = ctx->arith_expr();
    auto ops = ctx->comp_op();
    if (ops.empty()) {
      return visitArith_expr(arith_exprs[0]);
    }
    std::vector<AnyValue> vals;
    vals.push_back(visitArith_expr(arith_exprs[0]));
    // for (auto i : arith_exprs) {
    //   vals.push_back(visitArith_expr(i));
    // }
    bool ans = true;
    for (int i = 1; i < arith_exprs.size(); ++i) {
      vals.push_back(visitArith_expr(arith_exprs[i]));
      std::string op = ops[i - 1]->getText();
      if (op == "<") {
        ans = vals[i - 1] < vals[i];
      } else if (op == ">") {
        ans = vals[i - 1] > vals[i];
      } else if (op == "<=") {
        ans = vals[i - 1] <= vals[i];
      } else if (op == ">=") {
        ans = vals[i - 1] >= vals[i];
      } else if (op == "==") {
        ans = vals[i - 1] == vals[i];
      } else if (op == "!=") {
        ans = vals[i - 1] != vals[i];
      }
      if (!ans) return AnyValue(false);
    }
    return AnyValue(true);
    // return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitComp_op(Python3Parser::Comp_opContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitArith_expr(Python3Parser::Arith_exprContext *ctx) override {
    auto terms = ctx->term();
    if (terms.size() == 1) return visitTerm(terms[0]);

    auto ops = ctx->addorsub_op();
    AnyValue ans = visitTerm(terms[0]);
    // std::cout << "ans: " << ans << std::endl;

    for (int i = 1; i < terms.size(); ++i) {
      // std::string op = ops[i - 1] ->getText();
      AnyValue term2 = visitTerm(terms[i]);
      // std::cout << "term2: " << term2 << std::endl;
      if (ops[i-1]->ADD())
        ans += term2;
      else /*if (op == "-")*/
        ans -= term2;
    }
    // std::cout << "return ans: " << ans << std::endl;
    return ans;
  }

  virtual antlrcpp::Any visitAddorsub_op(Python3Parser::Addorsub_opContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitTerm(Python3Parser::TermContext *ctx) override {
    auto factors = ctx->factor();
    if (factors.size() == 1) return visitFactor(factors[0]);

    auto ops = ctx->muldivmod_op();
    AnyValue ans = visitFactor(factors[0]);

    for (int i = 1; i < factors.size(); ++i) {
      // std::string op = ops[i - 1]->getText();
      // if (op == "*")
      if (ops[i-1]->STAR())
        ans *= visitFactor(factors[i]);
      // else if (op == "/")
      else if (ops[i-1]->DIV())
        ans /= visitFactor(factors[i]);
      // else if (op == "//")
      else if (ops[i-1]->IDIV())
        ans = intDiv(ans, visitFactor(factors[i]));
      // else if (op == "%")
      else if (ops[i-1]->MOD())
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
      std::string funcName = ctx->atom()->getText();
      std::vector<Python3Parser::ArgumentContext *> args;
      if (ctx->trailer()->arglist()) args = ctx->trailer()->arglist()->argument();
      auto argNum = args.size();
      // Built-in functions
      // TODO: modify print to support multi parameter
      // std::cout << "funcName: " << funcName << std::endl;
      if (funcName == "print") {
        // std::cout << "try to print" << std::endl;
        bool second = false;
        for (auto arg : args) {
          if (second) std::cout << " ";
          second = true;
          std::cout << visitTest(arg->test()[0]).as<AnyValue>();
        }
        std::cout << std::endl;
        return None;
      } else if (funcName == "int") {
        return AnyValue(visitTest(args[0]->test()[0]).as<AnyValue>().toBigInt());
      } else if (funcName == "float") {
        return AnyValue(visitTest(args[0]->test()[0]).as<AnyValue>().toDouble());
      } else if (funcName == "str") {
        return AnyValue(visitTest(args[0]->test()[0]).as<AnyValue>().toString());
      } else if (funcName == "bool") {
        return AnyValue(visitTest(args[0]->test()[0]).as<AnyValue>().toBool());
      }

      // TODO: 函数调用!
      // auto [success, func] = scope.funcQuery(funcName);
      // if (!success) {
      //   throw Exception(NameError, funcName);
      //   return None;
      // }
      auto func = scope.funcQuery(funcName);
      Scope funcScope;
      // arguments -> parameters
      auto &paras = func.paras;
      auto paraNum = paras.size();
      std::size_t i;
      for (i = 0; i < argNum; ++i) {  // Positional argument
        auto tests = args[i]->test();
        if (tests.size() == 1) {
          std::string paraName = paras[i].name;
          // scope.varRegister(paraName, visitTest(tests[0]).as<AnyValue>());
          funcScope.varRegister(paraName, visitTest(tests[0]));
        } else {
          break;
        }
      }
      for (std::size_t j = i; j < argNum; ++j) {  // Keyword argument
        auto tests = args[j]->test();
        std::string paraName = tests[0]->getText();
        // scope.varRegister(paraName, visitTest(tests[1]).as<AnyValue>());
        funcScope.varRegister(paraName, visitTest(tests[1]));
      }
      for (; i < paraNum; ++i) {
        if (funcScope.hasVar(paras[i].name)) continue;
        funcScope.varRegister(paras[i].name, paras[i].defaultVal);
      }
      // call the function
      scope.enterFunc(funcScope);  // directly enter this scope before processing arguments will cause error
      // auto result = visitSuite(func.suite).as<AnyValueList>();
      auto result = visitSuite(func.suite);
      // AnyValue returnVal;
      // if (result.is<AnyValue>()) {
      //   returnVal = result;
      // }
      scope.quitFunc();
      if (result.is<AnyValue>()) return result;
      return None;
      // return result[0];
#ifdef DEBUG
      std::cout << "Function " << funcName << " returns: " << returnVal << std::endl;
#endif  // DEBUG
      // return returnVal;
      // if (result.size() == 1) return result[0];
      // return result;
    } else {
      return visitAtom(ctx->atom());
    }
  }

  virtual antlrcpp::Any visitTrailer(Python3Parser::TrailerContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitAtom(Python3Parser::AtomContext *ctx) override {
    if (ctx->test()) {
      return visitTest(ctx->test());
    }

    // std::string ctxText = ctx->getText();
    if (ctx->NAME()) {
      return scope.varQuery(ctx->getText());
      // auto [success, value] = scope.varQuery(ctxText);
      // if (success)
      //   return value;
      // else
      //   throw Exception(NameError, ctxText);
    } else if (ctx->NUMBER()) {
      std::string ctxText = ctx->getText();
      if (ctxText.find(".") == std::string::npos)
        return AnyValue(BigInt(ctxText));
      else {
        return AnyValue(std::stod(ctxText));
      }
    } else if (ctx->NONE()) {
      return None;
    } else if (ctx->TRUE()) {
      return True;
    } else if (ctx->FALSE()) {
      return False;
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
        // std::regex str_rgx("^(\"\"\"|'''|\"|')([^'\"]*)(\"\"\"|'''|\"|')$");
        // std::smatch matches;
        // std::regex_search(s, matches, str_rgx);
        // ans += matches[2].str();
      }
      return AnyValue(ans);
    }
  }

  virtual antlrcpp::Any visitTestlist(Python3Parser::TestlistContext *ctx) override {
    // TODO: unnecessary?
    auto tests = ctx->test();
    AnyValueList ans;
    for (auto i : tests) {
      ans.push_back(visitTest(i));
    }
    return ans;
    // return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArglist(Python3Parser::ArglistContext *ctx) override { return visitChildren(ctx); }

  virtual antlrcpp::Any visitArgument(Python3Parser::ArgumentContext *ctx) override { return visitChildren(ctx); }
};

#endif  // PYTHON_INT