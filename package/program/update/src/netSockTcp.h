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









class CSocketTCP
{
private:
	int m_nRecvTimeO;
	int m_nSendTimeO;
	
	u_int32 m_uIP;
	u_int16 m_uPort;

public:
	CSocketTCP();
	CSocketTCP(u_int16 uPort);
	CSocketTCP(u_int32 uIP, u_int16 uPort);
	CSocketTCP(const char* uIP, u_int16 uPort);

	// 获取 ip
	u_int32 GetSocketIP();
	// 获取端口
	u_int16 GetSocketPort();
	// 设置接收和发送超时
	void SetSocketTimeOut(int sndTimeOut, int rcvTimeOut);
	
	// 函数功能: 开始服务端监听，该监听为阻塞套接字，并设置套接字超时处理
	// 输入参数: 监听队列个数
	// 返 回 值: 套接字句柄
	int StartServer(int listenNum);
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
	// 接收一个数据包的内容
	// iSockFD 描述符句柄
	// cData 缓存地址
	// iLen  缓存长度
	// iTimeOut 超时，单位为毫秒
	int Recv(int iSockFD, char * cData, int iLen, int iTimeOut);

	// 函数功能: 开启监听套接字，该套接字为非阻塞
	// 输入参数: 监听队列个数
	// 返 回 值: 套接字句柄
	int InitTcpListen(int listenNum);
	
	
	// 接受连接
	int Accept(int iSockFD, struct sockaddr * pAddr, int * pAddrLen);
	// 连接服务器
	int Connect(const char* domain);
	// 重连，直到连上为止
	int ReConnect(const char* domain);
	// 关闭连接
	int Close(int iSockFD);

private:	
	// 设置套接字为非阻塞状态
	int SetSocketStatus(int SockFd);
	// 设置套接字发送缓冲区大小
	inline int SetSockSendBufLen(int SockFd, int nSendLen);
	// 设置套接字接收缓冲区大小
	inline int SetSockRecvBufLen(int SockFd, int nRecvLen);
	// 设置套接字为马上发送
	inline int SetSockSendRightNow(int SockFd);
	// 禁止nagel算法
	inline int SetSockOffNagel(int SockFd);
	// 设置马上关闭，不等待四次挥手
	inline int SetSockCloseRightNow(int SockFd);
	// 设置为非阻塞
	inline int SetLocalSockNonBlock(int SockFd);



	
	// 通过网卡名字获取 IP
	u_int32 GetIPByIfname(const char *ifname);
	// 获取域名对应 IP
	int GetIPByDomain(const char* pDomain, unsigned int *lAddr);
};





#endif /*__NetSockTCP__H__*/

