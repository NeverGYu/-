#pragma once 

#include <memory>
#include <string>

namespace sylar{
namespace ssl{

enum class SslVersion
{
    TLS_1_0,
    TLS_1_1,
    TLS_1_2,
    TLS_1_3,
};

enum class SslError
{
    NONE,
    WANT_READ,
    WANT_WRITE,
    SYSCALL,
    SSL,
    UNKNOWN
};

// SSL 状态
enum class SSLState 
{
    HANDSHAKE,
    ESTABLISHED,
    SHUTDOWN,
    ERROR
};

class SslConfig{
public:
    using ptr = std::shared_ptr<SslConfig>;

    /**
     *  @brief 构造函数 
     */
    SslConfig()
        : m_version(SslVersion::TLS_1_2)
        , m_cipherList("HIGH:!aNULL:!MDS")
        , m_verifyClient(false)
        , m_verifyDepth(4)
        , m_sessionTimeout(300)
        , m_sessionCacheSize(20480L)
    {};

    /**
     *  @brief 析构函数 
     */
    ~SslConfig() {}

    /**
     *  @brief 设置证书
     *  @param[in] certFile 证书文件 
     */
    void setCertificateFile(const std::string& certFile) { m_certfile = certFile; }

    /**
     *  @brief 获取证书
     */
    const std::string& getCertificateFile() const { return m_certfile; }
    
    /**
     *  @brief 设置私钥
     *  @param[in] keyFile 私钥 
     */
    void setPrivateKeyFile(const std::string& keyFile) { m_keyfile = keyFile; }

    /**
     *  @brief 获取私钥 
     */
    const std::string& getPrivateKeyFile() const { return m_keyfile; }
    
    /**
     *  @brief 设置证书链
     *  @param[in] chainFile 证书链 
     */
    void setCertificateChainFile(const std::string& chainFile) { m_chainfile = chainFile; }

    /**
     *  @brief 获取证书链
     */
    const std::string& getCertificateChainFile() const { return m_chainfile; }

    /**
     *  @brief 设置协议版本
     *  @param[in] version 
     */
    void setVersion(SslVersion version) { m_version = version; }

    /**
     *  @brief 获取协议版本 
     */
    SslVersion getVersion() const { return m_version; }
    
    /**
     *  @brief 设置加密套件
     *  @param[in] cipherlist 
     */
    void setCipherList(const std::string& cipherlist) { m_cipherList = cipherlist; }

    /**
     *  @brief 获取加密套件 
     */
    const std::string& getCipherList() const { return m_cipherList; }

    /**
     *  @brief 设置验证客户端
     *  @param[in] verify
     */
    void setVerifyClient(bool verify) { m_verifyClient = verify; }

    /**
     *  @brief 获取验证客户端
     */
    bool getVerifyClient() const { return m_verifyClient; }

    /**
    *  @brief 设置验证等级
     *  @param[in] depth
     */
    void setVerifyDepth(int depth) { m_verifyDepth = depth; }

    /**
     *  @brief 获取验证等级
     */
    int getVerifyDepth() const { return m_verifyDepth; }

    /**
     *  @brief 设置会话超时时间
     *  @param[in] seconds
     */
    void setSessionTimeout(int seconds) { m_sessionTimeout = seconds; }

    /**
     *  @brief 获取会话超时时间
     */
    int getSessionTimeout() const { return m_sessionTimeout; }

    /**
     *  @brief 设置会话缓存大小
     *  @param[in] size  
     */
    void setSessionCacheSize(long size) { m_sessionCacheSize = size; }

    /**
     *  @brief 获取会话缓存大小
     */
    long getSessionCacheSize() const { return m_sessionCacheSize; }

private:
    std::string m_certfile;     // 证书文件
    std::string m_keyfile;      // 私钥文件
    std::string m_chainfile;    // 证书链文件
    SslVersion m_version;       // 协议版本
    std::string m_cipherList;   // 加密套件
    bool m_verifyClient;        // 验证客户端
    int m_verifyDepth;          // 验证等级
    int m_sessionTimeout;       // 会话超时时间
    long m_sessionCacheSize;    // 会话缓存大小
};

}
}