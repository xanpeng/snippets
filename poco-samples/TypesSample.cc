/// g++ TypesSample.cc -o TypesSample
#include <Poco/Types.h>
#include <iostream>

int main() {
#if defined(__LP64__)
    std::cout << "__LP64__ defined" << std::endl;       // __LP64__ defined, see more at http://xanpeng.blogspot.com/2013/09/sizet.html
#endif
    std::cout << POCO_PTR_IS_64_BIT << std::endl;       // 1
    std::cout << POCO_LONG_IS_64_BIT << std::endl;      // 1
    std::cout << sizeof(Poco::UIntPtr) << std::endl;    // 8
    std::cout << sizeof(unsigned long) << std::endl;    // 8
    std::cout << sizeof(unsigned long long) << std::endl;   // 8

    return 0;
}
