
// #include <string>
// #include <string.h>
// #include <iostream>

// void test(const std::string& path) {
    
//     int length = path.size() +1;

//     std::cout << "before length " << length 
//               << "path.empty()"  << path.empty() << std::endl;
    
//     if(!path.empty() && path[0] == '\0') {
//         length--;
//     }

//     std::cout << "after length " << length << std::endl;

//     char str[1024];
//     memset(&str,0,sizeof(str));

//     memcpy(&str,&path,length);

//     std::cout << "str:" << str <<std::endl;
// }


// int main(){

//     const char* path_1 = "abcd";
//     test(path_1);

//     const char* path_2 = "";
//     test(path_2);

//     std::string str("");
//     test(str);

//     return 0;
// }

#include <iostream>
#include <string>

int main() {
    const char* path = "da";
    
    // 转换为 std::string
    std::string str(path);

    // 输出字符串长度和第一个字符
    std::cout << "String length: " << str.length() << std::endl;
    std::cout << "First character: " << (int)(str[2] == '\0') << std::endl;
    std::cout << "str.empty(): " << str.empty() << std::endl;

    return 0;
}
