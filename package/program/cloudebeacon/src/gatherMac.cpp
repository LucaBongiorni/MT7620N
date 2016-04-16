#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


#include "gatherMac.h"
#include "list.h"
#include "cbProjMain.h"
#include "procotol.h"
#include "cJSON.h"
#include "pthreadCom.h"
#include "thread_pool.h"
#include "defCom.h"





MacList::MacList()
{
	INIT_LIST_HEAD(&m_Head);
    pthread_mutex_init(&m_ListMutex, NULL);
	m_NodeNum = 0;
	m_MemPool = new MemPool(MacNodeLen, MAX_MAC_NODE);
	return ;
}


MacList::~MacList()
{
	DelAllMacListNode();
	delete m_MemPool, m_MemPool = NULL;
    pthread_mutex_destroy(&m_ListMutex);
}



int 
MacList::AddOneMacInfo(const char* mac)
{
	if (NULL == mac)
	{
		return -1;
	}
	struct list_head *pTempList, *pList; 
	MacNode *tempNode;
	bool delFlag = false;
	bool isNotAdd = true;

	// ����һ�Σ��������������ͬ�Ľڵ㣬�޸ĵڶ����ڵ��ʱ��
	pthread_mutex_lock(&m_ListMutex);
	if (m_NodeNum == 0)
	{
		isNotAdd = false;
	}
	else
	{
		list_for_each_safe(pList, pTempList, &m_Head)
		{
			tempNode = list_entry(pList, MacNode, list);
			if (strncmp(tempNode->Mac, mac, MAC_LEN) == 0)
			{
				if (false == delFlag)
				{
					delFlag = true;
				}
				else
				{
					isNotAdd = true;
					tempNode->TimePos = time(0);
					break;
				}
			}
		}
		if (pList == &m_Head) isNotAdd = false;
	}
	// write_log("mac=%s", mac);
	// �Ƿ�Ҫ���ص�β��
	if (isNotAdd == false)
	{
		// ����һ���ڵ�
		MacNode* newNode = (MacNode*)m_MemPool->Alloc(sizeof(MacNode));
		if (NULL == newNode)
		{
			Debug(I_ERROR, "Malloc memory failed.");
			return -1;
		}
		strncpy(newNode->Mac, mac, MAC_LEN+1);
		newNode->TimePos = time(0);
		list_add_tail(&newNode->list, &m_Head);
		++m_NodeNum;
		//write_log("-----------Add one node--------MacList->NodeNum=%d", m_NodeNum);
	}
	pthread_mutex_unlock(&m_ListMutex);
	return 0;
}

int 
MacList::GathAllMacInfo(char** macInfo)
{
	//int i;
	cJSON *root, *mac;
	struct list_head *pTempList, *pList; 
	MacNode *tempNode; 

	pthread_mutex_lock(&m_ListMutex);
	if (m_NodeNum == 0)
	{
		pthread_mutex_unlock(&m_ListMutex);
		return -1;
	}
	//write_log("---------------cccc------------------MacNum=%d", m_NodeNum);
	root = cJSON_CreateArray();
	list_for_each_safe(pList, pTempList, &m_Head)
	{
		tempNode = list_entry(pList, MacNode, list);
		cJSON_AddItemToArray(root, mac = cJSON_CreateObject());
		cJSON_AddStringToObject(mac, "MacAddr", tempNode->Mac);
		cJSON_AddNumberToObject(mac, "TimeInterval", tempNode->TimePos);

		list_del(&tempNode->list);
		--m_NodeNum;
		m_MemPool->Free(tempNode), tempNode = NULL;
	}
	pthread_mutex_unlock(&m_ListMutex);

	*macInfo = cJSON_Print(root);	
	cJSON_Delete(root);
	return strlen(*macInfo);
}

void MacList::DelAllMacListNode()
{	
	struct list_head *pTempList, *pList; 
	MacNode *tempNode;
	list_for_each_safe(pList, pTempList, &m_Head)
	{
		tempNode = list_entry(pList, MacNode, list);
		list_del(&tempNode->list);
		--m_NodeNum;
		m_MemPool->Free(tempNode), tempNode = NULL;
	}
}



//////////////////////////////////////////////////////////////////////
// ��������: ��ͻ���������ķ���������mac����
// ��������: buff��������������͵�json����
//           buffLen��������������͵�json���ݳ���
// �� �� ֵ: 0���ɹ���-1��û�в���ͻ���������-2�����ӷ�����ʧ��
int PostDataToCustomMacServer(const char* buff, int buffLen)
{
	IbeaconConfig *conf = GetConfigPosition();

	char* httpHead = (char*)malloc(buffLen+POST_HEAD_LEN);
	int postDataLen;
	// ��װ post ͷ
	postDataLen = snprintf(httpHead, buffLen+POST_HEAD_LEN, 
					"POST %s HTTP/1.1\r\n"
					"HOST: %s:%d\r\n"
					"Content-Type: application/json;charset=utf-8\r\n"
					"Content-Length: %d\r\n\r\n"
					"%s",
					conf->getMacSerUrl(), 
					conf->getMacSerHost(), 
					conf->getMacSerPort(), 
					buffLen, buff);
	//write_log("%s", httpHead);

	// ��������
	CSocketTCP *sockConnect = new CSocketTCP(conf->getMacSerHost(), conf->getMacSerPort());
	int sockfd = sockConnect->Connect();
	if (-1 == sockfd)
	{
		delete sockConnect, sockConnect = NULL;
		free(httpHead), httpHead = NULL;
		return -2;
	}
	
	// ��������
	Write(sockfd, httpHead, postDataLen, 3000);
	Close(sockfd);
	
	delete sockConnect, sockConnect = NULL;
	free(httpHead), httpHead = NULL;
	return 0;
}



static MacList g_macList;

/*
static pthread_rwlock_t enterPthreadMutex = PTHREAD_RWLOCK_INITIALIZER;
static bool enterPthread = false;
static inline void setEnterPthreadVal(bool val)
{
	pthread_rwlock_wrlock(&enterPthreadMutex);
	enterPthread = val;
	pthread_rwlock_unlock(&enterPthreadMutex);
}
static inline bool getEnterPthreadVal()
{
	bool temp;
	pthread_rwlock_rdlock(&enterPthreadMutex);
	temp = enterPthread;
	pthread_rwlock_unlock(&enterPthreadMutex);
	return temp;
}
*/

void* uploadPthread(void* arg)
{
	if (NULL == arg)
	{
		return (void*) NULL;
	}
	//setEnterPthreadVal(true);
	
	StrInfo *pStr = (StrInfo*)arg;
	IbeaconConfig* conf = GetConfigPosition();
	int nRet;
	
	// ��post������������ͨ��tcp�ϴ�������һ��������
	if (conf->getIsOpenMacSer())
	{
		PostDataToCustomMacServer(pStr->pBuff, pStr->buffLen);	
	}
	if (conf->getTCPMacSerOpenVal())
	{
		nRet = WriteDataToNet(pStr->pBuff, pStr->buffLen, MES_UPL_PHO_MAC_INFO, 1, 0);
	}

	printf("--------send typeID=%02x dataLen=%d to server\n", MES_UPL_PHO_MAC_INFO, nRet);
	if (pStr->pBuff) free(pStr->pBuff), pStr->pBuff = NULL;
	if (pStr) free(pStr), pStr = NULL;

	//setEnterPthreadVal(false);
	return (void*)NULL;
}


void uploadMacTask(void* arg)
{
	char* buff = NULL;
	int buffLen; 
	IbeaconConfig* conf = GetConfigPosition();
	
	bool isOpenPost = conf->getIsOpenMacSer();
	bool isOpenTcp  = conf->getTCPMacSerOpenVal();
	StrInfo *pStr = NULL;
	//pthread_t id;
	
	// �鿴�Ƿ����ϴ�mac��ַ����
	if (false == isOpenPost && false == isOpenTcp)
	{
		return;
	}
	
	// ��ȡ����json��ʽ
	buff = NULL;
	buffLen = g_macList.GathAllMacInfo(&buff);
	if (buff && buffLen > 0)
	{
		pStr = (StrInfo*)malloc(sizeof(StrInfo));
		pStr->pBuff   = buff;
		pStr->buffLen = buffLen;
		pool_add_worker(uploadPthread, (void*)pStr);
	}
	return ;

#if 0
	// �漰���紫�䣬���⴫��ӵ�µ���ռ��ʱ����������߳�ִ��
	if (buff && buffLen > 0)
	{
		buff[buffLen] = 0;
		printf("@@@@@@@@@@@@@\n%s\n@@@@@@@@@@@@", buff);
		if (getEnterPthreadVal() == false)
		{
			pStr = (StrInfo*)malloc(sizeof(StrInfo));
			pStr->pBuff   = buff;
			pStr->buffLen = buffLen;

			if ( 0 != pthread_create(&id, NULL, uploadPthread, (void*)pStr) )
			{
				Debug(I_ERROR, "pthread_create failed. pthread_id=%ld, %m", id);
				if (pStr) free(pStr), pStr = NULL;
				if (buff) free(buff), buff = NULL;	
			}
		}
		else
		{
			// �����紦�ڼ���ӵ�µ�����£���һ�����ݻ�δ���ͳ�ȥ������������ɾ��
			if (buff) free(buff), buff = NULL;
		}
	}
	return ;
#endif
}


// �ɼ������߳�
void gatherMacTask(void* arg)
{
	IbeaconConfig* conf = GetConfigPosition();

	char Output[1024] = {0};
	int len; 
	char* pMac = Output;
	int i = 0, j = 0, k = 0, l=0;

	// �鿴�Ƿ����ϴ�mac��ַ��Ϣ�������
	if (false == conf->getTCPMacSerOpenVal() && false == conf->getIsOpenMacSer())
	{
		return ;
	}

	// ����Ӧ��ʹ���ڴ�ӳ��Ƚ���Ч��
	len = GetShellCmdOutput(DEF_GET_MAC_INFO_CMD, Output, 1024);
	//write_log("len=%d, OutPut=%s", len, Output);
	if (len == -1) return;
	pMac = Output;

	// ���Թ��ƴ���ж��ٸ�Mac��ַ�������
	i =  len / DEF_MAC_LEN;
	(len % DEF_MAC_LEN) ? ++i : 0;

	char* mac[i];
	char* temp[i];
	j = 0;
	mac[j++] = pMac;
	k = 1;
	while(*pMac)
	{
		if (*pMac == '\n' || *pMac == '\r')
		{
			*pMac++ = 0;
			while((! isxdigit(*pMac)) && *pMac)++pMac;
			if (!*pMac) break;
			mac[j++] = pMac;
			++k;
		}
		else 
			++pMac;
	}
	
	// �����Ƿ���Ч
	for (l=0, j=0; j<k; ++j)
	{
		pMac = mac[j];
		for (i=0; i<MAC_LEN; ++i)
		{
			if (!isxdigit(pMac[i]) || 0 == pMac[i])
			{
				break;
			}
		}
		if (i == MAC_LEN)
		{
			temp[l++] = mac[j];
		}
	}
	
	// �ӵ�������
	for (i=0; i<l; ++i)
	{
		g_macList.AddOneMacInfo(temp[i]);
	}
}


