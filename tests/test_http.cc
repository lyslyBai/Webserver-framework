#include <iostream>
#include "http.h"
#include "log.h"

void test_request(){
    lyslg::http::HttpRequest::ptr req(new lyslg::http::HttpRequest);
    req->setHeader("host","www.baidu.com");
    req->setBody("hello lyslg");

    req->dump(std::cout) << std::endl;
}

void test_response() {
    lyslg::http::HttpResponse::ptr rsp(new lyslg::http::HttpResponse);
    rsp->setHeader("X_X","lyslg");
    rsp->setBody("hello lyslg");
    rsp->setStatus((lyslg::http::HttpStatus)400);
    rsp->setClose(false);
    rsp->dump(std::cout) << std::endl;
}


int main() {
    test_request();
    test_response();
    return 0;
}