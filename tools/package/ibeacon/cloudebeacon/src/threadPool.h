#ifndef THREAD_POOL_H
#define THREAD_POOL_H



/*
 *线程池里所有运行和等待的任务都是一个CThread_worker
 *由于所有任务都在链表里，所以是一个链表结构
 */
class CThreadWorker
{
public:
    /*回调函数，任务运行时会调用此函数，注意也可声明成其它形式*/
    void* (*process) (void *arg);
	/*回调函数的参数*/
    void* arg;
    CThreadWorker *next;
	
public:
	CThreadWorker(){};
	~CThreadWorker(){};
};



class CThreadPool
{
public:
	pthread_mutex_t m_queueLock;
    pthread_cond_t m_queueReady;

	 /*链表结构，线程池中所有等待任务*/
    CThreadWorker *m_queueHead;
    /*是否销毁线程池*/
    int m_shutdown;
    pthread_t *m_threadid;
    /*线程池中允许的活动线程数目*/
    int m_maxThreadNum;
    /*当前等待队列的任务数目*/
    int m_curQueueSize;

public:
	CThreadPool(int maxThreadNum);
	~CThreadPool();
	int poolAddWorker(void* (*process)(void *arg), void *arg);
	//void* threadRoutine();

};





#endif

