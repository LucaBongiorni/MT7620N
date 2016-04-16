#ifndef __DEALWITH_OPENWRT_H__
#define __DEALWITH_OPENWRT_H__

#ifdef USE_OPENWRT

#include "defCom.h"


// ��������: ��ȡwifi��ssid 
// ��������: ��
// �� �� ֵ: �ɹ����ػ�ȡ����ssid��ʧ�ܷ���NULL.
char* 
getWiFiSSID();
// ��������: ���apcli��ssid�����Ƿ����
// ��������: ��
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1
int 
checkWiFiCliSsidConnect();
// ��������: �л�Ϊwifi����
// ��������: ��
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1
int
switchWiFiNetwork();
// ��������: �л�Ϊ3G����
// ��������: type, 3/��ͨ��2/�ƶ���1/����
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1
int 
switch3GNetwork(int type);
// ��������: �л�Ϊ��������
// ��������: ��
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1
int 
switchReticleNetwork();
// ��������: wan����������
// ��������: ��
// �� �� ֵ: ��
void 
wanReconnectNetwork();



/*{
	"GetMacSerInfo": {
		"MacSerOpen":int,        # �����û�Mac��������0�رգ�1��ʼ
		"MacSerHost":"string",   # Mac ��������ַ
		"MacSerUrl":"string",    # Mac ������url
		"MacSerPort": int,       # Mac �������˿�
		"MacSerInterval":int     # Mac ���������ʱ��
	}
	"GetBeaconSerInfo": {
		"BeaconSerOpen":int,         # �����û�Beacon��������0�رգ�1��ʼ
		"BeaconSerHost":"string",    # Beacon ��������ַ
		"BeaconSerUrl":"string",     # Beacon ������url
		"BeaconSerPort": int,        # Beacon �������˿�
		"BeaconSerInterval":int      # Beacon ���������ʱ��
	}
	"GetWifiInfo": {
		"WifiApCliEnable": "string"         # �򿪻��ǹرգ�1�򿪣�0�ر�
		"WifiChannel" : "string",           # wifiͨ����
		"WifiApCliSsid" : "string",         # wifi ��ssid
		"WifiApCliAuthMode" : "string",     # wifi����ģʽ
		"WifiApCliEncrypType" : "string",   # wifi ��������
		"WifiApCliPassWord" : "string"      # wifi ����
	}
	"GetUploadIntervalInfo": {
		"MacSerInterval":int,          #�ϴ�������ʱ����
		"IbeaconSerInterval":int,      #�ϴ�������ʱ����
	}
	"GetOtherInfo": {
		"USB_DownLoadUrl":"string",    #�ļ�����·��
		"USB_DownLoadPort":int,        #���ض˿�
	}
}*/
// ��������: ��ȡcloudbeacon�Ļ�����Ϣ
void 
GetCBCInfo(int sockFD,  u_int16 typeID, char* RecvBuff, int buffLen);


/*{
	"SetMacSerInfo": {
		"MacSerOpen":int,        # �����û�Mac��������0�رգ�1��ʼ
		"MacSerHost":"string",   # Mac ��������ַ
		"MacSerUrl":"string",    # Mac ������url
		"MacSerPort": int,       # Mac �������˿�
		"MacSerInterval":int     # Mac ���������ʱ��
	}
	"SetBeaconSerInfo": {
		"BeaconSerOpen":int,         # �����û�Beacon��������0�رգ�1��ʼ
		"BeaconSerHost":"string",    # Beacon ��������ַ
		"BeaconSerUrl":"string",     # Beacon ������url
		"BeaconSerPort": int,        # Beacon �������˿�
		"BeaconSerInterval":int      # Beacon ���������ʱ��
	}
}*/
// ��������: ����cloudbeacon�Ļ�����Ϣ
void 
SetCBCInfo(int sockFD, int phoneHandle, char* RecvBuff, int buffLen);


/*[                       
	{
		"ifname":"string",             #��������
		"IPaddr":"string",             #IP��ַ
		"boardcast":"string",          #�㲥��ַ
		"gateway":"string",            #����
		"netmask";"string",            #��������
		"macaddr":"string",            #mac��ַ
		"MTU":int,                     #mtuֵ
	},
	......�����ж��
]*/
// ��������: ��ȡcloudbeacon����״̬
void
GetCBNetSta(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);



// ��������: ����cloudbeaconʹ��������������
void
SetCBConnectNetByReticle(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);

// ��������: ����cloudbeaconʹ��3g��������
void
SetCBConnectNetBy3G(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);


/*
{
	"WifiApCliEnable":"string",         # 1������0�ر�
	"WifiChannel" : "string",           # wifiͨ����
	"WifiApCliSsid" : "string",         # wifi ��ssid
	"WifiApCliAuthMode" : "string",     # wifi����ģʽ
	"WifiApCliEncrypType" : "string",   # wifi ��������
	"WifiApCliPassWord" : "string"      # wifi ����
}*/
// ��������: ����wifi�������ļ�
void
SetWifiConf(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);

/*
[
	{
		"Ch":int,                       #Ƶ��
		"SSID":"string",				#wifi ssid ��
		"BSSID":"string",				#wifi BSSID
		"Security":"string",			#wifi ����ģʽ
		"Siganl":int,					#wifi �ź�������λΪ�ٷֱ�
		"W-Mode":"string"				#wifi ģʽ
		"ExtCH": "string",
		"NT":"string";
	},
	....... �����ж��
]*/
// ��������: �鿴cloudbeacon��Χ���ȵ���Ϣ
void 
CheckWifiAPInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);


/* 
{
	"passwd":"string",         #�˺�����
}*/
// ��������: ����ϵͳ����
void
SetPasswd(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);


/*
{
	"usbInfo": [                         #����u�̵���Ϣ
		{
			"Filesystem": "string",      #�ļ�ϵͳ���ص�
			"mountOn": "string",         #���ص�
			"totalSize":"string",        #u ������
			"availableSize":"string",    #u �̿�������
			"usedSize":"string",         #u ����������
			"use%": "string",            #u ��ʹ�ðٷֱ�
		}
		����. �����ж��
	]
	"couldnotRecUSB"��[         #�޷�ʶ��U�̵��ļ�ϵͳ�����ز���
		"string" ��.            #usb �豸��
	]
}*/
// ��������: �鿴 U �̵�����״̬ 
void
CheckSDInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);

/*
{
	"memoryInfo":{
		"total": int,             #�ڴ��ܵĴ�С����λKB
		"used": int,              #�ڴ�ʹ�õĴ�С����λKB
		"free": int,              #ʣ��Ĵ�С����λKB
		"buffers" : int,          #��λKB
	},
	"flashInfo": {
		"totalSize": int,          #flash�ܵĴ�С����λMB
		"ubootSize", int,          #ubootʹ�ô�С����λKB
		"ubootEveSize": int,       #uboot �������ô�С����λKB
		"factorySize": int,        #ϵͳ���ô�С����λKB
		"firewareSize": int,       #�̼����ô�С����λKB
		"rootfsTotal": "string",   #���ļ�ϵͳ�ܴ�С
		"rootfsUsed": "string",    #���ļ�ϵͳ�Ѿ�ʹ�õĴ�С
		"rootfsFree": "string",    #���ļ�ϵͳʣ���С
		"romfsSize": string,       #�����ļ�ϵͳ��С
	}
	"systemInfo": {
		"systemName": "string",    #������
		"machine" : "string",      #�����ͺ�
		"kernelVer": "string",     #�ں˰汾
		"localTime": "string",     #����ʱ��
		"runTime": int,            #ϵͳ��������ǰ��ʱ�䣬��λs
		"freeTime":int,            #ϵͳ��������ǰ�Ŀ���ʱ�䣬��λs
		"loadAverage": "string",   #ƽ�����ؾ���
	}
}*/
// ��������: �鿴�ڴ桢flash��ϵͳ������Ϣ
void
CheckMemFlashSysInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);


/*
[
	{
		"IPAddress": "string",     # IP ��ַ
		"Flags": int,              # ���߱�־��0����ʾ�Ѿ����ߣ�2��ʾ����
		"MacAddress": "string"     # �豸mac��ַ
		"Device": "string",        # �����ĸ������豸
	}
    ......   #�ж��
]*/
// ��������: �鿴arp�б���Ϣ
void
CheckArpInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);


/*[
	{
		"LeasetimeRemaining": int,      # dhcp ��Լʣ����������λs
		"MAC-Address": "string",        # ���������mac��ַ
		"IPv4-Daddress": "string",      # ���������ip
		"Hostname": "string",           # �������������
	}
	........ #��N��
]*/
// ��������: �鿴dhcp�б���Ϣ
void
CheckDHCPInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);


/*{
	returnVal: int;
	# 0���ɹ�����
	# -280���޷�ʶ���3G��
	# -281���޷�ʶ���豸
}*/
// ��������: ����3G������
void 
Set3GNet(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);

// ��������: �鿴�������еĳ���
void
CheckPSInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);

void
GetCBLog(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);

void 
CheckNetStation(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);







void 
dealWithRecvPkg(int sockFD, char* RecvBuff, int buffLen, u_int16 typeID, int phoneFlag, const char* key);



void 
UpdateCBProgram(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);



/*
typedef struct WiFiInfo
{
	char  ssid[128];      // cb wifi ssid
	char  key[128];       // cb wifi ����
	char  channel[4];     // cb wifi ͨ����
	char  secMode[64];    // cb wifi ����ģʽ
	char  encrypType[64]; // cb wifi ��������
}WiFiInfo;
*/


struct WiFiInfo
{
    bool enable;        // true - ����WIFI, false - �ر�WIFI
    char ssid[128];     // wifi ssid
    char key[128];      // wifi key, 8 bytes at least
    int  channel;       // channel: 0 - 13
    int  secMode;       // security mode: 0 - NONE
                        //                1 - WEP
                        //                2 - WPAPSK
                        //                3 - WPA2PSK
    int  encrypType;    // encrypt mode:  0 - NONE
                        //                1 - WEP
                        //                2 - TKIP
                        //                3 - AES
};


// ��������: ��ȡ��Χwifi�ȵ���Ϣ
char* Get_AroundAPInfo();

// ��������: ��ȡwifiվ����Ϣ
int Get_StaInfo(WiFiInfo* info);

// ��������: ����wifiվ����Ϣ
int Set_StaInfo(WiFiInfo* info);

// ��������: ��ȡwifi�ȵ���Ϣ
int Get_APInfo(WiFiInfo* info);

// ��������: ����wifi�ȵ���Ϣ
int Set_APInfo(WiFiInfo* info);







#endif /*USE_OPENWRT*/
#endif /*__DEALWITH_OPENWRT_H__*/

