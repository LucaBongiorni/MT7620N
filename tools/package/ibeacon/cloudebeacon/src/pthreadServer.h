#ifndef __PTHREAD_SERVER_H__
#define __PTHREAD_SERVER_H__

#include "StrOperate.h"
#include <pthread.h>


void* 
localServerPthreadProc(void* argc);

// ��������: ��֤����
// ��������: sockFd���׽���������
//           prove���Ƿ��Ѿ���
//           phoneHandle���ֻ�ͨѶ���
// �� �� ֵ: 
int 
ProvePhoneConnect(int sockFd, bool prove, int phoneHandle, char* key);


// ��������: �����ֻ����͹�����key
// ��������: key���ֻ�key
//           mac��WAN��mac��ַ
void parsePhoneKey(char* key, const char* mac);


#endif /*__PTHREAD_SERVER_H__*/

