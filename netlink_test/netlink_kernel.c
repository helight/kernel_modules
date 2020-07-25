#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

#define NETLINK_USER 31     //the user defined channel, the key factor

struct sock *nl_sk = NULL;

static void hello_nl_recv_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	int pid;
	struct sk_buff *skb_out;
    int res;

	int msg_size;
	char *msg = "hello, this msg is from kernel";
	

	printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

	msg_size = strlen(msg);
	//for receiving...
	nlh = (struct nlmsghdr*)skb->data;    //nlh message comes from skb's data... (sk_buff: unsigned char *data)
	/*  
    static inline void *nlmsg_data(const struct nlmsghdr *nlh)
	{
	    return (unsigned char *) nlh + NLMSG_HDRLEN;
	}
	    nlmsg_data - head of message payload 
    */
	printk(KERN_INFO "Netlink received msg payload: %s\n",(char*)nlmsg_data(nlh));

	//for sending...
	pid = nlh->nlmsg_pid; // Sending process port ID, will send new message back to the 'user space sender'

    /**
    * nlmsg_new - Allocate a new netlink message
    * @payload: size of the message payload
    * @flags: the type of memory to allocate.
    *
    * Use NLMSG_DEFAULT_SIZE if the size of the payload isn't known
    * and a good default is needed.
    
    static inline struct sk_buff *nlmsg_new(size_t payload, gfp_t flags)
    {
        return alloc_skb(nlmsg_total_size(payload), flags);
    }
    static inline int nlmsg_total_size(int payload)
    {
        return NLMSG_ALIGN(nlmsg_msg_size(payload));
    }
    static inline int nlmsg_msg_size(int payload)
    {
        return NLMSG_HDRLEN + payload;
    }
    #define NLMSG_HDRLEN     ((int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))
    */

	skb_out = nlmsg_new(msg_size, 0);    //nlmsg_new - Allocate a new netlink message: skb_out

	if(!skb_out)
	{
		printk(KERN_ERR "Failed to allocate new skb\n");
		return;
	}
    // 这里只是修改skb_out的数据长度
	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);  
	/* nlmsg_put - Add a new netlink message to an skb
	 * @skb: socket buffer to store message in
	 * @portid: netlink PORTID of requesting application
	 * @seq: sequence number of message
	 * @type: message type
	 * @payload: length of message payload
	 * @flags: message flags
	//#define NETLINK_CB(skb)           (*(struct netlink_skb_parms*)&((skb)->cb))
	//cb: This is the control buffer. It is free to use for every layer. Please put your private variables there

	 struct netlink_skb_parms {
	struct ucred            creds;          // Skb credentials
	__u32                   pid;
	__u32                   dst_group;
	}; */
	//map skb->cb (char cb[48] __aligned(8); control buffer) to "struct netlink_skb_parms", so it has field pid/dst_group
	//so there should be convention: cb[48] is divided into creds/pid/dst_group...to convey those info
	NETLINK_CB(skb_out).dst_group = 0;                  /* not in mcast group */
    // 把实际数据放入到skb_out->data->data中，skb_out->data指向的是nlh，
    // nlmsg_data(nlh) #define NLMSG_DATA(nlh)  ((void*)(((char*)nlh) + NLMSG_LENGTH(0)))
    /**
    * nlmsg_data - head of message payload
    * @nlh: netlink message header
    
    static inline void *nlmsg_data(const struct nlmsghdr *nlh)
    {
        return (unsigned char *) nlh + NLMSG_HDRLEN;
    }

    #define NLMSG_HDRLEN     ((int) NLMSG_ALIGN(sizeof(struct nlmsghdr)))
    */
    // struct nlmsghdr {
    //    __u32       nlmsg_len;  /* Length of message including header */
    //    __u16       nlmsg_type; /* Message content */
    //    __u16       nlmsg_flags;    /* Additional flags */
    //    __u32       nlmsg_seq;  /* Sequence number */
    //    __u32       nlmsg_pid;  /* Sending process port ID */
    //};
    
	strncpy(nlmsg_data(nlh), msg, msg_size); //char *strncpy(char *dest, const char *src, size_t count)
	//msg "Hello from kernel" => nlh -> skb_out
	res = nlmsg_unicast(nl_sk, skb_out, pid); //nlmsg_unicast - unicast a netlink message
	//@pid: netlink pid of the destination socket
	if(res < 0)
		printk(KERN_INFO "Error while sending bak to user\n");
}

static int __init hello_init(void)
{
	//struct net init_net; defined in net_namespace.c
	//unit=NETLINK_USER: refer to some kernel examples
	//groups = 0, unicast
	//nl_sk: global sock, will be sent to hello_nl_recv_msg as argument (nl_sk ->...-> skb) and return from below func, by Tom Xue, not totally verified
	struct netlink_kernel_cfg cfg = {
		.input = hello_nl_recv_msg,//该函数原型可参考内核代码，其他参数默认即可
	};

	nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
	printk("Entering: %s\n",__FUNCTION__);
	if(!nl_sk)
	{
		printk(KERN_ALERT "Error creating socket.\n");
		return -10;
	}

    printk("Init OK!\n");
	return 0;
}

static void __exit hello_exit(void)
{
	printk(KERN_INFO "exiting hello module\n");
	netlink_kernel_release(nl_sk);
}


module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
