#ifndef __SOCKET_CLIENT_H__
#define __SOCKET_CLIENT_H__

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
// ���ڻ���ɼ�����mac��ַ����
typedef struct _MacInfo
{
	char Mac[MAC_LEN+1];
	time_t TimePos;
	struct list_head list;
}MacNode;
#define MacNodeLen   (sizeof(MacNode))

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
///////////////////////////////////////////////////////////////////


class MacList
{
public:
	MacList();
	~MacList();

	int AddOneMacInfo(const char* mac);
	int GathAllMacInfo(char** macInfo);
	void DelAllMacListNode();
	
private:
	struct list_head m_Head;
	pthread_mutex_t m_ListMutex;
	int m_NodeNum;
	MemPool* m_MemPool;
};


///////////////////////////////////////////////////////////////////////
void* 
ConnectWebSeverProc(void* argc);

int 
GetServerFD(void);

// ��������: �������֤����ݺ���
// ��������: sockFd���׽���������
//           uID�����ʶ��usid
// �� �� ֵ: �ɹ�����1�����������0����֤������-1��
int
ProveToServer(int sockFd, const char* uID);


// ��������: ��ȡ�ű�ִ������ַ�
// ��������: cmd��ִ�еĽű�����
//           output������Ľ��
//           OutputLen��output��������С
// �� �� ֵ: �ɹ����ض�ȡ�����ַ�����ʧ�ܷ���-1
int 
GetShellCmdOutput(const char* cmd, char* Output, int OutputLen);


// �ϴ�mac��ַ��������
void* 
uploadMacInfo(void* arg);
// �Ѽ�mac��ַ��Ϣ
void* 
gathMacDataFromProc(void* arg);



#endif /*__SOCKET_CLIENT_H__*/

