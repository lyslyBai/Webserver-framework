#include <ucontext.h>
#include <iostream>

ucontext_t context1, context2,main1;

void func1() {
    std::cout << "Entering func1" << std::endl;
    swapcontext(&context2, &main1);
    std::cout << "Exiting func1" << std::endl;
}

void func2() {
    std::cout << "Entering func2" << std::endl;
    swapcontext(&context2, &context1);
    std::cout << "Exiting func2" << std::endl;
}

int main() {
    // 初始化上下文
    puts("context1 before");
    getcontext(&context1);
    context1.uc_stack.ss_sp = new char[8192];
    context1.uc_stack.ss_size = 8192;
    makecontext(&context1, func1, 0);
    puts("context1 after");

    puts("main1 before");
    swapcontext(&main1, &context1);
    puts("main1 after");

    // getcontext(&context2);
    // context2.uc_stack.ss_sp = new char[8192];
    // context2.uc_stack.ss_size = 8192;
    // makecontext(&context2, func2, 0);

    // // 执行切换上下文
    // std::cout << "Starting main" << std::endl;
    // swapcontext(&context1, &context2);
    // std::cout << "Back to main" << std::endl;

    // 释放资源
    delete[] static_cast<char*>(context1.uc_stack.ss_sp);
    // delete[] static_cast<char*>(context2.uc_stack.ss_sp);

    return 0;
}
