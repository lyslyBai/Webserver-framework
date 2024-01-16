#include <iostream>
#include <string>
#include <cstdarg>

#include <iostream>
#include <cstdarg>

void exampleFunction(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    // 在这里可以使用 args 访问可变参数列表中的参数
    int intValue = va_arg(args, int);
    int intValue1 = va_arg(args, int);
    int intValue2 = va_arg(args, int);
    double doubleValue = va_arg(args, double);
    const char* stringValue = va_arg(args, const char*);

    // std::cout << << std::endl;
    // 打印示例
    std::cout << "Int Value: " << intValue << std::endl;
    std::cout << "Int Value: " << intValue1 << std::endl;
    std::cout << "Int Value: " << intValue2 << std::endl;
    std::cout << "Double Value: " << doubleValue << std::endl;
    std::cout << "String Value: " << stringValue << std::endl;

    va_end(args);
}

int main() {
    exampleFunction("%d %d %d %f %s", 42,23,14, 3.14, "Hello, World!");

    return 0;
}


// int sum(int count, ...) {
//     va_list args;
//     // 初始化可变参数列表
//     va_start(args, count);

//     int result = 0;
//     for (int i = 0; i < count; ++i) {
//         result += va_arg(args, int);
//     }

//     // 清理可变参数列表
//     va_end(args);

//     return result;
// }

// int main() {
//     // 调用可变参数函数
//     int result = sum(3, 10, 20, 30);

//     // 输出结果
//     std::cout << "Sum: " << result << std::endl;

//     return 0;
// }


// // 模板函数，接受任意数量的参数并打印它们
// template<typename... Args>
// void printArgs(Args&&... args) {
//     // 使用折叠表达式展开参数包，并打印每个参数, -std=c++17
//     (std::cout << ... << args) << std::endl;
// }

// int main() {
//     printArgs("Hello", " ", "World");  // 输出：Hello World
//     printArgs(42, " is the answer.");  // 输出：42 is the answer.
    
//     std::string name = "John";
//     printArgs("My name is ", name, ".");  // 输出：My name is John.

//     return 0;
// }
