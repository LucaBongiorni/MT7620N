#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>



#include "upBToothBin.h"
#include "cbProjMain.h"
#include "ble_central.h"
#include "cJSON.h"
#include "md5.h"
#include "procotol.h"
#include "thread_pool.h"


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

void* updateBluetoothBinPthread(void* arg)
{
	char* data;
	char* pJSON;
	int dataLen;
	int nRet;
	char* title = NULL;
	char* firmwareRemark = NULL;
	char* firmwareBUrl = NULL;
	char* firmwareAUrl = NULL;
	int   firmwareNum;
	char* code = NULL;
	char* md5A = NULL;
	char* md5B = NULL;
	char tmp[1024] = {0};
	char md5[64] = {0};

	//Debug(I_ERROR, "updateBluetoothBinPthread");
	int i;
	cJSON* root = NULL;
	BleCentral *central = getCentralPosition();

	setEnterPthreadVal(true);
	nRet = postDataToBlueSer(&data, dataLen);
	if (nRet < 0)
	{
		if (data) free(data), data = NULL;
		setEnterPthreadVal(false);
		return (void*)NULL;
	}
	data[dataLen] = 0;
	//printf("data:%s\n", data);

	// 处理接收到的json数据
	pJSON = data + dataLen;
	while(*pJSON != '}' && pJSON > data) pJSON--;
	if (pJSON < data + 10)
	{
		if (data) free(data), data = NULL;
		setEnterPthreadVal(false);
		return (void*)NULL;
	}
	++pJSON, *pJSON = 0;
	
	pJSON = strstr(data, "\r\n\r\n");
	if (pJSON == NULL) 
	{
		Debug(I_DEBUG, "json error.");
		if (data) free(data), data = NULL;
		setEnterPthreadVal(false);
		return (void*)NULL;
	}
	pJSON = strchr(pJSON, '{');
	if (pJSON == NULL) 
	{
		Debug(I_DEBUG, "json error.");
		if (data) free(data), data = NULL;
		setEnterPthreadVal(false);
		return (void*)NULL;
	}
	Debug(I_INFO, "%s\n", pJSON);

	// 解析收到的信息
	root  = cJSON_Parse(pJSON);
	if (! root) 
	{
		if (data) free(data), data = NULL;
		setEnterPthreadVal(false);
		return (void*)NULL;
	}
	title = cJSON_GetStringItem(root, "title");
	firmwareRemark = cJSON_GetStringItem(root, "firmwareRemark");
	firmwareAUrl = cJSON_GetStringItem(root, "firmwareAUrl");
	firmwareBUrl = cJSON_GetStringItem(root, "firmwareBUrl");
	firmwareNum  = cJSON_GetNumberItem(root, "firmwareNum");
	code = cJSON_GetStringItem(root, "code");
	md5A = cJSON_GetStringItem(root, "md5A");
	md5B = cJSON_GetStringItem(root, "md5B");


	// 对比条件，判断是否要下载
	if (central->GetFirmwareVer() >= firmwareNum || 
		0 == firmwareNum ||
		NULL == firmwareAUrl )//|| NULL == md5A)
	{
		Debug(I_DEBUG, "firmwareVer=%d, firmwareNum=%d, md5A=%s", 
			central->GetFirmwareVer(), firmwareNum, md5A);
		cJSON_Delete(root), root = NULL;
		if (data) free(data), data = NULL;
		setEnterPthreadVal(false);
		return (void*)NULL;
	}

	// 下载固件
	unlink(DEF_BLU_BIN_FILE);
	snprintf(tmp, 512, "/usr/bin/wget -t5 -T30 -c %s -O %s", firmwareAUrl, DEF_BLU_BIN_FILE);
	system(tmp);
	sleep(1);
	//Debug(I_ERROR, "%s", tmp);

/*
	// md5校验
	if ( 0 == access(DEF_BLU_BIN_FILE, F_OK) )
	{
		memset(md5, 0, sizeof(md5));
		MDFile(DEF_BLU_BIN_FILE, md5);
		
		memset(tmp, 0, sizeof(tmp));
		Procotol::ASCIIToHex((u_int8*)md5A, (u_int8*)tmp, 16);
		if (memcmp(tmp, md5, 16) != 0)
		{
			Debug(I_DEBUG, "md5 check failed.");
			cJSON_Delete(root), root = NULL;
			if (data) free(data), data = NULL;
			return (void*)NULL;
		}	
	}
*/
	// 释放资源
	if (data) free(data), data = NULL;
	cJSON_Delete(root), root = NULL;

	// 通知固件升级
	central->UpdateFirmware(DEF_BLU_BIN_FILE);
	setEnterPthreadVal(false);
	return (void*)NULL;
}



int ReadOnePkg(int iSockFD, char * cData, int iLen, int iTimeOut)
{
	int nread;

	struct timeval tv;
	tv.tv_sec = iTimeOut / 1000;	// 设置读超时
	tv.tv_usec = iTimeOut % 1000;
	(void)setsockopt(iSockFD, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv) );

	do 
	{
		nread = recv(iSockFD, cData, iLen, 0);
		if (nread < 0)
		{
			if (EINTR == errno || EAGAIN == errno)
				nread = 0;				// 被中断程序中断，重新调用read()
			else
				return -1;	
		}
		else if (nread == 0)
		{
			return 0;
		}
		else
		{
			return nread;
		}
	}while(1);
	return -1;
}

int postDataToBlueSer(char** out, int &outLen)
{
	IbeaconConfig *conf = GetConfigPosition();
	BleCentral *central = getCentralPosition();
	
	char content[128] = {0};
	int contentLen = snprintf(content, 128, 
		"{\"hardwareType\":%d,\"firmwareNum\":%d}", 
		central->GetCentralType(), 
		central->GetFirmwareVer());
	//printf("%s\n", content);
	
	*out = (char*)malloc(contentLen+POST_HEAD_LEN);
	int postDataLen;
	// 组装 post 头
	postDataLen = snprintf(*out, contentLen+POST_HEAD_LEN, 
					"POST %s HTTP/1.1\r\n"
					"HOST: %s:%d\r\n"
					"Content-Type: application/json;charset=utf-8\r\n"
					"Content-Length: %d\r\n\r\n"
					"%s",
					conf->getUpdateBlueBinUrl(), 
					conf->getUpdateBlueBinHost(), 
					conf->getUpdateBlueBinPort(), 
					contentLen, content);
	//printf("%s\n", *out);

	// 创建连接
	CSocketTCP *sockConnect = new CSocketTCP(conf->getUpdateBlueBinHost(), conf->getUpdateBlueBinPort());
	int sockfd = sockConnect->Connect();
	if (-1 == sockfd)
	{
		Debug(I_DEBUG, "connect update blue firmware server failed.%s:%d", 
			conf->getUpdateBlueBinHost(), conf->getUpdateBlueBinPort());
		delete sockConnect, sockConnect = NULL;
		free(*out), *out = NULL;
		return -1;
	}
	
	// 发送数据
	if ( 0 >= Write(sockfd, *out, postDataLen, 3000) )
	{
		Debug(I_DEBUG, "write data to blue firmware server failed.");
		Close(sockfd);
		delete sockConnect, sockConnect = NULL;
		free(*out), *out = NULL;
		return -1;
	}

	// 接收返回数据
	outLen = ReadOnePkg(sockfd, *out, contentLen+POST_HEAD_LEN, 1000*60);
	if (outLen <= 0)
	{
		Debug(I_DEBUG, "read data from blue firmware server failed. outLen=%d", outLen);
		Close(sockfd);
		delete sockConnect, sockConnect = NULL;
		free(*out), *out = NULL;
		return -1;
	}
	
	Close(sockfd);
	delete sockConnect, sockConnect = NULL;
	return 0;
}




void updateBluetoothBinTask(void* arg)
{
//	pool_add_worker(updateBluetoothBinPthread, arg);
	
	if (getEnterPthreadVal() == false)
	{
		//pthread_t id;
		//pthread_create(&id, NULL, updateBluetoothBinPthread, NULL);
		//pthread_detach(id);
		pool_add_worker(updateBluetoothBinPthread, arg);
	}

	return;
}


