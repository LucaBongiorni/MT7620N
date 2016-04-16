#ifndef __DEALWITH_OPENWRT_H__
#define __DEALWITH_OPENWRT_H__

#ifdef USE_OPENWRT

#include "defCom.h"


// 函数功能: 获取wifi的ssid 
// 函数参数: 空
// 返 回 值: 成功返回获取到的ssid，失败返回NULL.
char* 
getWiFiSSID();
// 函数功能: 检查apcli的ssid连接是否存在
// 函数参数: 空
// 返 回 值: 成功返回0，失败返回-1
int 
checkWiFiCliSsidConnect();
// 函数功能: 切换为wifi上网
// 函数参数: 空
// 返 回 值: 成功返回0，失败返回-1
int
switchWiFiNetwork();
// 函数功能: 切换为3G上网
// 函数参数: type, 3/联通，2/移动，1/电信
// 返 回 值: 成功返回0，失败返回-1
int 
switch3GNetwork(int type);
// 函数功能: 切换为网线上网
// 函数参数: 空
// 返 回 值: 成功返回0，失败返回-1
int 
switchReticleNetwork();
// 函数功能: wan口重新联网
// 函数参数: 空
// 返 回 值: 无
void 
wanReconnectNetwork();



/*{
	"GetMacSerInfo": {
		"MacSerOpen":int,        # 开关用户Mac服务器，0关闭，1开始
		"MacSerHost":"string",   # Mac 服务器地址
		"MacSerUrl":"string",    # Mac 服务器url
		"MacSerPort": int,       # Mac 服务器端口
		"MacSerInterval":int     # Mac 服务器间隔时间
	}
	"GetBeaconSerInfo": {
		"BeaconSerOpen":int,         # 开关用户Beacon服务器，0关闭，1开始
		"BeaconSerHost":"string",    # Beacon 服务器地址
		"BeaconSerUrl":"string",     # Beacon 服务器url
		"BeaconSerPort": int,        # Beacon 服务器端口
		"BeaconSerInterval":int      # Beacon 服务器间隔时间
	}
	"GetWifiInfo": {
		"WifiApCliEnable": "string"         # 打开还是关闭，1打开，0关闭
		"WifiChannel" : "string",           # wifi通道号
		"WifiApCliSsid" : "string",         # wifi 的ssid
		"WifiApCliAuthMode" : "string",     # wifi加密模式
		"WifiApCliEncrypType" : "string",   # wifi 加密类型
		"WifiApCliPassWord" : "string"      # wifi 密码
	}
	"GetUploadIntervalInfo": {
		"MacSerInterval":int,          #上传服务器时间间隔
		"IbeaconSerInterval":int,      #上传服务器时间间隔
	}
	"GetOtherInfo": {
		"USB_DownLoadUrl":"string",    #文件下载路径
		"USB_DownLoadPort":int,        #下载端口
	}
}*/
// 函数功能: 获取cloudbeacon的基本信息
void 
GetCBCInfo(int sockFD,  u_int16 typeID, char* RecvBuff, int buffLen);


/*{
	"SetMacSerInfo": {
		"MacSerOpen":int,        # 开关用户Mac服务器，0关闭，1开始
		"MacSerHost":"string",   # Mac 服务器地址
		"MacSerUrl":"string",    # Mac 服务器url
		"MacSerPort": int,       # Mac 服务器端口
		"MacSerInterval":int     # Mac 服务器间隔时间
	}
	"SetBeaconSerInfo": {
		"BeaconSerOpen":int,         # 开关用户Beacon服务器，0关闭，1开始
		"BeaconSerHost":"string",    # Beacon 服务器地址
		"BeaconSerUrl":"string",     # Beacon 服务器url
		"BeaconSerPort": int,        # Beacon 服务器端口
		"BeaconSerInterval":int      # Beacon 服务器间隔时间
	}
}*/
// 函数功能: 设置cloudbeacon的基本信息
void 
SetCBCInfo(int sockFD, int phoneHandle, char* RecvBuff, int buffLen);


/*[                       
	{
		"ifname":"string",             #网卡名字
		"IPaddr":"string",             #IP地址
		"boardcast":"string",          #广播地址
		"gateway":"string",            #网关
		"netmask";"string",            #子网掩码
		"macaddr":"string",            #mac地址
		"MTU":int,                     #mtu值
	},
	......可能有多个
]*/
// 函数功能: 获取cloudbeacon网络状态
void
GetCBNetSta(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);



// 函数功能: 设置cloudbeacon使用网线连接外网
void
SetCBConnectNetByReticle(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);

// 函数功能: 配置cloudbeacon使用3g连接外网
void
SetCBConnectNetBy3G(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);


/*
{
	"WifiApCliEnable":"string",         # 1开启，0关闭
	"WifiChannel" : "string",           # wifi通道号
	"WifiApCliSsid" : "string",         # wifi 的ssid
	"WifiApCliAuthMode" : "string",     # wifi加密模式
	"WifiApCliEncrypType" : "string",   # wifi 加密类型
	"WifiApCliPassWord" : "string"      # wifi 密码
}*/
// 函数功能: 设置wifi的配置文件
void
SetWifiConf(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);

/*
[
	{
		"Ch":int,                       #频道
		"SSID":"string",				#wifi ssid 号
		"BSSID":"string",				#wifi BSSID
		"Security":"string",			#wifi 加密模式
		"Siganl":int,					#wifi 信号量，单位为百分比
		"W-Mode":"string"				#wifi 模式
		"ExtCH": "string",
		"NT":"string";
	},
	....... 可能有多个
]*/
// 函数功能: 查看cloudbeacon周围的热点信息
void 
CheckWifiAPInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);


/* 
{
	"passwd":"string",         #账号密码
}*/
// 函数功能: 设置系统密码
void
SetPasswd(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);


/*
{
	"usbInfo": [                         #挂载u盘的信息
		{
			"Filesystem": "string",      #文件系统挂载点
			"mountOn": "string",         #挂载点
			"totalSize":"string",        #u 盘容量
			"availableSize":"string",    #u 盘可用容量
			"usedSize":"string",         #u 盘已用容量
			"use%": "string",            #u 盘使用百分比
		}
		……. 可能有多个
	]
	"couldnotRecUSB"：[         #无法识别U盘的文件系统，挂载不了
		"string" ….            #usb 设备名
	]
}*/
// 函数功能: 查看 U 盘的连接状态 
void
CheckSDInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);

/*
{
	"memoryInfo":{
		"total": int,             #内存总的大小，单位KB
		"used": int,              #内存使用的大小，单位KB
		"free": int,              #剩余的大小，单位KB
		"buffers" : int,          #单位KB
	},
	"flashInfo": {
		"totalSize": int,          #flash总的大小，单位MB
		"ubootSize", int,          #uboot使用大小，单位KB
		"ubootEveSize": int,       #uboot 引导配置大小，单位KB
		"factorySize": int,        #系统配置大小，单位KB
		"firewareSize": int,       #固件可用大小，单位KB
		"rootfsTotal": "string",   #根文件系统总大小
		"rootfsUsed": "string",    #根文件系统已经使用的大小
		"rootfsFree": "string",    #根文件系统剩余大小
		"romfsSize": string,       #备份文件系统大小
	}
	"systemInfo": {
		"systemName": "string",    #主机名
		"machine" : "string",      #主机型号
		"kernelVer": "string",     #内核版本
		"localTime": "string",     #本地时间
		"runTime": int,            #系统启动到当前的时间，单位s
		"freeTime":int,            #系统启动到当前的空闲时间，单位s
		"loadAverage": "string",   #平均负载均衡
	}
}*/
// 函数功能: 查看内存、flash、系统基本信息
void
CheckMemFlashSysInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);


/*
[
	{
		"IPAddress": "string",     # IP 地址
		"Flags": int,              # 在线标志，0，表示已经下线，2表示在线
		"MacAddress": "string"     # 设备mac地址
		"Device": "string",        # 连接哪个网卡设备
	}
    ......   #有多个
]*/
// 函数功能: 查看arp列表信息
void
CheckArpInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);


/*[
	{
		"LeasetimeRemaining": int,      # dhcp 租约剩余秒数，单位s
		"MAC-Address": "string",        # 分配的主机mac地址
		"IPv4-Daddress": "string",      # 分配的主机ip
		"Hostname": "string",           # 分配的主机名字
	}
	........ #有N个
]*/
// 函数功能: 查看dhcp列表信息
void
CheckDHCPInfo(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);


/*{
	returnVal: int;
	# 0：成功，；
	# -280：无法识别改3G卡
	# -281：无法识别设备
}*/
// 函数功能: 设置3G上网。
void 
Set3GNet(int sockFD, u_int16 typeID, char* RecvBuff, int buffLen);

// 函数功能: 查看正在运行的程序
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
	char  key[128];       // cb wifi 密码
	char  channel[4];     // cb wifi 通道号
	char  secMode[64];    // cb wifi 加密模式
	char  encrypType[64]; // cb wifi 加密类型
}WiFiInfo;
*/


struct WiFiInfo
{
    bool enable;        // true - 开启WIFI, false - 关闭WIFI
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


// 函数功能: 获取周围wifi热点信息
char* Get_AroundAPInfo();

// 函数功能: 获取wifi站点信息
int Get_StaInfo(WiFiInfo* info);

// 函数功能: 设置wifi站点信息
int Set_StaInfo(WiFiInfo* info);

// 函数功能: 获取wifi热点信息
int Get_APInfo(WiFiInfo* info);

// 函数功能: 设置wifi热点信息
int Set_APInfo(WiFiInfo* info);







#endif /*USE_OPENWRT*/
#endif /*__DEALWITH_OPENWRT_H__*/

