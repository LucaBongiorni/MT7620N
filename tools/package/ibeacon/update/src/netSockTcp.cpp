#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/tcp.h>
#include <linux/sockios.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <net/if.h>



#include "netSockTcp.h"
#include "defCom.h"
#include "procotol.h"


CSocketTCP::CSocketTCP()
	: m_nRecvTimeO(10000), m_nSendTimeO(10000), m_uIP(0), m_uPort(0)
{
	(void)signal(SIGPIPE, SIG_IGN);
}

CSocketTCP:: CSocketTCP(u_int16 uPort)
	: m_nRecvTimeO(10000), m_nSendTimeO(10000), m_uIP(0)
{
	(void)signal(SIGPIPE, SIG_IGN);
	m_uPort = uPort;
}

CSocketTCP::CSocketTCP(u_int32 uIP, u_int16 uPort)
	: m_nRecvTimeO(10000), m_nSendTimeO(10000)
{
	(void)signal(SIGPIPE, SIG_IGN);
	m_uIP   = uIP;
	m_uPort = uPort;
}

CSocketTCP::CSocketTCP(const char* uIP, u_int16 uPort)
	: m_nRecvTimeO(10000), m_nSendTimeO(10000)
{
	(void)signal(SIGPIPE, SIG_IGN);
	m_uPort = uPort;

	// 如果传入的是域名，需要进行域名解析
	u_int32 IP = inet_addr(uIP);
	if (IP == INADDR_NONE)
	{
		GetIPByDomain(uIP, &IP);
	}
	m_uIP = IP;
}

u_int32 CSocketTCP::GetSocketIP()
{
	return m_uIP;
}

u_int16 CSocketTCP::GetSocketPort()
{
	return m_uPort;
}

void CSocketTCP::SetSocketTimeOut(int sndTimeOut, int rcvTimeOut)
{
	m_nRecvTimeO = rcvTimeOut;
	m_nSendTimeO = sndTimeOut;
}


/*******************************************************
函数名称:	StartServer
函数功能:	开始服务端监听，该监听为阻塞套接字
输入参数:	uPort :		监听的端口号
			listenNum :	监听等待队列数
返 回 值:	成功: 创建的socket套接字描述符
			失败: -1
*******************************************************/
int CSocketTCP::StartServer(int listenNum)
{
	int iSockFD = -1;
	if ( ( iSockFD = socket(AF_INET, (int)SOCK_STREAM, 0) ) < 0 )
	{
		write_log("create listen socket error");
		return -1;
	}

	struct sockaddr_in stSockAddr;
	memset( &stSockAddr, 0, sizeof(stSockAddr) );
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port   = htons(m_uPort);
	stSockAddr.sin_addr.s_addr = INADDR_ANY;

	int iFlag = 1;
	(void)setsockopt(iSockFD, SOL_SOCKET, SO_REUSEADDR, &iFlag, sizeof(int));
	if (bind(iSockFD, (struct sockaddr*)&stSockAddr, sizeof(stSockAddr)) < 0 )
	{
		write_log("bind to listen port error");
		Close(iSockFD);
		return -1;
	}

	if (listen(iSockFD, listenNum) < 0 )
	{
		write_log("start listen error");
		Close(iSockFD);
		return -1;
	}

	return iSockFD;
}


/*******************************************************************************
**功    能: TCP 套接字监听
**输入参数: listenNum 监听个数
**返 回 值: -1 失败；成功返回套接字
*********************************************************************************/
int CSocketTCP::InitTcpListen(int listenNum)
{
    int SockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (SockFd != -1)
    {
        //设置为非阻塞模式
        //设置套接字的属性使它能够在计算机重启的时候可以再次使用套接字的端口和IP
        if (SetSocketStatus(SockFd) != SUCCESS)
        {
            write_error_log("set socket status errror!");
            return FAILED;
        }
		
        struct sockaddr_in myAddr = {0};
        myAddr.sin_family      = AF_INET;
        myAddr.sin_port        = htons(m_uPort);
        myAddr.sin_addr.s_addr = INADDR_ANY;
        int iRtn = bind(SockFd, (struct sockaddr*)&myAddr, sizeof(struct sockaddr_in));
        if (-1 == iRtn)
        {
            Close(SockFd);
            write_error_log("InitTcpListen failed, %s", strerror(errno));
            return FAILED;
        }
        else
        {
            iRtn = listen(SockFd, listenNum);
            if (-1 == iRtn)
            {
                Close(SockFd);
                write_error_log("InitTcpListen failed, %s", strerror(errno));
                return FAILED;
            }
            else
            {
                return SockFd;
            }
        }
    }
    else
    {
		write_error_log("InitTcpListen failed, %s", strerror(errno));
        return FAILED;
    }
}










/*******************************************************
函数名称:	Accept
函数功能:	原 accept 函数的封装，接受连接
输入参数:	iSockFD : StartServer返回的描述符
输出参数:	pAddr : 远端地址，可为NULL
			pAddrLen: 远端地址长度，可为NULL
返回值:		成功 : 新建立的套接字描述符
			失败 : -1
*******************************************************/
int CSocketTCP::Accept(int iSockFD, struct sockaddr * pAddr, int * pAddrLen)
{
	int iClientFD = -1;
	if ( ( iClientFD = accept( iSockFD, pAddr, (socklen_t*)pAddrLen ) ) < 0 )
	{
		write_log("The socket error: %s", strerror( errno ));
		return -1;
	}

	struct timeval tv;
	tv.tv_sec  = m_nRecvTimeO / 1000;
	tv.tv_usec = m_nRecvTimeO % 1000;
	(void)setsockopt(iClientFD, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	tv.tv_sec  = m_nSendTimeO / 1000;
	tv.tv_usec = m_nSendTimeO % 1000;
	(void)setsockopt(iClientFD, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

	return iClientFD;
}


/*******************************************************
函数名称:	Connect
函数功能:	原 connect 函数的封装
输入参数:	uIP : 服务器IP地址，主机字节序
			uPort:服务器端口号，主机字节序
返回值:		成功 : 创建成功的套接字描述符
			失败 : -1
*******************************************************/
int CSocketTCP::Connect()
{
	int iSockFD = -1;
	if( ( iSockFD = socket(AF_INET, (int)SOCK_STREAM, 0) ) < 0 )
		return -1;
	
	struct timeval tv;
	tv.tv_sec  = m_nSendTimeO / 1000;
	tv.tv_usec = m_nRecvTimeO % 1000;
	
	(void)setsockopt(iSockFD, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv) );
	(void)setsockopt(iSockFD, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv) );

	struct sockaddr_in stSockAddr;
	memset( &stSockAddr, 0, sizeof(stSockAddr) );
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port   = htons(m_uPort);
	stSockAddr.sin_addr.s_addr = m_uIP;

	int n = -1;
	if ( (n = connect(iSockFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr) )) < 0 )
	{
		if( errno == EINTR )			// 如果是被中断错误的，则表示已超时
			errno = ETIMEDOUT;
	}

	if( n < 0 )
	{
		close(iSockFD);
		return -1;
	}
	else
	{
		return iSockFD;
	}
}


/*******************************************************
函数名称:	ReConnect
函数功能:	重连函数
返回值:		连接成功的套接字
			此函数会一直尝试连接，直到成功为止
			此函数必须要调用过一次Connect后调用
*******************************************************/
int CSocketTCP::ReConnect()
{
	int iSocket;
	while( 1 )
	{
		if( ( iSocket = Connect() ) < 0 )
		{
			(void)usleep(100000);
		}
		else
		{
			return iSocket;
		}
	}
}


/*******************************************************
函数名称:	Read
函数功能:	接收指定字节的数据
参数:		iSockFD : 要接收数据的套接字描述符
			cData   : 数据存放地址
			iLen    : 要接收的数据长度
			iTimeOut: 超时时间，单位为毫秒
返回值:		成功 : 接收到的字节数
			失败 : -1，同时置errno为相应值
			超时 : 实际接收到的字节数
*******************************************************/
int CSocketTCP::Read(int iSockFD, char * cData, int iLen, int iTimeOut )
{
	int nleft, nread;
	char * ptr;

	if( iTimeOut != m_nRecvTimeO )
	{
		struct timeval tv;
		tv.tv_sec = iTimeOut / 1000;	// 设置读超时
		tv.tv_usec = iTimeOut % 1000;
		(void)setsockopt(iSockFD, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv) );

		m_nRecvTimeO = iTimeOut;
	}
	
	ptr   = cData;
	nleft = iLen;
	while ( nleft > 0 )
	{
		if ( (nread = read(iSockFD, ptr, nleft) ) < 0 )
		{
			if (EINTR == errno || EAGAIN == errno)
				nread = 0;				// 被中断程序中断，重新调用read()
			else
				return -1;				// 网络错误或超时
		}
		else if ( nread == 0 )
		{
			return 0;					// EOF 连接关闭
		}
		nleft -= nread;
		ptr += nread;
	}
	
	return (iLen - nleft);				// return >= 0
}


/*******************************************************
函数名称:	Recv
函数功能:	接收一个数据包
参数:		iSockFD : 要接收数据的套接字描述符
			cData   : 数据存放地址
			iLen    : 要接收的数据长度
			iTimeOut: 超时时间，单位为毫秒
返回值:		成功 : 接收到的字节数
			失败 : -1，同时置errno为相应值
			超时 : 实际接收到的字节数
*******************************************************/
int CSocketTCP::Recv(int iSockFD, char * cData, int iLen, int iTimeOut )
{
	int nread;

	if( iTimeOut != m_nRecvTimeO )
	{
		struct timeval tv;
		tv.tv_sec = iTimeOut / 1000;	// 设置读超时
		tv.tv_usec = iTimeOut % 1000;
		(void)setsockopt(iSockFD, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv) );

		m_nRecvTimeO = iTimeOut;
	}

	do 
	{
		nread = recv(iSockFD, cData, iLen, 0);
		if (nread < 0)
		{
			if (EINTR == errno || EAGAIN == errno)
				nread = 0;				// 被中断程序中断，重新调用read()
			else
				return -1;	
		}
		else if (nread == 0)
		{
			return 0;
		}
		else
		{
			return nread;
		}
	}while(1);
}



/*******************************************************
函数名称:	Write
函数功能:	发送指定字节的数据
输入参数:	iSockFD : 要发送数据的套接字描述符
			cData   : 数据存放地址
			iLen    : 要发送的数据长度
			iTimeOut: 超时时间，单位为毫秒
输出参数:	无
返回值:		成功:发送的字节数
			超时:-1
*******************************************************/
int CSocketTCP::Write( int iSockFD, const char * cData, int iLen, int iTimeOut )
{
	assert( cData != NULL );

	int nleft, nwritten;
	const char * ptr;

	if( iTimeOut != m_nSendTimeO )
	{
		struct timeval tv;
		tv.tv_sec = iTimeOut / 1000;	// 设置写超时
		tv.tv_usec = iTimeOut % 1000;
		(void)setsockopt( iSockFD, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv) );

		m_nSendTimeO = iTimeOut;
	}

	ptr = cData;
	nleft = iLen;
	while ( nleft > 0 )
	{
		if ( ( nwritten = write(iSockFD, ptr, nleft) ) <= 0 )
		{
			if( errno == EINTR )
				nwritten = 0;			// 被中断程序中断，重新调用write()
			else
				return -1;				// 网络错误或者超时
		}
		
		nleft -= nwritten;
		ptr   += nwritten;
	}	
	return iLen;
}


/*******************************************************
函数名称:	Close
函数功能:	原 close 函数的封装
输入参数:	iSockFD : 要关闭的套接字描述符
输出参数:	无
返回值:		0 : 关闭成功
			-1: 关闭失败
*******************************************************/
int CSocketTCP::Close(int iSockFD)
{
	 if (iSockFD > 0)
    {
        if(-1 == close(iSockFD))
        {
			write_log("close socket[%d] err:%s\n", iSockFD, strerror(errno));      
        }
        iSockFD = -1;
        return SUCCESS;
    }
    else
    {
        return FAILED;
    }
}



/*******************************************************************************
**功    能: 通过网卡名获取网卡IP
**输入参数: ifname 网卡名字
**返 回 值: 返回0失败
*********************************************************************************/
u_int32 CSocketTCP::GetIPByIfname(const char *ifname)
{
    struct ifreq if_data;
    int sockd;
    u_int32_t ip;
    if ((sockd = socket(AF_INET, SOCK_PACKET, htons(0x8086))) < 0)
    {
        write_error_log("Create Socket Failed.");
        return 0;
    }
    /* Get IP of internal interface */
    strcpy(if_data.ifr_name, ifname);
	
    /* Get the IP address */
    if (ioctl(sockd, SIOCGIFADDR, &if_data) < 0)
    {
        write_error_log("Ioctl Get Socket Ifaddr Failed.");
        close(sockd);
        return 0;
    }
    memcpy((void*)&ip, (char*)&if_data.ifr_addr.sa_data + 2, 4);
    close(sockd);
    return ip;
}



/*******************************************************************************
**功    能: 获取域名对应的ip
**输入参数: pIP, 域名；
**输出参数: lAddr 输出网络字节的ip
**返 回 值: 成功返回0， 失败返回 -1
*********************************************************************************/
int CSocketTCP::GetIPByDomain(const char* pDomain, unsigned int *lAddr)
{
    struct hostent *hp = NULL;
    if ( (hp = gethostbyname(pDomain)) != NULL )
    {
        if (hp->h_addr_list[0])
        {
            *lAddr = *((unsigned int*) hp->h_addr_list[0]);
            return SUCCESS;
        }
    }
    return FAILED;
}


/*******************************************************************************
**功    能: 设置socket，非阻塞，端口和ip复用状态
**输入参数:
**返 回 值: 成功 0；失败 -1
*********************************************************************************/
int CSocketTCP::SetSocketStatus(int SockFd)
{
    //取得文件描述符的状态
    int nFlags = fcntl(SockFd, F_GETFL, 0);
    if (-1 == nFlags)
    {
        write_error_log("SetSocketStatus error");
        return FAILED;
    }
	
    //设置文件描述符为非阻塞
    if ( -1 == fcntl(SockFd, F_SETFL, nFlags | O_NONBLOCK) )
    {
        write_error_log("SetSocketStatus fcntl error");
        return FAILED;
    }
	
    int bReUseAddr = 1;
    //设置套接字的属性使它能够在计算机重启的时候可以再次使用套接字的端口和IP
    int error = setsockopt(SockFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&bReUseAddr, sizeof(bReUseAddr));
    if(error != 0)
    {
        return FAILED;
    }
	
    //SO_LINGER, 优雅关闭socket，会保证缓冲区的数据全部发送完成
    //此选项对UDP的SOCKET无效
    //struct linger zeroLinger = {1, 2};
    //setsockopt(SockFd, SOL_SOCKET, SO_LINGER, (const char *)&zeroLinger, sizeof(linger));
    return SUCCESS;
}



/*******************************************************************************
**功    能: 设置套接字发送缓冲区长度
**输入参数: SockFd 套接字文件描述符，nSendLen 缓冲区长度
**返 回 值: 0 成功，-1 失败
*********************************************************************************/
int CSocketTCP::SetSockSendBufLen(int SockFd, int nSendLen)
{
    int sockbuflen = nSendLen;
    return setsockopt(SockFd, SOL_SOCKET, SO_SNDBUF, (char*) &sockbuflen, sizeof(int));
}


/*******************************************************************************
**功    能: 设置套接字接收缓冲区长度
**输入参数: SockFd 套接字文件描述符，nSendLen 缓冲区长度
**返 回 值: 0 成功，-1 失败
*********************************************************************************/
int CSocketTCP::SetSockRecvBufLen(int SockFd, int nRecvLen)
{
    int sockbuflen = nRecvLen;
    return setsockopt(SockFd, SOL_SOCKET, SO_RCVBUF, (char*) &sockbuflen, sizeof(int));
}


/*******************************************************************************
**功    能: 设置套接字为马上发送
**输入参数: SockFd 套接字文件描述符
**返 回 值: 0 成功，-1 失败
*********************************************************************************/
int CSocketTCP::SetSockSendRightNow(int SockFd)
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
int CSocketTCP::SetSockOffNagel(int SockFd)
{
    int flag = 1;
    return setsockopt(SockFd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
}


/*******************************************************************************
**功    能: 马上关闭套接字，不等待四次挥手
**输入参数:
**返 回 值:
*********************************************************************************/
int SetSockCloseRightNow(int SockFd)
{
    struct linger linger;
    linger.l_onoff  = 1;
    linger.l_linger = 0;
    return setsockopt(SockFd, SOL_SOCKET, SO_LINGER, (const char *) &linger, sizeof(linger));
}

int CSocketTCP::SetLocalSockNonBlock(int SockFd)
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

