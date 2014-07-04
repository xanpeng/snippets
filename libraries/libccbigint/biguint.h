#ifndef BIGUINT_H
#define BIGUINT_H

#include <algorithm>
#include <ostream>
#include <string>
#include <vector>
#include <memory>

class BigUint {
public:
  // construct zero
  BigUint();
  BigUint(const BigUint &x);
  void operator=(const BigUint &x);
  ~BigUint(); 

  BigUint(const std::string &s);
  BigUint(unsigned long  x);
  BigUint(         long  x);
  BigUint(unsigned int   x);
  BigUint(         int   x);
  BigUint(unsigned short x);
  BigUint(         short x);

  unsigned long ToUnsignedLong() const;
  long ToLong() const;
  unsigned int ToUnsignedInt() const;
  int ToInt() const;
  unsigned short ToUnsignedShort() const;
  short ToShort() const;

  bool operator==(const BigUint &x) const;
  bool operator!=(const BigUint &x) const;
  bool operator<(const BigUint &x) const;
  bool operator<=(const BigUint &x) const;
  bool operator>(const BigUint &x) const;
  bool operator>=(const BigUint &x) const;

  BigUint operator+(const BigUint &x) const;
  BigUint operator-(const BigUint &x) const;
  BigUint operator*(const BigUint &x) const;
  BigUint operator/(const BigUint &x) const;
  BigUint operator%(const BigUint &x) const;

  /*
  BigUint operator&(const BigUint &x) const;
  BigUint operator|(const BigUint &x) const;
  BigUint operator^(const BigUint &x) const;
  BigUint operator<<(int b) const;
  BigUint operator>>(int b) const;
  */

  // http://www.parashift.com/c++-faq-lite/increment-pre-post-overloading.html
  // here do not return *this,so prefix and postfix behave the same.
  void operator++();
  void operator++(int);
  void operator--();
  void operator--(int);

  void operator+=(const BigUint &x) const;
  void operator-=(const BigUint &x) const;
  void operator*=(const BigUint &x) const;
  void operator/=(const BigUint &x) const;
  void operator%=(const BigUint &x) const;

  /*
  void operator&=(const BigUint &x) const;
  void operator|=(const BigUint &x) const;
  void operator^=(const BigUint &x) const;
  void operator<<=(const BigUint &x) const;
  void operator>>=(const BigUint &x) const;
  */

  friend std::ostream &operator<<(std::ostream &os, const BigUint &x);

private:
  void ConstructFromString(const std::string &s);
  std::string ToString() const;

  std::unique_ptr<std::vector<char> >data_;
};

std::ostream &operator<<(std::ostream &os, const BigUint &x);

#include "biguint-inl.h"

#endif

