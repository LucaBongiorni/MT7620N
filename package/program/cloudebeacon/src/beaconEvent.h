#ifndef __BEACON_EVENT__H__
#define __BEACON_EVENT__H__

// 事件处理器，用于处理各种每隔一段时间做什么事情

#include "List.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct _TimeEventNode TimeEventNode;
struct _TimeEventNode
{
	struct list_head list;  // 节点链表
	unsigned int curTime;   // 下次执行的时间点
	int interTime;          // 间隔时间长度

	// 执行任务函数
	void (*pExcuteTask)(void);
	void (*pSetInterTime)(TimeEventNode* eventNode, int tempTime);
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */




class TimeEventList
{
public:
	TimeEventList();
	~TimeEventList();

private:
	struct list_head m_head;
	pthread_mutex_t m_eventMutex;
};

// 函数功能:
// 函数参数:
void InitTimeEventList(TimeEventList* timeList);
// 函数功能:
// 函数参数:
void UninitTimeEventList(TimeEventList* timeList);

// 函数功能:
// 函数参数:
void DelAllEventNode(TimeEventList* timeList)
{
	struct list_head *p, *temp;
    TimeEventNode *s = NULL;
    if (list_empty(&(timeList->head)))
    {
    	//printf("#################\n");
        return;
    }

	list_for_each_safe(p, temp, &timeList->head)
	{
		s = list_entry(p, TimeEventNode, list); 
		list_del(p);
		if (s) free(s), s = NULL;
	}
	return ;
}

// 函数功能:
// 函数参数:
int AddNodeToEventList(TimeEventList* timeList, int intervalTime, 
	void (*excuteTast)(void),
	void (*setInterTime)(TimeEventNode* eventNode, int tempTime))
{
	if (NULL == timeList || NULL == excuteTast)
	{
		return -1;
	}

	TimeEventNode* newNode = (TimeEventNode*)malloc(sizeof(TimeEventNode));
	if (NULL == newNode)
	{
		return -1;
	}

	newNode->interTime = intervalTime;
	newNode->pExcuteTask = excuteTast;
	newNode->pSetInterTime = setInterTime;
	
	

}

// 函数功能: 初始化所有节点的时间点
// 函数参数: 
// 返 回 值: 




// 函数功能: 轮询事件链表，并执行相关任务
// 函数参数: 
// 返 回 值: 












#endif /*__BEACON_EVENT__H__*/

