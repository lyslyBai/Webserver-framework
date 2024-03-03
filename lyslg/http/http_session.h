#ifndef __LYSLG_HTTP_SESSION_H__
#define __LYSLG_HTTP_SESSION_H__

#include "../socket_stream.h"
#include "http.h"

namespace lyslg{

// 概括一下就是接受请求报文，解析，然后，提供一个发送响应报文的函数

namespace http{
class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;
    HttpSession(Socket::ptr socket, bool owner = true);
    HttpRequest::ptr recvRequest();
    int sendResponse(HttpResponse::ptr rsp);


};

}
}


#endif