#ifndef _TOOL_H
#define _TOOL_H

#ifndef __LINUX_KERNEL__
#define printk  printf
#include <stdbool.h>
#endif

#define IFNAMESIZ  18

#define PRINT_BUG

/* 打印ip 地址 */
#define NIPQUAD(addr) ((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#ifdef  PRINT_BUG
#define write_log(fmt,args...) printk("%s %d %s "fmt "\n",__FILE__,__LINE__,__FUNCTION__,##args)
#else
#define write_log
#endif 

#undef  write_error_log
#define write_error_log(fmt,args...) printk("%s %d %s "fmt "\n",__FILE__,__LINE__,__FUNCTION__,##args)
#undef  write_normal_log
#define write_normal_log(fmt,args...) printk("%s %d %s "fmt "\n",__FILE__,__LINE__,__FUNCTION__,##args)

#define print_ip(ip) printk("%d.%d.%d.%d\n", NIPQUAD(ip))



/*判断两个 ip 是否为同个网段*/
#define IS_SAME_NET(ipS, ipD, mask) (((ipS)&(mask))==((ipD)&(mask)))
/*以 16 进制的形式打印字符串，每行打印16个*/
void print_hex(const unsigned char* buf, int len);
/* 打印字符串前 N 位*/
void print_nstr(const char* buf, int len);



static inline char* inet_ntoa_(unsigned int ip, char *buf)
{
    unsigned char *tmp = (unsigned char *)&ip;
	sprintf(buf, "%d.%d.%d.%d", 
		         tmp[0] & 0xff, 
		         tmp[1] & 0xff,
                 tmp[2] & 0xff, 
                 tmp[3] & 0xff);
	return buf;
}

/* 将字符串IP 转换成网络IP */
// in_aton(const char *str)  #include <linux/inet.h>


// 函数功能: 从一个字符串中查找另一个字符串
// 函数参数: buf,
//           find,
//           len,
// 返 回 值: 
char* nstrstr(const char* buf, const char* find, int len);


// 函数功能: 标准域名转换为dns域名
// 函数参数: domain，输入参数，标准域名
//           dnsDomain，输出参数，dns域名
// 返 回 值: 成功返回0；失败返回-1；
int stdDomain2DnsDomain(const char* domain, char* dnsDomain);

// 函数功能: dns域名转换为标准域名
// 函数参数: dnsDomain，输入参数，dns域名 
//           domain，输出参数，标准域名
// 返 回 值: 成功返回0；失败返回-1；
int DnsDomain2stdDomain(const char* dnsDomain, char* domain);



#ifndef __LINUX_KERNEL__
// 函数功能: 执行shell命令行命令
// 函数参数: cmd_line，命令行
//           quiet，是否打印错误消息，1打印，0不打印
// 返 回 值: 正常返回 0
int execute(char *cmd_line, int quiet);

// 函数功能: 执行iptables功能
// 函数参数: format,... 可变参数，类似于printf的参数
// 返 回 值: 成功返回
int iptablesCommand(const char *format, ...);


unsigned int getIPByIfname(const char *ifname);

char* getMacByIfName(const char *ifname);

int getIPByDomain(const char* pDomain, unsigned int *lAddr);



#else
// 函数功能: "通过设备名字获取网卡设备的基本信息
// 函数参数: pIfname: 输入参数，网卡名字
//           lanIP: 输出参数，网卡ip
//           mask:  输出参数，网卡网关
// 返 回 值: 成功返回true，失败返回false
bool getDevInfoByIfname(const char* p_ifname, unsigned int *lanip, unsigned int *mask);

/* 将网络ip 转换成字符串*/
const char* inet_ntoa(unsigned int ip);

#endif





#define MARK_PKG_DNS_REQUEST          0x00010000
#define MARK_PKG_HTTP_GET_REQUEST     0x00020000




#endif /*_TOOL_H */

