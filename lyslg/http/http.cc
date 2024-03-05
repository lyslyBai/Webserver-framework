#include "http.h"
#include <string.h>
#include "util.h"

namespace lyslg{

namespace http{

HttpMethod StringToHttpMethod(const std::string& m){
#define XX(num,name,string) \
    if(strcmp(#string,m.c_str()) == 0){ \
        return HttpMethod::name; \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVALID_METHOD;
}
HttpMethod CharsToHttpMethod(const char* m){
#define XX(num,name,string) \
    if(strncmp(#string,m,strlen(#string)) == 0){ \
        return HttpMethod::name; \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVALID_METHOD;
}

static const char* s_method_string[] = {
#define XX(num,name,string) #string,
     HTTP_METHOD_MAP(XX)
#undef XX
};

const char* HttpMethodToString(const HttpMethod& m){
    uint32_t idx = (uint32_t)m;
    if(idx >= (sizeof(s_method_string) / sizeof(s_method_string[0]))) {
        return "<unknown>";
    }
    return s_method_string[idx];
}

const char* HttpStatusToString(const HttpStatus& s){
    switch(s) {
#define XX(code,name,msg) \
        case HttpStatus::name: \
            return #msg;
        HTTP_STATUS_MAP(XX);
#undef XX
        default:
            return "<unknown>";
    }
}


bool CaseInsensitiveLess::operator()(const std::string& lhs, const std::string& rhs) const{
    return strcasecmp(lhs.c_str(),rhs.c_str()) < 0;
}

HttpRequest::HttpRequest(uint8_t version , bool close )
    :m_method(HttpMethod::GET)
    ,m_version(version)
    ,m_close(close)
    ,m_parserParamFlag(0)
    ,m_path("/"){
}

std::shared_ptr<HttpResponse> HttpRequest::createResponse() {
    HttpResponse::ptr rsp(new HttpResponse(getVersion()
                            ,isClose()));
    return rsp;
}

std::string HttpRequest::getHeader(const std::string& key, const std::string& def) const{
    auto it = m_headers.find(key);
    return it == m_headers.end() ? def : it->second;
}
std::string HttpRequest::getParam(const std::string& key, const std::string& def ) const{
    auto it = m_params.find(key);
    return it == m_params.end() ? def : it->second;
}
std::string HttpRequest::getCookie(const std::string& key, const std::string& def ) const{
    auto it = m_cookies.find(key);
    return it == m_cookies.end() ? def : it->second;
}   

void HttpRequest::setHeader(const std::string& key, const std::string& val){
    m_headers[key] = val;
}
void HttpRequest::setParam(const std::string& key,const std::string& val){
    m_params[key]=val;
}
void HttpRequest::setCookie(const std::string& key,const std::string& val){
    m_cookies[key] = val;
}

void HttpRequest::delHeader(const std::string& key){
    m_headers.erase(key);
}
void HttpRequest::delParam(const std::string& key){
    m_params.erase(key);
}
void HttpRequest::delCookie(const std::string& key){
    m_cookies.erase(key);
}   

bool HttpRequest::hasHeader(const std::string& key, std::string* val){
    auto it = m_headers.find(key);
    if(it  == m_headers.end()) {
        return false;
    }
    if(val) {
        *val = it->second;
    }
    return true;
}
bool HttpRequest::hasParam(const std::string& key, std::string* val){
    auto it = m_params.find(key);
    if(it  == m_params.end()) {
        return false;
    }
    if(val) {
        *val = it->second;
    }
    return true;
}
bool HttpRequest::hasCookie(const std::string& key, std::string* val){
    auto it = m_cookies.find(key);
    if(it  == m_cookies.end()) {
        return false;
    }
    if(val) {
        *val = it->second;
    }
    return true;
}

std::ostream& HttpRequest::dump(std::ostream& os) const{
    os << HttpMethodToString(m_method) << " "
       << m_path
       << (m_query.empty() ? "":"?")
       << m_query
       << (m_fragment.empty() ? "":"#")
       << m_fragment
       << " HTTP/"
       << ((uint32_t)(m_version >> 4))
       << "."
       << ((uint32_t)(m_version & 0x0f))
       << "\r\n";
    if(!m_websocket) {
        os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
    }
    for(auto& i : m_headers) {
        if(!m_websocket && strcasecmp(i.first.c_str(), "connection") == 0) {
            continue;
        }
        os << i.first << ": " << i.second << "\r\n";
    }
    if(!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n"
            << m_body;
    }else {
        os << "\r\n";
    }
    return os;
}

std::string HttpRequest::toString() const{
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

void HttpRequest::init() {
    std::string conn = getHeader("connection");
    if(!conn.empty()) {
        if(strcasecmp(conn.c_str(), "keep-alive") == 0) {
            m_close = false;
        } else {
            m_close = true;
        }
    }
}

void HttpRequest::initParam() {
    initQueryParam();
    initBodyParam();
    initCookies();
}

void HttpRequest::initQueryParam() {
    if(m_parserParamFlag & 0x1) {
        return;
    }

#define PARSE_PARAM(str, m, flag, trim) \
    size_t pos = 0; \
    do { \
        size_t last = pos; \
        pos = str.find('=', pos); \
        if(pos == std::string::npos) { \
            break; \
        } \
        size_t key = pos; \
        pos = str.find(flag, pos); \
        m.insert(std::make_pair(trim(str.substr(last, key - last)), \
                    lyslg::StringUtil::UrlDecode(str.substr(key + 1, pos - key - 1)))); \
        if(pos == std::string::npos) { \
            break; \
        } \
        ++pos; \
    } while(true);

    PARSE_PARAM(m_query, m_params, '&',);
    m_parserParamFlag |= 0x1;
}

void HttpRequest::initBodyParam() {
    if(m_parserParamFlag & 0x2) {
        return;
    }
    std::string content_type = getHeader("content-type");
    if(strcasestr(content_type.c_str(), "application/x-www-form-urlencoded") == nullptr) {
        m_parserParamFlag |= 0x2;
        return;
    }
    PARSE_PARAM(m_body, m_params, '&',);
    m_parserParamFlag |= 0x2;
}

void HttpRequest::initCookies() {
    if(m_parserParamFlag & 0x4) {
        return;
    }
    std::string cookie = getHeader("cookie");
    if(cookie.empty()) {
        m_parserParamFlag |= 0x4;
        return;
    }
    PARSE_PARAM(cookie, m_cookies, ';', lyslg::StringUtil::Trim);
    m_parserParamFlag |= 0x4;
}


HttpResponse::HttpResponse(uint8_t version ,bool close )
    :m_status(HttpStatus::OK)
    ,m_version(version)
    ,m_close(close){

}
std::string HttpResponse::getHeader(const std::string& key, const std::string& def ) const{
    auto it = m_headers.find(key);
    return it == m_headers.end() ? def : it->second;
}
void HttpResponse::setHeader(const std::string& key, const std::string& val){
    m_headers[key] = val;
}
void HttpResponse::delHeader(const std::string& key){
    m_headers.erase(key);
}

/*该函数用于设置 HTTP 响应的重定向。
参数 uri 表示重定向的目标 URI。
在函数中，m_status 被设置为 HttpStatus::FOUND，这通常对应 HTTP 状态码 302，表示找到了资源，但临时重定向到另一个位置。
使用 setHeader 函数设置了一个名为 "Location" 的响应头，值为传入的 uri。*/
void HttpResponse::setRedirect(const std::string& uri) {
    m_status = HttpStatus::FOUND;
    setHeader("Location", uri);
}

/*该函数用于设置 HTTP 响应的 Cookie。
参数包括 key 和 val，分别表示 Cookie 的键和值。
expired 表示 Cookie 的过期时间，使用 lyslg::Time2Str 函数将 expired 转换为 GMT 格式的时间字符串，然后作为 "expires" 字段的值添加到 Cookie 中。
domain 和 path 表示 Cookie 的域和路径，分别通过 ";domain=" 和 ";path=" 添加到 Cookie 字符串中。
secure 是一个布尔值，表示是否要设置安全标志，如果为 true，则添加 ";secure" 到 Cookie 字符串中。
使用 std::stringstream 将上述信息拼接成一个字符串，并将其添加到 m_cookies 向量中。*/
void HttpResponse::setCookie(const std::string& key, const std::string& val,
                             time_t expired, const std::string& path,
                             const std::string& domain, bool secure) {
    std::stringstream ss;
    ss << key << "=" << val;
    if(expired > 0) {
        ss << ";expires=" << lyslg::Time2Str(expired, "%a, %d %b %Y %H:%M:%S") << " GMT";
    }
    if(!domain.empty()) {
        ss << ";domain=" << domain;
    }
    if(!path.empty()) {
        ss << ";path=" << path;
    }
    if(secure) {
        ss << ";secure";
    }
    m_cookies.push_back(ss.str());
}

std::ostream& HttpResponse::dump(std::ostream& os) const{
    os << "HTTP/"
       << ((uint32_t)(m_version >> 4))
       << "."
       << ((uint32_t)(m_version & 0x0f))
       << " "
       << (uint32_t)m_status
       << " "
       << (m_reason.empty() ? HttpStatusToString(m_status) : m_reason) // m_reason 为自定义的原因短语,或原来的
       << "\r\n";
    
    for(auto& i : m_headers) {
        if(!m_websocket && strcasecmp(i.first.c_str(), "connection") == 0) {
            continue;
        }
        os << i.first << ": " << i.second << "\r\n";
    }

    for(auto& i : m_cookies) {
        os << "Set-Cookie: " << i << "\r\n";
    }

    if(!m_websocket) {
        os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
    }

    if(!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n"
            << m_body;
    } else {
        os << "\r\n";
    }
    return os;
}

std::string HttpResponse::toString() const{
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const HttpRequest& req){
    return req.dump(os);
}
std::ostream& operator<<(std::ostream& os, const HttpResponse& rsp){
     return rsp.dump(os);
}


}
}