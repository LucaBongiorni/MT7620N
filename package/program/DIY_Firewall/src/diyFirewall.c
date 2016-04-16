#include <linux/module.h>
#include <linux/moduleparam.h>  
#include <linux/init.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/cdev.h>
#include <linux/netfilter_bridge.h>
#include <linux/ctype.h>

#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_helper.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_expect.h>

#include "diyFirewall.h"
#include "dns.h"
#include "tool.h"


/////////////////////////ȫ�ֱ���////////////////////////
unsigned int gLanIP = 0;
unsigned int gLanMask = 0;
//const char* const gIfname = "br0";
/////////////////////////ȫ�ֱ���////////////////////////

unsigned int get_gLanIP(void)
{
	return gLanIP;
}


unsigned int get_gLanMask(void)
{
	return gLanMask;
}


#if 0
// ��������: �԰������
unsigned int markPackage(unsigned int hooknum, 
                  struct sk_buff *skb,
                  const struct net_device *in, 
                  const struct net_device *out,
                  int(*okfn)(struct sk_buff *))
{
	struct iphdr  *pre_iphdr  = NULL;
	struct udphdr *pre_udphdr = NULL;
	
	if ( (in == NULL) /*|| (memcmp(in->name, gIfname, 3) !=0)*/ )
	{
		return NF_ACCEPT;
	}
	/* �ж��Ƿ���dns��*/
    if ( NULL == skb || NULL == skb->mac_header || skb->len < 40 )
    {
		return NF_ACCEPT;
    }
	if (skb_is_nonlinear(skb))
	{
		return NF_ACCEPT;
	}
    //IP��
    pre_iphdr = ip_hdr(skb);
    if (NULL == pre_iphdr || pre_iphdr->ihl < 5 || pre_iphdr->version != IP_V4 || pre_iphdr->protocol != IPPROTO_UDP)
    {
		return NF_ACCEPT;
	}
	/* �ж�Դip�Ƿ�Ϊ lan �����ε�ip��������˵�ǲ����������İ���*/
	if (!IS_SAME_NET(pre_iphdr->saddr, gLanIP, gLanMask))
	{
		write_log("Warming: Non't lan package coming in.");
		return NF_ACCEPT;
	}

	//UDP��
	pre_udphdr = (void*)pre_iphdr + pre_iphdr->ihl*4;
	if (pre_udphdr->dest != uhDnsPort)  //DNS�˿�53
		return NF_ACCEPT;
	else
		skb->mark = MARK_PKG_DNS_REQUEST;
	
	return NF_ACCEPT;
}
#else
unsigned int markPackage(unsigned int hooknum, struct sk_buff *skb,
                    const struct net_device *in, const struct net_device *out,
                    int(*okfn)(struct sk_buff *))
{
	struct iphdr  *pre_iphdr  = NULL;
	
	/* �ӿڹ��ˣ�����lan  �ڵİ����˵�*/
	if ( (in == NULL) /*|| (strcmp(in->name, gIfname) !=0)*/)
	{
		return NF_ACCEPT;
	}
	/* �ж��Ƿ���dns��*/
    if ( NULL == skb || NULL == skb->mac_header || skb->len < 40 )
    {
		return NF_ACCEPT;
    }
	/* �����ԵĲ����� */
	if (skb_is_nonlinear(skb))
	{
		return NF_ACCEPT;
	}
    //IP��
    pre_iphdr = ip_hdr(skb);
    if (NULL == pre_iphdr  || 
		pre_iphdr->ihl < 5 || 
		pre_iphdr->version != IP_V4)
    {
		return NF_ACCEPT;
	}
	/* �ж�Դip�Ƿ�Ϊ lan �����ε�ip��������˵�ǲ����������İ���*/
	if (!IS_SAME_NET(pre_iphdr->saddr, gLanIP, gLanMask))
	{
		//write_log("Warming: Non't lan package coming in.");
		return NF_ACCEPT;
	}
	
    if ( true == netfilter_dns(skb, pre_iphdr) )
    {
        /*���˵�dns����*/
        return NF_DROP;
    }	
    return NF_ACCEPT;
}
#endif

unsigned int executeByMarkVal(unsigned int hooknum, 
                  struct sk_buff *skb,
                  const struct net_device *in, 
                  const struct net_device *out,
                  int(*okfn)(struct sk_buff *))
{
	if (unlikely(!skb))
	{
		return NF_ACCEPT;
	}
	
	switch (skb->mark)
	{
	case MARK_PKG_DNS_REQUEST:
		
		break;
	case MARK_PKG_HTTP_GET_REQUEST:
		
		break;
	default:
		break;
	}

	return NF_ACCEPT;
}



static struct nf_hook_ops http_hooks[] =
{
	{
		.owner		= THIS_MODULE,
	 	.hook 		= markPackage,
		.pf 		= PF_INET,
		.hooknum 	= NF_INET_PRE_ROUTING,
		.priority 	= NF_IP_PRI_FIRST,    // ��һ��
	},
/*	
	{
		.owner      = THIS_MODULE,
		.hook       = executeByMarkVal,
		.pf         = PF_INET,
		.hooknum    = NF_INET_POST_ROUTING,
		.priority   = NF_IP_PRI_LAST,      // ���һ��
	},
*/
};

static int __init diyFirewallInit(void)
{
	initDnsTables();
	if (false == initDnsGlobalVar())
	{
		return -1;
	}
	if (false == getDevInfoByIfname("br-lan", &gLanIP, &gLanMask))
	{	
		if (false == getDevInfoByIfname("br0", &gLanIP, &gLanMask))
		{
			write_error_log("Get IP/MASK of net device failed.");
			return -1;
		}
	}
	write_log("gLanIP=%s", inet_ntoa(gLanIP));
	write_log("gLanMask=%s", inet_ntoa(gLanMask));
	
    if (nf_register_hooks(http_hooks, ARRAY_SIZE(http_hooks)))
    {
        write_error_log("nf_register_hooks failed.");
		return -1;
    }

    return 0;
}

static void __exit diyFirewallExit(void)
{
	nf_unregister_hooks(http_hooks, ARRAY_SIZE(http_hooks));
	destroyGlobalVar();
}

MODULE_LICENSE("GPL");  
MODULE_DESCRIPTION("diyFirewall");
MODULE_VERSION("1.0");
MODULE_AUTHOR("hxw"); 

module_init(diyFirewallInit);
module_exit(diyFirewallExit);

