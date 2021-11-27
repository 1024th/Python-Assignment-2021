#ifndef APPLE_PIE_EXCEPTION_H
#define APPLE_PIE_EXCEPTION_H

#include <string>

enum ExceptionType {
  //   UNDEFINED,
  //   UNIMPLEMENTED,
  //   INVALID_VARNAME,
  //   INVALID_FUNC_CALL,
  TypeError,
  NameError,
  UndefinedBehavior
};

class Exception {
 private:
  std::string message;

 public:
  Exception(ExceptionType type, const std::string& arg = "") {
    switch (type) {
      case TypeError:
        message = "TypeError: " + arg;
        break;
      case NameError:
        message = "NameError: name '" + arg + "' is not defined";
        break;
      case UndefinedBehavior:
        message = "An undefined behavior occurred";
        break;
      //   case INVALID_FUNC_CALL:
      //     message = "Invalid function call: " + arg;
      //     break;
      default:
        break;
    }
#ifdef DEBUG
    std::cout << "Exception message is: " << message << std::endl;
#endif  // DEBUG
    // if (type == UNIMPLEMENTED)
    //   message = "Sorry, Apple Pie do not implement this.";
    // else if (type == UNDEFINED)
    //   message = "Undefined Variable: " + arg;
    // else if (type == INVALID_FUNC_CALL)
    //   message = "Invalid function call: " + arg;
  }

  std::string what() { return message; }
};

#endif  // APPLE_PIE_EXCEPTION_H