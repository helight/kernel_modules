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

#define NETLINK_XUX           21       /* testing */  
#define VFW_GROUP  1

struct netlink_data{
        struct nlmsghdr msg;
        char data[1024];
};

int link_open(void);

int main(int args, char *argv[])
{
        struct netlink_data nldata;
        struct nlmsghdr *msg = &nldata.msg;
        int retval;
        struct sockaddr_nl addr;
        char *data = "hello world!\0";
        int size = strlen(data);

        int fd = link_open();

        memset(&nldata, '\0', sizeof(nldata));
        msg->nlmsg_len = NLMSG_SPACE(size);
        msg->nlmsg_type = 0;
        msg->nlmsg_flags = 0;
        msg->nlmsg_seq = 0;
        addr.nl_family = AF_NETLINK;
        addr.nl_pid = 0;
        addr.nl_groups = 0;

        memcpy(NLMSG_DATA(msg), data, size);

        retval = sendto(fd, &nldata, msg->nlmsg_len, 0,
                        (struct sockaddr*)&addr, sizeof(addr));

        printf("hello:%02x len: %d  data:%s\n",
                        NLMSG_DATA(msg),
                        sizeof(NLMSG_DATA(msg)),
                        NLMSG_DATA(msg));
        close(fd);
        return 0;
}

int link_open(void)
{
        int saved_errno;
        int fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_XUX);

        if (fd < 0) {
                saved_errno = errno;
                if (errno == EINVAL || errno == EPROTONOSUPPORT ||
                                errno == EAFNOSUPPORT)
                        printf("Error - audit support not in kernel");
                else
                        printf("Error opening audit netlink socket (%s)",
                                strerror(errno));
                errno = saved_errno;
                return fd;
        }
        if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1) {
                saved_errno = errno;
                close(fd);
                        printf("Error setting audit netlink socket CLOEXEC flag (%s)",
                        strerror(errno));
                errno = saved_errno;
                return -1;
        }
        return fd;
}