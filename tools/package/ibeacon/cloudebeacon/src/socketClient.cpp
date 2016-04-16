#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


#include "procotol.h"
#include "socketClient.h"
#include "defCom.h"
#include "pthreadServer.h"
#include "netSockTcp.h"
#include "cbProjMain.h"
#include "base64RSA.h"
#include "list.h"
#include "listStruct.h"
#include "cJSON.h"
#include "confile.h"
#include "pthreadCom.h"


MacList *GathMacList = NULL;


static void *(*SktCliMalloc)(size_t size) = malloc;
static void (*SktCliFree)(void* ptr) = free;

static CSocketTCP* tcp = NULL;


int ServerFD = 0;
int 
GetServerFD(void)
{
	return ServerFD;
}

static void 
dealWithSerGetCBCinfo(int sockfd, char* buff);
static void
dealWithSetSerInfo(int sockfd, char* buff, int type);
static void
dealWithSetUPLInterval(int sockfd, char* buff);
static void
dealWithAplPhoBind(int sockFd, char* buff, int buffLen);
static void
dealWithAplPhoUnbind(int sockFd, char* buff, int buffLen);
static void
dealWithUplBluConfFile(int sockFd, char* buff, int buffLen);
static void
dealWithUpdBluConfFile(int sockFd, char* buff, int buffLen);





void* 
ConnectWebSeverProc(void* argc)
{
	int sockFd = 0;
	int nRet;
	char buff[1500] = {0};
	char* pBuff = buff;
	char* pContent = NULL;
	u_int16 length = 0;

	IbeaconConfig *conf = GetConfigPosition();
	(void)signal(SIGPIPE, SIG_IGN);
	write_normal_log("%s:%d", conf->getWebDomain(), conf->getWebPort());
	tcp = new CSocketTCP(conf->getWebDomain(), conf->getWebPort());

	u_int16 typeID; 
	u_int8 PkgTotal; 
	u_int8 PkgSeq;
	
	// 创建缓冲链表，存储多包数据
	ListManage IDHead;
	InitTypeIDList(&IDHead);

CONNECT_SERVER:
	//ServerFD = 0;
	setPthreadcomServerFd(0);
	//write_normal_log("2222222222222222222222222222222222");
	sockFd = tcp->Connect(3000);
	if (sockFd == -1)
	{
		write_error_log("Connect Web Server Failed. sockFd=%d", sockFd);
		sleep(10);
		goto CONNECT_SERVER;
	}
	//write_log_to_file("connect server. sockfd=%d", sockFd);
	SetSocketStatus(sockFd);
	//write_normal_log("333333333333333333333333333333333");
	//ServerFD = sockFd;
	// 连接认证
	nRet = ProveToServer(sockFd, conf->getSerials());
	if (nRet <= 0)
	{
		sleep(5);
		write_error_log("Prove To Server Failed.");
		Close(sockFd);
		goto CONNECT_SERVER;
	}
	//write_normal_log("444444444444444444444444444444444");
	setPthreadcomServerFd(sockFd);
	write_normal_log("Has Been Through Prove. sockFd=%d", sockFd);
	for (;;)
	{
		memset(buff, 0, 6);
		nRet = Recv(sockFd, buff, 4+HEAD_LEN, 20*1000);
		if (nRet < 0)
		{
			// 服务器已经关闭或者网络出错，隔一段时间重连
			sleep(10);
			Close(sockFd);
			goto CONNECT_SERVER;
		}
		else if (nRet == 0)
		{
			// 超时发送心跳包
			nRet = Procotol::AddPkgHeadAndTail(buff, 0, MES_SND_HEART, 1, 1);
			if (-1 == nRet)
			{
				continue;
			}
			Send(sockFd, buff, nRet, 3000);
			continue;
		}

		if (-1 == Procotol::CheckPkgHead(buff, &length) )
		{
			continue;
		}
		write_log("Read BuffLen=%d", length);
		
		nRet = Recv(sockFd, buff, length, 30*1000);
		if (nRet < 0)
		{
			// 服务器已经关闭或者网络出错，隔一段时间重连
			sleep(10);
			Close(sockFd);
			goto CONNECT_SERVER;
		}
		else if (nRet == 0)
		{
			// 超时发送心跳包，正常不会跑到这里
			nRet = Procotol::AddPkgHeadAndTail(buff, 0, MES_SND_HEART, 1, 1);
			if (-1 == nRet)
			{
				continue;
			}
			Send(sockFd, buff, nRet, 3000);
			continue;
		}
		if (nRet < length)
		{
			// 部分数据丢失
		}
		
		pBuff = NULL;
		nRet = Procotol::DelPkgHeadAndTail(buff, length-TAIL_LEN, &typeID, &PkgTotal, &PkgSeq);
		if (nRet < 0)
		{
			write_error_log("Error package.nRet=%d", nRet);
			continue;
		}
		if (PkgTotal != PkgSeq)
		{
			// 多包处理，将数值保存到链表中
			AddNodeToIDList(typeID, &IDHead, buff, nRet, PkgSeq);
			
			// 判断所有包是否已经全部接收完
			//if (false == CheckIDNodeIsComplete(IDHead, typeID, PkgTotal)) // 这里是使用tcp，不用使用这个
				continue;
		}
		else if ( 1 != PkgTotal && PkgTotal == PkgSeq )
		{
			// 多包中最后一个包
			AddNodeToIDList(typeID, &IDHead, buff, nRet, PkgSeq);

			// 判断所有包是否已经全部接收完	
			//if (false == CheckIDNodeIsComplete(IDHead, typeID, PkgTotal)) // 这里是使用tcp，不用使用这个
			//	continue;
		                                                                                                                     	
			// 从 IDList 中提取出所有字符，并删除该 ID 节点
			nRet = PickUpBuffFromIDList(&IDHead, typeID, &pBuff);
			DelOneNodeByTypeID(&IDHead, typeID);
			if (0 >= nRet)
			{
				// 提取失败
				write_error_log("Pick up Buffer From IDList Failed.");
				if (NULL != pBuff) 
				{
					delete [] pBuff;
					pBuff = NULL;
				}
				continue;
			}
		}
		//write_log("@@@@@@@@@@@@@@");
		//print_hex(buff, nRet);
		//write_log("@@@@@@@@@@@@@@\n");
		//buff 接收到的字符，已经转换，nRet 接收到的字符长度
		printf("%#x\n", typeID);
		if (pBuff)
			pContent = pBuff;
		else
			pContent = buff;
		
		switch(typeID)
		{	
		case MES_RST_SCAN_DEV:
		case MES_BEG_SCAN_DEV:
		case MES_STP_SCAN_DEV:
		case MES_REP_SCAN_INFO:
		case MES_BLD_DEV_CONNECT:
		case MES_BRK_DEV_CONNECT:
		case MES_SET_SERVER_PARAM:
		case MES_FIND_SERVER_PARAM:
		case MES_REP_SERVER_PARAM:
		case MES_RST_CBC_BTH: 
			// 发送数据给串口
			WriteDataToSerials(pContent, nRet, typeID, 0, true);
			break;		
		case MES_SER_GET_CBC_INFO:       // 服务器获取CloudBeacon设备的基本参数
			dealWithSerGetCBCinfo(sockFd, pContent);
			break;
		case MES_SET_MAC_SER_INFO:       // 设置客户Mac服务器基本信息
			dealWithSetSerInfo(sockFd, pContent, MAC_SERVER);
			break;
		case MES_SET_BCB_SER_INFO:       // 设置客户Beacon服务器基本信息
			dealWithSetSerInfo(sockFd, pContent, BEACON_SERVER);
			break;
		case MES_SET_UPL_INTERVAL:       // 配置tcp上传mac地址的间隔时间和上传beacon的间隔时间
			dealWithSetUPLInterval(sockFd, pContent);
			break;
		case MES_VFY_KEY_SER_ACK:        // 返回验证手机key结果
		case MES_APL_PHO_BIND_CBC_ACK:   // 返回手机账户绑定cloudbeacon设备的结果
		case MES_APL_PHO_UNBIND_CBC_ACK: // 返回手机账户解绑cloudbeacon设备的结果
		case MES_SYN_CBC_CONF_ACK:
			NetWriteDataToPhoneServer(pContent, nRet, typeID);
			break;
		case MES_APL_PHO_BIND_CBC:       // 通过服务器进行绑定
			dealWithAplPhoBind(sockFd, pContent, nRet);
			break;
		case MES_APL_PHO_UNBIND_CBC:     // 通过服务器进行解绑
			dealWithAplPhoUnbind(sockFd, pContent, nRet);
			break;
		case MES_UPL_BLU_CONF_FILE:      // 服务器下发命令，上传蓝牙配置文件  
			write_normal_log("-----------upload blueteeth config file.-------------");
			dealWithUplBluConfFile(sockFd, pContent, nRet);
			break;
		case MES_UPD_BLU_CONF_FILE:      // 服务器下发命令，更新蓝牙配置文件
			write_normal_log("------------update bluetooth config file.-------------");
			dealWithUpdBluConfFile(sockFd, pContent, nRet);
			break;
			
		case MES_SND_HEART:
		case MES_UPL_PHO_MAC_INFO_ACK:
			break;
		default:
			printf("Recv Unknown Package.\n");
		}

		if (NULL != pBuff)
		{
			delete [] pBuff;
			pBuff = NULL;
		}	
	}

	UninitTypeIDList(&IDHead);

	// 关闭 socket
	Close(sockFd);
	delete tcp;
	return (void*)NULL;
}


static void
dealWithSetUPLInterval(int sockfd, char* buff)
{
	IbeaconConfig* conf = GetConfigPosition();
	cJSON *root;
	time_t macTime, beaconTime;
	char timeInterval[16] = {0};
	int val = 1;
	char sendbuff[128];
	char* out = NULL;
	int sendbuffLen;

	CConfFile* confFile = new CConfFile();
	conf->lockConfFile();
	if (!confFile->LoadFile(conf->getConfFilePath()))
	{
		write_error_log("Open ini failure.");
		delete confFile, confFile = NULL;
		conf->UnlockConfFile();
		val = 0;
		goto SetFailed;
	}
	conf->UnlockConfFile();
	
	root = cJSON_Parse(buff);
	macTime = cJSON_GetNumberItem(root, "MacInterval");
	beaconTime = cJSON_GetNumberItem(root, "BeaconInterval");
	if (macTime <= 0 || beaconTime <= 0)
	{
		delete confFile, confFile = NULL;
		cJSON_Delete(root);
		val = 0;
		goto SetFailed;
	}

	// 修改配置文件
	snprintf(timeInterval, 16, "%ld", macTime);
	confFile->SetValue("LocalServerInfo", "MacInterval", timeInterval, "#上传mac地址间隔时间");
	snprintf(timeInterval, 16, "%ld", beaconTime);
	confFile->SetValue("LocalServerInfo", "BeaconInterval", timeInterval, "#上传beacon信息间隔时间");
	
	// 修改配置信息
	conf->setMacInterval(macTime);
	conf->setBeaconInterval(beaconTime);

	conf->lockConfFile();
	confFile->Save();
	conf->UnlockConfFile();
	delete confFile, confFile = NULL;
	cJSON_Delete(root);

SetFailed:
	root = cJSON_CreateObject();	
	cJSON_AddNumberToObject(root, "returnVal", val);
	out = cJSON_Print(root);	
	cJSON_Delete(root); 
	strncpy(sendbuff, out, 128);
	free(out), out = NULL;
	
	sendbuffLen = 
		Procotol::AddPkgHeadAndTail(sendbuff, strlen(sendbuff), MES_SET_UPL_INTERVAL_ACK, 1, 1);
	if (sendbuffLen != -1)
	{
		Send(sockfd, sendbuff, sendbuffLen, 3000);
	}
	return ;
}



static void
dealWithSetSerInfo(int sockfd, char* buff, int type)
{
	IbeaconConfig* conf = GetConfigPosition();
	char temp[16] = {0};
	//char host[DOMAIN_LEN] = {0};
	//char url[URL_LEN] = {0};
	u_int16 portVal;
	time_t timeVal;
	cJSON *root;
	char* out, *host, *url;
	char sendbuff[128];
	int sendbuffLen;
	int val = 1;
	int isOpen = 0;
	u_int16 typeID;

	CConfFile* confFile = new CConfFile();
	conf->lockConfFile();
	if (!confFile->LoadFile(conf->getConfFilePath()))
	{
		write_error_log("Open ini failure.");
		conf->UnlockConfFile();
		delete confFile, confFile = NULL;
		val = 0;
		goto SetFailed;
	}
	conf->UnlockConfFile();
	
	root = cJSON_Parse(buff);
	if (type == MAC_SERVER)
	{
		typeID = MES_SET_MAC_SER_INFO_ACK;
		isOpen = cJSON_GetNumberItem(root, "MacSerOpen");
		if (isOpen == 1)
		{
			portVal = cJSON_GetNumberItem(root, "MacSerPort");
			timeVal = cJSON_GetNumberItem(root, "MacSerInterval");
			host = cJSON_GetStringItem(root, "MacSerHost");
			url  = cJSON_GetStringItem(root, "MacSerUrl");
			if ( !host || !url || portVal < 1 || timeVal < 1 )
			{
				write_error_log("Some Value is Error.");
				delete confFile, confFile = NULL;
				cJSON_Delete(root), root = NULL;
				val = 0;
				goto SetFailed;
			}
		
			// 修改配置文件
			confFile->SetValue("CustomServerInfo", "MacSerHost", host, "#客户Mac服务器地址");
			confFile->SetValue("CustomServerInfo", "MacSerUrl", url, "#客户Mac服务器url");
			snprintf(temp, 8, "%d", portVal);
			confFile->SetValue("CustomServerInfo", "MacSerPort", temp, "#客户Mac服务器端口");
			snprintf(temp, 16, "%ld", timeVal);
			confFile->SetValue("CustomServerInfo", "MacSerInterval", temp, "#客户Mac服务器上传间隔时间(单位:s)");
			temp[0] = '1', temp[1] = 0;
			confFile->SetValue("CustomServerInfo", "OpenMacSer", temp, "#开启Mac服务器");
			
			// 修改配置信息
			conf->setIsOpenMacSer(true);
			conf->setMacSerHost(host);
			conf->setMacSerUrl(url);
			conf->setMacSerPort(portVal);
			conf->setMacSerInterval(timeVal);
		}
		else
		{
			temp[0] = '0', temp[1] = 0;
			confFile->SetValue("CustomServerInfo", "OpenMacSer", temp, "#关闭Mac服务器");
			conf->setIsOpenMacSer(false);
		}	
	}
	else if (type == BEACON_SERVER)
	{
		typeID = MES_SET_BCB_SER_INFO_ACK;
		isOpen = cJSON_GetNumberItem(root, "BeaconSerOpen");
		if (isOpen != 1)
		{
			portVal = cJSON_GetNumberItem(root, "BeaconSerPort");
			timeVal = cJSON_GetNumberItem(root, "BeaconSerInterval");
			host = cJSON_GetStringItem(root, "BeaconSerHost");
			url  = cJSON_GetStringItem(root, "BeaconSerUrl");
			if ( !host || !url || portVal < 1 || timeVal < 1 )
			{
				write_error_log("Some Value is Error.");
				delete confFile, confFile = NULL;
				cJSON_Delete(root), root = NULL;
				val = 0;
				goto SetFailed;
			}
	
			confFile->SetValue("CustomServerInfo", "BeaconSerHost", host, "#客户Beacon服务器地址");
			confFile->SetValue("CustomServerInfo", "BeaconSerUrl", url, "#客户Beacon服务器url");
			snprintf(temp, 8, "%d", portVal);
			confFile->SetValue("CustomServerInfo", "BeaconSerPort", temp, "#客户Beacon服务器端口");
			snprintf(temp, 16, "%ld", timeVal);
			confFile->SetValue("CustomServerInfo", "BeaconSerInterval", temp, "#客户Beacon服务器上传间隔时间(单位:s)");
			temp[0] = '1', temp[1] = 0;
			confFile->SetValue("CustomServerInfo", "OpenBeaconSer", temp, "#开启Beacon服务器");
			
			// 修改配置信息
			conf->setIsOpenBeaconSer(true);
			conf->setBeaconSerHost(host);
			conf->setBeaconSerUrl(url);
			conf->setBeaconSerPort(portVal);
			conf->setBeaconSerInterval(timeVal);
		}
		else
		{
			temp[0] = '0', temp[1] = 0;
			confFile->SetValue("CustomServerInfo", "OpenBeaconSer", temp, "#关闭Beacon服务器");
			conf->setIsOpenBeaconSer(false);
		}
	}

	conf->lockConfFile();
	confFile->Save();
	conf->UnlockConfFile();
	delete confFile, confFile = NULL;
	cJSON_Delete(root), root = NULL;

SetFailed:
	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "returnVal", val);
	out = cJSON_Print(root);
	cJSON_Delete(root);
	strncpy(sendbuff, out, 128);
	free(out), out = NULL;
	
	sendbuffLen = Procotol::AddPkgHeadAndTail(sendbuff, strlen(sendbuff), typeID, 1, 1);
	if (sendbuffLen != -1)
	{
		Send(sockfd, sendbuff, sendbuffLen, 3000);
	}
	return ;
}


static void 
dealWithSerGetCBCinfo(int sockfd, char* buff)
{
	int outLen;
	char* ptemp = Procotol::MakeSerGetCBCInfoAck();
	if (NULL == ptemp)
	{
		write_error_log("Make cloude beacon info failed.");
		return ;
	}
	
	outLen = strlen(ptemp);
	memcpy(buff, ptemp, outLen);
	free(ptemp), ptemp = NULL;
	
	outLen = Procotol::AddPkgHeadAndTail(buff, outLen, MES_SER_GET_CBC_INFO_ACK, 1, 1);
	if (-1 == outLen)
	{
		write_error_log("Add package haad and tail failed.");
		return;
	}

	// 发送数据
	Send(sockfd, buff, outLen, 3000);
}

// 服务器下发命令: 绑定手机账号
static void
dealWithAplPhoBind(int sockFd, char* buff, int buffLen)
{
	IbeaconConfig *conf = GetConfigPosition();
	char* phoneKey = NULL;
	cJSON* root = NULL;
	char temp[256] = {0};
	int nRet;

	buff[buffLen] = 0;
	root = cJSON_Parse(buff);
	phoneKey = cJSON_GetStringItem(root, "phoneKey");
	if (phoneKey)
	{
		unlink(DEF_PHONE_CONNECT_KEY);
		if (-1 == LoadMemToFile(phoneKey, strlen(phoneKey), DEF_PHONE_CONNECT_KEY) )
		{
			write_error_log("Load Memory To File Failed.");
			goto dealWithFailed;
		}
	}
	else
	{
		write_error_log("Get PhoneKey is NULL.");
		goto dealWithFailed;
	}

	// 同步线程数据
	conf->setPhoneKey(phoneKey);
	cJSON_Delete(root), root = NULL;

	// 通告服务器，绑定成功
	memcpy(temp, RETURN_JSON_0, sizeof(RETURN_JSON_0)-1);
	nRet = Procotol::AddPkgHeadAndTail(temp, sizeof(RETURN_JSON_0)-1, \
		MES_APL_PHO_BIND_CBC_ACK, 1, 1);
	Send(sockFd, temp, nRet, 3000);
	return ;
	
dealWithFailed:
	// 通告服务器，绑定失败
	cJSON_Delete(root), root = NULL;
	memcpy(temp, RETURN_JSON_1, sizeof(RETURN_JSON_1)-1);
	nRet = Procotol::AddPkgHeadAndTail(temp, sizeof(RETURN_JSON_1)-1, \
		MES_APL_PHO_BIND_CBC_ACK, 1, 1);
	Send(sockFd, temp, nRet, 3000);
	return ;
}


// 服务器下发命令: 解绑手机账号
static void
dealWithAplPhoUnbind(int sockFd, char* buff, int buffLen)
{
	IbeaconConfig *conf = GetConfigPosition();
	cJSON* root = NULL;
	char* phoneKey = NULL;
	char* fileKey = NULL;
	char temp[256] = {0};
	int nRet;

	buff[buffLen] = 0;
	root = cJSON_Parse(buff);
	phoneKey = cJSON_GetStringItem(root, "phoneKey");
	
	fileKey = conf->getPhoneKey();
	if ( 0 != strcmp(phoneKey, fileKey) )
	{
		// 解绑失败
		memcpy(temp, RETURN_JSON_1, sizeof(RETURN_JSON_1)-1);
		nRet = Procotol::AddPkgHeadAndTail(temp, sizeof(RETURN_JSON_1)-1, 
			MES_APL_PHO_UNBIND_CBC_ACK, 1, 1);
		Send(sockFd, temp, nRet, 3000);
		cJSON_Delete(root), root = NULL;
		return ;
	}
	else
	{
		unlink(DEF_PHONE_CONNECT_KEY);
		conf->setPhoneKey(NULL);
		cJSON_Delete(root), root = NULL;

		// 解绑成功
		memcpy(temp, RETURN_JSON_0, sizeof(RETURN_JSON_0)-1);
		nRet = Procotol::AddPkgHeadAndTail(temp, sizeof(RETURN_JSON_0)-1, 
			MES_APL_PHO_UNBIND_CBC_ACK, 1, 1);
		Send(sockFd, temp, nRet, 3000);
		return ;
	}
}


// 服务器下发命令，上传蓝牙配置文件
static void
dealWithUplBluConfFile(int sockFd, char* buff, int buffLen)
{
	// 提取配置文件名
	cJSON* root = NULL;
	char* file = NULL;
	char temp[EVERY_PKG_LEN+DEF_PRO_HEAD_LEN] = {0};
	char* content = NULL;
	int nRet, nSend, sendLen, len;
	char PkgTotal, PkgSeq;
	
	buff[buffLen] = 0;
	root = cJSON_Parse(buff);
	file = cJSON_GetStringItem(root, "FileName");

	if (file)
	{
		snprintf(temp, EVERY_PKG_LEN+DEF_PRO_HEAD_LEN, 
			DEF_BLU_CONF_DIR"%s", file);
		if (0 == access(temp, F_OK))
		{
			nRet = LoadFileToMem(&content, temp);
			if (-1 == nRet)
			{
				write_error_log("Load File To Memory Failed.");
				cJSON_AddNullToObject(root, "Content");
			}
			else
			{
				cJSON_AddStringToObject(root, "Content", content);
			}
		}
		else
		{
			cJSON_AddNullToObject(root, "Content");
		}
	}
	else
	{
		cJSON_AddNullToObject(root, "Content");
	}

	free(content), content = NULL;
	content = cJSON_Print(root);
	cJSON_Delete(root), root = NULL;

	nRet = strlen(content);
	PkgTotal = nRet / EVERY_PKG_LEN;
	(nRet % EVERY_PKG_LEN == 0) ? PkgTotal : (++PkgTotal);
	for (sendLen=0, PkgSeq=1; PkgSeq <= PkgTotal; ++PkgSeq)
	{
		if (PkgSeq == PkgTotal)
			nSend = nRet % EVERY_PKG_LEN;
		else
			nSend = EVERY_PKG_LEN;

		memcpy(temp, content+sendLen, nSend);
		len = Procotol::AddPkgHeadAndTail(temp, nSend, MES_UPL_BLU_CONF_FILE_ACK, PkgTotal, PkgSeq);
		sendLen += nSend;

		Send(sockFd, temp, len, 3000);
	}
	free(content), content = NULL;
	
	return ;
}

// 服务器下发命令，更新蓝牙配置文件
static void
dealWithUpdBluConfFile(int sockFd, char* buff, int buffLen)
{
	cJSON* root=NULL;
	char *file=NULL, *content=NULL;
	char temp[256] = {0};
	int nRet = 0;

	//write_log("111111111111111111111111, bufflen=%d", buffLen);
	buff[buffLen] = 0;
	root = cJSON_Parse(buff);
	file = cJSON_GetStringItem(root, "FileName");
	content = cJSON_GetStringItem(root, "Content");
	if (file && content)
	{
		//write_log("22222222222222222222222222222");
		snprintf(temp, 256, DEF_BLU_CONF_DIR"%s", file);
		LoadMemToFile(content, strlen(content), temp);

		//write_log("%s", content);
		memcpy(temp, RETURN_JSON_0, sizeof(RETURN_JSON_0)-1);
		nRet = Procotol::AddPkgHeadAndTail(temp, sizeof(RETURN_JSON_0)-1, \
			MES_UPD_BLU_CONF_FILE_ACK, 1, 1);
		Send(sockFd, temp, nRet, 3000);
	}
	else
	{
		memcpy(temp, RETURN_JSON_1, sizeof(RETURN_JSON_1)-1);
		nRet = Procotol::AddPkgHeadAndTail(temp, sizeof(RETURN_JSON_1)-1, \
			MES_UPD_BLU_CONF_FILE_ACK, 1, 1);
		Send(sockFd, temp, nRet, 3000);
	}
	cJSON_Delete(root), root = NULL;
	//write_log("3333333333333333333333333");
	// 退出程序，由监护进程重启程序
	sleep(3);
	exit(1);
}




int
ProveToServer(int sockFd, const char* uID)
{
	char buff[512] = {0};
	int  nRet;
	
	u_int16 length;
	u_int16 typeID;
	u_int8  PkgTotal, PkgSeq;

	unlink(DEF_PUBLIC_KEY_FILE);

	// 拼接请求公钥消息
	nRet = Procotol::AddPkgHeadAndTail(buff, 0, MES_APY_PUB_KEY, 1, 1);
	if (nRet == -1)
	{
		write_error_log("ProveToServer Failed.");
		return -1;
	}
	
	// 发送请求
	length = nRet;
	nRet = Send(sockFd, buff, nRet, 10000);
	if (nRet <= 0)
	{
		write_error_log("ProveToServer Failed.");
		return nRet;
	}
	//write_normal_log("bbbbbbbbbbbbbbbbbbbbbbbbbbbb");

	// 下载公钥
	nRet = Recv(sockFd, buff, HEAD_LEN+4, 10000);
	if (nRet <= 0)
	{
		write_error_log("ProveToServer Failed. nRet=%d, %m", nRet);
		return nRet;
	}
	//write_normal_log("ccccccccccccccccccccccccccccccccc");
	Procotol::CheckPkgHead(buff, &length);
	nRet = Recv(sockFd, buff, (int)length, 10000);
	if (nRet <= 0)
	{
		write_error_log("ProveToServer Failed.nRet=%d, %m", nRet);
		return nRet;
	}
	
	// 解析公钥
	nRet = Procotol::DelPkgHeadAndTail(buff, length-TAIL_LEN, &typeID, &PkgTotal, &PkgSeq);
	if (nRet == -1 || typeID != MES_APY_PUB_KEY_ACK)
	{
		write_error_log("ProveToServer Failed. nRet=%d", nRet);
		return -1;
	}
	LoadMemToFile(buff, nRet, DEF_PUBLIC_KEY_FILE);
	
	// 使用公钥加密 uid
	EVPRSA *rsa = new EVPRSA();
	nRet = rsa->openPublicKey(DEF_PUBLIC_KEY_FILE);
	if (-1 == nRet)
	{
		write_error_log("ProveToServer Failed.");
		delete rsa, rsa = NULL;	
		return -1;
	}
	//write_normal_log("fffffffffffffffffffffffffffffffffffff");

	// 使用公钥加密
	size_t encDataLen;
	nRet = rsa->rsaKeyEncrypt((unsigned char*)uID, DEF_UID_LEN, (u_int8*)buff, encDataLen);
	if (-1 == nRet)
	{
		write_error_log("ProveToServer Failed.");
		delete rsa, rsa = NULL;	
		return -1;
	}
	//printf("encDataLen=%d\n", encDataLen);
	delete rsa, rsa = NULL;	

	// 对加密后的数据进行base64
	CBase64Operate base64;
	char data[512] = {0};
	unsigned long dataLen = sizeof(data);
	if ( false == base64.Encrypt(buff, encDataLen, data, dataLen))
	{
		write_error_log("ProveToServer Failed.");
		return -1;
	}
	
	// 发送加密后的uid
	nRet = Procotol::AddPkgHeadAndTail(data, dataLen, MES_SND_UID_TO_SER, 1, 1);
	if (-1 == nRet)
	{
		write_error_log("ProveToServer Failed.");
		return -1;
	}
	nRet = Send(sockFd, data, nRet, 10000);
	if (-1 == nRet)
	{
		write_error_log("ProveToServer Failed.");
		return -1;
	}

	// 接收
	nRet = Recv(sockFd, buff, HEAD_LEN+4, 10000);
	if (0 >= nRet)
	{
		write_error_log("ProveToServer Failed. nRet=%d", nRet);
		return nRet;
	}
	Procotol::CheckPkgHead(buff, &length);
	nRet = Recv(sockFd, buff, length, 3000);
	if (0 >= nRet)
	{
		write_error_log("ProveToServer Failed. nRet=%d", nRet);
		return nRet;
	}

	// 解析
	nRet = Procotol::DelPkgHeadAndTail(buff, length-TAIL_LEN, &typeID, &PkgTotal, &PkgSeq);
	if (-1 == nRet || typeID != MES_SND_UID_TO_SER_ACK)
	{
		write_error_log("ProveToServer Failed.");
		return -1;
	}
	return 1;
}


// 函数功能: 获取脚本执行输出字符
// 函数参数: cmd，执行的脚本命令
//           output，输出的结果
//           OutputLen，output缓冲区大小
// 返 回 值: 成功返回读取到的字符数，失败返回-1
int 
GetShellCmdOutput(const char* cmd, char* Output, int OutputLen)
{
	if ( !cmd || !Output || OutputLen < 1)
	{
		 write_error_log("arguments error, szCmd is %s!", cmd);
		 return -1;
	}		

	int nRet = -1;
	FILE *pFile = NULL;   
	pFile = popen(cmd, "r");
	if (pFile)
	{
		nRet = fread(Output, sizeof(char), OutputLen, pFile);
		if ( nRet == 0)
		{
			pclose(pFile);
			pFile = NULL;
			return -1;
		}

		pclose(pFile);
		pFile = NULL;
	}
	return nRet;
}





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



int MacList::AddOneMacInfo(const char* mac)
{
	if (NULL == mac)
	{
		return -1;
	}
	struct list_head *pTempList, *pList; 
	MacNode *tempNode;
	bool delFlag = false;
	bool isNotAdd = true;

	// 遍历一次，如果存在两个相同的节点，修改第二个节点的时间
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
	// 是否要挂载到尾部
	if (isNotAdd == false)
	{
		// 申请一个节点
		MacNode* newNode = (MacNode*)m_MemPool->Alloc(sizeof(MacNode));
		if (NULL == newNode)
		{
			write_error_log("Malloc memory failed.");
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

int MacList::GathAllMacInfo(char** macInfo)
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


void* uploadMacInfo(void* arg)
{
	char* buff = NULL;
	int buffLen; 
	//int i, temp = 0;

	IbeaconConfig* conf = GetConfigPosition();
	bool isOpenPost;
	int time1, time2, MaxTime, MinTime;
	
	pthread_cond_t		cond = PTHREAD_COND_INITIALIZER;
	pthread_mutex_t		cond_mutex = PTHREAD_MUTEX_INITIALIZER;
	struct	timespec	timeout;

loop:
	time1 = conf->getMacInterval();
	time2 = conf->getMacSerInterval();
	MaxTime = Max(time1, time2);
	MinTime = Min(time1, time2);

	//write_log("========================time1=%d, time2=%d", time1, time2);
	timeout.tv_sec = time(NULL) + MinTime;
	timeout.tv_nsec = 0;
	pthread_mutex_lock(&cond_mutex);
	pthread_cond_timedwait(&cond, &cond_mutex, &timeout);
	pthread_mutex_unlock(&cond_mutex);

	// 提取数据json格式
	buffLen = GathMacList->GathAllMacInfo(&buff);
	isOpenPost = conf->getIsOpenMacSer();
	if (isOpenPost)    // 有开启post功能
	{
		if (time1 > time2)
		{
			// 先post到服务器
			PostDataToCustomMacServer(buff, buffLen);
		}
		else if (time1 == time2)
		{	
			// 用 post 发送到用户服务器
			PostDataToCustomMacServer(buff, buffLen);
			
			// 用tcp发送到服务器
			WriteDataToNet(buff, buffLen, MES_UPL_PHO_MAC_INFO, 1, 0);
			free(buff), buff = NULL;
			goto loop;
		}
		else
		{
			// 用tcp发送到服务器
			WriteDataToNet(buff, buffLen, MES_UPL_PHO_MAC_INFO, 1, 0);
		}
	}

	timeout.tv_sec = time(NULL) + (MaxTime - MinTime);
	timeout.tv_nsec = 0;
	pthread_mutex_lock(&cond_mutex);
	pthread_cond_timedwait(&cond, &cond_mutex, &timeout);
	pthread_mutex_unlock(&cond_mutex);
	write_log("time1=%d, time2=%d", time1, time2);

	if (time1 > time2)
	{
		//先用tcp发送到服务器
		//write_log("%s", buff);
		WriteDataToNet(buff, buffLen, MES_UPL_PHO_MAC_INFO, 1, 0);
	}
	else
	{
		// post到服务器
		//write_log("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		PostDataToCustomMacServer(buff, buffLen);
		//write_log("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
	}
	free(buff), buff = NULL;
	goto loop;

	if (buff)free(buff), buff=NULL;
	return (void*)NULL;
}



// 采集数据线程
void* gathMacDataFromProc(void* arg)
{
#define DEF_MAC_LEN   (sizeof("000000000000\n")-1)

	GathMacList = new MacList();
	pthread_cond_t		cond = PTHREAD_COND_INITIALIZER;
	pthread_mutex_t		cond_mutex = PTHREAD_MUTEX_INITIALIZER;
	struct timespec timeout;

	char* Output = (char*)malloc(1024);
	int len; 
	char* pMac = Output;
	int i = 0, j = 0, k = 0, l=0;
	
loop:
	timeout.tv_sec = time(0) + 5;
	timeout.tv_nsec = 0;
	pthread_mutex_lock(&cond_mutex);
	pthread_cond_timedwait(&cond, &cond_mutex, &timeout);
	pthread_mutex_unlock(&cond_mutex);

	len = GetShellCmdOutput(DEF_GET_MAC_INFO_CMD, Output, 1024);
	write_log("len=%d, OutPut=%s", len, Output);
	if (len == -1) goto loop;
	pMac = Output;

	// 粗略估计大概有多少个Mac地址，多估计
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
	
	// 数据是否有效
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
	
	// 加到链表保存
	for (i=0; i<l; ++i)
	{
		GathMacList->AddOneMacInfo(temp[i]);
	}
	
	goto loop;
	
	delete GathMacList;
	free(Output);
	return (void*)NULL;
}


