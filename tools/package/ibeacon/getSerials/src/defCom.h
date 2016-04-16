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





#ifndef SUCCESS
#define SUCCESS 0
#endif 
#ifndef FAILED
#define FAILED -1
#endif


#ifndef ASSERT
#define ASSERT		assert
#endif


typedef unsigned char    u_int8;
typedef unsigned short   u_int16;
typedef unsigned int     u_int32;
typedef unsigned long long u_int64;


#undef  u32tLen
#define u32tLen     (sizeof(u32t))
#undef  u16tLen
#define u16tLen     (sizeof(u16t))
#undef  u8tLen
#define u8tLen      (sizeof(u8t))



#undef  write_log
#ifdef  _DEBUG
#define write_log(fmt,args...) printf("[%s][%d]"fmt"\n",__FILE__,__LINE__,##args)
#else
#define write_log(fmt,args...)
#endif

#undef  write_error_log
#define write_error_log(fmt,args...)  printf("[%s][%d]Error:"fmt"\n",__FILE__,__LINE__,##args)
#undef  write_normal_log
#define write_normal_log(fmt,args...) printf("[%s][%d]Log:"fmt"\n",__FILE__,__LINE__,##args)




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



#define MARK_DEV_CFG_FILE_PATH	"./ssid.conf"	

#define MAX_FILE_NAME				512					// ����ļ�·����
#define DEF_CONFIG_FILE_PATH		"./cbProb.conf"		// Ĭ�ϵ������ļ�·��
#define DEF_PUBLIC_KEY_FILE         "./public.key"
#define DEF_RCVTIMEO				1000*30
#define DEF_SNDTIMEO 				1000*30
#define DEF_UID_LEN					(sizeof("1234567890")-1)

#define IBEACON_SERVER_IP			"www.ibeacon.com"	// ������ ip
#define IBEACON_SERVER_PORT			45838				// �������˿�
#define EVERY_PKG_LEN               1380







typedef struct _WebInfo
{
	const char* WebIP;
	u_int16 WebPort;
	int listenNum;
}WebInfo;





#endif /*__DEF_COM__H__*/

