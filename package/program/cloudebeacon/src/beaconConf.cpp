#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <errno.h>
#include <iostream>

#ifdef WIN32
#include <Windows.h>
#pragma comment(lib, "pthreadVC2.lib")
#pragma comment(lib, "ws2_32.lib")
#else
#include <pthread.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#endif


#include "beaconConf.h"
#include "defCom.h"
#include "confile.h"
#include "cbProjMain.h"
#include "thread_pool.h"
#include "netPing.h"
#include "dealWithOpenwrt.h"


IbeaconConfig::IbeaconConfig()
{
	m_confFilePath = new char[FILEPATH_LEN];
	m_webDomain = new char[DOMAIN_LEN];
	m_serials   = new char[SERIALS_LEN];

	memset(m_confFilePath, 0, FILEPATH_LEN);
	memset(m_webDomain, 0, DOMAIN_LEN);
	memset(m_serials, 0, SERIALS_LEN);
	memset(m_WanMAC,  0, 32);
	memset(m_LanMAC,  0, 32);
	memset(m_LanIP,   0, 16);
	memset(m_WanIP,   0, 16);

	m_WebPort   = 0;  
	m_LocalPort = 0;
	m_ListenNum = 0;
	m_BcastPort = 0;
	m_MacInterval    = UPL_MIN_INTERVAL;
	m_BeaconInterval = UPL_MIN_INTERVAL;
	m_isStartScanDev = false;
	debuglevel  = 0;

	m_MacSerHost = new char[DOMAIN_LEN];
	m_MacSerUrl  = new char[URL_LEN];
	m_MacSerPort = 0;
	//m_MacSerInterval = 0;
	memset(m_MacSerHost, 0, DOMAIN_LEN);
	memset(m_MacSerUrl, 0, URL_LEN);
	m_isOpenMacSer = false;

	m_BeaconSerHost = new char[DOMAIN_LEN];
	m_BeaconSerUrl = new char[URL_LEN];
	m_BeaconSerPort = 0;
	//m_BeaconSerInterval = 0;
	memset(m_BeaconSerHost, 0, DOMAIN_LEN);
	memset(m_MacSerUrl, 0, URL_LEN);
	m_isOpenBeaconSer = false;

	//memset(lanIfname, 0, sizeof(lanIfname));
	//memset(wanIfname, 0, sizeof(wanIfname));
	//memset(wifiIfname, 0, sizeof(wifiIfname));
	//memset(wan3gIfname, 0, sizeof(wan3gIfname));

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
IbeaconConfig::setBeaconInterval(unsigned int BeaconInterval)
{
	if (BeaconInterval > UPL_MAX_INTERVAL) 
		BeaconInterval = UPL_MAX_INTERVAL;
	if (BeaconInterval < UPL_MIN_INTERVAL) 
		BeaconInterval = UPL_MIN_INTERVAL;
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


#ifndef WIN32


void IbeaconConfig::setWanIP(const char* wanIP)
{
	if (wanIP)
	{
		pthread_rwlock_wrlock(&m_rwLock);
		strncpy(m_WanIP, wanIP, 16);
		pthread_rwlock_unlock(&m_rwLock);
	}
}
const char* IbeaconConfig::getWanIP()
{
	static char wanIP[16];
	pthread_rwlock_rdlock(&m_rwLock);
	strncpy(wanIP, m_WanIP, 16);
	pthread_rwlock_unlock(&m_rwLock);
	return wanIP;
}

void IbeaconConfig::setWanMAC(const char* mac)
{
	if (mac)
	{
		pthread_rwlock_wrlock(&m_rwLock);
		strncpy(m_WanMAC, mac, 32);
		pthread_rwlock_unlock(&m_rwLock);
	}
}
const char* IbeaconConfig::getWanMAC()
{
	static char Mac[32];
	pthread_rwlock_rdlock(&m_rwLock);
	strncpy(Mac, m_WanMAC, 32);
	pthread_rwlock_unlock(&m_rwLock);
	return Mac;
}

void IbeaconConfig::setMacInterval(unsigned int MacInterval)
{
	if (MacInterval > UPL_MAX_INTERVAL)
		MacInterval = UPL_MAX_INTERVAL;
	if (MacInterval < UPL_MIN_INTERVAL)
		MacInterval = UPL_MIN_INTERVAL;
	pthread_rwlock_wrlock(&m_rwLock);
	m_MacInterval = MacInterval;
	pthread_rwlock_unlock(&m_rwLock);
}
int IbeaconConfig::getMacInterval()
{
	int temp;
	pthread_rwlock_rdlock(&m_rwLock);
	temp = m_MacInterval;
	pthread_rwlock_unlock(&m_rwLock);
	return temp;
}

void IbeaconConfig::setMacSerHost(const char* MacSerHost)
{
	if (MacSerHost)
	{
		pthread_rwlock_wrlock(&m_rwLock);
		strncpy(m_MacSerHost, MacSerHost, DOMAIN_LEN);
		pthread_rwlock_unlock(&m_rwLock);
	}
}
const char* IbeaconConfig::getMacSerHost()
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
	if (MacSerUrl)
	{
		pthread_rwlock_wrlock(&m_rwLock);
		strncpy(m_MacSerUrl, MacSerUrl, URL_LEN);
		pthread_rwlock_unlock(&m_rwLock);
	}
}
const char* IbeaconConfig::getMacSerUrl()
{
	static char url[URL_LEN];
	pthread_rwlock_rdlock(&m_rwLock);
	strncpy(url, m_MacSerUrl, URL_LEN);
	pthread_rwlock_unlock(&m_rwLock);
	return url;
}

void IbeaconConfig::setMacSerPort(u_int16 MacSerPort)
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_MacSerPort = MacSerPort;
	pthread_rwlock_unlock(&m_rwLock);
}
u_int16 IbeaconConfig::getMacSerPort()
{
	u_int16 temp;
	pthread_rwlock_rdlock(&m_rwLock);
	temp = m_MacSerPort;
	pthread_rwlock_unlock(&m_rwLock);
	return temp;
}



/*
void IbeaconConfig::setMacSerInterval(int MacSerInterval)
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_MacSerInterval = MacSerInterval;
	pthread_rwlock_unlock(&m_rwLock);
}
int IbeaconConfig::getMacSerInterval()
{
	int temp;
	pthread_rwlock_rdlock(&m_rwLock);
	temp = m_MacSerInterval;
	pthread_rwlock_unlock(&m_rwLock);
	return temp;
}
*/

void IbeaconConfig::setIsOpenMacSer(bool isOpenMacSer)
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_isOpenMacSer = isOpenMacSer;
	pthread_rwlock_unlock(&m_rwLock);
}
bool IbeaconConfig::getIsOpenMacSer()
{
	bool temp;
	pthread_rwlock_rdlock(&m_rwLock);
	temp = m_isOpenMacSer;
	pthread_rwlock_unlock(&m_rwLock);
	return temp;
}
#endif

void 
IbeaconConfig::setBeaconSerHost(const char* BeaconSerHost)
{
	if (BeaconSerHost)
	{
		pthread_rwlock_wrlock(&m_rwLock);
		strncpy(m_BeaconSerHost, BeaconSerHost, DOMAIN_LEN);
		pthread_rwlock_unlock(&m_rwLock);
	}
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
	if (BeaconSerUrl)
	{
		pthread_rwlock_wrlock(&m_rwLock);
		strncpy(m_BeaconSerUrl, BeaconSerUrl, URL_LEN);
		pthread_rwlock_unlock(&m_rwLock);
	}
}
const char* 
IbeaconConfig::getBeaconSerUrl()
{
	static char url[URL_LEN];;
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

/*
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
*/

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

bool 
IbeaconConfig::getTCPMacSerOpenVal()
{
	bool temp;
	pthread_rwlock_rdlock(&m_rwLock);
	temp = m_TCP_MacSerOpen;
	pthread_rwlock_unlock(&m_rwLock);
	return temp;
}
void 
IbeaconConfig::setTCPMacSerOpenVal(bool val)
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_TCP_MacSerOpen = val;
	pthread_rwlock_unlock(&m_rwLock);
}
bool 
IbeaconConfig::getTCPBeaconSerOpenVal()
{
	bool temp;
	pthread_rwlock_rdlock(&m_rwLock);
	temp = m_TCP_BeaconSerOpen;
	pthread_rwlock_unlock(&m_rwLock);
	return temp;
}
void 
IbeaconConfig::setTCPBeaconSerOpenVal(bool val)
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_TCP_BeaconSerOpen = val;
	pthread_rwlock_unlock(&m_rwLock);
}

void 
IbeaconConfig::setOpenStartScanDev()
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_isStartScanDev = true;
	pthread_rwlock_unlock(&m_rwLock);
}

void 
IbeaconConfig::setCloseStartScanDev()
{
	pthread_rwlock_wrlock(&m_rwLock);
	m_isStartScanDev = false;
	pthread_rwlock_unlock(&m_rwLock);
}



void 
IbeaconConfig::setIsStartScanDev(bool val)
{
	char tmp[4] = {0};
	//char aVal = val ? 1 : 0;
	//char bVal;
	pthread_rwlock_wrlock(&m_rwLock);
	//bVal = m_isStartScanDev ? 1 : 0;
	//write_normal_log("aVal=%d, bVal=%d", aVal, bVal);
	if (m_isStartScanDev != val)
	{
		m_isStartScanDev = val;
		pthread_rwlock_unlock(&m_rwLock);

		// 修改配置文件
		CConfFile* confFile = new CConfFile();
		lockConfFile();
		if (!confFile->LoadFile(getConfFilePath()))
		{
			UnlockConfFile();
			Debug(I_ERROR, "Open %s failure.", getConfFilePath());
			delete confFile, confFile = NULL;
			exit(1);
		}

		if (val)
		{
			tmp[0] = '1', tmp[1] = 0;
			confFile->SetValue("LocalServerInfo", "OpenStartScanDev", tmp, "#开启开始扫描设备信息");
		}
		else
		{
			tmp[0] = '0', tmp[1] = 0;
			confFile->SetValue("LocalServerInfo", "OpenStartScanDev", tmp, "#关闭开始扫描设备信息");
		}
		confFile->Save();
		UnlockConfFile();
		
		delete confFile, confFile = NULL;
		return ;
	}
	pthread_rwlock_unlock(&m_rwLock);
}

bool 
IbeaconConfig::getIsStartScanDev()
{
	bool tmp;
	pthread_rwlock_rdlock(&m_rwLock);
	tmp = m_isStartScanDev;
	pthread_rwlock_unlock(&m_rwLock);
	return tmp;
}



static pthread_mutex_t logFileMutex = PTHREAD_MUTEX_INITIALIZER;   // 这里应该使用文件锁，而不是互斥锁
void _debug(const char *function, int line, int level, const char *format, ...)
{    
	char buf[28];    
	va_list vlist;    
	FILE* logFd = NULL;
	time_t ts; 

	if (level == I_FATAL)
	{
		time(&ts);
		pthread_mutex_lock(&logFileMutex);
		logFd = fopen(LOG_FILE_PATH, "a+");
		fprintf(logFd, "[%.24s][%u](%s:%d) ", ctime_r(&ts, buf), getpid(), function, line);
		fprintf(stderr, "[%.24s][%u](%s:%d) ", ctime_r(&ts, buf), getpid(), function, line);
		va_start(vlist, format);
		vfprintf(logFd, format, vlist);
		vfprintf(stderr, format, vlist);
		va_end(vlist); 
		fputc('\n', logFd);
		fputc('\n', stderr);
		fclose(logFd);
		pthread_mutex_unlock(&logFileMutex);
		return ;
	}

	IbeaconConfig* conf = GetConfigPosition();  
	if (conf && level >= conf->debuglevel)    
	{   
		time(&ts);
		switch(level)
		{
		case I_DEBUG:
			fprintf(stderr, "[%d][%.24s][%u](%s:%d) ", 
				level, ctime_r(&ts, buf), getpid(), function, line);  
			va_start(vlist, format);            
			vfprintf(stderr, format, vlist);
			va_end(vlist);
			fputc('\n', stderr);
			break;
		case I_INFO:
		case I_WARN:
			fprintf(stdout, "[%d][%.24s][%u](%s:%d) ", 
				level, ctime_r(&ts, buf), getpid(), function, line);
			va_start(vlist, format);
			vfprintf(stdout, format, vlist);
			va_end(vlist); 
			fputc('\n', stdout);
			fflush(stdout);
			break;
		case I_ERROR:  // 打印到日志
			pthread_mutex_lock(&logFileMutex);
			logFd = fopen(LOG_FILE_PATH, "a+");
			fprintf(logFd, "[%.24s][%u](%s:%d) ", ctime_r(&ts, buf), getpid(), function, line);
			fprintf(stderr, "[%.24s][%u](%s:%d) ", ctime_r(&ts, buf), getpid(), function, line);
			va_start(vlist, format);
			vfprintf(logFd, format, vlist);
			vfprintf(stderr, format, vlist);
			va_end(vlist); 
			fputc('\n', logFd);
			fputc('\n', stderr);
			fclose(logFd);
			pthread_mutex_unlock(&logFileMutex);
			break;
		default:
			break;
		}
	}
}


// 检测目录文件大小，超过规定大小，去除超过部分加200行的长度
void* __checkLogFileSize(void* arg)
{
	int fileSize;
	char* pStr = NULL, *ptemp = NULL;
	FILE* logFp = NULL;
	int lineFlag = 0;
	int readLen = 0;
	int writeLen = 0;
	int tolLen = 0;
	struct stat file_info;
	memset(&file_info, 0, sizeof(file_info));
		
	pthread_mutex_lock(&logFileMutex);
	if (stat(LOG_FILE_PATH, &file_info) == -1) 
	{
		Debug(I_DEBUG, "stat file size failed.");
		goto Out;
	}
	fileSize = file_info.st_size;
	if (fileSize < MAX_SIZE_LOG_FILE) 
	{
		Debug(I_DEBUG, "the file size less than %d byte.", MAX_SIZE_LOG_FILE);
		goto Out;
	}

	logFp = fopen(LOG_FILE_PATH, "r+");
	if (!logFp) 
	{
		Debug(I_DEBUG, "fopen %s file failed.", LOG_FILE_PATH);
		goto Out;
	}
	pStr = (char*)malloc(MAX_SIZE_LOG_FILE);
	if (!pStr) 
	{
		Debug(I_DEBUG, "malloc failed.");
		goto Out;
	}

	// 跳过超出的部分长度
	fseek(logFp, fileSize-MAX_SIZE_LOG_FILE, SEEK_SET);

	// 再跳过200行
	while (!feof(logFp) && fgets(pStr, 4096, logFp) )
	{
		++lineFlag;
		if (200 == lineFlag) break;
	}
	if (200 != lineFlag) goto Out;

	// 将剩下的全部读取到内存中
	memset(pStr, 0, MAX_SIZE_LOG_FILE);
	ptemp = pStr;
	while(!feof(logFp))
	{
		if (tolLen+1024 > MAX_SIZE_LOG_FILE) break;
		readLen = fread(ptemp, 1, 1024, logFp);
		tolLen += readLen;
		ptemp  += readLen;
	}
	while(*ptemp != '\n' && *ptemp != '\r') --ptemp;
	++ptemp, *ptemp = 0;

	// 将文件位置移到开头
	fseek(logFp, 0, SEEK_SET);
	
	// 将读到内存的日志重新写入到文件
	tolLen = readLen = ptemp - pStr;
	ptemp = pStr;
	while(readLen > 0)
	{
		writeLen = fwrite(ptemp, 1, 1024, logFp);
		readLen -= writeLen;
		ptemp   += writeLen;
	}

	if (pStr) free(pStr), pStr = NULL;
	if (logFp) fclose(logFp);
	truncate(LOG_FILE_PATH, tolLen);
	pthread_mutex_unlock(&logFileMutex);
	return (void*)NULL;
	
Out:
	if (pStr) free(pStr), pStr = NULL;
	if (logFp) fclose(logFp);
	pthread_mutex_unlock(&logFileMutex);
	return (void*)NULL;
}


void checkLogFileSize(void* arg)
{
	// 使用线程池线程进行处理，提高轮询的有效性
	pool_add_worker(__checkLogFileSize, arg);
}



// 检测wifi连接是否已经成功
void* __checkNetConnect(void* arg)
{
	//static unsigned char flags = 0;
	const char* pDNS = "114.114.114.114";
	const char* host = NULL;
	char* gateway = NULL;
	unsigned long temp;
	int nRet;

	if (arg) host = (char*)arg;

	if (0 == ping_main(host, 0) )  // ping 域名，服务器
	{
		return (void*)NULL;
	}
	if (0 == ping_main(pDNS, 0))   // ping dns, 外网
	{
		return (void*)NULL;
	}

	// ping WAN口网关
	temp = GetRandWanGateway();
	gateway = inet_ntoa(*(struct in_addr*)&temp);
	if (ping_main(gateway, 0))
	{
		return (void*)NULL;
	}

	// 查看wifi连接
	if (checkWiFiCliSsidConnect() != 0)     // 不存在该wifi热点，无法联网
	{
		return (void*)NULL;
	}

	system("/etc/init.d/network restart");
	return (void*)NULL;
}

void checkNetConnect(void* arg)
{
	// 使用线程池线程进行处理，提高轮询的有效性
	pool_add_worker(__checkNetConnect, arg);
}

