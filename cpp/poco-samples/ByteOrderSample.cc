/// g++ ByteOrderSample.cc
#include <Poco/ByteOrder.h>
#include <iostream>
#include <stdio.h>

using Poco::ByteOrder;
using Poco::Int64;
using Poco::Int32;
using Poco::UInt32;

void Flip() {
    {
        Int64 norm = (Int64(0x8899AABB) << 32) + 0xCCDDEEFF;
        Int64 flip = ByteOrder::flipBytes(norm);
        std::cout << (flip == (Int64(0xFFEEDDCC) << 32) + 0xBBAA9988) << std::endl; // 1
        flip = ByteOrder::flipBytes(flip);
        std::cout << (flip == norm) << std::endl;               // 1
    }
}

void CheckEndianess() {
    UInt32 intval = 0xAABBCCDD;
    unsigned char* charval = (unsigned char*)(&intval); 
    printf("%X\n", intval);                                                     // AABBCCDD
    printf("%X, %X, %X, %X\n", charval[0], charval[1], charval[2], charval[3]); // DD, BB, CC, AA
}

int main() {

    // Cannot compile, as POCO_ARCH_BIG_ENDIAN not defined
    // std::cout << "POCO_ARCH_BIG_ENDIAN: " << POCO_ARCH_BIG_ENDIAN << std::endl;
    std::cout << "POCO_ARCH_LITTLE_ENDIAN: " << POCO_ARCH_LITTLE_ENDIAN << std::endl;   // 1
    CheckEndianess();

    Flip();

    Int32 a = 0xAABBCCDD;
    std::cout << std::hex;
    std::cout << ByteOrder::toLittleEndian(a) << std::endl; // aabbccdd
    std::cout << ByteOrder::toBigEndian(a) << std::endl;    // ddccbbaa
    UInt32 b = 0xAABBCCDD;
    std::cout << ByteOrder::toLittleEndian(b) << std::endl; // aabbccdd
    std::cout << ByteOrder::toBigEndian(b) << std::endl;    // ddccbbaa

    return 0;
}
