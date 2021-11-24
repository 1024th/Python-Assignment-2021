#include <string>
#include <vector>

#include "Exception.hpp"
// #include "Python3Parser.h"
#include "utils.hpp"

enum ValueType { BIGINT, BOOL, FLOAT, STR };

class AnyValue {
  ValueType type;
  void* value = nullptr;
  // static const std::string typeName[] = {"int", "bool", "float", "str"};

  static std::string typeName(ValueType type) {
    switch (type) {
      case BIGINT:
        return "int";
      case BOOL:
        return "bool";
      case FLOAT:
        return "float";
      case STR:
        return "str";
      default:
        throw Exception(TypeError, "unknown type!");
        break;
    }
  }

 public:
  AnyValue() : value(nullptr) {}
  template <class T>
  void constructFrom(T x, ValueType type_) {
    T* tmp = new T;
    *tmp = x;
    value = tmp;
    // std::cout << *tmp;
    type = type_;
  }
  // set value, type unchanged.
  template <class T>
  AnyValue& setValue(T x) {
    free();
    // if (value) delete value;  // TODO: typed pointer
    T* tmp = new T;
    *tmp = x;
    // std::cout << *tmp << std::endl;
    value = tmp;
    return *this;
  }
  AnyValue(const AnyValue& x) { *this = x; }
  AnyValue& operator=(const AnyValue& x) {
    if(this == &x) return *this;
    free();
    if (!x.value) {
      return *this;
    }
    switch (x.type) {
      case BIGINT:
        constructFrom(x.as<BigInt>(), x.type);
        break;
      case BOOL:
        constructFrom(x.as<bool>(), x.type);
        break;
      case FLOAT:
        constructFrom(x.as<double>(), x.type);
        break;
      case STR:
        constructFrom(x.as<std::string>(), x.type);
        break;
      default:
        throw Exception(TypeError, "unknown type!");
        break;
    }
    return *this;
  }
  AnyValue(BigInt x) { constructFrom<BigInt>(x, BIGINT); }
  AnyValue(bool x) { constructFrom<bool>(x, BOOL); }
  AnyValue(double x) { constructFrom<double>(x, FLOAT); }
  AnyValue(std::string x) { constructFrom<std::string>(x, STR); }
  void free() {
    // std::cout << "in free()" << std::endl;
    // std::cout << value << std::endl;
    // std::cout << "value -> bool: " << bool(value) << std::endl;

    // std::cout << "size: ";
    // ((BigInt*)value)->print();
    // std::cout << std::endl;
    if (!value) return;
    switch (type) {
      case BIGINT:
        delete (BigInt*)value;
        break;
      case BOOL:
        delete (bool*)value;
        break;
      case FLOAT:
        delete (double*)value;
        break;
      case STR:
        delete (std::string*)value;
        break;
      default:
        throw Exception(TypeError, "unknown type!");
        break;
    }
    value = nullptr;
  }
  ~AnyValue() {
    // std::cout << typeName(type) << std::endl;
    free();
  }

  template <typename T>
  T as() const {
    return *((T*)value);
  }
  template <typename T>
  static double convertToFloat(T x) {  // T: BigInt, double, bool
    return double(x);
  }
  static double convertToFloat(std::string x) { return std::stod(x); }
  bool toBool() const {
    switch (type) {
      case BIGINT:
        return bool(as<BigInt>());
      case BOOL:
        return as<bool>();
      case FLOAT:
        return bool(as<double>());
      case STR:
        return as<std::string>() == "";
      default:
        throw Exception(TypeError, "unknown type!");
    }
  }
  BigInt toBigInt() const {
    switch (type) {
      case BIGINT:
        return as<BigInt>();
      case BOOL:
        return BigInt(as<bool>());
      case FLOAT:
        return BigInt(as<double>());
      case STR:
        return BigInt(as<std::string>());
      default:
        throw Exception(TypeError, "unknown type!");
    }
  }
  double toDouble() const {
    switch (type) {
      case BIGINT:
        return double(as<BigInt>());
      case BOOL:
        if (as<bool>())
          return 1.0;
        else
          return 0.0;
      case FLOAT:
        return as<double>();
      case STR:
        return std::stod(as<std::string>());
      default:
        throw Exception(TypeError, "unknown type!");
    }
  }
  std::string toString() const {
    switch (type) {
      case BIGINT:
        return std::string(as<BigInt>());
      case BOOL:
        if (as<bool>())
          return "True";
        else
          return "False";
      case FLOAT:
        return std::to_string(as<double>());
      case STR:
        return as<std::string>();
      default:
        throw Exception(TypeError, "unknown type!");
    }
  }
  friend std::ostream& operator<<(std::ostream& os, const AnyValue& x) {
    os << x.toString();
    return os;
  }
  // template <typename T>
  // T to(ValueType toType) const {
  //   switch (type) {
  //     case BIGINT:
  //       return T(as<BigInt>());  // to: bool,double,str
  //       break;
  //     case BOOL:
  //       if (toType == STR) {
  //         if (as<bool>())
  //           return "True";
  //         else
  //           return "False";
  //       }
  //       return T(as<bool>());  // to: bigint,double
  //       break;
  //     case FLOAT:
  //       if (toType == STR) {
  //         return std::to_string(as<double>());
  //       }
  //       return T(as<double>());  // to: bool,bigint,double
  //     case STR:
  //       if (toType == FLOAT) {
  //         return std::stod(as<std::string>());
  //       }
  //       return T(as<std::string>());  // to: bool,bigint,str
  //     default:
  //       throw Exception(TypeError, "unknown type!");
  //   }
  // }
  explicit operator BigInt() const { return as<BigInt>(); }
  explicit operator bool() const { return as<bool>(); }
  explicit operator double() const { return as<double>(); }
  explicit operator std::string() const { return as<std::string>(); }
  friend AnyValue operator+(const AnyValue& a, const AnyValue& b) {
    AnyValue ans;
    if (a.type == STR && b.type == STR) {
      ans.type = STR;
      return ans.setValue(a.as<std::string>() + b.as<std::string>());
    }
    if (a.type == STR) throw Exception(TypeError, "can only concatenate str (not \"" + typeName(b.type) + "\") to str");
    if (b.type == STR) throw Exception(TypeError, "can only concatenate str (not \"" + typeName(a.type) + "\") to str");
    if (a.type == FLOAT || b.type == FLOAT) {
      ans.type = FLOAT;
      return ans.setValue(a.toDouble() + b.toDouble());
    }
    // then the type will only be BIGINT or BOOL, convert to BigInt
    ans.type = BIGINT;
    return ans.setValue(a.toBigInt() + b.toBigInt());
  }

  friend AnyValue operator-(const AnyValue& a, const AnyValue& b) {
    if (a.type == STR || b.type == STR) {
      throw Exception(TypeError,
                      "unsupported operand type(s) for -: '" + typeName(a.type) + "' and '" + typeName(b.type) + "'");
    }
    AnyValue ans;
    if (a.type == FLOAT || b.type == FLOAT) {
      ans.type = FLOAT;
      return ans.setValue(a.toDouble() - b.toDouble());
    }
    // then the type will only be BIGINT or BOOL, convert to BigInt
    ans.type = BIGINT;
    return ans.setValue(a.toBigInt() - b.toBigInt());
  }

  friend AnyValue operator*(const AnyValue& a, const AnyValue& b) {
    AnyValue ans;
    if (a.type == STR) {
      if (b.type == BIGINT) {
        ans.type = STR;
        ans.setValue(repeatString(a.as<std::string>(), b.as<BigInt>()));
      } else {
        throw(TypeError, "can't multiply sequence by non-int of type 'str'");
      }
    }
    if (b.type == STR) {
      if (a.type == BIGINT) {
        ans.type = STR;
        ans.setValue(repeatString(b.as<std::string>(), a.as<BigInt>()));
      } else {
        throw(TypeError, "can't multiply sequence by non-int of type 'str'");
      }
    }
    if (a.type == FLOAT || b.type == FLOAT) {
      ans.type = FLOAT;
      return ans.setValue(a.toDouble() * b.toDouble());
    }
    // then the type will only be BIGINT or BOOL, convert to BigInt
    ans.type = BIGINT;
    return ans.setValue(a.toBigInt() * b.toBigInt());
  }

  friend AnyValue operator/(const AnyValue& a, const AnyValue& b) {
    if (a.type == STR || b.type == STR) {
      throw Exception(TypeError,
                      "unsupported operand type(s) for /: '" + typeName(a.type) + "' and '" + typeName(b.type) + "'");
    }
    AnyValue ans;
    // convert all the types to FLOAT
    ans.type = FLOAT;
    return ans.setValue(a.toDouble() / b.toDouble());
  }

  friend AnyValue intDiv(const AnyValue& a, const AnyValue& b) {
    if (a.type == STR || b.type == STR)
      throw Exception(TypeError,
                      "unsupported operand type(s) for //: '" + typeName(a.type) + "' and '" + typeName(b.type) + "'");
    if (a.type == FLOAT || b.type == FLOAT) throw Exception(UndefinedBehavior);
    AnyValue ans;
    ans.type = BIGINT;
    // BigInt x = a.toBigInt(), y = b.toBigInt();
    // BigInt div = x / y;
    // if ((y * div).abs_less_than(x)) div -= 1LL;
    return ans.setValue(intDiv(a.toBigInt(), b.toBigInt()));
  }

  friend AnyValue operator%(const AnyValue& a, const AnyValue& b) {
    if (a.type == STR || b.type == STR)
      throw Exception(TypeError,
                      "unsupported operand type(s) for /: '" + typeName(a.type) + "' and '" + typeName(b.type) + "'");
    if (a.type == FLOAT || b.type == FLOAT) throw Exception(UndefinedBehavior);
    AnyValue ans;
    ans.type = BIGINT;
    return ans.setValue(a.toBigInt() % b.toBigInt());
  }
  AnyValue& operator+=(const AnyValue& x) { return *this = *this + x; }
  AnyValue& operator-=(const AnyValue& x) { return *this = *this - x; }
  AnyValue& operator*=(const AnyValue& x) { return *this = (*this) * x; }
  AnyValue& operator/=(const AnyValue& x) { return *this = *this / x; }
  AnyValue& operator%=(const AnyValue& x) { return *this = *this % x; }
  friend AnyValue operator-(const AnyValue& x) {
    switch (x.type) {
      case BIGINT:
        return AnyValue(-x.as<BigInt>());
      case BOOL:
        return AnyValue(-x.toBigInt());
      case FLOAT:
        return AnyValue(-x.as<double>());
      case STR:
        throw Exception(TypeError, "bad operand type for unary -: 'str'");
      default:
        throw Exception(TypeError, "unknown type!");
    }
  }
};
class AnyValueList {
  std::vector<AnyValue> list;

 public:
  ~AnyValueList() {
    for (auto& i : list) {
      i.free();
    }
  }
  AnyValue& operator[](int x) { return list[x]; }
  void push_back(const AnyValue& a) { list.push_back(a); }
  auto begin() { return list.begin(); }
  auto end() { return list.end(); }
  auto size() { return list.size(); }
};
// class Func {
//  public:
//   Python3Parser::ParametersContext* para;
//   Python3Parser::SuiteContext* suite;
//   // std::unordered_map<Python3Parser::ParametersContext*, std::vector<Base> >
//   //     defaultArguments;
//   // TODO: 执行函数
// };