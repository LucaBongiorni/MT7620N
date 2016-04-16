#ifndef __DEF_COM__H__
#define __DEF_COM__H__

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>


#ifndef SUCCESS
#define SUCCESS 0
#endif
#ifndef FAILED
#define FAILED -1
#endif
#ifndef ASSERT
#define ASSERT      assert
#endif


#ifndef u_int8
#define u_int8 unsigned char
#endif
#ifndef u_int16
#define u_int16 unsigned short
#endif
#ifndef u_int32
#define u_int32 unsigned int
#endif
#ifndef u_int64
#define u_int64 unsigned long long
#endif


#undef  u32tLen
#define u32tLen     (sizeof(u_int32))
#undef  u16tLen
#define u16tLen     (sizeof(u_int16))
#undef  u8tLen
#define u8tLen      (sizeof(u_int8))


#define write_log(fmt,args...) printf("[%s][%d]"fmt"\n",__FILE__,__LINE__,##args)
#undef  write_error_log
#define write_error_log(fmt,args...)  printf("[%s][%d]Error:"fmt"\n",__FILE__,__LINE__,##args)
#undef  write_normal_log
#define write_normal_log(fmt,args...) printf("[%s][%d]Log:"fmt"\n",__FILE__,__LINE__,##args)
#define ERROR_FILE_LOG  "/root/cloudBeacon.log"

#define write_log_to_file(fmt,args...)      do{ \
    time_t timep;           \
    char buff[32] = {0};    \
    struct tm tempTime;     \
    time(&timep);           \
    localtime_r(&timep, &tempTime);     \
    sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d", tempTime.tm_year+1900, tempTime.tm_mon+1, tempTime.tm_mday, \
        tempTime.tm_hour, tempTime.tm_min, tempTime.tm_sec);\
    FILE* logfd = fopen(ERROR_FILE_LOG, "a+"); \
    fprintf(logfd, "[%s]Log:"fmt"\n", buff, ##args);  \
    fclose(logfd);              \
}while(0)

#define Max(a, b)  ((a)>(b)?(a):(b))
#define Min(a, b)  ((a)<(b)?(a):(b))


//˵��:����1,ָ��CLog��so�ļ����
//      ����2,Clog����ָ��
//CreateInstance
typedef void * (*INI_CREATEINSTANCE)(void *,void *);
//˵��:�ͷ�CreateInstance����Ķ���ʵ��
//DestroyInstance
typedef void (*INI_DESTROYINSTANCE)(void *);
//˵��:����1,ָ�����ʵ��
//      ����2,section��
//      ����3,ע��
//CreateSection
typedef bool (*INI_CREATESECTION)(void *, std::string, std::string);
//˵��:����1,ָ�����ʵ��
//     ����2,ָ��key��
//     ����3,ע��
//GetKey
typedef int (*INI_GETKEYINT)(void *,std::string, std::string);
//˵��:����1,ָ�����ʵ��
//      ����2,ָ�������key
//      ����3,ָ�������value
//      ����4,ָ�������ע��
//      ����5,ָ�������section
//SetValue
typedef bool (*INI_SETVALUE)(void *, std::string, std::string, std::string, std::string);
//˵��:����1,ָ�����ʵ��
//      ����2,ָ���ļ���
//Load
typedef bool (*INI_LOAD)(void *,std::string);//�����ļ�
//˵��:����1,ָ�����ʵ��
//Clear
typedef void (*INI_CLEAR)(void *);
//˵��:����1,ָ�����ʵ��
//     ����2,ָ��Ҫ��ֵ��ini��������ļ���
//SetFileName
typedef void (*INI_SETFILENAME)(void *,std::string);
//˵��:����1,ָ�����ʵ��
//SectionCount
typedef int (*INI_SECTIONCOUNT)(void *);
//˵��:����1,ָ�����ʵ��
//KeyCount
typedef int (*INI_KEYCOUNT)(void *);
//˵��:����1,ָ�����ʵ��
//Save
typedef bool (*INI_SAVE)(void *);
//˵��:����1,ָ�����ʵ��
//      ����2,ָ��key
//      ����3,ָ��section
//GetString
typedef std::string (*INI_GETSTRING)(void *,std::string, std::string);
//˵��:����1,ָ�����ʵ��
//      ����2,ָ��key
//      ����3,ָ��section
//GetFloat
typedef float (*INI_GETFLOAT)(void *,std::string, std::string);
//˵��:����1,ָ�����ʵ��
//      ����2,ָ��key
//      ����3,ָ��section
//GetInt
typedef int (*INI_GETINT)(void *,std::string, std::string);
//˵��:����1,ָ�����ʵ��
//      ����2,ָ��key
//      ����3,ָ��section
//GetBool
typedef bool (*INI_GETBOOL)(void *,std::string, std::string);
//˵��:����1,ָ�����ʵ��
//      ����2,ָ��key
//      ����3,section
//GetKeyComment
typedef std::string (*INI_GETKEYCOMMENT)(void *,std::string, std::string);





#define MAX_FILE_NAME               512                                 // ����ļ�·����
#define LOG_FILE_PATH               "/root/beacon.log"					// ��־�ļ�·��
#define MAX_SIZE_LOG_FILE           (1*256*1024)                        // ��־�ļ�������С
#define DEF_CONFIG_FILE_PATH        "/usr/ibeacon/conf/cbProb.conf"     // Ĭ�ϵ������ļ�·��
#define DEF_PUBLIC_KEY_FILE         "/etc/.public.key"
#define DEF_GET_MAC_INFO_CMD        "cat /proc/probe_sta"
#define DEF_PHONE_CONNECT_KEY       "/etc/.phone.key"                   // �ֻ�������֤key
#define DEF_UID_FILE_PATH           "/usr/ibeacon/conf/ssid.conf"
#define DEF_BLU_CONF_FILE           "/usr/ibeacon/conf/ScanFormats.xml" // ���������ļ�Ŀ¼
#define DEF_BLU_BIN_FILE            "/tmp/blue.bin"                     // �������¹̼�����
#define DEF_RCVTIMEO                1000*30
#define DEF_SNDTIMEO                1000*30
#define DEF_UID_LEN                 (sizeof("11:22:33:44:55:66:aa:bb:cc:dd:ee:ff")-1)
#define PHONE_KEY_LEN               32
#define ACCOUNT_LEN                 32
#define WiFi_Conf_File_Path         "/etc/config/wireless"
#define Network_Conf_File           "/etc/config/network"
#define UPL_MAX_INTERVAL            300                 // �ϴ�����ʱ����
#define UPL_MIN_INTERVAL            1                   // �ϴ���С��ʱ����
#define USB_DOWNLOAD_URL            "/download"
#define HTTP_PORT                   80
#define GET_WIFI_AP_INFO            "/usr/bin/aps"      // ��ȡwifi�ȵ���Ϣ
#define UPDATE_CONF_FILE            "/usr/ibeacon/conf/ibeaconUpdate.conf"




#define DEF_MAC_LEN      (sizeof("00:00:00:00:00:00"))
#define DOMAIN_LEN       256
#define FILEPATH_LEN     256
#define SERIALS_LEN      128
#define URL_LEN          256
#define POST_HEAD_LEN    1024
#ifndef IFNAMESIZ
#define IFNAMESIZ        16
#endif


#ifdef USE_OPENWRT
#define LAN_IFNAME       "br-lan"
#else
#define LAN_IFNAME       "eth0"
#endif
#define LAN_IFNAME_LEN   (sizeof(LAN_IFNAME)-1)


#ifdef WIN32
#define sleep          Sleep
#define pthread_create(a, b, c, d) CreateThread(a, b, c, d, 0, NULL)
#endif




#define GATH_MAC_TASK_ID     1      // �Ѽ�mac��ַ����
#define UPLD_MAC_TASK_ID     2      // �ϴ�mac��ַ����
#define UPDT_BTH_TASK_ID     3      // ���������̼�����
#define CHEK_LOG_FILE_ID     4      // �����־�ļ���С����
#define CHEK_NET_CONN_ID     5      // ��������Ƿ��Ѿ�����

#define PRINT_COM_CNT1_ID     6
#define PRINT_COM_CNT10_ID    7






// ��������: ��ȡ�ű�ִ������ַ�
// ��������: cmd��ִ�еĽű�����
//           output������Ľ��
//           OutputLen��output��������С
// �� �� ֵ: �ɹ����ض�ȡ�����ַ�����ʧ�ܷ���-1
int 
GetShellCmdOutput(const char* cmd, char* Output, int OutputLen);
// ��������: ʮ�����Ƶ��ַ���ת��Ϊʮ���Ƶ�����
// �������: strHexʮ�����Ƶ��ַ���, len,�ַ�������
// �� �� ֵ: ʮ���Ƶ�����
int StrHexToNumDec(const char* strHex, int len);
// ��������: ���ļ��ж�ȡ�೤�����ݵ���������
// �������: buff����������buffLen�����ݳ��ȣ�
//           FilePath���ļ���
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1��
int
ReadFileNLenToMem(char* buff, const int buffLen, const char* FilePath);
// ��������: ���ļ����ݼ��ص��ڴ���
// �������: FilePath���ļ���Size���ļ���С���ֽ���
// �� �� ֵ: �ɹ����ؼ��ص�����ָ��(�ǵ��Լ��ͷ��ڴ�)��ʧ�ܷ���NULL;
char*
LoadFileToMem(const char* FilePath, int* Size);
// ��������: ������д�뵽�ļ���
// �������: buff�����ݣ�buffLen�����ݳ��ȣ�
//           FilePath���ļ�
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1
int
LoadMemToFile(const char* buff, const int buffLen, const char* FilePath);
// ��������: ��16���Ƶ�ģʽ��ӡ����
// �������: buf����ӡ�����ݣ�len�����ݳ��ȣ�
void
print_hex(const unsigned char* buf, int len);
// ��������: ���ַ����͵�����ת�����������֣���(char*)"12345" = (int)12345
// ��������: str������ȥ���ַ���
// �� �� ֵ: ����-1Ϊʧ��
int
scan_int(char *str);
// ��������: ����������Ϊ����״̬
// ��������: pidfile �ļ�·��
void
makePidFile(char *pidfile);
// ��������: ����������Ϊ����״̬
void
detachPid();
// ��������: ����leftʱ����rightʱ�������
// ��������: dest���������������ʱ����
//           left�������������ʱ���
//           right�������������ʱ���
// �� �� ֵ: ��
void
diffTimeval(struct timeval *dest, struct timeval *left, struct timeval *right);
// ��������: �Ա�����ʱ��Ĵ�С
// ��������: tv1 ���������ʱ��1
//           tv2 ���������ʱ��2
// �� �� ֵ: tv1 > tv2 ���� 1��tv1 == tv2 ���� 0��tv1 < tv2 ���� -1��
int
cmpTimeval(struct timeval *tv1, struct timeval *tv2);
// ��������: ����¼���˻�����(δ����)
// ��������: inLogName ��¼����inLogPasswd δ���ܵĵ�¼����
// �� �� ֵ: �ɹ�����0�� ʧ�ܷ���-1 
int 
checkLoginPasswd(const char* inLogName, const char* inLogPasswd);
// ��������: ��ȡ��¼�˻��ļ�����ֵ
// ��������: inLogName�����������¼���֣�
//           salt, ������������ܵ���ֵ
// �� �� ֵ: �ɹ�����0��ʧ�ܷ���-1
int 
getLoginSaltVal(const char* inLogName, char* salt);
// ��������: ����¼���˻�����(����)
// ��������: passwd ���ܵĵ�¼����
// �� �� ֵ: �ɹ�����0�� ʧ�ܷ���-1  
int
checkPasswd(const char* name, const char* passwd);
// ��	 ��: ��ȡ������Ӧ��ip
// �������: pIP, ������
// �������: lAddr ��������ֽڵ�ip
// �� �� ֵ: �ɹ�����0�� ʧ�ܷ��� -1
int
GetIPByDomain(const char* pDomain, unsigned int *lAddr);
// ��	��: ͨ����������ȡ����IP
// �������: ifname ��������
// �� �� ֵ: ����0ʧ��
unsigned int
GetIPByIfname(const char *ifname);
// ��	��: ͨ����������ȡ���� mac ��ַ
// �������: ifname ��������
// �� �� ֵ: ���� MAC ��ַ��ʧ�ܷ��� NULL�����ص� MAC ��ַҪ�ͷ�
char*
GetMacByIfName(const char *ifname);
// ��������: ��ȡ�������������Ļ�����Ϣ
// �������: ��
// �� �� ֵ: ����һ��json����Ҫ�Լ��ͷ����json
char*
GetAllAvaildNetInfo();
// ��������: ��ȡlan��mac��ַ
// �� �� ֵ: ���� MAC ��ַ��ʧ�ܷ��� NULL
static inline char* GetLanMac()
{
	static char mac[DEF_MAC_LEN] = {0};
	char* ptr = GetMacByIfName(LAN_IFNAME);
	if (!ptr) return NULL;
	memcpy(mac, ptr, strlen(ptr));
	free(ptr), ptr = NULL;
	return mac;
}
char* GetWanMac();
u_int32 GetWanIP();



// ��������: ��ȡ����һ��wan������
// �� �� ֵ: �ɹ���������ֵ��ʧ�ܷ���0
unsigned long GetRandWanGateway();
// ��������: ��ȡ����һ��wan������
// �� �� ֵ: �ɹ���������ֵ��ʧ�ܷ���0
unsigned long GetRandWanIP();



#endif /*__DEF_COM__H__*/

