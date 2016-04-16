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
// 用于缓存采集到的mac地址数据
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

// 函数功能: 向服务器证明身份函数
// 函数参数: sockFd，套接字描述符
//           uID，身份识别usid
// 返 回 值: 成功返回1，网络出错返回0，认证出错返回-1；
int
ProveToServer(int sockFd, const char* uID);


// 函数功能: 获取脚本执行输出字符
// 函数参数: cmd，执行的脚本命令
//           output，输出的结果
//           OutputLen，output缓冲区大小
// 返 回 值: 成功返回读取到的字符数，失败返回-1
int 
GetShellCmdOutput(const char* cmd, char* Output, int OutputLen);


// 上传mac地址给服务器
void* 
uploadMacInfo(void* arg);
// 搜集mac地址信息
void* 
gathMacDataFromProc(void* arg);



#endif /*__SOCKET_CLIENT_H__*/

