#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include <linux/rtc.h>
#include <asm/ioctls.h>
#include <bits/errno.h>



#include "broadcast.h"
#include "defCom.h"
#include "procotol.h"
#include "cbProjMain.h"
#include "cJSON.h"
#include "dealWithOpenwrt.h"
#include "UCI_File.h"


#ifndef IFNAMESIZ
#define IFNAMESIZ  16
#endif


int 
UDPSOCKET::SetSocketBroacast(int SockFd)
{
	int optval = 1;
	return setsockopt(SockFd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)); 
}

int 
UDPSOCKET::SetSocketStatus(int SockFd)
{
	//取得文件描述符的状态
	int	nFlags = fcntl(SockFd, F_GETFL, 0);
	if (-1 == nFlags) 
	{	
		Debug(I_ERROR, "SetSocketStatus error");
		return -1;
	}

	//设置文件描述符为非阻塞
	if ( -1 == fcntl(SockFd, F_SETFL, nFlags | O_NONBLOCK) )	
	{
		Debug(I_ERROR, "SetSocketStatus fcntl error");
		return -1;
	}
	
	int bReUseAddr = 1;
	//设置套接字的属性使它能够在计算机重启的时候可以再次使用套接字的端口和IP 
	int error = setsockopt(SockFd, SOL_SOCKET, SO_REUSEADDR, (const char*) &bReUseAddr, sizeof(bReUseAddr));
	if (error != 0 )
	{
		return -1;
	}
	
	//SO_LINGER, 优雅关闭socket，会保证缓冲区的数据全部发送完成
	//此选项对UDP的SOCKET无效
	//struct linger zeroLinger = {1, 2};
	//setsockopt(SockFd, SOL_SOCKET, SO_LINGER, (const char *)&zeroLinger, sizeof(linger));
	return 0;
}

int 
UDPSOCKET::InitUdpListen(int &UdpSocket, unsigned short usPort)
{
	int iRtn;
	int SockFd;

	SockFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (SockFd != -1)
	{
		SetSocketStatus(SockFd);
		struct sockaddr_in MyAddr = {0};
		MyAddr.sin_family 	   = AF_INET;
		MyAddr.sin_port 	   = htons(usPort);
		MyAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		iRtn = bind(SockFd, (struct sockaddr*)&MyAddr, sizeof(struct sockaddr_in));
		if ( -1 == iRtn)
		{
		    close(SockFd);
		    return -1;
		} 
		else
		{
		    UdpSocket = SockFd;
		    return 0;
		}
	}
	else
	{
	    return -1;
	}
}

int 
UDPSOCKET::InitUdpSocket(int &SockFd)
{
	SockFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (SockFd == -1)
	{
		Debug(I_ERROR, "Create UDP Socket Failed. %m");
		return -1;
	}

	return SetSocketStatus(SockFd);
}

int 
UDPSOCKET::InitUdpSocket()
{
	int SockFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (SockFd == -1)
	{
		Debug(I_ERROR, "Create UDP Socket Failed. %m");
		return -1;
	}

	return SockFd;
}

int 
UDPSOCKET::BindBroadcastSocket(int sockFd, unsigned short port)
{
	struct sockaddr_in addrto;  
    bzero(&addrto, sizeof(struct sockaddr_in));  
    addrto.sin_family = AF_INET;  
    addrto.sin_addr.s_addr = htonl(INADDR_ANY);  
    addrto.sin_port = htons(port);  

	if (bind(sockFd,(struct sockaddr *)&(addrto), sizeof(struct sockaddr_in)) == -1)   
    {     
        Debug(I_ERROR, "bind error %m...");  
        return -1;  
    }  
	return 0;
}


int 
UDPSOCKET::SendUdpDataToIPAddr(int nUdpSocket, char *pBuf, int nLen, 
								char *pIp, int nPort, unsigned long ulTimeout)
{
	if (nUdpSocket < 0)
		return -1;
	
	struct sockaddr_in addrto = {0};
	addrto.sin_family      = AF_INET; //默认
	addrto.sin_addr.s_addr = inet_addr(pIp);
	addrto.sin_port        = htons(nPort);
	
	return SendUdpDataToSockaddr(nUdpSocket, pBuf, nLen, &addrto, ulTimeout);
}


int
UDPSOCKET::SelectSingleSocket(int SockFd, int nType, unsigned long ulTimeout)
{
	if (SockFd < 0)
		return -1;
	
	struct timeval timeout;
	int rc = 0;
	timeout.tv_sec  = ulTimeout / 1000;
	timeout.tv_usec = (ulTimeout % 1000) * 1000;
	while(1)
	{
		fd_set fd;
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
			if (FD_ISSET(SockFd, &fd))
				return rc;
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
}


int 
UDPSOCKET::SendUdpDataToSockaddr(int nUdpSocket, const char *pBuf, int nLen, 
							struct sockaddr_in *cliaddr, unsigned long ulTimeout)
{
	int iRetVal;
	if (nUdpSocket < 0 || NULL == cliaddr)
	{
		return -1;
	}

	if (SelectSingleSocket(nUdpSocket, SOCK_SEL_TYPE_WRITE, ulTimeout) > 0)
	{
		iRetVal = sendto(nUdpSocket, pBuf, nLen, 0, 
						(struct sockaddr*)cliaddr, sizeof(struct sockaddr_in));
		if (iRetVal < 0)
		{
			printf("send udp err:%s\n", strerror(errno));
		}
		return iRetVal;
	} 
	else
	{
		return 0;
	}
}

int 
UDPSOCKET::RecvUdpData(int nUdpSocket, char *pBuf, int nLen, 
		struct sockaddr_in *cliaddr, int* AddrLen, unsigned long ulTimeout)
{
	if (nUdpSocket < 0 || NULL == cliaddr)
	{
		return -1;
	}
	int nRet = SelectSingleSocket(nUdpSocket, SOCK_SEL_TYPE_READ, ulTimeout);
	if (nRet > 0)
	{
		int recvlen = 0;		
		do
		{
			recvlen = recvfrom(nUdpSocket, pBuf, nLen, 0, 
				(struct sockaddr*)cliaddr, (socklen_t*)AddrLen);
		}while(recvlen < 0 && EAGAIN == errno);
			
		if(recvlen < 0)
		{ 
			printf("recv udp err:%s\n", strerror(errno));
		}
		return recvlen;
	}
	else
	{
		return 0;
	}
}

int 
UDPSOCKET::Close(int iSockFD)
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

const char* 
UDPSOCKET::inetNtoA(unsigned int ip)
{
	static char buf[sizeof "123.456.789.123"] = {0};
	unsigned char *tmp = (unsigned char *)&ip;
	sprintf(buf, "%d.%d.%d.%d", tmp[0] & 0xff, tmp[1] & 0xff,
							    tmp[2] & 0xff, tmp[3] & 0xff);
	return buf;
}


unsigned int 
UDPSOCKET::GetBroadByIfName(const char *ifname)
{
	struct ifreq if_data;
	int sockd;
	unsigned int broadAddr;

	if (ifname == NULL || strlen(ifname) > (IFNAMESIZ-1))
	{
		printf("[%s][%d]Param Error.", __FILE__, __LINE__);
		return 0;
	}
	
	if ((sockd = socket(AF_INET, SOCK_PACKET, htons(0x8086))) < 0) 
	{
		printf("[%s][%d]Create Socket Failed.\n", __FILE__, __LINE__);
		return 0;
	}

	strcpy(if_data.ifr_name, ifname);
	if (ioctl(sockd, SIOCGIFBRDADDR, &if_data) < 0) 
	{
		printf("[%s][%d]Ioctl Get Socket Ifaddr Failed.", __FILE__, __LINE__);
		close(sockd);
		return 0;
	}
	memcpy((void *)&broadAddr, (char *)&if_data.ifr_addr.sa_data + 2, 4);
	close(sockd);

	write_normal_log("Broadcast-IP: %s\n", inetNtoA(broadAddr));
	return broadAddr;
}

int 
UDPSOCKET::InitBroadcastAddr(struct sockaddr_in *broadcastAddr, const char* ifname, unsigned short port)
{
	if (NULL == broadcastAddr || NULL == ifname)
	{
		return -1;
	}
	
	unsigned int broadcastIP;
	// 通过网口名字获取广播地址
	broadcastIP = GetBroadByIfName(ifname);
	if (broadcastIP == 0)
	{
		Debug(I_ERROR, "Get Broadcast By Ifname Failed. %m");
		return -1;
	}
	
	/*将获得的广播地址复制到broadcast_addr */
	memset(broadcastAddr, 0, sizeof(struct sockaddr_in));
	broadcastAddr->sin_family      = AF_INET;
	broadcastAddr->sin_addr.s_addr = broadcastIP;
	broadcastAddr->sin_port        = htons(port);
	Debug(I_INFO, "Broadcast-IP: %s\n", inet_ntoa(broadcastAddr->sin_addr));

	return 0;
}



char* 
CBcast::makeBcastMes()
{
	IbeaconConfig *conf = GetConfigPosition();
	cJSON *root = NULL, *netInfo = NULL, *DLinfo = NULL;
	const char *ssid = NULL;
	char* out = NULL;
	char salt[256] = {0};
	
	root = cJSON_CreateObject();	
	cJSON_AddItemToObject(root, "name", cJSON_CreateString("Get Device Info"));
	out = GetAllAvaildNetInfo();
	netInfo = cJSON_Parse(out);
	if (netInfo)
		cJSON_AddItemReferenceToObject(root, "netInfo", netInfo);
	else
		cJSON_AddItemToObject(root, "netInfo", netInfo = cJSON_CreateArray());
	cJSON_AddNumberToObject(root, "LocalPort", conf->getLocalPort());
	// 添加wifi ssid
	ssid = getWiFiSSID();
	if (ssid)
		cJSON_AddStringToObject(root, "WiFiSSID", ssid);

	cJSON_AddStringToObject(root, "LanIfname", LAN_IFNAME);
	cJSON_AddItemToObject(root, "DLFileInfo", DLinfo=cJSON_CreateObject());
	cJSON_AddStringToObject(DLinfo, "DL_URL", USB_DOWNLOAD_URL);
	cJSON_AddNumberToObject(DLinfo, "DL_PORT", HTTP_PORT);

	// 加密盐值
	if ( 0 == getLoginSaltVal("root", salt) )
		cJSON_AddStringToObject(root, "saltVal", salt);

	//////////////////////////////////////////////////////////////////////////
#ifdef OLD_VERSION
	// 兼顾老版本
	cJSON* Info = NULL;
	int netType;
	char* temp = NULL;
	unsigned long IP = 0;
	cJSON_AddItemToObject(root, "Info", Info = cJSON_CreateObject());
	if (ssid) cJSON_AddStringToObject(Info, "WiFiSSID", ssid);
	IP = GetIPByIfname(LAN_IFNAME);
	temp = inet_ntoa(*(struct in_addr*)&IP);
	cJSON_AddStringToObject(Info, "LanIP", temp);
	temp = GetLanMac();
	if (temp) cJSON_AddStringToObject(Info, "LanMac", temp);
	IP = GetWanIP();
	temp = inet_ntoa(*(struct in_addr*)&IP);
	cJSON_AddStringToObject(Info, "WanIP", temp);
	temp = GetWanMac();
	if (temp) cJSON_AddStringToObject(Info, "WanMac", temp);
	cJSON_AddNumberToObject(Info, "LocalPort", conf->getLocalPort());

	UCI *uci = new UCI(Network_Conf_File, true);
	const char* ifname = uci->UCI_GetOptionValue("wan", "ifname");
	if (strcmp(ifname, "apcli0") == 0)
		netType = 2;
	else if (strcmp(ifname, "eth0.2") == 0)
		netType = 1;
	else if (strcmp(ifname, "3g-wan") == 0)
		netType = 3;
	else
		netType = 0;
	delete uci, uci = NULL;
	cJSON_AddNumberToObject(Info, "netType", netType);
#endif
	//////////////////////////////////////////////////////////////////////////

	if (out) free(out), out = NULL;
	out = cJSON_Print(root);
	cJSON_Delete(root), root = NULL;
	cJSON_Delete(netInfo), netInfo = NULL;
	return out;
}

// 广播服务器线程
void* CBcast::bcastPthread(void* arg)
{
	int UdpSocket, nRet;
	char *recvBuff = new char[EVERY_PKG_LEN];
	struct sockaddr_in cliaddr;
	int AddrLen;
	IbeaconConfig *conf = GetConfigPosition();
	char *sendBuff = new char[EVERY_PKG_LEN];
	CBcast* bcast = (CBcast*)arg;

	// 初始化 udp
	nRet = UDPSOCKET::InitUdpSocket(UdpSocket);
	if (-1 == nRet)
	{
		Debug(I_DEBUG, "Init UDP socket failed.");
		return 0;
	}
	//printf("nRet=%d, UdpSocket=%d\n", nRet, UdpSocket);
	
	// 绑定
	nRet = UDPSOCKET::BindBroadcastSocket(UdpSocket, conf->getBcastPort());
	if (0 != nRet)
	{
		Debug(I_DEBUG, "Bind broadcast socket failed.");
		UDPSOCKET::Close(UdpSocket);
		return 0;
	}
	
	int sendBuffLen = 0;
	char* json = NULL;
	const char* pSendBuff = sendBuff;
	
	// 轮询接收
	for (;;)
	{
		pthread_rwlock_rdlock(&bcast->m_rdLock);
		if (bcast->m_isExitBcastPthread)
		{
			pthread_rwlock_unlock(&bcast->m_rdLock);
			break;
		}
		pthread_rwlock_unlock(&bcast->m_rdLock);
		
		// 接收
		memset(recvBuff, 0, EVERY_PKG_LEN);
		u_int16 length  = 0;
		u_int16 typeID  = 0;
		u_int8 PkgTotal = 0;
		u_int8 PkgSeq   = 0;
		char* pRecvBuff = recvBuff;
		bzero(&cliaddr, sizeof(struct sockaddr_in)); 
		AddrLen = sizeof(struct sockaddr_in);

		nRet = UDPSOCKET::RecvUdpData(UdpSocket, recvBuff, EVERY_PKG_LEN, &cliaddr, &AddrLen, 1000);
		if (0 >= nRet)
		{
			usleep(3000);
			//write_log("recv timeout.");
			continue;
		}
	#if 0		
		for (int i=0; i<nRet; ++i)
		{
			printf("%x ", recvBuff[i]);
		}
		printf("\n");
		for (int i=0; i<nRet; ++i)
		{
			printf("%c ", recvBuff[i]);
		}
		printf("\n");
		printf("clientIP=%s, Port=%d\n", UDPSOCKET::inetNtoA(cliaddr.sin_addr.s_addr), 
			ntohs(cliaddr.sin_port));
	#endif
	#if 1
		// 解析第一层包头
		nRet = Procotol::CheckPkgHead(recvBuff, &length);
		if (nRet != 0)
		{
			Debug(I_DEBUG, "This is not out package.");
			continue;
		}
		pRecvBuff += (HEAD_LEN + 4);
		printf("length=%d, HEAD_LEN=%d\n", length, HEAD_LEN);
		// 解析第二层包头，只有一个包，多包认为是错误包，不理
		nRet = Procotol::DelPkgHeadAndTail(pRecvBuff, length-TAIL_LEN, &typeID, &PkgTotal, &PkgSeq);
		if (-1 == nRet || PkgTotal > 1)
		{
			Debug(I_DEBUG, "This is not out package.");
			continue;
		}
		// 查看是否为广播类型的包
		if (MES_BCT_GET_CON_INFO != typeID)
		{
			Debug(I_DEBUG, "This is not out package.");
			continue;
		}
	#endif		

		Debug(I_DEBUG, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		Debug(I_DEBUG, "clientIP=%s, Port=%d", UDPSOCKET::inetNtoA(cliaddr.sin_addr.s_addr), ntohs(cliaddr.sin_port));
		Debug(I_DEBUG, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

		// 拼接发送信息
		json = makeBcastMes();
		if (NULL == json)
		{
			Debug(I_DEBUG, "Make SendBuff Failed.");
			continue;
		}
		//printf("%s\n", json);
		nRet = strlen(json);
		memcpy(sendBuff, json, nRet);
		CJSONFree(json), json = NULL;
		sendBuffLen = Procotol::AddPkgHeadAndTail(sendBuff, nRet, MES_BCT_GET_CON_INFO, 1, 1);
		if (sendBuffLen == -1)
		{
			Debug(I_DEBUG, "Add package head and tail failed.");
			continue;
		}
	
		// 发送
		nRet = UDPSOCKET::SendUdpDataToSockaddr(UdpSocket, pSendBuff, sendBuffLen, &cliaddr, 5000);
		Debug(I_DEBUG, "send udp broadcast package, sendLen=%d", nRet);

		usleep(10);
	}
	
	UDPSOCKET::Close(UdpSocket);
	delete [] recvBuff;
	delete [] sendBuff;
	Debug(I_INFO, "boardcast pthread exit ...");
	return 0;
}

int CBcast::bcastPthreadRun()
{
	pthread_rwlock_wrlock(&m_rdLock);
	m_isExitBcastPthread = false;
	pthread_rwlock_unlock(&m_rdLock);

	int iRet;
	memset(&m_attr, 0, sizeof(pthread_attr_t));
	iRet = pthread_attr_init(&m_attr); /*初始化线程属性*/
	if (iRet != 0)
	{
		Debug(I_DEBUG, "pthread attr init failed.");
		return -1;
	}
	
	iRet = pthread_attr_setstacksize(&m_attr, PthreadStackSize);
	if (iRet != 0)
	{
		Debug(I_DEBUG, "pthread attr set stack size failed.");
		pthread_attr_destroy(&m_attr);
		return -1;
	}

	pthread_create(&m_bcastID, &m_attr, bcastPthread, (void*)this);
	pthread_detach(m_bcastID);

	pthread_attr_destroy(&m_attr);
	return 0;
}























#define Test_Port     40000
#ifdef server
// 广播服务器阻塞
int main()
{
	int UdpSocket, nRet;
	char recvBuff[1500] = {0};
	struct sockaddr_in localAddr;
	struct sockaddr_in cliaddr;
	int AddrLen;

	// 初始化 udp
	UdpSocket = UDPSOCKET::InitUdpSocket();
	if (-1 == UdpSocket)
	{
		Debug(I_DEBUG, "init udp socket failed.");
		return 0;
	}
	printf("UdpSocket=%d\n", UdpSocket);

	// 绑定
	nRet = UDPSOCKET::BindBroadcastSocket(UdpSocket, Test_Port);
	if (0 != nRet)
	{
		Debug(I_DEBUG, "bind broadcast socket failed.");
		UDPSOCKET::Close(UdpSocket);
		return 0;
	}
	Debug(I_INFO, "Bind udp broadcast socket success.\n");

 	// 接收的广播地址
    bzero(&cliaddr, sizeof(struct sockaddr_in));  
    //cliaddr.sin_family = AF_INET;  
    //cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);  
    //cliaddr.sin_port = htons(Test_Port); 

	// 轮询接收
	for (;;)
	{
		// 接收
		memset(recvBuff, 0, 1500);
		nRet = recvfrom(UdpSocket, recvBuff, 1500, 0, (struct sockaddr*)&cliaddr, (socklen_t*)&AddrLen);
		if (0 >= nRet)
		{
			usleep(3000*10);
			continue;
		}
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
		printf("nRet=%d, RecvBuff=%s\n", nRet, recvBuff);
		printf("clientIP=%s, Port=%d\n", UDPSOCKET::inetNtoA(cliaddr.sin_addr.s_addr), ntohs(cliaddr.sin_port));
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

		// 发送
		memset(recvBuff, 0, 1500);
		memset(recvBuff, 'a', 100);
		nRet = sendto(UdpSocket, recvBuff, 100, 0, (struct sockaddr*)&cliaddr, AddrLen);
		usleep(3000*10);
	}
	UDPSOCKET::Close(UdpSocket);
	return 0;
}
#endif



#ifdef serverNB
#define _DEBUG
// 广播服务器非阻塞
int main()
{
	int UdpSocket, nRet;
	char recvBuff[1500] = {0};
	struct sockaddr_in localAddr;
	struct sockaddr_in cliaddr;
	int AddrLen;

	// 初始化 udp
	nRet = UDPSOCKET::InitUdpSocket(UdpSocket);
	if (-1 == nRet)
	{
		write_error_log("111111111111111111111111");
		return 0;
	}
	printf("nRet=%d, UdpSocket=%d\n", nRet, UdpSocket);

	// 绑定
	nRet = UDPSOCKET::BindBroadcastSocket(UdpSocket, Test_Port);
	if (0 != nRet)
	{
		write_error_log("3333333333333333333333333333333");
		UDPSOCKET::Close(UdpSocket);
		return 0;
	}
	printf("Bind success.\n");

 	// 接收的广播地址
    bzero(&cliaddr, sizeof(struct sockaddr_in));  
    //cliaddr.sin_family = AF_INET;  
    //cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);  
    //cliaddr.sin_port = htons(Test_Port); 

	// 轮询接收
	for (;;)
	{
		// 接收
		memset(recvBuff, 0, 1500);
		nRet = UDPSOCKET::RecvUdpData(UdpSocket, recvBuff, 1500, &cliaddr, &AddrLen, 10*1000);
		if (0 >= nRet)
		{
			usleep(3000*10);
			continue;
		}
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
		printf("nRet=%d, RecvBuff=%s\n", nRet, recvBuff);
		printf("clientIP=%s, Port=%d\n", UDPSOCKET::inetNtoA(cliaddr.sin_addr.s_addr), ntohs(cliaddr.sin_port));
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

		// 发送
		memset(recvBuff, 0, 1500);
		memset(recvBuff, 'a', 100);
		nRet = UDPSOCKET::SendUdpDataToSockaddr(UdpSocket, recvBuff, strlen(recvBuff), &cliaddr, 5000);
		usleep(3000*10);
	}
	UDPSOCKET::Close(UdpSocket);
	return 0;
}
#endif


#ifdef broadcast_client
// 广播子端
int main(int argc, char** argv)
{
	int UdpSocket, nRet;
	char recvBuff[1500] = {0};
	struct sockaddr_in cliaddr;
	int AddrLen;
	unsigned int broadAddr;

	UdpSocket = UDPSOCKET::InitUdpSocket();
	if (-1 == UdpSocket)
	{
		write_error_log("11111111111111111111111111");
		return 0;
	}
	printf("Init succuss.\n");
	
	// 设置广播发送
	nRet = UDPSOCKET::SetSocketBroacast(UdpSocket);
	if (-1 == nRet)
	{
		write_error_log("22222222222222222222222222");
		UDPSOCKET::Close(UdpSocket);
		return 0;
	}
	printf("setsocket Broadcast succuss.\n");

	// 填充广播地址
	struct sockaddr_in addrto = {0};
	addrto.sin_family      = AF_INET; 
	addrto.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	addrto.sin_port        = htons(Test_Port);
	socklen_t addrLen = sizeof(struct sockaddr_in);

	struct sockaddr_in serAddr;
	memset(&serAddr, 0, sizeof(serAddr));
	socklen_t serAddrLen;

	// 发送
	memset(recvBuff, 'b', 100);
	nRet = sendto(UdpSocket, recvBuff, 100, 0, (struct sockaddr*)&addrto, addrLen);
	if (0 >= nRet)
	{
		write_error_log("44444444444444444444444444");
		UDPSOCKET::Close(UdpSocket);
		return 0;
	}

	// 接收
	memset(recvBuff, 0, 1500);
	nRet = recvfrom(UdpSocket, recvBuff, 1500, 0, (struct sockaddr*)&serAddr, &serAddrLen);
	printf("nRet=%d, recvBuff=%s\n", nRet, recvBuff);
	printf("clientIP=%s, Port=%d\n", UDPSOCKET::inetNtoA(serAddr.sin_addr.s_addr), ntohs(serAddr.sin_port));

	// 关闭
	UDPSOCKET::Close(UdpSocket);
	return 0;
}
#endif








