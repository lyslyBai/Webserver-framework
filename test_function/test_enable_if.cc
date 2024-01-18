#include <iostream>
/*这意味着当std::enable_if条件不满足时，不是导致编译错误，
而是使编译器选择其他可行的函数。这使得在模板元编程中更加灵活，
因为编译器可以在候选函数中选择最合适的。*/

/*另外，使用if语句的方式可能导致更复杂的代码结构，
而使用std::enable_if可以在模板参数列表中直接处理这些条件，使代码更加简洁和可读。*/

// enabled via the return type

/*这种技术的应用包括在函数模板、
类模板、模板特化等场景中，通过std::enable_if的条件来选择不同的实现或特化。
这样的写法利用了SFINAE（Substitution Failure Is Not An Error）原则，该原则允许在模板实例化时忽略掉某些失败的替换，而不会引发编译错误。
*/
template<class T>
typename std::enable_if<std::is_trivially_default_constructible<T>::value>::type  // 充当返回类型，为true，则为默认的void
construct(T *) {
  std::cout << "default constructing trivially default constructible T\n";
}
template<class T>
typename std::enable_if<!std::is_trivially_default_constructible<T>::value>::type  // 充当返回类型，为false，则实例化失败，且不会触发编译错误
construct(T *p) {
  std::cout << "default constructing non-trivially default constructible T\n";
}

/*请注意，如果使用类模板的话，也可以将 std::enable_if 放在模板参数列表中，例如：*/
// template<class T, typename std::enable_if<std::is_trivially_default_constructible<T>::value, int>::type = 0>
// void construct(T *) {
//   // 实现针对 is_trivially_default_constructible 为 true 的情况的代码
//   std::cout << "default constructing trivially default constructible T\n";
// }

// template<class T, typename std::enable_if<!std::is_trivially_default_constructible<T>::value, int>::type = 0>
// void construct(T *p) {
//   // 实现针对 is_trivially_default_constructible 为 false 的情况的代码
//   std::cout << "default constructing non-trivially default constructible T\n";
// }


int main() {
  int a = 1;
  std::string str = "hell";
  construct(&a);
  construct(&str);
  return 0;
}