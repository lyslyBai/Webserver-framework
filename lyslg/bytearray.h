#ifndef __LYSLG_BYTEARRAY_H__
#define __LYSLG_BYTEARRAY_H__


#include <unistd.h>
#include <string.h>
#include <memory>
#include <vector>
#include <sys/uio.h>

namespace lyslg{



class ByteArray{
public:

    typedef std::shared_ptr<ByteArray> ptr;

    struct Node {
        Node(size_t s);
        Node();

        ~Node();
        /// 内存块地址指针
        char* ptr;
        Node* next;
        size_t size;
    };

    ByteArray(size_t base_size = 4096);
    ~ByteArray();

    // write   固定字节
    void writeFint8( int8_t value);
    void writeFuint8( uint8_t value);
    void writeFint16( int16_t value);
    void writeFuint16( uint16_t value);
    void writeFint32( int32_t value);
    void writeFuint32( uint32_t value);
    void writeFint64( int64_t value);
    void writeFuint64( uint64_t value);

    void writeInt32( int32_t value);
    void writeUint32( uint32_t value);
    void writeInt64( int64_t value);
    void writeUint64( uint64_t value);

    void writeFloat( float value);
    void writeDouble( double value);
    // length:int16 data
    /**
     * @brief 写入std::string类型的数据,用uint16_t作为长度类型
     * @post m_position += 2 + value.size()
     *       如果m_position > m_size 则 m_size = m_position
     */
    void writeStringF16(const std::string& value);
    // length:int32 data
    void writeStringF32(const std::string& value);
    // length:int64 data
    void writeStringF64(const std::string& value);
    // length:Varint data
    void writeStringVint(const std::string& value);

    // data
    void writeStringWithoutLength(const std::string& value);

    // read

    int8_t readFint8();
    uint8_t readFuint8();
    int16_t readFint16();
    uint16_t readFuint16();
    int32_t readFint32();
    uint32_t readFuint32();
    int64_t readFint64();
    uint64_t readFuint64();

    int32_t readInt32();
    uint32_t readUint32();
    int64_t readInt64();
    uint64_t readUint64();

    float readFloat();
    double readDouble();
    // length:int16 data
    std::string readStringF16();
    // length:int32 data
    std::string readStringF32();
    // length:int64 data
    std::string readStringF64();
    // length:Varint data
    std::string readStringVint();

    // 内部操作
    void clear();
    /**
     * @brief 写入size长度的数据
     * @param[in] buf 内存缓存指针
     * @param[in] size 数据大小
     * @post m_position += size, 如果m_position > m_size 则 m_size = m_position
     */
    void write(const void* buf,size_t size);
    /**
     * @brief 读取size长度的数据
     * @param[out] buf 内存缓存指针
     * @param[in] size 数据大小
     * @post m_position += size, 如果m_position > m_size 则 m_size = m_position
     * @exception 如果getReadSize() < size 则抛出 std::out_of_range
     */
    void read(void* buf,size_t size);
    /**
     * @brief 读取size长度的数据
     * @param[out] buf 内存缓存指针
     * @param[in] size 数据大小
     * @param[in] position 读取开始位置
     * @exception 如果 (m_size - position) < size 则抛出 std::out_of_range
     */
    void read(void* buf,size_t size,size_t position) const;
    /**
     * @brief 返回ByteArray当前位置
     */
    size_t getPosition() const {return m_position;}
    void setPosition(size_t v);

    bool writeToFile(const std::string& name) const;
    bool readFromFile(const std::string& name);

    size_t getBaseSize() const {return m_baseSize;}
    /**
     * @brief 返回可读取数据大小
     */
    size_t getReadSize() const {return m_size - m_position;}

    bool isLittleEndian() const;
    void setIsLittleEndian(bool val);

    std::string toString() const;
    std::string toHexString() const;

    uint64_t getReadBuffers(std::vector<iovec>& buffers,uint64_t len = ~0ull);
    uint64_t getReadBuffers(std::vector<iovec>& buffers,uint64_t len,uint64_t position) const;
    uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);

    size_t getSize() const {return m_size;}
private:
    /**
     * @brief 扩容ByteArray,使其可以容纳size个数据(如果原本可以可以容纳,则不扩容)
     */
    void addCapacity(size_t size);
    /**
     * @brief 获取当前的可写入容量
     */
    size_t getCapacity() const {return m_capacity - m_position;}
private:
    /// 内存块的大小
    size_t m_baseSize;
    /// 当前操作位置
    size_t m_position;
    /// 当前的总容量
    size_t m_capacity;
    /// 当前数据的大小
    size_t m_size;
    /// 字节序,默认大端
    int8_t m_endian;
    /// 第一个内存块指针
    Node* m_root;
    /// 当前操作的内存块指针
    Node* m_cur;
};

}


#endif
