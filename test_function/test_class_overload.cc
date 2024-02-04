#include <iostream>
#include <memory>

class Person
{
public:
    typedef std::shared_ptr<Person> ptr;
    ptr test() {
        std::cout << "this is person class "<< std::endl;
        return std::make_shared<Person>();
    }

    ptr test(int a) {
        std::cout << "this is person class a = "<< a << std::endl;
        return std::make_shared<Person>();
    }

    ptr test_1(int a) {
        std::cout << "this is person class a = "<< a << std::endl;
        return std::make_shared<Person>();
    }
};

class Child: public Person
{
public:
    typedef std::shared_ptr<Child> ptr;
    /*派⽣类᯿新定义⽗类中相同名字的⾮ virtual 函数，参数列表
和返回类型都可以不同，即⽗类中除了定义成 virtual 且完全相同的同名函数才
不会被派⽣类中的同名函数所隐藏（᯿定义）。*/
    ptr test() {
        std::cout << "this is Cild class "<< std::endl;
        return std::make_shared<Child>();
    }
};

int main()
{
    Child Ch;
    Person::ptr b = Ch.Person::test(); // 这里只能使用Person::来调用不同类中的函数
    Person::ptr a = Ch.test();
    // 只可以调用不同名的成员函数，否则就需要要加作用域
    // Person::ptr a = Ch.test_1(3);

    return 0;
}