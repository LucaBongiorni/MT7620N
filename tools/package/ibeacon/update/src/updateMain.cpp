#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "confile.h"
#include "defCom.h"
//#include "md5.h"
#include "netSockTcp.h"
#include "StrOperate.h"
#include "updateOperate.h"
#include "cJSON.h"

#define random(x)    (rand()%x)


UPDATE update;


// 升级操作
int 
applyUpdateInfo(CSocketTCP *tcp, int sockFd, int* kind, 
	char* filePathA, char* filePathB, char* md5, int* ver)
{
	if (NULL == tcp) 
		return -1;

	char sendBuff[1024] = {0};
	char *pJSON;
	int headLen;
	int flag;
	int nRet = 0;

	// 拼接发送消息
	update.sprintSendInfo(sendBuff, &headLen);

	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printf("%s", sendBuff);
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	
	// 发送指定的数据内容
	nRet = tcp->Write(sockFd, sendBuff, headLen, 1000*3);
	if (-1 == nRet)
	{
		write_error_log("Send Buff to Update Server Failed.");
		return -1;
	}
	
	// 接收一个数据包
	memset(sendBuff, 0, 1024);
	nRet = tcp->Recv(sockFd, sendBuff, 1024, 1000*3);
	if (0 >= nRet)
	{
		write_error_log("Recv Buff From Update Server Failed.");
		return -1;
	}

	// 提取 json
	pJSON = sendBuff + nRet;
	while(*pJSON != '}' && pJSON > sendBuff) pJSON--;
	if (pJSON < sendBuff + 10) return -1;
	++pJSON;
	*pJSON = 0;
	pJSON = strstr(sendBuff, "\r\n\r\n");
	if (pJSON == NULL) return -1;
	pJSON = strchr(pJSON, '{');
	if (pJSON == NULL) return -1;

	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@nRet=%d\n", nRet);
	printf("%s\n", pJSON);
	printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

	// 提取升级 JSON 信息
	cJSON *root = cJSON_Parse(pJSON);
	flag = cJSON_GetObjectItem(root, "code")->valueint;
	if (flag == 200)
	{
		//*kind = cJSON_GetObjectItem(root, "updateType")->valueint;
		char* firewareAUrl = cJSON_GetObjectItem(root, "firmwareAUrl")->valuestring;
		char* firewareBUrl = cJSON_GetObjectItem(root, "firmwareBUrl")->valuestring;
		//char* MD5 = cJSON_GetObjectItem(root, "firewareMD5")->valuestring;
		*ver = cJSON_GetObjectItem(root, "firmwareNum")->valueint;
		printf("title=%s\n", cJSON_GetObjectItem(root, "title")->valuestring);
		printf("firmwareRemark=%s\n", cJSON_GetObjectItem(root, "firmwareRemark")->valuestring);
		//nRet = strlen(MD5);
		//update.HexStringtoInt((u_int8*)MD5, &nRet, md5);
		//strncpy(ver, updateVersion, 32);
		strncpy(filePathA, firewareAUrl, 512);
		strncpy(filePathB, firewareBUrl, 512);
	}
	cJSON_Delete(root);
	return flag;
}




// 主程序
int 
main(int argc, char** argv)
{
	//int iRet = -1;
	int sleepTime = 0;
	CSocketTCP *sockConnect;
	int sockFD = -1;
	
	update.parseComLineAndConfFile(argc, argv);

	char filePathA[512] = {0};
	char filePathB[512] = {0};
	char md5[64]      = {0};
	int ver, nRet;

	sockConnect = new CSocketTCP(update.m_webDomain, update.m_WebPort);
	if (NULL == sockConnect)
	{
		write_error_log("Malloc Memory Failed.");
		return -1;
	}
	
	for(;;)
	{
		int kind = 1;
		
		// 连接更新服务器
		sockFD = sockConnect->Connect();
		if ( -1 != sockFD)
		{
			// 请求升级信息
			nRet = applyUpdateInfo(sockConnect, sockFD, &kind, filePathA, filePathB, md5, &ver);
			if ( 200 == nRet )
			{
				// 需要升级
				if ( -1 == update.updateSystem(kind, filePathA, filePathB, md5, ver) )
				{
					write_error_log("Update system Failed.");
				}
			}
			else
				write_error_log("apply update info failed. nRet=%d", nRet);
		}
		else
			write_error_log("connect server failed.");
	
		// 关闭socket
		sockConnect->Close(sockFD);
		// 休眠1~2分钟
		srand((int)time(0));
		sleepTime = 60 + random(60);
		sleep(sleepTime); 
	}

	if (sockConnect)
		delete sockConnect, sockConnect = NULL;


	return 0;
}


#if 0
// 测试 popen
int 
main()
{
	int len;
	int i;
	char Output[1024] = {0};

	len = GetShellCmdOutput("ifconfig eth0", Output, 1024);
	for (i=0; i<len; ++i)
	{
		printf("%c", Output[i]);
	}
	printf("\nlen=%d\n", len);
	return 0;
}
#endif 



#if 0
// 测试MD5
int 
main()
{
	unsigned char buff[64] = {0};
	unsigned char temp[64] = {0};
	int datalen;
	
	char* md5 = "a3d160b18ce44a025b59ff14c9fd8ea1";  // md5.cpp 该文件的 md5 值
	
	MDFile("./md5.cpp", (char*)buff);
	
	datalen = strlen(md5);
	memcpy(temp, md5, datalen);
	HexStringtoInt(temp, &datalen);

	
	if ( 0 == memcmp(buff, temp, 16) )
	{
		printf("####################\n");
	}
	return 0;
}
#endif 

#if 0
// 测试随机数
int 
main()
{
	int sleepTime;
	srand((int)time(0));
	sleepTime = 60 + random(60);
	printf("sleepTime=%d\n", sleepTime);
	return 0;
}
#endif

