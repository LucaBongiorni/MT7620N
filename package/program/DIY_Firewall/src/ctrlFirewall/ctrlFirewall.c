#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tool.h"
#define  JUMP_TO_LOCAL_SERVER  "local_server"

// 由于使用fork，要尽量少使用全局变量，避免fork时，子进程克隆全局变量

int main()
{
	char lanip[sizeof("111.111.111.111")] = {0};
	char lanIfName[16]  = {0};
	
	unsigned int ip = getIPByIfname("br-lan");
	if (0 == ip)
	{
		ip = getIPByIfname("br0");
		if (0 == ip)
		{
			write_error_log("The lan ifname is error.");
			return -1;
		}
		strncpy(lanIfName, "br0", sizeof(lanIfName));
	}
	strncpy(lanIfName, "br-lan", sizeof(lanIfName));
	inet_ntoa_(ip, lanip);

	// 创建规则链表
	iptablesCommand("iptables -t nat -N " JUMP_TO_LOCAL_SERVER);
	// 清空规则链表
	iptablesCommand("iptables -t nat -F " JUMP_TO_LOCAL_SERVER);
	// 添加规则链表规则
	iptablesCommand("iptables -t nat -A " JUMP_TO_LOCAL_SERVER " -p tcp -m multiport --dport 80,8080 -j DNAT --to %s:80", lanip);
	iptablesCommand("iptables -t nat -A " JUMP_TO_LOCAL_SERVER " -p tcp -m multiport --dport 443 -j DNAT --to %s:443", lanip);
	// 将规则链表插入 PREROUTING 链表中
	iptablesCommand("iptables -t nat -I PREROUTING 1 -i %s -j " JUMP_TO_LOCAL_SERVER, lanIfName);
	
	return 0;
}


