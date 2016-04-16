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
#include "UCI_File.h"
#include "base64RSA.h"



// ��������: ���豸���а�
static void 
dealWithPhoBindCBC(int sockFD, int phoneHandle, char* RecvBuff, int buffLen, char* key)
{
	cJSON *root;
	char* out = NULL;
	int dataLen = 0, nRet;
	char* phoneKey = NULL;
	u_int16 typeID;
	IbeaconConfig *conf = GetConfigPosition();
/*
	// �鿴�Ƿ񱻰󶨣��Ѿ��󶨣�ֱ�ӷ���
	if (0 == access(DEF_PHONE_CONNECT_KEY, F_OK))
	{
		memcpy(RecvBuff, BeenBinded, sizeof(BeenBinded));
		nRet = Procotol::AddPkgHeadAndTail(RecvBuff, sizeof(BeenBinded)-1, MES_PHO_BIND_CBC_ACK, 1, 1);
		Write(sockFD, RecvBuff, nRet, 3000);
		return ;
	}
*/	
	// ��װ����json
	RecvBuff[buffLen] = 0;
	root = cJSON_Parse(RecvBuff);
	cJSON_AddNumberToObject(root, "phoneHandle", phoneHandle);
	cJSON_DeleteItemFromObject(root, "phoneKey");
	cJSON_AddStringToObject(root, "phoneKey", key);
	out = cJSON_Print(root);
	cJSON_Delete(root);
	
	// ͨ������������а�
	if ( -1 == WriteDataToNet(out, strlen(out), MES_APL_PHO_BIND_CBC, 1, 0))
	{
		free(out), out = NULL;
		goto dealWithFailed;
	}
	free(out), out = NULL;
	
	nRet = PhoneServerReadDataFromNet(&out, &dataLen, phoneHandle, &typeID, 10*1000);
	if ( -1 == nRet || typeID != MES_APL_PHO_BIND_CBC_ACK)
	{
		goto dealWithFailed;
	}

	// ���ݷ���ֵ���жϰ��Ƿ�ɹ�
	root = cJSON_Parse(out);
	if (0 == cJSON_GetNumberItem(root, "returnVal"))
	{
	#if 0
		phoneKey = cJSON_GetStringItem(root, "phoneKey");
		if (NULL == phoneKey) 
		{
			write_error_log("phoneKey is NULL.");
			goto dealWithFailed;
		}
	#endif	
		// ͬ�������ļ�����
		unlink(DEF_PHONE_CONNECT_KEY);
		if (-1 == LoadMemToFile(phoneKey, strlen(phoneKey), DEF_PHONE_CONNECT_KEY) )
		{
			cJSON_Delete(root);
			goto dealWithFailed;
		}
		// ͬ���߳�����
		conf->setPhoneKey(phoneKey);
		cJSON_Delete(root);
	}
	else
	{
		// ʧ��
		cJSON_Delete(root);
		goto dealWithFailed;
	}
	// ���ͽ��
	memcpy(RecvBuff, out, dataLen);
	nRet = Procotol::AddPkgHeadAndTail(RecvBuff, dataLen, MES_PHO_BIND_CBC_ACK, 1, 1);
	Write(sockFD, RecvBuff, nRet, 3000);
	return ;
	
dealWithFailed:   // ����ʧ��
	memcpy(RecvBuff, FailedJSON, sizeof(FailedJSON));
	nRet = Procotol::AddPkgHeadAndTail(RecvBuff, sizeof(FailedJSON)-1, MES_PHO_BIND_CBC_ACK, 1, 1);
	Write(sockFD, RecvBuff, nRet, 3000);
	return ;
}


// ��������: ���豸���н��
static void 
dealWithPhoUnbindCBC(int sockFD, int phoneHandle, char* RecvBuff, int buffLen, char* key)
{
	int dataLen=0, nRet;
	cJSON *root;
	char* out = NULL;
	u_int16 typeID;
	IbeaconConfig *conf = GetConfigPosition();
/*	
	// �ж��Ƿ��Ѿ���
	if ( 0 != access(DEF_PHONE_CONNECT_KEY, F_OK) )
	{
		memcpy(RecvBuff, UnBinded, sizeof(UnBinded));
		nRet = Procotol::AddPkgHeadAndTail(RecvBuff, sizeof(UnBinded)-1, MES_PHO_UNBIND_CBC_ACK, 1, 1);
		Write(sockFD, RecvBuff, nRet, 3000);
		return ;
	}
*/
	// ��װ����json
	RecvBuff[buffLen] = 0;
	root = cJSON_Parse(RecvBuff);
	cJSON_AddNumberToObject(root, "phoneHandle", phoneHandle);
	cJSON_DeleteItemFromObject(root, "phoneKey");
	cJSON_AddStringToObject(root, "phoneKey", key);
	out = cJSON_Print(root);
	cJSON_Delete(root);

	// ͨ������������н��
	if ( -1 == WriteDataToNet(out, strlen(out), MES_APL_PHO_UNBIND_CBC, 1, 0))
	{
		free(out), out = NULL;
		goto dealWithFailed;
	}
	else
	{
		free(out), out = NULL;
		// ���ݷ���ֵ���жϰ��Ƿ�ɹ�
		nRet = PhoneServerReadDataFromNet(&out, &dataLen, phoneHandle, &typeID, 10*1000);
		if ( -1 == nRet || typeID != MES_APL_PHO_UNBIND_CBC_ACK)
		{
			goto dealWithFailed;
		}

		// ����
		root = cJSON_Parse(out);
		if (0 == cJSON_GetNumberItem(root, "returnVal"))
		{
			// �ɹ�
			unlink(DEF_PHONE_CONNECT_KEY);
			conf->setPhoneKey(NULL);
			cJSON_Delete(root);
		}
		else
		{
			// ʧ��
			cJSON_Delete(root);
			goto dealWithFailed;
		}
		
		// ���ͽ��
		memcpy(RecvBuff, out, dataLen);
		nRet = Procotol::AddPkgHeadAndTail(RecvBuff, dataLen, MES_PHO_BIND_CBC_ACK, 1, 1);
		Write(sockFD, RecvBuff, nRet, 3000);
	}
	return ;
	
dealWithFailed:   // ����ʧ��
	memcpy(RecvBuff, FailedJSON, sizeof(FailedJSON));
	nRet = Procotol::AddPkgHeadAndTail(RecvBuff, sizeof(FailedJSON)-1, MES_PHO_UNBIND_CBC_ACK, 1, 1);
	Write(sockFD, RecvBuff, nRet, 3000);
	return ;
}

// �ֻ���ȡcloudbeacon��������Ϣ 
static void 
dealWithPhoGetCBCInfo(int sockFD, int phoneHandle, char* RecvBuff, int buffLen)
{
	cJSON* root, *MacSer, *BeaconSer, *Wifi, *Interval;
	IbeaconConfig* conf = GetConfigPosition();
	char* out = NULL;
	int nRet, nSend, sendLen, temp;
	char seqPkg, tolPkg;
	const char *WifiChannel, *WifiApCliEnable, *WifiApCliSsid, *WifiApCliAuthMode, *WifiApCliEncrypType, *WifiApCliPassWord;
	UCI *uci = NULL;

	RecvBuff[buffLen] = 0;
	root = cJSON_Parse(RecvBuff);
	int GetMacSerInfo = cJSON_GetNumberItem(root, "GetMacSerInfo");
	int GetBeaconSerInfo = cJSON_GetNumberItem(root, "GetBeaconSerInfo");
	int GetWifiInfo = cJSON_GetNumberItem(root, "GetWifiInfo");
	int GetInterval = cJSON_GetNumberItem(root, "GetUploadIntervalInfo");
	cJSON_Delete(root), root = NULL;

	root = cJSON_CreateObject();
	if (1 == GetMacSerInfo)
	{
		cJSON_AddItemToObject(root, "GetMacSerInfo", MacSer = cJSON_CreateObject());
		if (conf->getIsOpenMacSer())
		{
			cJSON_AddNumberToObject(MacSer, "MacSerOpen", 1);
			cJSON_AddStringToObject(MacSer, "MacSerHost", conf->getMacSerHost());
			cJSON_AddStringToObject(MacSer, "MacSerUrl", conf->getMacSerUrl());
			cJSON_AddNumberToObject(MacSer, "MacSerPort", conf->getMacSerPort());
			cJSON_AddNumberToObject(MacSer, "MacSerInterval", conf->getMacSerInterval());
		}
		else
		{
			cJSON_AddNumberToObject(MacSer, "MacSerOpen", 0);
			cJSON_AddNullToObject(MacSer, "MacSerHost");
			cJSON_AddNullToObject(MacSer, "MacSerUrl");
			cJSON_AddNullToObject(MacSer, "MacSerPort");
			cJSON_AddNullToObject(MacSer, "MacSerInterval");
		}
	}
	if (1 == GetBeaconSerInfo)
	{
		cJSON_AddItemToObject(root, "GetBeaconSerInfo", BeaconSer = cJSON_CreateObject());
		if (conf->getIsOpenBeaconSer())
		{
			cJSON_AddNumberToObject(BeaconSer, "BeaconSerOpen", 1);
			cJSON_AddStringToObject(BeaconSer, "BeaconSerHost", conf->getBeaconSerHost());
			cJSON_AddStringToObject(BeaconSer, "BeaconSerUrl", conf->getBeaconSerUrl());
			cJSON_AddNumberToObject(BeaconSer, "BeaconSerPort", conf->getBeaconSerPort());
			cJSON_AddNumberToObject(BeaconSer, "BeaconSerInterval", conf->getBeaconSerInterval());
		}
		else
		{
			cJSON_AddNumberToObject(BeaconSer, "BeaconSerOpen", 0);
			cJSON_AddNullToObject(BeaconSer,   "BeaconSerHost");
			cJSON_AddNullToObject(BeaconSer,   "BeaconSerUrl");
			cJSON_AddNullToObject(BeaconSer,   "BeaconSerPort");
			cJSON_AddNullToObject(BeaconSer,   "BeaconSerInterval");
		}
	}
	if (1 == GetWifiInfo)
	{
		// ��ȡwifi���õĻ�����Ϣ
		uci = new UCI();
		uci->UCI_LoadFile(WiFi_Conf_File_Path);
		WifiApCliEnable = uci->UCI_GetOptionValue("@wifi-iface[0]", "ApCliEnable");
		WifiChannel = uci->UCI_GetOptionValue("ra0", "channel");
		WifiApCliSsid = uci->UCI_GetOptionValue("@wifi-iface[0]", "ApCliSsid");
		WifiApCliAuthMode = uci->UCI_GetOptionValue("@wifi-iface[0]", "ApCliAuthMode");
		WifiApCliEncrypType = uci->UCI_GetOptionValue("@wifi-iface[0]", "ApCliEncrypType"); 
		WifiApCliPassWord = uci->UCI_GetOptionValue("@wifi-iface[0]", "ApCliPassWord");

		cJSON_AddItemToObject(root, "GetWifiInfo", Wifi = cJSON_CreateObject());
		if (WifiApCliEnable)
			cJSON_AddStringToObject(Wifi, "WifiApCliEnable", WifiApCliEnable);
		else
			cJSON_AddStringToObject(Wifi, "WifiApCliEnable", "0");
		if (WifiChannel)
			cJSON_AddNumberToObject(Wifi, "WifiChannel", atoi(WifiChannel));
		if (WifiApCliSsid)
			cJSON_AddStringToObject(Wifi, "WifiApCliSsid", WifiApCliSsid);
		if (WifiApCliAuthMode)
			cJSON_AddStringToObject(Wifi, "WifiApCliAuthMode", WifiApCliAuthMode);
		if (WifiApCliEncrypType)
			cJSON_AddStringToObject(Wifi, "WifiApCliEncrypType", WifiApCliEncrypType);
		if (WifiApCliPassWord)
			cJSON_AddStringToObject(Wifi, "WifiApCliPassWord", WifiApCliPassWord);	
		delete uci, uci = NULL;
	}
	if (1 == GetInterval)
	{
		cJSON_AddItemToObject(root, "GetUploadIntervalInfo", Interval = cJSON_CreateObject());
		cJSON_AddNumberToObject(Interval, "MacSerInterval", conf->getMacInterval());
		cJSON_AddNumberToObject(Interval, "BeaconSerInterval", conf->getBeaconInterval());
	}
	out = cJSON_Print(root);
	cJSON_Delete(root);

	nSend = strlen(out);
	tolPkg = nSend / EVERY_PKG_LEN + 1;
	for (temp=0, seqPkg=1; seqPkg<=tolPkg; ++seqPkg)
	{
		if (seqPkg == tolPkg)
			sendLen = nSend % EVERY_PKG_LEN;
		else
			sendLen = EVERY_PKG_LEN;
		
		memcpy(RecvBuff, out+temp, sendLen);
		nRet = Procotol::AddPkgHeadAndTail(RecvBuff, sendLen, MES_PHO_GET_CBC_INFO_ACK, seqPkg, tolPkg);
		temp += sendLen;
		Write(sockFD, RecvBuff, nRet, 3000);
	}
	free(out), out = NULL;
	return ;
}


static void 
dealWithPhoSetCBCInfo(int sockFD, int phoneHandle, char* RecvBuff, int buffLen)
{
	cJSON* root, *MacSer, *BeaconSer;
	cJSON* vRoot;
	IbeaconConfig* conf = GetConfigPosition();
	char* out = NULL;
	char *host, *url;
	int port, interval, outLen, nRet;
	int SetBeaconSerInfoVal=1, SetMacSerInfoVal=1;
	u_int16 typeID;
	CConfFile* confFile;
	char temp[16] = {0};
	
	RecvBuff[buffLen] = 0;
	//printf("%s\n", RecvBuff);

	root = cJSON_Parse(RecvBuff);
	MacSer = cJSON_GetObjectItem(root, "SetMacSerInfo");
	BeaconSer = cJSON_GetObjectItem(root, "SetBeaconSerInfo"); 

	// ���������ļ����ڴ�
	confFile = new CConfFile();
	conf->lockConfFile();
	if (!confFile->LoadFile(conf->getConfFilePath()))
	{
		write_error_log("Open config file %s failure.", conf->getConfFilePath());
		conf->UnlockConfFile();

		SetMacSerInfoVal = SetBeaconSerInfoVal = 1;
		goto dealWithFailed;
	}
	conf->UnlockConfFile();

#if 1
	// �Ⱥͷ�����ͬ��
	if (NULL != MacSer || NULL != BeaconSer)
	{
		// ת����������
		if ( -1 == WriteDataToNet(RecvBuff, buffLen, MES_SYN_CBC_CONF, 1, 0))
		{
			SetMacSerInfoVal = SetBeaconSerInfoVal = 1;
			goto dealWithFailed;
		}
		// �ӷ�������ȡ����ֵ
		nRet = PhoneServerReadDataFromNet(&out, &outLen, phoneHandle, &typeID, 10000);
		if (nRet == -1 || typeID != MES_SYN_CBC_CONF_ACK)
		{
			SetMacSerInfoVal = SetBeaconSerInfoVal = 1;
			goto dealWithFailed;
		}
		// �����ͷ�����ͬ���Ľ��
		out[outLen] = 0;
		vRoot = cJSON_Parse(out);
		SetMacSerInfoVal = cJSON_GetNumberItem(vRoot, "SetMacSerInfoVal");
		SetBeaconSerInfoVal = cJSON_GetNumberItem(vRoot, "SetBeaconSerInfoVal");
		cJSON_Delete(vRoot);
	}
#else
	SetMacSerInfoVal = SetBeaconSerInfoVal = 0;
#endif	

	// �޸�������Ϣ
	if (0 == SetMacSerInfoVal && NULL != MacSer)
	{
		// �������Ѿ����£������ļ��ͳ�������ҲҪ����
		if ( 1 == cJSON_GetNumberItem(MacSer, "MacSerOpen"))
		{
			host = cJSON_GetStringItem(MacSer, "MacSerHost");
			url = cJSON_GetStringItem(MacSer, "MacSerUrl");
			port = cJSON_GetNumberItem(MacSer, "MacSerPort");
			interval = cJSON_GetNumberItem(MacSer, "MacSerInterval");

			// �޸�����
			conf->setIsOpenMacSer(true);
			conf->setBeaconSerHost(host);
			conf->setBeaconSerUrl(url);
			conf->setBeaconSerPort(port);
			conf->setBeaconSerInterval(interval);
			
			// �����ļ�
			temp[0] = '1', temp[1] = 0;
			confFile->SetValue("CustomServerInfo", "OpenMacSer", temp, "#����Mac������");
			confFile->SetValue("CustomServerInfo", "MacSerHost", host, "#�ͻ�Mac��������ַ");
			confFile->SetValue("CustomServerInfo", "MacSerUrl",  url, "#�ͻ�Mac������url");
			snprintf(temp, 16, "%d", port);
			confFile->SetValue("CustomServerInfo", "MacSerPort", temp, "#�ͻ�Mac�������˿�");
			snprintf(temp, 16, "%d", interval);
			confFile->SetValue("CustomServerInfo", "MacSerInterval", temp, "#�ͻ�Mac�������ϴ����ʱ��(��λ:s)");
		}
		else
		{
			conf->setIsOpenMacSer(false);
			// �����ļ�
			temp[0] = '0', temp[1] = 0;
			confFile->SetValue("CustomServerInfo", "OpenMacSer", temp, "#�ر�Mac������");
		}
		
	}
	if (0 == SetBeaconSerInfoVal && NULL != BeaconSer)
	{
		// �������Ѿ����£������ļ��ͳ�������ҲҪ����
		if ( 1 == cJSON_GetNumberItem(BeaconSer, "BeaconSerOpen"))
		{
			// ���³�����Ϣ
			host = cJSON_GetStringItem(BeaconSer, "BeaconSerHost");
			url = cJSON_GetStringItem(BeaconSer, "BeaconSerUrl");
			port = cJSON_GetNumberItem(BeaconSer, "BeaconSerPort");
			interval = cJSON_GetNumberItem(BeaconSer, "BeaconSerInterval");
			
			
			// �޸�����
			conf->setIsOpenBeaconSer(true);
			conf->setBeaconSerHost(host);
			conf->setBeaconSerUrl(url);
			conf->setBeaconSerPort(port);
			conf->setBeaconSerInterval(interval);

			// ���������ļ�����
			temp[0] = '1', temp[1] = 0;
			confFile->SetValue("CustomServerInfo", "OpenBeaconSer", temp, "#����Beacon������");
			confFile->SetValue("CustomServerInfo", "BeaconSerHost", host, "#�ͻ�Beacon��������ַ");
			confFile->SetValue("CustomServerInfo", "BeaconSerUrl",  url, "#�ͻ�Beacon������url");
			snprintf(temp, 16, "%d", port);
			confFile->SetValue("CustomServerInfo", "BeaconSerPort", temp, "#�ͻ�Beacon�������˿�");
			snprintf(temp, 16, "%d", interval);
			confFile->SetValue("CustomServerInfo", "BeaconSerInterval", temp, "#�ͻ�Beacon�������ϴ����ʱ��(��λ:s)");
		}
		else
		{
			// ͬ����������
			conf->setIsOpenBeaconSer(false);
			// ���������ļ�
			temp[0] = '0', temp[1] = 0;
			confFile->SetValue("CustomServerInfo", "OpenBeaconSer", temp, "#�ر�Beacon������");
		}
	}
	
	conf->lockConfFile();
	confFile->Save();
	conf->UnlockConfFile();

dealWithFailed:
	cJSON_Delete(root);
	delete confFile, confFile = NULL;
	
	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "SetMacSerInfoVal", SetMacSerInfoVal);
	cJSON_AddNumberToObject(root, "SetBeaconSerInfoVal", SetBeaconSerInfoVal);
	out = cJSON_Print(root);
	cJSON_Delete(root);

	outLen = strlen(out);
	memcpy(RecvBuff, out, outLen);
	free(out), out = NULL;
	nRet = Procotol::AddPkgHeadAndTail(RecvBuff, outLen, MES_PHO_SET_CBC_INFO_ACK, 1, 1);
	Write(sockFD, RecvBuff, nRet, 3000);
	return;
}

static void
dealWithSetWifiConf(int sockFD, int phoneHandle, char* RecvBuff, int buffLen)
{
	// ���������ò���
	char *WifiChannel, *WifiApCliEnable, *WifiApCliSsid, *WifiApCliAuthMode, *WifiApCliEncrypType, *WifiApCliPassWord;
	UCI *uci = NULL;
	cJSON *root = NULL;
	
	RecvBuff[buffLen] = 0;
	//printf("%s\n", RecvBuff);

	uci = new UCI();
	uci->UCI_LoadFile(WiFi_Conf_File_Path);

	root = cJSON_Parse(RecvBuff);
	WifiApCliEnable = cJSON_GetStringItem(root, "WifiApCliEnable");
	if (*WifiApCliEnable == '1')
	{
		WifiChannel = cJSON_GetStringItem(root, "WifiChannel");
		WifiApCliSsid = cJSON_GetStringItem(root, "WifiApCliSsid");
		WifiApCliAuthMode = cJSON_GetStringItem(root, "WifiApCliAuthMode");
		WifiApCliEncrypType = cJSON_GetStringItem(root, "WifiApCliEncrypType");
		WifiApCliPassWord = cJSON_GetStringItem(root, "WifiApCliPassWord");

		uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliEnable", WifiApCliEnable);
		uci->UCI_SetOptionValue("ra0", "channel", WifiChannel);
		uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliSsid", WifiApCliSsid);
		uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliAuthMode", WifiApCliAuthMode);
		uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliEncrypType", WifiApCliEncrypType);
		uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliPassWord", WifiApCliPassWord);
	}
	else
	{
		uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliEnable", WifiApCliEnable);
	}
	//write_log("ApCliSsid=%s", uci->UCI_GetOptionValue("@wifi-iface[0]", "ApCliSsid"));
	
	uci->UCI_Commit();
	delete uci, uci = NULL;

	cJSON_Delete(root);
	root = NULL;

	//system("nr");
	return ;
}


void* 
ClientPthreadProc(void* arg)
{
	if (NULL == arg)
	{
		write_error_log("Paramter is NULL.");
		return (void*)NULL;
	}
	// ��ȡ��������
	int sockFD = *(int*)arg;
	delete (int*)arg;

	IbeaconConfig* conf = GetConfigPosition();
	u_int16 typeID = 0;
	u_int8 PkgTotal = 0, PkgSeq = 0;
	int nRet = 0;
	char key[128] = {0};
	char* RecvBuff = new char[1500];
	int phoneFlag = OpenPhoneHandle();

	write_log("---------one connect.-----------");
#if 1	
	// ������֤
	if (conf->ifBeenActivated())
	{
		// ���˺��Ѿ�����
		if ( -1 == ProvePhoneConnect(sockFD, true, phoneFlag, key))
		{
			Close(sockFD);
			ClosePhoneHandle(phoneFlag);
			delete RecvBuff, RecvBuff = NULL;
			return (void*)NULL;
		}
		else
		{
			strncpy(RecvBuff, RETURN_JSON_0, 256);
			nRet = Procotol::AddPkgHeadAndTail(RecvBuff, sizeof(RETURN_JSON_0)-1, MES_PHO_SND_KEY_ACK, 1, 1);
			Write(sockFD, RecvBuff, nRet, 3000);
		}
	}
	else
	{
		// ���˺�δ����
		ProvePhoneConnect(sockFD, false, phoneFlag, key);
	}
#endif

	u_int16 nRead = 0;
	char* pBuff = NULL;
	int buffLen = 0;

	// Ϊÿ�������������Ӷ˴���һ���ڵ�����
	ListManage* IDHead = new ListManage;
	InitTypeIDList(IDHead);
	
	for (;;)
	{
		// ���հ�ͷ
		nRet = Read(sockFD, RecvBuff, 4+HEAD_LEN, 60*1000);
		if (0 >= nRet)
		{
			// nRet == -1 ��ʱ����Ϊ�Է��Ѿ��رգ����˳�����
			// nRet == 0  �Է��Ѿ��ر�
			write_log("Close Socket. nRet=%d", nRet);
			break;
		}
		// Э���ж�
		if ( -1 == Procotol::CheckPkgHead(RecvBuff, &nRead) )
		{
			continue;
		}
		nRet = Read(sockFD, RecvBuff, nRead, 60*1000);
		if (0 >= nRet)
		{
			// nRet == -1 ��ʱ����Ϊ�Է��Ѿ��رգ����˳�����
			// nRet == 0  �Է��Ѿ��ر�
			write_log("Close Socket. nRet=%d", nRet);
			break;
		}
		if (nRet != nRead)
		{
			// ��ȡ�����ַ������ԣ��ϸ�˵Ӧ�ò���
			write_error_log("nRet != nRead.");
		}

		buffLen = Procotol::DelPkgHeadAndTail(RecvBuff, nRet-TAIL_LEN, &typeID, &PkgTotal, &PkgSeq);
		if (0 > buffLen)
		{
			continue;
		}
		if (PkgTotal != PkgSeq)
		{
			// �����������ֵ���浽������
			AddNodeToIDList(typeID, IDHead, RecvBuff, buffLen, PkgSeq);
			
			// �ж����а��Ƿ��Ѿ�ȫ��������
			//if (false == CheckIDNodeIsComplete(IDHead, typeID, PkgTotal)) // ������ʹ��tcp������ʹ�����
				continue;
		}
		else if ( 1 != PkgTotal && PkgTotal == PkgSeq )
		{
			// ��������һ����
			AddNodeToIDList(typeID, IDHead, RecvBuff, buffLen, PkgSeq);

			// �ж����а��Ƿ��Ѿ�ȫ��������	
			//if (false == CheckIDNodeIsComplete(IDHead, typeID, PkgTotal)) // ������ʹ��tcp������ʹ�����
			//	continue;
			
			// �� IDList ����ȡ�������ַ�����ɾ���� ID �ڵ�
			buffLen = PickUpBuffFromIDList(IDHead, typeID, &pBuff);
			DelOneNodeByTypeID(IDHead, typeID);
			if (-1 == buffLen || 0 == buffLen)
			{
				// ��ȡʧ��
				write_error_log("Pick up Buffer From IDList Failed. buffLen=%d", buffLen);
				if (pBuff)
				{
					delete [] pBuff;
					pBuff = NULL;
				}
				continue;
			}
		}
		//write_log("333333333333333333333333333333, typeID=%x", typeID);
		// RecvBuff ���յ����ַ����Ѿ�ת����buffLen ���յ����ַ�����
		switch(typeID)
		{
		// ת��������
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
			// �������ݸ�����
			nRet = WriteDataToSerials(RecvBuff, buffLen, typeID, phoneFlag, false);
			break;
			
		case MES_UPL_PHO_MAC_INFO_ACK: // ��ʱ����
			break;
		case MES_PHO_BIND_CBC:         // �ֻ����豸
			printf("-----------bind--------\n");
			dealWithPhoBindCBC(sockFD, phoneFlag, RecvBuff, buffLen, key);
			break;
		case MES_PHO_UNBIND_CBC:       // ����豸
			printf("----------unbind--------\n");
			dealWithPhoUnbindCBC(sockFD, phoneFlag, RecvBuff, buffLen, key);
			break;
		case MES_PHO_GET_CBC_INFO:     // ��ȡ������Ϣ
			dealWithPhoGetCBCInfo(sockFD, phoneFlag, RecvBuff, buffLen);
			break;
		case MES_PHO_SET_CBC_INFO:     // ����������Ϣ
			dealWithPhoSetCBCInfo(sockFD, phoneFlag, RecvBuff, buffLen); 
			break;
		case MES_SET_WIFI_CONF:        // ����wifi������Ϣ
			dealWithSetWifiConf(sockFD, phoneFlag, RecvBuff, buffLen);
			break;
		
		default:
			memset(RecvBuff, 'a', 1024);
			nRet = Procotol::AddPkgHeadAndTail(RecvBuff, 1024, MES_SND_HEART, 1, 1);
			Write(sockFD, RecvBuff, nRet, 3000);
		}
		if (pBuff)
		{
			delete [] pBuff;
			pBuff = NULL;
		}
	}

	if (RecvBuff)delete [] RecvBuff, RecvBuff = NULL;

	UninitTypeIDList(IDHead);
	if (NULL != IDHead) delete IDHead, IDHead = NULL;

	// ����phoneFlag
	ClosePhoneHandle(phoneFlag);

	// �ر� socket
	Close(sockFD);
	return (void*)NULL;
}


void* 
localServerPthreadProc(void* argc)
{
	int sockFd;
	IbeaconConfig *conf = GetConfigPosition();
	
	// ����������
	CSocketTCP* serverTCP = new CSocketTCP(conf->getLocalPort());
	sockFd = serverTCP->StartServer(conf->getListenNum());
	if (sockFd <= 0)
	{
		write_error_log("Start Lock server Failed. sockFd=%d", sockFd);
		return (void*)NULL;
	}

	// ���ó�ʱ
	struct timeval tv;
	tv.tv_sec  = DEF_RCVTIMEO / 1000;
	tv.tv_usec = DEF_RCVTIMEO % 1000;
	(void)setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	tv.tv_sec  = DEF_SNDTIMEO / 1000;
	tv.tv_usec = DEF_SNDTIMEO % 1000;
	(void)setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

	for (;;)
	{
		pthread_t clientID;
		int AcceptFD = 0;
		struct sockaddr_in ClientAddr;
		memset(&ClientAddr, 0, sizeof(struct sockaddr));
		int AddrLen = sizeof(struct sockaddr);
		int *SerParam = NULL;
		
		usleep(1000);
		if ( -1 == (AcceptFD = serverTCP->Accept(sockFd, (struct sockaddr*)&ClientAddr, &AddrLen)) )
		{
			//write_error_log("acceptFD=%d", AcceptFD);
			continue;
		}

		// Ϊÿ�������������Ӷ˴���һ���̣߳�������Կ���ʹ���̳߳�
		SerParam = new int;
		*SerParam = AcceptFD;
		if ( 0 != pthread_create(&clientID, 0, ClientPthreadProc, (void*)SerParam) )
		{
			write_error_log("Create Pthread Failed.");
		}
		pthread_detach(clientID);
	}

	if (NULL != serverTCP)
	{
		delete serverTCP;
		serverTCP = NULL;
	}
	
	Close(sockFd);
	return (void*)NULL;
}


void 
parsePhoneKey(char* key, const char* mac)
{
	unsigned int i, j, k;
	char temp[13] = {0};

	// ȥ��ð��
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
	// ������key
	k = base64_decode(key, dest, 64);

	for (j=0, i=0; i<k; ++i, ++j)
	{
		key[i] = dest[i] - temp[j];
		j = j % 12;
	}
	key[i] = 0;
	return ;
}


int 
ProvePhoneConnect(int sockFd, bool prove, int phoneHandle, char* key)
{
	char RecvBuff[256] = {0};
	int buffLen, nRet; 
	u_int16 typeID = 0;
	u_int8 PkgTotal = 0, PkgSeq = 0;
	cJSON* root;
	//char key[128] = {0};
	char* out = NULL;
	IbeaconConfig *conf = GetConfigPosition();

	nRet = Procotol::RecvOnePkg(sockFd, RecvBuff, 256, 60*1000);
	if (0 > nRet) return -1;

	// δ�󶨣�ֱ�ӷ���
	if (!prove) 
	{
		strncpy(RecvBuff, RETURN_JSON_2, 256);
		nRet = Procotol::AddPkgHeadAndTail(RecvBuff, sizeof(RETURN_JSON_2)-1, MES_PHO_SND_KEY_ACK, 1, 1);
		Write(sockFd, RecvBuff, nRet, 3000);
		return 0;
	}
	buffLen = Procotol::DelPkgHeadAndTail(RecvBuff, nRet-TAIL_LEN, &typeID, &PkgTotal, &PkgSeq);
	if (0 > buffLen || typeID != MES_PHO_SND_KEY) return -1;

	// �� RecvBuff ����ȡ�� key ���˺���
	root = cJSON_Parse(RecvBuff);
	strncpy(key, cJSON_GetStringItem(root, "phoneKey"), 128);
	parsePhoneKey(key, conf->getWanMAC());    	      // ����key
	cJSON_DeleteItemFromObject(root, "phoneKey");
	cJSON_AddStringToObject(root, "phoneKey", key);
	cJSON_AddNumberToObject(root, "phoneHandle", phoneHandle);
	out = cJSON_Print(root);
	cJSON_Delete(root), root = NULL;


	// �ͷ�������֤key
	if (0 > WriteDataToNet(out, strlen(out), MES_VFY_KEY_SER, 1, 0)) 
	{
		free(out), out = NULL;
		// ʹ�ñ���key������֤
		if ( 0 == strncmp(conf->getPhoneKey(), key, PHONE_KEY_LEN) ) 
			return 0;
		else 
		{
			strncpy(RecvBuff, RETURN_JSON_2, 256);
			nRet = Procotol::AddPkgHeadAndTail(RecvBuff, sizeof(RETURN_JSON_2)-1, MES_PHO_SND_KEY_ACK, 1, 1);
			Write(sockFd, RecvBuff, nRet, 3000);
			return -1;
		}
	}
	else
	{
		free(out), out = NULL;
		// �ȴ��������ķ��أ�����10�룬��Ϊ���ִ���
		if ( 0 != PhoneServerReadDataFromNet(&out, &buffLen, phoneHandle, &typeID, 10*1000))
		{
			return -1;
		}
		else
		{
			if (typeID != MES_VFY_KEY_SER_ACK) return -1;
			// ��������������֤���
			root = cJSON_Parse(out);
			nRet = cJSON_GetNumberItem(root, "returnVal");
			cJSON_Delete(root);

			if (nRet == 1) 
				return 0;
			else 
			{
				strncpy(RecvBuff, RETURN_JSON_3, 256);
				nRet = Procotol::AddPkgHeadAndTail(RecvBuff, sizeof(RETURN_JSON_3)-1, MES_PHO_SND_KEY_ACK, 1, 1);
				Write(sockFd, RecvBuff, nRet, 3000);
				return -1;
			}
		}
	}
}


