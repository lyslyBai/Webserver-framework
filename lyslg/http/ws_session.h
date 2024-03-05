#ifndef __LYSLG_HTTP_WS_SESSION_H__
#define __LYSLG_HTTP_WS_SESSION_H__

#include "config.h"
#include "http_session.h"
#include <stdint.h>

namespace lyslg {
namespace http {
/*如果一个消息没有被分片，即整个消息可以在一帧中传输，那么这帧的 opcode 就是消息的类型（TEXT_FRAME 或 BIN_FRAME 等）。

如果消息被分片，那么第一个分片的 opcode 就是消息的类型，而后续分片的 opcode 通常是 CONTINUE。

但是，在最后一帧中，fin 标志会被设置为 true，表示这是消息的最后一帧。*/

/*#pragma pack(1)：这是一个编译器指令，用于设置结构体的字节对齐方式。在这里，pack(1)表示使用最小的字节对齐方式，
也就是按字节对齐，不会填充任何空隙。这通常用于确保结构体的大小是成员变量大小的总和，而不会因为对齐而增加额外的空间。*/
#pragma pack(1)
// 定义了websocket的头帧，所占字节为2-14字节
struct WSFrameHead {
/*定义了WebSocket帧的不同操作码（OPCODE）。每个操作码对应WebSocket协议中的不同类型的帧。*/
/*占用第一字节的后四位，0001为文本帧，0010为二进制帧，1000为断开连接，1001为ping帧，1010位pong帧*/
/*CONTINUE (0)：表示当前帧包含的数据是消息的中间部分（分片）,在消息的最后一个帧（FIN标志为1）之前的所有帧的opcode都应该是CONTINUE。*/
    enum OPCODE {
        /// 数据分片帧
        CONTINUE = 0,
        /// 文本帧
        TEXT_FRAME = 1,
        /// 二进制帧
        BIN_FRAME = 2,
        /// 断开连接
        CLOSE = 8,
        /// PING
        PING = 0x9,
        /// PONG
        PONG = 0xA
    };
/*表示WebSocket帧的操作码。由于WebSocket协议规定了操作码占用4位*/
    uint32_t opcode: 4;
/*这四个布尔值表示WebSocket帧头部的一些控制位，分别对应协议中的RSV3、RSV2、RSV1和FIN位。*/
    bool rsv3: 1; // 第四位，保留字节，必须为0
    bool rsv2: 1; // 第三位，保留字节，必须为0
    bool rsv1: 1; // 第二位，保留字节，必须为0
    bool fin: 1;  // 第一字节的第一位，
/*示WebSocket帧的有效载荷（payload）的长度，占用第二字节的后七位，和往后面的最大八字节*/
    uint32_t payload: 7;
/*占用第二字节的第一位，这个布尔值表示是否使用掩码，WebSocket协议允许在数据传输时使用掩码进行数据的加密,服务器端不使用掩码，客户端使用掩码，掩码占最后的4字节。*/
    bool mask: 1; 

    std::string toString() const;
};
/*恢复默认的字节对齐方式。*/
#pragma pack()

/*表示WebSocket帧的消息部分*/
class WSFrameMessage {
public:
    typedef std::shared_ptr<WSFrameMessage> ptr;
    WSFrameMessage(int opcode = 0, const std::string& data = "");

    int getOpcode() const { return m_opcode;}
    void setOpcode(int v) { m_opcode = v;}

    const std::string& getData() const { return m_data;}
    std::string& getData() { return m_data;}
    void setData(const std::string& v) { m_data = v;}
private:/*初始操作码（opcode）和数据（data）*/
    int m_opcode;
    std::string m_data;
};

class WSSession : public HttpSession {
public:
    typedef std::shared_ptr<WSSession> ptr;
    WSSession(Socket::ptr sock, bool owner = true);

    /// server client
    HttpRequest::ptr handleShake();

    WSFrameMessage::ptr recvMessage();
    int32_t sendMessage(WSFrameMessage::ptr msg, bool fin = true);
    int32_t sendMessage(const std::string& msg, int32_t opcode = WSFrameHead::TEXT_FRAME, bool fin = true);
    int32_t ping();
    int32_t pong();
private:
    bool handleServerShake();
    bool handleClientShake();
};

extern lyslg::ConfigVar<uint32_t>::ptr g_websocket_message_max_size;
WSFrameMessage::ptr WSRecvMessage(Stream* stream, bool client);
int32_t WSSendMessage(Stream* stream, WSFrameMessage::ptr msg, bool client, bool fin);
int32_t WSPing(Stream* stream);
int32_t WSPong(Stream* stream);

}
}

#endif
