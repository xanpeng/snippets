// snprintf()是整数转换为字符串的最常见方式，但对于简单的转换snprintf()显得大材小用
//
// C89和C++98的取整方式都留给实现去决定，C99和C++11都规定商向零取整。
// G++一直遵循C99规范，商向零取整，此下上述算法能正常工作。
// C99/C++11/Java/C#属于向零取整阵营，Python等脚本语言商是向负无穷取整（向下取整）。
// 下述算法在向零取整的情况下有效。
//
// 陈硕《Linux多线程服务端编程》12.3 “带符号整数的除法与余数”
// Efficient Integer to String Conversions, part 1, http://www.drdobbs.com/flexible-c-1-efficient-integer-to-string/184401596
// Efficient Integer to String Conversions, part 2, http://www.drdobbs.com/flexible-c-2-efficient-integer-to-string/184403874
// Efficient Integer to String Conversions, part 3, http://www.drdobbs.com/flexible-c-3-efficient-integer-to-string/184403880
// Efficient Integer to String Conversions, part 4, http://www.drdobbs.com/flexible-c-4-efficient-integer-to-strin/184403882
const char* convert(char buf[], int value) {
  static char digits[19] = {
    '9', '8', '7', '6', '5', '4', '3', '2', '1',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
  static const char* zero = digits + 9; // zero指向'0'

  // works for -2147483648 .. 2147483647
  int i = value;
  char* p = buf;
  do {
    // lsd - least significant digit
    int lsd = i % 10;   // lsd可能小于0
    i /= 10;            // 是向下取整还是向零取整？
    *p++ = zero[lsd];   // 下标可能是负
  } while (i != 0);

  if (value < 0) *p++ = '-';
  *p = '\0';
  std::reverse(buf, p);
  return p;   // p-buf为整数长度
}
