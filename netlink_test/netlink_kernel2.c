/*
 * Copyright (c) 2009-~ Helight.Xu
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License
 *
 * Author:       Helight.Xu<Helight.Xu@gmail.com>
 * Created Time: Sat 16 May 2009 04:20:14 PM CST
 * File Name:    knetlink.c
 *
 * Description:  
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <net/sock.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#define NETLINK_XUX           31       /* testing */  

static struct sock *xux_sock = NULL;

static void test_link(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    int pid;
	struct sk_buff *skb_out;
    int res;

    int msg_size;
    char *msg = "hello, this msg is from kernel";

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
    msg_size = strlen(msg);

    /*
    static inline struct nlmsghdr *nlmsg_hdr(const struct sk_buff *skb)
    {
        return (struct nlmsghdr *)skb->data;
    }
    */
    nlh = nlmsg_hdr(skb);
    printk("receive data from user process: %s", (char *)NLMSG_DATA(nlh));

    //for sending...
	pid = nlh->nlmsg_pid; // Sending process port ID, will send new message back to the 'user space sender'
	skb_out = nlmsg_new(msg_size, 0);    //nlmsg_new - Allocate a new netlink message: skb_out

	if(!skb_out)
	{
		printk(KERN_ERR "Failed to allocate new skb\n");
		return;
	}
    // 这里只是修改skb_out的数据长度
	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);  
    NETLINK_CB(skb_out).dst_group = 0;    
    strncpy(nlmsg_data(nlh), msg, msg_size); //char *strncpy(char *dest, const char *src, size_t count)
	//msg "Hello from kernel" => nlh -> skb_out
	res = nlmsg_unicast(xux_sock, skb_out, pid); //nlmsg_unicast - unicast a netlink message
	//@pid: netlink pid of the destination socket
	if(res < 0)
		printk(KERN_INFO "Error while sending bak to user\n");

}

int __init init_link(void)
{
    struct netlink_kernel_cfg cfg = {
		.input = test_link,//该函数原型可参考内核代码，其他参数默认即可
	};
    xux_sock = netlink_kernel_create(&init_net, NETLINK_XUX, &cfg);
    if (!xux_sock){
        printk("cannot initialize netlink socket");
        return -1;
    } 
    
    printk("Init OK!\n");
    return 0;
}

void __exit exit_link(void)
{
    netlink_kernel_release(xux_sock);
    printk(": Release OK!\n");
    return;
}

MODULE_AUTHOR("Helight.Xu");
MODULE_LICENSE("Dual BSD/GPL");
module_init(init_link);
module_exit(exit_link);