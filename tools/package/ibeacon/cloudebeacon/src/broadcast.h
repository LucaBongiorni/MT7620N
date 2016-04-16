#ifndef __BROADCAST__H__
#define __BROADCAST__H__


#undef  SOCK_SEL_TYPE_READ
#define SOCK_SEL_TYPE_READ 			0
#undef  SOCK_SEL_TYPE_WRITE
#define SOCK_SEL_TYPE_WRITE 		1




// 广播服务器，对于请求返回本机ip和端口号
class UDPSOCKET
{
public:
	UDPSOCKET();
	~UDPSOCKET();

public:
	// 函数功能: 设置为广播模式
	// 函数参数: SockFd，套接字描述符
	// 返 回 值: 成功返回0，失败返回 -1
	static int 
	SetSocketBroacast(int SockFd);
	
	// 函数功能: 设置描述符为非阻塞，可复用状态
	// 函数参数: SockFd，套接字描述符
	// 返 回 值: 成功返回0，失败返回 -1
	static int 
	SetSocketStatus(int SockFd);

	// 函数功能: 初始化广播服务器
	// 函数参数: udpsocket，输出参数，监听套接字描述符
	//           usPort，输入参数，监听端口
	// 返 回 值: 成功返回0，失败返回 -1
	static int 
	InitUdpListen(int &UdpSocket, unsigned short usPort);

	// 函数功能: 初始化 udp 套接字
	// 函数参数: SockFd，输出参数，套接字描述符
	// 返 回 值: 成功返回0，失败返回 -1
	static int
	InitUdpSocket(int &SockFd);

	static int 
	BindBroadcastSocket(int sockFd, unsigned short port);

	static int 
	InitUdpSocket();

	// 函数功能: 往 IP 地址发送字符信息
	// 函数参数: nUdpSocket，套接字描述符
	//           pBuf，发送字符内容；nLen，发送字符内容长度；  
	//           pIp，发送的目标ip；nPort，目标ip监听端口；
	//           ulTimeout，超时时间，单位毫秒
	// 返 回 值: 成功返回0，失败返回 -1
	static int 
	SendUdpDataToIPAddr(int nUdpSocket, char *pBuf, int nLen, 
								char *pIp, int nPort, unsigned long ulTimeout);

	// 函数功能: 轮询查看套接字描述符读写状态
	// 函数参数: SockFd，套接字描述符
	//           nType，发送还是接收
	//           ulTimeout，超时时间，单位毫秒
	// 返 回 值: 成功返回0，失败返回 -1
	static int 
	SelectSingleSocket(int SockFd, int nType, unsigned long ulTimeout);

	// 函数功能: 往 sockaddr 地址发送字符信息
	// 函数参数: nUdpSocket，套接字描述符
	//           pBuf，发送字符内容；nLen，发送字符内容长度；  
	//           cliaddr，发送的目标地址
	//           ulTimeout，超时时间，单位毫秒
	// 返 回 值: 成功返回0，失败返回 -1
	static int 
	SendUdpDataToSockaddr(int nUdpSocket, const char *pBuf, int nLen, 
							struct sockaddr_in *cliaddr, unsigned long ulTimeout);

	// 函数功能: 接收 socket 地址字符信息
	// 函数参数: nUdpSocket，套接字描述符
	//           pBuf，发送字符内容；nLen，发送字符内容长度；  
	//           cliaddr，输出参数，接收的地址；AddrLen，输出参数，接收的地址长度；
	//           ulTimeout，超时时间，单位毫秒
	// 返 回 值: 成功返回0，失败返回 -1
	static int RecvUdpData(int nUdpSocket, char *pBuf, int nLen, 
		struct sockaddr_in *cliaddr, int* AddrLen, unsigned long ulTimeout);

	// 函数功能: 原 close 函数的封装
	// 输入参数: iSockFD : 要关闭的套接字描述符
	// 输出参数: 无
	// 返 回 值: 0 : 关闭成功;-1: 关闭失败
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

