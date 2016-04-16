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

	// ������������������Ҫ������������
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
��������:	StartServer
��������:	��ʼ����˼������ü���Ϊ�����׽���
�������:	uPort :		�����Ķ˿ں�
			listenNum :	�����ȴ�������
�� �� ֵ:	�ɹ�: ������socket�׽���������
			ʧ��: -1
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
**��    ��: TCP �׽��ּ���
**�������: listenNum ��������
**�� �� ֵ: -1 ʧ�ܣ��ɹ������׽���
*********************************************************************************/
int CSocketTCP::InitTcpListen(int listenNum)
{
    int SockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (SockFd != -1)
    {
        //����Ϊ������ģʽ
        //�����׽��ֵ�����ʹ���ܹ��ڼ����������ʱ������ٴ�ʹ���׽��ֵĶ˿ں�IP
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
��������:	Accept
��������:	ԭ accept �����ķ�װ����������
�������:	iSockFD : StartServer���ص�������
�������:	pAddr : Զ�˵�ַ����ΪNULL
			pAddrLen: Զ�˵�ַ���ȣ���ΪNULL
����ֵ:		�ɹ� : �½������׽���������
			ʧ�� : -1
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
��������:	Connect
��������:	ԭ connect �����ķ�װ
�������:	uIP : ������IP��ַ�������ֽ���
			uPort:�������˿ںţ������ֽ���
����ֵ:		�ɹ� : �����ɹ����׽���������
			ʧ�� : -1
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
		if( errno == EINTR )			// ����Ǳ��жϴ���ģ����ʾ�ѳ�ʱ
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
��������:	ReConnect
��������:	��������
����ֵ:		���ӳɹ����׽���
			�˺�����һֱ�������ӣ�ֱ���ɹ�Ϊֹ
			�˺�������Ҫ���ù�һ��Connect�����
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
��������:	Read
��������:	����ָ���ֽڵ�����
����:		iSockFD : Ҫ�������ݵ��׽���������
			cData   : ���ݴ�ŵ�ַ
			iLen    : Ҫ���յ����ݳ���
			iTimeOut: ��ʱʱ�䣬��λΪ����
����ֵ:		�ɹ� : ���յ����ֽ���
			ʧ�� : -1��ͬʱ��errnoΪ��Ӧֵ
			��ʱ : ʵ�ʽ��յ����ֽ���
*******************************************************/
int CSocketTCP::Read(int iSockFD, char * cData, int iLen, int iTimeOut )
{
	int nleft, nread;
	char * ptr;

	if( iTimeOut != m_nRecvTimeO )
	{
		struct timeval tv;
		tv.tv_sec = iTimeOut / 1000;	// ���ö���ʱ
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
				nread = 0;				// ���жϳ����жϣ����µ���read()
			else
				return -1;				// ��������ʱ
		}
		else if ( nread == 0 )
		{
			return 0;					// EOF ���ӹر�
		}
		nleft -= nread;
		ptr += nread;
	}
	
	return (iLen - nleft);				// return >= 0
}


/*******************************************************
��������:	Recv
��������:	����һ�����ݰ�
����:		iSockFD : Ҫ�������ݵ��׽���������
			cData   : ���ݴ�ŵ�ַ
			iLen    : Ҫ���յ����ݳ���
			iTimeOut: ��ʱʱ�䣬��λΪ����
����ֵ:		�ɹ� : ���յ����ֽ���
			ʧ�� : -1��ͬʱ��errnoΪ��Ӧֵ
			��ʱ : ʵ�ʽ��յ����ֽ���
*******************************************************/
int CSocketTCP::Recv(int iSockFD, char * cData, int iLen, int iTimeOut )
{
	int nread;

	if( iTimeOut != m_nRecvTimeO )
	{
		struct timeval tv;
		tv.tv_sec = iTimeOut / 1000;	// ���ö���ʱ
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
				nread = 0;				// ���жϳ����жϣ����µ���read()
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
��������:	Write
��������:	����ָ���ֽڵ�����
�������:	iSockFD : Ҫ�������ݵ��׽���������
			cData   : ���ݴ�ŵ�ַ
			iLen    : Ҫ���͵����ݳ���
			iTimeOut: ��ʱʱ�䣬��λΪ����
�������:	��
����ֵ:		�ɹ�:���͵��ֽ���
			��ʱ:-1
*******************************************************/
int CSocketTCP::Write( int iSockFD, const char * cData, int iLen, int iTimeOut )
{
	assert( cData != NULL );

	int nleft, nwritten;
	const char * ptr;

	if( iTimeOut != m_nSendTimeO )
	{
		struct timeval tv;
		tv.tv_sec = iTimeOut / 1000;	// ����д��ʱ
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
				nwritten = 0;			// ���жϳ����жϣ����µ���write()
			else
				return -1;				// ���������߳�ʱ
		}
		
		nleft -= nwritten;
		ptr   += nwritten;
	}	
	return iLen;
}


/*******************************************************
��������:	Close
��������:	ԭ close �����ķ�װ
�������:	iSockFD : Ҫ�رյ��׽���������
�������:	��
����ֵ:		0 : �رճɹ�
			-1: �ر�ʧ��
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
**��    ��: ͨ����������ȡ����IP
**�������: ifname ��������
**�� �� ֵ: ����0ʧ��
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
**��    ��: ��ȡ������Ӧ��ip
**�������: pIP, ������
**�������: lAddr ��������ֽڵ�ip
**�� �� ֵ: �ɹ�����0�� ʧ�ܷ��� -1
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
**��    ��: ����socket�����������˿ں�ip����״̬
**�������:
**�� �� ֵ: �ɹ� 0��ʧ�� -1
*********************************************************************************/
int CSocketTCP::SetSocketStatus(int SockFd)
{
    //ȡ���ļ���������״̬
    int nFlags = fcntl(SockFd, F_GETFL, 0);
    if (-1 == nFlags)
    {
        write_error_log("SetSocketStatus error");
        return FAILED;
    }
	
    //�����ļ�������Ϊ������
    if ( -1 == fcntl(SockFd, F_SETFL, nFlags | O_NONBLOCK) )
    {
        write_error_log("SetSocketStatus fcntl error");
        return FAILED;
    }
	
    int bReUseAddr = 1;
    //�����׽��ֵ�����ʹ���ܹ��ڼ����������ʱ������ٴ�ʹ���׽��ֵĶ˿ں�IP
    int error = setsockopt(SockFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&bReUseAddr, sizeof(bReUseAddr));
    if(error != 0)
    {
        return FAILED;
    }
	
    //SO_LINGER, ���Źر�socket���ᱣ֤������������ȫ���������
    //��ѡ���UDP��SOCKET��Ч
    //struct linger zeroLinger = {1, 2};
    //setsockopt(SockFd, SOL_SOCKET, SO_LINGER, (const char *)&zeroLinger, sizeof(linger));
    return SUCCESS;
}



/*******************************************************************************
**��    ��: �����׽��ַ��ͻ���������
**�������: SockFd �׽����ļ���������nSendLen ����������
**�� �� ֵ: 0 �ɹ���-1 ʧ��
*********************************************************************************/
int CSocketTCP::SetSockSendBufLen(int SockFd, int nSendLen)
{
    int sockbuflen = nSendLen;
    return setsockopt(SockFd, SOL_SOCKET, SO_SNDBUF, (char*) &sockbuflen, sizeof(int));
}


/*******************************************************************************
**��    ��: �����׽��ֽ��ջ���������
**�������: SockFd �׽����ļ���������nSendLen ����������
**�� �� ֵ: 0 �ɹ���-1 ʧ��
*********************************************************************************/
int CSocketTCP::SetSockRecvBufLen(int SockFd, int nRecvLen)
{
    int sockbuflen = nRecvLen;
    return setsockopt(SockFd, SOL_SOCKET, SO_RCVBUF, (char*) &sockbuflen, sizeof(int));
}


/*******************************************************************************
**��    ��: �����׽���Ϊ���Ϸ���
**�������: SockFd �׽����ļ�������
**�� �� ֵ: 0 �ɹ���-1 ʧ��
*********************************************************************************/
int CSocketTCP::SetSockSendRightNow(int SockFd)
{
    const char delay_opt = 1;
    // �����׽���������������
    return setsockopt(SockFd, IPPROTO_TCP, TCP_NODELAY, &delay_opt, sizeof(char));
}


/*******************************************************************************
**��    ��: ���� Nagel �㷨
**�������:
**�� �� ֵ:
*********************************************************************************/
int CSocketTCP::SetSockOffNagel(int SockFd)
{
    int flag = 1;
    return setsockopt(SockFd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
}


/*******************************************************************************
**��    ��: ���Ϲر��׽��֣����ȴ��Ĵλ���
**�������:
**�� �� ֵ:
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
    //ȡ���ļ���������״̬
    int nFlags = fcntl(SockFd, F_GETFL, 0);
    if (-1 == nFlags)
    {
        return FAILED;
    }
    //�����ļ�������Ϊ������
    if ( -1 == fcntl(SockFd, F_SETFL, nFlags | O_NONBLOCK) )
    {
        return FAILED;
    }
    return SUCCESS;
}

