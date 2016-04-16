#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/fs.h>		// for basic filesystem
#include <linux/proc_fs.h>	// for the proc filesystem
#include <linux/seq_file.h>	// for sequence files
#include <linux/slab.h>		// for kzalloc, kfree
#include <linux/string.h>   // for memcpy memset

#include <linux/inetdevice.h>
#include <linux/netdevice.h>


#include "rtmp_type.h"
#include "ap_mac_probe.h"



ProbeSta* g_probe_sta = NULL;
static char gLanMac[6] = {0};


static inline void init_probe_sta(void)
{
	if (NULL == g_probe_sta)
	{
		g_probe_sta = (ProbeSta*)kzalloc(ProbeStaLen, GFP_KERNEL);	
		if (unlikely(!g_probe_sta))	
		{		
			printk("[%s:%d]Kzalloc Memory failed.\n", __FILE__, __LINE__);		
			return;	
		}
		memset(g_probe_sta, 0, ProbeStaLen);
	}
	
	g_probe_sta->mac_num = 0;
	spin_lock_init(&g_probe_sta->lock);
}


void add_prob_mac(const UCHAR* pMac)
{
	int i;
	unsigned long flags;                                //能从中断上下文中被调用

	if (g_probe_sta && pMac)
	{
		spin_lock_irqsave(&g_probe_sta->lock, flags);       //加锁
		if (g_probe_sta->mac_num < MAX_MAC_SIZE - 1)
		{
			for (i=0; i<g_probe_sta->mac_num; ++i)
			{
				if ( 0 == memcmp(g_probe_sta->sta_mac[i], pMac, MAC_ADDR_LEN) )
				{
					break;
				}
			}
			if (i == g_probe_sta->mac_num)
			{
				memcpy(&g_probe_sta->sta_mac[i][0], pMac, MAC_ADDR_LEN);
				//printk("%02x:%02x:%02x:%02x:%02x:%02x\n", pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5]);
				g_probe_sta->mac_num++;
			}
		}
		spin_unlock_irqrestore(&g_probe_sta->lock, flags);  //解锁
	}
}



static int probe_sta_show(struct seq_file *m, void *v)
{
	int i;
	unsigned long flags;                               //能从中断上下文中被调用
	if (g_probe_sta)
	{
		spin_lock_irqsave(&g_probe_sta->lock, flags);      //加锁
		for (i=0; i<g_probe_sta->mac_num; ++i)
		{
			if (i >= MAX_MAC_SIZE) break;
			seq_printf(m, "%02x%02x%02x%02x%02x%02x\n", 
				g_probe_sta->sta_mac[i][0],
				g_probe_sta->sta_mac[i][1],
				g_probe_sta->sta_mac[i][2],
				g_probe_sta->sta_mac[i][3],
				g_probe_sta->sta_mac[i][4],
				g_probe_sta->sta_mac[i][5]);
		}
		g_probe_sta->mac_num = 0;
		
		spin_unlock_irqrestore(&g_probe_sta->lock, flags);  //解锁
	}
	return 0; 
}

static int probe_sta_open(struct inode *inode, struct file *file)
{
	return single_open(file, probe_sta_show, NULL);
}

static const struct file_operations probe_sta_fops = 
{
	.owner		= THIS_MODULE,
	.open		= probe_sta_open,
	.read		= seq_read,
	.write 		= seq_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};


int probe_sta_init(void)
{
	struct proc_dir_entry* probe_sta_file;
	init_probe_sta();
	
	probe_sta_file = proc_create("probe_sta", 0, NULL, &probe_sta_fops);
	if (NULL == probe_sta_file)
	{
		printk("-------init probe_sta failed.----------\n");
	    return -ENOMEM;
	}

	if ( 0 != setLanMac() )
	{
		printk("[%s:%d]----set lan mac failed---------\n", __FILE__, __LINE__);
	}
	return 0;
}

void probe_sta_exit(void)
{
	remove_proc_entry("probe_sta", NULL);
	if (g_probe_sta) kfree(g_probe_sta), g_probe_sta = NULL;
}


int __setLanMac(const char* ifname)
{
	struct net_device *pNetdev = NULL;
	//struct in_device  *p_indev  = NULL;
	if (NULL == ifname)
	{
		printk("[%s:%d]error: param is NULL.\n", __FILE__, __LINE__);
		return -1;
	}

	pNetdev = dev_get_by_name(&init_net, ifname);
	if (NULL == pNetdev)
	{
		printk("[%s:%d]dev_get_by_name function return NULL.\n", __FILE__, __LINE__);
		return -1;
	}
	memset(gLanMac, 0, sizeof(gLanMac));
	memcpy(gLanMac, pNetdev->dev_addr, 6);
	
	dev_put(pNetdev);
	return 0;
}



int setLanMac()
{
	int nRet;
	nRet = __setLanMac("br-lan");
	if (0 != nRet)
	{
		nRet = __setLanMac("eth0");	
	}
	return nRet;
}

const unsigned char* getLanMac()
{
	return gLanMac;
}






