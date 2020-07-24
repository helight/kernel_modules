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
    struct netlink_ext_ack extack = {};

    while (skb->len >= NLMSG_SPACE(0)) {
        nlh = nlmsg_hdr(skb);
        if (nlh->nlmsg_len < sizeof(*nlh) || skb->len < nlh->nlmsg_len)
                return;
        rlen = NLMSG_ALIGN(nlh->nlmsg_len);
        if (rlen > skb->len)
                rlen = skb->len;
        data = NLMSG_DATA(nlh);
        printk("link:%s", (char *)data);
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