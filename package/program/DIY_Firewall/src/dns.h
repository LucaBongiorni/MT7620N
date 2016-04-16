#ifndef __DNS_H__
#define __DNS_H__

#include <linux/if_ether.h>
#include <linux/inet.h>
#include <linux/ip.h>
#include <linux/udp.h>

#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>


#pragma pack(push, 1) 
// 标准DNS  协议头
typedef struct _dns
{
	unsigned short id;
	unsigned short flags;
	unsigned short quests;
	unsigned short answers;
	unsigned short author;
	unsigned short addition;
}DNS;
#define DNS_HEAD_LEN (sizeof(DNS))

typedef struct _query
{
	unsigned short type;
	unsigned short classes;
}QUERY;
#define QUERY_LEN	(sizeof(QUERY))

typedef struct _response
{
	unsigned short 	name;		// 表示域名指针
	unsigned short 	type;		// 
	unsigned short 	classes;
	unsigned int   	ttl;	
	unsigned short 	length;
	unsigned int  	addr;
}RESPONSE;
#define RESPONSE_LEN  (sizeof(RESPONSE))


typedef struct _dnstable
{
	char host[64];
	char chip[18];
	unsigned int ip;
}DNSTABLE;
#pragma pack(pop)


#define IP_V4               4
#define PORT_DNS			53





void initDnsTables(void);
unsigned int getIpByHost(const char* host);



bool initDnsGlobalVar(void);
void destroyGlobalVar(void);


// 函数功能: 发送DNS数据包
// 函数参数: preSkb，skb包指针；
//           preIPHdr，IP包头指针
//           preUdpHdr，UDP包头指针
//           preDnsHdr，DNS包头指针
//           respIP，解析ip；
// 返 回 值: 成功返回true；失败返回false；
bool sendDNSPackage(struct sk_buff *preSkb, 
	                struct iphdr *preIPHdr,
	                struct udphdr *preUdpHdr,
	                DNS			 *preDnsHdr,
	                unsigned int respIP);

// 函数功能: 过滤DNS数据包
// 函数参数: preSkb，skb包指针；
//           preIPHdr，IP包头指针
// 返 回 值: 成功返回true；失败返回false；
int netfilter_dns(struct sk_buff *skb, struct iphdr *pre_iphdr);



#endif //__DNS_H__

