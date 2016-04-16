#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/*
*�̳߳����������к͵ȴ���������һ��CThread_worker
*�������������������������һ������ṹ
*/
typedef struct worker
{
    /*�ص���������������ʱ����ô˺�����ע��Ҳ��������������ʽ*/
    void *(*process) (void *arg);
    void *arg;/*�ص������Ĳ���*/
    struct worker *next;	
} CThread_worker;	
	
/*�̳߳ؽṹ*/
typedef struct
{
     pthread_mutex_t queue_lock;
     pthread_cond_t queue_ready;
    /*����ṹ���̳߳������еȴ�����*/
     CThread_worker *queue_head;

    /*�Ƿ������̳߳�*/
    int shutdown;
    pthread_t *threadid;
	
    /*�̳߳�������Ļ�߳���Ŀ*/
    int max_thread_num;
	
    /*��ǰ�ȴ����е�������Ŀ*/
    int cur_queue_size;	
} CThread_pool;	


int pool_add_worker (void *(*process) (void *arg), void *arg);
void *thread_routine (void *arg);
void pool_init (int max_thread_num);
int pool_destroy ();


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif
