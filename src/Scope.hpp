#ifndef APPLE_PIE_SCOPE_H
#define APPLE_PIE_SCOPE_H

#include <queue>
#include <string>
#include <unordered_map>

#include "AnyType.hpp"
#include "Python3BaseVisitor.h"

class Func {
  struct Para {
    std::string name;
    Python3Parser::TestContext* defaultVal;
    Para(std::string name_, Python3Parser::TestContext* test) : name(name_), defaultVal(test) {}
  };

 public:
  Python3Parser::FuncdefContext* ctx;
  // Python3Parser::ParametersContext* para;
  std::vector<Para> paras;
  Python3Parser::SuiteContext* suite;
  //     defaultArguments;
  // TODO: 执行函数
  Func(Python3Parser::FuncdefContext* ctx_) : ctx(ctx_) {
#ifdef DEBUG
    std::cout << "Now creating a function: ";
#endif  // DEBUG
    if (ctx == nullptr) {
#ifdef DEBUG
      std::cout << "function is nullptr" << std::endl;
#endif  // DEBUG
      return;
    }
    suite = ctx_->suite();
#ifdef DEBUG
    if (suite) {
      std::cout << "suite is not nullptr" << std::endl;
    }
#endif  // DEBUG
    auto args = ctx->parameters()->typedargslist();
    if (args) {
#ifdef DEBUG
      std::cout << "has parameter" << std::endl;
#endif  // DEBUG
      auto paraNames = args->tfpdef();
      auto tests = args->test();
      auto paraNum = paraNames.size();
      auto testNum = tests.size();
      auto shift = paraNum - testNum;
      for (std::size_t i = 0; i < paraNum; ++i) {
        paras.push_back(Para(paraNames[i]->getText(), i >= shift ? tests[i - shift] : nullptr));
      }
    } else {
#ifdef DEBUG
      std::cout << "not have parameter" << std::endl;
#endif  // DEBUG
    }
  }
};

class ScopeStack;
class Scope {
  friend ScopeStack;

 private:
  std::unordered_map<std::string, AnyValue> varTable;
  std::unordered_map<std::string, Func> funcTable;  // TODO

 public:
  //  std::string name;
  Scope() : varTable() {}
  AnyValue& operator[](const std::string& s) { return varTable[s]; }
  void varRegister(const std::string& varName, const AnyValue& varData) { varTable[varName] = varData; }

  std::pair<bool, AnyValue> varQuery(const std::string& varName) const {
    auto it = varTable.find(varName);
    if (it == varTable.end()) return std::make_pair(false, AnyValue());
    return std::make_pair(true, it->second);
  }
  void funcRegister(Python3Parser::FuncdefContext* ctx) {
    std::string funcName = ctx->NAME()->getText();
#ifdef DEBUG
    std::cout << "Register function: " << funcName;
#endif  // DEBUG
    funcTable.emplace(funcName, Func(ctx));
#ifdef DEBUG
    std::cout << " succeeded" << std::endl;
#endif  // DEBUG
  }
  Func funcQuery(const std::string& funcName) const {
    auto it = funcTable.find(funcName);
    if (it == funcTable.end()) {
      throw Exception(NameError, funcName);
      return nullptr;
    }
    return it->second;
  }
  bool hasVar(const std::string& varName) const { return varTable.count(varName); }
};

// enum Status { GLOBAL, WHILE, FUNCTION };

class ScopeStack {
  std::vector<Scope> scopes;
  // std::vector<Status> status;

 public:
  ScopeStack() {
    scopes.push_back(Scope());
    // status.push_back(GLOBAL);
  }

  void varRegister(const std::string& varName, const AnyValue& varData) {
    // std::cout << "varName: " << varName << " varData: " << varData << std::endl;
    if (scopes.size() > 1) {
      if (scopes.back().hasVar(varName)) {  // in current scope
        scopes.back().varRegister(varName, varData);
        return;
      }
    }
    if (scopes.front().hasVar(varName)) {  // in global scope
      scopes.front().varRegister(varName, varData);
      return;
    }
    scopes.back().varRegister(varName, varData);  // create
  }

  AnyValue varQuery(const std::string& varName) {
    // auto result = scopes.back().varQuery(varName);

    if (scopes.size() > 1) {
      auto it = scopes.back().varTable.find(varName);
      if (it != scopes.back().varTable.end()) {
        return it->second;
      }
    }
    auto it = scopes.front().varTable.find(varName);
    if (it != scopes.front().varTable.end()) {
      return it->second;
    }

    // if (scopes.back().hasVar(varName)) {
    //   return scopes.back()[varName];
    // }
    // if (scopes.front().hasVar(varName)) {
    //   return scopes.front()[varName];
    // }

    // auto result = scopes.back().varQuery(varName);
    // if (result.first) {
    //   return result.second;
    // }
    // result = scopes.front().varQuery(varName);
    // if (result.first) {
    //   return result.second;
    // }

    // for (auto it = scopes.rbegin(); it != scopes.rend(); it++) {
    //   auto [success, val] = it->varQuery(varName);
    //   if (success) return std::make_pair(true, val);
    // }

    throw Exception(NameError, varName);
    return AnyValue();
  }
  void funcRegister(Python3Parser::FuncdefContext* ctx) { scopes.front().funcRegister(ctx); }
  Func funcQuery(const std::string& funcName) const {
    // for (auto it = scopes.rbegin(); it != scopes.rend(); it++) {
    //   auto [success, val] = it->funcQuery(funcName);
    //   if (success) return std::make_pair(true, val);
    // }
    // return std::make_pair(false, nullptr);
    return scopes.front().funcQuery(funcName);
  }
  // void enterWhile() { status.push_back(WHILE); }
  // void quitWhile() {
  //   if (status.back() == WHILE) {
  //     status.pop_back();
  //   } else {
  //     throw "Error: Current Status is not WHILE";
  //   }
  // }
  void enterFunc(const Scope& scope) {
    // std::cout << "enterFunc" << std::endl;
    scopes.push_back(scope);
    // status.push_back(FUNCTION);
  }
  void quitFunc() {
    // std::cout << "quitFunc" << std::endl;
    // if (status.back() == FUNCTION) {
    scopes.pop_back();
    // status.pop_back();
    // } else {
    // throw "Error: Current Status is not FUNCTION";
    // }
  }
  // bool varExistInCurrentScope(std::string varName) { return scopes.back().hasVar(varName); }
  // Status currentStatus() { return status.back(); }
};

#endif  // APPLE_PIE_SCOPE_H