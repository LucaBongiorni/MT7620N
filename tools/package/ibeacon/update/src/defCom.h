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


// 默认服务器配置信息
#define UPDATE_DOMAIN  	"sdk.brtbeacon.com"
#define UPDATE_PORT		8183
#define UPDATE_URL		"/rest/version/deviceVersion"



#define UPDATE_FILESYS_VAL      '1'
#define UPDATE_KERNEL_VAL       '2'
#define UPDATE_UBOOT_VAL        '3'

// 更新方法对应的更新脚本
#define UPDATE_SHELL            "/usr/ibeacon/tool/updateIBeacon.sh"


// 下载更新文件默认保存的路径
#define UPDATE_FILE_PATH        "/tmp/IBeaconUpdateFile"   
// 下载更新文件的命令
#define DOWNLOAD_UPDATEFILE_CMD "wget -t5 -T30 -c %s -O %s"
// 获取序列号的命令
#define GET_SERIALS_CMD			"/usr/ibeacon/tool/getSerials.sh"


#define MAX_FILE_NAME			256
// 配置文件路径
#define DEF_CONFIG_FILE_PATH	"/usr/ibeacon/conf/ibeaconUpdate.conf"





















//说明:参数1,指向CLog的so文件句柄
//		参数2,Clog对象指针
//CreateInstance
typedef void * (*INI_CREATEINSTANCE)(void *,void *);

//说明:释放CreateInstance创造的对象实例
//DestroyInstance
typedef void (*INI_DESTROYINSTANCE)(void *);

//说明:参数1,指向对象实例
//		参数2,section名
//		参数3,注释
//CreateSection
typedef bool (*INI_CREATESECTION)(void *, std::string, std::string);

//说明:参数1,指向对象实例
//	   参数2,指向key名
//	   参数3,注释
//GetKey
typedef int (*INI_GETKEYINT)(void *,std::string, std::string);

//说明:参数1,指向对象实例
//		参数2,指向给定的key
//		参数3,指向给定的value
//		参数4,指向给定的注释
//		参数5,指向给定的section
//SetValue
typedef bool (*INI_SETVALUE)(void *, std::string, std::string, std::string, std::string);

//说明:参数1,指向对象实例
//		参数2,指向文件名
//Load
typedef bool (*INI_LOAD)(void *,std::string);//加载文件

//说明:参数1,指向对象实例
//Clear
typedef void (*INI_CLEAR)(void *);

//说明:参数1,指向对象实例
//	   参数2,指向要赋值给ini操作类的文件名
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
//		参数2,指向key
//		参数3,指向section
//GetString
typedef std::string (*INI_GETSTRING)(void *,std::string, std::string);

//说明:参数1,指向对象实例
//		参数2,指向key
//		参数3,指向section
//GetFloat
typedef float (*INI_GETFLOAT)(void *,std::string, std::string);

//说明:参数1,指向对象实例
//		参数2,指向key
//		参数3,指向section
//GetInt
typedef int (*INI_GETINT)(void *,std::string, std::string);

//说明:参数1,指向对象实例
//		参数2,指向key
//		参数3,指向section
//GetBool
typedef bool (*INI_GETBOOL)(void *,std::string, std::string);

//说明:参数1,指向对象实例
//		参数2,指向key
//		参数3,section
//GetKeyComment
typedef std::string (*INI_GETKEYCOMMENT)(void *,std::string, std::string);







#endif /*__DEF_COM__H__*/

