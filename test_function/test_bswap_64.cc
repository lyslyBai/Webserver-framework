#include <iostream>
#include <cstdint>
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

/*以 32 位整数 0x12345678 为例：

在小端序系统中，内存中的存储顺序是 78 56 34 12，即最低有效字节 0x78 存储在较低地址，而最高有效字节 0x12 存储在较高地址。

在大端序 (Big-Endian) 系统中，存储顺序是 12 34 56 78，即最低有效字节 0x78 存储在较高地址，而最高有效字节 0x12 存储在较低地址*/



int main()
{
    uint64_t original_value = 0x1234567890abcdef;

    uint64_t new_value = swaptype(original_value);

    std::cout << std::hex << "original_value " << original_value  << std::endl;
    std::cout << std::hex << "new_value " << new_value  << std::endl;

    return 0;
}