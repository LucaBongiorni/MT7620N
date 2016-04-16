#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <netdb.h>
#include <pthread.h>
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
#include "cbProjMain.h"
#include "listStruct.h"
#include "StrOperate.h"




CSocketTCP::CSocketTCP()
	: m_nRecvTimeO(10000), m_nSendTimeO(10000), m_uIP(0), m_uPort(0)
{
	pthread_mutex_init(&SendMutex, NULL);
	pthread_mutex_init(&RecvMutex, NULL);
	//pthread_rwlock_init(&m_MemLock, NULL);
	//m_MemSize = 0;
	//m_pMem    = NULL;
	//m_MemLen  = 0;
}

CSocketTCP:: CSocketTCP(u_int16 uPort)
	: m_nRecvTimeO(10000), m_nSendTimeO(10000), m_uIP(0)
{
	pthread_mutex_init(&SendMutex, NULL);
	pthread_mutex_init(&RecvMutex, NULL);
	//pthread_rwlock_init(&m_MemLock, NULL);
	m_uPort = uPort;
	//m_MemSize = 0;
	//m_pMem    = NULL;
	//m_MemLen  = 0;
}

CSocketTCP::CSocketTCP(u_int32 uIP, u_int16 uPort)
	: m_nRecvTimeO(10000), m_nSendTimeO(10000)
{
	pthread_mutex_init(&SendMutex, NULL);
	pthread_mutex_init(&RecvMutex, NULL);
	//pthread_rwlock_init(&m_MemLock, NULL);
	m_uIP   = uIP;
	m_uPort = uPort;
}

CSocketTCP::CSocketTCP(const char* uIP, u_int16 uPort)
	: m_nRecvTimeO(10000), m_nSendTimeO(10000)
{
	pthread_mutex_init(&SendMutex, NULL);
	pthread_mutex_init(&RecvMutex, NULL);
	//pthread_rwlock_init(&m_MemLock, NULL);
	m_uPort = uPort;

	// 如果传入的是域名，需要进行域名解析
	u_int32 IP = inet_addr(uIP);
	if (IP == INADDR_NONE || IP <= 0)
	{
		if (0 != GetIPByDomain(uIP, &IP))
		{
			m_uIP = INADDR_NONE;
		}
		else
		{
			m_uIP = IP;
		}
	}
	else
	{
		m_uIP = IP;
	}
	//m_MemSize = 1500;
	//m_pMem    = new char[1500];
	//m_MemLen  = 0;
}
CSocketTCP::~CSocketTCP()
{
	//if (m_pMem)
	//{
	//	delete [] m_pMem;
	//	m_pMem = NULL;
	//}
	//pthread_rwlock_destroy(&m_MemLock);
	pthread_mutex_destroy(&SendMutex);
	pthread_mutex_destroy(&RecvMutex);
}

u_int32 
CSocketTCP::GetSocketIP()
{
	return m_uIP;
}

u_int16 
CSocketTCP::GetSocketPort()
{
	return m_uPort;
}

void 
CSocketTCP::SetSocketTimeOut(int sndTimeOut, int rcvTimeOut)
{
	m_nRecvTimeO = rcvTimeOut;
	m_nSendTimeO = sndTimeOut;
}

#if 0
void 
CSocketTCP::SetMemBuff(const char* buff, int buffLen)
{
	pthread_rwlock_wrlock(&m_MemLock);
	if (buffLen > m_MemSize)
	{
		delete [] m_pMem;
		m_pMem = new char[buffLen];
		m_MemSize = buffLen;
	}
	memcpy(m_pMem, buff, buffLen);
	m_MemLen = buffLen;
	pthread_rwlock_unlock(&m_MemLock);
	return ;
}

int 
CSocketTCP::GetMembuff(char* buff, int* buffLen)
{
	pthread_rwlock_rdlock(&m_MemLock);
	if (m_MemLen > *buffLen || NULL == buff)
	{
		pthread_rwlock_unlock(&m_MemLock);
		return -1;
	}
	memcpy(buff, m_pMem, m_MemLen);
	*buffLen = m_MemLen;
	pthread_rwlock_unlock(&m_MemLock);
	return 0;
}
#endif 



/*******************************************************
函数名称:	StartServer
函数功能:	开始服务端监听，该监听为阻塞套接字
输入参数:	uPort :		监听的端口号
			listenNum :	监听等待队列数
返 回 值:	成功: 创建的socket套接字描述符
			失败: -1
*******************************************************/
int 
CSocketTCP::StartServer(int listenNum)
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
int 
CSocketTCP::InitTcpListen(int listenNum)
{
    int SockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (SockFd != -1)
    {
        //设置为非阻塞模式
        //设置套接字的属性使它能够在计算机重启的时候可以再次使用套接字的端口和IP
        if (SetSocketStatus(SockFd) != SUCCESS)
        {
            Debug(I_DEBUG, "set socket status errror!");
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
            Debug(I_DEBUG, "InitTcpListen failed, %s", strerror(errno));
            return FAILED;
        }
        else
        {
            iRtn = listen(SockFd, listenNum);
            if (-1 == iRtn)
            {
                Close(SockFd);
                Debug(I_DEBUG, "InitTcpListen failed, %s", strerror(errno));
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
		Debug(I_DEBUG, "InitTcpListen failed, %s", strerror(errno));
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
int 
CSocketTCP::Accept(int iSockFD, struct sockaddr * pAddr, int * pAddrLen)
{
	int iClientFD = -1;
	if ( 0 >= SelectSingleSocket(iSockFD, SOCK_SEL_TYPE_READ, 3000)) return -1;
	if ((iClientFD = accept(iSockFD, pAddr, (socklen_t*)pAddrLen ) ) < 0 )
	{
		//write_log("The socket error=%s", strerror( errno ));
		return -1;
	}

/*
	struct timeval tv;
	tv.tv_sec  = m_nRecvTimeO / 1000;
	tv.tv_usec = m_nRecvTimeO % 1000;
	(void)setsockopt(iClientFD, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	tv.tv_sec  = m_nSendTimeO / 1000;
	tv.tv_usec = m_nSendTimeO % 1000;
	(void)setsockopt(iClientFD, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
*/
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
int 
CSocketTCP::Connect()
{
	int iSockFD = -1;
	if( ( iSockFD = socket(AF_INET, (int)SOCK_STREAM, 0) ) < 0 )
		return -1;
	
	struct timeval tv;
	tv.tv_sec  = m_nSendTimeO / 1000;
	tv.tv_usec = m_nRecvTimeO % 1000;
	
	(void)setsockopt( iSockFD, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv) );
	(void)setsockopt( iSockFD, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv) );

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

int 
CSocketTCP::Connect(unsigned int ulTimeout, const char* domain)
{
    int nRet;
    int error = 0;
    socklen_t len;
	
    int SockFd = socket(PF_INET, SOCK_STREAM, 0);
    if (SockFd != -1)
    {
		//if (m_uIP <= 0 || m_uIP == INADDR_NONE)
		//{
			m_uIP = inet_addr(domain);
			if (0 >= m_uIP || m_uIP == INADDR_NONE)
			{
				if ( 0 != GetIPByDomain(domain, &m_uIP) || m_uIP <= 0 || m_uIP == INADDR_NONE)
				{
					close(SockFd);
					return -1;
				}
			}
		//}
	
        //设置为非阻塞模式
        //设置套接字的属性使它能够在计算机重启的时候可以
        //再次使用套接字的端口和IP
        struct sockaddr_in serv_addr = {0};
        serv_addr.sin_family      = AF_INET;
        serv_addr.sin_port        = htons(m_uPort);
        serv_addr.sin_addr.s_addr = m_uIP;
		
        nRet = connect(SockFd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (nRet == -1)
        {
            if (errno != EINPROGRESS)
            {
                Debug(I_DEBUG, "0 connect err:%s", strerror(errno));
                Close(SockFd);
                return -1;
            }
			
            struct timeval tm;
            fd_set set;
            tm.tv_sec  = ulTimeout / 1000;
            tm.tv_usec = (ulTimeout % 1000) * 1000;
            while(1)
            {
                FD_ZERO(&set);
                FD_SET(SockFd, &set);
                switch (select(SockFd + 1, NULL, &set, NULL, &tm))
                {
                case -1:
                    if (EINTR == errno || EAGAIN == errno)
                    {
                        continue;
                    }
                    Debug(I_DEBUG, "1 connect err:%s", strerror(errno));
                    Close(SockFd);
                    return -1;
                case 0:
                    Debug(I_DEBUG, "2 connect: timeout");
                    Close(SockFd);
                    return -1;
                default:
                    len = sizeof(int);
                    if (( 0 == getsockopt(SockFd, SOL_SOCKET, SO_ERROR, &error, &len) ))
                    {
                        if( 0 == error )
                        {
                            usleep(100000);
                            return SockFd;
                        }
                    }
                    Debug(I_DEBUG, "5 connect err:%s", strerror(errno));
                    Close(SockFd);
                    return -1;
                }
            }
        }
        else
        {
            Debug(I_INFO, "Connect OK\n");
            return SockFd;
        }
    }
    else
    {
        return -1;
    }
    return -1;
}

int 
CSocketTCP::Connect(unsigned int ulTimeout, unsigned int IP)
{
    int nRet;
    int error = 0;
    socklen_t len;
    int SockFd = socket(PF_INET, SOCK_STREAM, 0);
    if (SockFd != -1)
    {
        //设置为非阻塞模式
        //设置套接字的属性使它能够在计算机重启的时候可以
        //再次使用套接字的端口和IP
        struct sockaddr_in serv_addr = {0};
        serv_addr.sin_family      = AF_INET;
        serv_addr.sin_port        = htons(m_uPort);
        serv_addr.sin_addr.s_addr = IP;
		
        nRet = connect(SockFd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (nRet == -1)
        {
            if(errno != EINPROGRESS)
            {
                Debug(I_DEBUG, "0 connect err:%s", strerror(errno));
                Close(SockFd);
                return -1;
            }
            struct timeval tm;
            fd_set set;
            tm.tv_sec  = ulTimeout / 1000;
            tm.tv_usec = (ulTimeout % 1000) *1000;
            while(1)
            {
                FD_ZERO(&set);
                FD_SET(SockFd, &set);
                switch (select(SockFd + 1, NULL, &set, NULL, &tm))
                {
                case -1:
                    if (EINTR == errno || EAGAIN == errno)
                    {
                        continue;
                    }
                    Debug(I_DEBUG, "1 connect err:%s", strerror(errno));
                    Close(SockFd);
                    return -1;
                case 0:
                    Debug(I_DEBUG, "2 connect: timeout");
                    Close(SockFd);
                    return -1;
                default:
                    len = sizeof(int);
                    if (( 0 == getsockopt(SockFd, SOL_SOCKET, SO_ERROR, &error, &len) ))
                    {
                        if( 0 == error )
                        {
                            usleep(100000);
                            return SockFd;
                        }
                    }
                    Debug(I_DEBUG, "5 connect err:%s", strerror(errno));
                    Close(SockFd);
                    return -1;
                }
            }
        }
        else
        {
            Debug(I_INFO, "Connect OK");
            return SockFd;
        }
    }
    else
    {
        return -1;
    }
    return -1;
}


/*******************************************************
函数名称:	ReConnect
函数功能:	重连函数
返回值:		连接成功的套接字
			此函数会一直尝试连接，直到成功为止
			此函数必须要调用过一次Connect后调用
*******************************************************/
int 
CSocketTCP::ReConnect()
{
	int iSocket;
	while (1)
	{
		if ( ( iSocket = Connect() ) < 0 )
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
int 
Read(int iSockFD, char * cData, int iLen, int iTimeOut )
{
	int nread;
 	char *ptr = cData;
	int nleft = iLen;
	//int flags = 0;

	struct timeval tv;
	tv.tv_sec = iTimeOut / 1000;	// 设置读超时
	tv.tv_usec = iTimeOut % 1000;
	(void)setsockopt(iSockFD, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	
	while ( nleft > 0 )
	{
		if ( (nread = read(iSockFD, ptr, nleft) ) < 0 )
		{
			
			if (EINTR == errno || EAGAIN == errno)
			{
				nread = 0;				// 被中断程序中断，重新调用read()
				//if (flags++ == 5) return -1;
				continue;
			}
			else 
				return 0;				// 网络错误或超时
		}
		else if ( nread == 0 )
		{
			return 0;					// EOF 连接关闭
		}
		nleft -= nread;
		ptr += nread;
	}
	
	return (iLen-nleft);				// return >= 0
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
int 
Write(int iSockFD, const char * cData, int iLen, int iTimeOut )
{
	assert( cData != NULL );

	int nwritten;
	const char * ptr = cData;
	int nleft = iLen;
	//int flags = 0;

#if 1
	struct timeval tv;
	tv.tv_sec = iTimeOut / 1000;	// 设置写超时
	tv.tv_usec = iTimeOut % 1000;
	(void)setsockopt( iSockFD, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv) );
#endif 

	while ( nleft > 0 )
	{
		nwritten = write(iSockFD, ptr, nleft);
		if ( nwritten < 0 )
		{
			if ( errno == EINTR )
			{
				nwritten = 0;			// 被中断程序中断，重新调用write()
				//if (flags++ == 5) return -1;
			}
			else if (EAGAIN == errno)
				return -1;
			else
				return 0;				// 网络错误或者超时
		}
		else if (nwritten == 0)
			return 0;
		
		nleft -= nwritten;
		ptr   += nwritten;
	}	
	return iLen;
}





// 锁住发送函数
void CSocketTCP::LockWriteFun()
{
	pthread_mutex_lock(&SendMutex);
	return ;
}
// 解锁发送函数
void CSocketTCP::UnLockWriteFun()
{
	pthread_mutex_unlock(&SendMutex);
	return ;
}

void CSocketTCP::LockReadFun()
{
	pthread_mutex_lock(&RecvMutex);
	return;
}
void CSocketTCP::UnLockReadFun()
{
	pthread_mutex_unlock(&RecvMutex);
	return ;
}


/*******************************************************
函数功能:	原 close 函数的封装
输入参数:	iSockFD : 要关闭的套接字描述符
输出参数:	无
返回值:		0 : 关闭成功
			-1: 关闭失败
*******************************************************/
int 
Close(int iSockFD)
{
	 if (iSockFD > 0)
    {
        if (-1 == close(iSockFD))
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

int SelectSingleSocket(int SockFd, int nType, unsigned long ulTimeout)
{
    if (SockFd < 0)
    {
        return -1;
    }
    struct timeval timeout;
    fd_set fd;
    int rc = 0;
    while(1)
    {
        timeout.tv_sec  =  ulTimeout / 1000;
        timeout.tv_usec = (ulTimeout % 1000) * 1000;
        FD_ZERO(&fd);
        FD_SET(SockFd, &fd);
        if (SOCK_SEL_TYPE_READ == nType)
        {
            rc = select(SockFd+1, &fd, NULL, NULL, &timeout);
        }
        else
        {
            rc = select(SockFd+1, NULL, &fd, NULL, &timeout);
        }
        if (rc > 0)
        {
            if (FD_ISSET(SockFd, &fd))  // 侦测SockFd这个描述符是否在fd这个集合中
            {
                return rc;
            }
        }
        else if (-1 == rc)
        {
            if (EINTR == errno || EAGAIN == errno)
            {
                continue;
            }
            printf("select err:%s\n", strerror(errno));
            return -1;
        }
        else if (rc == 0)
        {
            return 0;
        }
        else
        {
            continue;
        }
    }
    return 0;
}



int Send(int nTcpSocket, const char *pBuf, int nLen, unsigned int ulTimeout)
{
    if ( nTcpSocket < 0)
    {
        return -1;
    }

	int nwritten;
	const char * ptr = pBuf;
	int nleft = nLen;
	//int flags = 0;

    int nSelectRet;
    nSelectRet = SelectSingleSocket(nTcpSocket, SOCK_SEL_TYPE_WRITE, ulTimeout);
    if (nSelectRet > 0)
    {
		while ( nleft > 0 )
		{
			nwritten = send(nTcpSocket, ptr, nleft, 0);
			if ( nwritten < 0 )
			{
				if (errno == EINTR || errno == EAGAIN)
				{
					nwritten = 0;			// 被中断程序中断，重新调用send()
					//if (flags++ == 5) return -1;
				}
				else
					return -1;				// 网络错误
			}
			else if (nwritten == 0)
				return -1;                  // 对方已经关闭
			
			nleft -= nwritten;
			ptr   += nwritten;
		}	
		return nLen;
    }
    else
    {
        return nSelectRet;
    }
}


int 
SetSocketStatus(int SockFd)
{
    //取得文件描述符的状态
    int nFlags = fcntl(SockFd, F_GETFL, 0);
    if (-1 == nFlags)
    {
        Debug(I_DEBUG, "SetSocketStatus error");
        return FAILED;
    }
	
    //设置文件描述符为非阻塞
    if ( -1 == fcntl(SockFd, F_SETFL, nFlags | O_NONBLOCK) )
    {
        Debug(I_DEBUG, "SetSocketStatus fcntl error");
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

// 0 超时，-1出错，关闭
int Recv(int nTcpSocket, char *pBuf, int nLen, unsigned int ulTimeout)
{
    int ret =0;
    if (nTcpSocket < 0)
    {
        return -1;
    }
	
	int nread = 0;
	char *ptr = pBuf;
	int nleft = nLen;
	int flags = 0;

    ret = SelectSingleSocket(nTcpSocket, SOCK_SEL_TYPE_READ, ulTimeout);
    if (ret > 0)
    {
		while (nleft > 0)
		{
			if ( (nread = recv(nTcpSocket, ptr, nleft, 0) ) < 0 )
			{
				if (errno == EINTR || errno == EAGAIN)  // || EAGAIN == errno
				{
					continue;				// 被中断程序中断，重新调用recv()
				}
				else 
				{
					printf("recv tcp err:%s\n", strerror(errno));
					return -1;				
				}
			}
			else if ( nread == 0 )
			{
				Debug(I_DEBUG, "nread == 0");
				return -1;					// EOF 连接关闭
			}
			else
			{
				nleft -= nread;
				ptr += nread;
			}
		}
		return (nLen-nleft);				// return >= 0
    }
    else
    {
    	//Debug(I_DEBUG, "timeOut ret=%d", ret);
        return ret;
    }
}





