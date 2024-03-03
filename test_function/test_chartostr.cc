#include <iostream>


std::string chartostr(std::string str) {
    return str;
}



int main() {

    const char* ads= "abcdefg";

    std::cout << std::string(ads+1) << std::endl;

    std::cout << chartostr(ads+1) << std::endl;
    
    // char* aads= "abcdefg"; // 
    // 这是不允许的 现代C++标准中，字符串字面值是const char[]类型的

    // std::cout << std::string(aads+1) << std::endl;

    // std::cout << chartostr(aads+1) << std::endl;

    return 0;
}