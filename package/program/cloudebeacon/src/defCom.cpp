#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <shadow.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdarg.h>


#include "defCom.h"
#include "cJSON.h"


// 函数功能: 对比两个时间的大小
// 函数参数: tv1 输入参数，时间1
//           tv2 输入参数，时间2
// 返 回 值: tv1 > tv2 返回 1，tv1 == tv2 返回 0，tv1 < tv2 返回 -1；
int
cmpTimeval(struct timeval *tv1, struct timeval *tv2)
{
    if (tv1->tv_sec < tv2->tv_sec)
    {
        return -1;
    }
    if (tv1->tv_sec > tv2->tv_sec)
    {
        return 1;
    }
    if (tv1->tv_usec < tv2->tv_usec)
    {
        return -1;
    }
    if (tv1->tv_usec > tv2->tv_usec)
    {
        return 1;
    }
    return 0;
}


// 函数功能: 计算left时间点比right时间点大多少
// 函数参数: dest，输出参数，返回时间点差
//           left，输入参数，左时间点
//           right，输入参数，右时间点
// 返 回 值: 无
void
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


// 函数功能: 将进程设置为分离状态
void
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
        printf("Error forking first fork: %s", strerror(errno));
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
            printf("Error forking second fork: %s", strerror(errno));
            exit(1);
        }
    }
}


// 函数功能: 将进程设置为分离状态
// 函数参数: pidfile 文件路径
void
makePidFile(char *pidfile)
{
    FILE *fpidfile;
    if (!pidfile)
    {
        return;
    }
    fpidfile = fopen(pidfile, "w");
    if (!fpidfile)
    {
        printf("Error opening pidfile '%s': %m, pidfile not created", pidfile);
        return;
    }
    fprintf(fpidfile, "%d\n", getpid());
    fclose(fpidfile);
}


// 函数功能: 将字符类型的整数转换成整形数字，如(char*)"12345" = (int)12345
// 函数参数: str，传进去的字符串
// 返 回 值: 返回-1为失败
int
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


void
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


int
LoadMemToFile(const char* buff, const int buffLen, const char* FilePath)
{
    FILE *fd = fopen(FilePath, "w");  // 如果文件存在，会干掉这个文件
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



char*
LoadFileToMem(const char* FilePath, int* Size)
{
    struct stat sb;
    if ( -1 == stat(FilePath, &sb) )
    {
        printf("stat file failed.\n");
		*Size = 0;
        return NULL;
    }
	int flag = 0;
	// 申请内存
	char* buff = NULL;
    int fileSize = sb.st_size+1;
    buff = (char*)malloc(fileSize);
    if (NULL == buff)
    {
        printf("Malloc memory failed.\n");
		*Size = 0;
        return NULL;
    }
    memset(buff, 0, fileSize);
	*Size = fileSize; --fileSize;
	// 打开文件
    FILE *fd = fopen(FilePath, "r");
    if (NULL == fd)
    {
        printf("Open %s Failed.\n", FilePath);
        if (buff) free(buff), buff = NULL;
		*Size = 0;
        return NULL;
    }
	// 每次读512个字节，读多少次。
    int nRead = 0;	
    int cnt = fileSize / 512;
	if (fileSize % 512) 
	{
		cnt++; 
		flag = 1;
	}

	// 读取数据	
    nRead = fread(buff, 512, cnt, fd);
	if (1 == flag) ++nRead;
    if (nRead != cnt)
    {
        printf("[%s:%d]Error: fread to file failed. nRead=%d, cnt=%d\n", __FILE__, __LINE__, nRead, cnt);
        if (buff) free(buff), buff = NULL;
		*Size = 0;
		fclose(fd);
        return NULL;
    }
	// 关闭文件，返回数据
    fclose(fd);
    *(buff + fileSize) = 0;
    return buff;
}


int
ReadFileNLenToMem(char* buff, const int buffLen, const char* FilePath)
{
    FILE *fd = fopen(FilePath, "r");
    if (NULL == fd)
    {
        printf("Open %s Failed.\n", FilePath);
        return -1;
    }
    int i = 0;
    i = fread(buff, buffLen, 1, fd);
    if (i != 1 && i != 0)
    {
        printf("[%s:%d]Error: fread from file(%s) failed. i=%d\n",
			__FILE__, __LINE__, FilePath, i);
        fclose(fd);
        return -1;
    }
    fclose(fd);
    return 0;
}

// 函数功能: 十六进制的字符串转换为十进制的数字
// 输入参数: strHex十六进制的字符串, len,字符串长度
// 返 回 值: 十进制的数字
int StrHexToNumDec(const char* strHex, int len)
{
	if (NULL == strHex) return 0;
	
	const char *p = strHex;
	int i = 0, j = 0, k = 0;
	int y = len;

	if ( 0 == strncasecmp(p, "0x", 2) )p += 2, y -= 2;	
	while( *p != '\0' )
	{
		if (*p >= '0' && *p <= '9')
			i = *p - '0';
		else if (*p >= 'a' && *p <= 'f')
			i = *p - 'a' + 10;
		else if (*p >= 'A' && *p <= 'F')
			i = *p - 'A' + 10;
		else 
			return 0;
		
		for (j=1; j<y; ++j)
		{
			i *= 16;
		}
		k += i;
		--y;
		++p;
	}
	return k;
}


// 函数功能: 获取脚本执行输出字符
// 函数参数: cmd，执行的脚本命令
//           output，输出的结果
//           OutputLen，output缓冲区大小
// 返 回 值: 成功返回读取到的字符数，失败返回-1
int 
GetShellCmdOutput(const char* cmd, char* Output, int OutputLen)
{
	if ( !cmd || !Output || OutputLen < 1)
	{
		 printf("[%s:%d]arguments error, szCmd is %s!", __FUNCTION__, __LINE__, cmd);
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


static inline void
getSalt(char *outSalt, char *inPasswd)
{
	int i,j;
	if ((!inPasswd) || (!outSalt)) return ;
	
	//取出salt,i记录密码字符下标,j记录$出现次数
	for (i=0,j=0; inPasswd[i] && j != 3;++i)
	{
		if (inPasswd[i] == '$') ++j;
	}
	strncpy(outSalt, inPasswd, i-1);
}

int 
checkLoginPasswd(const char* inLogName, const char* inLogPasswd)
{
	char salt[512] = {0};
	char psswd[512] = {0};
	const char* pStr = NULL;
	struct spwd *sp = getspnam(inLogName);

	getSalt(salt, sp->sp_pwdp);
	pStr = crypt(inLogPasswd, salt);  // 使用盐值加密登录密码
	if (pStr)
		strncpy(psswd, pStr, sizeof(psswd));

	if (strcmp(psswd, sp->sp_pwdp) == 0)
		return 0;
	return -1;
}

int getLoginSaltVal(const char* inLogName, char* salt)
{
	if (!salt ) return -1;

	struct spwd *sp = getspnam(inLogName);
	getSalt(salt, sp->sp_pwdp);
	return 0;
}

int
checkPasswd(const char* name, const char* passwd)
{
	struct spwd *sp = getspnam(name);
	if (strcmp(passwd, sp->sp_pwdp) == 0)
		return 0;
	return -1;
}





u_int32 
GetIPByIfname(const char *ifname)
{
    struct ifreq if_data;
    int sockd;
    u_int32 ip = 0;
    if ((sockd = socket(AF_INET, SOCK_PACKET, htons(0x8086))) < 0)
    {
        printf("[%s:%d]Create Socket Failed. %m", __FUNCTION__, __LINE__);
        return 0;
    }
    /* Get IP of internal interface */
    strcpy(if_data.ifr_name, ifname);
	
    /* Get the IP address */
    if (ioctl(sockd, SIOCGIFADDR, &if_data) < 0)
    {
        printf("[%s:%d]Ioctl Get Socket Ifaddr Failed. %m", __FUNCTION__, __LINE__);
        close(sockd);
        return 0;
    }
    memcpy((void*)&ip, (char*)&if_data.ifr_addr.sa_data + 2, 4);
    close(sockd);
    return ip;
}

char* 
GetMacByIfName(const char *ifname)
{
	int r;
	int sockd;
	struct ifreq ifr;
	char *hwaddr, mac[32];

	if (ifname == NULL || strlen(ifname) > (IFNAMESIZ-1))
	{
		printf("[%s:%d]Param Error.", __FILE__, __LINE__);
		return NULL;
	}
	
	if (-1 == (sockd = socket(PF_INET, SOCK_DGRAM, 0))) 
	{
		printf("[%s:%d]Create Socket Failed.", __FILE__, __LINE__);
		return NULL;
	}
	
	strcpy(ifr.ifr_name, ifname);
	r = ioctl(sockd, SIOCGIFHWADDR, &ifr);
	if (r == -1) 
	{
		printf("[%s:%d]Ioctl Get Socket Ifaddr Failed.", __FILE__, __LINE__);
		close(sockd);
		return NULL;
	}
	hwaddr = ifr.ifr_hwaddr.sa_data;
	close(sockd);
	
	snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X", 
 			hwaddr[0] & 0xFF, hwaddr[1] & 0xFF, hwaddr[2] & 0xFF, 
 			hwaddr[3] & 0xFF, hwaddr[4] & 0xFF, hwaddr[5] & 0xFF);
	return strdup(mac);
}

int 
GetIPByDomain(const char* pDomain, unsigned int *lAddr)
{
    struct hostent *hp = NULL;
    if ( (hp = gethostbyname(pDomain)) != NULL )
    {
        if (hp->h_addr_list[0])
        {
            *lAddr = *((unsigned int*) hp->h_addr_list[0]);
            return SUCCESS;
        }
    }
    return FAILED;
}

char* GetWanMac()
{
	static char Mac[DEF_MAC_LEN] = {0};
	char* temp = NULL;
	memset(Mac, 0, sizeof(Mac));
	
	temp = GetMacByIfName("apcli0");
	if (temp) 
	{
		strncpy(Mac, temp, sizeof(Mac));
		free(temp), temp = NULL;
		return Mac;
	}
	temp = GetMacByIfName("eth0.2");
	if (temp) 
	{
		strncpy(Mac, temp, sizeof(Mac));
		free(temp), temp = NULL;
		return Mac;
	}
	temp = GetMacByIfName("3g-wan");
	if (temp)
	{
		strncpy(Mac, temp, sizeof(Mac));
		free(temp), temp = NULL;
		return Mac;
	}
	return NULL;
}

u_int32 GetWanIP()
{
	u_int32 temp = 0;

	temp = GetIPByIfname("apcli0");
	if (0 != temp) return temp;
	temp = GetIPByIfname("eth0.2");
	if (0 != temp) return temp;
	temp = GetIPByIfname("3g-wan");
	if (0 != temp) return temp;
	return 0;
}



char*
GetAllAvaildNetInfo()
{
	struct ifreq ifr;
	struct ifconf ifc;
	char buf[1024] = {0};
	cJSON* root = NULL, *netinfo = NULL;
	unsigned long ip, mask, bcast, gateway, mtu;
	char mac[18] = {0};
	char* out = NULL;

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1)
	{
		printf("socket error:%m\n");
		return NULL;
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) // 获取所有有效的网卡名字
	{
		printf("ioctl error:%m\n");
		close(sock);
		return NULL;
	}

	struct ifreq* it = ifc.ifc_req;
	const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));
	root = cJSON_CreateArray();

	for (; it != end; ++it) 
	{
		// 是否为lo和在线
		strcpy(ifr.ifr_name, it->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) == -1) 
		{
			printf("[%d]ioctl error:%m\n", __LINE__);
			continue;
		}
		if (ifr.ifr_flags & IFF_LOOPBACK) continue;
		//printf("ifr_name=%s: up;", it->ifr_name);

		// mac地址
		if (ifr.ifr_flags & IFF_POINTOPOINT)
		{
			strncpy(mac, "00:00:00:00:00:00", sizeof(mac));
			bcast = 0;
		}
		else
		{
			if (ioctl(sock, SIOCGIFHWADDR, &ifr) == -1)
			{
				printf("[%d]ioctl error:%m\n", __LINE__);
				continue;
			}
			snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x", 
				ifr.ifr_hwaddr.sa_data[0] & 0xff, ifr.ifr_hwaddr.sa_data[1] & 0xff, 
				ifr.ifr_hwaddr.sa_data[2] & 0xff, ifr.ifr_hwaddr.sa_data[3] & 0xff,
				ifr.ifr_hwaddr.sa_data[4] & 0xff, ifr.ifr_hwaddr.sa_data[5] & 0xff);

			// 广播地址
			if (ioctl(sock, SIOCGIFBRDADDR, &ifr) == -1)
			{
				printf("[%d]ioctl error:%m\n", __LINE__);
				continue;
			}
			bcast = *(unsigned long*)&ifr.ifr_broadaddr.sa_data[2];
		}
		
		// IP 地址
		if (ioctl(sock, SIOCGIFADDR, &ifr) == -1)
		{
			printf("[%d]ioctl error:%m\n", __LINE__);
			continue;
		}
		ip = *(unsigned long*)&ifr.ifr_addr.sa_data[2];
		//printf("IP:%s; ", inet_ntoa(*(struct in_addr*)&ip));

		// 子网掩码
		if (ioctl(sock, SIOCGIFNETMASK, &ifr) == -1)
		{
			printf("[%d]ioctl error:%m\n", __LINE__);
			continue;
		}
		mask = *(unsigned long*)&ifr.ifr_netmask.sa_data[2];
		//printf("netmask:%s; ", inet_ntoa(*(struct in_addr*)&mask));

		// 网关
		gateway = ip & mask;
		//printf("gateway:%s; ", inet_ntoa(*(struct in_addr*)&gateway));

		// MTU
		if (ioctl(sock, SIOCGIFMTU, &ifr) == -1)
		{
			printf("[%d]ioctl error:%m\n", __LINE__);
			continue;
		}
		mtu = ifr.ifr_mtu;
		//printf("MTU=%d; ", ifr.ifr_mtu);

		// 测度
		//if (ioctl(sock, SIOCGIFMETRIC, &ifr) == -1)
		//{
		//	printf("[%d]ioctl error:%m\n", __LINE__);
		//	continue;
		//}
		//printf("metric:%d; \n", ifr.ifr_metric);

		// 拼接json
		cJSON_AddItemToArray(root, netinfo = cJSON_CreateObject());
		cJSON_AddStringToObject(netinfo, "ifname", ifr.ifr_name);
		cJSON_AddStringToObject(netinfo, "IPaddr", inet_ntoa(*(struct in_addr*)&ip));
		cJSON_AddStringToObject(netinfo, "boardcast", inet_ntoa(*(struct in_addr*)&bcast));
		cJSON_AddStringToObject(netinfo, "gateway", inet_ntoa(*(struct in_addr*)&gateway));
		cJSON_AddStringToObject(netinfo, "netmask", inet_ntoa(*(struct in_addr*)&mask));
		cJSON_AddStringToObject(netinfo, "macaddr", mac);
		cJSON_AddNumberToObject(netinfo, "MTU", mtu);
	}
	close(sock);
	out = cJSON_PrintUnformatted(root);
	cJSON_Delete(root);
	return out;
}


unsigned long GetRandWanGateway()
{
	struct ifreq ifr;
	struct ifconf ifc;
	char buf[1024] = {0};
	unsigned long ip, mask, gateway;

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1)
	{
		printf("socket error:%m\n");
		return 0;
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) // 获取所有有效的网卡名字
	{
		printf("ioctl error:%m\n");
		close(sock);
		return 0;
	}

	struct ifreq* it = ifc.ifc_req;
	const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

	for (; it != end; ++it) 
	{
		// lan口返回
		if (strncmp(LAN_IFNAME, it->ifr_name, LAN_IFNAME_LEN) == 0) continue;
		strcpy(ifr.ifr_name, it->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) == -1) 
		{
			printf("[%d]ioctl error:%m\n", __LINE__);
			continue;
		}
		// 本地环路返回
		if (ifr.ifr_flags & IFF_LOOPBACK ) continue;
		
		// IP 地址
		if (ioctl(sock, SIOCGIFADDR, &ifr) == -1)
		{
			printf("[%d]ioctl error:%m\n", __LINE__);
			continue;
		}
		ip = *(unsigned long*)&ifr.ifr_addr.sa_data[2];
		//printf("IP:%s; ", inet_ntoa(*(struct in_addr*)&ip));

		// 子网掩码
		if (ioctl(sock, SIOCGIFNETMASK, &ifr) == -1)
		{
			printf("[%d]ioctl error:%m\n", __LINE__);
			continue;
		}
		mask = *(unsigned long*)&ifr.ifr_netmask.sa_data[2];

		// 网关
		gateway = ip & mask;
		close(sock);
		return gateway;
	}
	close(sock);
	return 0;
}


unsigned long GetRandWanIP()
{
	struct ifreq ifr;
	struct ifconf ifc;
	char buf[1024] = {0};
	unsigned long ip;

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1)
	{
		printf("socket error:%m\n");
		return 0;
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) // 获取所有有效的网卡名字
	{
		printf("ioctl error:%m\n");
		close(sock);
		return 0;
	}

	struct ifreq* it = ifc.ifc_req;
	const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

	for (; it != end; ++it) 
	{
		// lan口返回
		if (strncmp(LAN_IFNAME, it->ifr_name, LAN_IFNAME_LEN) == 0) continue;
		strcpy(ifr.ifr_name, it->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) == -1) 
		{
			printf("[%d]ioctl error:%m\n", __LINE__);
			continue;
		}
		// 本地环路返回
		if (ifr.ifr_flags & IFF_LOOPBACK ) continue;
		
		// IP 地址
		if (ioctl(sock, SIOCGIFADDR, &ifr) == -1)
		{
			printf("[%d]ioctl error:%m\n", __LINE__);
			continue;
		}
		ip = *(unsigned long*)&ifr.ifr_addr.sa_data[2];
		close(sock);
		return ip;
	}

	close(sock);
	return 0;
}








