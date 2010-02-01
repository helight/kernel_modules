/*
 * Copyright (c) 2009-~ Helight.Xu
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License
 *
 * Author:       Helight.Xu<Helight.Xu@gmail.com>
 * Created Time: Mon 01 Feb 2010 10:08:56 AM HKT
 * File Name:    netaudit.c
 *
 * Description:  version 0.2
 */

#include <linux/init.h>     
#include <linux/module.h>     
#include <linux/kernel.h>     
#include <linux/ip.h>     
#include <linux/tcp.h>     
#include <linux/udp.h>     
#include <linux/audit.h>
#include <linux/netfilter_ipv4.h>

static struct nf_hook_ops nfho;   
      
unsigned int hook_func(unsigned int hookunm, struct sk_buff *skb,
		const struct net_device *in, const struct net_device *out,
		int (*okfn)(struct sk_buff *))
{
	struct iphdr *iph;
	struct audit_buffer *ab;
	struct sk_buff *pskb = skb;

	printk("pskb:%d",(unsigned int)pskb->protocol);     
 	iph = ip_hdr(pskb);
	printk("IPHDR:%d\n",(unsigned int)iph->protocol);     
	printk("IP: [%u.%u.%u.%u]-->[%u.%u.%u.%u]\n", NIPQUAD(iph->saddr),NIPQUAD(iph->daddr));

	if (iph->protocol == 6){      
		struct tcphdr *tcph;
		tcph = tcp_hdr(pskb);//获取tcp数据包的包头   
/*
		ab = audit_log_start(NULL, GFP_KERNEL, AUDIT_MAC_IPSEC_ADDSA);
		audit_log_format(ab, "IP: [%u]->[%u]", iph->saddr, iph->daddr);
		audit_log_end(ab);
*/
		printk("TCP:[%u]->[%u]\n", ntohs(tcph->source), ntohs(tcph->dest));     
	}      

	if (iph->protocol == 17){
		struct udphdr *udph = udp_hdr(pskb);//获取udp数据包的包头
		printk("UDP:[%u]->[%u]\n", ntohs(udph->source), ntohs(udph->dest));     
	}
	return NF_ACCEPT;     
}      
      
/*init the module*/    
static int init_netaudit(void)    
{      
	printk("netaudit init ...\n");   
	nfho.hook = (nf_hookfn *)hook_func;      
	nfho.hooknum = NF_IP_PRE_ROUTING; 
	nfho.pf = PF_INET;      
	nfho.priority = NF_IP_PRI_FIRST;      
      
	nf_register_hook(&nfho);      
	printk("netaudit hello\n");   
	return 0;     
}      
      
/*Clear the module*/    
void exit_netaudit(void)     
{      
	nf_unregister_hook(&nfho);      
	printk("netaudit bey!!!\n");   
}      
      
module_init(init_netaudit);      
module_exit(exit_netaudit);      
MODULE_AUTHOR("Helight@QQ.com");
MODULE_LICENSE("Dual BSD/GPL");      
