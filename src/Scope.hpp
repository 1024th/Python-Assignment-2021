#ifndef APPLE_PIE_SCOPE_H
#define APPLE_PIE_SCOPE_H

#include <map>
#include <string>

#include "AnyType.hpp"

class Scope {
 private:
  std::map<std::string, AnyValue> varTable; // TODO: stack
  // std::map<std::string, Func> funcTable;  // TODO

 public:
  Scope() : varTable() {}
  void varRegister(const std::string& varName, AnyValue varData) {
    varTable[varName] = varData;
  }

  std::pair<bool, AnyValue> varQuery(const std::string& varName) const {
    auto it = varTable.find(varName);
    if (it == varTable.end()) return std::make_pair(false, AnyValue());
    return std::make_pair(true, it->second);
  }
};

#endif  // APPLE_PIE_SCOPE_H