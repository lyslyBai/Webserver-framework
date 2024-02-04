#include <iostream>

int main(){

    int a = -1;
    // 这里的a为补码形式
    std::cout << "a(-1) = " << std::hex << a << std::dec << std::endl;

    std::cout << "a(-1) = " << a << std::dec << std::endl;


    std::cout << "-a(-1) = " << std::hex << (-a) << std::dec << std::endl;


    return 0;
}