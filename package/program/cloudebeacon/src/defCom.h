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


//说明:参数1,指向CLog的so文件句柄
//      参数2,Clog对象指针
//CreateInstance
typedef void * (*INI_CREATEINSTANCE)(void *,void *);
//说明:释放CreateInstance创造的对象实例
//DestroyInstance
typedef void (*INI_DESTROYINSTANCE)(void *);
//说明:参数1,指向对象实例
//      参数2,section名
//      参数3,注释
//CreateSection
typedef bool (*INI_CREATESECTION)(void *, std::string, std::string);
//说明:参数1,指向对象实例
//     参数2,指向key名
//     参数3,注释
//GetKey
typedef int (*INI_GETKEYINT)(void *,std::string, std::string);
//说明:参数1,指向对象实例
//      参数2,指向给定的key
//      参数3,指向给定的value
//      参数4,指向给定的注释
//      参数5,指向给定的section
//SetValue
typedef bool (*INI_SETVALUE)(void *, std::string, std::string, std::string, std::string);
//说明:参数1,指向对象实例
//      参数2,指向文件名
//Load
typedef bool (*INI_LOAD)(void *,std::string);//加载文件
//说明:参数1,指向对象实例
//Clear
typedef void (*INI_CLEAR)(void *);
//说明:参数1,指向对象实例
//     参数2,指向要赋值给ini操作类的文件名
//SetFileName
typedef void (*INI_SETFILENAME)(void *,std::string);
//说明:参数1,指向对象实例
//SectionCount
typedef int (*INI_SECTIONCOUNT)(void *);
//说明:参数1,指向对象实例
//KeyCount
typedef int (*INI_KEYCOUNT)(void *);
//说明:参数1,指向对象实例
//Save
typedef bool (*INI_SAVE)(void *);
//说明:参数1,指向对象实例
//      参数2,指向key
//      参数3,指向section
//GetString
typedef std::string (*INI_GETSTRING)(void *,std::string, std::string);
//说明:参数1,指向对象实例
//      参数2,指向key
//      参数3,指向section
//GetFloat
typedef float (*INI_GETFLOAT)(void *,std::string, std::string);
//说明:参数1,指向对象实例
//      参数2,指向key
//      参数3,指向section
//GetInt
typedef int (*INI_GETINT)(void *,std::string, std::string);
//说明:参数1,指向对象实例
//      参数2,指向key
//      参数3,指向section
//GetBool
typedef bool (*INI_GETBOOL)(void *,std::string, std::string);
//说明:参数1,指向对象实例
//      参数2,指向key
//      参数3,section
//GetKeyComment
typedef std::string (*INI_GETKEYCOMMENT)(void *,std::string, std::string);





#define MAX_FILE_NAME               512                                 // 最大文件路径名
#define LOG_FILE_PATH               "/root/beacon.log"					// 日志文件路径
#define MAX_SIZE_LOG_FILE           (1*256*1024)                        // 日志文件的最大大小
#define DEF_CONFIG_FILE_PATH        "/usr/ibeacon/conf/cbProb.conf"     // 默认的配置文件路径
#define DEF_PUBLIC_KEY_FILE         "/etc/.public.key"
#define DEF_GET_MAC_INFO_CMD        "cat /proc/probe_sta"
#define DEF_PHONE_CONNECT_KEY       "/etc/.phone.key"                   // 手机连接认证key
#define DEF_UID_FILE_PATH           "/usr/ibeacon/conf/ssid.conf"
#define DEF_BLU_CONF_FILE           "/usr/ibeacon/conf/ScanFormats.xml" // 蓝牙配置文件目录
#define DEF_BLU_BIN_FILE            "/tmp/blue.bin"                     // 蓝牙更新固件名字
#define DEF_RCVTIMEO                1000*30
#define DEF_SNDTIMEO                1000*30
#define DEF_UID_LEN                 (sizeof("11:22:33:44:55:66:aa:bb:cc:dd:ee:ff")-1)
#define PHONE_KEY_LEN               32
#define ACCOUNT_LEN                 32
#define WiFi_Conf_File_Path         "/etc/config/wireless"
#define Network_Conf_File           "/etc/config/network"
#define UPL_MAX_INTERVAL            300                 // 上传最大的时间间隔
#define UPL_MIN_INTERVAL            1                   // 上传最小的时间间隔
#define USB_DOWNLOAD_URL            "/download"
#define HTTP_PORT                   80
#define GET_WIFI_AP_INFO            "/usr/bin/aps"      // 获取wifi热点信息
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




#define GATH_MAC_TASK_ID     1      // 搜集mac地址任务
#define UPLD_MAC_TASK_ID     2      // 上传mac地址任务
#define UPDT_BTH_TASK_ID     3      // 更新蓝牙固件任务
#define CHEK_LOG_FILE_ID     4      // 检查日志文件大小任务
#define CHEK_NET_CONN_ID     5      // 检测网络是否已经连接

#define PRINT_COM_CNT1_ID     6
#define PRINT_COM_CNT10_ID    7






// 函数功能: 获取脚本执行输出字符
// 函数参数: cmd，执行的脚本命令
//           output，输出的结果
//           OutputLen，output缓冲区大小
// 返 回 值: 成功返回读取到的字符数，失败返回-1
int 
GetShellCmdOutput(const char* cmd, char* Output, int OutputLen);
// 函数功能: 十六进制的字符串转换为十进制的数字
// 输入参数: strHex十六进制的字符串, len,字符串长度
// 返 回 值: 十进制的数字
int StrHexToNumDec(const char* strHex, int len);
// 函数功能: 从文件中读取多长的数据到缓冲区中
// 输入参数: buff，缓冲区，buffLen，数据长度；
//           FilePath，文件；
// 返 回 值: 成功返回0，失败返回-1；
int
ReadFileNLenToMem(char* buff, const int buffLen, const char* FilePath);
// 函数功能: 将文件内容加载到内存中
// 输入参数: FilePath，文件；Size，文件大小，字节数
// 返 回 值: 成功返回加载的内容指针(记得自己释放内存)，失败返回NULL;
char*
LoadFileToMem(const char* FilePath, int* Size);
// 函数功能: 将数据写入到文件中
// 输入参数: buff，数据；buffLen，数据长度；
//           FilePath，文件
// 返 回 值: 成功返回0，失败返回-1
int
LoadMemToFile(const char* buff, const int buffLen, const char* FilePath);
// 函数功能: 以16进制的模式打印数据
// 输入参数: buf，打印的数据；len，数据长度；
void
print_hex(const unsigned char* buf, int len);
// 函数功能: 将字符类型的整数转换成整形数字，如(char*)"12345" = (int)12345
// 函数参数: str，传进去的字符串
// 返 回 值: 返回-1为失败
int
scan_int(char *str);
// 函数功能: 将进程设置为分离状态
// 函数参数: pidfile 文件路径
void
makePidFile(char *pidfile);
// 函数功能: 将进程设置为分离状态
void
detachPid();
// 函数功能: 计算left时间点比right时间点大多少
// 函数参数: dest，输出参数，返回时间点差
//           left，输入参数，左时间点
//           right，输入参数，右时间点
// 返 回 值: 无
void
diffTimeval(struct timeval *dest, struct timeval *left, struct timeval *right);
// 函数功能: 对比两个时间的大小
// 函数参数: tv1 输入参数，时间1
//           tv2 输入参数，时间2
// 返 回 值: tv1 > tv2 返回 1，tv1 == tv2 返回 0，tv1 < tv2 返回 -1；
int
cmpTimeval(struct timeval *tv1, struct timeval *tv2);
// 函数功能: 检查登录的账户密码(未加密)
// 函数参数: inLogName 登录名，inLogPasswd 未加密的登录密码
// 返 回 值: 成功返回0， 失败返回-1 
int 
checkLoginPasswd(const char* inLogName, const char* inLogPasswd);
// 函数功能: 获取登录账户的加密盐值
// 函数参数: inLogName，输入参数登录名字，
//           salt, 输出参数，加密的盐值
// 返 回 值: 成功返回0，失败返回-1
int 
getLoginSaltVal(const char* inLogName, char* salt);
// 函数功能: 检查登录的账户密码(加密)
// 函数参数: passwd 加密的登录密码
// 返 回 值: 成功返回0， 失败返回-1  
int
checkPasswd(const char* name, const char* passwd);
// 功	 能: 获取域名对应的ip
// 输入参数: pIP, 域名；
// 输出参数: lAddr 输出网络字节的ip
// 返 回 值: 成功返回0， 失败返回 -1
int
GetIPByDomain(const char* pDomain, unsigned int *lAddr);
// 功	能: 通过网卡名获取网卡IP
// 输入参数: ifname 网卡名字
// 返 回 值: 返回0失败
unsigned int
GetIPByIfname(const char *ifname);
// 功	能: 通过网卡名获取网卡 mac 地址
// 输入参数: ifname 网卡名字
// 返 回 值: 返回 MAC 地址，失败返回 NULL，返回的 MAC 地址要释放
char*
GetMacByIfName(const char *ifname);
// 函数功能: 获取所有有用网卡的基本信息
// 输入参数: 无
// 返 回 值: 返回一个json，需要自己释放这个json
char*
GetAllAvaildNetInfo();
// 函数功能: 获取lan口mac地址
// 返 回 值: 返回 MAC 地址，失败返回 NULL
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



// 函数功能: 获取随意一个wan口网关
// 返 回 值: 成功返回网关值，失败返回0
unsigned long GetRandWanGateway();
// 函数功能: 获取随意一个wan口网关
// 返 回 值: 成功返回网关值，失败返回0
unsigned long GetRandWanIP();



#endif /*__DEF_COM__H__*/

