#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/netfilter_bridge.h>
#include <linux/timer.h>
#include <linux/ctype.h>
#include <net/tcp.h>
#include <net/sock.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_helper.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_expect.h>

#include "dns.h"
#include "tool.h"
#include "diyFirewall.h"
const unsigned short uhDnsPort    = htons(PORT_DNS);
const unsigned short uhEthIpProto = htons(ETH_P_IP);



static DNS *g_stdnshead = NULL;
static QUERY *g_stquery = NULL;
static RESPONSE *g_stresponse = NULL;
static struct iphdr *g_iphdr = NULL;




// 对应解析表
static DNSTABLE dns_tables[] = 
{
		{"222baidu.com", "192.168.8.1", 0},
		{"333126.com", "192.168.12.1", 0},
		{{"\0"}, {"\0"}, 0}
};

void initDnsTables(void)
{
	int i = 0;
	while ( dns_tables[i].chip[0] != '\0' )
	{
		dns_tables[i].ip = in_aton(dns_tables[i].chip);
		++i;
	}
}

unsigned int getIpByHost(const char* host)
{
	int i=0;
	while ( dns_tables[i].host[0] != '\0' )
	{
		if ( NULL != nstrstr(dns_tables[i].host, host, strlen(dns_tables[i].host)) )
		{
			return dns_tables[i].ip;
		}
		++i;
	}
	return -1;
}

bool initDnsGlobalVar(void)
{
	if ( NULL == g_stdnshead )
	{
		g_stdnshead = (DNS*)kzalloc(DNS_HEAD_LEN, GFP_KERNEL);
		if ( NULL == g_stdnshead )
		{
			write_error_log("kzalloc memory failed.");
			return false;
		}
	}
	g_stdnshead->id 		= 0;
	g_stdnshead->flags 		= htons(0x8180);
	g_stdnshead->quests 	= htons(1);
	g_stdnshead->answers 	= htons(1);
	g_stdnshead->author 	= 0;
	g_stdnshead->addition 	= 0;
	
	if ( likely(!g_stquery) )
	{	
		g_stquery = (QUERY*)kzalloc(QUERY_LEN, GFP_KERNEL);
		if ( unlikely(!g_stquery) )
		{
			write_error_log("kzalloc memory failed.");
			return false;
		}
	}
	g_stquery->type 	= htons(0x1);   /* 00 01 为类，1表示Internet数据。*/
	g_stquery->classes 	= htons(0x1);	/* 00 01 为类型，1表示A查询*/	

	if ( likely(!g_stresponse) )
	{	
		g_stresponse = (RESPONSE*)kzalloc(RESPONSE_LEN, GFP_KERNEL);
		if ( unlikely(!g_stresponse) )
		{
			write_error_log("kzalloc memory failed.");
			return false;
		}
	}
	g_stresponse->name 		= htons(0xC00C);
	g_stresponse->type 		= htons(0x1);
	g_stresponse->classes 	= htons(0x1);
	g_stresponse->ttl 		= 0xE10;
	g_stresponse->length 	= htons(4);
	g_stresponse->addr 		= get_gLanIP();

	if ( NULL == g_iphdr )
	{	
		g_iphdr = (struct iphdr*)kzalloc(IP_HEADER_LEN, GFP_KERNEL);
		if (NULL == g_iphdr)
		{
			write_error_log("kzalloc memory failed.");
			return false;
		}
	}
	g_iphdr->version = 4;
	g_iphdr->ihl = sizeof(struct iphdr) >> 2;
	g_iphdr->tos = 0;
	g_iphdr->tot_len  = 0;	// 待处理选项
	g_iphdr->frag_off = 0;
	g_iphdr->ttl = 0x40;
	g_iphdr->protocol = IPPROTO_UDP;
	g_iphdr->check = 0;
	g_iphdr->daddr = 0;		// 待处理选项
	g_iphdr->saddr = 0; 	// 待处理选项
	
	return true;
}

void destroyGlobalVar(void)
{
	if (NULL != g_stdnshead)
	{
		kfree(g_stdnshead);
		g_stdnshead = NULL;
	}
	if (NULL != g_stquery)
	{
		kfree(g_stquery);
		g_stquery = NULL;
	}
	if (NULL != g_stresponse)
	{
		kfree(g_stresponse);
		g_stresponse = NULL;
	}
	if (NULL != g_iphdr)
	{
		kfree(g_iphdr);
		g_iphdr = NULL;
	}
}




//过滤DNS包
int netfilter_dns(struct sk_buff *skb, struct iphdr *pre_iphdr)
{
#define HOST_LEN  		64
#define MIN_SKB_LEN 	40
#define IP_V4			4
#define PAYLOAD_LEN		(DNS_HEAD_LEN + HOST_LEN + QUERY_LEN + RESPONSE_LEN)

    struct udphdr *pre_udphdr = NULL;
	DNS			  *pre_dnshdr = NULL;
	char 		  *pre_dname  = NULL;
	unsigned int  dst_ip 	  = get_gLanIP();

	// 是否为udp协议
	if (pre_iphdr->protocol != IPPROTO_UDP)
	{
		return false;
	}
    //UDP段
    pre_udphdr = (void*)pre_iphdr + pre_iphdr->ihl*4;
    if ( pre_udphdr->dest != uhDnsPort )  //DNS端口53
    {
        return false;
    }
	
	// DNS段
	pre_dnshdr = (DNS*)((char*)pre_udphdr + UDP_HEADER_LEN);	/*指向dns包头*/
	pre_dname = (char*)pre_dnshdr + DNS_HEAD_LEN;  			    /*指向DNS数据头*/
	//pre_dnlen = ntohs(pre_udphdr->len) - ((char*)pre_dname - (char*)pre_udphdr); /* UDP 记录的域名长度*/
	
	/* DNS 包处理，*/
#if 0
	/* 1、提取请求的域名*/
	DnsDomain2stdDomain(pre_dname, host);
    /* 判断是否为要处理的域名，获取返回的ip */
	if ( -1 == (dst_ip = getIpByHost((const char*)host)) )
	{
		// 不做处理
		return false;
	}
#endif

	return sendDNSPackage(skb, pre_iphdr, pre_udphdr, pre_dnshdr, dst_ip);
}



bool sendDNSPackage(struct sk_buff *preSkb, 
	                       struct iphdr *preIPHdr,
	                       struct udphdr *preUdpHdr,
	                       DNS			 *preDnsHdr,
	                       unsigned int respIP)
{
	struct net_device *netdev = NULL;
	struct ethhdr *newEthhdr  = NULL;
	struct iphdr  *newIPHdr  = NULL;
	struct udphdr *new_udphdr = NULL;
	struct sk_buff *new_skb   = NULL;
	char 		  *new_data	  = NULL;
	DNS 		  *new_dns	  = NULL;
	QUERY		  *new_query  = NULL;
	RESPONSE	*new_response = NULL;
	short 		  skbLen 	  = 0;
	short         hostLen     = 0;
	
	/* 封装自定义的包 */
	netdev = dev_get_by_index(&init_net, preSkb->dev->ifindex);
	if (NULL == netdev)
	{
		write_log("dev_get_by_index(&init_net, skb->dev->ifindex) is NULL \n");
		return false;
	}
	skbLen = LL_RESERVED_SPACE(netdev) + IP_HEADER_LEN + UDP_HEADER_LEN + PAYLOAD_LEN;
	new_skb = dev_alloc_skb(skbLen);
	if (NULL == new_skb)
	{
		write_error_log("dev_alloc_skb(%d) is NULL.", skbLen);
		dev_put(netdev);
		return false;
	}
    new_skb->dev = netdev;
    dev_put(netdev);
    new_skb->pkt_type  = PACKET_OTHERHOST;
    new_skb->protocol  = uhEthIpProto;     //htons(ETH_P_IP);
    new_skb->ip_summed = CHECKSUM_NONE;
    new_skb->priority  = 0;
    skb_reserve(new_skb, LL_RESERVED_SPACE(netdev));

    skb_set_network_header(new_skb, 0);
    /* 分配内存给udp头 */
    skb_set_transport_header(new_skb, IP_HEADER_LEN);
    skb_put(new_skb, skbLen - LL_RESERVED_SPACE(netdev));
	
    /* 构建ip  头*/
    newIPHdr = ip_hdr(new_skb);
	memcpy(newIPHdr, g_iphdr, IP_HEADER_LEN);
	newIPHdr->tot_len = htons(new_skb->len);
	newIPHdr->daddr   = preIPHdr->saddr;
	newIPHdr->saddr   = preIPHdr->daddr;
	
 	// 构建udp  头
    new_udphdr = udp_hdr(new_skb);
    new_udphdr->source 	= preUdpHdr->dest;
    new_udphdr->dest 	= preUdpHdr->source;
    new_udphdr->len 	= htons(new_skb->len - preIPHdr->ihl*4); 	//htons(256);
    new_udphdr->check 	= 0;

    //copy data in skb
    new_dns = (DNS*)((char*)new_udphdr + UDP_HEADER_LEN);
    memcpy(new_dns, g_stdnshead, DNS_HEAD_LEN);		
    new_dns->id = preDnsHdr->id;
	new_data = (char*)new_dns + DNS_HEAD_LEN;
	hostLen = strlen(((char*)preDnsHdr)+DNS_HEAD_LEN);
    memcpy(new_data, ((char*)preDnsHdr)+DNS_HEAD_LEN, hostLen+1);
    new_query = (QUERY*)(new_data + hostLen+1);
    memcpy(new_query, g_stquery, QUERY_LEN);
	new_response = (RESPONSE*)((char*)new_query + QUERY_LEN);
    memcpy(new_response, g_stresponse, RESPONSE_LEN);
	new_response->addr = respIP;

    // caculate checksum
    new_skb->csum 	  = skb_checksum(new_skb, newIPHdr->ihl*4, (new_skb->len - newIPHdr->ihl*4), 0);
    newIPHdr->check   = ip_fast_csum(newIPHdr, newIPHdr->ihl);
    new_udphdr->check = csum_tcpudp_magic(newIPHdr->saddr, newIPHdr->daddr, 
						(new_skb->len - newIPHdr->ihl*4), IPPROTO_UDP, new_skb->csum);

    //construct ethernet header in skb
    newEthhdr = (struct ehthdr*)(skb_push(new_skb, ETH_HLEN));
    memcpy(newEthhdr->h_dest, preSkb->mac_header+6, ETH_ALEN);
    memcpy(newEthhdr->h_source, preSkb->mac_header, ETH_ALEN);
    newEthhdr->h_proto = uhEthIpProto; //htons(ETH_P_IP);
	
    if ( unlikely(dev_queue_xmit(new_skb) < 0) )
    {
        kfree_skb(new_skb);
        write_error_log("Response fail.");
        return false;
    }
    return true;
}

