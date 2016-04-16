#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tool.h"
#define  JUMP_TO_LOCAL_SERVER  "local_server"

// ����ʹ��fork��Ҫ������ʹ��ȫ�ֱ���������forkʱ���ӽ��̿�¡ȫ�ֱ���

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

	// ������������
	iptablesCommand("iptables -t nat -N " JUMP_TO_LOCAL_SERVER);
	// ��չ�������
	iptablesCommand("iptables -t nat -F " JUMP_TO_LOCAL_SERVER);
	// ��ӹ����������
	iptablesCommand("iptables -t nat -A " JUMP_TO_LOCAL_SERVER " -p tcp -m multiport --dport 80,8080 -j DNAT --to %s:80", lanip);
	iptablesCommand("iptables -t nat -A " JUMP_TO_LOCAL_SERVER " -p tcp -m multiport --dport 443 -j DNAT --to %s:443", lanip);
	// ������������� PREROUTING ������
	iptablesCommand("iptables -t nat -I PREROUTING 1 -i %s -j " JUMP_TO_LOCAL_SERVER, lanIfName);
	
	return 0;
}


