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
	struct rb_node     treeNode;                  // 树节点
	unsigned long long executeTime;               // key，执行的时间点，单位毫秒，
	int                id;                        // 标记
	//int              interTime;                 // 间隔时间，单位毫秒
	//void             (*pCheckInterTime)(void);  // 检测间隔时间
	//void             (*pGetInterTime)(void);    // 获取间隔时间
	void               (*pHandle)(void* arg);
	void*              arg;                       // 回调函数参数
}TaskNode;

typedef struct _taskTree
{
	struct rb_root  treeRoot; 
	unsigned int    taskNum;
	pthread_mutex_t mutex;       // 互斥锁
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
	// 函数功能: 
	// 函数参数: 
	// 返 回 值: 
	TaskNode* getMinTime(void);
	// 函数功能: 
	// 函数参数: 
	// 返 回 值: 
	TaskNode* getMaxTime(void);
	// 函数功能: 添加一个任务到时间轮询中
	// 函数参数: id，事件id号；interTime，超时时间(单位毫秒)
	//           pTimeOut，超时句柄；arg，超时句柄函数参数；
	// 返 回 值: 
	int addOneTask(int id, 
			void (*pTimeOut)(void* arg), void* arg);

	int addOneTask(TaskNode* tasker);
	// 函数功能: 根据事件id，删除一个事件
	// 函数参数: 事件id号。
	// 返 回 值: 成功返回0，失败返回-1
	int delOneTask(int id);
	// 函数功能: 查看已经加入的任务
	// 函数参数: 
	// 返 回 值: 
	void checkAddTask(void);
	// 函数功能: 
	// 函数参数: 
	// 返 回 值: 
	int changeOneTask(TaskNode* pNode, unsigned long long curTime, int timeOut);
	// 函数功能: 获取当前时间毫秒数
	// 函数参数: 无
	// 返 回 值: 当前时间毫秒数
	static unsigned long long getCurTime(void);

	// 函数功能: 通过id号，获取间隔时间
	// 函数参数: 任务id号
	// 返 回 值: 成功返回获取到的时间，失败返回-1
	int getInterTimeByID(int id);
	
private:
	// 函数功能: 
	// 函数参数: 
	// 返 回 值: 
	static inline void freeNode(TaskNode* node);
	// 函数功能: 
	// 函数参数: 
	// 返 回 值: 
	TaskNode* createNode(int id, int interTime, void (*pHandle)(void* arg), void* parg);
	// 函数功能: 
	// 函数参数: 
	// 返 回 值: 
	void destroyAllNode(void);
	// 函数功能: 通过任务id号获取当前任务节点
	// 函数参数: 任务id号
	// 返 回 值: 任务节点
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
	I_DEBUG = 1,      // 调试信息
	I_INFO,           // 调试信息
	I_WARN,           // 警告信息
	I_ERROR,          // 错误信息，写日记
	I_FATAL,          // 失败信息，写日志
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






// 重新加载蓝牙配置文件
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

