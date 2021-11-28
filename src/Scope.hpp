#ifndef APPLE_PIE_SCOPE_H
#define APPLE_PIE_SCOPE_H

#include <queue>
#include <string>
#include <unordered_map>

#include "AnyType.hpp"
#include "Python3BaseVisitor.h"

class Func {
 public:
  struct Para {
    std::string name;
    AnyValue defaultVal;
    // Python3Parser::TestContext* defaultVal;
    Para(std::string name_, AnyValue defaultVal_) : name(name_), defaultVal(defaultVal_) {}
  };
  // Python3Parser::FuncdefContext* ctx;
  // Python3Parser::ParametersContext* para;
  std::vector<Para> paras;
  Python3Parser::SuiteContext* suite;
};

class ScopeStack;
class Scope {
  friend ScopeStack;

 private:
  std::unordered_map<std::string, AnyValue> varTable;

 public:
  Scope() : varTable() {}
  AnyValue& operator[](const std::string& s) { return varTable[s]; }
  void varRegister(const std::string& varName, const AnyValue& varData) { varTable[varName] = varData; }

  // std::pair<bool, AnyValue> varQuery(const std::string& varName) const {
  //   auto it = varTable.find(varName);
  //   if (it == varTable.end()) return std::make_pair(false, AnyValue());
  //   return std::make_pair(true, it->second);
  // }
  bool hasVar(const std::string& varName) const { return varTable.count(varName); }
};

class GlobalScope : Scope {
  friend ScopeStack;
  std::unordered_map<std::string, Func> funcTable;  // TODO
 public:
  void funcRegister(const std::string& funcName, const Func& fun) {
    //     std::string funcName = ctx->NAME()->getText();
    // #ifdef DEBUG
    //     std::cout << "Register function: " << funcName;
    // #endif  // DEBUG
    //     funcTable.emplace(funcName, Func(ctx));
    // #ifdef DEBUG
    //     std::cout << " succeeded" << std::endl;
    // #endif  // DEBUG
    funcTable.emplace(funcName, fun);
  }
  Func& funcQuery(const std::string& funcName) {
    auto it = funcTable.find(funcName);
    if (it == funcTable.end()) {
      throw Exception(NameError, funcName);
      // return Func();
    }
    return funcTable[funcName];
  }
};

// enum Status { GLOBAL, WHILE, FUNCTION };

class ScopeStack {
  GlobalScope global;
  std::vector<Scope> scopes;
 public:
  ScopeStack() {}

  void varRegister(const std::string& varName, const AnyValue& varData) {
    // std::cout << "varName: " << varName << " varData: " << varData << std::endl;
    if (scopes.empty()) {
      global.varRegister(varName, varData);
      return;
    }

    if (scopes.back().hasVar(varName)) {  // in current scope
      scopes.back().varRegister(varName, varData);
      return;
    }
    if (global.hasVar(varName)) {  // in global scope
      global.varRegister(varName, varData);
      return;
    }
    scopes.back().varRegister(varName, varData);  // create
  }

  AnyValue& varQuery(const std::string& varName) {
    // auto result = scopes.back().varQuery(varName);

    if (!scopes.empty()) {
      auto it = scopes.back().varTable.find(varName);
      if (it != scopes.back().varTable.end()) {
        return it->second;
      }
    }
    auto it = global.varTable.find(varName);
    if (it != global.varTable.end()) {
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
    return None;
  }
  void funcRegister(const std::string funcName, const Func& fun) { global.funcRegister(funcName, fun); }
  Func& funcQuery(const std::string& funcName) {
    // for (auto it = scopes.rbegin(); it != scopes.rend(); it++) {
    //   auto [success, val] = it->funcQuery(funcName);
    //   if (success) return std::make_pair(true, val);
    // }
    // return std::make_pair(false, nullptr);
    return global.funcQuery(funcName);
  }
  void enterFunc(const Scope& scope) {
    // std::cout << "enterFunc" << std::endl;
    scopes.push_back(scope);
  }
  void quitFunc() {
    scopes.pop_back();
  }
};

#endif  // APPLE_PIE_SCOPE_H