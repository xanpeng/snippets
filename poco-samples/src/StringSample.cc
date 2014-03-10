/// g++ StringSample.cc -lPocoFoundation
#include <Poco/StringTokenizer.h>
#include <Poco/String.h>
#include <iostream>

using Poco::StringTokenizer;

void Trim() {
    std::string s = " abc ";
    std::cout << Poco::trimLeft(s) << std::endl;    // abc
    std::cout << Poco::trimRight(s) << std::endl;   //  abc
    std::cout << Poco::trim(s) << std::endl;        // abc
    // Same result when use trimLeftInPlace, ...
}
    
void ChangeCase() {
    std::string s = "Abc";
    std::cout << Poco::toUpper(s) << std::endl;     // ABC
    std::cout << Poco::toLower(s) << std::endl;     // abc
    // Same result when use toUpperInPlace, ...
}

int main() {

    std::string tokens = "white; black; magenta, blue, green; yellow";
    StringTokenizer tokenizer(tokens, ";,", StringTokenizer::TOK_TRIM);
    for (StringTokenizer::Iterator it = tokenizer.begin(); it != tokenizer.end(); ++it) {
        std::cout << *it << std::endl;
        // white
        // black
        // magenta
        // blue
        // green
        // yellow
    }

    Trim();
    ChangeCase();

    // compare
    // translate
    // replace
    // cat

    return 0;
}

