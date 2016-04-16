#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <bits/errno.h>

#include "netPing.h"

int logprintflag = 0;


/* It turns out that libc5 doesn't have proper icmp support
* built into it header files, so we have to supplement it */
#if __GNU_LIBRARY__ < 5
static const int ICMP_MINLEN = 8;               /* abs minimum */

struct icmp_ra_addr
{
    u_int32_t ira_addr;
    u_int32_t ira_preference;
};


struct icmp
{
    u_int8_t  icmp_type;    /* type of message, see below */
    u_int8_t  icmp_code;    /* type sub code */
    u_int16_t icmp_cksum;   /* ones complement checksum of struct */
    union
    {
        u_char ih_pptr;     /* ICMP_PARAMPROB */
        struct in_addr ih_gwaddr;   /* gateway address */
        struct ih_idseq     /* echo datagram */
        {
            u_int16_t icd_id;
            u_int16_t icd_seq;
        } ih_idseq;
        u_int32_t ih_void;

        /* ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191) */
        struct ih_pmtu
        {
            u_int16_t ipm_void;
            u_int16_t ipm_nextmtu;
        } ih_pmtu;

        struct ih_rtradv
        {
            u_int8_t irt_num_addrs;
            u_int8_t irt_wpa;
            u_int16_t irt_lifetime;
        } ih_rtradv;
    } icmp_hun;
#define icmp_pptr       icmp_hun.ih_pptr
#define icmp_gwaddr     icmp_hun.ih_gwaddr
#define icmp_id         icmp_hun.ih_idseq.icd_id
#define icmp_seq        icmp_hun.ih_idseq.icd_seq
#define icmp_void       icmp_hun.ih_void
#define icmp_pmvoid     icmp_hun.ih_pmtu.ipm_void
#define icmp_nextmtu    icmp_hun.ih_pmtu.ipm_nextmtu
#define icmp_num_addrs  icmp_hun.ih_rtradv.irt_num_addrs
#define icmp_wpa        icmp_hun.ih_rtradv.irt_wpa
#define icmp_lifetime   icmp_hun.ih_rtradv.irt_lifetime
    union
    {
        struct
        {
            u_int32_t its_otime;
            u_int32_t its_rtime;
            u_int32_t its_ttime;
        } id_ts;
        struct
        {
            struct ip idi_ip;
            /* options and then 64 bits of data */
        } id_ip;
        struct icmp_ra_addr id_radv;
        u_int32_t   id_mask;
        u_int8_t    id_data[1];
    } icmp_dun;
#define icmp_otime  icmp_dun.id_ts.its_otime
#define icmp_rtime  icmp_dun.id_ts.its_rtime
#define icmp_ttime  icmp_dun.id_ts.its_ttime
#define icmp_ip     icmp_dun.id_ip.idi_ip
#define icmp_radv   icmp_dun.id_radv
#define icmp_mask   icmp_dun.id_mask
#define icmp_data   icmp_dun.id_data
};
#endif


int create_icmp_socket(void)
{
    struct protoent *proto;
    int sock;
    proto = getprotobyname("icmp");
    /* if getprotobyname failed, just silently force
    * proto->p_proto to have the correct value for "icmp" */
    if ((sock = socket(AF_INET, SOCK_RAW,
                       (proto ? proto->p_proto : 1))) < 0)
    {
        /* 1 == ICMP */
        if (errno == EPERM)
        {
            //  error_msg_and_die("permission denied. (are you root?)");
            printf("permission denied. (are you root?)");
        }
        else
        {
            //  perror_msg_and_die(can_not_create_raw_socket);
            printf( "canot create raw socket");
        }
    }
    /* drop root privs if running setuid */
    setuid(getuid());
    return sock;
}

static const int DEFDATALEN = 56;
static const int MAXIPLEN = 60;
static const int MAXICMPLEN = 76;
static const int MAXPACKET = 65468;
#define MAX_DUP_CHK (8 * 128)
static const int MAXWAIT = 10;
static const int PINGINTERVAL = 1;      /* second */

#define O_QUIET     (1 << 0)
#define A(bit)      rcvd_tbl[(bit)>>3]       /* identify byte in array */
#define B(bit)      (1 << ((bit) & 0x07))   /* identify bit in byte */
#define SET(bit)    (A(bit) |= B(bit))
#define CLR(bit)    (A(bit) &= (~B(bit)))
#define TST(bit)    (A(bit) & B(bit))

static void ping(const char *host);

/* common routines */
static int in_cksum(unsigned short *buf, int sz)
{
    int nleft = sz;
    int sum = 0;
    unsigned short *w = buf;
    unsigned short ans = 0;
    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1)
    {
        *(unsigned char *) (&ans) = *(unsigned char *) w;
        sum += ans;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    ans = ~sum;
    return (ans);
}

static struct hostent *xgethostbyname(const char *name)
{
    struct hostent *retval;

    if ((retval = gethostbyname(name)) == NULL)
    {
        if (logprintflag)
        {
            printf("%s   %s \n", name, strerror(errno));
        }
        else
        {
            printf("%s   %s \n", name, strerror(errno));
        }
        return NULL;
    }
    return retval;
}


/* simple version */
#if 0//ndef CONFIG_FEATURE_FANCY_PING
static char *hostname = NULL;
static void noresp(int ign)
{
    printf("No response from %s\n", hostname);
    exit(0);
}

static void ping(const char *host)
{
    struct hostent *h;
    struct sockaddr_in pingaddr;
    struct icmp *pkt;
    int pingsock, c;
    char packet[DEFDATALEN + MAXIPLEN + MAXICMPLEN];
    pingsock = create_icmp_socket();
    memset(&pingaddr, 0, sizeof(struct sockaddr_in));
    pingaddr.sin_family = AF_INET;
    h = xgethostbyname(host);
    memcpy(&pingaddr.sin_addr, h->h_addr, sizeof(pingaddr.sin_addr));
    hostname = h->h_name;
    pkt = (struct icmp *) packet;
    memset(pkt, 0, sizeof(packet));
    pkt->icmp_type = ICMP_ECHO;
    pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));
    c = sendto(pingsock, packet, sizeof(packet), 0,
               (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));
    if (c < 0 || c != sizeof(packet))
    {
        //perror_msg_and_die("sendto");
        fzzgxLog(MODULE_NETPING,stderr, "sendto");
        return ;
    }
    signal(SIGALRM, noresp);
    alarm(5);                   /* give the host 5000ms to respond */
    /* listen for replies */
    while (1)
    {
        struct sockaddr_in from;
        size_t fromlen = sizeof(from);
        if ((c = recvfrom(pingsock, packet, sizeof(packet), 0,
                          (struct sockaddr *) &from, &fromlen)) < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                if(logprintflag)
                {
                    zzgxLog(MODULE_NETPING,"recvfrom\n");
                }
                else
                {
                    printf("recvfrom\n");
                }
                return;
            }
            //perror_msg("recvfrom");
            //  perror();
            continue;
        }
        if (c >= 76)            /* ip + icmp */
        {
            struct iphdr *iphdr = (struct iphdr *) packet;
            pkt = (struct icmp *) (packet + (iphdr->ihl << 2)); /* skip ip hdr */
            if (pkt->icmp_type == ICMP_ECHOREPLY)
            {
                break;
            }
        }
    }
    printf("%s is alive!\n", hostname);
    return;
}

extern int ping_main(int argc, char **argv)
{
    /*
    argc--;
    argv++;
    if (argc < 1)
    show_usage();
        */
    ping(*argv);
    return EXIT_SUCCESS;
}

#else /* ! CONFIG_FEATURE_FANCY_PING */


/* full(er) version */
static struct sockaddr_in pingaddr;
static int pingsock = -1;
static int datalen; /* intentionally uninitialized to work around gcc bug */

static long ntransmitted, nreceived, nrepeats, pingcount;
static int myid;//, options;
static unsigned long tmin = ULONG_MAX, tmax=0, tsum=0;
static char rcvd_tbl[MAX_DUP_CHK / 8];

struct hostent *hostent;

static void sendping(int);
static void pingstats(int);
static void unpack(char *, int, struct sockaddr_in *);


static void pingstats(int junk)
{
    int status;
    //signal(SIGINT, SIG_IGN);
    //printf("\n--- %s ping statistics ---\n", hostent->h_name);
    //printf("%ld packets transmitted, \n", ntransmitted);
    //printf("%ld packets received, \n", nreceived);
    if (nrepeats)
    {
        if(logprintflag)
        {
            printf("%ld duplicates, \n", nrepeats);
        }
        else
        {
            printf("%ld duplicates, \n", nrepeats);
        }
    }
    if (ntransmitted)
    {
        if(logprintflag)
        {
            printf("%ld%% packet loss\n", (ntransmitted - nreceived) * 100 / ntransmitted);
        }
        else
        {
            printf("%ld%% packet loss\n", (ntransmitted - nreceived) * 100 / ntransmitted);
        }
    }
    if (nreceived)
    {
        if(logprintflag)
        {
            printf("round-trip min/avg/max = %lu.%lu/%lu.%lu/%lu.%lu ms\n",
                   tmin / 10, tmin % 10,
                   (tsum / (nreceived + nrepeats)) / 10,
                   (tsum / (nreceived + nrepeats)) % 10, tmax / 10, tmax % 10);
        }
        else
        {
            printf("round-trip min/avg/max = %lu.%lu/%lu.%lu/%lu.%lu ms\n",
                   tmin / 10, tmin % 10,
                   (tsum / (nreceived + nrepeats)) / 10,
                   (tsum / (nreceived + nrepeats)) % 10, tmax / 10, tmax % 10);
        }
    }
    if (nreceived != 0)
    {
        status = EXIT_SUCCESS;
    }
    else
    {
        status = EXIT_FAILURE;
    }
    pingcount = 0;
    return;
}

static void sendping(int junk)
{
    struct icmp *pkt;
    int i;
    char packet[datalen + 8];
    pkt = (struct icmp *) packet;
    pkt->icmp_type = ICMP_ECHO;
    pkt->icmp_code = 0;
    pkt->icmp_cksum = 0;
    pkt->icmp_seq = ntransmitted++;
    pkt->icmp_id = myid;
    CLR(pkt->icmp_seq % MAX_DUP_CHK);
    gettimeofday((struct timeval *) &packet[8], NULL);
    pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));
    i = sendto(pingsock, packet, sizeof(packet), 0,
               (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));
    if (i < 0)
    {
        //perror_msg_and_die("sendto");
        if (logprintflag)
        {
            printf("sendto  failure \n");
        }
        else
        {
            printf("sendto  failure \n");
        }
        //  return;
    }
    else if ((size_t)i != sizeof(packet))
    {
        //  error_msg_and_die("ping wrote %d chars; %d expected", i,
        //     (int)sizeof(packet));
        if (logprintflag)
        {
            printf("ping wrote %d chars; %d expected\n", i, (int)sizeof(packet));
        }
        else
        {
            printf("ping wrote %d chars; %d expected\n", i, (int)sizeof(packet));
        }
        //  return;
    }
    if (pingcount > 0)
    {
        pingcount--;
    }
#if 0
    signal(SIGALRM, sendping);
    if (pingcount == 0 || ntransmitted < pingcount)     /* schedule next in 1s */
    {
        alarm(PINGINTERVAL);
    }
    else                        /* done, wait for the last ping to come back */
    {
        /* todo, don't necessarily need to wait so long... */
        signal(SIGALRM, pingstats);
        alarm(MAXWAIT);
    }
#endif
}

static const char *icmp_type_name (int id)
{
    switch (id)
    {
    case ICMP_ECHOREPLY:
        return "Echo Reply";
    case ICMP_DEST_UNREACH:
        return "Destination Unreachable";
    case ICMP_SOURCE_QUENCH:
        return "Source Quench";
    case ICMP_REDIRECT:
        return "Redirect (change route)";
    case ICMP_ECHO:
        return "Echo Request";
    case ICMP_TIME_EXCEEDED:
        return "Time Exceeded";
    case ICMP_PARAMETERPROB:
        return "Parameter Problem";
    case ICMP_TIMESTAMP:
        return "Timestamp Request";
    case ICMP_TIMESTAMPREPLY:
        return "Timestamp Reply";
    case ICMP_INFO_REQUEST:
        return "Information Request";
    case ICMP_INFO_REPLY:
        return "Information Reply";
    case ICMP_ADDRESS:
        return "Address Mask Request";
    case ICMP_ADDRESSREPLY:
        return "Address Mask Reply";
    default:
        return "unknown ICMP type";
    }
}

static void unpack(char *buf, int sz, struct sockaddr_in *from)
{
    struct icmp *icmppkt;
    struct iphdr *iphdr;
    struct timeval tv, *tp;
    int hlen, dupflag;
    unsigned long triptime;
    gettimeofday(&tv, NULL);
    /* check IP header */
    iphdr = (struct iphdr *) buf;
    hlen = iphdr->ihl << 2;
    /* discard if too short */
    if (sz < (datalen + ICMP_MINLEN))
    {
        return;
    }
    sz -= hlen;
    icmppkt = (struct icmp *) (buf + hlen);
    if (icmppkt->icmp_id != myid)
    {
        return;    /* not our ping */
    }
    if (icmppkt->icmp_type == ICMP_ECHOREPLY)
    {
        ++nreceived;
        tp = (struct timeval *) icmppkt->icmp_data;
        if ((tv.tv_usec -= tp->tv_usec) < 0)
        {
            --tv.tv_sec;
            tv.tv_usec += 1000000;
        }
        tv.tv_sec -= tp->tv_sec;
        triptime = tv.tv_sec * 10000 + (tv.tv_usec / 100);
        tsum += triptime;
        if (triptime < tmin)
        {
            tmin = triptime;
        }
        if (triptime > tmax)
        {
            tmax = triptime;
        }
        if (TST(icmppkt->icmp_seq % MAX_DUP_CHK))
        {
            ++nrepeats;
            --nreceived;
            dupflag = 1;
        }
        else
        {
            SET(icmppkt->icmp_seq % MAX_DUP_CHK);
            dupflag = 0;
        }
        //  if (options & O_QUIET)
        //      return;
        if(logprintflag)
        {
            printf("%d bytes from %s: icmp_seq=%u  ttl=%d  time=%lu.%lu ms\n", sz,inet_ntoa(*(struct in_addr *) &from->sin_addr.s_addr),icmppkt->icmp_seq,iphdr->ttl,triptime / 10, triptime % 10);
            if (dupflag)
            {
                printf(" (DUP!)\n");
            }
        }
        else
        {
            printf("%d bytes from %s: icmp_seq=%u  ttl=%d  time=%lu.%lu ms\n", sz,inet_ntoa(*(struct in_addr *) &from->sin_addr.s_addr),icmppkt->icmp_seq,iphdr->ttl,triptime / 10, triptime % 10);
            if (dupflag)
            {
                printf(" (DUP!)\n");
            }
        }
    }
    else if (icmppkt->icmp_type != ICMP_ECHO)
    {
        //  error_msg("Warning: Got ICMP %d (%s)",
        //          icmppkt->icmp_type, icmp_type_name (icmppkt->icmp_type));
        printf("Warning: Got ICMP %d (%s)\n",
               icmppkt->icmp_type, icmp_type_name(icmppkt->icmp_type));
        return;
    }
}

static void ping(const char *host)
{
    char packet[datalen + MAXIPLEN + MAXICMPLEN];
    int sockopt;
    struct timeval recv_timeout;
    time_t  CurTime, LastTime;
    pingsock = create_icmp_socket();
    if(pingsock < 0)
    {
        if(logprintflag)
        {
            printf("create ping socket failure  \n");
        }
        else
        {
            printf("create ping socket failure  \n");
        }
        return;
    }
    memset(&pingaddr, 0, sizeof(struct sockaddr_in));
    pingaddr.sin_family = AF_INET;
    hostent = xgethostbyname(host);
    if(hostent == NULL)
    {
        if(logprintflag)
        {
            printf("host is null \n");
        }
        else
        {
            printf("host is null \n");
        }
        close(pingsock);
        return;
    }
    if ((hostent!=NULL) && (hostent->h_addrtype != AF_INET))
    {
        //error_msg_and_die("unknown address type; only AF_INET is currently supported.");
        if (logprintflag)
        {
            printf("unknown address type; only AF_INET is currently supported.\n");
        }
        else
        {
            printf("unknown address type; only AF_INET is currently supported.\n");
        }
        close(pingsock);
        return;
    }
    memcpy(&pingaddr.sin_addr, hostent->h_addr, sizeof(pingaddr.sin_addr));
    /* enable broadcast pings */
    sockopt = 1;
    setsockopt(pingsock, SOL_SOCKET, SO_BROADCAST, (char *) &sockopt, sizeof(sockopt));
    /* set recv buf for broadcast pings */
    sockopt = 48 * 1024;
    setsockopt(pingsock, SOL_SOCKET, SO_RCVBUF, (char *) &sockopt, sizeof(sockopt));
    recv_timeout.tv_sec=5;
    recv_timeout.tv_usec=0;
    setsockopt(pingsock, SOL_SOCKET, SO_RCVTIMEO, (char *)&recv_timeout, sizeof(recv_timeout));
    if (logprintflag)
    {
        printf("PING %s (%s): %d data bytes\n",
               hostent->h_name,
               inet_ntoa(*(struct in_addr *) &pingaddr.sin_addr.s_addr),
               datalen);
    }
    else
    {
        printf("PING %s (%s): %d data bytes\n",
               hostent->h_name,
               inet_ntoa(*(struct in_addr *) &pingaddr.sin_addr.s_addr),
               datalen);
    }
    //signal(SIGINT, pingstats);
    /* start the ping's going ... */
    //sendping(0);
    CurTime = time(NULL);
    LastTime = CurTime;
    /* listen for replies */
    while (pingcount > 0)
    {
        struct sockaddr_in from;
        socklen_t fromlen = (socklen_t) sizeof(from);
        int c;
        sendping(0);
        if ((c = recvfrom(pingsock, packet, sizeof(packet), 0,
                          (struct sockaddr *) &from, &fromlen)) < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            //perror_msg("recvfrom");
            perror("error recvfrom \n");
            continue;
        }
        unpack(packet, c, &from);
        if (pingcount > 0 && nreceived >= pingcount)
        {
            break;
        }
    }
    pingstats(0);
    if(pingsock > 0)
    {
        close(pingsock);
        pingsock = -1;
    }
}







//extern int ping_main(int argc, char **argv)
int ping_main(const char *host, int flag)
{
    datalen = DEFDATALEN; /* initialized here rather than in global scope to work around gcc bug */
    nreceived = 0;
    ntransmitted = 0;
    pingcount = 8;
    tmax = 0;
    tmin = ULONG_MAX;
    tsum = 0;
    logprintflag = flag;
    myid = getpid() & 0xFFFF;

	ping(host);

    if (ntransmitted == 0 || nreceived ==0 || 
		((ntransmitted - nreceived) * 100 / ntransmitted > 60)) //60%
    {
    	return -1;
    }
    return 0;
}


#endif /* ! CONFIG_FEATURE_FANCY_PING */

int GetPingRecieveCount()
{
    return nreceived;
}

/*
int main(int argc, char**argv)
{
	ping_main(argv[1], 1);
	return 0;
}
*/

