#ifndef __CBPROJ__MAiN_H__
#define __CBPROJ__MAiN_H__

#include <pthread.h>

#ifdef UPDATEBLUEBIN
#include "upBToothBin.h"
#endif

#ifdef UPLOADMACINFO
#include "gatherMac.h"
#endif

#ifdef LOCALSOCKET  
#include "pthreadServer.h"
#endif

#ifdef SERIALSCOM
#include "ble_central.h"
#include "SerialCom.h"
#endif 

#ifdef BROADCAST
#include "broadcast.h"
#endif

#include "beaconConf.h"
#include "socketClient.h"
#include "rbtree.h"



#if __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */


typedef struct _taskNode
{
	struct rb_node     treeNode;                  // ���ڵ�
	unsigned long long executeTime;               // key��ִ�е�ʱ��㣬��λ���룬
	int                id;                        // ���
	//int              interTime;                 // ���ʱ�䣬��λ����
	//void             (*pCheckInterTime)(void);  // �����ʱ��
	//void             (*pGetInterTime)(void);    // ��ȡ���ʱ��
	void               (*pHandle)(void* arg);
	void*              arg;                       // �ص���������
}TaskNode;

typedef struct _taskTree
{
	struct rb_root  treeRoot; 
	unsigned int    taskNum;
	pthread_mutex_t mutex;       // ������
	void (*freeNode)(TaskNode*);
}TaskTree;

typedef struct _strInfo
{
	char* pBuff;
	int   buffLen;
}StrInfo;


#if __cplusplus
}
#endif

static const int TaskNodeLen = sizeof(TaskNode);
static const int TaskTreeLen = sizeof(TaskTree);

class CTask
{
public:
	CTask();
	~CTask();
	// ��������: 
	// ��������: 
	// �� �� ֵ: 
	TaskNode* getMinTime(void);
	// ��������: 
	// ��������: 
	// �� �� ֵ: 
	TaskNode* getMaxTime(void);
	// ��������: ���һ������ʱ����ѯ��
	// ��������: id���¼�id�ţ�interTime����ʱʱ��(��λ����)
	//           pTimeOut����ʱ�����arg����ʱ�������������
	// �� �� ֵ: 
	int addOneTask(int id, 
			void (*pTimeOut)(void* arg), void* arg);

	int addOneTask(TaskNode* tasker);
	// ��������: �����¼�id��ɾ��һ���¼�
	// ��������: �¼�id�š�
	// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1
	int delOneTask(int id);
	// ��������: �鿴�Ѿ����������
	// ��������: 
	// �� �� ֵ: 
	void checkAddTask(void);
	// ��������: 
	// ��������: 
	// �� �� ֵ: 
	int changeOneTask(TaskNode* pNode, unsigned long long curTime, int timeOut);
	// ��������: ��ȡ��ǰʱ�������
	// ��������: ��
	// �� �� ֵ: ��ǰʱ�������
	static unsigned long long getCurTime(void);

	// ��������: ͨ��id�ţ���ȡ���ʱ��
	// ��������: ����id��
	// �� �� ֵ: �ɹ����ػ�ȡ����ʱ�䣬ʧ�ܷ���-1
	int getInterTimeByID(int id);
	
private:
	// ��������: 
	// ��������: 
	// �� �� ֵ: 
	static inline void freeNode(TaskNode* node);
	// ��������: 
	// ��������: 
	// �� �� ֵ: 
	TaskNode* createNode(int id, int interTime, void (*pHandle)(void* arg), void* parg);
	// ��������: 
	// ��������: 
	// �� �� ֵ: 
	void destroyAllNode(void);
	// ��������: ͨ������id�Ż�ȡ��ǰ����ڵ�
	// ��������: ����id��
	// �� �� ֵ: ����ڵ�
	TaskNode* getTask(int id);
	

private:
	TaskTree* m_tree;
};












class cloudBeaconMain
{
public:
	cloudBeaconMain()
	{
		m_conWebSer = NULL;
		m_config = new IbeaconConfig;
		m_task   = new CTask;
		m_isExitMain = false;
		
	#ifdef BROADCAST
		m_phoneBcast = NULL;
	#endif

	#ifdef SERIALSCOM
		m_central = NULL;
	#endif
		pthread_rwlock_init(&m_exitLock, NULL);
	}
	
	~cloudBeaconMain()
	{
		if (m_phoneBcast) 
			delete m_phoneBcast, m_phoneBcast = NULL;
		if (m_conWebSer) 
			delete m_conWebSer, m_conWebSer = NULL;
	#ifdef SERIALSCOM
		if (m_central) 
			delete m_central, m_central = NULL;
	#endif
		if (m_config) 
			delete m_config, m_config = NULL;
		if (m_task) 
			delete m_task, m_task = NULL;

		pthread_rwlock_destroy(&m_exitLock);
	}
	void run(int argc, char** argv);
	//void doExitProgram(int signo);
	void stop();

public:
	IbeaconConfig* m_config;
	CTask* m_task;
	int setSerials();

	pthread_rwlock_t m_exitLock;
	bool m_isExitMain;

public:
	CConWebSerProc* m_conWebSer;
	int  conWebSerProcStart();
	void conWebSerProcExit();


#ifdef BROADCAST
	CBcast* m_phoneBcast;
	int  broadcastPthreadStart();
	void broadcastPthreadExit();
#endif

	
#ifdef SERIALSCOM
	BleCentral* m_central;
	int resetBlueCentral();
	int  startBlueCentral();
	void endBlueCentral();
	int  reloadBlueConfFile();
	int getBlueMacAddr(char* mac);
#endif	

private:
	static void usage(char*);
	void parseComLineAndConfFile(int argc, char **argv);
};





typedef enum _DebugLevel
{
	I_DEBUG = 1,      // ������Ϣ
	I_INFO,           // ������Ϣ
	I_WARN,           // ������Ϣ
	I_ERROR,          // ������Ϣ��д�ռ�
	I_FATAL,          // ʧ����Ϣ��д��־
}DebugLevel;

#define Debug(level, format...) _debug(__FUNCTION__, __LINE__, level, format)






void doExitProgram(int signo);


static inline void* safeMalloc(int size)
{
	void* ptr = malloc(size);
	if (!ptr)
	{
		Debug(I_FATAL, "malloc failed.");
		doExitProgram(SIGINT);
		sleep(10);
	}
	return ptr;
}

static inline void safeFree(void* ptr)
{
	if (ptr) free(ptr), ptr = NULL;
}






// ���¼������������ļ�
#ifdef SERIALSCOM
BleCentral *getCentralPosition();
int  reloadBlueConfFile();
#endif

IbeaconConfig* GetConfigPosition();

static inline void* CJSONMalloc(size_t size)
{
	void* temp = malloc(size);
	if (NULL == temp)
	{
		Debug(I_ERROR, "CJSON Malloc Failed.");
		return NULL;
	}
	return temp;
}
static inline void CJSONFree(void* pstr)
{
	free(pstr);
}


#endif /*__CBPROJ__MAiN_H__*/

