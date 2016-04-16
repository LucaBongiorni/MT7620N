#ifndef __BEACON_CONF__H__
#define __BEACON_CONF__H__

#include <pthread.h>
#include <string.h>

#include "defCom.h"

class IbeaconConfig
{
public:
    IbeaconConfig();
    ~IbeaconConfig();
	
    inline void lockConfFile(){pthread_mutex_lock(&m_confFileLock);}
    inline void UnlockConfFile(){pthread_mutex_unlock(&m_confFileLock);}
	
    const char* getConfFilePath()const{return m_confFilePath;}
    void setConfFilePath(const char* confFilePath)
    {
    	//printf("[%s:%d]ccccccccccccccccccccccccccccccccc\n", __FILE__, __LINE__);
		if (confFilePath)
			strncpy(m_confFilePath, confFilePath, FILEPATH_LEN);
		//printf("[%s:%d]ddddddddddddddddddddddddddddddddd\n", __FILE__, __LINE__);
	}

    const char* getWebDomain()const{return m_webDomain;}
    void setWebDomain(const char* webDomain)
    {
		if (webDomain)
			strncpy(m_webDomain, webDomain, DOMAIN_LEN);
	}

    const char* getSerials()const{return m_serials;}
    void setSerials(const char* serials)
    {
		if (serials)
			strncpy(m_serials, serials, SERIALS_LEN);
	}

    unsigned short getWebPort()const{return m_WebPort;}
    void setWebPort(unsigned short WebPort){m_WebPort = WebPort;}
	void setVersion(unsigned int version){m_version = version;}
    unsigned int getVersion()const{return m_version;}

	void setOpenStartScanDev();
	void setCloseStartScanDev();

	// 设置上传beacon扫描信息时间间隔，(tcp服务器和http服务器共用)
    void setBeaconInterval(unsigned int BeaconInterval);
	// 获取上传beacon扫描信息时间间隔，(tcp服务器和http服务器共用)
    int  getBeaconInterval();

	
#ifndef WIN32
    unsigned short getLocalPort()const{return m_LocalPort;}
    void setLocalPort(unsigned short LocalPort){m_LocalPort = LocalPort;}

    unsigned short getListenNum()const{return m_ListenNum;}
    void setListenNum(unsigned short ListenNum){m_ListenNum = ListenNum;}

    const char* getWanIP();
    void setWanIP(const char* wanIP);
    const char* getWanMAC();
    void setWanMAC(const char* mac);

    unsigned short getBcastPort()const{return m_BcastPort;}
    void setBcastPort(unsigned short port){m_BcastPort = port;}

	// 设置上传mac地址时间间隔，(tcp服务器和http服务器共用)
    void setMacInterval(unsigned int MacInterval);
	// 获取上传mac地址时间间隔，(tcp服务器和http服务器共用)
    int  getMacInterval();

	// 设置上传mac地址http服务器域名
    void setMacSerHost(const char* MacSerHost);
	// 获取上传mac地址http服务器域名
    const char* getMacSerHost();
	
	// 设置上传mac地址http服务器url
    void setMacSerUrl(const char* MacSerUrl);
	// 获取上传mac地址http服务器url
    const char* getMacSerUrl();

	// 设置上传mac地址http服务器端口
    void setMacSerPort(u_int16 MacSerPort);
	// 获取上传mac地址http服务器端口
    u_int16 getMacSerPort();
#endif


	// 设置上传mac地址时间间隔，(http服务器)
    //void setMacSerInterval(int MacSerInterval);
    // 获取上传mac地址时间间隔，(http服务器)
    //int getMacSerInterval();


	// 设置上传beacon扫描信息服务器域名(http服务器)
    void setBeaconSerHost(const char* BeaconSerHost);
	// 获取上传beacon扫描信息服务器域名(http服务器)
    const char* getBeaconSerHost();

	// 设置上传beacon扫描信息服务器url(http服务器)
    void setBeaconSerUrl(const char* BeaconSerUrl);
	// 获取上传beacon扫描信息服务器url(http服务器)
    const char* getBeaconSerUrl();

	// 设置上传beacon扫描信息服务器端口(http服务器)
    void setBeaconSerPort(u_int16 BeaconSerPort);
	// 获取上传beacon扫描信息服务器端口(http服务器)
    u_int16 getBeaconSerPort();

	// 设置上传beacon扫描信息服务器时间间隔(http服务器)
	//void setBeaconSerInterval(int BeaconSerInterval);
	// 获取上传beacon扫描信息服务器时间间隔(http服务器)
    //int getBeaconSerInterval();

#ifndef WIN32
	// 设置开启/关闭上传mac地址功能(http服务器)
    void setIsOpenMacSer(bool isOpenMacSer);
	// 获取开启/关闭上传mac地址功能(http服务器)
    bool getIsOpenMacSer();

	// 设置开启/关闭上传mac地址功能(tcp服务器)
	void setTCPMacSerOpenVal(bool val);
	// 获取开启/关闭上传mac地址功能(tcp服务器)
	bool getTCPMacSerOpenVal();
#endif
	
	// 设置开启/关闭上传beacon扫描信息功能(http服务器)
    void setIsOpenBeaconSer(bool isOpenBeaconSer);
	// 获取开启/关闭上传beacon扫描信息功能(http服务器)
    bool getIsOpenBeaconSer();


	// 设置开启/关闭上传beacon扫描信息功能(tcp服务器)
	void setTCPBeaconSerOpenVal(bool val);
	// 获取开启/关闭上传beacon扫描信息功能(tcp服务器)
	bool getTCPBeaconSerOpenVal();


	// 设置开启/关闭开始扫描beacon信息
	void setIsStartScanDev(bool val);
	// 获取开启/关闭开始扫描beacon信息
	bool getIsStartScanDev();

    char* getPhoneKey(){return m_PhoneKey;}
    void  setPhoneKey(const char* key);
    // 判断是否已经激活
    bool  ifBeenActivated();

	char* getComName()
	{
		return m_ComName;
	}
	void setComName(const char* name)
	{
		strncpy(m_ComName, name, 32);
	}

	char* getUpdateBlueBinHost()
	{
		return m_updateBlueBinHost;
	}
	void  setUpdateBlueBinHost(const char* host)
	{
		strncpy(m_updateBlueBinHost, host, 64);
	}
	u_int16 getUpdateBlueBinPort()
	{
		return m_updateBlueBinPort;
	}
	void setUpdateBlueBinPort(u_int16 port)
	{
		m_updateBlueBinPort = port;
	}
	char* getUpdateBlueBinUrl()
	{
		return m_updateBlueBinUrl;
	}
	void setUpdateBlueBinUrl(const char *url)
	{
		strncpy(m_updateBlueBinUrl, url, 128);
	}
	
private:
	char  m_ComName[32];           // 串口名字
    char* m_confFilePath;          // 配置文件
    char* m_webDomain;             // 服务器域名
    char* m_serials;               // 序列号
    char m_WanMAC[32];             // WiFi 网卡 mac 地址
    char m_LanMAC[32];             // 本地网卡 mac 地址
    char m_WanIP[16];              // wan 口IP
    char m_LanIP[16];              // LAN 口IP
    unsigned short m_WebPort;      // 服务器端口
    unsigned short m_LocalPort;    // 本地端口
    unsigned short m_BcastPort;    // 本地广播监听端口
    unsigned short m_ListenNum;    // 监听个数，允许子端连接数量
    unsigned int   m_version;      // 版本号

	bool m_TCP_MacSerOpen;		   // 是否上传mac地址信息到 tcp 服务器中
    //bool m_isUploadMacInfo;      // 是否上传mac地址信息到服务器中
    bool m_TCP_BeaconSerOpen;      // 是否上传beacon信息到 tcp 服务器中
    //bool m_isUploadBeaconInfo;   // 是否上传beacon信息到服务器中

	int m_MacInterval;             // 上传mac地址间隔时间(tcp和http服务器共用)
    int m_BeaconInterval;          // 上传Beacon信息间隔时间(tcp和http服务器共用)
    
    char* m_MacSerHost;            // 客户mac服务器地址
    char* m_MacSerUrl;             // 客户mac服务器url
    u_int16   m_MacSerPort;        // 客户mac服务器端口
    //int m_MacSerInterval;        // 客户mac服务器间隔时间
    bool  m_isOpenMacSer;          // 是否向客户Mac服务器上传
    char* m_BeaconSerHost;         // 客户Beacon服务器地址
    char* m_BeaconSerUrl;          // 客户Beacon服务器url
    u_int16 m_BeaconSerPort;     	// 客户Beacon服务器端口
    bool  m_isOpenBeaconSer;       // 是否向客户Beacon服务器上传信息
    
    bool  m_isStartScanDev;        // 是否开启扫描设备信息
    char  m_PhoneKey[PHONE_KEY_LEN+1];
    pthread_rwlock_t m_rwLock;

	// 更新蓝牙固件需要的配置
	char m_updateBlueBinHost[64];
	u_int16 m_updateBlueBinPort;
	char m_updateBlueBinUrl[128];
	
	
public:
    //char wanIfname[16];
    //char wifiIfname[16];
	//char wan3gIfname[16];
    //char lanIfname[16];
	char debuglevel;
    // 功能函数

public:
    pthread_mutex_t m_confFileLock;  // 配置文件锁
};




void _debug(const char *function, int line, int level, const char *format, ...);
void checkLogFileSize(void* arg);


void checkNetConnect(void* arg);



#endif /*__BEACON_CONF__H__*/

