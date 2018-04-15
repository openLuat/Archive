#ifndef __IOT_SOCKET_H__
#define __IOT_SOCKET_H__

#include "am_openat.h"

/**
 * @defgroup iot_sdk_socket socket接口
 * @{
 */
/**@example demo_socket/src/demo_socket.c
* socket接口示例
*/
/**创建socket
*@param		domain:		仅支持AF_INET (IPV4 网络协议)
@param		type:		支持SOCK_STREAM/SOCK_DGRAM，分别表示TCP、UDP连接
@param		protocol:   仅支持0


*@return	>=0: 	    socket描述符，用于后续操作
*           <0:         创建socket失败
*@note      创建的socket不用后需要用close将其关闭
**/

int socket(int domain, int type, int protocol);
/**获取域名对应的IP地址
*@param		name:		域名，例如:www.airm2m.com/www.baidu.com
*@return	struct hostent 结构体: 该结构体包含了一个DNS域名解析后的ip地址
*           NULL:  域名解析失败
**/                       
struct hostent* gethostbyname(const char *name);
/**关闭socket
*@param		fd:	调用socket接口返回的socket描述符
*@return	0:  表示成功
            -1  表示有错误
*           
**/                          
int close (int fd);
/**设置socket的属性
*@param		socketfd:	调用socket接口返回的socket描述符
@param      level: 支持SOL_SOCKET/IPPROTO_TCP
@param      optname:  SOL_SOCKET对应optname为 SO_DEBUG/SO_OOBINLINE/SO_SNDTIMEO/SO_RCVTIMEO/SO_RCVBUF/SO_SNDBUF
                      IPPROTO_TCP对应optname为 SO_TCP_SACKDISABLE/SO_TCP_NODELAY
@param      optval_p:
@param      optlen:
*@return	0:  表示成功
            <0  表示有错误
*
**/          

int setsockopt(int socketfd, 
                        int level, 
                        int optname,
                        void *optval_p, 
                        socklen_t optlen);
/**获取socket的属性
*@param   socketfd: 调用socket接口返回的socket描述符
@param      level: 支持SOL_SOCKET/IPPROTO_TCP
@param      optname:  SOL_SOCKET对应optname为 SO_DEBUG/SO_OOBINLINE/SO_SNDTIMEO/SO_RCVTIMEO/SO_RCVBUF/SO_SNDBUF
                      IPPROTO_TCP对应optname为 SO_TCP_SACKDISABLE/SO_TCP_NODELAY
@param      optval_p:
@param      optlen_p:
*@return  0:  表示成功
            <0  表示有错误
*
**/          

int getsockopt(int socketfd, 
                        int level, 
                        int optname,
                        void *optval_p, 
                        socklen_t* optlen_p);
/**设置socket的本地端口和ip地址，一般针对服务器代码需要设置
*@param		socketfd:	调用socket接口返回的socket描述符
@param      my_addr:   ip地址和端口，ip一般设置INADDR_ANY
@param      addrlen:  地址长度
*@return	0:  表示成功
            <0  表示有错误
*           
**/                         
int bind(int socketfd, 
                      const struct sockaddr *my_addr, 
                      socklen_t addrlen);
/**建立和服务器端的连接
*@param		socketfd:	调用socket接口返回的socket描述符
@param      addr:   指定服务器地址和端口
@param      addrlen:  sizeof(struct sockaddr)
*@return	0:  表示成功
            <0  表示有错误
*           
**/                                      
int connect(int socketfd, const struct sockaddr *addr, socklen_t addrlen);
/**监听socket连接，一般用作服务器监听客户端的连接
*@param		socketfd:	调用socket接口返回的socket描述符
@param      backlog:   0
*@return	0:  表示成功
            <0  表示有错误
*           
**/                             
int listen(int socketfd, 
                       int backlog);
/**等待连接，一般用于listen之后等待客户端的连接
*@param		socketfd:	调用socket接口返回的socket描述符
@param      addr:   返回客户端ip地址和端口
@param      addrlen: 返回地址长度
*@return	0:  表示成功
            <0  表示有错误
*@note      函数会一直阻塞，知道有客户端连接           
**/                             
int accept(int socketfd, 
                        struct sockaddr *addr, 
                        socklen_t *addrlen);
/**接收数据
*@param		socketfd:	调用socket接口返回的socket描述符
@param      buf:   用于存放数据的缓存
@param      len:   buf的长度
@param      flags: 仅支持MSG_DONTWAIT/MSG_PEEK/MSG_OOB，可以通过或来指定多个标志，一般为0

*@return	>0:  接收到的数据长度
            =0:  对方已经断开连接
            <0:  读取错误
*@note      当flags没有设置MSG_DONTWAIT，该函数会阻塞，直到有数据或者读取超时
**/                                        
int recv(int socketfd, 
                      void *buf, 
                      size_t len,
                      int flags);
/**接收指定ip地址发送来的数据，一般用于UDP收取数据
*@param		sockfd:	调用socket接口返回的socket描述符
@param      buf:   用于存放数据的缓存
@param      len:   buf的长度
@param      flags: 仅支持0
@param      src_addr: ip地址和端口
@param      addrlen: sizeof(struct sockaddr)

*@return	>0: 实际收到的数据长度
            =0:  对方已经断开连接
            <0:  读取错误
**/   

int recvfrom(int sockfd, void *buf, size_t len, int flags,
                    struct sockaddr *src_addr, socklen_t *addrlen);
/**发送数据
*@param		socketfd:	调用socket接口返回的socket描述符
@param      msg:   数据内容
@param      len:   数据长度
@param      flags: 仅支持MSG_DONTWAIT/MSG_OOB，可以通过或来指定多个标志，一般为0

*@return	>=0:  实际发送的长度
            <0: 发送错误
**/   

int send(int socketfd,
                      const void *msg,
                      size_t len,
                      int flags);
/**发送数据到指定ip地址，一般用于udp发送数据
*@param		socketfd:	调用socket接口返回的socket描述符
@param      buf:   数据内容
@param      len:   数据长度
@param      flags: 仅支持0
@param      to_p: 指定ip地址和端口号
@param      tolen: sizeof(struct sockaddr)

*@return	>=0:  实际发送的长度
            <0:  发送错误
**/                        
int sendto(int socketfd,
                        const void *buf,
                        size_t len,
                        int flags,
                        const struct sockaddr *to_p, 
                        socklen_t tolen);
/**阻塞方式等待socket连接的状态
*@param		maxfdp1:	最大socketfd+1
@param      readset:   读取集合，可以为NULL
@param      writeset:  写集合，可以为NULL
@param      exceptset: 异常集合，可以为NULL
@param      timeout: 超时时间

*@return	0:   等待超时
            >0:  readset+writeset+exceptset的集合个数
            <0  -1
**/                 
int select(int maxfdp1, 
                        fd_set *readset,
                        fd_set *writeset,
                        fd_set *exceptset,
                        struct timeval *timeout);
/**获取socket的错误值
*@param		socketfd:	调用socket接口返回的socket描述符
*@return	[EBADF 到 ENO_RECOVERY]
**/                                       
int socket_errno(int socketfd);
/**本地字节顺序转化为网络字节顺序(16bits)
*@param		n: 本地字节书序数据
*@return	网络字节顺序数据
**/                 
#define htons(n) 				((n & 0xff) << 8) | ((n & 0xff00) >> 8)
/**本地字节顺序转化为网络字节顺序(32bits)
*@param		n: 本地字节书序数据
*@return	网络字节顺序数据
**/           
#define htonl(n) 				((n & 0xff) << 24) |\
                                    ((n & 0xff00) << 8) |\
                                ((n & 0xff0000UL) >> 8) |\
                                    ((n & 0xff000000UL) >> 24)
/**网络字节顺序转化为本地字节顺序(16bits)
*@param		n: 网络字节顺序
*@return	本地字节顺序
**/                                           
#define ntohs(n) 				htons(n)
/**网络字节顺序转化为本地字节顺序(32bits)
*@param		n: 网络字节顺序
*@return	本地字节顺序
**/
#define ntohl(n) 				htonl(n)

/**将ip地址字符串转为数值，转化后的数值为网络字节顺序
*@param		cp: ip地址字符串，例如"192.168.1.1"
*@param		addr: struct in_addr 返回的ip地址数值
*@return    1: 成功
            0: 失败
**/ 
#define inet_aton(cp, addr)   ipaddr_aton(cp, (ip_addr_t*)addr)

/**将ip地址数值(网络字节顺序)，转化为ip地址字符串
*@param		addr: struct in_addr ip地址数值
*@return	ip地址字符串
**/ 
#define inet_ntoa(addr)       ipaddr_ntoa((ip_addr_t*)&(addr))


char *
ipaddr_ntoa(const ip_addr_t *addr);

int
ipaddr_aton(const char *cp, ip_addr_t *addr);

/** @}*/

#endif

