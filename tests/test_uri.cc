#include "uri.h"
#include <iostream>

int main(int argc, char** argv) {
    // lyslg::Uri::ptr uri = lyslg::Uri::Create("http://www.baidu.com?id=100&name=lyslg#frg");
    lyslg::Uri::ptr uri = lyslg::Uri::Create("http://admin@www.sylar.top/test/中文/uri?id=100&name=sylar&vv=中文#frg中文");
    // lyslg::Uri::ptr uri = lyslg::Uri::Create("http://admin@www.lyslg.top");
    //lyslg::Uri::ptr uri = lyslg::Uri::Create("http://www.lyslg.top/test/uri");
    std::cout << uri->toString() << std::endl;
    auto addr = uri->createAddress();
    std::cout << *addr << std::endl;
    return 0;
}
