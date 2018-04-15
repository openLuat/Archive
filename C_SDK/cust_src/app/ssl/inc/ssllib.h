#ifndef __SSL_LIB_H__
#define __SSL_LIB_H__

/*
 * ssllib.h接口大部分是将移植过来的axtls源码部分接口再次封装，主要是使用了一些默认输入参数。
 * 除了必须调用SSL_RegSocketCallback外，其他接口均可以不使用，而只使用ssl.h里的接口。
 * ssl.h的接口更全面，如果ssllib.h里的函数无法满足用户要求，请使用ssl.h里的原始接口
 * 特性：见 http://axtls.sourceforge.net/
 */

#include "os_port.h"
#include "ssl.h"
typedef int(*SocketAPI)(int SocketFd, void *Buf, uint16_t Len);
extern EXP_FUNC int STDCALL new_pem_obj(SSL_CTX *ssl_ctx, int is_cacert, char *where,
        int remain, const char *password);
extern EXP_FUNC int STDCALL do_obj(SSL_CTX *ssl_ctx, int obj_type,
        SSLObjLoader *ssl_obj, const char *password);
/**
 * @brief 注册SSL的数据收发函数，由于默认使用socket形式，为方便AT指令用户，提供此接口，将数据的收发交由AT指令完成
 * @param SendFun 发送函数
 * @param ReceiveFun 接收函数
 * @return  无
 */
void SSL_RegSocketCallback(SocketAPI SendFun, SocketAPI ReceiveFun);

/**
 * @brief 创建一个SSL控制结构体，没有默认KEY，需要之后用SSL_LoadKey来加载证书内容
 * @param NumSessions 最多允许缓存的Session，可以为0
 * @return  一个SSL控制结构体的地址指针
 */
SSL_CTX * SSL_CreateCtrl(uint16_t NumSessions);

/**
 * @brief 删除一个SSL控制结构体
 * @param SSLCtrl 控制结构体的地址指针
 * @return 无
 */
void SSL_FreeCtrl(SSL_CTX *SSLCtrl);

/**
 * @brief 创建一个SSL连接结构体，并开始SSL握手过程。
 * @param SSLCtrl [in] SSL控制结构体.
 * @param ClientID [in] 如果是socket，则是socketID，如果是AT指令，单路链接填0，多路链接填通道号.
 * @param SessionID [in] 一个最多32字节的SessionID，如果启动了Session缓存才有效，用于恢复已经保存的session
 * @param SessionIDSize SessionID长度
 * @param Hostname [in] 用于验证的域名，大部分情况下不需要，填NULL
 * @param MaxFragmentSize [in] 最大片段长度，大部分情况下不需要，填NULL，可以填的值只有2^9, 2^10 .. 2^14
 * @return 返回一个SSL连接结构体的地址指针
 */
SSL * SSL_NewLink(
		SSL_CTX *SSLCtrl,
		int32_t ClientID,
		const uint8_t *SessionID,
		uint8_t SessionIDSize,
		const char **Hostname,
		uint16_t *MaxFragmentSize);

/**
 * @brief 重新开始SSL握手过程。
 * @param SSLLink [in] SSL连接结构体.
 * @return 0表示成功，其他失败
 */
int32_t SSL_ReHandshake(SSL *SSLLink);

/**
 * @brief 返回握手结果.
 * @param SSLLink [in] SSL连接结构体的地址指针.
 * @return 0表示成功，其他失败
 */
int32_t SSL_HandshakeStatus(const SSL *SSLLink);

/**
 * @brief 删除一个SSL连接结构体，在删除前，如果没有发送过关闭通知，会自动对连接着的服务器发送关闭通知
 * @param SSLLink SSL连接结构体的地址指针
 * @return 无
 */
void SSL_FreeLink(SSL *SSLLink);

/**
 * @brief 在握手完成后，接收数据必须使用此接口，从而获取解密后的数据
 * @param SSLLink [in] SSL连接结构体的地址指针.
 * @param InData [out] 解密后的数据指针地址，注意，该参数不需要malloc空间，也不要free空间，
 * 非NULL表示有数据，NULL表示没有数据。
 * @return  >0 解密后数据长度, =0 数据还没有接收完，需要再次读取， <0 有错误
 */
int32_t SSL_Read(SSL *SSLLink, uint8_t **InData);

/**
 * @brief 在握手完成后，发送数据必须使用此接口，输入未加密的数据，并加密发送，注意该接口阻塞发送
 * @param SSLLink [in] SSL连接结构体的地址指针.
 * @param OutData [in] 需要发送的未加密数据指针
 * @param OutLen [in] 需要发送的未加密数据长度.
 * @return >0实际发送的长度 <0 有错误
 */
int32_t SSL_Write(SSL *SSLLink, const uint8_t *OutData, uint16_t OutLen);

/**
 * @brief 加载证书或者RSAkey，如果是文件形式，需要预先从文件里读取全部原始数据到内存
 * @param SSLCtrl [in] SSLCtrl 控制结构体的地址指针
 * @param Type [in] 加载的数据类型，只能是
 * - SSL_OBJ_X509_CERT (no password required) 客户端的证书，用于服务器验证客户端，大部分情况不需要。目前涉及到银行金融时可能需要
 * - SSL_OBJ_X509_CACERT (no password required) 验证服务器证书用的CA证书，基本上是必须的
 * - SSL_OBJ_RSA_KEY (AES128/AES256 PEM encryption supported)
 * - SSL_OBJ_PKCS8 (RC4-128 encrypted data supported)
 * - SSL_OBJ_PKCS12 (RC4-128 encrypted data supported).
 * @param Data [in] 加载的数据指针，可以是16进制数据，也可以是ASCII字符串.
 * @param Len [in] 数据长度
 * @param Password [in] 如果是加密的证书需要密码来解密，否则填NULL
 * @return 0成功 其他失败
 */
int32_t SSL_LoadKey(SSL_CTX *SSLCtrl, int32_t Type, const uint8_t *Data, int32_t Len, const int8_t *Password);

/**
 * @brief 验证服务器证书，比原版改进在于可以使用次级根证书来验证，一般浏览器内置了大量次级根证书，可以导出使用，在测试实际网页时，适合使用本函数
 * @param SSLLink SSL连接结构体的地址指针
 * @return 0成功 其他失败
 */
int32_t SSL_VerifyCert(SSL *SSLLink);
#endif
