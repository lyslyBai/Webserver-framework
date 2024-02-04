#include <iostream>


static uint32_t EncodeZigzag32(const int32_t& v){
    if(v < 0) {
        return ((uint32_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

static int32_t DecodeZigzag32(const uint32_t& v){
    return(v >>1 ) ^ -(v & 1);
}


int main(){

    int32_t a = -5;

    std::cout << " original a=" << std::hex << a << std::dec << std::endl;

    uint32_t b = EncodeZigzag32(a);

    std::cout << " EncodeZigzag32(a) b=" << std::hex << b << std::dec << std::endl;

    int32_t c = DecodeZigzag32(b);

    std::cout << " DecodeZigzag32(b) c=" << std::hex << c << std::dec << std::endl;


    return 0;
}