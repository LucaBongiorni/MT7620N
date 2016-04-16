#ifndef _H_COMMON_H_
#define _H_COMMON_H_
void HexToASCII(u8*phex,u8*pascii,int len);
void ASCIIToHex(u8*ascii,u8*hex,int len );
u8 com_Rx_chk_xor(u8*p_msg, int msg_len);


#endif

