#ifndef BIGUINT_INL_H
#define BIGUINT_INL_H

#include <assert.h>
#include <ctype.h>
#include <iterator>

#include "biguint.h"

inline BigUint::BigUint() : data_(new std::vector<char>(1)){
  data_->push_back('0');
}

inline BigUint::BigUint(const BigUint &x)
    : data_(new std::vector<char>(x.data_->size())) {
  std::copy(x.data_->begin(), x.data_->end(), data_->begin());
}

inline void BigUint::operator=(const BigUint &x) {
  data_.reset(new std::vector<char>(x.data_->size()));
  std::copy(x.data_->begin(), x.data_->end(), data_->begin());
}

inline BigUint::~BigUint() {}

inline BigUint::BigUint(const std::string &s) {
  ConstructFromString(s);
}

inline BigUint::BigUint(unsigned long x) { ConstructFromString(std::to_string(x)); }
inline BigUint::BigUint(         long x) { ConstructFromString(std::to_string(x)); }
inline BigUint::BigUint(unsigned int x)  { ConstructFromString(std::to_string(x)); }
inline BigUint::BigUint(         int x)  { ConstructFromString(std::to_string(x)); }
inline BigUint::BigUint(unsigned short x) { ConstructFromString(std::to_string(x)); }
inline BigUint::BigUint(         short x) { ConstructFromString(std::to_string(x)); }

unsigned long BigUint::ToUnsignedLong() const { return std::stoul(ToString()); }
         long BigUint::ToLong()         const { return std::stol(ToString()); }
unsigned int BigUint::ToUnsignedInt()   const { return std::stoi(ToString()); } 
         int BigUint::ToInt()           const { return std::stoi(ToString()); }
unsigned short BigUint::ToUnsignedShort() const { return std::stoi(ToString()); }
         short BigUint::ToShort()       const { return std::stoi(ToString()); }

inline bool BigUint::operator==(const BigUint &x) const {
  return (data_->size() == x.data_->size()) && (*data_ == *(x.data_));
}
inline bool BigUint::operator!=(const BigUint &x) const {
  return !operator==(x); 
}
inline bool BigUint::operator<(const BigUint &x) const {
  return (data_->size() < x.data_->size()) ||
      (data_->size() == x.data_->size() && *data_ < *(x.data_));
}
inline bool BigUint::operator<=(const BigUint &x) const {
  return operator==(x) || operator<(x);
}
inline bool BigUint::operator>(const BigUint &x) const {
  return !operator<=(x);
}
inline bool BigUint::operator>=(const BigUint &x) const {
  return !operator<(x);
}

BigUint BigUint::operator+(const BigUint &x) const {
  // TODO
  return NULL;
}

BigUint BigUint::operator-(const BigUint &x) const {
  // TODO
  return NULL;
}

BigUint BigUint::operator*(const BigUint &x) const {
  // TODO
  return NULL;
}

BigUint BigUint::operator/(const BigUint &x) const {
  // TODO
  return NULL;
}

BigUint BigUint::operator%(const BigUint &x) const {
  // TODO
  return NULL;
}

inline void BigUint::ConstructFromString(const std::string &s) {
  data_.reset(new std::vector<char>(s.length()));
  std::copy(s.begin(), s.end(), data_->begin());

  // remove leading zeros
  for (std::vector<char>::iterator it = data_->begin(); it != std::prev(data_->end()); ) {
    if (*it != '0') break;
    it = data_->erase(it);
  }

  std::for_each(data_->begin(), data_->end(), [](char &c) { (void) c; assert(isdigit(c)); });
}

inline std::string BigUint::ToString() const {
  std::string str;
  std::for_each(data_->begin(), data_->end(), [&str](char &c){ str.push_back(c); });
  return str;
}

inline std::ostream &operator<<(std::ostream &os, const BigUint &x) {
  os << x.ToString();
  return os;
}

#endif

