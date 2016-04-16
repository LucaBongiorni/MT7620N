#ifndef _IP_CONTROLLER_H
#define _IP_CONTROLLER_H

#include <linux/inet.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/netfilter.h>




#define IP_HEADER_LEN		(sizeof(struct iphdr))
#define ARP_HEADER_LEN		28
#define TCP_HEADER_LEN		(sizeof(struct tcphdr))
#define UDP_HEADER_LEN		(sizeof(struct udphdr))
#define ETH_HEADER_LEN		14




unsigned int get_gLanIP(void);

unsigned int get_gLanMask(void);





#endif  //_IP_CONTROLLER_H

