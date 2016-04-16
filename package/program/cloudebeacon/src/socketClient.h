#ifndef __SOCKET_CLIENT_H__
#define __SOCKET_CLIENT_H__

#include <pthread.h>
#include "netSockTcp.h"


class CConWebSerProc
{
public:
	CConWebSerProc();
	~CConWebSerProc()
	{
		pthread_rwlock_destroy(&m_exitLock);
		if (m_tcp)
		{
			delete m_tcp, m_tcp = NULL;
		}
	}
	
	int conWebSerProcRun()
	{
		pthread_rwlock_wrlock(&m_exitLock);
		m_isExitWebSerPthread = false;
		pthread_rwlock_unlock(&m_exitLock);

		pthread_create(&m_conWebSerPthreadID, NULL, conWebSerPthread, (void*)this);
		//pthread_detach(m_conWebSerPthreadID);
		return 0;
	}
	void conWebSerProcExit()
	{
		pthread_rwlock_wrlock(&m_exitLock);
		m_isExitWebSerPthread = true;
		pthread_rwlock_unlock(&m_exitLock);
		pthread_join(m_conWebSerPthreadID, NULL);
	}

private:
	static void* conWebSerPthread(void* arg);

	// 函数功能: 向服务器证明身份函数
	// 函数参数: sockFd，套接字描述符
	//           uID，身份识别usid
	// 返 回 值: 成功返回1，网络出错返回0，认证出错返回-1；
	static int  ProveToServer(int sockFd, const char* uID);
	static char* MakeSerGetCBCInfoAck();
	static void dealWithSerGetCBCinfo(int sockfd, char* buff);
	static void dealWithSetSerInfo(int sockfd, char* buff, int buffLen);
  	static void dealWithSetUPLInterval(int sockfd, char* buff);
	static void dealWithAplPhoBind(int sockFd, char* buff, int buffLen);
	static void dealWithAplPhoUnbind(int sockFd, char* buff, int buffLen);
	static void dealWithUplBluConfFile(int sockFd, char* buff, int buffLen);
	static void dealWithUpdBluConfFile(int sockFd, char* buff, int buffLen);
	static void dealWithSerInfo(int sockfd);
	static void UploadDataToNet(int sockFd, const char* data, int dataLen, u_int16 typeID);

private:
	pthread_t m_conWebSerPthreadID;
	bool m_isExitWebSerPthread;
	pthread_rwlock_t m_exitLock;
	CSocketTCP* m_tcp;

public:
	int m_reconnectWifi;
	int m_reconnectFlag;
};







#endif /*__SOCKET_CLIENT_H__*/

