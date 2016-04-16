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
// ����ͷ
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
// ʵ��һ��udp���ܵ��������
// �ڵ���Ϣ
typedef struct _NODE_INFO
{
    u_int8 seqNum;
	char* pBuff;
    int   BuffLen;
}SeqNodeInfo;
#undef  SeqNodeInfo_LEN
#define SeqNodeInfo_LEN       (sizeof(SeqNodeInfo))

// ����ڵ�
typedef struct _list_node
{
    struct list_head list;
    SeqNodeInfo Info;
}SeqListNode;
#undef  SeqListNode_LEN
#define SeqListNode_LEN       (sizeof(SeqListNode))

////////////////////////////////////////////////////////////////
// ��������: ��ʼ�� seq ����
// ��������: seqHead seq����ͷָ��
void 
InitSeqList(ListManage* seqHead);

// ��������: ���� seq ����
// ��������: seqHead seq����ͷָ��
void 
UninitSeqList(ListManage* seqHead);

// ��������: ɾ������seq�ڵ�
// ��������: seqHead seq����ͷָ��
void 
DelAllSeqNode(ListManage* seqHead);

// ��������: ����seq�жϸýڵ��Ƿ���ڣ�����ӽڵ㵽����ʱ��Ҫ���ж�
// ��������: seqHead seq����ͷָ�룬seq��seqֵ
// �� �� ֵ: ���ڷ��� true�������ڷ��� false
bool 
CheckSeqListNodeIsExist(ListManage* seqHead, u_int8 seq);

// ��������: ��seq����β�����һ���ڵ�
// ��������: data ������ַ���datalen �ַ�����
//           seq  ����� seq ֵ
//           seqHead seq����ͷָ��
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1
int 
AddNodeToSeqListTail(const char* data, int dataLen, char seq, ListManage* seqHead);

// ��������: ��seq����β�����һ���ڵ�
// ��������: seqHead seq����ͷָ��
//           NodeNum ��������ܹ����ٸ��ڵ�
// �� �� ֵ: �ɹ����ؼ��㵽���ַ����ȣ�ʧ�ܷ���-1
int 
CountSeqListBuffLen(ListManage* seqHead, int* NodeNum);

// ��������: ���� pkgTotal �ж������Ƿ�����
// ��������: seqHead seq����ͷָ��, pkgTotal ���ܸ���
// �� �� ֵ: �ɹ����� true��ʧ�ܷ��� false
bool 
CheckSeqListNode(ListManage* seqHead, u_int8 pkgTotal); 

// ��������: ����seq��ȡ�ַ���Ϣ��seq�ĸ�ʽΪ1��2��3��4ģʽ
// ��������: seqHead seq����ͷָ��
//           ppBuff �������������������ַ����ǵ��ú��ͷ�
// �� �� ֵ: �ɹ���������ַ����ȣ�ʧ�ܷ��� -1
int 
PickUpBuffFromSeqList(ListManage* seqHead, char** ppBuff);
////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////
// typeID �ڵ���Ϣ��ÿ��typeID�Ľڵ��������һ��seq����
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
// ��������: Ϊ ID ������� 
static inline void 
LockIDList(ListManage* IDHead)
{
	pthread_mutex_lock(&IDHead->ListMutex);
}

// ��������: Ϊ ID ������� 
static inline void 
UnLockIDList(ListManage* IDHead)
{
	pthread_mutex_unlock(&IDHead->ListMutex);
}

// ��������: ��ʼ�� IDHead ���� 
// ��������: IDHead ����ͷָ��
void 
InitTypeIDList(ListManage* IDHead);

// ��������: ���� IDHead ���� 
// ��������: IDHead ����ͷָ��
void 
UninitTypeIDList(ListManage* IDHead);

// ��������: �жϸ�id�Ƿ��ڶ�����
// ��������: IDHead ����ͷָ��
//           typeID ����ID
// �� �� ֵ: �ɹ������棬ʧ�ܷ��ؼ�
bool 
TestNodeInList(int typeID, ListManage* IDHead);

// ��������: ���һ���ڵ㵽ID������
// ��������: typeID ����ID
//           IDHead ����ͷָ��
//           buff ��ӵ������е��ַ���buffLen �ַ�����
//           seq seqֵ
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1
int 
AddNodeToIDList(int typeID, ListManage* IDHead, const char* buff, int buffLen, char seq);

// ��������: ɾ�����нڵ�
// ��������: IDHead ����ͷָ��
void 
DelAllTypeIdNode(ListManage* IDHead);

// ��������: ���ݽڵ� ID ɾ���ڵ�
// ��������: IDHead ����ͷָ��
//           typeID �ڵ� ID 
void 
DelOneNodeByTypeID(ListManage* IDHead, int typeID);

// ��������: ���� IDHead ��ȡ���� IDHead �����������ַ�
// ��������: IDHead ����ͷָ��
//           typeID ����ID 
//           buff ����������������е��ַ����ǵ��ͷ��ڴ�
// �� �� ֵ: �ɹ������ַ����ȣ�ʧ�ܷ���-1������0�����id����û���ַ� 
int 
PickUpBuffFromIDList(ListManage* IDHead, int typeID, char** buff);

// ��������: ��� IDHead �ڵ��ڵ�seq�����Ƿ�����
// ��������: IDHead ����ͷָ�룬typeID ����ID 
// �� �� ֵ: ��������true������������false��
bool 
CheckIDNodeIsComplete(ListManage* IDHead, int typeID, u_int8 pkgTotal);
////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////
// ���ڻ��洮�ں�����ͨѶ����Ϣ
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
// ��������: �������Ļ���ڵ���
// ��������: num; ����������ڵ����
void 
setNet2SerNodeNum(int num);

// ��������: Ϊnet2ser������� 
// ��������: net2SerHead ����ͷָ��
static inline void 
LockNet2SerList(ListManage* net2SerHead)
{
	pthread_mutex_lock(&net2SerHead->ListMutex);
}

// ��������: Ϊnet2ser������� 
// ��������: net2SerHead ����ͷָ��
static inline void 
UnLockNet2SerList(ListManage* net2SerHead)
{
	pthread_mutex_unlock(&net2SerHead->ListMutex);
}

// ��������: ��ʼ�� net2SerHead ���� 
// ��������: net2SerHead ����ͷָ��
void 
InitNet2SerList(ListManage* net2SerHead);

// ��������: ���� net2SerHead ���� 
// ��������: net2SerHead ����ͷָ��
void 
UninitNet2SerList(ListManage* net2SerHead);

// ��������: ��ȡ�����ж��ٸ��ڵ�
// ��������: net2SerHead ����ͷָ��
int 
GetNet2SerListNodeNum(ListManage* net2SerHead);

// ��������: ������β����ӽڵ�
// ��������: net2SerHead ����ͷָ��
//           data����ӵ�����
//           dataLen, ���ݳ���
//           typeID����������
// �� �� ֵ: �ɹ�����0�� ʧ�ܷ���-1��
int 
AddNodeToNet2SerList(ListManage* net2SerHead, 
	const char* data, int dataLen, u_int16 typeID, int phoneFlag, bool netFlag);

// ��������: ɾ�������һ���ڵ�
// ��������: net2SerHead ����ͷָ��
// �� �� ֵ: �ɹ�����0�� ʧ�ܷ���-1��
int 
DelFirstNodeFromNet2SerList(ListManage* net2SerHead);

// ��������: ɾ���������нڵ�
// ��������: net2SerHead ����ͷָ��
// �� �� ֵ: �ɹ�����0�� ʧ�ܷ���-1��
int 
DelAllNet2SerListNode(ListManage* net2SerHead);


// ��������: ������ͷ����ȡһ���ڵ�
// ��������: net2SerHead ����ͷָ��
//           data�������������ȡ���Ľڵ�����
//           dataLen, �����������������С
//           typeID�������������������
//           phoneFlag ����������ֻ���־
//           netFlag ���������1�����磬0���ֻ�
//           MemSize ���������1��ʾ����Ҫ���Ļ�����
// �� �� ֵ: �ɹ����ػ�ȡ���ݵĳ��ȣ�ʧ�ܷ���-1��
int 
GetNodeInfoFromNet2SerList(ListManage* net2SerHead, 
	char* data, int dataLen, 
	u_int16* typeID, u_int32 *phoneFlag, char* netFlag, int* MemSize);


// ��������: �����ֻ���־λ����ȡ����������
// ��������: ser2NetHead ����ͷָ��
//           data��������������ص�����
//           dataLen�����������data��������С
//           typeID�������������������
//           phoneFlag, ����������ֻ���־
//           MemSize��������������ʣ������ݴ�С
// �� �� ֵ: ����-1ʧ�ܣ�����0���Ҳ������ɹ����ػ�ȡ�������ݳ���
int 
GetNodeInfoFromListByPhoneFlag(ListManage* ser2NetHead, char* data, int dataLen, 
		u_int16* typeID, int phoneFlag, int* MemSize);








#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*_LIST_MANAGE_H_*/

