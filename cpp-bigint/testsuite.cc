#include "biguint.h"

#include <string>
#include <iostream>
using namespace std;

// Evaluate expr and print the result or "error" as appropriate.
#define TEST(expr) do {\
  cout << "Line " << __LINE__ << ": ";\
  try {\
    cout << (expr);\
  } catch (const char *err) {\
    cout << "error";\
  }\
  cout << endl;\
} while (0)

int main() {
  BigUint z(0), one(1), ten(10);
  TEST(z); //0
  TEST(1); //1
  TEST(10); //10

  // === Default constructors ===
  TEST(BigUint()); //0

  // === BigUint conversion limits ===
  TEST(BigUint(0).ToUnsignedLong()); //0
  TEST(BigUint(4294967295U).ToUnsignedLong()); //4294967295
  TEST(BigUint("4294967296").ToUnsignedLong()); //

  TEST(BigUint(0).ToLong()); //0
  TEST(BigUint(2147483647).ToLong()); //2147483647
  TEST(BigUint(2147483648U).ToLong()); //

  TEST(BigUint(0).ToUnsignedInt()); //0
  // TEST(BigUint(4294967295U).ToUnsignedInt()); //4294967295
  // TEST(BigUint("4294967296").ToUnsignedInt()); //

  TEST(BigUint(0).ToInt()); //0
  TEST(BigUint(2147483647).ToInt()); //2147483647
  // TEST(BigUint(2147483648U).ToInt()); //

  TEST(BigUint(0).ToUnsignedShort()); //0
  TEST(BigUint(65535).ToUnsignedShort()); //65535
  TEST(BigUint(65536).ToUnsignedShort()); //

  TEST(BigUint(0).ToShort()); //0
  TEST(BigUint(32767).ToShort()); //32767
  TEST(BigUint(32768).ToShort()); //

  // ...during subtraction
  TEST(BigUint(5) - BigUint(6)); //error
  TEST(BigUint("314159265358979323") - BigUint("314159265358979324")); //error
  TEST(BigUint(5) - BigUint(5)); //0
  TEST(BigUint("314159265358979323") - BigUint("314159265358979323")); //0
  TEST(BigUint("4294967296") - BigUint(1)); //4294967295

  // === BigUint addition ===
  TEST(BigUint(0) + 0); //0
  TEST(BigUint(0) + 1); //1
  // Ordinary carry
  TEST(BigUint("8589934591" /* 2^33 - 1*/)
      + BigUint("4294967298" /* 2^32 + 2 */)); //12884901889
  // Creation of a new block
  TEST(BigUint(0xFFFFFFFFU) + 1); //4294967296

  // === BigUint subtraction ===

  TEST(BigUint(1) - 0); //1
  TEST(BigUint(1) - 1); //0
  TEST(BigUint(2) - 1); //1
  // Ordinary borrow
  TEST(BigUint("12884901889")
      - BigUint("4294967298")); //8589934591
  // Borrow that removes a block
  TEST(BigUint("4294967296") - 1); //4294967295

  // === BigUint multiplication and division ===

  BigUint a = BigUint(314159265) * 358979323;
  TEST(a); //112776680263877595
  TEST(a / 123); //916883579381118
  TEST(a % 123); //81

  TEST(BigUint(5) / 0); //error

  // === Combining BigUint, BigInteger, and primitive integers ===

  BigUint p1 = BigUint(3) * 5;
  TEST(p1); //15
  return 0;
}
