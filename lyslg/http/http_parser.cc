#include "http_parser.h"
#include "log.h"
#include "config.h"

namespace lyslg{

namespace http{
static lyslg::Logger::ptr g_logger = LYSLG_LOG_ROOT();
static lyslg::ConfigVar<uint64_t>::ptr g_http_request_buffer_size = 
    lyslg::Config::Lookup("http.request.buffer_size", 4 * 1024,"http request buffer size");

static lyslg::ConfigVar<uint64_t>::ptr g_http_request_max_body_size = 
    lyslg::Config::Lookup("http.request.max_body_size", 64 * 1024 * 1024ull,"http request max body size");

static uint64_t s_http_request_buffer_size = 0;
static uint64_t s_http_request_max_body_size = 0;

struct _RequestSizeIniter {
    _RequestSizeIniter(){
        s_http_request_buffer_size = g_http_request_buffer_size->getValue();
        s_http_request_max_body_size = g_http_request_max_body_size->getValue();

        g_http_request_buffer_size->addListener([]
            (const uint64_t& ov, const uint64_t& nv){
                s_http_request_buffer_size = nv;
        });

         g_http_request_max_body_size->addListener([]
            (const uint64_t& ov, const uint64_t& nv){
                s_http_request_max_body_size = nv;
        });
    }
};

static _RequestSizeIniter _init;


void on_request_method(void* data,const char*at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    HttpMethod m = CharToHttpMethod(at);

    if(m == HttpMethod::INVALID_METHOD) {
        LYSLG_LOG_WARN(g_logger) << "invalid http request metnod "
             << std::string(at,length);
        parser->setError(1000);
        return ;
    }
    parser->getData()->setMethod(m);
}
void on_request_uri(void* data,const char*at, size_t length) {

}
void on_request_fragment(void* data,const char*at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setFragment(std::string(at,length));
}
void on_request_path(void* data,const char*at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setPath(std::string(at,length));
}
void on_request_query(void* data,const char*at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setQuery(std::string(at,length));
}
void on_request_version(void* data,const char*at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    uint8_t v = 0;
    if(strncmp(at,"HTTP/1.1",length) == 0) {
        v = 0x11;
    } else if(strncmp(at,"HTTP/1.0",length) == 0) {
        v = 0x10;
    } else {
        LYSLG_LOG_WARN(g_logger) << "invalid http request version: "
            << std::string(at,length);
        setError(1001);
        return ;
    }
    parser->getData()->setVersion(v);
}
void on_request_header_done(void* data,const char*at, size_t length) {

}

void on_request_http_field(void *data, const char* field,size_t flen,
                const char* value,size_t vlen){
    
}

HttpRequestParser::HttpRequestParser(){
    m_data.reset(new lyslg::http::HttpRequest);
    http_parser_init(&m_parser);
    m_parser.request_method = on_request_method;
    m_parser.request_uri = on_request_uri;
    m_parser.fragment = on_request_fragment;
    m_parser.request_path = on_request_path;
    m_parser.query_string = on_request_query;
    m_parser.http_version = on_request_version;
    m_parser.header_done = on_request_header_done;
    m_parser.http_field = on_request_http_field;
    m_parser.data = this;

}
size_t HttpRequestParser::excute(const char* data,size_t len,size_t off){
    return 0;
}
int HttpRequestParser::isFinish() const{
    return 0;
}
int HttpRequestParser::hasError() const{
    return 0;
}   


void on_response_reason(void *data, const char* at,size_t length){

}
void on_response_status(void *data, const char* at,size_t length){
    
}
void on_response_chunk(void *data, const char* at,size_t length){
    
}
void on_response_version(void *data, const char* at,size_t length){
    
}
void on_response_header_done(void *data, const char* at,size_t length){
    
}
void on_response_last_chunk(void *data, const char* at,size_t length){
    
}
void on_response_http_field(void *data, const char* field,size_t flen,
                const char* value,size_t vlen){
    
}

HttpResponseParser::HttpResponseParser(){
    m_data.reset(new lyslg::http::HttpResponse);
    httpclient_parser_init(&m_parser);
    m_parser.reason_phrase = on_response_reason;
    m_parser.status_code = on_response_status;
    m_parser.chunk_size = on_response_chunk;
    m_parser.http_version = on_response_version;
    m_parser.header_done = on_response_header_done;
    m_parser.last_chunk = on_response_last_chunk;
    m_parser.http_field = on_response_http_field;

}
size_t HttpResponseParser::excute(const char* data,size_t len,size_t off){
    return 0;
}
int HttpResponseParser::isFinish() const{
    return 0;
}
int HttpResponseParser::hasError() const{
    return 0;
}

}

}
