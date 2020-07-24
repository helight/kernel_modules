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

#define NETLINK_XUX           21       /* testing */  
#define VFW_GROUP  1
#define MSG_SIZE NLMSG_SPACE(2048)

static struct sock *xux_sock;

struct netlink_data{
        struct nlmsghdr msg;
        char data[1024];
};

static void test_link(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    u32             rlen;
    void            *data;
    int msg_size;
    char *msg = "hello,from kernel";
    struct netlink_ext_ack extack = {};
    msg_size = strlen(msg);

    while (skb->len >= NLMSG_SPACE(0)) {
        nlh = nlmsg_hdr(skb);
        if (nlh->nlmsg_len < sizeof(*nlh) || skb->len < nlh->nlmsg_len)
                return;
        rlen = NLMSG_ALIGN(nlh->nlmsg_len);
        if (rlen > skb->len)
                rlen = skb->len;
        data = NLMSG_DATA(nlh);
        printk("receive data from user process: %s", (char *)data);
        
        // send data to userspace
        struct sk_buff *skb_out = nlmsg_new(msg_size, 0);    //nlmsg_new - Allocate a new netlink message: skb_out
        if(!skb_out)
        {
            printk(KERN_ERR "Failed to allocate new skb\n");
            return;
        }

	    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
        //so there should be convention: cb[48] is divided into creds/pid/dst_group...to convey those info
        NETLINK_CB(skb_out).dst_group = 0;                  /* not in mcast group */
        strncpy(nlmsg_data(nlh), msg, msg_size); //char *strncpy(char *dest, const char *src, size_t count)
        //msg "Hello from kernel" => nlh -> skb_out  
        netlink_ack(skb, nlh, 0, &extack);
        skb_pull(skb, rlen);
    }
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
    } else
        xux_sock->sk_sndtimeo = MAX_SCHEDULE_TIMEOUT;

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