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

	// �����ϴ�beaconɨ����Ϣʱ������(tcp��������http����������)
    void setBeaconInterval(unsigned int BeaconInterval);
	// ��ȡ�ϴ�beaconɨ����Ϣʱ������(tcp��������http����������)
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

	// �����ϴ�mac��ַʱ������(tcp��������http����������)
    void setMacInterval(unsigned int MacInterval);
	// ��ȡ�ϴ�mac��ַʱ������(tcp��������http����������)
    int  getMacInterval();

	// �����ϴ�mac��ַhttp����������
    void setMacSerHost(const char* MacSerHost);
	// ��ȡ�ϴ�mac��ַhttp����������
    const char* getMacSerHost();
	
	// �����ϴ�mac��ַhttp������url
    void setMacSerUrl(const char* MacSerUrl);
	// ��ȡ�ϴ�mac��ַhttp������url
    const char* getMacSerUrl();

	// �����ϴ�mac��ַhttp�������˿�
    void setMacSerPort(u_int16 MacSerPort);
	// ��ȡ�ϴ�mac��ַhttp�������˿�
    u_int16 getMacSerPort();
#endif


	// �����ϴ�mac��ַʱ������(http������)
    //void setMacSerInterval(int MacSerInterval);
    // ��ȡ�ϴ�mac��ַʱ������(http������)
    //int getMacSerInterval();


	// �����ϴ�beaconɨ����Ϣ����������(http������)
    void setBeaconSerHost(const char* BeaconSerHost);
	// ��ȡ�ϴ�beaconɨ����Ϣ����������(http������)
    const char* getBeaconSerHost();

	// �����ϴ�beaconɨ����Ϣ������url(http������)
    void setBeaconSerUrl(const char* BeaconSerUrl);
	// ��ȡ�ϴ�beaconɨ����Ϣ������url(http������)
    const char* getBeaconSerUrl();

	// �����ϴ�beaconɨ����Ϣ�������˿�(http������)
    void setBeaconSerPort(u_int16 BeaconSerPort);
	// ��ȡ�ϴ�beaconɨ����Ϣ�������˿�(http������)
    u_int16 getBeaconSerPort();

	// �����ϴ�beaconɨ����Ϣ������ʱ����(http������)
	//void setBeaconSerInterval(int BeaconSerInterval);
	// ��ȡ�ϴ�beaconɨ����Ϣ������ʱ����(http������)
    //int getBeaconSerInterval();

#ifndef WIN32
	// ���ÿ���/�ر��ϴ�mac��ַ����(http������)
    void setIsOpenMacSer(bool isOpenMacSer);
	// ��ȡ����/�ر��ϴ�mac��ַ����(http������)
    bool getIsOpenMacSer();

	// ���ÿ���/�ر��ϴ�mac��ַ����(tcp������)
	void setTCPMacSerOpenVal(bool val);
	// ��ȡ����/�ر��ϴ�mac��ַ����(tcp������)
	bool getTCPMacSerOpenVal();
#endif
	
	// ���ÿ���/�ر��ϴ�beaconɨ����Ϣ����(http������)
    void setIsOpenBeaconSer(bool isOpenBeaconSer);
	// ��ȡ����/�ر��ϴ�beaconɨ����Ϣ����(http������)
    bool getIsOpenBeaconSer();


	// ���ÿ���/�ر��ϴ�beaconɨ����Ϣ����(tcp������)
	void setTCPBeaconSerOpenVal(bool val);
	// ��ȡ����/�ر��ϴ�beaconɨ����Ϣ����(tcp������)
	bool getTCPBeaconSerOpenVal();


	// ���ÿ���/�رտ�ʼɨ��beacon��Ϣ
	void setIsStartScanDev(bool val);
	// ��ȡ����/�رտ�ʼɨ��beacon��Ϣ
	bool getIsStartScanDev();

    char* getPhoneKey(){return m_PhoneKey;}
    void  setPhoneKey(const char* key);
    // �ж��Ƿ��Ѿ�����
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
	char  m_ComName[32];           // ��������
    char* m_confFilePath;          // �����ļ�
    char* m_webDomain;             // ����������
    char* m_serials;               // ���к�
    char m_WanMAC[32];             // WiFi ���� mac ��ַ
    char m_LanMAC[32];             // �������� mac ��ַ
    char m_WanIP[16];              // wan ��IP
    char m_LanIP[16];              // LAN ��IP
    unsigned short m_WebPort;      // �������˿�
    unsigned short m_LocalPort;    // ���ض˿�
    unsigned short m_BcastPort;    // ���ع㲥�����˿�
    unsigned short m_ListenNum;    // ���������������Ӷ���������
    unsigned int   m_version;      // �汾��

	bool m_TCP_MacSerOpen;		   // �Ƿ��ϴ�mac��ַ��Ϣ�� tcp ��������
    //bool m_isUploadMacInfo;      // �Ƿ��ϴ�mac��ַ��Ϣ����������
    bool m_TCP_BeaconSerOpen;      // �Ƿ��ϴ�beacon��Ϣ�� tcp ��������
    //bool m_isUploadBeaconInfo;   // �Ƿ��ϴ�beacon��Ϣ����������

	int m_MacInterval;             // �ϴ�mac��ַ���ʱ��(tcp��http����������)
    int m_BeaconInterval;          // �ϴ�Beacon��Ϣ���ʱ��(tcp��http����������)
    
    char* m_MacSerHost;            // �ͻ�mac��������ַ
    char* m_MacSerUrl;             // �ͻ�mac������url
    u_int16   m_MacSerPort;        // �ͻ�mac�������˿�
    //int m_MacSerInterval;        // �ͻ�mac���������ʱ��
    bool  m_isOpenMacSer;          // �Ƿ���ͻ�Mac�������ϴ�
    char* m_BeaconSerHost;         // �ͻ�Beacon��������ַ
    char* m_BeaconSerUrl;          // �ͻ�Beacon������url
    u_int16 m_BeaconSerPort;     	// �ͻ�Beacon�������˿�
    bool  m_isOpenBeaconSer;       // �Ƿ���ͻ�Beacon�������ϴ���Ϣ
    
    bool  m_isStartScanDev;        // �Ƿ���ɨ���豸��Ϣ
    char  m_PhoneKey[PHONE_KEY_LEN+1];
    pthread_rwlock_t m_rwLock;

	// ���������̼���Ҫ������
	char m_updateBlueBinHost[64];
	u_int16 m_updateBlueBinPort;
	char m_updateBlueBinUrl[128];
	
	
public:
    //char wanIfname[16];
    //char wifiIfname[16];
	//char wan3gIfname[16];
    //char lanIfname[16];
	char debuglevel;
    // ���ܺ���

public:
    pthread_mutex_t m_confFileLock;  // �����ļ���
};




void _debug(const char *function, int line, int level, const char *format, ...);
void checkLogFileSize(void* arg);


void checkNetConnect(void* arg);



#endif /*__BEACON_CONF__H__*/

