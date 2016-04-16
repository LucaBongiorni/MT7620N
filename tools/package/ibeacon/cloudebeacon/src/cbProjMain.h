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

	// �ж��Ƿ��Ѿ�����
	bool  ifBeenActivated();

private:
	char* m_confFilePath;		   // �����ļ�
	char* m_webDomain;			   // ����������
	char* m_serials;               // ���к�
	char m_WanMAC[32];             // WiFi ���� mac ��ַ
	char m_LanMAC[32];             // �������� mac ��ַ
	char m_WanIP[16];     		   // wan ��IP
	char m_LanIP[16];     		   // LAN ��IP 
	unsigned short m_WebPort;      // �������˿�
	unsigned short m_LocalPort;    // ���ض˿�
	unsigned short m_BcastPort;    // ���ع㲥�����˿�
	unsigned short m_ListenNum;	   // ���������������Ӷ���������
	unsigned int   m_version;      // �汾��
	int m_MacInterval;             // tcp�ϴ�mac��ַ���ʱ��
	int m_BeaconInterval;          // tcp�ϴ�Beacon��Ϣ���ʱ��

	char* m_MacSerHost;            // �ͻ�mac��������ַ
	char* m_MacSerUrl;             // �ͻ�mac������url
	u_int16   m_MacSerPort;        // �ͻ�mac�������˿�
	int   m_MacSerInterval;        // �ͻ�mac���������ʱ��
	bool  m_isOpenMacSer;          // �Ƿ���Mac�������ϴ�

	char* m_BeaconSerHost;         // �ͻ�Beacon��������ַ
	char* m_BeaconSerUrl;          // �ͻ�Beacon������url
	u_int16   m_BeaconSerPort;     // �ͻ�Beacon�������˿�
	int   m_BeaconSerInterval;     // �ͻ�Beacon���������ʱ��
	bool  m_isOpenBeaconSer;       // �Ƿ���Beacon�������ϴ���Ϣ

	char m_PhoneKey[PHONE_KEY_LEN+1]; 

	pthread_rwlock_t m_rwLock;

	// ���ܺ���
public:
	// ��    ��: ��ȡ������Ӧ��ip
	// �������: pIP, ������
	// �������: lAddr ��������ֽڵ�ip
	// �� �� ֵ: �ɹ�����0�� ʧ�ܷ��� -1
	static int 
	GetIPByDomain(const char* pDomain, unsigned int *lAddr);

	// ��    ��: ͨ����������ȡ����IP
	// �������: ifname ��������
	// �� �� ֵ: ����0ʧ��
	static unsigned int 
	GetIPByIfname(const char *ifname);

	// ��    ��: ͨ����������ȡ���� mac ��ַ
	// �������: ifname ��������
	// �� �� ֵ: ���� MAC ��ַ��ʧ�ܷ��� NULL�����ص� MAC ��ַҪ�ͷ�
	static char* 
	GetMacByIfName(const char *ifname);

	pthread_mutex_t m_confFileLock;  // �����ļ���
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

// ��Ҫ�����̲߳����Ĵ���
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

