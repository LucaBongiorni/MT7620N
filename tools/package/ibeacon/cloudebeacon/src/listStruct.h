#ifndef  _LIST_MANAGE_H_
#define  _LIST_MANAGE_H_

#include "list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#ifndef u_int8
#define u_int8 unsigned char
#endif 
#ifndef u_int16
#define u_int16 unsigned short
#endif 
#ifndef u_int32
#define u_int32 unsigned int 
#endif 




#ifndef write_log
#define write_log(fmt,args...) printf("[%s][%d]"fmt"\n",__FILE__,__LINE__,##args)
#endif



#undef  write_error_log
#define write_error_log(fmt,args...)  printf("[%s][%d]Error:"fmt"\n",__FILE__,__LINE__,##args)
#undef  write_normal_log
#define write_normal_log(fmt,args...) printf("[%s][%d]Log:"fmt"\n",__FILE__,__LINE__,##args)




////////////////////////////////////////////////////////////////
// 链表头
typedef struct _LIST_MANAGE
{
    struct list_head head;
    pthread_mutex_t ListMutex;
	int NodeNum;
}ListManage;
#undef  ListManage_LEN
#define ListManage_LEN     (sizeof(ListManage))
////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////
// 实现一个udp接受的链表队列
// 节点信息
typedef struct _NODE_INFO
{
    u_int8 seqNum;
	char* pBuff;
    int   BuffLen;
}SeqNodeInfo;
#undef  SeqNodeInfo_LEN
#define SeqNodeInfo_LEN       (sizeof(SeqNodeInfo))

// 链表节点
typedef struct _list_node
{
    struct list_head list;
    SeqNodeInfo Info;
}SeqListNode;
#undef  SeqListNode_LEN
#define SeqListNode_LEN       (sizeof(SeqListNode))

////////////////////////////////////////////////////////////////
// 函数功能: 初始化 seq 链表
// 函数参数: seqHead seq链表头指针
void 
InitSeqList(ListManage* seqHead);

// 函数功能: 销毁 seq 链表
// 函数参数: seqHead seq链表头指针
void 
UninitSeqList(ListManage* seqHead);

// 函数功能: 删除所有seq节点
// 函数参数: seqHead seq链表头指针
void 
DelAllSeqNode(ListManage* seqHead);

// 函数功能: 根据seq判断该节点是否存在，在添加节点到链表时，要做判断
// 函数参数: seqHead seq链表头指针，seq，seq值
// 返 回 值: 存在返回 true，不存在返回 false
bool 
CheckSeqListNodeIsExist(ListManage* seqHead, u_int8 seq);

// 函数功能: 在seq链表尾部添加一个节点
// 函数参数: data 传入的字符，datalen 字符长度
//           seq  传入的 seq 值
//           seqHead seq链表头指针
// 返 回 值: 成功返回0，失败返回-1
int 
AddNodeToSeqListTail(const char* data, int dataLen, char seq, ListManage* seqHead);

// 函数功能: 在seq链表尾部添加一个节点
// 函数参数: seqHead seq链表头指针
//           NodeNum 输出参数总共多少个节点
// 返 回 值: 成功返回计算到的字符长度，失败返回-1
int 
CountSeqListBuffLen(ListManage* seqHead, int* NodeNum);

// 函数功能: 根据 pkgTotal 判断链表是否完整
// 函数参数: seqHead seq链表头指针, pkgTotal 包总个数
// 返 回 值: 成功返回 true，失败返回 false
bool 
CheckSeqListNode(ListManage* seqHead, u_int8 pkgTotal); 

// 函数功能: 根据seq提取字符信息，seq的格式为1、2、3、4模式
// 函数参数: seqHead seq链表头指针
//           ppBuff 输出参数，整个链表的字符，记得用后释放
// 返 回 值: 成功返回输出字符长度，失败返回 -1
int 
PickUpBuffFromSeqList(ListManage* seqHead, char** ppBuff);
////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////
// typeID 节点信息，每个typeID的节点里面包含一个seq链表
typedef struct _IDNodeInfo
{
	ListManage SeqListHead;
	int typeID;
}IDNodeInfo;
#undef  IDNodeInfo_LEN
#define IDNodeInfo_LEN     (sizeof(IDNodeInfo))

typedef struct id_list_node
{
    struct list_head list;
    IDNodeInfo Info;
}IDListNode;
#undef  IDListNode_LEN
#define IDListNode_LEN    (sizeof(IDListNode))


////////////////////////////////////////////////////////////////
// 函数功能: 为 ID 链表加锁 
static inline void 
LockIDList(ListManage* IDHead)
{
	pthread_mutex_lock(&IDHead->ListMutex);
}

// 函数功能: 为 ID 链表解锁 
static inline void 
UnLockIDList(ListManage* IDHead)
{
	pthread_mutex_unlock(&IDHead->ListMutex);
}

// 函数功能: 初始化 IDHead 链表 
// 函数参数: IDHead 链表头指针
void 
InitTypeIDList(ListManage* IDHead);

// 函数功能: 销毁 IDHead 链表 
// 函数参数: IDHead 链表头指针
void 
UninitTypeIDList(ListManage* IDHead);

// 函数功能: 判断该id是否在队列中
// 函数参数: IDHead 链表头指针
//           typeID 类型ID
// 返 回 值: 成功返回真，失败返回假
bool 
TestNodeInList(int typeID, ListManage* IDHead);

// 函数功能: 添加一个节点到ID链表中
// 函数参数: typeID 类型ID
//           IDHead 链表头指针
//           buff 添加到链表中的字符，buffLen 字符长度
//           seq seq值
// 返 回 值: 成功返回0，失败返回-1
int 
AddNodeToIDList(int typeID, ListManage* IDHead, const char* buff, int buffLen, char seq);

// 函数功能: 删除所有节点
// 函数参数: IDHead 链表头指针
void 
DelAllTypeIdNode(ListManage* IDHead);

// 函数功能: 根据节点 ID 删除节点
// 函数参数: IDHead 链表头指针
//           typeID 节点 ID 
void 
DelOneNodeByTypeID(ListManage* IDHead, int typeID);

// 函数功能: 根据 IDHead 提取出该 IDHead 包含的所有字符
// 函数参数: IDHead 链表头指针
//           typeID 类型ID 
//           buff 输出参数，返回所有的字符，记得释放内存
// 返 回 值: 成功返回字符长度，失败返回-1，返回0代表该id里面没有字符 
int 
PickUpBuffFromIDList(ListManage* IDHead, int typeID, char** buff);

// 函数功能: 检查 IDHead 节点内的seq链表是否完整
// 函数参数: IDHead 链表头指针，typeID 类型ID 
// 返 回 值: 完整返回true，不完整返回false；
bool 
CheckIDNodeIsComplete(ListManage* IDHead, int typeID, u_int8 pkgTotal);
////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////
// 用于缓存串口和网络通讯的信息
typedef struct _net2SerInfo
{
	char*   pBuff;
	int     BuffLen;
	u_int16 typeID;
	int     phoneFlag;
	char    netFlag;
}Net2SerInfo;

typedef struct _Net2SerNode
{
	struct list_head list;
	Net2SerInfo Info;
}Net2SerNode;
////////////////////////////////////////////////////////////////
// 函数功能: 设置最大的缓存节点数
// 函数参数: num; 输入参数，节点个数
void 
setNet2SerNodeNum(int num);

// 函数功能: 为net2ser链表加锁 
// 函数参数: net2SerHead 链表头指针
static inline void 
LockNet2SerList(ListManage* net2SerHead)
{
	pthread_mutex_lock(&net2SerHead->ListMutex);
}

// 函数功能: 为net2ser链表解锁 
// 函数参数: net2SerHead 链表头指针
static inline void 
UnLockNet2SerList(ListManage* net2SerHead)
{
	pthread_mutex_unlock(&net2SerHead->ListMutex);
}

// 函数功能: 初始化 net2SerHead 链表 
// 函数参数: net2SerHead 链表头指针
void 
InitNet2SerList(ListManage* net2SerHead);

// 函数功能: 销毁 net2SerHead 链表 
// 函数参数: net2SerHead 链表头指针
void 
UninitNet2SerList(ListManage* net2SerHead);

// 函数功能: 获取链表有多少个节点
// 函数参数: net2SerHead 链表头指针
int 
GetNet2SerListNodeNum(ListManage* net2SerHead);

// 函数功能: 往链表尾部添加节点
// 函数参数: net2SerHead 链表头指针
//           data，添加的数据
//           dataLen, 数据长度
//           typeID，数据类型
// 返 回 值: 成功返回0， 失败返回-1；
int 
AddNodeToNet2SerList(ListManage* net2SerHead, 
	const char* data, int dataLen, u_int16 typeID, int phoneFlag, bool netFlag);

// 函数功能: 删除链表第一个节点
// 函数参数: net2SerHead 链表头指针
// 返 回 值: 成功返回0， 失败返回-1；
int 
DelFirstNodeFromNet2SerList(ListManage* net2SerHead);

// 函数功能: 删除链表所有节点
// 函数参数: net2SerHead 链表头指针
// 返 回 值: 成功返回0， 失败返回-1；
int 
DelAllNet2SerListNode(ListManage* net2SerHead);


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
GetNodeInfoFromNet2SerList(ListManage* net2SerHead, 
	char* data, int dataLen, 
	u_int16* typeID, u_int32 *phoneFlag, char* netFlag, int* MemSize);


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
		u_int16* typeID, int phoneFlag, int* MemSize);








#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*_LIST_MANAGE_H_*/

