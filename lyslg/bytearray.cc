#include "bytearray.h"
#include "my_endian.h"
#include "log.h"
#include <string.h>
#include <fstream>
#include <iomanip>
#include <cmath>

namespace lyslg{

static Logger::ptr g_logger = LYSLG_LOG_NAME("system");

ByteArray::Node::Node(size_t s)
    :ptr(new char[s])
    ,next(nullptr)
    ,size(s){
}

ByteArray::Node::Node()
    :ptr(nullptr)
    ,next(nullptr)
    ,size(0){
}

ByteArray::Node::~Node(){
    if(ptr) {
        delete[] ptr;
    }
}

ByteArray::ByteArray(size_t base_size)
    :m_baseSize(base_size)
    ,m_position(0)
    ,m_capacity(base_size)
    ,m_size(0)
    ,m_endian(LYSLG_BIG_ENDIAN)
    ,m_root(new Node(base_size))
    ,m_cur(m_root){
}

ByteArray::~ByteArray(){
    Node* tmp = m_root;
    while(tmp) {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }
}

// 下面两个函数完全不知道有什么用？？？
bool ByteArray::isLittleEndian() const{
    return m_endian == LYSLG_LITTLE_ENDIAN;
}
void ByteArray::setIsLittleEndian(bool val){
    if(val) {
        m_endian = LYSLG_LITTLE_ENDIAN;
    } else {
        m_endian = LYSLG_BIG_ENDIAN;
    }
}

// ByteArray::write   固定字节
// 固定长度才有字节序？？,可变字节，一个字节一个字节读取，不管字节序？？还是无法理解
void ByteArray::writeFint8(int8_t value){
    write(&value,sizeof(value));
}
void ByteArray::writeFuint8(uint8_t value){
    write(&value,sizeof(value));
}
void ByteArray::writeFint16(int16_t value){
    if(m_endian != LYSLG_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value,sizeof(value));
}
void ByteArray::writeFuint16(uint16_t value){
    if(m_endian != LYSLG_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value,sizeof(value));
}
void ByteArray::writeFint32(int32_t value){
    if(m_endian != LYSLG_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value,sizeof(value));
}
void ByteArray::writeFuint32(uint32_t value){
    if(m_endian != LYSLG_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value,sizeof(value));
}
void ByteArray::writeFint64(int64_t value){
    if(m_endian != LYSLG_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value,sizeof(value));
}
void ByteArray::writeFuint64(uint64_t value){
    if(m_endian != LYSLG_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value,sizeof(value));
}

/*Zigzag编码的规则是：

对于正整数v，将v左移1位（相当于乘以2），得到编码后的值。
对于负整数v，先取绝对值，然后左移1位，最后在结果上减1。*/

/*  Zigzag编码通过将有符号整数的最低位用于表示符号，其余位则用于表示绝对值的两倍。*/
static uint32_t EncodeZigzag32(const int32_t& v){
    if(v < 0) {
        return ((uint32_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

static uint64_t EncodeZigzag64(const int64_t& v){
    if(v < 0) {
        return ((uint64_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}
/*(v >> 1): 这部分右移 v 的二进制表示一位，相当于除以2。这是因为在 Zigzag 编码中，正整数被乘以2，而负整数被乘以2并减去1，所以右移一位是为了还原这个过程。

-(v & 1): 这部分是对 v 的最低位进行按位与操作 &，然后取负数。 (v & 1) 的结果是1或0，表示 v 的最低位是1或0。

如果 (v & 1) 的结果为1，表示 v 是奇数（最低位为1），负号 - 将其变为 -1。

如果 (v & 1) 的结果为0，表示 v 是偶数（最低位为0），负号 - 将其变为 0。

最后，使用异或运算符 ^ 结合这两个结果。这个操作的目的是将右移后的数和取负的结果按位异或，实现还原原始的符号。如果最低位为1，结果就是 -1，如果最低位为0，结果就是 0。*/
static int32_t DecodeZigzag32(const uint32_t& v){
    return(v >>1 ) ^ -(v & 1);
}

static int64_t DecodeZigzag64(const uint64_t& v){
    return(v >> 1) ^ -(v & 1);
}

void ByteArray::writeInt32( int32_t value){
    writeUint32(EncodeZigzag32(value));
}
void ByteArray::writeUint32( uint32_t value){
    uint8_t tmp[5];
    uint8_t i = 0;
    while(value >= 0x80) {
        tmp[i++] = (value & 0x7F) |0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp,i);
}


void ByteArray::writeInt64(int64_t value){
    writeUint64(EncodeZigzag64(value));
}
/*Varint编码的规则是：

*用7个比特（位）存储整数的一部分，其中高位的最高位
用来指示是否还有后续字节。如果最高位为1，表示后续
还有字节；如果最高位为0，表示整数编码结束。
*这样设计的好处是，较小的整数可以用较少的字节表示，
而较大的整数可能需要多个字节。
   对于64位整数，最大值为2^64 - 1，而这个值在二进制
   中需要64位。因此，Varint编码可能需要多个字节来存
   储这个整数，每个字节存储7个比特的整数数据，以及一
   个用于表示是否有后续字节的比特*/
void ByteArray::writeUint64( uint64_t value){
    uint8_t tmp[10];
    uint8_t i = 0;
    // 不满足则只剩下最后的七位
    while(value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp,i);
}

void ByteArray::writeFloat(float value){
    uint32_t v;
    memcpy(&v,&value,sizeof(value));
    writeFuint32(v);
}
void ByteArray::writeDouble(double value){
    uint64_t v;
    memcpy(&v,&value,sizeof(value));
    writeFuint64(v);
}

void ByteArray::writeStringF16(const std::string& value){
    writeFuint16(value.size());
    write(value.c_str(),value.size());
}

void ByteArray::writeStringF32(const std::string& value){
    writeFuint32(value.size());
    write(value.c_str(),value.size());
}

void ByteArray::writeStringF64(const std::string& value){
    writeFuint64(value.size());
    write(value.c_str(),value.size());
}

void ByteArray::writeStringVint(const std::string& value){
    writeFuint64(value.size());
    write(value.c_str(),value.size());
}

void ByteArray::writeStringWithoutLength(const std::string& value){
    write(value.c_str(),value.size());
}


int8_t ByteArray::readFint8(){
    int8_t v;
    read(&v,sizeof(v));
    return v;
}

uint8_t ByteArray::readFuint8(){
    uint8_t v;
    read(&v,sizeof(v));
    return v;
}

#define XX(type) \
    type v; \
    read(&v,sizeof(v)); \
    if(m_endian == LYSLG_BYTE_ORDER) { \
        return v; \
    } else{ \
        return byteswap(v); \
    }


int16_t ByteArray::readFint16(){
    XX(int16_t);
}
uint16_t ByteArray::readFuint16(){
    XX(uint16_t);
}
int32_t ByteArray::readFint32(){
    XX(int32_t);
}
uint32_t ByteArray::readFuint32(){
    XX(uint32_t);
}
int64_t ByteArray::readFint64(){
    XX(int64_t);
}
uint64_t ByteArray::readFuint64(){
    XX(uint64_t);
}

#undef XX

int32_t ByteArray::readInt32(){
    return DecodeZigzag32(readUint32());
}

uint32_t ByteArray::readUint32(){
    uint32_t result = 0;
    for(int i = 0;i<32;i +=7) {
        uint8_t b = readFuint8();
        // 则是最后一个单元块
        if(b < 0x80) {
            result |= ((uint32_t)b) << i;
            break;
        } else {
            // 则是后面依然有数据，(b& 0x7f )这一步去掉最高位的1
            result |= (((uint32_t)(b& 0x7f )) << i);
        }
    }
    return result;
}

int64_t ByteArray::readInt64(){
    return DecodeZigzag64(readUint64());
}
uint64_t ByteArray::readUint64(){
    uint64_t result = 0;
    for(int i = 0;i<64;i +=7) {
        uint8_t b = readFuint8();
        if(b < 0x80) {
            result |= ((uint64_t)b) << i;
            break;
        } else {
            result |= (((uint64_t)(b& 0x7f )) << i);
        }
    }
    return result;
}

float ByteArray::readFloat(){
    uint32_t v = readFuint32();
    float value;
    memcpy(&value,&v,sizeof(v));
    return value;
}
double ByteArray::readDouble(){
    uint64_t v = readFuint64();
    double value;
    memcpy(&value,&v,sizeof(v));
    return value;
}

std::string ByteArray::readStringF16(){
    uint16_t len = readFuint16();
    std::string buff;
    buff.resize(len);
    read(&buff[0],len);
    return buff;
}

std::string ByteArray::readStringF32(){
    uint32_t len = readFuint32();
    std::string buff;
    buff.resize(len);
    read(&buff[0],len);
    return buff;
}

std::string ByteArray::readStringF64(){
    uint64_t len = readFuint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0],len);
    return buff;
}

std::string ByteArray::readStringVint(){
    uint64_t len = readUint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0],len);
    return buff;
}

void ByteArray::clear(){
    m_position = m_size = 0;
    m_capacity = m_baseSize;
    Node* tmp = m_root->next;
    while(tmp) {
        m_cur = tmp;
        tmp = tmp->next;
        delete m_cur;
    }
    m_cur = m_root;
    m_root->next = NULL;
}

void ByteArray::write(const void* buf,size_t size){
    if(size == 0) {
        return;
    }
    addCapacity(size);
    // 计算当前块中可写的起始位置
    size_t npos = m_position % m_baseSize;
    // 计算当前块中容纳量
    size_t ncap = m_cur->size- npos;
    // 写入数组的起始位置
    size_t bpos = 0;

    while(size > 0) {
        if(ncap >= size) {
            memcpy(m_cur->ptr + npos, (const char*)buf + bpos, size);
            if(m_cur->size == (npos + size)) {
                m_cur = m_cur->next;
            }
            m_position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy(m_cur->ptr + npos,(const char*)buf + bpos, ncap);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            m_cur = m_cur->next;
            ncap = m_cur->size;
            npos = 0;
        }
    }

    if(m_position > m_size) {
        m_size = m_position;
    }
}
void ByteArray::read(void* buf,size_t size){
    if(size  > getReadSize()) {
        throw std::out_of_range("not enough len");
    }

    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->size - npos; // 直接使用当前直接快是否不太严谨呢
    size_t bpos = 0;
    while(size > 0) {
        if(ncap >= size) {
            memcpy((char*)buf + bpos , m_cur->ptr + npos, size);
            if(m_cur ->size == (npos + size)) {
                m_cur = m_cur->next;
            }
            m_position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy((char*)buf + bpos,m_cur->ptr + npos, ncap);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            m_cur = m_cur->next;
            ncap = m_cur->size;
            npos = 0;
        }
    }
}

// 这个不会修改成员变量
void ByteArray::read(void* buf,size_t size,size_t position) const{
    if(size  > getReadSize()) {
        throw std::out_of_range("not enough len");
    }

    size_t npos = position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    size_t bpos = 0;
    Node* cur = m_cur; // 这个直接使用当前直接快有问题
    while(size > 0) {
        if(ncap >= size) {
            memcpy((char*)buf + bpos , cur->ptr + npos, size);
            if(cur ->size == (npos + size)) {
                cur = cur->next;
            }
            position += size;
            bpos += size;
            size = 0;
        } else {
            memcpy((char*)buf + bpos,cur->ptr + npos, ncap);
            position += ncap;
            bpos += ncap;
            size -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
    }
}



void ByteArray::setPosition(size_t v){
    if(v > m_capacity) {
        throw std::out_of_range("set_position out of range");
    }
    m_position = v;
    if(m_position > m_size) {
        m_size = m_position;
    }
    // 这里确保m_position与字节块是对应的
    m_cur = m_root;
    while(v > m_cur->size) {
        v -= m_cur->size;
        m_cur = m_cur->next;
    }
    if(v == m_cur->size) {
        m_cur = m_cur->next;
    }
}

bool ByteArray::writeToFile(const std::string& name) const{
    std::ofstream ofs;
    ofs.open(name,std::ios::trunc | std::ios::binary);
    if(!ofs) {
        LYSLG_LOG_ERROR(g_logger) << "writeToFile name=" << name
            << " error, errno=" <<errno << " errstr=" << strerror(errno);
        return false;
    }

    int64_t read_size = getReadSize();
    int64_t pos = m_position;
    Node* cur = m_cur;

    while(read_size > 0) {
        int diff = pos % m_baseSize;
        int64_t len = (read_size > (int64_t)m_baseSize ? m_baseSize : read_size) - diff;
        ofs.write(cur->ptr + diff, len);
        cur = cur->next;
        pos += len;
        read_size -= len;
    }
    return true;
}
bool ByteArray::readFromFile(const std::string& name){
    std::ifstream ifs;
    ifs.open(name,std::ios::binary);
    if(!ifs) {
        LYSLG_LOG_ERROR(g_logger) << "readFromFile name=" << name
            << " error, errno=" <<errno << " errstr=" << strerror(errno);
        return false;
    }
    /*字符数组是通过 new[] 运算符动态分配的，而 delete 无法正确释放通过 new[] 分配的数组内存。对于数组的释放，应该使用 delete[]。*/
    std::shared_ptr<char> buff(new char[m_baseSize],[](char* ptr) {delete[] ptr;});
    
    /*将读取的数据写入到 ByteArray 对象中。使用 ifs.gcount() 获取实际读取的字节数。*/
    while(!ifs.eof()) {
        ifs.read(buff.get(),m_baseSize);
        write(buff.get(),ifs.gcount());
    }
    return true;
}

void ByteArray::addCapacity(size_t size){
    if(size == 0) {
        return;
    }

    size_t old_cap = getCapacity();
    if(old_cap >= size) {
        return;
    }

    size = size - old_cap;
    /*ceil 函数将浮点数结果向上取整，得到一个大于或等于除法结果的最小整数。*/
    size_t count = ceil(1.0 * size / m_baseSize);
    // size_t count = (size / m_baseSize) + (((size % m_baseSize) > old_cap) ? 1 : 0);
    Node* tmp = m_root;
    while(tmp->next) {
        tmp = tmp->next;
    }

    Node* first = NULL;
    for(size_t i = 0;i<count;i++) {
        tmp->next = new Node(m_baseSize);
        if(first == NULL) {
            first = tmp->next;
        }
        tmp = tmp->next;
        m_capacity += m_baseSize;
    }

    if(old_cap == 0) {
        m_cur = first;
    }
}

std::string ByteArray::toString() const{
    std::string str;
    str.resize(getReadSize());
    if(str.empty()) {
        return str;
    }
    read(&str[0],str.size(),m_position);
    return str;
}
/*std::setw(2)：设置输出的字段宽度为 2，确保输出的内容至少占用两个字符的宽度*/
/*设置填充字符为 '0'，即在不足宽度时使用 '0' 进行填充。*/
/*将 str[i] 强制转换为 uint8_t 类型，然后再将其强制转换为 int 类型。这是为了确保以无符号的方式获取字节值，避免出现符号扩展的问题。*/

/*将每个字节以十六进制形式输出到字符串流 ss 中。std::setw(2) 和 
std::setfill('0') 用于确保输出的宽度为两个字符，不足两个字符的部分用 '0' 进行填充。*/
std::string ByteArray::toHexString() const{
    std::string str = toString();
    std::stringstream ss;

    for(size_t i = 0;i <str.size();++i) {
        if(i>0 && i % 32 == 0) {
            ss << std::endl;
        }
        
        ss << std::setw(2) << std::setfill('0') << std::hex 
            << (int)(uint8_t)str[i] << " ";
    }
    return ss.str();
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers,uint64_t len){
    len = len > getReadSize() ? getReadSize() : len;
    if(len == 0) {
        return 0;
    }

    uint64_t size = len;
    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    struct iovec iov;
    Node* cur = m_cur;

    while(len > 0) {
        if(ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

/*struct iovec {
    // Starting address (内存起始地址）
    void  *iov_base;   

    // Number of bytes to transfer（这块内存长度） 
    size_t iov_len;  
};*/
uint64_t  ByteArray::getReadBuffers(std::vector<iovec>& buffers,uint64_t len,uint64_t position) const{
    len = len > getReadSize() ? getReadSize() : len;
    if(len == 0) {
        return 0;
    }

    uint64_t size = len;
    size_t npos = position % m_baseSize;

    size_t count = position / m_baseSize;
    Node* cur = m_root;
    while(count>0) {
        cur = cur->next;
        count--;
    }

    size_t ncap = m_cur->size - npos;
    struct iovec iov;
    

    while(len > 0) {
        if(ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}
uint64_t  ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t len){
    if(len == 0) {
        return 0;
    }
    addCapacity(len);
    uint64_t size = len;

    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->size - npos;
    struct iovec iov;
    Node* cur = m_cur;

    while(len > 0) {
        if(ncap >= len) {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->size;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}



}