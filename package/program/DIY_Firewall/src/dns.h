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
// ��׼DNS  Э��ͷ
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
	unsigned short 	name;		// ��ʾ����ָ��
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


// ��������: ����DNS���ݰ�
// ��������: preSkb��skb��ָ�룻
//           preIPHdr��IP��ͷָ��
//           preUdpHdr��UDP��ͷָ��
//           preDnsHdr��DNS��ͷָ��
//           respIP������ip��
// �� �� ֵ: �ɹ�����true��ʧ�ܷ���false��
bool sendDNSPackage(struct sk_buff *preSkb, 
	                struct iphdr *preIPHdr,
	                struct udphdr *preUdpHdr,
	                DNS			 *preDnsHdr,
	                unsigned int respIP);

// ��������: ����DNS���ݰ�
// ��������: preSkb��skb��ָ�룻
//           preIPHdr��IP��ͷָ��
// �� �� ֵ: �ɹ�����true��ʧ�ܷ���false��
int netfilter_dns(struct sk_buff *skb, struct iphdr *pre_iphdr);



#endif //__DNS_H__

