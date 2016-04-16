#ifndef __BEACON_EVENT__H__
#define __BEACON_EVENT__H__

// �¼������������ڴ������ÿ��һ��ʱ����ʲô����

#include "List.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct _TimeEventNode TimeEventNode;
struct _TimeEventNode
{
	struct list_head list;  // �ڵ�����
	unsigned int curTime;   // �´�ִ�е�ʱ���
	int interTime;          // ���ʱ�䳤��

	// ִ��������
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

// ��������:
// ��������:
void InitTimeEventList(TimeEventList* timeList);
// ��������:
// ��������:
void UninitTimeEventList(TimeEventList* timeList);

// ��������:
// ��������:
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

// ��������:
// ��������:
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

// ��������: ��ʼ�����нڵ��ʱ���
// ��������: 
// �� �� ֵ: 




// ��������: ��ѯ�¼�������ִ���������
// ��������: 
// �� �� ֵ: 












#endif /*__BEACON_EVENT__H__*/

