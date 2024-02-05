#include <iostream>
#include <string.h>


const char test_request_data[] = "GET / HTTP/1.1\r\n"
                                "Host: www.baidu.com\r\n"
                                "Content-Length: 10\r\n\r\n"
                                "1234567890";

const char test_request_data_1[] = "GET / HTTP/1.1\r\n"
                                "Host: www.baidu.com\r\n"
                                "Content-Length: 10\r\n\r\n"
                                "1234567890\0";                           

int main() {

   // std::string tmp = test_request_data; 70   69
    std::cout << sizeof(test_request_data) << strlen(test_request_data) << std::endl;

   // tmp = test_request_data_1;  71  69
    std::cout << sizeof(test_request_data_1) << strlen(test_request_data_1) << std::endl;


    return 0;
}