#include <cstdint>
#include <iostream>
#include <byteswap.h>


template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t),T>::type
swaptype(T t) {
    return static_cast<T>(bswap_64(static_cast<uint64_t>(t)));
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t),T>::type
swaptype(T t) {
    return static_cast<T>(bswap_32(static_cast<uint32_t>(t)));
}

template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t),T>::type
swaptype(T t) {
    return static_cast<T>(bswap_16(static_cast<uint16_t>(t)));
}

template<class T>
static T CreateMask(uint32_t bits) {
    // 1 << 3 表示1 想左移3位，得到1000，减一，得到0111.
    return static_cast<T>((1 << (sizeof(T) * 8 - bits)) - 1);
}

void test_net(uint32_t prefix_len){
    if(prefix_len > 32) {
        return;
    }
    uint32_t b = 0x12345678;
    std::cout << std::hex << "before swap b=" << b << std::endl;
    // uint32_t a = swaptype(b);
    // std::cout << std::hex << "after swap a=" << a << std::endl;
    uint32_t c = CreateMask<uint32_t>(prefix_len);
    std::cout << std::hex << "after swap CreateMask c=" << c << std::endl;

    uint32_t d = c&b;
    std::cout << std::hex << "after swap CreateMask & d=" << d << std::endl;
}

int main() {

    test_net(5);

    return 0;
}

// int main() {
//     uint32_t mask32 = CreateMask<uint32_t>(5);
//     uint64_t mask64 = CreateMask<uint64_t>(10);

//     std::cout << " mask32:" << mask32 << " mask64:" << mask64 << std::endl;
    
//     int a = 5;  // 二进制表示：0000 0101
//     int b = 3;  // 二进制表示：0000 0011
//     a |= b;     // 执行按位或赋值操作
//     // 结果：0000 0111，即 7
    
//     a = 5;  // 二进制表示：0000 0101
//     b = 3;  // 二进制表示：0000 0011
//     a &= b;     // 执行按位与赋值操作
//     // 结果：0000 0001，即 1

    
//     return 0;
// }
