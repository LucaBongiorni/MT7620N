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
    // ��ȡ ip
    u_int32 GetSocketIP();
    // ��ȡ�˿�
    u_int16 GetSocketPort();
    // ���ý��պͷ��ͳ�ʱ
    void SetSocketTimeOut(int sndTimeOut, int rcvTimeOut);

    /**********************************
     ��������: ��ʼ����˼������ü���Ϊ�����׽��֣��������׽��ֳ�ʱ����
     �������: �������и���
     �� �� ֵ: �׽��־��
    ***********************************/
    int StartServer(int listenNum);


    /**********************************
     ��������: ���������׽��֣����׽���Ϊ������
     �������: �������и���
     �� �� ֵ: �׽��־��
    ***********************************/
    int InitTcpListen(int listenNum);

    // ��������
    int Accept(int iSockFD, struct sockaddr * pAddr, int * pAddrLen);
    // ���ӷ�����
    int Connect();
    int Connect(unsigned int ulTimeout, const char* domain);
	int Connect(unsigned int ulTimeout, unsigned int IP);
    // ������ֱ������Ϊֹ
    int ReConnect();

    // ��ס���ͺ���
    void LockWriteFun();
    // �������ͺ���
    void UnLockWriteFun();
    // ��ס���պ���
    void LockReadFun();
    // �������պ���
    void UnLockReadFun();
};


// ��������
// iSockFD ���������
// cData �����ַ
// iLen  ���泤��
// iTimeOut ��ʱ����λΪ����
int Read(int iSockFD, char* cData, int iLen, int iTimeOut);
// ��������
// iSockFD ���������
// cData �����ַ
// iLen  ���泤��
// iTimeOut ��ʱ����λΪ����
int Write(int iSockFD, const char* cData, int iLen, int iTimeOut);
// �ر�����
int Close(int iSockFD);
int Send(int nTcpSocket, const char *pBuf, int nLen, unsigned int ulTimeout);
int Recv(int nTcpSocket, char *pBuf, int nLen, unsigned int ulTimeout);
int SelectSingleSocket(int SockFd, int nType, unsigned long ulTimeout);
int SetSocketStatus(int SockFd);


/*******************************************************************************
**��    ��: �����׽��ַ��ͻ���������
**�������: SockFd �׽����ļ���������nSendLen ����������
**�� �� ֵ: 0 �ɹ���-1 ʧ��
*********************************************************************************/
static inline int
SetSockSendBufLen(int SockFd, int nSendLen)
{
    int sockbuflen = nSendLen;
    return setsockopt(SockFd, SOL_SOCKET, SO_SNDBUF, (char*) &sockbuflen, sizeof(int));
}

/*******************************************************************************
**��    ��: �����׽��ֽ��ջ���������
**�������: SockFd �׽����ļ���������nSendLen ����������
**�� �� ֵ: 0 �ɹ���-1 ʧ��
*********************************************************************************/
static inline int
SetSockRecvBufLen(int SockFd, int nRecvLen)
{
    int sockbuflen = nRecvLen;
    return setsockopt(SockFd, SOL_SOCKET, SO_RCVBUF, (char*) &sockbuflen, sizeof(int));
}

/*******************************************************************************
**��    ��: �����׽���Ϊ���Ϸ���
**�������: SockFd �׽����ļ�������
**�� �� ֵ: 0 �ɹ���-1 ʧ��
*********************************************************************************/
static inline int
SetSockSendRightNow(int SockFd)
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
static inline int
SetSockOffNagel(int SockFd)
{
    int flag = 1;
    return setsockopt(SockFd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
}

/*******************************************************************************
**��    ��: ���Ϲر��׽��֣����ȴ��Ĵλ���
**�������:
**�� �� ֵ:
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

#if 0
// ����Ϊ�����ʱ������ͨ��select��ѯ����׽���
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
// ѭ��������������
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
        // ʧ�ܴ���
    }
    nRead -= nTemp;
    pRecvBuff += nRet;
}
#endif

#endif /*__NetSockTCP__H__*/

