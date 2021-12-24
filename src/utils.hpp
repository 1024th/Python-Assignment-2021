#ifndef APPLE_PIE_UTILS_H
#define APPLE_PIE_UTILS_H

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// int stringToInt(const std::string &str) {
//   int ret = 0;
//   for (const auto &ch : str) {
//     ret = ret * 10 + ch - '0';
//   }
//   return ret;
// }

bool validateVarName(const std::string &str) {
  for (const auto &ch : str) {
    if (!(ch >= 'a' && ch <= 'z') && !(ch >= 'A' && ch <= 'Z')) {
      return false;
    }
  }
  return true;
}

const int power = 9;  // 压的位数
const long long base = 1000000000;  // 10 的 power 次方
const int pow10[9] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000};

class BigInt {
  std::vector<long long> value;
  int highest;
  bool positive;

 public:
  BigInt() {
    value.push_back(0);
    highest = 0;
    positive = true;
  }
  BigInt(long long x) {
    highest = 0;
    positive = true;
    if (x < 0) positive = false, x = 0ULL - x;
    while (true) {
      value.push_back(x % base);
      x /= base;
      if (x)
        ++highest;
      else
        break;
    }
  }
  BigInt(bool x) : BigInt(x ? 1LL : 0LL) {}  // 委托构造函数
  static std::string intPart(std::string x) {
    auto pos = x.find('.');
    if (pos != std::string::npos) {
      return x.substr(0, pos);
    } else {
      return x;
    }
  }
  BigInt(double x) : BigInt(intPart(std::to_string(x))) {}
  BigInt(const std::string &s) { this->read(s); }
  BigInt(const char *s) : BigInt(std::string(s)) {}
  BigInt(const BigInt &x) : value(x.value), highest(x.highest), positive(x.positive) {}
  BigInt(const BigInt &&x) : value(x.value), highest(x.highest), positive(x.positive) {}

  inline void shrink() {
    while (value[highest] == 0 && highest) {
      --highest;
    }
    // value.resize(highest + 1);
  }

  // 读入一个大整数
  void read(const std::string &s) {
    highest = 0;
    positive = true;
    value.clear();
    int str_len = s.size() - 1, maxi = str_len;
    if (str_len == -1) return;
    if (s[0] == '-') positive = false, --maxi;
    value.resize(str_len / power + 1);
    for (int i = 0, k = 1; i <= maxi; i++, k *= 10) {
      if (k == base) {
        ++highest, k = 1;
      }
      value[highest] += k * (s[str_len - i] - '0');
    }
    shrink();
    if (highest == 0 && value[0] == 0) positive = true;
  }
  // 输出储存的大整数
  void print() {
    if (!positive) printf("-");
    printf("%d", value[highest]);
    for (int i = highest - 1; i >= 0; --i) {
      printf("%09d", value[i]);
    }
  }

  friend BigInt operator-(const BigInt &x) {
    if (x == 0LL) return x;
    BigInt y(x);
    y.positive = !y.positive;
    return y;
  }
  BigInt &direct_add(const BigInt &x) {
    highest = std::max(highest, x.highest);
    value.resize(highest + 2);
    // while (value.size() <= highest + 1) value.push_back(0);
    long long carry = 0, i;
    for (i = 0; i <= std::min(highest, x.highest); i++) {
      value[i] += x.value[i] + carry;
      carry = 0;
      if (value[i] >= base) {
        carry = value[i] / base;
        value[i] %= base;
      }
    }
    if (carry) {
      value[i] += carry;
      ++highest;
    }
    shrink();
    return *this;
  }
  BigInt &add(const BigInt &x) {
    if (positive) {
      if (x.positive) {
        return this->direct_add(x);
      } else {
        return this->direct_minus(x);
      }
    } else {
      if (x.positive) {
        this->direct_minus(x);
        this->positive = !(this->positive);
        return *this;
      } else {
        return this->direct_add(x);
      }
    }
  }
  // 输出两个大整数之和
  friend BigInt add(BigInt a, const BigInt &b) { return a.add(b); }
  // 比较绝对值
  bool abs_greater_than(const BigInt &x) const {
    if (highest > x.highest) {
      return true;
    } else if (highest < x.highest) {
      return false;
    } else {
      for (int i = highest; i >= 0; --i) {
        if (value[i] > x.value[i])
          return true;
        else if (value[i] < x.value[i])
          return false;
      }
      return false;  // equal
    }
  }
  inline bool abs_greater_than(const BigInt &x, int low) {
    int len = x.highest - low;
    if (highest > len) {
      return true;
    } else if (highest < len) {
      return false;
    } else {
      for (int i = highest, j = highest + low; i >= 0; --i, --j) {
        if (value[i] > x.value[j])
          return true;
        else if (value[i] < x.value[j])
          return false;
      }
      return false;  // equal
    }
  }
  // 比较绝对值
  bool abs_less_than(const BigInt &x) const {
    if (highest < x.highest) {
      return true;
    } else if (highest > x.highest) {
      return false;
    } else {
      for (int i = highest; i >= 0; --i) {
        if (value[i] < x.value[i])
          return true;
        else if (value[i] > x.value[i])
          return false;
      }
      return false;  // equal
    }
  }
  // 无视符号，绝对值相减
  BigInt &direct_minus(const BigInt &x) {
    if (this->abs_less_than(x)) {
      this->positive = false;
      highest = x.highest;
      value.resize(highest + 1);

      for (int i = 0, borrow = 0; i <= x.highest; i++) {
        value[i] = x.value[i] - value[i] - borrow;
        borrow = 0;
        if (value[i] < 0) {
          value[i] += base;
          borrow++;
        }
      }
      shrink();
      return *this;
    } else {
      this->positive = true;
      long long borrow = 0;
      for (int i = 0; i <= x.highest; i++) {
        value[i] -= x.value[i] + borrow;
        borrow = 0;
        if (value[i] < 0) {
          value[i] += base;
          borrow++;
        }
      }
      for (int i = x.highest + 1; i <= highest; i++) {
        value[i] -= borrow;
        borrow = 0;
        if (value[i] < 0) {
          value[i] += base;
          borrow++;
        }
      }
      shrink();
      return *this;
    }
  }
  // 减去一个大整数
  BigInt &minus(const BigInt &x) {
    if (positive) {
      if (x.positive) {
        return this->direct_minus(x);
      } else {
        return this->direct_add(x);
      }
    } else {
      if (x.positive) {
        return this->direct_add(x);
      } else {
        this->direct_minus(x);
        if (highest != 0 || value[0] != 0) this->positive = !(this->positive);
        return *this;
      }
    }
  }
  // 输出两个大整数之差
  friend BigInt minus(BigInt a, const BigInt &b) { return a.minus(b); }

  // =================================== Integer2
  // ===================================

  BigInt &operator=(const BigInt &x) {
    highest = x.highest, positive = x.positive;
    value = x.value;
    return *this;
  }

  BigInt &operator+=(BigInt x) { return this->add(x); }
  friend BigInt operator+(BigInt a, const BigInt &b) { return a.add(b); }

  BigInt &operator-=(BigInt x) { return this->minus(x); }
  friend BigInt operator-(BigInt a, const BigInt &b) { return a.minus(b); }

  BigInt &operator*=(const BigInt &x) { return *this = (*this) * x; }
  friend BigInt operator*(const BigInt &a, const BigInt &b) {
    BigInt ans;
    ans.highest = a.highest + b.highest + 1;
    ans.value.resize(ans.highest + 1);
    for (int i = 0; i <= ans.highest; ++i) {
      for (int j = std::max(0, i - b.highest); j <= std::min(i, a.highest); ++j) {
        ans.value[i] += a.value[j] * b.value[i - j];
        ans.value[i + 1] += ans.value[i] / base;
        ans.value[i] %= base;
      }
    }
    ans.shrink();
    ans.positive = a.positive == b.positive;
    if (ans.highest == 0 && ans.value[0] == 0) ans.positive = true;
    return ans;
  }
  BigInt &no_expand_abs_multiply(const BigInt &a, long long b) {
    // TODO:!!
    // a.highest += 1;
    // a.value.resize(a.highest + 1);

    long long carrier = 0;
    for (int i = 0; i <= a.highest; ++i) {
      value[i] = a.value[i] * b + carrier;
      carrier = value[i] / base;
      value[i] %= base;
    }
    if (carrier) {
      highest = a.highest + 1;
      value[highest] = carrier;
    } else {
      highest = a.highest;
    }
    return *this;
  }
  void sub_minus(int low, const BigInt &x) {
    for (int i = 0, j = low; i <= x.highest; ++i, ++j) {
      value[j] -= x.value[i];
      if (value[j] < 0) {
        value[j] += base;
        value[j + 1]--;  // TODO: !!?
      }
    }
    while (highest && value[highest] == 0) {
      --highest;
      // value.pop_back();
    }
    // value.resize(highest + 1);
  }

  BigInt &operator/=(const BigInt &x) { return *this = (*this) / x; }
  friend BigInt operator/(BigInt a, const BigInt &b) {
    BigInt ans, tmp_product;
    if (a.highest < b.highest) return ans;
    ans.highest = a.highest - b.highest;
    ans.value.resize(ans.highest + 1);
    // while (ans.value.size() <= ans.highest) ans.value.push_back(0);
    tmp_product.highest = b.highest + 1;
    tmp_product.value.resize(tmp_product.highest + 1);

    long long l, r = base - 1, tmp;
    for (int i1 = a.highest, i2 = a.highest - b.highest; i2 >= 0; --i1, --i2) {
      // l = 0;
      // r = base - 1;
      tmp = a.value[i1];
      if (a.highest > i1) tmp += a.value[a.highest] * base;
      l = tmp / (b.value[b.highest] + 1);
      r = (tmp) / b.value[b.highest];
      while (l < r) {
        long long mid = (l + r + 1) >> 1;
        if (tmp_product.no_expand_abs_multiply(b, mid).abs_greater_than(a, i2))
          r = mid - 1;
        else
          l = mid;
      }
      a.sub_minus(i2, tmp_product.no_expand_abs_multiply(b, l));
      ans.value[i2] = l;
    }
    ans.shrink();
    ans.value.resize(ans.highest + 1);
    ans.positive = a.positive == b.positive;
    if (ans.highest == 0 && ans.value[0] == 0) ans.positive = true;
    return ans;
  }
  friend BigInt intDiv(const BigInt &a, const BigInt &b) {
    BigInt ans = a / b;
    // cannot use !ans.positive, because when a/b = -0.5,
    // ans = 0
    // cannot use ans <= 0LL, because when a/b = 0.5,
    // ans = 0 <= OLL
    if (a.positive != b.positive && (ans * b != a)) ans -= 1LL;
    return ans;
  }

  friend BigInt operator%(const BigInt &a, const BigInt &b) { return a - intDiv(a, b) * b; }

  friend std::istream &operator>>(std::istream &is, BigInt &x) {
    std::string s;
    is >> s;
    x.read(s);
    return is;
  }
  friend std::ostream &operator<<(std::ostream &os, const BigInt &x) {
    // char out[power + 1];
    if (!x.positive) os << "-";
    os << x.value[x.highest];
    for (int i = x.highest - 1, tmp; i >= 0; --i) {
      // sprintf(out, "%09d", x.value[i]);
      // os << out;
      tmp = x.value[i];
      for (int j = power - 1; j >= 0; --j) {
        os << tmp / pow10[j];
        tmp %= pow10[j];
      }
    }
    return os;
  }
  explicit operator std::string() const {
    std::ostringstream oss;
    oss << (*this);
    return oss.str();
  }
  explicit operator double() const { return std::stod(std::string(*this)); }
  explicit operator bool() const { return (*this) != BigInt(); }

  inline friend bool operator==(const BigInt &a, const BigInt &b) {
    if (a.positive != b.positive) return false;
    if (a.highest != b.highest) {
      return false;
    } else {
      for (int i = 0; i <= a.highest; ++i) {
        if (a.value[i] != b.value[i]) return false;
      }
      return true;
    }
  }
  inline friend bool operator!=(const BigInt &a, const BigInt &b) { return !(a == b); }
  inline friend bool operator<(const BigInt &a, const BigInt &b) {
    if (a.positive) {
      if (b.positive)
        return a.abs_less_than(b);
      else
        return false;
    } else {
      if (b.positive)
        return true;
      else
        return a.abs_greater_than(b);
    }
  }
  inline friend bool operator>(const BigInt &a, const BigInt &b) {
    if (a.positive) {
      if (b.positive)
        return a.abs_greater_than(b);
      else
        return true;
    } else {
      if (b.positive)
        return false;
      else
        return a.abs_less_than(b);
    }
  }
  inline friend bool operator<=(const BigInt &a, const BigInt &b) { return !(a > b); }
  inline friend bool operator>=(const BigInt &a, const BigInt &b) { return !(a < b); }
};

std::string repeatString(const std::string &str, BigInt n) {
  std::string ans;
  BigInt one(1LL);
  for (BigInt i = 0LL; i < n; i += one) {
    ans += str;
  }
  return ans;
}

#endif  // APPLE_PIE_UTILS_H