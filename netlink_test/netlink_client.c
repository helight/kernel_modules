#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>

#define NETLINK_USER 31  //self defined
#define MAX_PAYLOAD 1024 /* maximum payload size*/

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct msghdr msg;  //msghdr includes: struct iovec *   msg_iov;
struct iovec iov;
int sock_fd;

int main(int args, char *argv[])
{
    //int socket(int domain, int type, int protocol);
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);

    if(sock_fd < 0)
        return -1;

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */

    //int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    if(bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr))){
        perror("bind() error\n");
        close(skfd);
        return -1;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;       /* For Linux Kernel */
    dest_addr.nl_groups = 0;    /* unicast */

    //nlh: contains "Hello"
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();  //self pid
    nlh->nlmsg_flags = 0; 

    strcpy(NLMSG_DATA(nlh), "Hello this is a msg from userspace");   //put "Hello" into nlh

    iov.iov_base = (void *)nlh;         //iov -> nlh
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;  //msg_name is Socket name: dest
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;                 //msg -> iov
    msg.msg_iovlen = 1;

    printf("Sending message to kernel\n");

    int ret = sendmsg(sock_fd, &msg, 0);   
    printf("send ret: %d\n", ret); 
    //msg -> find the (destination) socket name: dest
    //msg -> iov -> nlh -> "Hello"

    printf("Waiting for message from kernel\n");

    /* Read message from kernel */

    recvmsg(sock_fd, &msg, 0);  //msg is also receiver for read

    printf("Received message payload: %s\n", NLMSG_DATA(nlh));  //msg -> iov -> nlh

    close(sock_fd);

    return 0;
}