#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


#include "listStruct.h"
#include "StrOperate.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


// 配置发送给串口信息的缓存个数
static int net2serNodeNum = 20;
static void *(*ListMalloc)(size_t size) = malloc;
static void (*ListFree)(void* ptr) = free;


void setNet2SerNodeNum(int num)
{
	net2serNodeNum = num;
}

void 
InitSeqList(ListManage* seqHead)
{
	INIT_LIST_HEAD(&(seqHead->head));
    pthread_mutex_init(&(seqHead->ListMutex), NULL);
	return ;
}
void 
UninitSeqList(ListManage* seqHead)
{
    DelAllSeqNode(seqHead);
    pthread_mutex_destroy(&seqHead->ListMutex);
	return ;
}
void 
DelAllSeqNode(ListManage* seqHead)
{
	struct list_head *p;
    struct list_head *temp;
    SeqListNode *s;
    if (list_empty(&(seqHead->head)))
    {
    	//printf("#################\n");
        return;
    }
    for (p=(&seqHead->head)->next;  p != &seqHead->head;)
    {
        temp = p;
        p = p->next;
        s = list_entry(temp, SeqListNode, list);  
        if (NULL != s->Info.pBuff)
        {
            if (s->Info.pBuff)
            {
                ListFree(s->Info.pBuff);
                s->Info.pBuff = NULL;
            }
        }
        list_del(temp);
        if (s)
        {
            ListFree(s);
            s = NULL;
        }
    }
    return ;
}

bool 
CheckSeqListNodeIsExist(ListManage* seqHead, u_int8 seq)
{
	struct list_head *p = NULL;
	SeqListNode *temp = NULL;

	// 判空
	if (list_empty(&(seqHead->head)))
	{
		return false;
	}

	// 遍历
	list_for_each(p, &(seqHead->head))
	{
		temp = list_entry(p, SeqListNode, list);
	    if (temp->Info.seqNum == seq)
			return true;
	}
	return false;
}

int 
AddNodeToSeqListTail(const char* data, int dataLen, char seq, ListManage* seqHead)
{
	// 参数判断
    if ( NULL == data || dataLen <= 0 )
    {
        return -1;
    }

	// 检测该seq是否已经存在
	if ( true == CheckSeqListNodeIsExist(seqHead, seq) )
	{
		return -1;
	}

	// 创建新的节点
    SeqListNode *_new = NULL;
    _new = (SeqListNode *)ListMalloc(sizeof(SeqListNode));
    if (NULL == _new)
    {
        write_error_log("Malloc Memory failed.");
        return -1;
    }
	
    _new->Info.pBuff = NULL;
    _new->Info.pBuff = (char*)ListMalloc(dataLen);
    if (NULL == _new->Info.pBuff)
    {
        if (_new)
        {
            ListFree(_new);
            _new = NULL;
        }
        return -1;
    }

    memcpy(_new->Info.pBuff, data, dataLen);
    _new->Info.BuffLen = dataLen;
	_new->Info.seqNum  = seq;
    list_add_tail(&(_new->list), &(seqHead->head));
    return 0;
}
int 
CountSeqListBuffLen(ListManage* seqHead, int* NodeNum)
{
	int BuffLen = 0;
	struct list_head *p = NULL;
	SeqListNode *temp = NULL;
	*NodeNum = 0;

	// 判空
	if (list_empty(&(seqHead->head)))
    {
        return -1;
    }

	// 遍历
	list_for_each(p, &(seqHead->head))
	{
		temp = list_entry(p, SeqListNode, list);
        BuffLen += temp->Info.BuffLen;
		*NodeNum += 1;
	}
	return BuffLen;
}
//函数功能: 根据 pkgTotal 判断链表是否完整
bool 
CheckSeqListNode(ListManage* seqHead, u_int8 pkgTotal)
{
	struct list_head *p = NULL;
	SeqListNode *temp = NULL;
	int i;
	char flag[pkgTotal];

	// 判空
	if (list_empty(&(seqHead->head)))
    {
        return false;
    }

	for (i=1; i<pkgTotal; ++i)
	{
		flag[i] = 0;
	}

	// 遍历
	list_for_each(p, &(seqHead->head))
	{
		temp = list_entry(p, SeqListNode, list);
		flag[temp->Info.seqNum-1] = 1;
	}

	for (i=0; i<pkgTotal; ++i)
	{
		if ( flag[i] == 0 )
		{
			return false;
		}
	}
	return true;
}
// 根据seq提取字符信息，seq的格式为1、2、3、4模式
int 
PickUpBuffFromSeqList(ListManage* seqHead, char** ppBuff)
{
	struct list_head *pp = NULL;
	struct list_head *p  = NULL;
	SeqListNode *temp    = NULL;
	int BuffLen;
	int NodeNum, i;
	//int leftVal = 0;
	char *ptemp = NULL;

	if ( (BuffLen = CountSeqListBuffLen(seqHead, &NodeNum)) == 0 )
	{
		write_error_log("Count SeqList Bufflen failed.");
		return -1;
	}
	//printf("BuffLen=%d, NodeNum=%d\n", BuffLen, NodeNum);
	
	*ppBuff = (char*)ListMalloc(BuffLen);
	if (NULL == *ppBuff)
	{
		write_error_log("Malloc Memory Failed.");
		return -1;
	}
	ptemp = *ppBuff;

	for (i=1; i<=NodeNum; ++i)
	{
		for (pp=(&seqHead->head)->next; pp != &seqHead->head;)
		{
			p  = pp;
			pp = pp->next;
			temp = list_entry(p, SeqListNode, list);
	  		if (temp->Info.seqNum == i)
	  		{
				if (NULL != temp->Info.pBuff)
		        {
		        	memcpy(ptemp, temp->Info.pBuff, temp->Info.BuffLen);
		            if (temp->Info.pBuff)
		            {
		                ListFree(temp->Info.pBuff);
		                temp->Info.pBuff = NULL;
		            }
					ptemp += temp->Info.BuffLen;
		        }
				
		        list_del(p);
		        if (temp)
		        {
		            ListFree(temp);
					temp = NULL;
		        }
				break;
			}
		}
	}
	return BuffLen;
}

void 
printSeqList(ListManage* seqHead)
{
	int i;
	struct list_head *p = NULL;
	SeqListNode *temp = NULL;

	// 判空
	if (list_empty(&(seqHead->head)))
    {
        return ;
    }

	// 遍历
	list_for_each(p, &(seqHead->head))
	{
		temp = list_entry(p, SeqListNode, list);
		for (i=0; i<temp->Info.BuffLen; ++i)
		{
			printf("%c", temp->Info.pBuff[i]);
		}
		printf("\n");
	}
	return;
}





///////////////////////////////////////////////////////////////////////
void 
InitTypeIDList(ListManage* IDHead)
{
	INIT_LIST_HEAD(&(IDHead->head));
    pthread_mutex_init(&(IDHead->ListMutex), NULL);
	return ;
}
void 
UninitTypeIDList(ListManage* IDHead)
{
    DelAllTypeIdNode(IDHead);
    pthread_mutex_destroy(&IDHead->ListMutex);
}

// 判断该id是否在队列中
bool 
TestNodeInList(int typeID, ListManage* IDHead)
{
	if (NULL == IDHead)
		return false;
	
	struct list_head *p = NULL;
	IDListNode *temp = NULL;

	// 判空
	if (list_empty(&(IDHead->head)))
    {
        return false;
    }

	// 遍历
	list_for_each(p, &(IDHead->head))
	{
		temp = list_entry(p, IDListNode, list);
		if (temp->Info.typeID == typeID)
		{
			return true;
		}
	}
	return false;
}

// 添加一个节点到ID链表中
int 
AddNodeToIDList(int typeID, ListManage* IDHead, 
			const char* buff, int buffLen, char seq)
{
	if (NULL == IDHead)
		return -1;

	struct list_head *p = NULL;
	IDListNode *temp = NULL;
	IDListNode *_new = NULL;

	list_for_each(p, &(IDHead->head))
	{
		temp = list_entry(p, IDListNode, list);
		if (temp->Info.typeID == typeID)
		{
			// 在链表内
			AddNodeToSeqListTail(buff, buffLen, seq, &temp->Info.SeqListHead);
			return 0;
		}
	}

	// 不在链表内
	_new = (IDListNode*)ListMalloc(sizeof(IDListNode));
	if (NULL == _new)
	{
	    write_error_log("Malloc Memory failed.");
	    return -1;
	}
	_new->Info.typeID = typeID;
	InitSeqList(&_new->Info.SeqListHead);
	AddNodeToSeqListTail(buff, buffLen, seq, &_new->Info.SeqListHead);
    list_add_tail(&(_new->list), &(IDHead->head));
    return 0;
}

void 
DelOneNodeByTypeID(ListManage* IDHead, int typeID)
{
	if (NULL == IDHead) return ;
	
	if ( list_empty(&IDHead->head) )return ;
  
	struct list_head *pID = NULL;
	struct list_head *p = NULL;
	IDListNode *temp = NULL;

	for (pID = IDHead->head.next; pID != &IDHead->head;)
	{
		p   = pID;
		pID = pID->next;
		
		temp = list_entry(p, IDListNode, list);
		if (temp->Info.typeID == typeID)
		{
			DelAllSeqNode(&temp->Info.SeqListHead);
			list_del(p);
			if (temp)
			{
				ListFree(temp), temp = NULL;
			}
		}
	}
	return ;
}

void 
DelAllTypeIdNode(ListManage* IDHead)
{
	if (NULL == IDHead) return ;
	
	if ( list_empty(&IDHead->head) )return ;
  
	struct list_head *pID = NULL;
	struct list_head *p = NULL;
	IDListNode *temp = NULL;

	for (pID = IDHead->head.next; pID != &IDHead->head;)
	{
		p = pID;
		pID = pID->next;
		
		temp = list_entry(p, IDListNode, list);
		DelAllSeqNode(&temp->Info.SeqListHead);

		list_del(p);
		if (temp)
		{
			ListFree(temp), temp = NULL;
		}
	}
	return ;
}


int 
PickUpBuffFromIDList(ListManage* IDHead, int typeID, char** buff)
{
	if (NULL == IDHead)
	{
		write_error_log("Paramter is Error.");
		return -1;
	}
	if ( list_empty(&IDHead->head) )
	{
		write_normal_log("List is empty.");
		return -1;
	}

	struct list_head *p = NULL;
	IDListNode *temp = NULL;
	int nRet = 0;

	list_for_each(p, &IDHead->head)
	{
		temp = list_entry(p, IDListNode, list);
		if (temp->Info.typeID == typeID)
		{
			if ( -1 == (nRet=PickUpBuffFromSeqList(&temp->Info.SeqListHead, buff)) )
			{
				write_normal_log("The seq List is empty. typeID=%d", typeID);
				return 0;
			}
			else
			{
				return nRet;
			}
		}
	}
	return 0;
}

bool 
CheckIDNodeIsComplete(ListManage* IDHead, int typeID, u_int8 pkgTotal)
{
	if (NULL == IDHead)
	{
		write_error_log("Paramter is Error.");
		return false;
	}
	if ( list_empty(&IDHead->head) )
	{
		write_normal_log("List is empty.");
		return false;
	}

	struct list_head *p = NULL;
	IDListNode *temp = NULL;
	//int nRet = 0;

	list_for_each(p, &IDHead->head)
	{
		temp = list_entry(p, IDListNode, list);
		if (temp->Info.typeID == typeID)
		{
			return CheckSeqListNode(&temp->Info.SeqListHead, pkgTotal);
		}
	}
	return false;
}


void 
PrintAllTypeIdContent(ListManage* IDHead)
{
	if (NULL == IDHead) return ;
	if ( list_empty(&IDHead->head) )return ;
	
	struct list_head *p = NULL;
	IDListNode *temp = NULL;

	list_for_each(p, &IDHead->head)
	{
		temp = list_entry(p, IDListNode, list);
		printf("\n---------------typeID=%d-------------\n", temp->Info.typeID);
		printSeqList(&temp->Info.SeqListHead);
		printf("---------------typeID=%d-------------\n", temp->Info.typeID);
	}
}










////////////////////////////////////////////////////////////////////////
// 函数功能: 初始化 net2SerHead 链表 
// 函数参数: net2SerHead 链表头指针
void InitNet2SerList(ListManage* net2SerHead)
{
	INIT_LIST_HEAD(&(net2SerHead->head));
    pthread_mutex_init(&(net2SerHead->ListMutex), NULL);
	net2SerHead->NodeNum = 0;
	return ;
}

// 函数功能: 销毁 net2SerHead 链表 
// 函数参数: net2SerHead 链表头指针
void UninitNet2SerList(ListManage* net2SerHead)
{
	DelAllNet2SerListNode(net2SerHead);
    pthread_mutex_destroy(&net2SerHead->ListMutex);
	return ;
}

// 函数功能: 获取链表有多少个节点
// 函数参数: net2SerHead 链表头指针
int 
GetNet2SerListNodeNum(ListManage* net2SerHead)
{
	int temp = 0;
	pthread_mutex_lock(&net2SerHead->ListMutex);
	temp = net2SerHead->NodeNum;
	pthread_mutex_unlock(&net2SerHead->ListMutex);
	return temp;
}


// 函数功能: 往链表尾部添加节点
// 函数参数: net2SerHead 链表头指针
//           data，添加的数据
//           dataLen, 数据长度
//           typeID，数据类型
// 返 回 值: 成功返回0， 失败返回-1；
int 
AddNodeToNet2SerList(ListManage* net2SerHead, 
	const char* data, int dataLen, u_int16 typeID, int phoneHandle, bool netFlag)
{
	if (!net2SerHead || !data)
	{
		return -1;
	}
	// 超出最大节点数，删掉最先的节点
	if (net2serNodeNum < GetNet2SerListNodeNum(net2SerHead))
	{
		if (-1 == DelFirstNodeFromNet2SerList(net2SerHead) )
			return -1;
	}
	//static int iNode = 0;
	// 创建一个新的节点
	Net2SerNode *newNode = (Net2SerNode*)ListMalloc(sizeof(Net2SerNode));
	if (NULL == newNode)
	{
		return -1;
	}
	//printf("new a node, nodeNum=%d\n", ++iNode);
	
	newNode->Info.pBuff = (char*)ListMalloc(dataLen);
	if (NULL == newNode->Info.pBuff)
	{
		ListFree(newNode); 
		newNode = NULL;
		return -1;
	}
	// 填充节点内容
	memcpy(newNode->Info.pBuff, data, dataLen);
	newNode->Info.BuffLen = dataLen;
	newNode->Info.typeID  = typeID;
	if (true == netFlag)
	{
		newNode->Info.phoneFlag = 0;
		newNode->Info.netFlag = 1;
	}
	else
	{
		newNode->Info.phoneFlag = phoneHandle;
		newNode->Info.netFlag = 0;
	}

	// 挂载节点到链表
	pthread_mutex_lock(&net2SerHead->ListMutex);
	list_add_tail(&(newNode->list), &(net2SerHead->head));
	net2SerHead->NodeNum++;
	pthread_mutex_unlock(&net2SerHead->ListMutex);
    return 0;
}

// 函数功能: 删除链表第一个节点
// 函数参数: net2SerHead 链表头指针
// 返 回 值: 成功返回0， 失败返回-1；
int 
DelFirstNodeFromNet2SerList(ListManage* net2SerHead)
{
	if (NULL == net2SerHead)
	{
		return -1;
	}
	struct list_head *pHead;
	Net2SerNode* pNode;

	// 卸载节点
	pthread_mutex_lock(&net2SerHead->ListMutex);
	pHead = net2SerHead->head.next;
	list_del(pHead);
	net2SerHead->NodeNum--;
	pthread_mutex_unlock(&net2SerHead->ListMutex);

	// 删除节点和节点内容
	pNode = list_entry(pHead, Net2SerNode, list);
	if (pNode->Info.pBuff)
	{
		ListFree(pNode->Info.pBuff);
		pNode->Info.pBuff = NULL;
	}
	if (pNode)
	{
		ListFree(pNode);
		pNode = NULL;
	}
	return 0;
}

// 函数功能: 删除链表所有节点
// 函数参数: net2SerHead 链表头指针
// 返 回 值: 成功返回0， 失败返回-1；
int 
DelAllNet2SerListNode(ListManage* net2SerHead)
{
	struct list_head *pHead;
	struct list_head *pTemp;
	Net2SerNode *pNode;

	pthread_mutex_lock(&net2SerHead->ListMutex);
    if (list_empty(&(net2SerHead->head)))
    {
    	pthread_mutex_unlock(&net2SerHead->ListMutex);
        return -1;
    }
    for (pHead=(&net2SerHead->head)->next;  pHead != &net2SerHead->head;)
    {
        pTemp = pHead;
        pHead = pHead->next;
        pNode = list_entry(pTemp, Net2SerNode, list);
        if (NULL != pNode->Info.pBuff)
        {
            if (pNode->Info.pBuff)
            {
                ListFree(pNode->Info.pBuff);
                pNode->Info.pBuff = NULL;
            }
        }
        list_del(pTemp);
		//printf("----------delete---------%d\n", net2SerHead->NodeNum);
		ListFree(pNode), pNode = NULL;
        net2SerHead->NodeNum--;
    }
	pthread_mutex_unlock(&net2SerHead->ListMutex);
    return 0;
}

// 函数功能: 根据手机标志位，获取缓冲区数据
// 函数参数: ser2NetHead 链表头指针
//           data，输出参数；返回的数据
//           dataLen，输入参数；data缓冲区大小
//           typeID，输出参数，数据类型
//           phoneFlag, 输入参数，手机标志
//           MemSize，缓冲区不够，剩余的数据大小
// 返 回 值: 返回-1失败，返回0，找不到，成功返回获取到的数据长度
int 
GetNodeInfoFromListByPhoneFlag(ListManage* ser2NetHead, char* data, int dataLen, 
		u_int16* typeID, int phoneFlag, int* MemSize)
{
	if (!ser2NetHead || !data || !dataLen || !typeID)
	{
		return -1;
	}
	struct list_head* pTemp = NULL;
	Net2SerNode* pNode;
	int length = 0, i, j;
	//char* buff = NULL;
	//int buffLen;

	// 判断是否存在数据
	pthread_mutex_lock(&ser2NetHead->ListMutex);
	if (list_empty(&(ser2NetHead->head)))
    {
        pthread_mutex_unlock(&ser2NetHead->ListMutex);
		return -1;
    }
	
	list_for_each(pTemp, &ser2NetHead->head)
	{
		pNode = list_entry(pTemp, Net2SerNode, list);
		if (pNode->Info.phoneFlag == phoneFlag)
		{
			if (dataLen < pNode->Info.BuffLen)
			{
				memcpy(data, pNode->Info.pBuff, dataLen);
				for (i=dataLen, j=0; i<pNode->Info.BuffLen; ++i, ++j)
				{
					pNode->Info.pBuff[j] = pNode->Info.pBuff[i];
				}
				pNode->Info.BuffLen = j;
				pthread_mutex_unlock(&ser2NetHead->ListMutex);
				
				*MemSize = j;
				return dataLen;
			}
			else
			{
				memcpy(data, pNode->Info.pBuff, pNode->Info.BuffLen);
				length = pNode->Info.BuffLen;
				// 删除节点
				list_del(pTemp);
				ser2NetHead->NodeNum--;
				pthread_mutex_unlock(&ser2NetHead->ListMutex);
				
				// 删除第一个节点
				if (pNode->Info.pBuff)
				{
					ListFree(pNode->Info.pBuff);
					pNode->Info.pBuff= NULL;
				}
				ListFree(pNode), pNode = NULL;
				*MemSize = 0;
				return length;	
			}
		}
	}

	pthread_mutex_unlock(&ser2NetHead->ListMutex);
	return length;
}

// 函数功能: 从链表头部获取一个节点
// 函数参数: net2SerHead 链表头指针
//           data，输出参数，获取到的节点数据
//           dataLen, 输入参数，缓冲区大小
//           typeID，输出参数，数据类型
//           phoneFlag 输出参数，手机标志
//           netFlag 输出参数，1是网络，0是手机
//           MemSize 输出参数，1表示还需要多大的缓冲区
// 返 回 值: 成功返回获取数据的长度，失败返回-1；
int 
GetNodeInfoFromNet2SerList(ListManage* net2SerHead, char* data, int dataLen, 
		u_int16* typeID, u_int32 *phoneFlag, char* netFlag, int* MemSize)
{
	if (!net2SerHead || !data || !dataLen || !typeID)
	{
		return -1;
	}

	int length, i, j;
	struct list_head *pHead;
	Net2SerNode* pNode;
	
	// 判断是否存在数据
	pthread_mutex_lock(&net2SerHead->ListMutex);
	if (list_empty(&(net2SerHead->head)))
    {
        pthread_mutex_unlock(&net2SerHead->ListMutex);
		return -1;
    }
	
	// 获取第一个节点的数据
	pHead = net2SerHead->head.next;
	pNode = list_entry(pHead, Net2SerNode, list);
	*typeID  = pNode->Info.typeID;
	*phoneFlag = pNode->Info.phoneFlag;
	*netFlag = pNode->Info.netFlag;

	// 如果缓冲区大小小于数据大小，拷贝缓冲区大小的数据，并重新整理数据
	if (dataLen >= pNode->Info.BuffLen)
	{
		memcpy(data, pNode->Info.pBuff, pNode->Info.BuffLen);
		length = pNode->Info.BuffLen;
		
		// 卸载第一个节点
		list_del(pHead);
		net2SerHead->NodeNum--;
		pthread_mutex_unlock(&net2SerHead->ListMutex);
		
		// 删除第一个节点
		if (pNode->Info.pBuff)
		{
			ListFree(pNode->Info.pBuff);
			pNode->Info.pBuff= NULL;
		}
		
		ListFree(pNode), pNode = NULL;
		*MemSize = 0;
		return length;
	}
	else
	{
		memcpy(data, pNode->Info.pBuff, dataLen);
		length = dataLen;
		for (i=dataLen, j=0; i<pNode->Info.BuffLen; ++i, ++j)
		{
			pNode->Info.pBuff[j] = pNode->Info.pBuff[i];
		}
		pNode->Info.BuffLen = j;
		pthread_mutex_unlock(&net2SerHead->ListMutex);

		*MemSize = j;
		return length;
	}
}


#ifdef TestList
// 测试 net2ser 链表
int main()
{
	ListManage net2SerHead;
	InitNet2SerList(&net2SerHead);
	const char* data = "1234567890abcdefghijklmnopqrstuvwxyz";
	int dataLen = strlen(data);
	u_int16 typeID = 0;
	u_int32 count = 0;
	u_int32 phoneFlag; 
	char netFlag;
	int MemSize, nRet;
	char buff[1024] = {0};
	int buffLen = 20;
	u_int16 tempID;
	write_normal_log("data=%s", data);
	while(1)
	{
		typeID++;
		count++;
		
		if ( -1 == AddNodeToNet2SerList(&net2SerHead, data, dataLen, typeID%100, 0, true) )
		{
			write_error_log("Add Node To Net2Ser List Failed.");
			return 0;
		}
		if (count == 100)
		{
			DelAllNet2SerListNode(&net2SerHead);
			typeID = 0;
			count = 0;
		}
		if (count > 20)
		{
			nRet = GetNodeInfoFromNet2SerList(&net2SerHead, buff, buffLen, 
				&tempID, &phoneFlag, &netFlag, &MemSize);
			buff[nRet] = 0;
			printf("buff=%s\n", buff);
			printf("nRet=%d, tempID=%d, count=%d, MemSize=%d\n", nRet, tempID, count, MemSize);
			if (MemSize>0)
			{
				nRet = GetNodeInfoFromNet2SerList(&net2SerHead, buff, MemSize, 
						&tempID, &phoneFlag, &netFlag, &MemSize);
				buff[nRet] = 0;
				printf("buff=%s\n", buff);
			}
		}
		sleep(1);
	}
	UninitNet2SerList(&net2SerHead);
	return 0;
}
#endif

#if 0
// 测试 ID 链表
int main()
{
	ListManage IDHead;
	char buff[128] = {0};
	char* pStr = NULL;

	InitTypeIDList(&IDHead);

	memset(buff, 'a', 8);
	AddNodeToIDList(243, &IDHead, buff, 8, 2);
	memset(buff, 'b', 7);
	AddNodeToIDList(243, &IDHead, buff, 7, 4);
	memset(buff, 'c', 6);
	AddNodeToIDList(243, &IDHead, buff, 6, 1);
	memset(buff, 'd', 5);
	AddNodeToIDList(243, &IDHead, buff, 5, 5);
	memset(buff, 'e', 4);
	AddNodeToIDList(243, &IDHead, buff, 4, 3);

	memset(buff, '1', 5);
	AddNodeToIDList(242, &IDHead, buff, 5, 4);
	memset(buff, '2', 4);
	AddNodeToIDList(242, &IDHead, buff, 4, 2);
	memset(buff, '3', 3);
	AddNodeToIDList(242, &IDHead, buff, 3, 1);
	memset(buff, '4', 2);
	AddNodeToIDList(242, &IDHead, buff, 2, 5);
	memset(buff, '5', 1);
	AddNodeToIDList(242, &IDHead, buff, 1, 3);

	memset(buff, 'A', 5);
	AddNodeToIDList(244, &IDHead, buff, 5, 1);
	memset(buff, 'B', 4);
	AddNodeToIDList(244, &IDHead, buff, 4, 2);
	memset(buff, 'C', 3);
	AddNodeToIDList(244, &IDHead, buff, 3, 3);
	memset(buff, 'D', 4);
	AddNodeToIDList(244, &IDHead, buff, 4, 4);
	memset(buff, 'E', 5);
	AddNodeToIDList(244, &IDHead, buff, 5, 5);

	PrintAllTypeIdContent(&IDHead);
	DelOneNodeByTypeID(&IDHead, 244);
	PrintAllTypeIdContent(&IDHead);

	int len = PickUpBuffFromIDList(&IDHead, 243, &pStr);
	for (int i=0; i<len; ++i)
	{
		printf("%c", pStr[i]);
	}
	printf("\n");

	DelOneNodeByTypeID(&IDHead, 243);

	UninitTypeIDList(&IDHead);
	if (pStr) delete []pStr;
	return 0;
}
#endif

#if 0
// 测试 seq 链表
int main()
{
	char buff[128] = {0};
	char* pStr = NULL;
	int len;
	ListManage seqHead;
	InitSeqList(&seqHead);

	memset(buff, 'd', 7);
	AddNodeToSeqListTail(buff, 7, 4, &seqHead);

	memset(buff, 'b', 6);
	AddNodeToSeqListTail(buff, 6, 2, &seqHead);

	memset(buff, 'a', 5);
	AddNodeToSeqListTail(buff, 5, 1, &seqHead);

	memset(buff, 'c', 4);
	AddNodeToSeqListTail(buff, 4, 3, &seqHead);

	printSeqList(&seqHead);

	printf("---------------------------------------\n");
	len = PickUpBuffFromSeqList(&seqHead, &pStr);
	for(int i=0; i<len; ++i)
	{
		printf("%c", pStr[i]);
	}
	printf("\n");
	
	UninitSeqList(&seqHead);
	if (pStr) delete pStr;
	return 0;
}
#endif








#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

