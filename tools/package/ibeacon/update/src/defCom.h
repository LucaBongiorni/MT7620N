#ifndef __DEF_COM__H__
#define __DEF_COM__H__

#include <assert.h>
#include <cstring>
#include <vector>
#include <string>
#include <cstdio>


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

#undef  u32tLen
#define u32tLen     (sizeof(u_int32))
#undef  u16tLen
#define u16tLen     (sizeof(u_int16))
#undef  u8tLen
#define u8tLen      (sizeof(u_int8))



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


// Ĭ�Ϸ�����������Ϣ
#define UPDATE_DOMAIN  	"sdk.brtbeacon.com"
#define UPDATE_PORT		8183
#define UPDATE_URL		"/rest/version/deviceVersion"



#define UPDATE_FILESYS_VAL      '1'
#define UPDATE_KERNEL_VAL       '2'
#define UPDATE_UBOOT_VAL        '3'

// ���·�����Ӧ�ĸ��½ű�
#define UPDATE_SHELL            "/usr/ibeacon/tool/updateIBeacon.sh"


// ���ظ����ļ�Ĭ�ϱ����·��
#define UPDATE_FILE_PATH        "/tmp/IBeaconUpdateFile"   
// ���ظ����ļ�������
#define DOWNLOAD_UPDATEFILE_CMD "wget -t5 -T30 -c %s -O %s"
// ��ȡ���кŵ�����
#define GET_SERIALS_CMD			"/usr/ibeacon/tool/getSerials.sh"


#define MAX_FILE_NAME			256
// �����ļ�·��
#define DEF_CONFIG_FILE_PATH	"/usr/ibeacon/conf/ibeaconUpdate.conf"





















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







#endif /*__DEF_COM__H__*/

