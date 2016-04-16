#ifndef __NetSockTCP__H__
#define __NetSockTCP__H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <netinet/tcp.h>
#include <linux/sockios.h>
#include <linux/rtc.h>
#include <asm/ioctls.h>

#include "defCom.h"

#ifndef SOCK_SEL_TYPE_READ
#define SOCK_SEL_TYPE_READ          0
#endif
#ifndef SOCK_SEL_TYPE_WRITE
#define SOCK_SEL_TYPE_WRITE         1
#endif


class CSocketTCP
{
private:
    int m_nRecvTimeO;
    int m_nSendTimeO;

	char* domain;
    u_int32 m_uIP;
    u_int16 m_uPort;
    pthread_mutex_t SendMutex;
    pthread_mutex_t RecvMutex;
	
public:
    CSocketTCP();
    CSocketTCP(u_int16 uPort);
    CSocketTCP(u_int32 uIP, u_int16 uPort);
    CSocketTCP(const char* uIP, u_int16 uPort);
    ~CSocketTCP();
    // 获取 ip
    u_int32 GetSocketIP();
    // 获取端口
    u_int16 GetSocketPort();
    // 设置接收和发送超时
    void SetSocketTimeOut(int sndTimeOut, int rcvTimeOut);

    /**********************************
     函数功能: 开始服务端监听，该监听为阻塞套接字，并设置套接字超时处理
     输入参数: 监听队列个数
     返 回 值: 套接字句柄
    ***********************************/
    int StartServer(int listenNum);


    /**********************************
     函数功能: 开启监听套接字，该套接字为非阻塞
     输入参数: 监听队列个数
     返 回 值: 套接字句柄
    ***********************************/
    int InitTcpListen(int listenNum);

    // 接受连接
    int Accept(int iSockFD, struct sockaddr * pAddr, int * pAddrLen);
    // 连接服务器
    int Connect();
    int Connect(unsigned int ulTimeout, const char* domain);
	int Connect(unsigned int ulTimeout, unsigned int IP);
    // 重连，直到连上为止
    int ReConnect();

    // 锁住发送函数
    void LockWriteFun();
    // 解锁发送函数
    void UnLockWriteFun();
    // 锁住接收函数
    void LockReadFun();
    // 解锁接收函数
    void UnLockReadFun();
};


// 接收数据
// iSockFD 描述符句柄
// cData 缓存地址
// iLen  缓存长度
// iTimeOut 超时，单位为毫秒
int Read(int iSockFD, char* cData, int iLen, int iTimeOut);
// 发送数据
// iSockFD 描述符句柄
// cData 缓存地址
// iLen  缓存长度
// iTimeOut 超时，单位为毫秒
int Write(int iSockFD, const char* cData, int iLen, int iTimeOut);
// 关闭连接
int Close(int iSockFD);
int Send(int nTcpSocket, const char *pBuf, int nLen, unsigned int ulTimeout);
int Recv(int nTcpSocket, char *pBuf, int nLen, unsigned int ulTimeout);
int SelectSingleSocket(int SockFd, int nType, unsigned long ulTimeout);
int SetSocketStatus(int SockFd);


/*******************************************************************************
**功    能: 设置套接字发送缓冲区长度
**输入参数: SockFd 套接字文件描述符，nSendLen 缓冲区长度
**返 回 值: 0 成功，-1 失败
*********************************************************************************/
static inline int
SetSockSendBufLen(int SockFd, int nSendLen)
{
    int sockbuflen = nSendLen;
    return setsockopt(SockFd, SOL_SOCKET, SO_SNDBUF, (char*) &sockbuflen, sizeof(int));
}

/*******************************************************************************
**功    能: 设置套接字接收缓冲区长度
**输入参数: SockFd 套接字文件描述符，nSendLen 缓冲区长度
**返 回 值: 0 成功，-1 失败
*********************************************************************************/
static inline int
SetSockRecvBufLen(int SockFd, int nRecvLen)
{
    int sockbuflen = nRecvLen;
    return setsockopt(SockFd, SOL_SOCKET, SO_RCVBUF, (char*) &sockbuflen, sizeof(int));
}

/*******************************************************************************
**功    能: 设置套接字为马上发送
**输入参数: SockFd 套接字文件描述符
**返 回 值: 0 成功，-1 失败
*********************************************************************************/
static inline int
SetSockSendRightNow(int SockFd)
{
    const char delay_opt = 1;
    // 设置套接字立即发送数据
    return setsockopt(SockFd, IPPROTO_TCP, TCP_NODELAY, &delay_opt, sizeof(char));
}

/*******************************************************************************
**功    能: 禁用 Nagel 算法
**输入参数:
**返 回 值:
*********************************************************************************/
static inline int
SetSockOffNagel(int SockFd)
{
    int flag = 1;
    return setsockopt(SockFd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
}

/*******************************************************************************
**功    能: 马上关闭套接字，不等待四次挥手
**输入参数:
**返 回 值:
*********************************************************************************/
static inline int
SetSockCloseRightNow(int SockFd)
{
    struct linger linger;
    linger.l_onoff  = 1;
    linger.l_linger = 0;
    return setsockopt(SockFd, SOL_SOCKET, SO_LINGER, (const char *)&linger, sizeof(linger));
}

static inline int
SetLocalSockNonBlock(int SockFd)
{
    //取得文件描述符的状态
    int nFlags = fcntl(SockFd, F_GETFL, 0);
    if (-1 == nFlags)
    {
        return FAILED;
    }
    //设置文件描述符为非阻塞
    if ( -1 == fcntl(SockFd, F_SETFL, nFlags | O_NONBLOCK) )
    {
        return FAILED;
    }
    return SUCCESS;
}

#if 0
// 在作为服务端时，可以通过select轮询多个套接字
typedef struct  _SocketClient
{
    int     sockFD;
    bool    ConFlag;
} SocketClient;
class CSelectSocket
{
public:
    CSelectSocket();
    ~CSelectSocket();
};
#endif

#if 0
// 循环接收所有数据
while(nRead > 0)
{
    nTemp = nRead / 1400;
    if (nTemp > 0)
    {
        nTemp = 1400;
    }
    else
    {
        nTemp = nRead % 1400;
    }
    nRet = serverTCP->Read(sockFD, pRecvBuff, nTemp, 10*1000);
    if (nRet == -1)
    {
        // 失败处理
    }
    nRead -= nTemp;
    pRecvBuff += nRet;
}
#endif

#endif /*__NetSockTCP__H__*/

