#ifndef __LINUX_KERNEL__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <netdb.h>
#else
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/kernel.h>
#include <linux/string.h>

#endif

#include "tool.h"

#ifdef __LINUX_KERNEL__

bool getDevInfoByIfname(const char* p_ifname, unsigned int *lanip, unsigned int *mask)
{
	struct net_device *p_netdev = NULL;
	struct in_device  *p_indev  = NULL;
	if (NULL == p_ifname)
	{
		write_error_log("error: param is NULL.");
		return false;
	}

	p_netdev = dev_get_by_name(&init_net, p_ifname);
	if (NULL == p_netdev)
	{
		write_error_log("dev_get_by_name(&init_net, pIfName) is NULL.");
		return false;
	}

	p_indev = (struct in_device*)p_netdev->ip_ptr;
	if(NULL == p_indev || NULL == p_indev->ifa_list)
	{
		write_error_log("p_netdev->in_ptr is NULL.");
		dev_put(p_netdev);
		return false;
	}

	*lanip = p_indev->ifa_list->ifa_address;
	*mask  = p_indev->ifa_list->ifa_mask;
	dev_put(p_netdev);

	return true;
}

const char* inet_ntoa(unsigned int ip)
{
	static char buf[sizeof "123.456.789.123"];
	unsigned char *tmp = (unsigned char *)&ip;
	sprintf(buf, "%d.%d.%d.%d", tmp[0] & 0xff, tmp[1] & 0xff,
							    tmp[2] & 0xff, tmp[3] & 0xff);
	return buf;
}


#else
int execute(char *cmd_line, int quiet)
{
	int pid, status, rc;

	const char *new_argv[4];
	new_argv[0] = "/bin/sh";
	new_argv[1] = "-c";
	new_argv[2] = cmd_line;
	new_argv[3] = NULL;

	pid = fork();
	if (pid == 0) 
	{   
		/* 子进程 */
		/* We don't want to see any errors if quiet flag is on */
		if (quiet) 
			close(2);
		if (execvp("/bin/sh", (char *const *)new_argv) == -1) 
		{    /* execute the command  */
			write_error_log("execvp(): %s", strerror(errno));
		} 
		else 
		{
			write_error_log("execvp() failed");
		}
		exit(1);
	}

	/* 父进程 */
	write_error_log("Waiting for PID %d to exit", pid);
	rc = waitpid(pid, &status, 0);
	write_error_log("Process PID %d exited", rc);

    return (WEXITSTATUS(status));
}

int safeAsprintf(char **strp, const char *fmt, ...)
{
	va_list ap;
	int retval;

	va_start(ap, fmt);
	retval = safeVasprintf(strp, fmt, ap);
	va_end(ap);

	return (retval);
}

int safeVasprintf(char **strp, const char *fmt, va_list ap)
{
	int retval;
	
	retval = vasprintf(strp, fmt, ap);
	if (retval == -1) 
	{
		write_error_log("Failed to vasprintf: %s.  Bailing out", strerror(errno));
		exit (1);
	}
	
	return (retval);
}

int iptablesCommand(const char *format, ...)
{
	va_list vlist;
	char *cmd;
	int rc;

	va_start(vlist, format);
	safeVasprintf(&cmd, format, vlist);
	va_end(vlist);
	
	write_log("Executing command: %s", cmd);

	rc = execute(cmd, 0);
	if (rc != 0 ) 
	{
		printf("iptables command failed(%d): %s", rc, cmd);
	}
	printf("%s\n", cmd);
	free(cmd);
	return rc;
}

unsigned int getIPByIfname(const char *ifname)
{
    struct ifreq if_data;
    int sockd;
    unsigned int ip;
    if ((sockd = socket(AF_INET, SOCK_PACKET, htons(0x8086))) < 0)
    {
        write_error_log("Create Socket Failed.");
        return 0;
    }
    /* Get IP of internal interface */
    strcpy(if_data.ifr_name, ifname);
	
    /* Get the IP address */
    if (ioctl(sockd, SIOCGIFADDR, &if_data) < 0)
    {
        write_error_log("Ioctl Get Socket Ifaddr Failed. %m");
        close(sockd);
        return 0;
    }
    memcpy((void*)&ip, (char*)&if_data.ifr_addr.sa_data + 2, 4);
    close(sockd);
    return ip;
}

char* getMacByIfName(const char *ifname)
{
	int r;
	int sockd;
	struct ifreq ifr;
	char *hwaddr, mac[32];

	if (ifname == NULL || strlen(ifname) > (IFNAMESIZ-1))
	{
		printf("[%s][%d]Param Error.", __FILE__, __LINE__);
		return NULL;
	}
	
	if (-1 == (sockd = socket(PF_INET, SOCK_DGRAM, 0))) 
	{
		printf("[%s][%d]Create Socket Failed.", __FILE__, __LINE__);
		return NULL;
	}
	
	strcpy(ifr.ifr_name, ifname);
	r = ioctl(sockd, SIOCGIFHWADDR, &ifr);
	if (r == -1) 
	{
		printf("[%s][%d]Ioctl Get Socket Ifaddr Failed.", __FILE__, __LINE__);
		close(sockd);
		return NULL;
	}
	hwaddr = ifr.ifr_hwaddr.sa_data;
	close(sockd);
	
	snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X", 
			hwaddr[0] & 0xFF,
			hwaddr[1] & 0xFF,
			hwaddr[2] & 0xFF,
			hwaddr[3] & 0xFF,
			hwaddr[4] & 0xFF,
			hwaddr[5] & 0xFF
		);
	return strdup(mac);
}

int getIPByDomain(const char* pDomain, unsigned int *lAddr)
{
    struct hostent *hp = NULL;
    if ( (hp = gethostbyname(pDomain)) != NULL )
    {
        if (hp->h_addr_list[0])
        {
            *lAddr = *((unsigned int*) hp->h_addr_list[0]);
            return 0;
        }
    }
    return -1;
}








#endif


// 函数功能: 标准域名转换为dns域名
// 函数参数: domain，输入参数，标准域名
//           dnsDomain，输出参数，dns域名
// 返 回 值: 成功返回0；
int stdDomainTrunDnsDomain(const char* domain, char* dnsDomain)
{
    const char* pStr = domain;
    int cnt = 0;
    int i = 0;
    
    while (*pStr)
    {   
        if (*pStr != '.') 
            ++cnt;
        else
        {   
            dnsDomain[i++] = cnt;
            memcpy(&dnsDomain[i], pStr-cnt, cnt);
            i += cnt;
            cnt = 0;
        }   
        ++pStr;
    }
    dnsDomain[i++] = cnt;
    memcpy(&dnsDomain[i], pStr-cnt, cnt+1);
    return 0;
}

// 函数功能: dns域名转换为标准域名
// 函数参数: dnsDomain，输入参数，dns域名 
//           domain，输出参数，标准域名
// 返 回 值: 成功返回0；失败返回-1；
int DnsDomain2stdDomain(const char* dnsDomain, char* domain)
{
    const char* pStr = dnsDomain;
    int cnt;
    int i = 0;

    while(*pStr && pStr < dnsDomain+256)
    {   
        cnt = *pStr++;
        memcpy(&domain[i], pStr, cnt);
        i += cnt;
        domain[i++] = '.'; 
        pStr += cnt;
    }   
    domain[i-1] = 0;
    return 0;
}



/* 从一个字符串中查找另一个字符串*/
char* nstrstr(const char* buf, const char* find, int len)
{
	int i;
	int temp = strlen(find);
	len -= temp;

	for (i=0; i<=len; ++i)
	{
		if (!memcmp(buf+i, find, temp))
		{
			return (char*)buf + i;
		}
	}
	return NULL;
}

// 以16进制的形式打印字符串，每行打印16个
void print_hex(const unsigned char* buf, int len)
{
	int i;
	if (NULL == buf)
	{
		return;
	}
	for (i=0; i<len; ++i)
	{
		printk("%02x ", buf[i]);
		if( ((i+1)%16 == 0) && i )
		{
			printk("\n");
		}
	}
	if (i)
	{
		printk("\n");
	}
}


// 打印字符串前N位
void print_nstr(const char* buf, int len)
{
	int i;
	if ( NULL == buf )
	{
		return;
	}
	
	for (i=0; i<len; ++i)
	{
		printk("%c", buf[i]);
	}
	if (i)
	{
		printk("\n");
	}
}


