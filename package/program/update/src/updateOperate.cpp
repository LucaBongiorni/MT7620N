#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>




#include "confile.h"
#include "updateOperate.h"
#include "cJSON.h"
//#include "md5.h"


int
execute(char *cmd_line)
{
	int pid, status, rc;

	const char *new_argv[4];
	new_argv[0] = "/bin/sh";
	new_argv[1] = "-c";
	new_argv[2] = cmd_line;
	new_argv[3] = NULL;

	pid = fork();
	if (pid == 0) 
	{   /* for the child process: */
		/* We don't want to see any errors if quiet flag is on */ 
		close(2);
		if (execvp("/bin/sh", (char *const *)new_argv) == -1) 
			printf("execvp(): %s", strerror(errno));
		else 
			printf("execvp() failed");
		exit(1);
	}

	/* for the parent:      */
	//printf("Waiting for PID %d to exit", pid);
	rc = waitpid(pid, &status, 0);
	//printf("Process PID %d exited", rc);

    return (WEXITSTATUS(status));
}





UPDATE::UPDATE()
{
	m_confFilePath = new char[MAX_FILE_NAME];
	if (m_confFilePath) memset(m_confFilePath, 0, MAX_FILE_NAME);

	m_webDomain = new char[MAX_FILE_NAME];
	if (m_webDomain) memset(m_webDomain, 0, MAX_FILE_NAME);
	
	m_UpdateFilePath = new char[MAX_FILE_NAME];
	if (m_UpdateFilePath) memset(m_UpdateFilePath, 0, MAX_FILE_NAME);
	
	m_serials = new char[32];
	if (m_serials) memset(m_serials, 0, 32);

	m_URL = new char[128];
	if (m_URL) memset(m_URL, 0, 128);
}


UPDATE::~UPDATE()
{
	if (m_confFilePath)
		delete [] m_confFilePath, m_confFilePath = NULL;
	if (m_webDomain)
		delete [] m_webDomain, m_webDomain = NULL;
	if (m_UpdateFilePath)
		delete [] m_UpdateFilePath, m_UpdateFilePath = NULL;
	if (m_serials)
		delete [] m_serials, m_serials = NULL;
	if (m_URL)
		delete [] m_URL, m_URL = NULL;
}




// 函数功能: 获取脚本执行输出字符
// 函数参数: cmd，执行的脚本命令
//           output，输出的结果
// 返 回 值: 成功返回读取到的字符数，失败返回-1
int 
UPDATE::GetShellCmdOutput(const char* cmd, char* Output, int OutputLen)
{
	if ( !cmd || !Output || OutputLen < 1)
	{
		 write_error_log("arguments error, szCmd is %s!", cmd);
		 return -1;
	}		

	int nRet = -1;
	FILE *pFile = NULL;   
	pFile = popen(cmd, "r");
	if (pFile)
	{
		nRet = fread(Output, sizeof(char), OutputLen, pFile);
		if ( nRet == 0)
		{
			pclose(pFile);
			pFile = NULL;
			return -1;
		}

		pclose(pFile);
		pFile = NULL;
	}
	return nRet;
}


int 
UPDATE::HexStringtoInt(unsigned char *str, int *datalen, char* md5)
{
	int i=0, j=0;
	unsigned char tmpstr[128] = {0};

	if (NULL == str || NULL == datalen)
	{
		return -1;
	}
	if (0 != *datalen % 2)
	{
		return -1;
	}

	for(i = 0; i < *datalen; i += 2)
	{
		if (str[i] >= '0' && str[i] <= '9')
		{
			str[i] = (str[i] - '0' ) * 16;
		}
		else if (str[i] <= 'f' && str[i] >= 'a')
		{
			str[i] = (str[i] - 'a' + 10) * 16;
		}
		else if (str[i] <= 'F' && str[i] >= 'A')
		{
			str[i]  = (str[i] - 'A' + 10) * 16;
		}

		if (str[i + 1] >= '0' && str[i + 1] <= '9')
		{
			str[i] += str[i + 1] - '0' ;
		}
		else if (str[i + 1] <= 'f' && str[i + 1] >= 'a')
		{
			str[i] += str[i + 1] - 'a' + 10;
		}
		else if (str[i + 1] <= 'F' && str[i + 1] >= 'A')
		{
			str[i]  += str[i + 1] - 'A' + 10;
		}
		tmpstr[j++] = str[i];
	}
	
	tmpstr[j] = '\0';
	*datalen =j;
	memcpy(md5, tmpstr, *datalen);
	md5[*datalen] = '\0';
	return 0;
}




// 函数功能: 解析输入的命令和读取配置文件
// 函数参数: argc，argv，main 函数参数
void 
UPDATE::parseComLineAndConfFile(int argc, char **argv)
{
    int c;
	bool webDomainFlag = false;
	bool confFilePathFlag = false;
	bool serialsFlag = false;
	char Buff[512] = {0};
	
	while (-1 != (c = getopt(argc, argv, "c:hvP:H:")))
    {
		switch(c) 
		{
			case 'h':
				usage(argv[0]);
				exit(1);
				break;
				
			case 'c':
				if (optarg) 
				{
					strncpy(m_confFilePath, optarg, MAX_FILE_NAME-1);
					confFilePathFlag = true;
				}
				break;
				
			case 'v':
				printf("This is cbProj version: %s %s\n", 
							__VERDATA__, __VERTIME__);
				exit(1);
				break;

			case 'P':
				if (optarg)
				{
					m_WebPort = atoi(optarg);
				}
				break;

			case 'H':
				if (optarg)
				{
					strncpy(m_webDomain, optarg, MAX_FILE_NAME-1);
					webDomainFlag = true;
				}
				break;
		#if 0
			case 's':
				if (optarg)
				{
					strncpy(m_serials, optarg, 32);
					serialsFlag = true;
				}
				break;
		#endif
		
			default:
				usage(argv[0]);
				exit(1);
				break;
		}
    }

	if (false == confFilePathFlag)
	{
		strncpy(m_confFilePath, DEF_CONFIG_FILE_PATH, MAX_FILE_NAME-1);
	}

	// 配置文件
	if (!CConfFile::Init())
	{
		printf("Init ini failure.");
		exit(1);
	}
	CConfFile* confFile = new CConfFile();
	// 加载配置文件
	if (!confFile->LoadFile(m_confFilePath))
	{
		write_error_log("Open ini failure.");
		delete confFile;
		exit(1);
	}
	
	// 获取域名
	if (false == webDomainFlag)
		confFile->GetString("UpdateInfo", "Host", m_webDomain);
	// 获取端口号
	if (m_WebPort == 0)
		m_WebPort = confFile->GetInt("UpdateInfo", "Port");
	// 获取 URL
	confFile->GetString("UpdateInfo", "URL", m_URL);
	// 获取升级版本
	m_version = confFile->GetInt("UpdateInfo", "UpdateVersion");
	// 获取升级固件存储路径
	confFile->GetString("UpdateInfo", "UpdateFilePath", m_UpdateFilePath);
	// 获取硬件类型
	m_HardwareType = confFile->GetInt("UpdateInfo", "hardwareType");
	delete confFile, confFile = NULL;

	// 序列号的获取方法应当是统一的，避免不同的方法更改序列号配置文件格式
	if (false == serialsFlag)
	{
		if ( -1 == GetShellCmdOutput(GET_SERIALS_CMD, Buff, 128))
		{
		    // 获取序列号失败，退出程序
			write_error_log("Get serials by popen failed.");
			exit(1);
		}
		// 提取序列号
		memcpy(m_serials, Buff, 32);
	}

	printf("WebServerHost=%s, Port=%d\n", m_webDomain, m_WebPort);
	printf("confFilePath=%s\n", m_confFilePath);
	printf("version=%d\n", m_version);
	printf("UpdateFilePath=%s\n", m_UpdateFilePath);
	printf("serials=%s\n", m_serials);
	return ;
}


int
UPDATE::sprintSendInfo(char* httpHead, int *headLen)
{
	// 拼接发送JSON
	cJSON *root;
	char *out;
	
	root = cJSON_CreateObject();
	// 硬件类型
	cJSON_AddNumberToObject(root, "hardwareType", m_HardwareType);
	// 版本号
	cJSON_AddNumberToObject(root, "firmwareNum", m_version);
	//cJSON_AddStringToObject(root, "updateVersion", m_version);
	out = cJSON_Print(root);	
	cJSON_Delete(root); 
	
	*headLen = snprintf(httpHead, 1024, 
					"POST %s HTTP/1.1\r\n"
					"HOST: %s:%d\r\n"
					//"Connection: Keep-Alive\r\n"
					"Content-Type: application/json;charset=utf-8\r\n"
					//"User-Agent: Apache-HttpClient/4.3.1 (java 1.5)\r\n"
					"Content-Length: %d\r\n\r\n"
					"%s",
					m_URL, m_webDomain, m_WebPort, strlen(out), out);
	free(out);	
	return 0;
}


// 函数功能: 更新系统
// 函数参数: 
// 返 回 值: 返回0 成功，
//           返回-1 文件下载失败
//           返回-2 文件格式不对
int 
UPDATE::updateSystem(char kind, const char* filePathA, const char* filePathB, 
		const char* md5, int ver)
{
	if (NULL == filePathA || NULL == md5 || NULL == filePathB)
	{
		return -1;
	}
	char cmd[512] = {0};
	int nRet = 0;
	char* pStr = NULL;
	//char outbuff[1024] = {0};
	//char buff[64] = {0};
	//int datalen;
	//int updatType = 0;

	// 判断版本是否为最新
	if (ver <= m_version)
	{
		write_log("The version is newest. No Upgrade.");
		return -1;
	}

	unlink(m_UpdateFilePath);
	sleep(1);
	// 拼接下载命令
	//sprintf(cmd, "echo \"wget -t100 -T30 -c %s -O %s\" >> /root/update.dat", filePathA, m_UpdateFilePath);
	//system(cmd);
	snprintf(cmd, sizeof(cmd), "/usr/bin/wget -t10 -T30 -c %s -O %s", filePathA, m_UpdateFilePath);
	//snprintf(cmd, sizeof(cmd), "/usr/ibeacon/tool/wgetUpdateFile.sh %s", filePathA);
	
	// 下载升级文件
	system(cmd);
	sleep(1);

	////////////////////////////////////////////////////
	// 检测文件是否已经下载
	if (0 != access(m_UpdateFilePath, F_OK))
	{
		write_log("Didn't download update file.");
		return -1;
	}

#if 0	
	/////////////////////////////////////////////////////
	// 检测文件格式是否符合要求
	memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd), "/sbin/sysupgrade -T %s", m_UpdateFilePath);
	nRet = GetShellCmdOutput(cmd, outbuff, sizeof(outbuff));
	if (nRet > 0) 
		updatType = 1;
	else
	{
		snprintf(cmd, sizeof(cmd), "tar -zxvf %s", m_UpdateFilePath);
		nRet = GetShellCmdOutput(cmd, outbuff, sizeof(outbuff));
		if (nRet > 0)
			updatType = 2;
	}
	
	if (updatType == 0)
		return -1;
#endif
#if 0		
	/////////////////////////////////////////////////////
	cmd[nRet] = 0;
	printf("%s\n", cmd);
	pStr = strstr(cmd, "returnVal");
	if (NULL == pStr)
	{
		write_log("update file format is error. returnVal=%s", pStr);
		return -2;
	}
	// 跳过returnVal和空格，提取返回值
	while(!isspace(*pStr))++pStr;
	while(isspace(*pStr))++pStr;
	if (*pStr != '0')
	{
		write_log("update file format is error. returnVal=%s", pStr);
		return -2;
	}
#endif
#if 0	
	/////////////////////////////////////////////////////
	// md5 校验
	MDFile(UPDATE_FILE_PATH, buff);
	if (memcmp(md5, buff, 16) != 0)
	{
		write_error_log("DownLoad File Error.");
		return -1;
	}
#endif
#if 0
	// 保存新的版本号
	CConfFile* confFile = new CConfFile();
	if (!confFile->LoadFile(m_confFilePath))
	{
		write_error_log("Open ini failure.");
		delete confFile;
		return -1;
	}
	char version[16] = {0};
	sprintf(version, "%d", ver);
	confFile->SetValue("UpdateInfo", "UpdateVersion", version, "#版本号");
	confFile->Save();
	delete confFile, confFile = NULL;
#endif

	// 执行升级命令
	write_normal_log("excl update shell script. updating version=%d ...", ver);
	sprintf(cmd, UPDATE_SHELL" %d", ver);
	system(cmd);
	write_normal_log("has been update system. reboot system.");
	return 0;
}




