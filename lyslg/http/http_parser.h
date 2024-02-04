#ifndef __LYSLG_HTTP_PARSER_H__
#define __LYSLG_HTTP_PARSER_H__

#include <memory>
#include "http11_parser.h"
#include "httpclient_parser.h"
#include "http.h"

namespace lyslg{
namespace http{

class HttpRequestParser {
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;
    HttpRequestParser();
    size_t excute(const char* data,size_t len,size_t off);
    int isFinish() const;
    int hasError() const;

    HttpRequest::ptr getData() const {return m_data;}
    void setError(int v) {m_error = v;}
private:
    http_parser m_parser;
    HttpRequest::ptr m_data;
    int m_error;
};

class HttpResponseParser {
public:
    typedef std::shared_ptr<HttpResponseParser> ptr;
    HttpResponseParser();
    size_t excute(const char* data,size_t len,size_t off);
    int isFinish() const;
    int hasError() const;

    HttpResponse::ptr getData() const {return m_data;}
private:
    httpclient_parser m_parser;
    HttpResponse::ptr m_data;
    int m_error;
};


}

}






#endif 