#ifndef __AP_MAC_PROBE_H__
#define __AP_MAC_PROBE_H__

#include "rtmp_type.h"
#include "rtmp_comm.h"

#define MAX_MAC_SIZE     32

#pragma pack(push, 1)
typedef struct probe_sta
{
	UCHAR sta_mac[MAX_MAC_SIZE][MAC_ADDR_LEN];
	UCHAR mac_num;
	spinlock_t lock;
}ProbeSta;
#define ProbeStaLen    (sizeof(ProbeSta))
#pragma pack(pop)



void add_prob_mac(const UCHAR* pMac);
int  probe_sta_init(void);
void probe_sta_exit(void);



int setLanMac();
const unsigned char* getLanMac();



#endif /*__AP_MAC_PROBE_H__*/

