#ifndef __CBPROJ__MAiN_H__
#define __CBPROJ__MAiN_H__

#include "netSockTcp.h"
#include "listStruct.h"

#define DOMAIN_LEN       256
#define FILEPATH_LEN     256
#define SERIALS_LEN      128
#define URL_LEN          256
#define POST_HEAD_LEN    1024

#ifndef IFNAMESIZ
#define IFNAMESIZ   16
#endif


class IbeaconConfig
{
public:
	IbeaconConfig();
	~IbeaconConfig();

	void lockConfFile();
	void UnlockConfFile();

	const char* getConfFilePath()const;
	void setConfFilePath(const char* confFilePath);
	
	const char* getWebDomain()const;
	void setWebDomain(const char* webDomain);
	
	const char* getSerials()const;
	void setSerials(const char* serials);
	
	unsigned short getWebPort()const;
	void setWebPort(unsigned short WebPort);
	
	unsigned short getLocalPort()const;
	void setLocalPort(unsigned short LocalPort);
	
	unsigned short getListenNum()const;
	void setListenNum(unsigned short ListenNum);
	
	const char* getWanIP()const;
	void setWanIP(const char* wanIP);
	
	const char* getLanIP()const;
	void setLanIP(const char* lanIP);
	
	const char* getWanMAC()const;
	void setWanMAC(const char* mac);
	
	const char* getLanMAC()const;
	void setLanMAC(const char* mac);
	
	unsigned short getBcastPort()const;
	void setBcastPort(unsigned short port);

	void setVersion(unsigned int version);
	unsigned int getVersion()const;

	void setMacInterval(unsigned int MacInterval);
	int  getMacInterval();

	void setBeaconInterval(unsigned int BeaconInterval);
	int  getBeaconInterval();

	void setMacSerHost(const char* MacSerHost);
	const char* getMacSerHost();

	void setMacSerUrl(const char* MacSerUrl);
	const char* getMacSerUrl();

	void setMacSerPort(u_int16 MacSerPort);
	u_int16 getMacSerPort();

	void setMacSerInterval(int MacSerInterval);
	int getMacSerInterval();

	void setIsOpenMacSer(bool isOpenMacSer);
	bool getIsOpenMacSer();

	void setBeaconSerHost(const char* BeaconSerHost);
	const char* getBeaconSerHost();

	void setBeaconSerUrl(const char* BeaconSerUrl);
	const char* getBeaconSerUrl();

	void setBeaconSerPort(u_int16 BeaconSerPort);
	u_int16 getBeaconSerPort();

	void setBeaconSerInterval(int BeaconSerInterval);
	int getBeaconSerInterval();

	void setIsOpenBeaconSer(bool isOpenBeaconSer);
	bool getIsOpenBeaconSer();

	char* getPhoneKey();
	void  setPhoneKey(const char* key);

	// 判断是否已经激活
	bool  ifBeenActivated();

private:
	char* m_confFilePath;		   // 配置文件
	char* m_webDomain;			   // 服务器域名
	char* m_serials;               // 序列号
	char m_WanMAC[32];             // WiFi 网卡 mac 地址
	char m_LanMAC[32];             // 本地网卡 mac 地址
	char m_WanIP[16];     		   // wan 口IP
	char m_LanIP[16];     		   // LAN 口IP 
	unsigned short m_WebPort;      // 服务器端口
	unsigned short m_LocalPort;    // 本地端口
	unsigned short m_BcastPort;    // 本地广播监听端口
	unsigned short m_ListenNum;	   // 监听个数，允许子端连接数量
	unsigned int   m_version;      // 版本号
	int m_MacInterval;             // tcp上传mac地址间隔时间
	int m_BeaconInterval;          // tcp上传Beacon信息间隔时间

	char* m_MacSerHost;            // 客户mac服务器地址
	char* m_MacSerUrl;             // 客户mac服务器url
	u_int16   m_MacSerPort;        // 客户mac服务器端口
	int   m_MacSerInterval;        // 客户mac服务器间隔时间
	bool  m_isOpenMacSer;          // 是否向Mac服务器上传

	char* m_BeaconSerHost;         // 客户Beacon服务器地址
	char* m_BeaconSerUrl;          // 客户Beacon服务器url
	u_int16   m_BeaconSerPort;     // 客户Beacon服务器端口
	int   m_BeaconSerInterval;     // 客户Beacon服务器间隔时间
	bool  m_isOpenBeaconSer;       // 是否向Beacon服务器上传信息

	char m_PhoneKey[PHONE_KEY_LEN+1]; 

	pthread_rwlock_t m_rwLock;

	// 功能函数
public:
	// 功    能: 获取域名对应的ip
	// 输入参数: pIP, 域名；
	// 输出参数: lAddr 输出网络字节的ip
	// 返 回 值: 成功返回0， 失败返回 -1
	static int 
	GetIPByDomain(const char* pDomain, unsigned int *lAddr);

	// 功    能: 通过网卡名获取网卡IP
	// 输入参数: ifname 网卡名字
	// 返 回 值: 返回0失败
	static unsigned int 
	GetIPByIfname(const char *ifname);

	// 功    能: 通过网卡名获取网卡 mac 地址
	// 输入参数: ifname 网卡名字
	// 返 回 值: 返回 MAC 地址，失败返回 NULL，返回的 MAC 地址要释放
	static char* 
	GetMacByIfName(const char *ifname);

	pthread_mutex_t m_confFileLock;  // 配置文件锁
};








IbeaconConfig* GetConfigPosition();


static inline void 
usage(void)
{
    printf("Usage: cbProj [options]\n");
    printf("\n");
    printf("  -c [filename] Use this config file.\n");
    printf("  -h            Print usage.\n");
    printf("  -v            Print version information.\n");
	printf("  -p            Local server listen port.\n");
	printf("  -b            Local broadcast listen port.\n");
	printf("  -n            Save Node Number.\n");
	printf("  -H            Web server domain.\n");
	printf("  -P            Web sever port.\n");
    printf("\n");
}


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

// 主要用于线程参数的传递
typedef struct _ServerTcpParam
{
	int sockFD;
	CSocketTCP* serverTCP;
	ListManage* net2SerHead;
}ServerTcpParam;
#undef  ServerTcpParam_Len
#define ServerTcpParam_Len   sizeof(ServerTcpParam)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__CBPROJ__MAiN_H__*/

