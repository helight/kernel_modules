/*
 * Copyright (c) 2009-~ Helight.Xu
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License
 *
 * Author:       Helight.Xu<Helight.Xu@gmail.com>
 * Created Time: Sat 16 May 2009 04:20:14 PM CST
 * File Name:    unetlink.c
 *
 * Description:  
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define NETLINK_XUX           31       /* testing */  
#define MAX_PAYLOAD 1024 /* maximum payload size*/

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct msghdr msg;  //msghdr includes: struct iovec *   msg_iov; 
struct iovec iov;
int fd;

int main(int args, char *argv[])
{
        fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_XUX);

        if (fd < 0) {
                printf("Error - not support not in kernel");
                return fd;
        }

        memset(&src_addr, 0, sizeof(src_addr));
        src_addr.nl_family = AF_NETLINK;
        src_addr.nl_pid = getpid(); /* self pid */

        bind(fd, (struct sockaddr *) &src_addr, sizeof(src_addr));

        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.nl_family = AF_NETLINK;
        dest_addr.nl_pid = 0; /* For Linux Kernel */
        dest_addr.nl_groups = 0; /* unicast */

        // 构造nlmsg空间
        nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
        memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
        nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
        nlh->nlmsg_pid = getpid();  //self pid
        nlh->nlmsg_flags = 0;
        // nlmsg->nlmsg_type = 0;
        // nlmsg->nlmsg_seq = 0;

        // strcpy(NLMSG_DATA(nlmsg), "Hello"); 
        // 给数据空间写数据
        // /usr/include/linux/netlink.h:94:#define NLMSG_DATA(nlh)  ((void*)(((char*)nlh) + NLMSG_LENGTH(0)))
        // /usr/include/linux/netlink.h:92:#define NLMSG_LENGTH(len) ((len) + NLMSG_HDRLEN)
        // /usr/include/linux/netlink.h:91:#define NLMSG_HDRLEN	 ((int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))
        // memcpy(NLMSG_DATA(nlmsg), data, size);
        strcpy(NLMSG_DATA(nlh), "Hello this is a msg from userspace");

        iov.iov_base = (void *)nlh;         //iov -> nlh
        iov.iov_len = nlh->nlmsg_len;
        msg.msg_name = (void *)&dest_addr;  //msg_name is Socket name: dest
        msg.msg_namelen = sizeof(dest_addr);
        msg.msg_iov = &iov;                 //msg -> iov
        msg.msg_iovlen = 1;
        /*
        #include <sys/types.h>
        #include <sys/socket.h>

        ssize_t send(int sockfd, const void *buf, size_t len, int flags);

        ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
                        const struct sockaddr *dest_addr, socklen_t addrlen);

        ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
        */

        // retval = sendto(fd, &nldata, msg->nlmsg_len, 0,
        //                (struct sockaddr*)&addr, sizeof(addr));
        printf("Sending message to kernel\n");

        int ret = sendmsg(fd, &msg, 0);
        printf("send ret: %d\n", ret);
        // printf("hello:%02x len: %d  data:%s\n",
        //                NLMSG_DATA(msg),
        //                sizeof(NLMSG_DATA(msg)),
        //                NLMSG_DATA(msg));
        /*
        ssize_t recv(int sockfd, void *buf, size_t len, int flags);

        ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                        struct sockaddr *src_addr, socklen_t *addrlen);

        ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
        */

        printf("Waiting for message from kernel\n");
        ret = recvmsg(fd, &msg, 0);

        printf("recv ret: %d data: %s\n", ret, (char *)NLMSG_DATA(nlh));

        close(fd);
        return 0;
}
