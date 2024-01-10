#include <functional>
#include <iostream>

// 一个普通的函数
void greeting(const std::string& name, const std::string& greeting_msg) {
    std::cout << greeting_msg << ", " << name << "!" << std::endl;
}

// 一个简单的类
class MyClass {
public:
    void printMessage(const std::string& message) {
        std::cout << "Message from MyClass: " << message << std::endl;
    }
};

int main() {
    // 使用 std::bind 绑定普通函数
    auto sayHello = std::bind(greeting, std::placeholders::_1, "Hello");
    sayHello("John");

    // 使用 std::bind 绑定成员函数
    MyClass obj;
    auto printMsg = std::bind(&MyClass::printMessage, &obj, std::placeholders::_1);
    printMsg("Greetings");

    return 0;
}
