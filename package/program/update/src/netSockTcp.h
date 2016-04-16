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

	// ��ȡ ip
	u_int32 GetSocketIP();
	// ��ȡ�˿�
	u_int16 GetSocketPort();
	// ���ý��պͷ��ͳ�ʱ
	void SetSocketTimeOut(int sndTimeOut, int rcvTimeOut);
	
	// ��������: ��ʼ����˼������ü���Ϊ�����׽��֣��������׽��ֳ�ʱ����
	// �������: �������и���
	// �� �� ֵ: �׽��־��
	int StartServer(int listenNum);
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
	// ����һ�����ݰ�������
	// iSockFD ���������
	// cData �����ַ
	// iLen  ���泤��
	// iTimeOut ��ʱ����λΪ����
	int Recv(int iSockFD, char * cData, int iLen, int iTimeOut);

	// ��������: ���������׽��֣����׽���Ϊ������
	// �������: �������и���
	// �� �� ֵ: �׽��־��
	int InitTcpListen(int listenNum);
	
	
	// ��������
	int Accept(int iSockFD, struct sockaddr * pAddr, int * pAddrLen);
	// ���ӷ�����
	int Connect(const char* domain);
	// ������ֱ������Ϊֹ
	int ReConnect(const char* domain);
	// �ر�����
	int Close(int iSockFD);

private:	
	// �����׽���Ϊ������״̬
	int SetSocketStatus(int SockFd);
	// �����׽��ַ��ͻ�������С
	inline int SetSockSendBufLen(int SockFd, int nSendLen);
	// �����׽��ֽ��ջ�������С
	inline int SetSockRecvBufLen(int SockFd, int nRecvLen);
	// �����׽���Ϊ���Ϸ���
	inline int SetSockSendRightNow(int SockFd);
	// ��ֹnagel�㷨
	inline int SetSockOffNagel(int SockFd);
	// �������Ϲرգ����ȴ��Ĵλ���
	inline int SetSockCloseRightNow(int SockFd);
	// ����Ϊ������
	inline int SetLocalSockNonBlock(int SockFd);



	
	// ͨ���������ֻ�ȡ IP
	u_int32 GetIPByIfname(const char *ifname);
	// ��ȡ������Ӧ IP
	int GetIPByDomain(const char* pDomain, unsigned int *lAddr);
};





#endif /*__NetSockTCP__H__*/

