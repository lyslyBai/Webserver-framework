// #pragma once   // 防止头文件重复包含,放在头文件中，不应放在主文件中

#define LYSLG 5   // 宏定义
#undef LYSLG   // 取消宏定义
 
#include <iostream>

#define LYSLG 5

/*在C++中，直接在宏条件编译块中赋值是不允许的。
条件编译块中只能包含预处理指令，不能包含普通的语句。
你可以通过使用预处理宏来定义常量，然后在代码中使用这个常量。*/

#ifndef LYSLG  // 条件编译  如果未LYSLG定义了
int a = 6;
#else
int a = 5;  // 高亮部分为当前条件会执行的语句
#endif

#ifdef LYSLG  // 条件编译  如果LYSLG定义了
int b = 6;
#else
int b = 5;
#endif

#if LYSLG == 42
// 如果 LYSLG 的值等于 42
int c = 14;
#elif LYSLG > 42   // 用于多个条件的判断。
// 如果 LYSLG 的值大于 42
int c = 12;
#elif LYSLG > 36
// 如果 LYSLG 的值大于 36
int c = 56
#else
// 如果以上条件都不满足
int c = 5;
#endif



int main()
{
    std::cout << "a=" << a
              << " b=" << b
              << " c=" << c 
              <<std::endl;
    return 0;
}