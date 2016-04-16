#ifndef THREAD_POOL_H
#define THREAD_POOL_H



/*
 *�̳߳����������к͵ȴ���������һ��CThread_worker
 *�������������������������һ������ṹ
 */
class CThreadWorker
{
public:
    /*�ص���������������ʱ����ô˺�����ע��Ҳ��������������ʽ*/
    void* (*process) (void *arg);
	/*�ص������Ĳ���*/
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

	 /*����ṹ���̳߳������еȴ�����*/
    CThreadWorker *m_queueHead;
    /*�Ƿ������̳߳�*/
    int m_shutdown;
    pthread_t *m_threadid;
    /*�̳߳�������Ļ�߳���Ŀ*/
    int m_maxThreadNum;
    /*��ǰ�ȴ����е�������Ŀ*/
    int m_curQueueSize;

public:
	CThreadPool(int maxThreadNum);
	~CThreadPool();
	int poolAddWorker(void* (*process)(void *arg), void *arg);
	//void* threadRoutine();

};





#endif

