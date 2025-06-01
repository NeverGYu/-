#include "bytearray.hpp"
#include "endian.hpp"
#include "log.h"
#include <string.h>
#include <math.h>
#include <fstream>
#include <iomanip>

sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

namespace sylar
{

/*------------------------------------  Node  ------------------------------------*/
ByteArray::Node::Node()
    : pos(nullptr)
    , length(0)
    , next(nullptr)
{}

ByteArray::Node::Node(size_t len)
    : pos(new char[len])
    , length(len)
    , next(nullptr)
{}

ByteArray::Node::~Node()
{
    if (pos)
    {
        delete [] pos;
    }
}


/*------------------------------------  ByteArray构造函数  ------------------------------------*/
ByteArray::ByteArray(size_t base_size)
    : m_baseSize(base_size)
    , m_position(0)
    , m_capacity(base_size)
    , m_size(0)
    , m_endian(SYLAR_BIG_ORDER)
    , m_root(new Node(base_size))
    , m_cur(m_root)
{}

ByteArray::~ByteArray()
{
    Node* tmp = m_root;
    while (tmp)
    {
        Node* cur = tmp;
        tmp = tmp->next;
        delete cur;
    }
}

/*------------------------------------  判断是否是小端序  ------------------------------------*/
bool ByteArray::isLittleEndian() const
{
    return m_endian == SYLAR_LITTLE_ORDER;
}

void ByteArray::setIsLittleEndian(bool val)
{
    if (val)
    {
        m_endian = SYLAR_LITTLE_ORDER;
    }
    else
    {
        m_endian = SYLAR_BIG_ORDER;
    }
}


/*------------------------------------  ZigZag ------------------------------------*/

/**
 * @brief 将有符号整数（int）转成更容易压缩的无符号整数
 */
static uint32_t EncodingZigZag32(const int32_t val)
{
    if (val < 0)
    {
        return (uint32_t(-val) * 2 - 1);
    }
    else
    {
        return 2 * val;
    }
}

static uint64_t EncodingZigZag64(const int64_t val)
{
    if (val < 0)
    {
        return (uint32_t(-val) * 2 - 1);
    }
    else
    {
        return 2 * val;
    }
}

/**
 *  @brief 还原有符号整数
 *  @details ^ 相同为0，不同为1
 */
static int32_t DecodingZigZag32(uint32_t val)
{
    return (val >> 1) ^ -(val & 1);
}

static int64_t DecodeZigzag64(const uint64_t& v) 
{
    return (v >> 1) ^ -(v & 1);
}


/*------------------------------------  读写相关操作  ------------------------------------*/
void ByteArray::write(const void* buf, size_t size)
{
    // 如果写入的数据长度小于0，则直接返回
    if (size <= 0)
    {
        return;
    }
    // 否则，先增加容量
    addCapacity(size);
    // 这代表当前结点开始存储的位置
    size_t npos = m_position % m_baseSize;
    // 这代表当前结点可用的容量
    size_t ncap = m_cur->length - npos;
    // 从源 buf 中已写入的偏移量
    size_t bpos = 0;
    // 通过循环来处理size的长度超过当前结点的可以保存数据的长度
    while (size > 0)
    {   
        // 这表示当前结点的剩余容量可以继续写入长度为size的数据
        if (ncap >= size)
        {
            // 将数据写入到当前结点对应数据存储的位置
            memcpy(m_cur->pos + npos, (const char*)buf + bpos, size);
            // 这表示当前结点以及存储满了数据，那么就切换到下一个结点来进行存储
            if (m_cur->length == (npos + size))
            {
                m_cur = m_cur->next;
            }
            // 记录已经存储的数据大小
            m_position += size;
            // 记录偏移量
            bpos += size;
            // 这个size用于跳出循环，表示当前结点的容量可以存储完数据
            size = 0;
        }
        // 这表示剩余容量不可以继续写入长度为size的数据
        else
        {
            // 将当前结点的数据部分存满
            memcpy(m_cur->pos + npos, (const char*)buf + bpos, ncap);
            // 更新总的已经存放了的数据长度
            m_position += ncap;
            // 更新当前buf的偏移量
            bpos += ncap;
            // 更新还需要存放的数据长度
            size -= ncap;
            // 切换到下一个结点
            m_cur = m_cur->next;
            // 获得下一个结点的可用容量
            ncap = m_cur->length;
            // 将偏移量置于0
            npos = 0;
        }
    }
    if (m_position > m_size)
    {
        m_size = m_position;
    }
}

void ByteArray::read(void* buf, size_t size)
{
    // 如果可读数据小于size，则直接返回
    if (getReadSize() < size)
    {
        throw std::out_of_range("not enough len");
    }
    // 否则
    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->length - npos;
    size_t bpos = 0;
    while(size > 0)
    {
        if (ncap >= size)
        {
            memcpy((char*)buf + bpos, m_cur->pos + npos, size);
            if (m_cur->length == (npos + size))
            {
                m_cur = m_cur->next;
            }
            m_position += size;
            bpos += size;
            size = 0;
        }
        else
        {
            memcpy((char*)buf + bpos, m_cur->pos + npos, ncap);
            m_position += ncap;
            bpos += ncap;
            size -= ncap;
            m_cur = m_cur->next;
            ncap = m_cur->length;
            npos = 0;
        }
    } 
}

void ByteArray::read(void* buf, size_t size, size_t position) const {
    if(size > (m_size - position)) 
    {
        throw std::out_of_range("not enough len");
    }

    size_t npos = position % m_baseSize;
    size_t ncap = m_cur->length - npos;
    size_t bpos = 0;
    Node* cur = m_cur;
    while(size > 0) 
    {
        if(ncap >= size) 
        {
            memcpy((char*)buf + bpos, cur->pos + npos, size);
            if(cur->length == (npos + size)) 
            {
                cur = cur->next;
            }
            position += size;
            bpos += size;
            size = 0;
        } 
        else 
        {
            memcpy((char*)buf + bpos, cur->pos + npos, ncap);
            position += ncap;
            bpos += ncap;
            size -= ncap;
            cur = cur->next;
            ncap = cur->length;
            npos = 0;
        }
    }
}

void ByteArray::writeFint8(int8_t value)
{
    write(&value, sizeof(value));
}

void ByteArray::writeFuint8(uint8_t value)
{
    write(&value, sizeof(value));
}

void ByteArray::writeFint16(int16_t value)
{
    if (m_endian != SYLAR_BYTE_ORDER)
    {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint16(uint16_t value)
{
    if (m_endian != SYLAR_BYTE_ORDER)
    {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint32 (int32_t value) 
{
    if(m_endian != SYLAR_BYTE_ORDER) 
    {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint32(uint32_t value)
 {
    if(m_endian != SYLAR_BYTE_ORDER) 
    {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint64 (int64_t value) 
{
    if(m_endian != SYLAR_BYTE_ORDER) 
    {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint64(uint64_t value) 
{
    if(m_endian != SYLAR_BYTE_ORDER) 
    {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeUint32(uint32_t value)
{
    uint8_t tmp[5];
    uint8_t i = 0;
    while (value >= 0x80)
    {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeInt32(int32_t value)
{
    writeUint32(EncodingZigZag32(value));
}

void ByteArray::writeUint64 (uint64_t value) 
{
    uint8_t tmp[10];
    uint8_t i = 0;
    while(value >= 0x80) 
    {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeInt64(int64_t value)
{
    writeUint64(EncodingZigZag64(value));
}

void ByteArray::writeFloat(float value)
{
    uint32_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint32(v);
}

void ByteArray::writeDouble (double value) 
{
    uint64_t v;
    memcpy(&v, &value, sizeof(value));
    writeFuint64(v);
}

void ByteArray::writeStringF16(const std::string& value) 
{
    writeFuint16(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringF32(const std::string& value)
{
    writeFuint32(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringF64(const std::string& value)
{
    writeFuint64(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringVint(const std::string& value)
{
    writeUint64(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringWithoutLength(const std::string& value)
{
    write(value.c_str(), value.size());
}

int8_t ByteArray::readFint8() 
{
    int8_t v;
    read(&v, sizeof(v));
    return v;
}

uint8_t ByteArray::readFuint8() 
{
    uint8_t v;
    read(&v, sizeof(v));
    return v;
}

#define XX(type) \
    type v; \
    read(&v, sizeof(v)); \
    if(m_endian == SYLAR_BYTE_ORDER) { \
        return v; \
    } else { \
        return byteswap(v); \
    }

int16_t ByteArray::readFint16() 
{
    XX(int16_t);
}

uint16_t ByteArray::readFuint16() 
{
    XX(uint16_t);
}

int32_t ByteArray::readFint32() 
{
    XX(int32_t);
}

uint32_t ByteArray::readFuint32() 
{
    XX(uint32_t);
}

int64_t ByteArray::readFint64() 
{
    XX(int64_t);
}

uint64_t ByteArray::readFuint64() 
{
    XX(uint64_t);
}

#undef XX

int32_t ByteArray::readInt32() 
{
    return DecodingZigZag32(readUint32());
}

uint32_t ByteArray::readUint32() 
{
    uint32_t result = 0;
    for(int i = 0; i < 32; i += 7) 
    {
        uint8_t b = readFuint8();
        if(b < 0x80) 
        {
            result |= ((uint32_t)b) << i;
            break;
        } 
        else 
        {
            result |= (((uint32_t)(b & 0x7f)) << i);
        }
    }
    return result;
}

int64_t  ByteArray::readInt64() 
{
    return DecodeZigzag64(readUint64());
}

uint64_t ByteArray::readUint64() 
{
    uint64_t result = 0;
    for(int i = 0; i < 64; i += 7) 
    {
        uint8_t b = readFuint8();
        if(b < 0x80)
         {
            result |= ((uint64_t)b) << i;
            break;
        } 
        else 
        {
            result |= (((uint64_t)(b & 0x7f)) << i);
        }
    }
    return result;
}

float ByteArray::readFloat() 
{
    uint32_t v = readFuint32();
    float value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

double ByteArray::readDouble() 
{
    uint64_t v = readFuint64();
    double value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

std::string ByteArray::readStringF16() 
{
    uint16_t len = readFuint16();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringF32() 
{
    uint32_t len = readFuint32();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringF64() 
{
    uint64_t len = readFuint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringVint() 
{
    uint64_t len = readUint64();
    std::string buff;
    buff.resize(len);
    read(&buff[0], len);
    return buff;
}


/*------------------------------------  辅助函数  ------------------------------------*/
void ByteArray::clear()
{
    m_position = m_size = 0;
    m_capacity = m_baseSize;
    Node* tmp = m_root;
    while(tmp)
    {
        Node* cur = tmp;
        tmp = tmp->next;
        delete cur;
    }
    m_cur = m_root;
    m_root->next = nullptr;
}

void ByteArray::setPosition(size_t v)
{
    if (v > m_capacity)   
    {
        throw std::out_of_range("set position out of range");
    }
    m_position = v;
    if (m_position > m_size)
    {
        m_size = m_position;
    }
    m_cur = m_root;
    while(v > m_cur->length)
    {
        v -= m_cur->length;
        m_cur = m_cur->next;
    }
    if (v == m_cur->length)
    {
        m_cur = m_cur->next;
    }
}

bool ByteArray::writeToFile(const std::string& name)
{
    std::ofstream ofs;
    ofs.open(name, std::ios::trunc | std::ios::binary);
    if (!ofs)
    {
        SYLAR_LOG_ERROR(g_logger) << "writeToFile name=" << name
                                  << " error , errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    // 剩余可读的字节数（从 m_position 开始到 m_size）
    int read_size = getReadSize();
    // 当前读取位置
    int64_t pos = m_position;
    // 当前块
    Node* cur = m_cur;
    // 判断剩余可读的字节数是否大于0
    while (read_size > 0)
    {
        int diff = pos % m_baseSize;
        int len = (read_size > m_baseSize ? m_baseSize : read_size) - diff;
        ofs.write(cur->pos, len);
        cur = cur->next;
        pos += len;
        read_size -= len;
    }
    return true;
}

bool ByteArray::readFromFile(const std::string& name) 
{
    // 定义文件输入流
    std::ifstream ifs;
    // 打开文件name
    ifs.open(name, std::ios::binary);
    if(!ifs) 
    {
        SYLAR_LOG_ERROR(g_logger) << "readFromFile name=" << name
            << " error, errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    // 定义一个智能指针指向一个数组
    std::shared_ptr<char> buff(new char[m_baseSize], [](char* ptr) { delete[] ptr;});
    // 循环读取该文件直到碰到文件结束符EOF
    while(!ifs.eof()) 
    {
        // 将文件内容读取到buff
        ifs.read(buff.get(), m_baseSize);
        // 再将buff写入到ByteArray中
        write(buff.get(), ifs.gcount());
    }
    return true;
}

void ByteArray::addCapacity(size_t size)
 {
    // 如果size == 0 直接返回
    if(size == 0)
    {
        return;
    }
    // 获得当前可写入的容量
    size_t old_cap = getCapacity();
    // 判断剩余容量是否大于数据块的长度
    if(old_cap >= size) 
    {
        return;
    }
    // 获得填充完整个容量后海剩余的数据块长度
    size = size - old_cap;
    // 查看好需要几个结点来存储剩余数据
    size_t count = std::ceil(1.0 * size / m_baseSize);
    // 遍历到最后一个结点
    Node* tmp = m_root;
    while(tmp->next)
    {
        tmp = tmp->next;
    }
    // 创建count数量的结点
    Node* first = NULL;
    for(size_t i = 0; i < count; ++i)
    {
        tmp->next = new Node(m_baseSize);
        if(first == NULL) 
        {
            // first是tmp之后的第一个结点
            first = tmp->next;
        }
        tmp = tmp->next;
        m_capacity += m_baseSize;
    }

    if(old_cap == 0) 
    {
        m_cur = first;
    }
}

std::string ByteArray::toString() const 
{
    std::string str;
    str.resize(getReadSize());
    if(str.empty()) 
    {
        return str;
    }
    read(&str[0], str.size(), m_position);
    return str;
}

std::string ByteArray::toHexString() const 
{
    std::string str = toString();
    std::stringstream ss;

    for(size_t i = 0; i < str.size(); ++i) 
    {
        if(i > 0 && i % 32 == 0) 
        {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex
           << (int)(uint8_t)str[i] << " ";
    }

    return ss.str();
}


uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers, uint64_t len) const 
{
    // 判断要读取的长度是否大于可读缓冲区的长度，是->可读缓冲区的长度，不是->要读取的长度
    len = len > getReadSize() ? getReadSize() : len;
    if(len == 0) 
    {
        return 0;
    }

    uint64_t size = len;
    // 获得结点上的偏移量
    size_t npos = m_position % m_baseSize;
    // 获得节点上的剩余容量
    size_t ncap = m_cur->length - npos;
    // 定义结构体iovec，用来保存缓存地址以及长度
    struct iovec iov;
    // 获得当前结点
    Node* cur = m_cur;
    while(len > 0) {
        if(ncap >= len) 
        {
            iov.iov_base = cur->pos + npos;
            iov.iov_len = len;
            len = 0;
        } 
        else 
        {
            iov.iov_base = cur->pos + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->length;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getReadBuffers(std::vector<iovec>& buffers,uint64_t len, uint64_t position) const 
{
    len = len > getReadSize() ? getReadSize() : len;
    if(len == 0) 
    {
        return 0;
    }

    uint64_t size = len;

    size_t npos = position % m_baseSize;
    size_t count = position / m_baseSize;
    Node* cur = m_root;
    while(count > 0) 
    {
        cur = cur->next;
        --count;
    }

    size_t ncap = cur->length - npos;
    struct iovec iov;
    while(len > 0) {
        if(ncap >= len) 
        {
            iov.iov_base = cur->pos + npos;
            iov.iov_len = len;
            len = 0;
        } 
        else 
        {
            iov.iov_base = cur->pos + npos;
            iov.iov_len = ncap;
            len -= ncap;
            cur = cur->next;
            ncap = cur->length;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

uint64_t ByteArray::getWriteBuffers(std::vector<iovec>& buffers, uint64_t len) 
{
    if(len == 0) 
    {
        return 0;
    }
    addCapacity(len);
    uint64_t size = len;

    size_t npos = m_position % m_baseSize;
    size_t ncap = m_cur->length - npos;
    struct iovec iov;
    Node* cur = m_cur;
    while(len > 0) 
    {
        if(ncap >= len) 
        {
            iov.iov_base = cur->pos + npos;
            iov.iov_len = len;
            len = 0;
        } 
        else 
        {
            iov.iov_base = cur->pos+ npos;
            iov.iov_len = ncap;

            len -= ncap;
            cur = cur->next;
            ncap = cur->length;
            npos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}


}