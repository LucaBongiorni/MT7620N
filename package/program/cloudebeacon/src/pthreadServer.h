#ifndef __PTHREAD_SERVER_H__
#define __PTHREAD_SERVER_H__

#include "StrOperate.h"
#include <pthread.h>


typedef struct __clientID
{
	pthread_t ptheadID;
	bool used;
}ClientID;


class phoneSerProc
{
public:
	phoneSerProc();
	~phoneSerProc()
	{
		pthread_rwlock_destroy(&m_exitLock);
		//if (m_clientPthreadID) free(m_clientPthreadID), m_clientPthreadID = NULL;
	}
	
	int  phoneSerPthreadRun()
	{
		pthread_rwlock_wrlock(&m_exitLock);
		m_exitPhoneSerPthread = false;
		pthread_rwlock_unlock(&m_exitLock);

		int nRet;
		nRet = pthread_create(&m_phoneSerPthreadID, NULL, phoneSerPthread, (void*)this);
		return nRet;
	}
	void phoneSerPthreadExit()
	{
		pthread_rwlock_wrlock(&m_exitLock);
		m_exitPhoneSerPthread = true;
		pthread_rwlock_unlock(&m_exitLock);
		pthread_join(m_phoneSerPthreadID, NULL);

		//printf("111111111111111111111111111111111-------------1111111111111\n");
		/*
		for (int i=0; i<m_listenNum; ++i)
		{
			if (m_clientPthreadID[i].used == true)
			{
				// 已经交由线程池处理，这里不能再处理了。
				pthread_join(m_clientPthreadID[i].ptheadID, NULL);
			}
		}
		*/
	}

	//int RecvOnePkg(int sockFD, char *recvBuff, int buffLen, int timeOut);
	
private:
	static void* phoneSerPthread(void* arg);
	static void* ClientPthreadProc(void* arg);

	// 函数功能: 认证连接
	// 函数参数: sockFd，套接字描述符
	//           prove，是否已经绑定
	//           phoneHandle，手机通讯句柄
	// 返 回 值:
	static int
	ProvePhoneConnect(int sockFd, bool prove, int phoneHandle, char* key);

	// 函数功能: 解析手机发送过来的key
	// 函数参数: key，手机key
	//           mac，WAN口mac地址
	static void parsePhoneKey(char* key, const char* mac);

/*
	int addClientPthreadID(pthread_t pthreadID)
	{
		for (int i=0; i<m_listenNum; ++i)
		{
			if (m_clientPthreadID[i].used == false)
			{
				m_clientPthreadID[i].ptheadID = pthreadID;
				m_clientPthreadID[i].used = true;
				return 0;
			}
		}
		return -1;
	}
	void delClientPhteadID(pthread_t pthreadID)
	{
		for (int i=0; i<m_listenNum; ++i)
		{
			if (m_clientPthreadID[i].ptheadID == pthreadID)
			{
				m_clientPthreadID[i].ptheadID = 0;
				m_clientPthreadID[i].used = false;
				return ;
			}
		}
	}
*/
	
private:
	int m_listenNum;
	pthread_t m_phoneSerPthreadID;
	bool m_exitPhoneSerPthread;
	pthread_rwlock_t m_exitLock;
	//ClientID* m_clientPthreadID;      // 用于保存子端的线程id
};



typedef struct _cliParam
{
	phoneSerProc* phoneSer;
	int sockFd;
	//pthread_t pthreadID;
}CliParam;


// 函数功能: 对设备进行绑定
void 
dealWithPhoBindCBC(int sockFD, int phoneHandle, char* RecvBuff, 
		int buffLen, const char* key);
// 函数功能: 对设备进行解绑
void 
dealWithPhoUnbindCBC(int sockFD, int phoneHandle, char* RecvBuff, 
		int buffLen, const char* key);



int  phoneSerPthreadStart();
void phoneSerPthreadExit();


#endif /*__PTHREAD_SERVER_H__*/

