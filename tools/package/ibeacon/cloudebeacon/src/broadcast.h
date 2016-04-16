#ifndef __BROADCAST__H__
#define __BROADCAST__H__


#undef  SOCK_SEL_TYPE_READ
#define SOCK_SEL_TYPE_READ 			0
#undef  SOCK_SEL_TYPE_WRITE
#define SOCK_SEL_TYPE_WRITE 		1




// �㲥���������������󷵻ر���ip�Ͷ˿ں�
class UDPSOCKET
{
public:
	UDPSOCKET();
	~UDPSOCKET();

public:
	// ��������: ����Ϊ�㲥ģʽ
	// ��������: SockFd���׽���������
	// �� �� ֵ: �ɹ�����0��ʧ�ܷ��� -1
	static int 
	SetSocketBroacast(int SockFd);
	
	// ��������: ����������Ϊ���������ɸ���״̬
	// ��������: SockFd���׽���������
	// �� �� ֵ: �ɹ�����0��ʧ�ܷ��� -1
	static int 
	SetSocketStatus(int SockFd);

	// ��������: ��ʼ���㲥������
	// ��������: udpsocket����������������׽���������
	//           usPort����������������˿�
	// �� �� ֵ: �ɹ�����0��ʧ�ܷ��� -1
	static int 
	InitUdpListen(int &UdpSocket, unsigned short usPort);

	// ��������: ��ʼ�� udp �׽���
	// ��������: SockFd������������׽���������
	// �� �� ֵ: �ɹ�����0��ʧ�ܷ��� -1
	static int
	InitUdpSocket(int &SockFd);

	static int 
	BindBroadcastSocket(int sockFd, unsigned short port);

	static int 
	InitUdpSocket();

	// ��������: �� IP ��ַ�����ַ���Ϣ
	// ��������: nUdpSocket���׽���������
	//           pBuf�������ַ����ݣ�nLen�������ַ����ݳ��ȣ�  
	//           pIp�����͵�Ŀ��ip��nPort��Ŀ��ip�����˿ڣ�
	//           ulTimeout����ʱʱ�䣬��λ����
	// �� �� ֵ: �ɹ�����0��ʧ�ܷ��� -1
	static int 
	SendUdpDataToIPAddr(int nUdpSocket, char *pBuf, int nLen, 
								char *pIp, int nPort, unsigned long ulTimeout);

	// ��������: ��ѯ�鿴�׽�����������д״̬
	// ��������: SockFd���׽���������
	//           nType�����ͻ��ǽ���
	//           ulTimeout����ʱʱ�䣬��λ����
	// �� �� ֵ: �ɹ�����0��ʧ�ܷ��� -1
	static int 
	SelectSingleSocket(int SockFd, int nType, unsigned long ulTimeout);

	// ��������: �� sockaddr ��ַ�����ַ���Ϣ
	// ��������: nUdpSocket���׽���������
	//           pBuf�������ַ����ݣ�nLen�������ַ����ݳ��ȣ�  
	//           cliaddr�����͵�Ŀ���ַ
	//           ulTimeout����ʱʱ�䣬��λ����
	// �� �� ֵ: �ɹ�����0��ʧ�ܷ��� -1
	static int 
	SendUdpDataToSockaddr(int nUdpSocket, const char *pBuf, int nLen, 
							struct sockaddr_in *cliaddr, unsigned long ulTimeout);

	// ��������: ���� socket ��ַ�ַ���Ϣ
	// ��������: nUdpSocket���׽���������
	//           pBuf�������ַ����ݣ�nLen�������ַ����ݳ��ȣ�  
	//           cliaddr��������������յĵ�ַ��AddrLen��������������յĵ�ַ���ȣ�
	//           ulTimeout����ʱʱ�䣬��λ����
	// �� �� ֵ: �ɹ�����0��ʧ�ܷ��� -1
	static int RecvUdpData(int nUdpSocket, char *pBuf, int nLen, 
		struct sockaddr_in *cliaddr, int* AddrLen, unsigned long ulTimeout);

	// ��������: ԭ close �����ķ�װ
	// �������: iSockFD : Ҫ�رյ��׽���������
	// �������: ��
	// �� �� ֵ: 0 : �رճɹ�;-1: �ر�ʧ��
	static int 
	Close(int iSockFD);

	static unsigned int 
	GetBroadByIfName(const char *ifname);

	
	static int 
	InitBroadcastAddr(struct sockaddr_in *broadcastAddr, const char* ifname, unsigned short port);

	static const char* 
	inetNtoA(unsigned int ip);
};





void *StartBcastID(void* arg);




#endif /*__BROADCAST__H__*/

