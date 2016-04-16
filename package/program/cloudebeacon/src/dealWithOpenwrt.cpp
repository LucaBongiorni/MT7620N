#ifdef USE_OPENWRT

#include "defCom.h"
#include "pthreadCom.h"
#include "cbProjMain.h"
#include "procotol.h"
#include "socketClient.h"
#include "cJSON.h"
#include "confile.h"
#include "base64RSA.h"
#include "UCI_File.h"
#include "netPing.h"
#include "pthreadServer.h"
#include "dealWithOpenwrt.h"


//extern cloudBeaconMain *g_main;
//static IbeaconConfig* g_config = g_main->m_config;

char* 
getWiFiSSID()
{
	static char ssid[128];
	const char *pstr = NULL;
	UCI *uci = new UCI();
	
	memset(ssid, 0, sizeof(ssid));
	uci->UCI_LoadFile(WiFi_Conf_File_Path);
	pstr = uci->UCI_GetOptionValue("@wifi-iface[0]", "ssid");
	if (pstr)
	{
		strncpy(ssid, pstr, sizeof(ssid));
		delete uci, uci = NULL;
		return ssid;
	}
	if (uci) delete uci;
	return NULL;
}

int 
checkWiFiCliSsidConnect()
{
	UCI *uci = NULL;
	const char *pstr = NULL;
	char ssid[128] = {0};
	char SSID[128] = {0};
	int nRet;
	char *Output = NULL;
	char* ptemp = NULL;
	char* pVal;

	uci = new UCI();
	if (!uci) return -1;
	uci->UCI_LoadFile(WiFi_Conf_File_Path);
	pstr = uci->UCI_GetOptionValue("@wifi-iface[0]", "ApCliSsid");
	strncpy(ssid, pstr, sizeof(ssid));
	delete uci, uci = NULL;

	Output = (char*)malloc(4096);
	if (!Output) return -1;
	ptemp = Output;
	nRet = GetShellCmdOutput(GET_WIFI_AP_INFO, Output, 4096);
	if (nRet > 20)
	{
		// 跳过头部信息
		for(;;)
		{
			if (strncmp(ptemp, "Ch", 2) == 0)
			{
				while(*ptemp++ != '\n');
				break;
			}
			while(*ptemp != '\n' && *ptemp != 0)++ptemp; ++ptemp;
		}

		// 提取信息
		for (;;)
		{
			if (*ptemp == 0) break;
			pVal = ptemp;
			while(*ptemp != '\n' && *ptemp != 0)++ptemp; *ptemp++ = 0;
			nRet = sscanf(pVal, "%*d %s %*s %*s %*d %*s %*s %*s", SSID);
			if (1 != nRet) break;

			if (strcmp(SSID, ssid) == 0) 
			{
				if (Output) free(Output), Output = NULL;
				return 0;
			}
		}	
	}
	
	if (Output) free(Output), Output = NULL;
	return -1;
}


// 函数功能: 切换为wifi上网
// 函数参数: 空
// 返 回 值: 成功返回0，失败返回-1
int
switchWiFiNetwork()
{
	u_int32 tempIP = 0;
	UCI* uci = NULL;
	
	// 查看是否已经获取ip
	tempIP = GetIPByIfname("apcli0");
	if (tempIP > 0) return 0;

	// 配置network
	uci = new UCI(Network_Conf_File, true);
	uci->UCI_SetOptionValue("wan", "ifname", "apcli0");
	uci->UCI_SetOptionValue("wan", "proto", "dhcp");
	uci->UCI_SetOptionValue("@switch_vlan[0]", "ports", "1 2 3 4 6t");
	uci->UCI_SetOptionValue("@switch_vlan[1]", "ports", "0 6t");
	uci->UCI_DelOption("wan", "maxwait");
	uci->UCI_DelOption("wan", "device");
	uci->UCI_DelOption("wan", "service");
	uci->UCI_DelOption("wan", "apn");
	uci->UCI_DelOption("wan", "username");
	uci->UCI_DelOption("wan", "password");
	uci->UCI_DelOption("wan", "dialnumber");
	uci->UCI_Commit();
	delete uci, uci = NULL;
	
	return 0;
}

// 函数功能: 切换为3G上网
// 函数参数: type, 3/联通，2/移动，1/电信
// 返 回 值: 成功返回0，无法识别3G卡托返回-2，无法是被usb设备返回-3
int 
switch3GNetwork(int type)
{
	UCI* uci = NULL;
	int nRet;
	int returnVal = 0;
	char buff[1024] = {0};
	u_int32 tempIP = 0;

	if (type < 1 || type > 3) return -1;
	// 查看是否已经获取ip
	tempIP = GetIPByIfname("3g-wan");
	if (tempIP > 0)
	{
		goto setError;    // 已经通过3g上网，无需再操作。
	}
	// 是否识别usb设备
	if (access("/dev/ttyUSB0", F_OK) != 0)
	{
		returnVal = -3;   // 无法识别usb设备
		goto setError;   
	}
	// 是否识别 3G 卡
	nRet = GetShellCmdOutput("/sbin/usbmode -l", buff, 1024);
	if (nRet < 5)
	{
		returnVal = -2;
		goto setError;
	}

	uci = new UCI();
	uci->UCI_LoadFile(Network_Conf_File);
	uci->UCI_SetOptionValue("wan", "ifname", "eth0.2");
	uci->UCI_SetOptionValue("wan", "proto", "3g");
	uci->UCI_SetOptionValue("wan", "maxwait", "1");
	uci->UCI_SetOptionValue("wan", "device", "/dev/ttyUSB0");
	switch(type)
	{
	case 1:         // 电信
		uci->UCI_SetOptionValue("wan", "service", "evdo");
		uci->UCI_SetOptionValue("wan", "apn", "ctnet");
		uci->UCI_SetOptionValue("wan", "username", "ctnet@mycdma.cn");
		uci->UCI_SetOptionValue("wan", "password", "vnet.mobi");
		uci->UCI_SetOptionValue("wan", "dialnumber", "#777");
		break;
	case 2:         // 移动
		uci->UCI_SetOptionValue("wan", "service", "umts");
		uci->UCI_SetOptionValue("wan", "apn", "cmnet/cmwap");
		uci->UCI_SetOptionValue("wan", "dialnumber", "*99***1#");
		uci->UCI_DelOption("wan", "username");
		uci->UCI_DelOption("wan", "password");
		break;
	case 3:         // 联通
		uci->UCI_SetOptionValue("wan", "service", "umts");
		uci->UCI_SetOptionValue("wan", "apn", "cmnet");
		uci->UCI_SetOptionValue("wan", "dialnumber", "*99#");
		uci->UCI_DelOption("wan", "username");
		uci->UCI_DelOption("wan", "password");
		break;
	}
	uci->UCI_SetOptionValue("@switch_vlan[0]", "ports", "1 2 3 4 6t");
	uci->UCI_SetOptionValue("@switch_vlan[1]", "ports", "0 6t");
	uci->UCI_Commit();

setError:
	if (uci) delete uci;
	return returnVal;
}

// 函数功能: 切换为网线上网
// 函数参数: 空
// 返 回 值: 成功返回0，失败返回-1
int 
switchReticleNetwork()
{
	u_int32 tempIP = 0;
	UCI* uci = NULL;
	
	// 查看是否已经获取ip
	tempIP = GetIPByIfname("eth0.2");
	if (tempIP > 0)return 0;

	// 配置network
	uci = new UCI(Network_Conf_File, true);
	uci->UCI_SetOptionValue("wan", "ifname", "eth0.2");
	uci->UCI_SetOptionValue("wan", "proto", "dhcp");
	uci->UCI_DelOption("wan", "maxwait");
	uci->UCI_DelOption("wan", "device");
	uci->UCI_DelOption("wan", "service");
	uci->UCI_DelOption("wan", "apn");
	uci->UCI_DelOption("wan", "username");
	uci->UCI_DelOption("wan", "password");
	uci->UCI_DelOption("wan", "dialnumber");

	uci->UCI_SetOptionValue("@switch_vlan[0]", "ports", "0 1 2 3 6t");
	uci->UCI_SetOptionValue("@switch_vlan[1]", "ports", "4 6t");
	
	uci->UCI_Commit();
	delete uci, uci = NULL;
	
	return 0;
}

void wanReconnectNetwork()
{
	char cmd[1024] = {0};
	//strncpy(cmd, "/etc/init.d/usbmode restart", 1024);
	//system(cmd);
	strncpy(cmd, "/etc/init.d/network restart", 1024);
	sleep(1);
	system(cmd);
	strncpy(cmd, "/etc/init.d/dnsmasq restart", 1024);
	system(cmd);
	strncpy(cmd, "/etc/init.d/dropbear restart", 1024);
	system(cmd);
}

// 手机获取cloudbeacon的配置信息 
void 
GetCBCInfo(int sockFD,  u_int16 typeID, char* RecvBuff, int buffLen)
{
	cJSON *root = NULL, *MacSer = NULL, *BeaconSer = NULL, *Wifi = NULL;
	IbeaconConfig* g_config = GetConfigPosition();
	char* out = NULL;
	const char *WifiChannel, *WifiApCliEnable, *WifiApCliSsid, *WifiApCliAuthMode, *WifiApCliEncrypType, *WifiApCliPassWord;

	RecvBuff[buffLen] = 0;
	root = cJSON_Parse(RecvBuff);
	if (!root)
	{
		Debug(I_ERROR, "JSON Parse Failed...");
		return;
	}
	int GetMacSerInfo = cJSON_GetNumberItem(root, "GetMacSerInfo");
	int GetBeaconSerInfo = cJSON_GetNumberItem(root, "GetBeaconSerInfo");
	int GetWifiInfo = cJSON_GetNumberItem(root, "GetWifiInfo");
	int GetInterval = cJSON_GetNumberItem(root, "GetUploadIntervalInfo");
	cJSON_Delete(root), root = NULL;

	root = cJSON_CreateObject();
	if (1 == GetMacSerInfo)
	{
		cJSON_AddItemToObject(root, "GetMacSerInfo", MacSer = cJSON_CreateObject());
		if (g_config->getIsOpenMacSer())
		{
			cJSON_AddNumberToObject(MacSer, "MacSerOpen", 1);
			cJSON_AddStringToObject(MacSer, "MacSerHost", g_config->getMacSerHost());
			cJSON_AddStringToObject(MacSer, "MacSerUrl", g_config->getMacSerUrl());
			cJSON_AddNumberToObject(MacSer, "MacSerPort", g_config->getMacSerPort());
			cJSON_AddNumberToObject(MacSer, "MacSerInterval", g_config->getMacInterval());
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
		if (g_config->getIsOpenBeaconSer())
		{
			cJSON_AddNumberToObject(BeaconSer, "BeaconSerOpen", 1);
			cJSON_AddStringToObject(BeaconSer, "BeaconSerHost", g_config->getBeaconSerHost());
			cJSON_AddStringToObject(BeaconSer, "BeaconSerUrl", g_config->getBeaconSerUrl());
			cJSON_AddNumberToObject(BeaconSer, "BeaconSerPort", g_config->getBeaconSerPort());
			cJSON_AddNumberToObject(BeaconSer, "BeaconSerInterval", g_config->getBeaconInterval());
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
		// 获取wifi配置的基本信息
		UCI *uci = new UCI();
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

#ifdef OLD_VERSION
	cJSON *Interval = NULL, *OtherInfo = NULL;
	unsigned long IP = 0;
	char* temp = NULL;
	if (1 == GetInterval)
	{
		cJSON_AddItemToObject(root, "GetUploadIntervalInfo", Interval = cJSON_CreateObject());
		cJSON_AddNumberToObject(Interval, "MacSerInterval", g_config->getMacInterval());
		cJSON_AddNumberToObject(Interval, "BeaconSerInterval", g_config->getBeaconInterval());
	}
	cJSON_AddItemToObject(root, "GetOtherInfo", OtherInfo = cJSON_CreateObject());
	cJSON_AddNumberToObject(OtherInfo, "USB_DownLoadPort", HTTP_PORT);
	IP = GetWanIP();
	temp = inet_ntoa(*(struct in_addr*)&IP);
	cJSON_AddStringToObject(OtherInfo, "USB_DownLoadHost", temp);
	cJSON_AddStringToObject(OtherInfo, "USB_DownLoadUrl", USB_DOWNLOAD_URL);
#endif
	out = cJSON_Print(root);
	cJSON_Delete(root);

	sendDataByTCP(sockFD, out, strlen(out),  typeID);
	CJSONFree(out), out = NULL;
	return ;
}


void 
SetCBCInfo(int sockFD, int phoneHandle, char* RecvBuff, int buffLen)
{
	cJSON* root, *MacSer, *BeaconSer;
	//cJSON* vRoot;
	IbeaconConfig* g_config = GetConfigPosition();
	char* out = NULL;
	char *host, *url;
	int port, interval, outLen, nRet;
	int SetBeaconSerInfoVal=1, SetMacSerInfoVal=1;
	//u_int16 typeID;
	CConfFile* confFile;
	char temp[16] = {0};

	//printf("recvBuff=%s\n", RecvBuff);
	root = cJSON_Parse(RecvBuff);
	if (!root)
	{
		Debug(I_ERROR, "JSON Parse Failed...");
		SetMacSerInfoVal = SetBeaconSerInfoVal = -31;
		goto dealWithFailed;
	}
	MacSer = cJSON_GetObjectItem(root, "SetMacSerInfo");
	BeaconSer = cJSON_GetObjectItem(root, "SetBeaconSerInfo"); 

	// 加载配置文件到内存
	confFile = new CConfFile();
	g_config->lockConfFile();
	if (!confFile->LoadFile(g_config->getConfFilePath()))
	{
		Debug(I_DEBUG, "Open config file %s failure.", g_config->getConfFilePath());
		g_config->UnlockConfFile();

		SetMacSerInfoVal = SetBeaconSerInfoVal = -32;
		goto dealWithFailed;
	}
	g_config->UnlockConfFile();

#if 0
	// 先和服务器同步
	if (NULL != MacSer || NULL != BeaconSer)
	{
		cJSON_AddNumberToObject(root, "phoneHandle", phoneHandle);
		out = cJSON_Print(root);
		printf("out=%s\n", out);
		// 转发给服务器
		if ( -1 == WriteDataToNet(out, strlen(out), MES_SYN_CBC_CONF, 1, 0))
		{
			if (out) CJSONFree(out), out = NULL;
			SetMacSerInfoVal = SetBeaconSerInfoVal = -33;
			goto dealWithFailed;
		}
		if (out) CJSONFree(out), out = NULL;
		
		// 从服务器读取返回值
		nRet = PhoneServerReadDataFromNet(&out, &outLen, phoneHandle, &typeID, 10000);
		if (nRet == -1 || typeID != MES_SYN_CBC_CONF_ACK)
		{
			SetMacSerInfoVal = SetBeaconSerInfoVal = -33;
			goto dealWithFailed;
		}
		// 解析和服务器同步的结果
		out[outLen] = 0;
		vRoot = cJSON_Parse(out);
		SetMacSerInfoVal = cJSON_GetNumberItem(vRoot, "SetMacSerInfoVal");
		SetBeaconSerInfoVal = cJSON_GetNumberItem(vRoot, "SetBeaconSerInfoVal");
		cJSON_Delete(vRoot);
		if (SetMacSerInfoVal == 1) SetMacSerInfoVal = -34;
		if (SetBeaconSerInfoVal == 1) SetBeaconSerInfoVal = -34;
	}
#else
	SetMacSerInfoVal = SetBeaconSerInfoVal = 0;
#endif	

	// 修改配置信息
	if (0 == SetMacSerInfoVal && NULL != MacSer)
	{
		// 服务器已经更新，配置文件和程序配置也要更新
		if ( 1 == cJSON_GetNumberItem(MacSer, "MacSerOpen"))
		{
			host = cJSON_GetStringItem(MacSer, "MacSerHost");
			url = cJSON_GetStringItem(MacSer, "MacSerUrl");
			port = cJSON_GetNumberItem(MacSer, "MacSerPort");
			interval = cJSON_GetNumberItem(MacSer, "MacSerInterval");

			if (!host || !url || port <=0 || interval < 0)
			{
				SetMacSerInfoVal = -31;
				goto setbeacon;
			}

			// 修改配置
			g_config->setIsOpenMacSer(true);
			
			// 配置文件
			temp[0] = '1', temp[1] = 0;
			confFile->SetValue("CustomServerInfo", "OpenMacSer", temp, "#开启Mac服务器");
			confFile->SetValue("CustomServerInfo", "MacSerHost", host, "#客户Mac服务器地址");
			confFile->SetValue("CustomServerInfo", "MacSerUrl",  url, "#客户Mac服务器url");
			snprintf(temp, 16, "%d", port);
			confFile->SetValue("CustomServerInfo", "MacSerPort", temp, "#客户Mac服务器端口");
			snprintf(temp, 16, "%d", interval);
			confFile->SetValue("CustomServerInfo", "MacSerInterval", temp, "#客户Mac服务器上传间隔时间(单位:s)");
			
			g_config->setMacInterval(interval);
			g_config->setMacSerHost(host);
			g_config->setMacSerUrl(url);
			g_config->setMacSerPort(port);
		}
		else
		{
			g_config->setIsOpenMacSer(false);
			// 配置文件
			temp[0] = '0', temp[1] = 0;
			confFile->SetValue("CustomServerInfo", "OpenMacSer", temp, "#关闭Mac服务器");
		}	
	}
setbeacon:
	if (0 == SetBeaconSerInfoVal && NULL != BeaconSer)
	{
		// 服务器已经更新，配置文件和程序配置也要更新
		if ( 1 == cJSON_GetNumberItem(BeaconSer, "BeaconSerOpen"))
		{
			// 更新程序信息
			host = cJSON_GetStringItem(BeaconSer, "BeaconSerHost");
			url = cJSON_GetStringItem(BeaconSer, "BeaconSerUrl");
			port = cJSON_GetNumberItem(BeaconSer, "BeaconSerPort");
			interval = cJSON_GetNumberItem(BeaconSer, "BeaconSerInterval");

			if (!host || !url || port <=0 || interval < 0)
			{
				SetBeaconSerInfoVal = -31;
				goto dealWithFailed;
			}
			
			// 修改配置
			g_config->setIsOpenBeaconSer(true);
			g_config->setBeaconSerHost(host);
			g_config->setBeaconSerUrl(url);
			g_config->setBeaconSerPort(port);
			g_config->setBeaconInterval(interval);
			
			// 更新配置文件内容
			temp[0] = '1', temp[1] = 0;
			confFile->SetValue("CustomServerInfo", "OpenBeaconSer", temp, "#开启Beacon服务器");
			confFile->SetValue("CustomServerInfo", "BeaconSerHost", host, "#客户Beacon服务器地址");
			confFile->SetValue("CustomServerInfo", "BeaconSerUrl",  url, "#客户Beacon服务器url");
			snprintf(temp, 16, "%d", port);
			confFile->SetValue("CustomServerInfo", "BeaconSerPort", temp, "#客户Beacon服务器端口");
			snprintf(temp, 16, "%d", interval);
			confFile->SetValue("CustomServerInfo", "BeaconSerInterval", temp, "#客户Beacon服务器上传间隔时间(单位:s)");
		}
		else
		{
			// 同步程序数据
			g_config->setIsOpenBeaconSer(false);
			// 更新配置文件
			temp[0] = '0', temp[1] = 0;
			confFile->SetValue("CustomServerInfo", "OpenBeaconSer", temp, "#关闭Beacon服务器");
		}
	}
	
	g_config->lockConfFile();
	confFile->Save();
	g_config->UnlockConfFile();

dealWithFailed:
	cJSON_Delete(root);
	delete confFile, confFile = NULL;
	
	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "SetMacSerInfoVal", SetMacSerInfoVal);
	cJSON_AddNumberToObject(root, "SetBeaconSerInfoVal", SetBeaconSerInfoVal);
	out = cJSON_Print(root);
	cJSON_Delete(root);

	sendDataByTCP(sockFD, out, strlen(out), MES_PHO_SET_CBC_INFO_ACK);
	CJSONFree(out), out = NULL;
	return;
}


void
GetCBNetSta(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
	char* out = GetAllAvaildNetInfo();
	sendDataByTCP(sockFD, out, strlen(out), typeID);
	if (out) free(out), out = NULL;
}


void
SetCBConnectNetByReticle(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
	int nRet;
	int returnVal = 0;
	returnVal = switchReticleNetwork();
	nRet = snprintf(RecvBuff, 256, "{\"returnVal\": %d}", returnVal);
	sendDataByTCP(sockFD, RecvBuff, nRet, typeID);
	// 查看是否重启网络
	if (0 == returnVal) wanReconnectNetwork();
}

void
SetCBConnectNetBy3G(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
	int nRet;
	int returnVal = 0;
	int type;
	cJSON *root = NULL;

	RecvBuff[buffLen] = 0;
	root = cJSON_Parse(RecvBuff);
	if (! root ) 
	{
		returnVal = -1;
		goto sendPkg;
	}
	type = cJSON_GetNumberItem(root, "3G-type");
	nRet = switch3GNetwork(type);
	if (-1 == nRet) 
		returnVal = -290;
	else if (-3 == nRet) 
		returnVal = -291;
	else if (-2 == nRet) 
		returnVal = -292;
	cJSON_Delete(root), root = NULL;

sendPkg:	
	nRet = snprintf(RecvBuff, 256, "{\"returnVal\": %d}", returnVal);
	sendDataByTCP(sockFD, RecvBuff, nRet, typeID);
	// 查看是否重启网络
	if (0 == returnVal) wanReconnectNetwork();
}


void
SetWifiConf(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
	// 解析出配置参数
	int WifiApCliEnable = 0;
	char *WifiApCliSsid = NULL;
	char *WifiApCliAuthMode = NULL;
	char *WifiApCliEncrypType = NULL;
	char *WifiApCliPassWord = NULL;
	UCI *uci = NULL;
	cJSON *root = NULL;
	char *WifiApSSID = NULL;
	char *WifiApPasswd = NULL;
	int WifiChannel;
	char tmp[128] = {0};
	int returnVal = 0;
	int nRet;
	
	RecvBuff[buffLen] = 0;
	root = cJSON_Parse(RecvBuff);
	if (!root)
	{
		Debug(I_ERROR, "JSON Parse Failed...");
		returnVal = -1;
		goto sendPkg;
	}
	
	uci = new UCI(WiFi_Conf_File_Path, true);
	// 设置wifi sta信息
	WifiApCliEnable = cJSON_GetNumberItem(root, "WifiApCliEnable");
	if (WifiApCliEnable == 1)
	{
		WifiChannel = cJSON_GetNumberItem(root, "WifiChannel");
		WifiApCliSsid = cJSON_GetStringItem(root, "WifiApCliSsid");
		WifiApCliAuthMode = cJSON_GetStringItem(root, "WifiApCliAuthMode");
		WifiApCliEncrypType = cJSON_GetStringItem(root, "WifiApCliEncrypType");
		WifiApCliPassWord = cJSON_GetStringItem(root, "WifiApCliPassWord");

		snprintf(tmp, sizeof(tmp), "%d", WifiApCliEnable);
		uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliEnable", tmp);
		if (1 <= WifiChannel && 13 >= WifiChannel)
		{
			snprintf(tmp, sizeof(tmp), "%d", WifiChannel);
			uci->UCI_SetOptionValue("ra0", "channel", tmp);
		}
		else
			uci->UCI_SetOptionValue("ra0", "channel", "auto");

		if (WifiApCliSsid)
			uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliSsid", WifiApCliSsid);
		if (WifiApCliAuthMode)
		{
			if (strcmp(WifiApCliAuthMode, "WPA2PSK") == 0 ||
				strcmp(WifiApCliAuthMode, "WPAPSK") == 0 ||
				strcmp(WifiApCliAuthMode, "WEP") == 0 ||
				strcmp(WifiApCliAuthMode, "NONE") == 0)
				uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliAuthMode", WifiApCliAuthMode);
			else
			{
				returnVal = -170;
				goto sendPkg;
			}	
		}
		if (WifiApCliEncrypType)
		{
			if (strcmp(WifiApCliEncrypType, "AES") == 0  ||
				strcmp(WifiApCliEncrypType, "TKIP") == 0 || 
				strcmp(WifiApCliEncrypType, "WEP") == 0  ||
				strcmp(WifiApCliEncrypType, "NONE")== 0 )	
				uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliEncrypType", WifiApCliEncrypType);
			else
			{
				returnVal = -171;
				goto sendPkg;
			}
		}
		if (WifiApCliPassWord && strlen(WifiApCliPassWord) >= 8)
			uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliPassWord", WifiApCliPassWord);
		else
		{
			returnVal = -172;
			goto sendPkg;
		}
	}
	else
	{
		snprintf(tmp, sizeof(tmp), "%d", 0);
		uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliEnable", tmp);
	}

	// 设置wifi热点信息
	WifiApSSID   = cJSON_GetStringItem(root, "WifiApSsid");
	WifiApPasswd = cJSON_GetStringItem(root, "WifiApPasswd");
	if (WifiApSSID)
		uci->UCI_SetOptionValue("@wifi-iface[0]", "ssid", WifiApSSID);
	if (WifiApPasswd)
	{
		if (strlen(WifiApPasswd) >= 8)
			uci->UCI_SetOptionValue("@wifi-iface[0]", "key", WifiApPasswd);
		else
		{
			returnVal = -173;
			goto sendPkg;
		}
	}
	uci->UCI_Commit();
	if (uci) delete uci, uci = NULL;

	// 切换为wifi联网模式
	switchWiFiNetwork();
	cJSON_Delete(root), root = NULL;

sendPkg:
	nRet = snprintf(RecvBuff, 256, "{\"returnVal\": %d}", returnVal);
	sendDataByTCP(sockFD, RecvBuff, nRet, typeID);

	if (root) cJSON_Delete(root), root = NULL;
	if (uci) delete uci, uci = NULL;
	if (returnVal == 0) wanReconnectNetwork();  // 网络重启
}


void 
CheckWifiAPInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
	int nRet;
	char* pBuff = (char*)malloc(4096);
	char* ptemp = pBuff;
	const char* pVal;
	int CH, Siganl;
	char SSID[64], BSSID[18], Security[32], W_Mode[16], ExtCH[8], NT[8];
	cJSON* root = NULL, *apInfo = NULL;
	char* out = NULL;

	root = cJSON_CreateArray();
	memset(pBuff, 0, 4096);
	nRet = GetShellCmdOutput(GET_WIFI_AP_INFO, pBuff, 4096);
	if (nRet > 20)
	{
		// 跳过头部信息
		for(;;)
		{
			if (strncmp(ptemp, "Ch", 2) == 0)
			{
				while(*ptemp++ != '\n');
				break;
			}
			while(*ptemp != '\n' && *ptemp != 0)++ptemp; ++ptemp;
		}

		// 提取信息
		for (;;)
		{
			if (*ptemp == 0) break;
			memset(SSID, 0, sizeof(SSID));
			memset(BSSID, 0, sizeof(BSSID));
			memset(Security, 0, sizeof(Security));
			memset(W_Mode, 0, sizeof(W_Mode));
			memset(ExtCH, 0, sizeof(ExtCH));
			memset(NT, 0, sizeof(NT));
			
			pVal = ptemp;
			while(*ptemp != '\n' && *ptemp != 0)++ptemp; *ptemp++ = 0;
			nRet = sscanf(pVal, "%d %s %s %s %d %s %s %s", 
				&CH, SSID, BSSID, Security, &Siganl, W_Mode, ExtCH, NT);
			if (8 != nRet) break;

			// 保存到json中
			cJSON_AddItemToArray(root, apInfo = cJSON_CreateObject());
			cJSON_AddNumberToObject(apInfo, "Ch", CH);
			cJSON_AddStringToObject(apInfo, "SSID", SSID);
			cJSON_AddStringToObject(apInfo, "BSSID", BSSID);
			cJSON_AddStringToObject(apInfo, "Security", Security);
			cJSON_AddNumberToObject(apInfo, "Siganl", Siganl);
			cJSON_AddStringToObject(apInfo, "W_Mode", W_Mode);
			cJSON_AddStringToObject(apInfo, "ExtCH", ExtCH);
			cJSON_AddStringToObject(apInfo, "NT", NT);
		}	
	}

	out = cJSON_Print(root);
	//printf("----------len=%d, wifi info=%s\n", strlen(out)+21, out);
	sendDataByTCP(sockFD, out, strlen(out), typeID);
	cJSON_Delete(root);
	if (out) free(out), out = NULL;
	if (pBuff) free(pBuff), pBuff = NULL;
	return;	
}

void
SetPasswd(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
	if (! RecvBuff || buffLen <= 0) return;

	cJSON* root = NULL;
	char* newPasswd = NULL;
	char* orgPasswd = NULL;
	int returnVal = 0;
	char cmd[128] = {0};
	int nRet;

	RecvBuff[buffLen] = 0;
	root = cJSON_Parse(RecvBuff);
	if (! root )
	{
		Debug(I_ERROR, "cJSON Parse Failed.");
		returnVal = -1;
		goto sendPackage;
	}

	newPasswd = cJSON_GetStringItem(root, "newPasswd");
	orgPasswd = cJSON_GetStringItem(root, "orgPasswd");
	if ( 0 != checkLoginPasswd("root", orgPasswd) )
	{
		Debug(I_ERROR, "cJSON Parse Failed.");
		returnVal = -192;
		goto sendPackage;
	}

	snprintf(cmd, sizeof(cmd), 
		"( echo %s; echo %s ) | passwd >/dev/null 2>&1", newPasswd, newPasswd);
	system(cmd);
	
sendPackage:
	nRet = snprintf(RecvBuff, 256, "{\"returnVal\": %d}", returnVal);
	sendDataByTCP(sockFD, RecvBuff, nRet, typeID);
	cJSON_Delete(root), root = NULL;
}


void
CheckSDInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
	char tmp[64] = {0};
	char cmd[256] = {0};
	char Output[1024] = {0};
	char i, j;
	int nRet;
	//int returnVal = 0;
	cJSON *root = NULL, *usbInfo = NULL, *couldnotRecUSB = NULL, *tmpCJSON = NULL;
	char Filesystem[64], Size[16], Used[16], Available[16], UseP[8], MountedOn[64];
	char *out = NULL;
	
	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "usbInfo", usbInfo=cJSON_CreateArray());
	cJSON_AddItemToObject(root, "couldnotRecUSB", couldnotRecUSB=cJSON_CreateArray());
	for (i='a'; i<='z'; ++i)
	{
		for (j='1'; j<='9'; ++j)
		{
			snprintf(tmp, sizeof(tmp), "/dev/sd%c%c", i, j);
			if (0 == access(tmp, F_OK))
			{
				memset(Output, 0, 1024);
				memset(Filesystem, 0, sizeof(Filesystem));
				memset(Size, 0, sizeof(Size));
				memset(Used, 0, sizeof(Used));
				memset(Available, 0, sizeof(Available));
				memset(UseP, 0, sizeof(UseP));
				memset(MountedOn, 0, sizeof(MountedOn));
			
				snprintf(cmd, sizeof(cmd), "df -h | grep %s", tmp);
				nRet = GetShellCmdOutput(cmd, Output, 1024);
				if (nRet > 10)
				{
					nRet = sscanf(Output, "%s %s %s %s %s %s", 
						Filesystem, Size, Used, Available, UseP, MountedOn);
					if (nRet != 6) break;
					
					cJSON_AddItemToArray(usbInfo, tmpCJSON = cJSON_CreateObject());
					cJSON_AddStringToObject(tmpCJSON, "Filesystem", Filesystem);
					cJSON_AddStringToObject(tmpCJSON, "mountOn", MountedOn);
					cJSON_AddStringToObject(tmpCJSON, "totalSize", Size);
					cJSON_AddStringToObject(tmpCJSON, "availableSize", Available);
					cJSON_AddStringToObject(tmpCJSON, "usedSize", Used);
					cJSON_AddStringToObject(tmpCJSON, "use%", UseP);
				}
				else
				{
					// 无法挂载
					cJSON_AddItemToArray(couldnotRecUSB, cJSON_CreateString(tmp));
				}
			}
			else 
			{
				break;
			}
		}
	}

	out = cJSON_Print(root);
	sendDataByTCP(sockFD, out, strlen(out), typeID);
	if (out) free(out);
	cJSON_Delete(root), root = NULL;
}

void
CheckMemFlashSysInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
#define outBuffSize   1024
	cJSON *root = NULL;
	cJSON *memoryInfo = NULL, *flashInfo = NULL, *systemInfo = NULL;
	int nRet;
	float runTime, freeTime, loadavg;
	unsigned int temp;
	char *ptmp, *out = NULL;
	int total, freeS, buffers;
	int ubootSize, ubootEveSize, factorySize, firmwareSize, flashSize;
	char size[16];
	char rootfsTotal[8], rootfsUsed[8], rootfsFree[8], romfsSize[8];
	char machine[32];
	FILE* pfile = NULL;
	char *outBuff = (char*)malloc(outBuffSize);

	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "memoryInfo", memoryInfo=cJSON_CreateObject());
	cJSON_AddItemToObject(root, "flashInfo", flashInfo=cJSON_CreateObject());
	cJSON_AddItemToObject(root, "systemInfo", systemInfo=cJSON_CreateObject());

	if ((pfile = fopen("/proc/meminfo", "r")) != NULL)
	{
		memset(outBuff, 0, outBuffSize);
		while (!feof(pfile) && fgets(outBuff, outBuffSize, pfile)) 
		{
			if (sscanf(outBuff, "MemTotal: %d", &total) == 1)
			{
				cJSON_AddNumberToObject(memoryInfo, "total", total);
			}
			else if (sscanf(outBuff, "MemFree: %d", &freeS) == 1)
			{
				cJSON_AddNumberToObject(memoryInfo, "free", freeS);
			}
			else if (sscanf(outBuff, "Buffers: %d", &buffers) == 1)
			{
				cJSON_AddNumberToObject(memoryInfo, "buffers", buffers);
			}
			else ;
			memset(outBuff, 0, outBuffSize);
		}

		cJSON_AddNumberToObject(memoryInfo, "used", total-freeS);
		fclose(pfile), pfile = NULL;
	}

	if ((pfile = fopen("/proc/mtd", "r")) != NULL)
	{
		while (!feof(pfile) && fgets(outBuff, outBuffSize, pfile)) 
		{
			if (strstr(outBuff, "u-boot") && (!strstr(outBuff, "u-boot-env")))
			{
				sscanf(outBuff, "%*s %s", size);
				ubootSize = (StrHexToNumDec(size, strlen(size))) / 1024;
				cJSON_AddNumberToObject(flashInfo, "ubootSize", ubootSize);
			}
			else if (strstr(outBuff, "u-boot-env"))
			{
				sscanf(outBuff, "%*s %s", size);
				ubootEveSize = (StrHexToNumDec(size, strlen(size))) / 1024;
				cJSON_AddNumberToObject(flashInfo, "ubootEveSize", ubootEveSize);
			}
			else if (strstr(outBuff, "factory"))
			{
				sscanf(outBuff, "%*s %s", size);
				factorySize = StrHexToNumDec(size, strlen(size)) / 1024;
				cJSON_AddNumberToObject(flashInfo, "factorySize", factorySize);
			}
			else if (strstr(outBuff, "firmware"))
			{
				sscanf(outBuff, "%*s %s", size);
				firmwareSize = (StrHexToNumDec(size, strlen(size))) / 1024;
				cJSON_AddNumberToObject(flashInfo, "firewareSize", firmwareSize);
			}
			else
			{
				;
			}
			memset(outBuff, 0, outBuffSize);
			memset(size, 0, sizeof(size));
		}
		fclose(pfile), pfile = NULL;
	}
	flashSize = (ubootSize + ubootEveSize + factorySize + firmwareSize)/1024;
	cJSON_AddNumberToObject(flashInfo, "totalSize", flashSize);

	memset(outBuff, 0, outBuffSize);
	nRet = GetShellCmdOutput("df -h", outBuff, outBuffSize);
	if (nRet > 10)
	{
		memset(rootfsTotal, 0, sizeof(rootfsTotal));
		memset(rootfsUsed,  0, sizeof(rootfsUsed));
		memset(rootfsFree,  0, sizeof(rootfsFree));
		memset(romfsSize,   0, sizeof(romfsSize));
		ptmp = strstr(outBuff, "rootfs");
		nRet = sscanf(ptmp, "%*s %s %s %s %*s %*s", rootfsTotal, rootfsUsed, rootfsFree);
		if (nRet == 3)
		{
			cJSON_AddStringToObject(flashInfo, "rootfsTotal", rootfsTotal);
			cJSON_AddStringToObject(flashInfo, "rootfsUsed", rootfsUsed);
			cJSON_AddStringToObject(flashInfo, "rootfsFree", rootfsFree);
		}

		ptmp = strstr(outBuff, "/dev/root");
		nRet = sscanf(ptmp, "%*s %s %*s %*s %*s %*s", romfsSize);
		if (nRet == 1)
		{
			cJSON_AddStringToObject(flashInfo, "romfsSize", romfsSize);
		}
	}

	if ((pfile = fopen("/proc/version", "r")) != NULL)
	{
		fscanf(pfile, "Linux version %s", outBuff);
		cJSON_AddStringToObject(systemInfo, "kernelVer", outBuff);
		fclose(pfile), pfile = NULL;
	}
	
	memset(outBuff, 0, outBuffSize);
	nRet = GetShellCmdOutput("uname -n", outBuff, outBuffSize);
	if (nRet > 0)
	{
		ptmp = outBuff;
		while(*ptmp != '\n' && *ptmp != 0) ++ptmp; *ptmp = 0;
		cJSON_AddStringToObject(systemInfo, "systemName", outBuff);
	}
	
	if ((pfile = fopen("/proc/cpuinfo", "r")) != NULL)
	{
		while (!feof(pfile)) 
		{
			if (fscanf(pfile, "machine : %s", machine) == 0)
				while (!feof(pfile) && fgetc(pfile) != '\n');
			else 
			{
				cJSON_AddStringToObject(systemInfo, "machine", machine);
				break;
			}
		}
		fclose(pfile), pfile = NULL;
	}
	
	memset(outBuff, 0, outBuffSize);
	nRet = GetShellCmdOutput("date \"+%Y-%m-%d %H:%M:%S\"", outBuff, outBuffSize);
	if (nRet > 0)
	{
		ptmp = outBuff;
		while(*ptmp != '\n' && *ptmp != 0) ++ptmp; *ptmp = 0;
		cJSON_AddStringToObject(systemInfo, "localTime", outBuff);
	}

	if ((pfile = fopen("/proc/uptime", "r")) != NULL)
	{
		fscanf(pfile, "%f %f", &runTime, &freeTime);
		temp = runTime;
		cJSON_AddNumberToObject(systemInfo, "runTime", temp);
		temp = freeTime;
		cJSON_AddNumberToObject(systemInfo, "freeTime", temp);
		fclose(pfile), pfile = NULL;
	}

	if ((pfile=fopen("/proc/loadavg", "r")) != NULL)
	{
		fscanf(pfile, "%f %f %f", &runTime, &freeTime, &loadavg);
		snprintf(outBuff, 1024, "%.2f, %.2f, %.2f", runTime, freeTime, loadavg);
		cJSON_AddStringToObject(systemInfo, "loadAverage", outBuff);
		fclose(pfile), pfile = NULL;
	}
	
	out = cJSON_Print(root);
	sendDataByTCP(sockFD, out, strlen(out), typeID);
	cJSON_Delete(root);
	if (out) free(out);
	if (outBuff) free(outBuff);
}


void
CheckArpInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
	FILE* pfile = NULL;
	char IPAddr[18], Flags[4], HWAddr[32], Dev[16];
	cJSON *root = NULL, *arp = NULL;
	char* out = NULL;

	root = cJSON_CreateArray();
	if ((pfile = fopen("/proc/net/arp", "r")) != NULL)
	{
		while (!feof(pfile) && fgetc(pfile) != '\n');  // 跳过第一行
		while (!feof(pfile) && 
			(fscanf(pfile, "%15[0-9.] %*s %s %17[A-Fa-f0-9:] %*s %s", IPAddr, Flags, HWAddr, Dev) == 4))
		{
			cJSON_AddItemToArray(root, arp = cJSON_CreateObject());
			cJSON_AddStringToObject(arp, "IPAddress", IPAddr);
			cJSON_AddNumberToObject(arp, "Flags", StrHexToNumDec(Flags, strlen(Flags)));
			cJSON_AddStringToObject(arp, "MacAddress", HWAddr);
			cJSON_AddStringToObject(arp, "Dev", Dev);
			while (!feof(pfile) && fgetc(pfile) != '\n');
		}
		fclose(pfile), pfile=NULL;
	}

	out = cJSON_Print(root);
	sendDataByTCP(sockFD, out, strlen(out), typeID);
	cJSON_Delete(root);
	free(out);
}


void
CheckDHCPInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
	FILE* pfile = NULL;
	time_t Leasetime;
	char IPAddr[18], HWAddr[32], hostName[128];
	cJSON *root = NULL, *dhcp = NULL;
	char *out = NULL;

	root = cJSON_CreateArray();
	if ((pfile = fopen("/var/dhcp.leases", "r")) != NULL)
	{
		while (!feof(pfile) && 
			(fscanf(pfile, "%ld %17[A-Fa-f0-9:] %15[0-9.] %s", &Leasetime, HWAddr, IPAddr, hostName) == 4))
		{
			cJSON_AddItemToArray(root, dhcp = cJSON_CreateObject());
			cJSON_AddNumberToObject(dhcp, "LeasetimeRemaining", Leasetime - time(0));
			cJSON_AddStringToObject(dhcp, "IPv4-Daddress", IPAddr);
			cJSON_AddStringToObject(dhcp, "MAC-Address", HWAddr);
			cJSON_AddStringToObject(dhcp, "Hostname", hostName);
			while (!feof(pfile) && fgetc(pfile) != '\n');
		}
		fclose(pfile), pfile = NULL;
	}

	out = cJSON_Print(root);
	sendDataByTCP(sockFD, out, strlen(out), typeID);
	cJSON_Delete(root);
	free(out);
}


void
CheckPSInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
	char *Output = NULL, *out = NULL;
	const int OutputLen = 4096;
	int nRet;
	char* ptemp = NULL;
	char* pVal = NULL;
	char* pLine = NULL;
	cJSON* root = NULL, *info = NULL;
	char USER[32] = {0};
	char STAT[16] = {0};
	char COMMAND[256] = {0};
	int PID, VSZ;
	
	root = cJSON_CreateArray();
	Output = (char*)malloc(OutputLen);
	if (!Output)
	{
		goto sendPkg;
	}
	nRet = GetShellCmdOutput("/bin/ps", Output, OutputLen);
	if (nRet == -1)
	{
		goto sendPkg;
	}
	
	pLine = strstr(Output, "PID");
	if (pLine)
		while(*pLine++ != '\n');     // 跑到下一行
	else
		goto sendPkg;

	for (;;)
	{
		pVal = pLine;
		while(*pLine != '\n' && *pLine)++pLine; *pLine++ = 0;
		if (pLine - Output >= nRet) break;

		while(isspace(*pVal))++pVal; ptemp = pVal;
		while(!isspace(*ptemp))++ptemp; *ptemp++ = 0;
		PID = atoi(pVal);

		pVal = ptemp;
		while(isspace(*pVal))++pVal; ptemp = pVal;
		while(!isspace(*ptemp))++ptemp; *ptemp++ = 0;
		strncpy(USER, pVal, sizeof(USER));

		pVal = ptemp;
		while(isspace(*pVal))++pVal; ptemp = pVal;
		while(!isspace(*ptemp))++ptemp; *ptemp++ = 0;
		VSZ = atoi(pVal);

		pVal = ptemp;
		while(isspace(*pVal))++pVal; ptemp = pVal;
		while(!isspace(*ptemp))++ptemp; *ptemp++ = 0;
		strncpy(STAT, pVal, sizeof(STAT));

		pVal = ptemp;
		while(isspace(*pVal))++pVal; ptemp = pVal; 
		strncpy(COMMAND, pVal, sizeof(COMMAND));

		//printf("PID=%d, USER=%s, VSZ=%d, STAT=%s, COMMAND=%s\n", PID, USER, VSZ, STAT, COMMAND);
		//nRet = sscanf(pVal, " %d %s %d %s %s", &PID, USER, &VSZ, STAT, COMMAND);
		//if (5 != nRet) break;

		info = cJSON_CreateObject();
		cJSON_AddNumberToObject(info, "PID", PID);
		cJSON_AddStringToObject(info, "USER", USER);
		cJSON_AddNumberToObject(info, "VSZ", VSZ);
		cJSON_AddStringToObject(info, "STAT", STAT);
		cJSON_AddStringToObject(info, "COMMAND", COMMAND);
		cJSON_AddItemToArray(root, info);
	}
	
sendPkg:
	if (Output) free(Output), Output = NULL;
	out = cJSON_Print(root);
	sendDataByTCP(sockFD, out, strlen(out), typeID);
	cJSON_Delete(root), root = NULL;
	if (out) CJSONFree(out), out = NULL;
}

void
GetCBLog(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
	char *out = NULL;
	int returnVal = 0;
	int nRet;
	int fileSize;
	char *pbuff = NULL;
	cJSON* root = NULL;

	if (0 != access(LOG_FILE_PATH, F_OK))
	{
		returnVal = -330;
		goto sendPkg;
	}
	pbuff = LoadFileToMem(LOG_FILE_PATH, &fileSize);
	if (!pbuff)
	{
		returnVal = -331;
		goto sendPkg;
	}
	
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "log", pbuff);
	if (pbuff) free(pbuff), pbuff = NULL;
	cJSON_AddNumberToObject(root, "returnVal", 0);
	out = cJSON_PrintUnformatted(root);
	if (root) cJSON_Delete(root), root = NULL;
	
sendPkg:
	if (returnVal != 0)
	{
		nRet = snprintf(RecvBuff, 256, "{\"returnVal\": %d}", returnVal);
		sendDataByTCP(sockFD, RecvBuff, nRet, typeID);
	}
	else
	{
		sendDataByTCP(sockFD, out, strlen(out), typeID);
		if (out) CJSONFree(out), out = NULL;
	}
}


void 
CheckNetStation(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
#define DEFIPSIZE  (sizeof("255.255.255.255"))
	const char* pTaobao = "www.taobao.com";
	const char* pDNS = "114.114.114.114";
	const char* plo  = "127.0.0.1";
	char gateway[DEFIPSIZE] = {0};
	int returnVal = 0;
	unsigned long temp = 0;
	cJSON* root = NULL;
	UCI *uci = NULL;
	const char* ifname = NULL;
	int netType = 0;
	char* out = NULL;

	// 获取上网方式
	uci = new UCI(Network_Conf_File, true);
	ifname = uci->UCI_GetOptionValue("wan", "ifname");
	if (strcmp(ifname, "apcli0") == 0)
		netType = 2;
	else if (strcmp(ifname, "eth0.2") == 0)
		netType = 1;
	else if (strcmp(ifname, "3g-wan") == 0)
		netType = 3;
	else
		netType = 0;
	delete uci, uci = NULL;

	if (0 == ping_main(pTaobao, 0))
	{
		goto sendPkg;
	}
	if (0 == ping_main(pDNS, 0))
	{
		returnVal = -350;
		goto sendPkg;
	}

	temp = GetRandWanGateway();
	strncpy(gateway, inet_ntoa(*(struct in_addr*)&temp), sizeof(gateway));
	//printf("------------------gateway=%s\n", gateway);
	if (0 == ping_main(gateway, 0))
	{
		returnVal = -351;
		goto sendPkg;
	}
	if (0 == ping_main(plo, 0))
	{
		returnVal = -352;
		goto sendPkg;
	}
	returnVal = -353;
	
sendPkg:
	root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "returnVal", returnVal);
	cJSON_AddNumberToObject(root, "netType", netType);
	out = cJSON_Print(root);
	if (out)
	{
		sendDataByTCP(sockFD, out, strlen(out), typeID);
		CJSONFree(out), out = NULL;
	}
	if (root)cJSON_Delete(root);
}

void 
GetCBAllInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
	char *out = NULL;
	IbeaconConfig* conf = GetConfigPosition();
	cJSON* root = NULL, *blueUpdateInfo = NULL;
	cJSON *POSTSerInfo = NULL, *TCPSerInfo = NULL, *cloudbeacon = NULL;
	cJSON *updateInfo = NULL;
	cJSON *blueInfo = NULL;
	char* publicKey = NULL;
	int size, nRet;
	char tmp[512] = {0};
	
	root = cJSON_CreateObject();
	POSTSerInfo = cJSON_CreateObject();
	TCPSerInfo = cJSON_CreateObject();
	cloudbeacon = cJSON_CreateObject();
	blueUpdateInfo = cJSON_CreateObject();
	updateInfo = cJSON_CreateObject();
	blueInfo = cJSON_CreateObject();

	if (updateInfo)
	{
		CConfFile* confFile = new CConfFile();
		if (confFile->LoadFile(UPDATE_CONF_FILE))
		{
			confFile->GetString("UpdateInfo", "Host", tmp);
			cJSON_AddStringToObject(updateInfo, "Host", tmp);
			nRet = confFile->GetInt("UpdateInfo", "Port");
			cJSON_AddNumberToObject(updateInfo, "Port", nRet);
			confFile->GetString("UpdateInfo", "URL", tmp);
			cJSON_AddStringToObject(updateInfo, "URL", tmp);
			nRet = confFile->GetInt("UpdateInfo", "HardwareType");
			cJSON_AddNumberToObject(updateInfo, "HardwareType", nRet);
			nRet = confFile->GetInt("UpdateInfo", "UpdateVersion");
			cJSON_AddNumberToObject(updateInfo, "UpdateVersion", nRet);
			confFile->GetString("UpdateInfo", "UpdateFilePath", tmp);
			cJSON_AddStringToObject(updateInfo, "UpdateFilePath", tmp);	
		}
		cJSON_AddItemToObject(root, "updateInfo", updateInfo);
		delete confFile, confFile = NULL;
	}
	
	//Post 服务器信息
	if (POSTSerInfo)
	{
		cJSON_AddBoolToObject(POSTSerInfo,   "BeaconPostOpen", conf->getIsOpenBeaconSer());
		cJSON_AddStringToObject(POSTSerInfo, "BeaconPostHost", conf->getBeaconSerHost());
		cJSON_AddNumberToObject(POSTSerInfo, "BeaconPostPort", conf->getBeaconSerPort());
		cJSON_AddStringToObject(POSTSerInfo, "BeaconPostUrl", conf->getBeaconSerUrl());
		cJSON_AddNumberToObject(POSTSerInfo, "BeaconPostInterval", conf->getBeaconInterval());
		cJSON_AddBoolToObject(POSTSerInfo,   "MacPostOpen", conf->getIsOpenMacSer());
		cJSON_AddStringToObject(POSTSerInfo, "MacPostHost", conf->getMacSerHost());
		cJSON_AddNumberToObject(POSTSerInfo, "MacPostPort", conf->getMacSerPort());
		cJSON_AddStringToObject(POSTSerInfo, "MacPostUrl", conf->getMacSerUrl());
		cJSON_AddNumberToObject(POSTSerInfo, "MacPostInterval", conf->getMacInterval());
		cJSON_AddItemToObject(root, "POSTSerInfo", POSTSerInfo);
	}
	//tcp 服务器信息
	if (TCPSerInfo)
	{
		cJSON_AddStringToObject(TCPSerInfo, "BeaconTCPHost", conf->getWebDomain());
		cJSON_AddNumberToObject(TCPSerInfo, "BeaconTCPPort", conf->getWebPort());
		cJSON_AddBoolToObject(TCPSerInfo, "BeaconTCPOpen", conf->getTCPBeaconSerOpenVal());
		cJSON_AddNumberToObject(TCPSerInfo, "BeaconTCPInterval", conf->getBeaconInterval());
		cJSON_AddBoolToObject(TCPSerInfo, "BeaconTCPOpen", conf->getTCPMacSerOpenVal());
		cJSON_AddNumberToObject(TCPSerInfo, "MacTCPInterval", conf->getMacInterval());
		cJSON_AddItemToObject(root, "TCPSerInfo", TCPSerInfo);
	}
	if (blueUpdateInfo)
	{
		cJSON_AddStringToObject(blueUpdateInfo, "blueUpdateHost", conf->getUpdateBlueBinHost());
		cJSON_AddNumberToObject(blueUpdateInfo, "blueUpdatePort", conf->getUpdateBlueBinPort());
		cJSON_AddStringToObject(blueUpdateInfo, "blueUpdateUrl", conf->getUpdateBlueBinUrl());
		cJSON_AddItemToObject(root, "blueUpdateInfo", blueUpdateInfo);
	}
	
	// cloudbeacon 本身信息
	if (cloudbeacon)
	{
		cJSON_AddNumberToObject(cloudbeacon, "advPort", conf->getBcastPort());
		cJSON_AddStringToObject(cloudbeacon, "phoneKey", conf->getPhoneKey());
		cJSON_AddStringToObject(cloudbeacon, "COM", conf->getComName());
		cJSON_AddStringToObject(cloudbeacon, "serials", conf->getSerials());
		cJSON_AddNumberToObject(cloudbeacon, "LocalTCPPort", conf->getLocalPort());
		cJSON_AddNumberToObject(cloudbeacon, "LocalTCPListen", conf->getListenNum());
		cJSON_AddBoolToObject(cloudbeacon, "BeaconScanDevice", conf->getIsStartScanDev());
		cJSON_AddBoolToObject(cloudbeacon, "isCloudbeaconBind", conf->ifBeenActivated());
		cJSON_AddStringToObject(cloudbeacon, "configFilePath", conf->getConfFilePath());
		snprintf(tmp, sizeof(tmp), "%s %s",  __VERDATA__, __VERTIME__);
		cJSON_AddStringToObject(cloudbeacon, "makeTime", tmp);
		
		publicKey = LoadFileToMem(DEF_PUBLIC_KEY_FILE, &size);
		if (publicKey)
		{
			cJSON_AddStringToObject(cloudbeacon, "publicKey", publicKey);
			free(publicKey), publicKey = NULL;
		}
		publicKey = LoadFileToMem(DEF_UID_FILE_PATH, &size);
		if (publicKey)
		{
			cJSON_AddStringToObject(cloudbeacon, "ssid", publicKey);
			free(publicKey), publicKey = NULL;
		}
		
		cJSON_AddItemToObject(root, "cloudbeacon", cloudbeacon);
	}

	if (blueInfo)
	{
		BleCentral *central = getCentralPosition();
		cJSON_AddNumberToObject(blueInfo, "BlueToothType", central->GetCentralType());
		cJSON_AddNumberToObject(blueInfo, "BlueToothVer", central->GetFirmwareVer());	
		cJSON_AddItemToObject(root, "blueInfo", blueInfo);
	}
	
	// 发送数据
	out = cJSON_Print(root);
	cJSON_Delete(root);
	sendDataByTCP(sockFD, out, strlen(out), typeID);
	CJSONFree(out), out = NULL;
}




void 
dealWithRecvPkg(int sockFD, char* RecvBuff, int buffLen, 
	u_int16 typeID, int phoneFlag, const char* key)
{
	int nRet;
	// RecvBuff 接收到的字符，已经转换，buffLen 接收到的字符长度
	switch(typeID)
	{
	case MES_RST_BLU_DEV ... MES_SET_BLU_INFO_ACK: // 转发给串口
		// 发送数据给串口
		WriteDataToSerials(RecvBuff, buffLen, typeID, phoneFlag, false);
		break;

	case MES_PHO_GET_CBC_INFO:     // 获取配置信息
		Debug(I_DEBUG, "---------------get cloudbeacon info------------");
		GetCBCInfo(sockFD, MES_PHO_GET_CBC_INFO_ACK, RecvBuff, buffLen);
		break;
	case MES_PHO_SET_CBC_INFO:     // 设置配置信息
		Debug(I_DEBUG, "---------------set cloudbeacon info------------");
		SetCBCInfo(sockFD, phoneFlag, RecvBuff, buffLen); 
		break;
	case MES_PHO_BIND_CBC:         // 手机绑定设备
		Debug(I_DEBUG, "-----------bind cloudbeacon--------key=%s\n", key);
		dealWithPhoBindCBC(sockFD, phoneFlag, RecvBuff, buffLen, key);
		break;
	case MES_PHO_UNBIND_CBC:       // 解绑设备
		Debug(I_DEBUG, "----------unbind cloudbeacon--------key=%s\n", key);
		dealWithPhoUnbindCBC(sockFD, phoneFlag, RecvBuff, buffLen, key);
		break;
	case MES_GET_CB_IFNET_STA:      // 获取cloudbeacon 的网络状态
		Debug(I_DEBUG, "----------get cloudbeacon net station----------");
		GetCBNetSta(sockFD, MES_GET_CB_IFNET_STA_ACK, RecvBuff, buffLen);
		break;
	case MES_SET_CB_CON_BY_RTC:     // 使用网线上网
		Debug(I_DEBUG, "----------set cloudbeacon net station----------");
		SetCBConnectNetByReticle(sockFD, MES_SET_CB_CON_BY_RTC_ACK, RecvBuff, buffLen);
		break;
	case MES_CHE_WIFI_AP:          // 手机获取cloudbeacon周围wifi热点的基本信息
		Debug(I_DEBUG, "-----------check wifi ap info--------");
		CheckWifiAPInfo(sockFD, MES_CHE_WIFI_AP_ACK, RecvBuff, buffLen);
		break;		
	case MES_SET_WIFI_CONF:        // 配置wifi基本信息
		Debug(I_DEBUG, "---------------set wifi conf------------");
		SetWifiConf(sockFD, MES_SET_WIFI_CONF_ACK, RecvBuff, buffLen);
		break;
	case MES_PHO_SND_KEY:          // 发送key，按道理不会有的
		nRet = snprintf(RecvBuff, 256, "{\"returnVal\": %d}", -7);
		nRet = Procotol::AddPkgHeadAndTail(RecvBuff, nRet, MES_PHO_SND_KEY_ACK, 1, 1);
		Write(sockFD, RecvBuff, nRet, 3000);
		break;
	case MES_SET_ROOT_PASSWD:      // 设置新的密码
		Debug(I_DEBUG, "-----------set passwd----------------");
		SetPasswd(sockFD, MES_SET_ROOT_PASSWD_ACK, RecvBuff, buffLen);
		break;
	case MES_CHK_SD_INFO:          // 查看U盘挂载情况
		Debug(I_DEBUG, "-----------check sd info-------------");
		CheckSDInfo(sockFD, MES_CHK_SD_INFO_ACK, RecvBuff, buffLen);
		break;
	case MES_CHK_MEM_FLASH_SYS:    // 查看系统信息
		Debug(I_DEBUG, "---check memory/flash/system info----");
		CheckMemFlashSysInfo(sockFD, MES_CHK_MEM_FLASH_SYS_ACK, RecvBuff, buffLen);
		break;
	case MES_CHK_ARP_INFO:         // 查看arp表信息
		Debug(I_DEBUG, "-----------check arp info------------");
		CheckArpInfo(sockFD, MES_CHK_ARP_INFO_ACK, RecvBuff, buffLen);
		break;
	case MES_CHK_DHCP_INFO:        // 查看dhcp表信息
		Debug(I_DEBUG, "-----------check DHCP info----------");
		CheckDHCPInfo(sockFD, MES_CHK_DHCP_INFO_ACK, RecvBuff, buffLen);
		break;
	case MES_SET_3G_NET:           // 使用3G上网
		Debug(I_DEBUG, "-------get 3g net info-----");
		SetCBConnectNetBy3G(sockFD, MES_SET_3G_NET_ACK, RecvBuff, buffLen);
		break;
	case MES_CHK_PS_INFO:          // 查看正在运行的程序
		Debug(I_DEBUG, "-------check cloudbeacon running program--------");
		CheckPSInfo(sockFD, MES_CHK_PS_INFO_ACK, RecvBuff, buffLen);
		break;
	case MES_GET_CB_LOG:           // 查看运行日记
		Debug(I_DEBUG, "-------get cloudbeacon running log--------");
		GetCBLog(sockFD, MES_GET_CB_LOG_ACK, RecvBuff, buffLen);
		break;
	case MES_CHK_CB_NET_STA:      // 
		Debug(I_DEBUG, "-------check cloudbeacon net station--------");
		CheckNetStation(sockFD, MES_CHK_CB_NET_STA_ACK, RecvBuff, buffLen);
		break;

	////////////////////////////////////////////////////////////////////////
	case MES_GET_CB_ALL_INFO:
		Debug(I_DEBUG, "-------Get cloudbeacon all info--------");
		GetCBAllInfo(sockFD, MES_GET_CB_ALL_INFO_ACK, RecvBuff, buffLen);
		break;
	case MES_UPL_PHO_MAC_INFO_ACK: // 暂时不用
		break;
	default:
		Debug(I_DEBUG, "recv error package.typeID=%04x", typeID);
	}
}



void UpdateCBProgram(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen)
{
#define DOWNLOAD_UPDATEFILE_CMD "wget -t5 -T30 -c %s -O %s"
	cJSON *root = NULL;
	char* filePath = NULL;
	char cmd[1024] = {0};
	int returnVal = 0;
	int nRet = 0;

	root = cJSON_Parse(RecvBuff);
	if (!root)
	{
		returnVal = 303;
		goto sendPkg;
	}
	filePath = cJSON_GetStringItem(root, "FilePath");

	// 拼接下载命令
	snprintf(cmd, sizeof(cmd), "wget -t5 -T30 -c %s -O /tmp/cloudbeacon.tar.gz", filePath);
	system(cmd);
	sleep(1);

	// 检查文件
	if (access("/tmp/cloudbeacon.tar.gz", F_OK) != 0)
	{
		returnVal = 301;
		goto sendPkg;
	}

	system("cd /tmp; tar -zxvf cloudbeacon.tar.gz; cd -");
	usleep(500);

	if (access("/tmp/rootfs/usr/ibeacon/bin/cloudbeacon", F_OK) != 0)
	{
		returnVal = 302;
		goto sendPkg;
	}
	
sendPkg:
	if (root) cJSON_Delete(root), root = NULL;
	nRet = snprintf(cmd, sizeof(cmd), "{\"returnVal\": %d}", returnVal);
	sendDataByTCP(sockFD, cmd, nRet, typeID);
	if (returnVal == 0)
	{
		// 执行更新脚本
		system("sh /usr/ibeacon/tool/updateCB.sh");
	}
}






char* Get_AroundAPInfo()
{
	int nRet;
	char* pBuff = (char*)malloc(4096);
	char* ptemp = pBuff;
	const char* pVal;
	int CH, Siganl;
	char SSID[64], BSSID[18], Security[32], W_Mode[16], ExtCH[8], NT[8];
	cJSON* root = NULL, *apInfo = NULL;
	char* out = NULL;

	root = cJSON_CreateArray();
	memset(pBuff, 0, 4096);
	nRet = GetShellCmdOutput(GET_WIFI_AP_INFO, pBuff, 4096);
	if (nRet > 20)
	{
		// 跳过头部信息
		for(;;)
		{
			if (strncmp(ptemp, "Ch", 2) == 0)
			{
				while(*ptemp++ != '\n');
				break;
			}
			while(*ptemp != '\n' && *ptemp != 0)++ptemp; ++ptemp;
		}

		// 提取信息
		for (;;)
		{
			if (*ptemp == 0) break;
			memset(SSID, 0, sizeof(SSID));
			memset(BSSID, 0, sizeof(BSSID));
			memset(Security, 0, sizeof(Security));
			memset(W_Mode, 0, sizeof(W_Mode));
			memset(ExtCH, 0, sizeof(ExtCH));
			memset(NT, 0, sizeof(NT));
			
			pVal = ptemp;
			while(*ptemp != '\n' && *ptemp != 0)++ptemp; *ptemp++ = 0;
			nRet = sscanf(pVal, "%d %s %s %s %d %s %s %s", 
				&CH, SSID, BSSID, Security, &Siganl, W_Mode, ExtCH, NT);
			if (8 != nRet) break;

			// 保存到json中
			cJSON_AddItemToArray(root, apInfo = cJSON_CreateObject());
			cJSON_AddNumberToObject(apInfo, "Ch", CH);
			cJSON_AddStringToObject(apInfo, "SSID", SSID);
			cJSON_AddStringToObject(apInfo, "BSSID", BSSID);
			cJSON_AddStringToObject(apInfo, "Security", Security);
			cJSON_AddNumberToObject(apInfo, "Siganl", Siganl);
			cJSON_AddStringToObject(apInfo, "W_Mode", W_Mode);
			cJSON_AddStringToObject(apInfo, "ExtCH", ExtCH);
			cJSON_AddStringToObject(apInfo, "NT", NT);
		}	
	}

	out = cJSON_Print(root);
	cJSON_Delete(root);
	if (pBuff) free(pBuff), pBuff = NULL;
	return out;	
}


    int  secMode;       // security mode: 0 - NONE
                        //                1 - WEP
                        //                2 - WPAPSK
                        //                3 - WPA2PSK
    int  encrypType;    // encrypt mode:  0 - NONE
                        //                1 - WEP
                        //                2 - TKIP
                        //                3 - AES

#define WIFI_SEC_MODE_COUNT 		4
static const char *wifiSecModes[WIFI_SEC_MODE_COUNT] = {
    "NONE",
    "WEP",
    "WPAPSK",
    "WPA2PSK",
};

#define WIFI_ENCRYPT_TYPE_COUNT 	4
static const char *wifiEncryptTypes[WIFI_ENCRYPT_TYPE_COUNT] = {
    "NONE",
    "WEP",
    "WPAPSK",
    "WPA2PSK",
};

int Get_StaInfo(WiFiInfo* info)
{
	const char* ssid, *key, *channel, *secMode, *encrypType, *enable;
	UCI *uci = NULL;

	if (!info) return -1;
	memset(info, 0, sizeof(WiFiInfo));

	uci = new UCI(WiFi_Conf_File_Path, true);
    if (!uci) return -1;

	enable      = uci->UCI_GetOptionValue("@wifi-iface[0]", "ApCliEnable");
	ssid        = uci->UCI_GetOptionValue("@wifi-iface[0]", "ApCliSsid");
	key         = uci->UCI_GetOptionValue("@wifi-iface[0]", "ApCliPassWord");
	secMode     = uci->UCI_GetOptionValue("@wifi-iface[0]", "ApCliAuthMode");
	encrypType  = uci->UCI_GetOptionValue("@wifi-iface[0]", "ApCliEncrypType");
	channel     = uci->UCI_GetOptionValue("ra0", "channel");

    info->enable = (enable && (*enable == '1'));
	if (ssid)
		strncpy(info->ssid, ssid, sizeof(info->ssid));
	if (key)
		strncpy(info->key, key, sizeof(info->key));
	if (channel)
        info->channel = atoi(channel);

	if (secMode)
    {
        for (int i = 0; i <= WIFI_SEC_MODE_COUNT; i++)
        {
            if (strcmp(wifiSecModes[i], secMode) == 0)
            {
                info->secMode = i;
                break;
            }
        }
    }
	if (encrypType)
    {
        for (int i = 0; i <= WIFI_ENCRYPT_TYPE_COUNT; i++)
        {
            if (strcmp(wifiEncryptTypes[i], encrypType) == 0)
            {
                info->encrypType = i;
                break;
            }
        }
    }

	if (uci) delete uci, uci = NULL;
	return 0;
}

int Set_StaInfo(WiFiInfo* info)
{
	const char* ssid, *key, *channel, *secMode, *encrypType;
    char szChannel[32] = {0};
	UCI *uci = NULL;
	if (!info) return -1;

	uci = new UCI(WiFi_Conf_File_Path, true);
	if (!uci) return -1;

	if (!info->enable)
	{
		if (-1 == uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliEnable", "0"))
			goto setStaFailed;

        uci->UCI_Commit();
        if (uci) delete uci, uci = NULL;
        switchWiFiNetwork();     // 切换wifi连接外网
        wanReconnectNetwork();   // 重启网络
        return 0;
	}

	if (-1 == uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliEnable", "1"))
		goto setStaFailed;
	if (-1 == uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliSsid", info->ssid))
		goto setStaFailed;
	if (-1 == uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliPassWord", info->key))
		goto setStaFailed;

    sprintf(szChannel, "%d", info->channel);
	if (-1 == uci->UCI_SetOptionValue("ra0", "channel", szChannel))
		goto setStaFailed;

    if (info->secMode < 0 || info->secMode >= WIFI_SEC_MODE_COUNT)
        goto setStaFailed;
	if (-1 == uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliAuthMode", wifiSecModes[info->secMode]))
		goto setStaFailed;

    if (info->encrypType < 0 || info->encrypType >= WIFI_ENCRYPT_TYPE_COUNT)
        goto setStaFailed;
	if (-1 == uci->UCI_SetOptionValue("@wifi-iface[0]", "ApCliEncrypType", wifiEncryptTypes[info->encrypType]))
		goto setStaFailed;

	uci->UCI_Commit();
	if (uci) delete uci, uci = NULL;
	switchWiFiNetwork();     // 切换wifi连接外网
	wanReconnectNetwork();   // 重启网络
	return 0;
	
setStaFailed:
	if (uci) delete uci, uci = NULL;
	return -1;
}


int Get_APInfo(WiFiInfo* info)
{
	const char* ssid, *key, *disable;
	UCI *uci = NULL;

	if (!info) return -1;
	memset(info, 0, sizeof(WiFiInfo));

	uci = new UCI(WiFi_Conf_File_Path, true);
	if (!uci) return -1;

	ssid    = uci->UCI_GetOptionValue("@wifi-iface[0]", "ssid");
	key     = uci->UCI_GetOptionValue("@wifi-iface[0]", "key");
	disable = uci->UCI_GetOptionValue("ra0", "disabled");
	
	if (ssid)
		strncpy(info->ssid, ssid, sizeof(info->ssid));
	if (key)
		strncpy(info->key, key, sizeof(info->key));
    info->enable = (!disable || (*disable == '0'));

	if (uci) delete uci, uci = NULL;
	return 0;
}

int Set_APInfo(WiFiInfo* info)
{
	UCI *uci = NULL;
	if (!info) return -1;

	uci = new UCI(WiFi_Conf_File_Path, true);
	if (!uci) return -1;

	if (!info->enable)
	{
		if (-1 == uci->UCI_SetOptionValue("ra0", "disabled", "1"))
            goto setApFailed;

        uci->UCI_Commit();
        delete uci, uci = NULL;
        wanReconnectNetwork();   // 重启网络
        return 0;
	}
	
    if (-1 == uci->UCI_SetOptionValue("ra0", "disabled", "0"))
		goto setApFailed;
	if (-1 == uci->UCI_SetOptionValue("@wifi-iface[0]", "ssid", info->ssid))
		goto setApFailed;
	if (-1 == uci->UCI_SetOptionValue("@wifi-iface[0]", "key", info->key))
		goto setApFailed;
	
	
	uci->UCI_Commit();
	delete uci, uci = NULL;
	wanReconnectNetwork();   // 重启网络
	return 0;

setApFailed:
	delete uci, uci = NULL;
	return -1;
}

#endif /*USE_OPENWRT*/

