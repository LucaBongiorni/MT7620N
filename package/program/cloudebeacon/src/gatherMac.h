#ifndef __GATHER_MAC_H__
#define __GATHER_MAC_H__

#include <pthread.h>
#include "list.h"
#include "MemPool.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#undef  MAC_LEN
#define MAC_LEN       12
#define MAX_MAC_NODE  256
// 用于缓存采集到的mac地址数据
typedef struct _MacInfo
{
    char Mac[MAC_LEN+1];
    time_t TimePos;
    struct list_head list;
} MacNode;
#define MacNodeLen   (sizeof(MacNode))

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */



class MacList
{
public:
    MacList();
    ~MacList();
    int  AddOneMacInfo(const char* mac);
    int  GathAllMacInfo(char** macInfo);
    void DelAllMacListNode();

private:
    struct list_head m_Head;
    pthread_mutex_t m_ListMutex;
    int m_NodeNum;
    MemPool* m_MemPool;
};


void gatherMacTask(void* arg);
void uploadMacTask(void* arg);


#endif /*__GATHER_MAC_H__*/

