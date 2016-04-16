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
        /*如果等待队列为0并且不销毁线程池，则处于阻塞状态; 注意
            pthread_cond_wait是一个原子操作，等待前会解锁，唤醒后会加锁*/
        while (threadPool->m_curQueueSize == 0 && !threadPool->m_shutdown)
        {
            pthread_cond_wait(&threadPool->m_queueReady, &threadPool->m_queueLock);
        }
		
        /*线程池要销毁了*/
        if (threadPool->m_shutdown)
        {
            /*遇到break,continue,return等跳转语句，千万不要忘记先解锁*/
            pthread_mutex_unlock (&threadPool->m_queueLock);
            pthread_exit (NULL);
        }
		
        assert(threadPool->m_curQueueSize != 0);
        assert(threadPool->m_queueHead != NULL);
		
        /*等待队列长度减去1，并取出链表中的头元素*/
        threadPool->m_curQueueSize--;
        CThreadWorker *worker = threadPool->m_queueHead;
        threadPool->m_queueHead = worker->next;
        pthread_mutex_unlock(&threadPool->m_queueLock);
		
        /*调用回调函数，执行任务*/
        (*(worker->process))(worker->arg);
        if (worker)
		{
			delete worker; 
			worker = NULL;
		}
    }
    /*这一句应该是不可达的*/
    pthread_exit (NULL);
	return (void*)NULL;
}
#else
void* CThreadPool::threadRoutine()
{
	for (;;)
    {
        pthread_mutex_lock (&m_queueLock);
        /*如果等待队列为0并且不销毁线程池，则处于阻塞状态; 注意
            pthread_cond_wait是一个原子操作，等待前会解锁，唤醒后会加锁*/
        while (m_curQueueSize == 0 && !m_shutdown)
        {
            pthread_cond_wait(&m_queueReady, &m_queueLock);
        }
		
        /*线程池要销毁了*/
        if (m_shutdown)
        {
            /*遇到break,continue,return等跳转语句，千万不要忘记先解锁*/
            pthread_mutex_unlock (&m_queueLock);
            pthread_exit(NULL);
        }
		
        assert(m_curQueueSize != 0);
        assert(m_queueHead != NULL);
		
        /*等待队列长度减去1，并取出链表中的头元素*/
        m_curQueueSize--;
        CThreadWorker *worker = m_queueHead;
        m_queueHead = worker->next;
        pthread_mutex_unlock(&m_queueLock);
		
        /*调用回调函数，执行任务*/
        (*(worker->process))(worker->arg);
		
        if (worker)
		{
			delete worker; 
			worker = NULL;
		}
    }
    /*这一句应该是不可达的*/
    pthread_exit (NULL);
	return (void*)NULL;
}
#endif



// 构造线程池
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


/*销毁线程池，等待队列中的任务不会再被执行，但是正在运行的线程会一直
  把任务运行完后再退出*/
CThreadPool::~CThreadPool()
{
	if (m_shutdown) return ;
	m_shutdown = 1;
	
    /*唤醒所有等待线程，线程池要销毁了*/
    pthread_cond_broadcast (&m_queueReady);
	
    /*阻塞等待线程退出，否则就成僵尸了*/
    for (int i=0; i<m_maxThreadNum; ++i)
    {
        pthread_join(m_threadid[i], NULL);
    }
    delete [] m_threadid;
	
    /*销毁等待队列*/
    CThreadWorker *head = NULL;
    while (m_queueHead != NULL)
    {
        head = m_queueHead;
        m_queueHead = m_queueHead->next;
        delete (head);
    }
    /*条件变量和互斥量也别忘了销毁*/
    pthread_mutex_destroy(&m_queueLock);
    pthread_cond_destroy(&m_queueReady);	
}


int 
CThreadPool::poolAddWorker(void* (*process)(void *arg), void *arg)
{
	/*构造一个新任务*/
    CThreadWorker *newWorker = new CThreadWorker;
    newWorker->process = process;
    newWorker->arg     = arg;
    newWorker->next    = NULL;

	/*将任务加入到等待队列中*/
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
    /*好了，等待队列中有任务了，唤醒一个等待线程*/
    pthread_cond_signal(&m_queueReady);
    return 0;
}



#ifdef TestPthreadPool
// 测试线程池
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


