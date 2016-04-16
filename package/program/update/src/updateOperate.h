#ifndef __UPDATE_OPERATE_H__
#define __UPDATE_OPERATE_H__

#include <stdio.h>


class UPDATE
{
public:
	char*      m_confFilePath;		// 配置文件
	char*      m_webDomain;		    // 服务器域名
	u_int16    m_WebPort;
	int        m_version;
	char*      m_UpdateFilePath;
	char*      m_serials;
	char*      m_URL;
	int        m_HardwareType;

public:
	UPDATE();
	~UPDATE();

	// 函数功能: 
	// 函数参数: 
	//           
	// 返 回 值: 
	int 
	HexStringtoInt(u_int8 *str, int *datalen, char* md5);

	// 函数功能: 获取脚本执行输出字符
	// 函数参数: cmd，执行的脚本命令
	//           output，输出的结果
	// 返 回 值: 成功返回读取到的字符数，失败返回-1
	int 
	GetShellCmdOutput(const char* cmd, char* Output, int OutputLen);

	// 函数功能: 解析输入的命令和读取配置文件
	// 函数参数: argc，argv，main 函数参数
	void 
	parseComLineAndConfFile(int argc, char **argv);

	// 函数功能: 拼接发送信息
	// 函数参数: httpHead，存储拼接后的post数据
	//           headLen，输出参数，拼接后的post数据长度
	// 返 回 值: 成功返回0，失败返回-1；
	int
	sprintSendInfo(char* httpHead, int *headLen);

	// 函数功能: 执行系统升级
	// 函数参数: kind，升级模式；
	//           filePath，下载路径
	//           md5，md5校验
	//           ver，版本号
	// 返 回 值: 成功返回0，失败返回-1
	int 
	updateSystem(char kind, const char* filePathA, const char* filePathB,const char* md5, int ver);
};



static inline void 
usage(const char* argv)
{
    printf("Usage: %s [options]\n", argv);
    printf("\n");
    printf("  -c [filename] Use this config file.\n");
    printf("  -h            Print usage.\n");
    printf("  -v            Print version information.\n");
	printf("  -P            Web sever port.\n");
	printf("  -H            Web server domain.\n");
	//printf("  -s            Client Serials.\n");
    printf("\n");
}

#endif /*__UPDATE_OPERATE_H__*/
