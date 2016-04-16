#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if_arp.h>



#include "defCom.h"
#include "confile.h"
#include "StrOperate.h"
#include "netSockTcp.h"
#include "procotol.h"
#include "listStruct.h"
#include "broadcast.h"
#include "cbProjMain.h"
#include "pthreadServer.h"
#include "socketClient.h"
#include "pthreadCom.h"

using namespace std;



IbeaconConfig config;
IbeaconConfig* GetConfigPosition()
{
	return &config;
}

u_int32 
IbeaconConfig::GetIPByIfname(const char *ifname)
{
    struct ifreq if_data;
    int sockd;
    u_int32 ip;
    if ((sockd = socket(AF_INET, SOCK_PACKET, htons(0x8086))) < 0)
    {
        write_error_log("Create Socket Failed.");
        return 0;
    }
    /* Get IP of internal interface */
    strcpy(if_data.ifr_name, ifname);
	
    /* Get the IP address */
    if (ioctl(sockd, SIOCGIFADDR, &if_data) < 0)
    {
        write_error_log("Ioctl Get Socket Ifaddr Failed. %m");
        close(sockd);
        return 0;
    }
    memcpy((void*)&ip, (char*)&if_data.ifr_addr.sa_data + 2, 4);
    close(sockd);
    return ip;
}
char* 
IbeaconConfig::GetMacByIfName(const char *ifname)
{
	int r;
	int sockd;
	struct ifreq ifr;
	char *hwaddr, mac[32];

	if (ifname == NULL || strlen(ifname) > (IFNAMESIZ-1))
	{
		printf("[%s][%d]Param Error.", __FILE__, __LINE__);
		return NULL;
	}
	
	if (-1 == (sockd = socket(PF_INET, SOCK_DGRAM, 0))) 
	{
		printf("[%s][%d]Create Socket Failed.", __FILE__, __LINE__);
		return NULL;
	}
	
	strcpy(ifr.ifr_name, ifname);
	r = ioctl(sockd, SIOCGIFHWADDR, &ifr);
	if (r == -1) 
	{
		printf("[%s][%d]Ioctl Get Socket Ifaddr Failed.", __FILE__, __LINE__);
		close(sockd);
		return NULL;
	}
	hwaddr = ifr.ifr_hwaddr.sa_data;
	close(sockd);
	
	snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X", 
			hwaddr[0] & 0xFF,
			hwaddr[1] & 0xFF,
			hwaddr[2] & 0xFF,
			hwaddr[3] & 0xFF,
			hwaddr[4] & 0xFF,
			hwaddr[5] & 0xFF
		);
	return strdup(mac);
}
int 
IbeaconConfig::GetIPByDomain(const char* pDomain, unsigned int *lAddr)
{
    struct hostent *hp = NULL;
    if ( (hp = gethostbyname(pDomain)) != NULL )
    {
        if (hp->h_addr_list[0])
        {
            *lAddr = *((unsigned int*) hp->h_addr_list[0]);
            return SUCCESS;
        }
    }
    return FAILED;
}

IbeaconConfig::IbeaconConfig()
{
	m_confFilePath = new char[FILEPATH_LEN];
	m_webDomain = new char[DOMAIN_LEN];
	m_serials   = new char[SERIALS_LEN];

	memset(m_confFilePath, 0, FILEPATH_LEN);
	memset(m_webDomain, 0, DOMAIN_LEN);
	memset(m_serials, 0, SERIALS_LEN);
	memset(m_WanMAC, 0, 32);
	memset(m_LanMAC, 0, 32);
	memset(m_LanIP, 0, 16);
	memset(m_WanIP, 0, 16);

	m_WebPort   = 0;  
	m_LocalPort = 0;
	m_ListenNum = 0;
	m_BcastPort = 0;
	m_MacInterval = 0;
	m_BeaconInterval = 0;

	m_MacSerHost = new char[DOMAIN_LEN];
	m_MacSerUrl  = new char[URL_LEN];
	m_MacSerPort = 0;
	m_MacSerInterval = 0;
	memset(m_MacSerHost, 0, DOMAIN_LEN);
	memset(m_MacSerUrl, 0, URL_LEN);
	m_isOpenMacSer = false;

	m_BeaconSerHost = new char[DOMAIN_LEN];
	m_BeaconSerUrl = new char[URL_LEN];
	m_BeaconSerPort = 0;
	m_BeaconSerInterval = 0;
	memset(m_BeaconSerHost, 0, DOMAIN_LEN);
	memset(m_MacSerUrl, 0, URL_LEN);
	m_isOpenBeaconSer = false;

	pthread_mutex_init(&m_confFileLock, 0);
	pthread_rwlock_init(&m_rwLock, 0);
}
IbeaconConfig::~IbeaconConfig()
{
	if (m_confFilePath)
		delete [] m_confFilePath, m_confFilePath = NULL;
	if (m_webDomain)
		delete [] m_webDomain, m_webDomain = NULL;
	if (m_serials)
		delete [] m_serials, m_serials = NULL;
	if (m_MacSerHost)
		delete [] m_MacSerHost, m_MacSerHost = NULL;
	if (m_BeaconSerHost)
		delete [] m_BeaconSerHost, m_BeaconSerHost = NULL;
	if (m_MacSerUrl)
		delete [] m_MacSerUrl, m_MacSerUrl = NULL;
	if (m_BeaconSerUrl)
		delete [] m_BeaconSerUrl, m_BeaconSerUrl = NULL;

	pthread_mutex_destroy(&m_confFileLock);
	pthread_rwlock_destroy(&m_rwLock);
}
void 
IbeaconConfig::lockConfFile()
{
	pthread_mutex_lock(&m_confFileLock);
}
void 
IbeaconConfig::UnlockConfFile()
{
	pthread_mutex_unlock(&m_confFileLock);
}
const char* 
IbeaconConfig::getConfFilePath()const
{
	return m_confFilePath;
}
const char* 
IbeaconConfig::getWebDomain()const
{
	return m_webDomain;
}
const char* 
IbeaconConfig::getSerials()const
{
	return m_serials;
}
unsigned short 
IbeaconConfig::getWebPort()const
{
	return m_WebPort;
}
unsigned short 
IbeaconConfig::getLocalPort()const
{
	return m_LocalPort;
}
unsigned short 
IbeaconConfig::getListenNum()const
{
	return m_ListenNum;
}
const char* 
IbeaconConfig::getWanIP()const
{
	return m_WanIP;
}
const char* 
IbeaconConfig::getLanIP()const
{
	return m_LanIP;
}
const char* 
IbeaconConfig::getWanMAC()const
{
	return m_WanMAC;
}
const char* 
IbeaconConfig::getLanMAC()const
{
	return m_LanMAC;
}
unsigned short 
IbeaconConfig::getBcastPort()const
{
	return m_BcastPort;
}
void 
IbeaconConfig::setConfFilePath(const char* confFilePath)
{
	strncpy(m_confFilePath, confFilePath, FILEPATH_LEN);
}
void 
IbeaconConfig::setWebDomain(const char* webDomain)
{
	strncpy(m_webDomain, webDomain, DOMAIN_LEN);
}
void 
IbeaconConfig::setSerials(const char* serials)
{
	strncpy(m_serials, serials, SERIALS_LEN);
}
void 
IbeaconConfig::setWebPort(unsigned short WebPort)
{
	m_WebPort = WebPort;
}
void 
IbeaconConfig::setLocalPort(unsigned short LocalPort)
{
	m_LocalPort = LocalPort;
}
void 
IbeaconConfig::setListenNum(unsigned short ListenNum)
{
	m_ListenNum = ListenNum;
}
void 
IbeaconConfig::setWanIP(const char* wanIP)
{
	strncpy(m_WanIP, wanIP, 16);
}
void 
IbeaconConfig::setLanIP(const char* lanIP)
{
	strncpy(m_LanIP, lanIP, 16);
}
void 
IbeaconConfig::setWanMAC(const char* mac)
{
	strncpy(m_WanMAC, mac, 32);
}
void 
IbeaconConfig::setLanMAC(const char* mac)
{
	strncpy(m_LanMAC, mac, 32);
}
void 
IbeaconConfig::setBcastPort(unsigned short port)
{
	m_BcastPort = port;
}
void 
IbeaconConfig::setVersion(unsigned int version)
{
	m_version = version;
}
unsigned int 
IbeaconConfig::getVersion()const
{
	return m_version;
}

void 
IbeaconConfig::setMacInterval(unsigned int MacInterval)
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_MacInterval = MacInterval;
	pthread_rwlock_unlock(&m_rwLock);
}
int  
IbeaconConfig::getMacInterval()
{
	int temp;
	pthread_rwlock_rdlock(&m_rwLock);
	temp = m_MacInterval;
	pthread_rwlock_unlock(&m_rwLock);
	return temp;
}

void 
IbeaconConfig::setBeaconInterval(unsigned int BeaconInterval)
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_BeaconInterval = BeaconInterval;
	pthread_rwlock_unlock(&m_rwLock);
}
int  
IbeaconConfig::getBeaconInterval()
{
	int temp;
	pthread_rwlock_rdlock(&m_rwLock);
	temp = m_BeaconInterval;
	pthread_rwlock_unlock(&m_rwLock);
	return temp;
}	

void 
IbeaconConfig::setMacSerHost(const char* MacSerHost)
{
	pthread_rwlock_wrlock(&m_rwLock);
	strncpy(m_MacSerHost, MacSerHost, DOMAIN_LEN);
	pthread_rwlock_unlock(&m_rwLock);
}
const char* 
IbeaconConfig::getMacSerHost()
{	
	static char host[DOMAIN_LEN];
	pthread_rwlock_rdlock(&m_rwLock);
	strncpy(host, m_MacSerHost, DOMAIN_LEN);
	pthread_rwlock_unlock(&m_rwLock);
	return host;
}

void 
IbeaconConfig::setMacSerUrl(const char* MacSerUrl)
{
	pthread_rwlock_wrlock(&m_rwLock);
	strncpy(m_MacSerUrl, MacSerUrl, URL_LEN);
	pthread_rwlock_unlock(&m_rwLock);
}
const char* 
IbeaconConfig::getMacSerUrl()
{
	static char url[URL_LEN];
	pthread_rwlock_rdlock(&m_rwLock);
	strncpy(url, m_MacSerUrl, URL_LEN);
	pthread_rwlock_unlock(&m_rwLock);
	return url;
}

void 
IbeaconConfig::setMacSerPort(u_int16 MacSerPort)
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_MacSerPort = MacSerPort;
	pthread_rwlock_unlock(&m_rwLock);
}
u_int16 
IbeaconConfig::getMacSerPort()
{
	u_int16 temp;
	pthread_rwlock_rdlock(&m_rwLock);
	temp = m_MacSerPort;
	pthread_rwlock_unlock(&m_rwLock);
	return temp;
}

void 
IbeaconConfig::setMacSerInterval(int MacSerInterval)
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_MacSerInterval = MacSerInterval;
	pthread_rwlock_unlock(&m_rwLock);
}
int 
IbeaconConfig::getMacSerInterval()
{
	int temp;
	pthread_rwlock_rdlock(&m_rwLock);
	temp = m_MacSerInterval;
	pthread_rwlock_unlock(&m_rwLock);
	return temp;
}

void 
IbeaconConfig::setIsOpenMacSer(bool isOpenMacSer)
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_isOpenMacSer = isOpenMacSer;
	pthread_rwlock_unlock(&m_rwLock);
}
bool 
IbeaconConfig::getIsOpenMacSer()
{
	bool temp;
	pthread_rwlock_rdlock(&m_rwLock);
	temp = m_isOpenMacSer;
	pthread_rwlock_unlock(&m_rwLock);
	return temp;
}

void 
IbeaconConfig::setBeaconSerHost(const char* BeaconSerHost)
{
	pthread_rwlock_wrlock(&m_rwLock);
	strncpy(m_BeaconSerHost, BeaconSerHost, DOMAIN_LEN);
	pthread_rwlock_unlock(&m_rwLock);
}
const char* 
IbeaconConfig::getBeaconSerHost()
{
	static char host[DOMAIN_LEN];
	pthread_rwlock_rdlock(&m_rwLock);
	strncpy(host, m_BeaconSerHost, DOMAIN_LEN);
	pthread_rwlock_unlock(&m_rwLock);
	return host;
}

void 
IbeaconConfig::setBeaconSerUrl(const char* BeaconSerUrl)
{
	pthread_rwlock_wrlock(&m_rwLock);
	strncpy(m_BeaconSerUrl, BeaconSerUrl, URL_LEN);
	pthread_rwlock_unlock(&m_rwLock);
}
const char* 
IbeaconConfig::getBeaconSerUrl()
{
	static char url[URL_LEN];
	pthread_rwlock_rdlock(&m_rwLock);
	strncpy(url, m_BeaconSerUrl, URL_LEN);
	pthread_rwlock_unlock(&m_rwLock);
	return url;
}

void 
IbeaconConfig::setBeaconSerPort(u_int16 BeaconSerPort)
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_BeaconSerPort = BeaconSerPort;
	pthread_rwlock_unlock(&m_rwLock);
}
u_int16 
IbeaconConfig::getBeaconSerPort()
{
	u_int16 temp;
	pthread_rwlock_rdlock(&m_rwLock);
	temp = m_BeaconSerPort;
	pthread_rwlock_unlock(&m_rwLock);
	return temp;
}

void 
IbeaconConfig::setBeaconSerInterval(int BeaconSerInterval)
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_BeaconSerInterval = BeaconSerInterval;
	pthread_rwlock_unlock(&m_rwLock);
}
int 
IbeaconConfig::getBeaconSerInterval()
{
	int temp;
	pthread_rwlock_rdlock(&m_rwLock);
	temp = m_BeaconSerInterval;
	pthread_rwlock_unlock(&m_rwLock);
	return temp;
}

void 
IbeaconConfig::setIsOpenBeaconSer(bool isOpenBeaconSer)
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_isOpenBeaconSer = isOpenBeaconSer;
	pthread_rwlock_unlock(&m_rwLock);
}
bool 
IbeaconConfig::getIsOpenBeaconSer()
{
	bool temp;
	pthread_rwlock_rdlock(&m_rwLock);
	temp = m_isOpenBeaconSer;
	pthread_rwlock_unlock(&m_rwLock);
	return temp;
}

char* 
IbeaconConfig::getPhoneKey()
{
	return m_PhoneKey;
}
void  
IbeaconConfig::setPhoneKey(const char* key)
{
	if (NULL == key)
		memset(m_PhoneKey, 0, PHONE_KEY_LEN);
	else
		strncpy(m_PhoneKey, key, PHONE_KEY_LEN);
}

bool 
IbeaconConfig::ifBeenActivated()
{
	if (0 == access(DEF_PHONE_CONNECT_KEY, F_OK))
	{
		return true;
	}
	else
	{
		return false;
	}
}


void 
parseComLineAndConfFile(int argc, char **argv)
{
    int c;
	bool webDomainFlag  = false;
	bool confFilePathFlag = false;
	bool net2serNumFlag = false;
	char temp[256] = {0};
	//char webDomain[DOMAIN_LEN] = {0};
	//char url[URL_LEN] = {0};
	char WiFiIfname[16] = {0};
	char LanIfname[16]  = {0};
	char phoneKey[PHONE_KEY_LEN+1] = {0};
	char WanIfname[16]  = {0};
	char UpdateFilePath[64] = {0};
	char* pMac = NULL;
	u_int32 tempIP;

    while (-1 != (c = getopt(argc, argv, "c:hvP:p:H:n:")))
    {
		switch(c) 
		{
			case 'h':
				usage();
				exit(1);
				break;
				
			case 'c':
				if (optarg) 
				{
					config.setConfFilePath(optarg);
					confFilePathFlag = true;
				}
				break;
				
			case 'v':
				printf("This is cbProj version: %s %s\n", __VERDATA__, __VERTIME__);
				exit(1);
				break;

			case 'P':
				if (optarg)
				{
					config.setWebPort(atoi(optarg));
				}
				break;

			case 'p':
				if (optarg)
				{
					config.setLocalPort(atoi(optarg));
				}
				break;

			case 'b':
				if (optarg)
				{
					config.setBcastPort(atoi(optarg));
				}
				break;

			case 'H':
				if (optarg)
				{
					config.setWebDomain(optarg);
					webDomainFlag = true;
				}
				break;
			case 'n':
				if (optarg)
				{
					setNet2SerNodeNum(atoi(optarg));
					net2serNumFlag = true;
				}
				break;
				
			default:
				usage();
				exit(1);
				break;
		}
    }

	if (false == confFilePathFlag)
	{
		config.setConfFilePath(DEF_CONFIG_FILE_PATH);
	}

	// 加载配置文件
	if (!CConfFile::Init())
	{
		printf("Init ini failure.");
		exit(1);
	}
	CConfFile* confFile = new CConfFile();
	if (!confFile->LoadFile(config.getConfFilePath()))
	{
		write_error_log("Open ini failure.");
		delete confFile;
		exit(1);
	}

	if (false == webDomainFlag)
	{
		confFile->GetString("WebServerInfo", "Host", temp);
		config.setWebDomain(temp);
	}
	if (config.getWebPort() == 0)
	{
		config.setWebPort(confFile->GetInt("WebServerInfo", "Port"));
	}
	if (config.getLocalPort() == 0)
	{
		config.setLocalPort(confFile->GetInt("LocalServerInfo", "Port"));
	}
	if (config.getBcastPort() == 0)
	{
		config.setBcastPort(confFile->GetInt("LocalServerInfo", "BcastPort"));
	}
	config.setListenNum(confFile->GetInt("LocalServerInfo", "ListenNum"));
	if (false == net2serNumFlag)
	{
		setNet2SerNodeNum(confFile->GetInt("LocalServerInfo", "net2serNodeNum"));
	}	

	// 获取网口名字
	confFile->GetString("LocalServerInfo", "WiFiAPIfname", WiFiIfname);
	confFile->GetString("LocalServerInfo", "LanIfname", LanIfname);
	confFile->GetString("LocalServerInfo", "WanIfname", WanIfname);

	// tcp上传间隔时间
	config.setMacInterval(confFile->GetInt("LocalServerInfo", "MacInterval"));
	config.setBeaconInterval(confFile->GetInt("LocalServerInfo", "BeaconInterval"));

	// 更新配置文件
	confFile->GetString("LocalServerInfo", "UpdateFilePath", UpdateFilePath);

	// 客户服务器信息
	if (confFile->GetInt("CustomServerInfo", "OpenMacSer") != 0)
	{
		// 开启
		config.setIsOpenMacSer(true);
		confFile->GetString("CustomServerInfo", "MacSerHost", temp);
		config.setMacSerHost(temp);
		confFile->GetString("CustomServerInfo", "MacSerUrl", temp);
		config.setMacSerUrl(temp);
		config.setMacSerPort(confFile->GetInt("CustomServerInfo", "MacSerPort"));
		config.setMacSerInterval(confFile->GetInt("CustomServerInfo", "MacSerInterval"));
	}
	else
	{
		// 没有开启
		config.setIsOpenMacSer(false);
	}

	if (confFile->GetInt("CustomServerInfo", "OpenBeaconSer") != 0)
	{
		// 开启
		config.setIsOpenBeaconSer(true);
		confFile->GetString("CustomServerInfo", "BeaconSerHost", temp);
		config.setBeaconSerHost(temp);
		confFile->GetString("CustomServerInfo", "BeaconSerUrl", temp);
		config.setBeaconSerUrl(temp);
		config.setBeaconSerPort(confFile->GetInt("CustomServerInfo", "BeaconSerPort"));
		config.setBeaconSerInterval(confFile->GetInt("CustomServerInfo", "BeaconSerInterval"));
	}
	else
	{
		// 没有开启
		config.setIsOpenBeaconSer(false);
	}
	delete confFile, confFile = NULL;

	// 提取版本号
	confFile = new CConfFile();
	if (!confFile->LoadFile(UpdateFilePath))
	{
		write_error_log("Open ini failure.");
		delete confFile;
		exit(1);
	}
	config.setVersion(confFile->GetInt("UpdateInfo", "UpdateVersion"));
	delete confFile, confFile = NULL;

	// 获取 LAN 口 IP 和 MAC 地址
	tempIP = config.GetIPByIfname(LanIfname);
	if (tempIP != 0)
	{
		config.setLanIP(UDPSOCKET::inetNtoA(tempIP));
		pMac = config.GetMacByIfName(LanIfname);
		config.setLanMAC(pMac);
		free(pMac), pMac = NULL;
	}
	
	// 获取 WAN 口 IP 和 MAC 地址，存在两种情况
	tempIP = config.GetIPByIfname(WiFiIfname);
	if (0 == tempIP)
	{
		// 网线模式
		tempIP = config.GetIPByIfname(WanIfname);
		config.setWanIP(UDPSOCKET::inetNtoA(tempIP));
		pMac = config.GetMacByIfName(WanIfname);
		config.setWanMAC(pMac);
		free(pMac), pMac = NULL;
	}
	else
	{
		// wifi apcli 模式
		config.setWanIP(UDPSOCKET::inetNtoA(tempIP));
		pMac = config.GetMacByIfName(WiFiIfname);
		config.setWanMAC(pMac);
		free(pMac), pMac = NULL;
	}

	// 加载phoneKey
	if (0 == access(DEF_PHONE_CONNECT_KEY, F_OK))
	{
		ReadFileNLenToMem(phoneKey, PHONE_KEY_LEN, DEF_PHONE_CONNECT_KEY);
		phoneKey[PHONE_KEY_LEN] = 0;
		config.setPhoneKey(phoneKey);
	}
	// 加载serilas
	if (0 == access(DEF_UID_FILE_PATH, F_OK))
	{
		confFile = new CConfFile();
		if (!confFile->LoadFile(DEF_UID_FILE_PATH))
		{
			write_error_log("Open ini failure.");
			delete confFile;
			exit(1);
		}
		confFile->GetString("Mark_cb_Info", "cb", temp);
		config.setSerials(temp);
		delete confFile, confFile = NULL;
	}
	else
	{
		config.setSerials("ff:ff:ff:ff:ff:ff:11:22:33:44:55:66");
	}

	write_log("WebServerHost=%s, Port=%d", 
		config.getWebDomain(), config.getWebPort());
	write_log("LocalPort=%d, BcastPort=%d, ListenNum=%d", 
		config.getLocalPort(), config.getBcastPort(), config.getListenNum());
	write_log("WanInfo: ifname=%s, IP=%s, MAC=%s", 
		WiFiIfname, config.getWanIP(), config.getWanMAC());
	write_log("LanInfo: ifname=%s, IP=%s, MAC=%s", 
		LanIfname, config.getLanIP(), config.getLanMAC());
	write_log("serials: %s", config.getSerials());
	return ;
}





static inline int SetPthreadStackSize(pthread_attr_t *attr, int size)
{
	int iRet;
	memset(attr, 0, sizeof(pthread_attr_t));
	iRet = pthread_attr_init(attr); /*初始化线程属性*/
	if (iRet != 0)
	{
		write_error_log("pthread attr init failed.");
		return -1;
	}
	iRet = pthread_attr_setstacksize(attr, size);
	if (iRet != 0)
	{
		write_error_log("pthread attr set stack size failed.");
		return -1;
	}
	return 0;
}


#if 1
int 
main(int argc, char** argv)
{
	int iRet = -1;
	pthread_attr_t attr;
	
	parseComLineAndConfFile(argc, argv);
	(void)signal(SIGPIPE, SIG_IGN);

	ListManage* net2SerHead = GetNet2SerHead();
	InitNet2SerList(net2SerHead);

	// 开启一个广播服务器线程
#ifdef BROADCAST
	pthread_t bcastID; 
	if ( 0 != SetPthreadStackSize(&attr, 1024*20) ) return -1;
	iRet = pthread_create(&bcastID, &attr, StartBcastID, NULL);
	if (iRet != 0)
	{
		write_error_log("Create Pthread Failed.");
		return 0;
	}
	pthread_detach(bcastID);
	pthread_attr_destroy(&attr);
#endif

#if 0
	// 开启第一个线程
	pthread_t bluetoothID;
	if ( 0 != SetPthreadStackSize(&attr, 1024*1024) ) return -1;
	iRet = pthread_create(&bluetoothID, &attr, NULL, NULL);
	if (iRet != 0)
	{
		write_error_log("Create Pthread Failed.");
		return 0;
	}
	pthread_detach(bluetoothID);
#endif

#ifdef WEBSERVER
	// 开启第二个线程，连接网络服务器
	pthread_t webServerID;
	//if ( 0 != SetPthreadStackSize(&attr, 1024*1024) ) return -1;
	iRet = pthread_create(&webServerID, 0, ConnectWebSeverProc, NULL);
	if (iRet != 0)
	{
		write_error_log("Create Pthread Failed.");
		return 0;
	}
	pthread_detach(webServerID);
	sleep(1);
#endif

#ifdef LOCALSOCKET
	// 开启第三个线程，本地监听服务器
	pthread_t localSocketID;
	//if ( 0 != SetPthreadStackSize(&attr, 1024*512) ) return -1;
	iRet = pthread_create(&localSocketID, 0, localServerPthreadProc, NULL);
	if (iRet != 0)
	{
		write_error_log("Create Pthread Failed.");
		return 0;
	}
	pthread_detach(localSocketID);
#endif

#ifdef UPLOADMACINFO
	// 搜集MAC地址信息
	if ( 0 != SetPthreadStackSize(&attr, 1024*20) ) return -1;
	pthread_t gathMacInfo, uploadMacInfoID;
	iRet = pthread_create(&gathMacInfo, 0, gathMacDataFromProc, NULL);
	if (iRet != 0)
	{
		write_error_log("Create Pthread Failed.");
		return 0;
	}
	pthread_detach(gathMacInfo);
	pthread_attr_destroy(&attr);

	sleep(2);   // 等待采集线程启动
	// 上传mac地址信息
	if ( 0 != SetPthreadStackSize(&attr, 1024*20) ) return -1;
	iRet = pthread_create(&uploadMacInfoID, 0, uploadMacInfo, NULL);
	if (iRet != 0)
	{
		write_error_log("Create Pthread Failed.");
		return 0;
	}
	pthread_detach(uploadMacInfoID);
	pthread_attr_destroy(&attr);
#endif

	// 主函数睡眠
	while(1)sleep(100);
	UninitNet2SerList(net2SerHead);
	return 0;
}
#endif

