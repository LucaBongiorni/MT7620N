#include "includes.h"

void HexToASCII(u8*phex,u8*pascii,int len)//xtm 高位在前
{
  u8 tmp;
  int i=0;
  for(i=0;i<len;i++)
  {
  tmp=((*phex)&0xF0)>>4;
  if(tmp<10)
  	tmp+=0x30;
  else
  	tmp+=0x37;
  *(pascii++)=tmp;
  
   tmp=(*(phex++))&0x0F;
    if(tmp<10)
  	tmp+=0x30;
  else
  	tmp+=0x37;
   *(pascii++)=tmp;
   
  }
	
}
void ASCIIToHex(u8*ascii,u8*hex,int len )//xtm 高位在前
{
  u8 tmp,tmp1;
  int i=0;
  for(i=0;i<len;i++)
  {
	  tmp=*(ascii++);
	  if(tmp<0x3A)
	  	tmp-=0x30;
	  else
	  	tmp-=0x37;
	  
	  tmp1=*(ascii++);
	  
	  if(tmp1<0x3A)
	  	tmp1-=0x30;
	  else
	  	tmp1-=0x37;
	  
	  tmp=(tmp<<4)&0xF0;
	  tmp1&=0x0F;
	  *(hex++)=tmp|tmp1;
 }
  	
}		


u8 com_Rx_chk_xor(u8*p_msg, int msg_len)
{
    int i; 
	u8 val;

    val = p_msg[0];
    for (i = 1; i < msg_len; i++)
        val ^= p_msg[i];

    return val;
}


