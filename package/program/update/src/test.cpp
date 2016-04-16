#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "../defCom.h"



int 
pUpdateSystem(int ver)
{
	char cmd[512] = {0};

	char m_UpdateFilePath[1024];
	strcpy(m_UpdateFilePath, "/tmp/IBeaconUpdateFile");
	////////////////////////////////////////////////////
	// 检测文件是否已经下载
	if (0 != access(m_UpdateFilePath, F_OK))
	{
		write_log("Didn't download update file. %s", m_UpdateFilePath);
		return -1;
	}

	// 执行升级命令
	write_normal_log("excl update shell script. updating version=%d ...", ver);
	sprintf(cmd, UPDATE_SHELL" %d", ver);
	printf("cmd=%s\n", cmd);
	system(cmd);
	sleep(2);
	write_normal_log("has been update system. reboot system.");
	return 0;

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
}










int main()
{
	pUpdateSystem(2);
	return 0;
}



