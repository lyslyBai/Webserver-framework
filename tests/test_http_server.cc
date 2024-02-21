#include "log.h"
#include "http_server.h"


void run() {
    lyslg::http::HttpServer::ptr server(new lyslg::http::HttpServer);
    lyslg::Address::ptr addr = lyslg::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }

    auto sd = server->getServletDispatch();
    sd->addServlet("/lyslg/xx", [](lyslg::http::HttpRequest::ptr req
                ,lyslg::http::HttpResponse::ptr rsp
                ,lyslg::http::HttpSession::ptr session) {
            rsp->setBody(req->toString());
            return 0;
    });

    sd->addGlobServlet("/lyslg/*", [](lyslg::http::HttpRequest::ptr req
                ,lyslg::http::HttpResponse::ptr rsp
                ,lyslg::http::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->toString());
            return 0;
    });


    server->start();

}


int main() {

    lyslg::IoManager iom(2);
    iom.schedule(run);

    return 0;   
}