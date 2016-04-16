#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>

#include "pthreadServer.h"
#include "defCom.h"
#include "pthreadCom.h"
#include "cbProjMain.h"
#include "procotol.h"
#include "socketClient.h"
#include "cJSON.h"
#include "confile.h"
#include "base64RSA.h"
#include "thread_pool.h"
#include "dealWithOpenwrt.h"
#include "UCI_File.h"



// 函数功能: 对设备进行绑定
void 
dealWithPhoBindCBC(int sockFD, int phoneHandle, char* RecvBuff, int buffLen, const char* key)
{
	cJSON *root = NULL;
	char* out = NULL;
	int dataLen = 0, nRet, returnVal = 0;
	char* phoneKey = NULL;
	u_int16 typeID = 0;
	IbeaconConfig *conf = GetConfigPosition();

	// 组装发送json
	root = cJSON_Parse(RecvBuff);
	if (!root)
	{
		Debug(I_ERROR, "JSON Parse Failed...%s", RecvBuff);
		returnVal = -73;
		goto dealWithFailed;
	}
	
	cJSON_AddNumberToObject(root, "phoneHandle", phoneHandle);
	cJSON_DeleteItemFromObject(root, "phoneKey");
	cJSON_AddStringToObject(root, "phoneKey", key);
	//printf("--------------------key=%s\n", key);
	out = cJSON_Print(root);
	cJSON_Delete(root), root = NULL;
	
	// 通告服务器，进行绑定
	returnVal = WriteDataToNet(out, strlen(out), MES_APL_PHO_BIND_CBC, 1, 0);
	if (-1 == returnVal)
	{
		returnVal = -70;
		if (out) CJSONFree(out), out = NULL;
		goto dealWithFailed;
	}
	if (out) CJSONFree(out), out = NULL;

	returnVal = PhoneServerReadDataFromNet(&out, &dataLen, phoneHandle, &typeID, 3*1000);
	if ( -1 == returnVal || typeID != MES_APL_PHO_BIND_CBC_ACK)
	{
		if (returnVal == -1) returnVal = -70;
		Debug(I_ERROR, "returnVal=%d, typeID=%x", returnVal, typeID);
		out[dataLen] = 0;
		Debug(I_INFO, "recvBuff=%s\n", out);
		returnVal = -71;
		goto dealWithFailed;
	}
	out[dataLen] = 0;
	Debug(I_INFO, "recvBuff=%s\n", out);

	// 根据返回值，判断绑定是否成功
	root = cJSON_Parse(out);
	if (!root)
	{
		Debug(I_ERROR, "JSON Parse Failed...");
		return ;
	}
	
	nRet = cJSON_GetNumberItem(root, "returnVal");
	if (0 == nRet)
	{
		phoneKey = cJSON_GetStringItem(root, "phoneKey");
		if (NULL == phoneKey) 
		{
			Debug(I_DEBUG, "phoneKey is NULL.");
			returnVal = -1;
			cJSON_Delete(root), root = NULL;
			goto dealWithFailed;
		}		
		// 同步配置文件数据
		//unlink(DEF_PHONE_CONNECT_KEY);
		if (-1 == LoadMemToFile(phoneKey, strlen(phoneKey), DEF_PHONE_CONNECT_KEY) )
		{
			returnVal = -1;
			cJSON_Delete(root);
			goto dealWithFailed;
		}
		// 同步线程数据
		conf->setPhoneKey(phoneKey);
		returnVal = 0;
	}
	else if (2 == nRet)
		returnVal = -72;   // 已经绑定
	else if (3 == nRet)
		returnVal = -74;   // key不在
	else if (4 == nRet)
		returnVal = -75;   // 账户不存在
	else if (5 == nRet)
		returnVal = -76;   // 已经被其他用户绑定
	else
		returnVal = -71;   // 失败
		
	cJSON_Delete(root), root = NULL;
	
dealWithFailed:   // 发送失败
	nRet = snprintf(RecvBuff, 256, "{\"returnVal\": %d}", returnVal);
	printf("-----bind:%s\n", RecvBuff);
	sendDataByTCP(sockFD, RecvBuff, nRet, MES_PHO_BIND_CBC_ACK);
	return;
}


// 函数功能: 对设备进行解绑
void 
dealWithPhoUnbindCBC(int sockFD, int phoneHandle, char* RecvBuff, int buffLen, const char* key)
{
	int dataLen=0, nRet, returnVal;
	cJSON *root;
	char* out = NULL;
	u_int16 typeID = 0;
	IbeaconConfig *conf = GetConfigPosition();
	
/*	// 判断是否已经绑定
	if ( 0 != access(DEF_PHONE_CONNECT_KEY, F_OK) )
	{
		memcpy(RecvBuff, UnBinded, sizeof(UnBinded));
		nRet = Procotol::AddPkgHeadAndTail(RecvBuff, sizeof(UnBinded)-1, MES_PHO_UNBIND_CBC_ACK, 1, 1);
		Write(sockFD, RecvBuff, nRet, 3000);
		return ;
	}
*/
	// 组装发送json
	RecvBuff[buffLen] = 0;
	root = cJSON_Parse(RecvBuff);
	if (!root)
	{
		Debug(I_ERROR, "JSON Parse Failed...");
		return;
	}
	cJSON_AddNumberToObject(root, "phoneHandle", phoneHandle);
	cJSON_DeleteItemFromObject(root, "phoneKey");
	cJSON_AddStringToObject(root, "phoneKey", key);
	out = cJSON_Print(root);
	cJSON_Delete(root);

	// 通告服务器，进行解绑
	returnVal = WriteDataToNet(out, strlen(out), MES_APL_PHO_UNBIND_CBC, 1, 0);
	if ( -1 == returnVal)
	{
		// 解绑必须通告服务器
		returnVal = -10;
		CJSONFree(out), out = NULL;
		goto dealWithFailed;
	}
	
	CJSONFree(out), out = NULL;
	// 根据返回值，判断绑定是否成功
	returnVal = PhoneServerReadDataFromNet(&out, &dataLen, phoneHandle, &typeID, 10*1000);
	if ( -1 == returnVal || typeID != MES_APL_PHO_UNBIND_CBC_ACK)
	{
		if (-1 == returnVal) returnVal = -10;
		//write_normal_log("-----------------returnVal=%d, typeID=%x", returnVal, typeID);
		goto dealWithFailed;
	}

	// 配置
	root = cJSON_Parse(out);
	if (!root)
	{
		returnVal = -21;
		goto dealWithFailed;
	}
	nRet = cJSON_GetNumberItem(root, "returnVal");
	if (0 == nRet)       // 成功
	{
		unlink(DEF_PHONE_CONNECT_KEY);
		conf->setPhoneKey(NULL);
		returnVal = 0;
	}
	else if (2 == nRet)  // 未绑定
		returnVal = -6;
	else
		returnVal = -21; // 失败
	cJSON_Delete(root);

dealWithFailed:   // 发送失败
	nRet = snprintf(RecvBuff, 256, "{\"returnVal\": %d}", returnVal);
	sendDataByTCP(sockFD, RecvBuff, nRet, MES_PHO_UNBIND_CBC_ACK);
	return ;
}

phoneSerProc::phoneSerProc():
	m_phoneSerPthreadID(0), 
	m_exitPhoneSerPthread(false)
{
	IbeaconConfig *conf = GetConfigPosition();
	pthread_rwlock_init(&m_exitLock, NULL);
	m_listenNum = conf->getListenNum();

/*
	m_clientPthreadID = (ClientID*)malloc(sizeof(ClientID) * m_listenNum);
	for (int i=0; i<m_listenNum; ++i)
	{
		m_clientPthreadID[i].used = false;
		m_clientPthreadID[i].ptheadID = 0;
	}
*/
}

void* phoneSerProc::ClientPthreadProc(void* arg)
{
	if (NULL == arg)
	{
		Debug(I_DEBUG, "Paramter is NULL.");
		return (void*)NULL;
	}
	
	// 提取参数内容
	CliParam *serParam = (CliParam *)arg;
	int sockFD = serParam->sockFd;
	phoneSerProc* phoneSer = serParam->phoneSer;
	delete (CliParam *)arg;

	IbeaconConfig* conf = GetConfigPosition();
	u_int16 typeID = 0;
	u_int8 PkgTotal = 0, PkgSeq = 0;
	int nRet = 0;
	char key[128] = {0};     // 保存手机的key；
	char* RecvBuff = new char[1500];
	int phoneFlag = OpenPhoneHandle();
	int returnVal = 0;

	// 为每个连接上来的子端创建一个节点链表
	ListManage* IDHead = new ListManage;
	InitTypeIDList(IDHead);

	int timeOutCnt = 0;
	char* pBuff = NULL;
	int contentLen = 0;
	char* pContent = NULL;
		
	if (conf->ifBeenActivated())
	{
 		returnVal = ProvePhoneConnect(sockFD, true, phoneFlag, key);
		//printf("[%d]-----------returnVal=%d\n", __LINE__, returnVal);
		nRet = snprintf(RecvBuff, 256, "{\"returnVal\": %d}", returnVal);
		sendDataByTCP(sockFD, RecvBuff, nRet, MES_PHO_SND_KEY_ACK);
		usleep(300);
		if ( returnVal != 0 )
		{
			Debug(I_DEBUG, "--------Prove Phone Connect Failed. close socket. returnVal=%d",
				returnVal);
			goto CloseSocket;
		}
	}
	else
	{
		// 未绑定
		returnVal = ProvePhoneConnect(sockFD, false, phoneFlag, key);
		//printf("[%d]-----------returnVal=%d\n", __LINE__, returnVal);
		nRet = snprintf(RecvBuff, 256, "{\"returnVal\": %d}", returnVal);
		sendDataByTCP(sockFD, RecvBuff, nRet, MES_PHO_SND_KEY_ACK);
		usleep(300);
		if (returnVal != -6)
		{
			goto CloseSocket;
		}
	}
	Debug(I_INFO, "-----one phone client connect.-------------key=%s", key);

	for (;;)
	{
		pthread_rwlock_rdlock(&phoneSer->m_exitLock);
		if (phoneSer->m_exitPhoneSerPthread)
		{
			pthread_rwlock_unlock(&phoneSer->m_exitLock);
			break;
		}
		pthread_rwlock_unlock(&phoneSer->m_exitLock);

		nRet = RecvOnePkg(sockFD, RecvBuff, 4+HEAD_LEN, 3000);
		if (nRet < 0)        // 对方已经关闭
		{
			if (nRet == -2 || nRet == -3) continue;
			break;           // 退出
		}
		else if (nRet == 0)  // 超时
		{
			if (timeOutCnt++ == 100)   // 5分钟没有收到心跳包，退出
				break;
			else
				continue;
		}
		else 
		{
			timeOutCnt = 0;
		}

		// 处理一个包
		contentLen = Procotol::DelPkgHeadAndTail(RecvBuff, nRet-TAIL_LEN, &typeID, &PkgTotal, &PkgSeq);
		if (0 > contentLen)
		{
			continue;
		}
		if (PkgTotal != PkgSeq)
		{
			// 多包处理，将数值保存到链表中
			AddNodeToIDList(typeID, IDHead, RecvBuff, contentLen, PkgSeq);
			
			// 判断所有包是否已经全部接收完
			//if (false == CheckIDNodeIsComplete(IDHead, typeID, PkgTotal)) // 这里是使用tcp，不用使用这个
				continue;
		}
		else if ( 1 != PkgTotal && PkgTotal == PkgSeq )
		{
			// 多包中最后一个包
			AddNodeToIDList(typeID, IDHead, RecvBuff, contentLen, PkgSeq);

			// 判断所有包是否已经全部接收完	
			//if (false == CheckIDNodeIsComplete(IDHead, typeID, PkgTotal)) // 这里是使用tcp，不用使用这个
			//	continue;
			
			// 从 IDList 中提取出所有字符，并删除该 ID 节点
			contentLen = PickUpBuffFromIDList(IDHead, typeID, &pBuff);
			DelOneNodeByTypeID(IDHead, typeID);
			if (-1 == contentLen || 0 == contentLen)
			{
				// 提取失败
				Debug(I_DEBUG, "Pick up Buffer From IDList Failed. buffLen=%d", contentLen);
				if (pBuff) free(pBuff), pBuff = NULL;
				continue;
			}
		}
		if (pBuff) 
			pContent = pBuff;
		else
			pContent = RecvBuff;
		
		dealWithRecvPkg(sockFD, pContent, contentLen, typeID, phoneFlag, key);
		
		if (pBuff) free(pBuff), pBuff = NULL;
	}


CloseSocket:
	UninitTypeIDList(IDHead);
	if (IDHead) delete IDHead, IDHead = NULL;

	if (RecvBuff)
		delete [] RecvBuff, RecvBuff = NULL;

	// 返回phoneFlag
	ClosePhoneHandle(phoneFlag);

	// 关闭 socket
	Close(sockFD);

	// 去除线程ID
	//phoneSer->delClientPhteadID(pthread_self());
	write_normal_log("socket phoneclient pthread(%ld) will exit.", pthread_self());
	return (void*)NULL;
}



void* phoneSerProc::phoneSerPthread(void* arg)
{
	int sockFd;
	IbeaconConfig *conf = GetConfigPosition();
	phoneSerProc* phoneSer = (phoneSerProc*)arg;
	
	// 开启服务器
	CSocketTCP* serverTCP = new CSocketTCP(conf->getLocalPort());
	sockFd = serverTCP->StartServer(conf->getListenNum());
	if (sockFd <= 0)
	{
		Debug(I_DEBUG, "Start Lock server Failed. sockFd=%d", sockFd);
		return (void*)NULL;
	}

	// 设置超时
	struct timeval tv;
	tv.tv_sec  = DEF_RCVTIMEO / 1000;
	tv.tv_usec = DEF_RCVTIMEO % 1000;
	(void)setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	tv.tv_sec  = DEF_SNDTIMEO / 1000;
	tv.tv_usec = DEF_SNDTIMEO % 1000;
	(void)setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

	for (;;)
	{
		pthread_rwlock_rdlock(&phoneSer->m_exitLock);
		if (phoneSer->m_exitPhoneSerPthread)
		{
			pthread_rwlock_unlock(&phoneSer->m_exitLock);
			break;
		}
		pthread_rwlock_unlock(&phoneSer->m_exitLock);
		
		pthread_t clientID = -1;
		int AcceptFD = 0;
		struct sockaddr_in ClientAddr;
		memset(&ClientAddr, 0, sizeof(struct sockaddr));
		int AddrLen = sizeof(struct sockaddr);
		CliParam *SerParam = NULL;
		
		usleep(100);
		if ( -1 == (AcceptFD = serverTCP->Accept(sockFd, (struct sockaddr*)&ClientAddr, &AddrLen)) )
		{
			//Debug(I_DEBUG, "acceptFD=%d", AcceptFD);
			continue;
		}

		SerParam  = new CliParam;
		SerParam->phoneSer = phoneSer;
		SerParam->sockFd   = AcceptFD;
		pool_add_worker(ClientPthreadProc, (void*)SerParam);
		//phoneSer->addClientPthreadID(clientID);
	#if 0	
		if ( 0 != pthread_create(&clientID, 0, ClientPthreadProc, (void*)SerParam) ){
			Debug(I_DEBUG, "Create Pthread Failed. clientID=%ld, %m", clientID);
			if (SerParam) delete SerParam, SerParam = NULL;
		}else{
			//pthread_detach(clientID);
			phoneSer->addClientPthreadID(clientID);
		}
	#endif
	}

	if (serverTCP) delete serverTCP, serverTCP = NULL;
	Close(sockFd);
	//printf("[%s:%d]phone server pthread exit...\n", __FILE__, __LINE__);
	return (void*)NULL;
}


void 
phoneSerProc::parsePhoneKey(char* key, const char* mac)
{
	unsigned int i, j, k;
	char temp[13] = {0};

	// 去掉冒号
	k = strlen(mac);
	for (j=0, i=0; i<k; ++i)
	{
		if (isxdigit(mac[i]))
			temp[j++] = mac[i];
	}
	temp[12] = 0;
	//write_log("mac:%s", temp);
	//write_log("strlen(key)=%d, key:%s", strlen(key), key);
	j = strlen(key);
	for(i=0, k=0; i<j; ++i)
	{
		if (!iscntrl(key[i]))
		{
			key[k++] = key[i];
		}
	}
	key[k] = 0;
	//write_log("strlen(key)=%d, key:%s", strlen(key), key);
	
	unsigned char dest[64] = {0};
	// 解析出key
	k = base64_decode(key, dest, 64);

	for (j=0, i=0; i<k; ++i, ++j)
	{
		j = j % 12;
		key[i] = dest[i] - temp[j];
	}
	key[i] = 0;
	return ;
}


int 
phoneSerProc::ProvePhoneConnect(int sockFd, bool prove, int phoneHandle, char* key)
{
	char RecvBuff[256] = {0};
	int buffLen, nRet; 
	u_int16 typeID = 0;
	u_int8 PkgTotal = 0, PkgSeq = 0;
	cJSON* root;
	//char key[128] = {0};
	char* out = NULL;
	IbeaconConfig *conf = GetConfigPosition();

	nRet = RecvOnePkg(sockFd, RecvBuff, 256, 60*1000);
	if (0 > nRet) 
	{
		Debug(I_DEBUG, "recv one pkg failed.");
		return -1;
	}
	buffLen = Procotol::DelPkgHeadAndTail(RecvBuff, nRet-TAIL_LEN, &typeID, &PkgTotal, &PkgSeq);
	if (0 > buffLen || typeID != MES_PHO_SND_KEY) 
	{
		if (typeID == MES_PHO_SND_KEY_ACK)   // 用于测试
		{
			if (0 != checkPasswd("root", RecvBuff) )
			{
				Debug(I_DEBUG, "proved failed. passwd=%s", RecvBuff);
				return -1;
			}
			if (prove) 
				return 0;
			else
				return -6;
		}
		Debug(I_DEBUG, "del package and tail failed.buffLen=%d, typeID=%02x", buffLen, typeID);
		return -1;
	}

	// 从 RecvBuff 中提取出 key 和账号名
	root = cJSON_Parse(RecvBuff);
	if (!root) return -1;
	strncpy(key, cJSON_GetStringItem(root, "phoneKey"), 128);
	parsePhoneKey(key, GetLanMac());    	      // 解析key
	cJSON_DeleteItemFromObject(root, "phoneKey");
	cJSON_AddStringToObject(root, "phoneKey", key);
	//printf("key=%s\n", key);
	cJSON_AddNumberToObject(root, "phoneHandle", phoneHandle);
	out = cJSON_Print(root);
	cJSON_Delete(root), root = NULL;

	// 未绑定，直接返回
	if (false == prove) 
	{
		CJSONFree(out), out = NULL;
		return -6;
	}

	// 和服务器验证key
	if (0 > WriteDataToNet(out, strlen(out), MES_VFY_KEY_SER, 1, 0)) 
	{
		// 连网失败，使用本地key进行认证
		CJSONFree(out), out = NULL;
		if ( 0 == strncmp(conf->getPhoneKey(), key, PHONE_KEY_LEN) ) 
			return 0;
		else 
			return -2;  // 本地验证失败
	}
	else
	{
		CJSONFree(out), out = NULL;
		// 等待服务器的返回，超过10秒，认为出现错误
		if ( 0 != PhoneServerReadDataFromNet(&out, &buffLen, phoneHandle, &typeID, 10*1000))
		{
			return -3;  // 服务器未返回
		}
		else
		{
			if (typeID != MES_VFY_KEY_SER_ACK) 
			{
				write_normal_log("typeID=%x", typeID);
				return -4;   // 服务器返回错误
			}
			// 解析服务器的验证结果
			root = cJSON_Parse(out);
			if (! root) return -9;
			nRet = cJSON_GetNumberItem(root, "returnVal");
			cJSON_Delete(root);

			if (nRet == 0) 
				return 0;
			else if (nRet == 1)
				return -5; 	// 服务器验证失败，不存在该key
			else if (nRet == 2)
				return -8;  // 服务器返回失败，不存在该用户
			else if (nRet == 3)
				return -9;
			else
				return -10;  // 服务器返回未知错误。
		}
	}
}


phoneSerProc* phoneSer = NULL;
int phoneSerPthreadStart()
{
	if (NULL == phoneSer)
	{
		phoneSer = new phoneSerProc;
		if (NULL == phoneSer) return -1;
		return phoneSer->phoneSerPthreadRun();
	}
	return 0;
}

void phoneSerPthreadExit()
{
	if (phoneSer)
	{
		phoneSer->phoneSerPthreadExit();
		delete phoneSer, phoneSer = NULL;
	}
}


