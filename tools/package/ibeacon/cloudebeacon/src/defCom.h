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
#define ASSERT		assert
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
    fprintf(logfd, "[%s][%s][%d]Log:"fmt"\n", buff, __FILE__,__LINE__,##args);  \
    fclose(logfd);              \
}while(0)







#define Max(a, b)  ((a)>(b)?(a):(b))
#define Min(a, b)  ((a)<(b)?(a):(b))




//˵��:����1,ָ��CLog��so�ļ����
//		����2,Clog����ָ��
//CreateInstance
typedef void * (*INI_CREATEINSTANCE)(void *,void *);

//˵��:�ͷ�CreateInstance����Ķ���ʵ��
//DestroyInstance
typedef void (*INI_DESTROYINSTANCE)(void *);

//˵��:����1,ָ�����ʵ��
//		����2,section��
//		����3,ע��
//CreateSection
typedef bool (*INI_CREATESECTION)(void *, std::string, std::string);

//˵��:����1,ָ�����ʵ��
//	   ����2,ָ��key��
//	   ����3,ע��
//GetKey
typedef int (*INI_GETKEYINT)(void *,std::string, std::string);

//˵��:����1,ָ�����ʵ��
//		����2,ָ�������key
//		����3,ָ�������value
//		����4,ָ�������ע��
//		����5,ָ�������section
//SetValue
typedef bool (*INI_SETVALUE)(void *, std::string, std::string, std::string, std::string);

//˵��:����1,ָ�����ʵ��
//		����2,ָ���ļ���
//Load
typedef bool (*INI_LOAD)(void *,std::string);//�����ļ�

//˵��:����1,ָ�����ʵ��
//Clear
typedef void (*INI_CLEAR)(void *);

//˵��:����1,ָ�����ʵ��
//	   ����2,ָ��Ҫ��ֵ��ini��������ļ���
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
//		����2,ָ��key
//		����3,ָ��section
//GetString
typedef std::string (*INI_GETSTRING)(void *,std::string, std::string);

//˵��:����1,ָ�����ʵ��
//		����2,ָ��key
//		����3,ָ��section
//GetFloat
typedef float (*INI_GETFLOAT)(void *,std::string, std::string);

//˵��:����1,ָ�����ʵ��
//		����2,ָ��key
//		����3,ָ��section
//GetInt
typedef int (*INI_GETINT)(void *,std::string, std::string);

//˵��:����1,ָ�����ʵ��
//		����2,ָ��key
//		����3,ָ��section
//GetBool
typedef bool (*INI_GETBOOL)(void *,std::string, std::string);

//˵��:����1,ָ�����ʵ��
//		����2,ָ��key
//		����3,section
//GetKeyComment
typedef std::string (*INI_GETKEYCOMMENT)(void *,std::string, std::string);
















// ��������: �Ա�����ʱ��Ĵ�С
// ��������: tv1 ���������ʱ��1
//           tv2 ���������ʱ��2
// �� �� ֵ: tv1 > tv2 ���� 1��tv1 == tv2 ���� 0��tv1 < tv2 ���� -1��
static inline int
cmpTimeval(struct timeval *tv1, struct timeval *tv2)
{
    if (tv1->tv_sec < tv2->tv_sec)
		return -1;

    if (tv1->tv_sec > tv2->tv_sec)
		return 1;

    if (tv1->tv_usec < tv2->tv_usec)
		return -1;

    if (tv1->tv_usec > tv2->tv_usec)
		return 1;

    return 0;
}


// ��������: ����leftʱ����rightʱ������� 
// ��������: dest���������������ʱ����
//           left�������������ʱ���
//           right�������������ʱ���
// �� �� ֵ: ��
static inline void
diffTimeval(struct timeval *dest, struct timeval *left, struct timeval *right)
{
    if (  (left->tv_sec < right->tv_sec) || 
		  ((left->tv_sec == right->tv_sec) && (left->tv_usec < right->tv_usec))
		)
    {
		/* If left < right, just force to zero, don't allow negative numbers. */
		dest->tv_sec  = 0;
		dest->tv_usec = 0;
		return ;
    }

    dest->tv_sec  = left->tv_sec - right->tv_sec;
    dest->tv_usec = left->tv_usec - right->tv_usec;
    while (dest->tv_usec < 0)
	{
		dest->tv_usec += 1000000;
		dest->tv_sec--;
    }
	return ;
}

// ��������: ����������Ϊ����״̬
static inline void 
detachPid()
{
	int pid;

	/* Detach from the calling terminal. */
	if ((pid = fork()) > 0) 
	{
	    exit(0);
	}
	else if (pid < 0) 
	{
	    write_error_log("Error forking first fork: %s", strerror(errno));
	    exit(1);
	} 
	else 
	{
	    /* setsid() is necessary if we really want to demonize */
	    setsid();
	    /* Second fork to really deamonize me. */
	    if ((pid = fork()) > 0) 
		{
			exit(0);
	    } 
		else if (pid < 0) 
		{
			write_error_log("Error forking second fork: %s", strerror(errno));
			exit(1);
	    }
	}
}

// ��������: ����������Ϊ����״̬
// ��������: pidfile �ļ�·��
static inline void
makePidFile(char *pidfile)
{
    FILE *fpidfile;
    if (!pidfile)
	return;
    fpidfile = fopen(pidfile, "w");
    if (!fpidfile) 
	{
		write_error_log("Error opening pidfile '%s': %m, pidfile not created", pidfile);
		return;
    }
    fprintf(fpidfile, "%d\n", getpid());
    fclose(fpidfile);
}


// ��������: ���ַ����͵�����ת�����������֣���(char*)"12345" = (int)12345
// ��������: str������ȥ���ַ���
// �� �� ֵ: ����-1Ϊʧ��
static inline int
scan_int(char *str)
{
    int rv = 0;

    if (*str == '\0') 
	{
		return -1;
    }

    for (;;) 
	{
		if (*str >= '0' && *str <= '9') 
		{
			rv = (rv * 10) + ((*str) - '0');
		}
		else if (*str == '\0')
		{
			return rv;
		}
		else
		{
			return -1;
		}
		str++;
    }
    return rv;
}


static inline void 
print_hex(const unsigned char* buf, int len)
{
    int i;
    if (NULL == buf)
    {
        return;
    }
    for (i=0; i<len; ++i)
    {
        printf("%02x ", buf[i]);
        if ( ((i+1)%32 == 0) && i )
        {
            printf("\n");
        }
    }
    if (i)
    {
        printf("\n");
    }
    return ;
}


static inline int 
LoadMemToFile(const char* buff, const int buffLen, const char* FilePath)
{
    FILE *fd = fopen(FilePath, "w");
    if (NULL == fd)
    {
        printf("Open %s Failed.", FilePath);
        return -1;
    }
    int i = 0;
    i = fwrite(buff, buffLen, 1, fd);
    if (i != 1)
    {
        printf("Error: fwrite to file failed.\n");
        fclose(fd);
        return -1;
    }
    fclose(fd);
    return 0;
}

static inline int
LoadFileToMem(char** buff, const char* FilePath)
{
	struct stat sb;
	if ( -1 == stat(FilePath, &sb) )
	{
		write_error_log("stat file failed.");
		return -1;
	}
	int fileSize = sb.st_size+1;

	*buff = (char*)malloc(fileSize);
	if (NULL == *buff)
	{
		write_error_log("Malloc memory failed.");
		return -1;
	}
	memset(*buff, 0, fileSize);

	FILE *fd = fopen(FilePath, "r");
    if (NULL == fd)
    {
        printf("Open %s Failed.\n", FilePath);
		free(*buff), *buff = NULL;
        return -1;
    }

	int nRead = 0;
	int j = fileSize / 512;
	nRead = fread(*buff, 512, j+1, fd);
	if (nRead != j)
	{
		write_error_log("Error: fread to file failed.");
		free(*buff), *buff = NULL;
        return -1;
	}
    fclose(fd);
	*((*buff) + fileSize) = 0;
	return fileSize;
}

static inline int
ReadFileNLenToMem(char* buff, const int buffLen, const char* FilePath)
{
    FILE *fd = fopen(FilePath, "r");
    if (NULL == fd)
    {
        printf("Open %s Failed.", FilePath);
        return -1;
    }
    int i = 0;
    i = fread(buff, buffLen, 1, fd);
    if (i != 1)
    {
        printf("Error: fread to file failed.\n");
        fclose(fd);
        return -1;
    }
    fclose(fd);
    return 0;
}


#define MAX_FILE_NAME				512					// ����ļ�·����
#define DEF_CONFIG_FILE_PATH		"/usr/ibeacon/conf/cbProb.conf"		// Ĭ�ϵ������ļ�·��
#define DEF_PUBLIC_KEY_FILE         "/etc/.public.key"
#define DEF_GET_MAC_INFO_CMD        "cat /proc/probe_sta"
#define DEF_PHONE_CONNECT_KEY       "/etc/.phone.key"      // �ֻ�������֤key
#define DEF_UID_FILE_PATH           "/etc/ssid.conf"
#define DEF_BLU_CONF_DIR            "/usr/ibeacon/protfile/"     // ���������ļ�Ŀ¼
#define DEF_RCVTIMEO				1000*30
#define DEF_SNDTIMEO 				1000*30
#define DEF_UID_LEN					(sizeof("11:22:33:44:55:66:aa:bb:cc:dd:ee:ff")-1)

#define IBEACON_SERVER_IP			"www.ibeacon.com"	// ������ ip
#define IBEACON_SERVER_PORT			45838				// �������˿�
#define EVERY_PKG_LEN               1380
#define PHONE_KEY_LEN               32
#define ACCOUNT_LEN                 32


#define WiFi_Conf_File_Path         "/etc/config/wireless"



typedef struct _WebInfo
{
	const char* WebIP;
	u_int16 WebPort;
	int listenNum;
}WebInfo;





#endif /*__DEF_COM__H__*/

