#include "http_session.h"
#include "http_parser.h"
#include "log.h"


namespace lyslg{
namespace http {

static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system");

HttpSession::HttpSession(Socket::ptr socket, bool owner)
    :SocketStream(socket,owner){
}

http::HttpRequest::ptr HttpSession::recvRequest(){
    HttpRequestParser::ptr parser(new HttpRequestParser);
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    // uint64_t buff_size = 100;
    std::shared_ptr<char> buffer(
        new char[buff_size], [](char* ptr){
            delete[] ptr;
        }
    );
    char* data = buffer.get();
    int offset = 0;
    do{
        int len = read(data + offset, buff_size - offset); // 这里offset是未解析的数据，buff_size为缓存剩下的空间
        if(len <= 0) {
            close();
            return nullptr;
        }
        len += offset;
        size_t nparse = parser->execute(data,len);
        if(parser->hasError()) {
            close();
            return nullptr;
        }
        offset = len - nparse;
        if(offset == (int)buff_size) {  //如果缓存满了（offset 等于缓存大小），则关闭连接并返回空指针。
            close();
            return nullptr;
        }
        if(parser->isFinished()) {
            break;
        }
    }while(true);
    int64_t length = parser->getContentLength();
    if(length > 0) {  // 如果内容长度大于 0，则创建一个 std::string 对象 body 用于存储请求体。
        std::string body;
        body.reserve(length);

        int len = 0;
        if(length >= offset) {
            memcpy(&body[0], data, offset);
            len = offset;
        } else {
            memcpy(&body[0], data, length);
            len = length;
        }
        // 上述将之前读到的正文报文copy到body，若之前读取的内容不足，则读取指定字节，未读到，则退出
        length -= offset;
        if(length > 0) {
            if(readFixSize(&body[len], length) <= 0) {
                close();
                return nullptr;
            }
        }
        parser->getData()->setBody(body);
    }
    // 设置m_close标志位
    parser->getData()->init();
    return parser->getData();
}

int HttpSession::sendResponse(http::HttpResponse::ptr rsp){
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    return writeFixSize(data.c_str(), data.size()); 
}

}

};