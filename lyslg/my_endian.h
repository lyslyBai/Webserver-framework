#ifndef __LYSLG_MY_ENDIAN_H__
#define __LYSLG_MY_ENDIAN_H__


#define LYSLG_LITTLE_ENDIAN 1
#define LYSLG_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>
#include <type_traits>

namespace lyslg{

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t),T>::type
byteswap(T value) {
    return (T)bswap_64((uint64_t)value);
}

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t),T>::type
byteswap(T value) {
    return (T)bswap_32((uint32_t)value);
}

template<class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t),T>::type
byteswap(T value) {
    return (T)bswap_16((uint16_t)value);
}
/*BYTE_ORDER 是一个宏，通常由系统头文件定义，
用于表示当前系统的字节序。在标准头文件 <endian.h> 中，
BYTE_ORDER 通常被定义为 BIG_ENDIAN 或 LITTLE_ENDIAN。本系统小端序*/
#if BYTE_ORDER == BIG_ENDIAN
#define LYSLG_BYTE_ORDER LYSLG_BIG_ENDIAN
#else
#define LYSLG_BYTE_ORDER LYSLG_LITTLE_ENDIAN
#endif

#if LYSLG_BYTE_ORDER == LYSLG_BIG_ENDIAN
template<class T>
T byteswapOnLittleEndian(T t){
    return t;
}

template<class T>
T byteswapOnBigEndian(T t){
    return byteswap(t);
}

#else
template<class T>
T byteswapOnLittleEndian(T t){
    return byteswap(t);
}

template<class T>
T byteswapOnBigEndian(T t){
    return t;
}
#endif

}


#endif