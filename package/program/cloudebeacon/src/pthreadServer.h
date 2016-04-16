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
				// �Ѿ������̳߳ش������ﲻ���ٴ����ˡ�
				pthread_join(m_clientPthreadID[i].ptheadID, NULL);
			}
		}
		*/
	}

	//int RecvOnePkg(int sockFD, char *recvBuff, int buffLen, int timeOut);
	
private:
	static void* phoneSerPthread(void* arg);
	static void* ClientPthreadProc(void* arg);

	// ��������: ��֤����
	// ��������: sockFd���׽���������
	//           prove���Ƿ��Ѿ���
	//           phoneHandle���ֻ�ͨѶ���
	// �� �� ֵ:
	static int
	ProvePhoneConnect(int sockFd, bool prove, int phoneHandle, char* key);

	// ��������: �����ֻ����͹�����key
	// ��������: key���ֻ�key
	//           mac��WAN��mac��ַ
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
	//ClientID* m_clientPthreadID;      // ���ڱ����Ӷ˵��߳�id
};



typedef struct _cliParam
{
	phoneSerProc* phoneSer;
	int sockFd;
	//pthread_t pthreadID;
}CliParam;


// ��������: ���豸���а�
void 
dealWithPhoBindCBC(int sockFD, int phoneHandle, char* RecvBuff, 
		int buffLen, const char* key);
// ��������: ���豸���н��
void 
dealWithPhoUnbindCBC(int sockFD, int phoneHandle, char* RecvBuff, 
		int buffLen, const char* key);



int  phoneSerPthreadStart();
void phoneSerPthreadExit();


#endif /*__PTHREAD_SERVER_H__*/

