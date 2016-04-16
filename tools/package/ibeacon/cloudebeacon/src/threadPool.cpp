#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>
#include <sys/time.h>

#include "threadPool.h"

#if 1
void* 
threadRoutine(void *arg)
{
	if (NULL == arg) return (void*)NULL;
	
	CThreadPool* threadPool = (CThreadPool*)arg;
	for (;;)
    {
        pthread_mutex_lock (&threadPool->m_queueLock);
        /*����ȴ�����Ϊ0���Ҳ������̳߳أ���������״̬; ע��
            pthread_cond_wait��һ��ԭ�Ӳ������ȴ�ǰ����������Ѻ�����*/
        while (threadPool->m_curQueueSize == 0 && !threadPool->m_shutdown)
        {
            pthread_cond_wait(&threadPool->m_queueReady, &threadPool->m_queueLock);
        }
		
        /*�̳߳�Ҫ������*/
        if (threadPool->m_shutdown)
        {
            /*����break,continue,return����ת��䣬ǧ��Ҫ�����Ƚ���*/
            pthread_mutex_unlock (&threadPool->m_queueLock);
            pthread_exit (NULL);
        }
		
        assert(threadPool->m_curQueueSize != 0);
        assert(threadPool->m_queueHead != NULL);
		
        /*�ȴ����г��ȼ�ȥ1����ȡ�������е�ͷԪ��*/
        threadPool->m_curQueueSize--;
        CThreadWorker *worker = threadPool->m_queueHead;
        threadPool->m_queueHead = worker->next;
        pthread_mutex_unlock(&threadPool->m_queueLock);
		
        /*���ûص�������ִ������*/
        (*(worker->process))(worker->arg);
        if (worker)
		{
			delete worker; 
			worker = NULL;
		}
    }
    /*��һ��Ӧ���ǲ��ɴ��*/
    pthread_exit (NULL);
	return (void*)NULL;
}
#else
void* CThreadPool::threadRoutine()
{
	for (;;)
    {
        pthread_mutex_lock (&m_queueLock);
        /*����ȴ�����Ϊ0���Ҳ������̳߳أ���������״̬; ע��
            pthread_cond_wait��һ��ԭ�Ӳ������ȴ�ǰ����������Ѻ�����*/
        while (m_curQueueSize == 0 && !m_shutdown)
        {
            pthread_cond_wait(&m_queueReady, &m_queueLock);
        }
		
        /*�̳߳�Ҫ������*/
        if (m_shutdown)
        {
            /*����break,continue,return����ת��䣬ǧ��Ҫ�����Ƚ���*/
            pthread_mutex_unlock (&m_queueLock);
            pthread_exit(NULL);
        }
		
        assert(m_curQueueSize != 0);
        assert(m_queueHead != NULL);
		
        /*�ȴ����г��ȼ�ȥ1����ȡ�������е�ͷԪ��*/
        m_curQueueSize--;
        CThreadWorker *worker = m_queueHead;
        m_queueHead = worker->next;
        pthread_mutex_unlock(&m_queueLock);
		
        /*���ûص�������ִ������*/
        (*(worker->process))(worker->arg);
		
        if (worker)
		{
			delete worker; 
			worker = NULL;
		}
    }
    /*��һ��Ӧ���ǲ��ɴ��*/
    pthread_exit (NULL);
	return (void*)NULL;
}
#endif



// �����̳߳�
CThreadPool::CThreadPool(int maxThreadNum)
{
	pthread_mutex_init(&m_queueLock, NULL);
	pthread_cond_init(&m_queueReady, NULL);
	m_queueHead = NULL;
	m_maxThreadNum = maxThreadNum;
	m_curQueueSize = 0;
	m_shutdown = 0;
	m_threadid = new pthread_t[m_maxThreadNum];
	for (int i=0; i<m_maxThreadNum; ++i)
	{
		pthread_create(&m_threadid[i], NULL, threadRoutine, this);
	}
}


/*�����̳߳أ��ȴ������е����񲻻��ٱ�ִ�У������������е��̻߳�һֱ
  ����������������˳�*/
CThreadPool::~CThreadPool()
{
	if (m_shutdown) return ;
	m_shutdown = 1;
	
    /*�������еȴ��̣߳��̳߳�Ҫ������*/
    pthread_cond_broadcast (&m_queueReady);
	
    /*�����ȴ��߳��˳�������ͳɽ�ʬ��*/
    for (int i=0; i<m_maxThreadNum; ++i)
    {
        pthread_join(m_threadid[i], NULL);
    }
    delete [] m_threadid;
	
    /*���ٵȴ�����*/
    CThreadWorker *head = NULL;
    while (m_queueHead != NULL)
    {
        head = m_queueHead;
        m_queueHead = m_queueHead->next;
        delete (head);
    }
    /*���������ͻ�����Ҳ����������*/
    pthread_mutex_destroy(&m_queueLock);
    pthread_cond_destroy(&m_queueReady);	
}


int 
CThreadPool::poolAddWorker(void* (*process)(void *arg), void *arg)
{
	/*����һ��������*/
    CThreadWorker *newWorker = new CThreadWorker;
    newWorker->process = process;
    newWorker->arg     = arg;
    newWorker->next    = NULL;

	/*��������뵽�ȴ�������*/
    pthread_mutex_lock (&m_queueLock);
    CThreadWorker *member = m_queueHead;
    if (member != NULL)
    {
        while (member->next != NULL)
        {
            member = member->next;
        }
        member->next = newWorker;
    }
    else
    {
        m_queueHead = newWorker;
    }
	
    assert(m_queueHead != NULL);
    ++m_curQueueSize;
    pthread_mutex_unlock(&m_queueLock);
    /*���ˣ��ȴ��������������ˣ�����һ���ȴ��߳�*/
    pthread_cond_signal(&m_queueReady);
    return 0;
}



#ifdef TestPthreadPool
// �����̳߳�
void *
doSomeThings(void* arg)
{
	static int num = 0;
	printf("Add ThreadNum=%d\n", num++);
	while(1)
		usleep(500*1000);
	printf("Exit Thread(%d)\n", num--);
	return (void*)NULL;
}

int 
main()
{
	CThreadPool* pThreadPool = new CThreadPool(50);
	for (int i=0; i<10; ++i)
	{
		pThreadPool->poolAddWorker(doSomeThings, NULL);
	}
	while(1)
	{
		pThreadPool->poolAddWorker(doSomeThings, NULL);
		usleep(2000*1000);
	}
	
	while(1)sleep(10);

	delete pThreadPool;
	return 0;
}
#endif 


