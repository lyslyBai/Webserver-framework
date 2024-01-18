#include <iostream>
#include <cstddef>

struct ExampleStruct {
    int a;
    char b;
    double c;
};

// 返回结构体中每个元素的偏移量，
// 也就是当前元素的起始位置相对于结构体的起始位置

int main() {
    std::cout << "Offset of 'a': " << offsetof(ExampleStruct, a) << " bytes\n";
    std::cout << "Offset of 'b': " << offsetof(ExampleStruct, b) << " bytes\n";
    std::cout << "Offset of 'c': " << offsetof(ExampleStruct, c) << " bytes\n";

    return 0;
}
