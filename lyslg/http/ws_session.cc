#include "ws_session.h"
#include "log.h"
#include "my_endian.h"
#include <string.h>
#include "hash_util.h"

namespace lyslg {
namespace http {

static lyslg::Logger::ptr g_logger = LYSLG_LOG_NAME("system");

lyslg::ConfigVar<uint32_t>::ptr g_websocket_message_max_size
    = lyslg::Config::Lookup("websocket.message.max_size"
            ,(uint32_t) 1024 * 1024 * 32, "websocket message max size");

WSSession::WSSession(Socket::ptr sock, bool owner)
    :HttpSession(sock, owner) {
}

HttpRequest::ptr WSSession::handleShake() {
    HttpRequest::ptr req;
    do {
        req = recvRequest();
        if(!req) {
            LYSLG_LOG_INFO(g_logger) << "invalid http request";
            break;
        }
        /*这行代码检查HTTP请求头部中的 Upgrade 字段，该字段指示客户端希望升级协议。WebSocket握手要求该字段的值为 "websocket"*/
        if(strcasecmp(req->getHeader("Upgrade").c_str(), "websocket")) {
            LYSLG_LOG_INFO(g_logger) << "http header Upgrade != websocket";
            break;
        }
        /*这行代码检查HTTP请求头部中的 Connection 字段，该字段指示客户端是否希望进行连接的升级。WebSocket握手要求该字段的值为 "Upgrade"*/
        if(strcasecmp(req->getHeader("Connection").c_str(), "Upgrade")) {
            LYSLG_LOG_INFO(g_logger) << "http header Connection != Upgrade";
            break;
        }
        /*这行代码检查HTTP请求头部中的 Sec-WebSocket-Version 字段，该字段指示客户端使用的WebSocket协议版本。WebSocket握手要求该字段的值为13*/
        if(req->getHeaderAs<int>("Sec-webSocket-Version") != 13) {
            LYSLG_LOG_INFO(g_logger) << "http header Sec-webSocket-Version != 13";
            break;
        }
        /*这行代码获取HTTP请求头部中的 Sec-WebSocket-Key 字段，该字段是WebSocket握手过程中用于生成响应头部的关键。*/
        std::string key = req->getHeader("Sec-WebSocket-Key");
        if(key.empty()) {
            LYSLG_LOG_INFO(g_logger) << "http header Sec-WebSocket-Key = null";
            break;
        }
        /*这个字符串是为了后续计算WebSocket握手中的 Sec-WebSocket-Accept 头部而准备的。*/
        std::string v = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        /*sha1sum()对提供的数据进行 SHA-1 哈希计算*/
        /*base64encode  Base64 编码是一种将二进制数据转换为可打印字符的编码方式*/
        v = lyslg::base64encode(lyslg::sha1sum(v));
        // 确认websocket
        req->setWebsocket(true);

        auto rsp = req->createResponse();
        rsp->setStatus(HttpStatus::SWITCHING_PROTOCOLS);
        rsp->setWebsocket(true);
        rsp->setReason("Web Socket Protocol Handshake");
        rsp->setHeader("Upgrade", "websocket");
        rsp->setHeader("Connection", "Upgrade");
        rsp->setHeader("Sec-WebSocket-Accept", v); 

        sendResponse(rsp);
        LYSLG_LOG_DEBUG(g_logger) << *req;
        LYSLG_LOG_DEBUG(g_logger) << *rsp;
        return req;
    } while(false);
    if(req) {
        LYSLG_LOG_INFO(g_logger) << *req;
    }
    return nullptr;
}

WSFrameMessage::WSFrameMessage(int opcode, const std::string& data)
    :m_opcode(opcode)
    ,m_data(data) {
}

std::string WSFrameHead::toString() const {
    std::stringstream ss;
    ss << "[WSFrameHead fin=" << fin
       << " rsv1=" << rsv1
       << " rsv2=" << rsv2
       << " rsv3=" << rsv3
       << " opcode=" << opcode
       << " mask=" << mask
       << " payload=" << payload
       << "]";
    return ss.str();
}

WSFrameMessage::ptr WSSession::recvMessage() {
    return WSRecvMessage(this, false);
}

int32_t WSSession::sendMessage(WSFrameMessage::ptr msg, bool fin) {
    return WSSendMessage(this, msg, false, fin);
}

int32_t WSSession::sendMessage(const std::string& msg, int32_t opcode, bool fin) {
    return WSSendMessage(this, std::make_shared<WSFrameMessage>(opcode, msg), false, fin);
}

int32_t WSSession::ping() {
    return WSPing(this);
}

WSFrameMessage::ptr WSRecvMessage(Stream* stream, bool client) {
    int opcode = 0; // WebSocket帧的操作码
    std::string data;  // 存储消息数据的字符串
    int cur_len = 0;   // 当前处理的消息数据长度
    do {
        // // 读取WebSocket帧头部
        WSFrameHead ws_head;
        if(stream->readFixSize(&ws_head, sizeof(ws_head)) <= 0) {
            break;
        }
        LYSLG_LOG_DEBUG(g_logger) << "WSFrameHead " << ws_head.toString();
        // 处理PING帧
        if(ws_head.opcode == WSFrameHead::PING) {
            LYSLG_LOG_INFO(g_logger) << "PING";
            if(WSPong(stream) <= 0) {
                break;
            }
        } else if(ws_head.opcode == WSFrameHead::PONG) {
            //// 这里可以添加处理PONG帧的逻辑，暂时为空
        } else if(ws_head.opcode == WSFrameHead::CONTINUE
                || ws_head.opcode == WSFrameHead::TEXT_FRAME
                || ws_head.opcode == WSFrameHead::BIN_FRAME) {
            // 在服务端，如果数据帧没有掩码，直接断开连接
            if(!client && !ws_head.mask) {
                LYSLG_LOG_INFO(g_logger) << "WSFrameHead mask != 1";
                break;
            }
            // 读取帧的数据长度
            uint64_t length = 0;
            if(ws_head.payload == 126) {
                uint16_t len = 0;  // 接受额外的两字节
                if(stream->readFixSize(&len, sizeof(len)) <= 0) {
                    break;
                }
                length = lyslg::byteswapOnLittleEndian(len);
            } else if(ws_head.payload == 127) {
                uint64_t len = 0;  // 接受额外的8字节，弃用前面的7位
                if(stream->readFixSize(&len, sizeof(len)) <= 0) {
                    break;
                }
                length = lyslg::byteswapOnLittleEndian(len);
            } else {
                length = ws_head.payload;
            }
            // 检查消息长度是否超过限制
            if((cur_len + length) >= g_websocket_message_max_size->getValue()) {
                LYSLG_LOG_WARN(g_logger) << "WSFrameMessage length > "
                    << g_websocket_message_max_size->getValue()
                    << " (" << (cur_len + length) << ")";
                break;
            }
            // 处理掩码
            char mask[4] = {0};
            if(ws_head.mask) {
                if(stream->readFixSize(mask, sizeof(mask)) <= 0) {
                    break;
                }
            }
            // 读取帧的数据
            data.resize(cur_len + length);
            if(stream->readFixSize(&data[cur_len], length) <= 0) {
                break;
            }
            // 解掩码，简单的异或
            if(ws_head.mask) {
                for(int i = 0; i < (int)length; ++i) {
                    data[cur_len + i] ^= mask[i % 4];
                }
            }
            cur_len += length;
            // 记录第一个数据帧的操作码,第一帧说明本消息为二进制帧或文本帧
            if(!opcode && ws_head.opcode != WSFrameHead::CONTINUE) {
                opcode = ws_head.opcode;
            }
            // 如果是最后一个数据帧，则构造WSFrameMessage并返回，最后一帧ws_head.fin设为true
            if(ws_head.fin) {
                LYSLG_LOG_DEBUG(g_logger) << data;
                return WSFrameMessage::ptr(new WSFrameMessage(opcode, std::move(data)));
            }
        } else {
            LYSLG_LOG_DEBUG(g_logger) << "invalid opcode=" << ws_head.opcode;
        }
    } while(true);
    stream->close();
    return nullptr;
}

int32_t WSSendMessage(Stream* stream, WSFrameMessage::ptr msg, bool client, bool fin) {
    do {
        //// 构造WebSocket帧头部信息
        WSFrameHead ws_head;
        memset(&ws_head, 0, sizeof(ws_head));
        ws_head.fin = fin;     // fin标志，表示消息是否为最后一个帧
        ws_head.opcode = msg->getOpcode();   // 操作码，表示消息的类型
        ws_head.mask = client;               // 是否使用掩码
        uint64_t size = msg->getData().size();    // 消息数据的大小
        if(size < 126) {           // 根据数据大小设置payload字段
            ws_head.payload = size;
        } else if(size < 65536) {   // 2^16
            ws_head.payload = 126;
        } else {                    // 2^64
            ws_head.payload = 127;
        }
        
        // 写入WebSocket帧头部信息
        if(stream->writeFixSize(&ws_head, sizeof(ws_head)) <= 0) {
            break;
        }
        //// 处理payload字段为126的情况，写入长度信息
        if(ws_head.payload == 126) {
            uint16_t len = size;
            len = lyslg::byteswapOnLittleEndian(len);
            if(stream->writeFixSize(&len, sizeof(len)) <= 0) {
                break;
            }
        } else if(ws_head.payload == 127) {
            // // 处理payload字段为127的情况，写入长度信息
            uint64_t len = lyslg::byteswapOnLittleEndian(size);
            if(stream->writeFixSize(&len, sizeof(len)) <= 0) {
                break;
            }
        }
        //// 如果使用掩码，生成随机掩码并应用于消息数据
        if(client) {
            char mask[4];
            uint32_t rand_value = rand();
            memcpy(mask, &rand_value, sizeof(mask));
            std::string& data = msg->getData();
            for(size_t i = 0; i < data.size(); ++i) {
                data[i] ^= mask[i % 4];
            }

            if(stream->writeFixSize(mask, sizeof(mask)) <= 0) {
                break;
            }
        }
        if(stream->writeFixSize(msg->getData().c_str(), size) <= 0) {
            break;
        }
        return size + sizeof(ws_head);
    } while(0);
    stream->close();
    return -1;
}

int32_t WSSession::pong() {
    return WSPong(this);
}
/*WebSocket的PING帧用于检测连接是否仍然存活。发送PING后，
接收到PONG的响应表示连接正常。这样的机制可以用于保持WebSocket连接的活跃状态。*/

int32_t WSPing(Stream* stream) {
    // 构造WebSocket PING帧的头部信息
    WSFrameHead ws_head;
    memset(&ws_head, 0, sizeof(ws_head));
    ws_head.fin = 1;
    ws_head.opcode = WSFrameHead::PING;
    // 将构造好的WebSocket帧头部信息写入到流中
    int32_t v = stream->writeFixSize(&ws_head, sizeof(ws_head));
    if(v <= 0) {
        stream->close();
    }
    return v;
}
// 向流中写入WebSocket PONG帧的头部信息
int32_t WSPong(Stream* stream) {
    WSFrameHead ws_head;
    memset(&ws_head, 0, sizeof(ws_head));
    ws_head.fin = 1;
    ws_head.opcode = WSFrameHead::PONG;
    int32_t v = stream->writeFixSize(&ws_head, sizeof(ws_head));
    if(v <= 0) {
        stream->close();
    }
    return v;
}

}
}
