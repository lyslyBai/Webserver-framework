#include <iostream>
#include <memory>

// 模板偏特化的使用例子

template<class T, class Enable = void>
class A {
 public:
  A() { std::cout << "primary template\r\n"; }
}; // primary template

template<class T>  // 当 T 为int类型，这  typename std::enable_if<std::is_floating_point<T>::value>::type> 实例化成功，执行此模板
class A<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {  // 模板偏特化，更加具体
 public:
  A() { std::cout << "partial specialization\r\n"; }
}; // specialization for floating point types

int main() {
  std::shared_ptr<A<int>> a1 = std::make_shared<A<int>>();
  auto a2 = std::make_shared<A<double>>(); // 第二个偏特化成功，使用第二个
}