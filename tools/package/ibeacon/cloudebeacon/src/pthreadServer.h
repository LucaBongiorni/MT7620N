#ifndef __PTHREAD_SERVER_H__
#define __PTHREAD_SERVER_H__

#include "StrOperate.h"
#include <pthread.h>


void* 
localServerPthreadProc(void* argc);

// 函数功能: 认证连接
// 函数参数: sockFd，套接字描述符
//           prove，是否已经绑定
//           phoneHandle，手机通讯句柄
// 返 回 值: 
int 
ProvePhoneConnect(int sockFd, bool prove, int phoneHandle, char* key);


// 函数功能: 解析手机发送过来的key
// 函数参数: key，手机key
//           mac，WAN口mac地址
void parsePhoneKey(char* key, const char* mac);


#endif /*__PTHREAD_SERVER_H__*/

