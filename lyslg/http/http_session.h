#ifndef __LYSLG_HTTP_SESSION_H__
#define __LYSLG_HTTP_SESSION_H__

#include "../socket_stream.h"
#include "http.h"

namespace lyslg{

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