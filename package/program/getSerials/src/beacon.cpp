#include "includes.h"
u8 send_buf_dn[120]="123456789";//xtm 往下位机发数据
u8 g_rev_remain[READ_DATA_LEN];
int g_rev_remain_len=0;


serialconfig_t com_conf;

Serialcom com;
int(Serialcom::*pfun_send_dn)(char* pbuffer,int dataLen);
int(Serialcom::*pfun_rev_check_dn)();
int(Serialcom::*pfun_rev_dn)(char* pbuffer,int dataLen);





int init_cloud_beacon(void)
{
	com_conf.baud=115200;
	com_conf.databits=8;
	com_conf.parity=0;
	com_conf.stopbits=1;
	com.SetSerialcom(com_conf,1000,2);
	com.SerialcomOpen();
	pfun_send_dn=&Serialcom::SerialcomWrite;
	pfun_rev_check_dn=&Serialcom::CheckSerialcomData;
	pfun_rev_dn=&Serialcom::SerialcomRead;
	return 0;


}

int rst_cloud_beacon(void)
{

  int rev_len=0;
  int chg_len=0;
  u8 rev_buf[READ_DATA_LEN];
  u8 tmp_u8[2];
  clr_rev_buf();
  rst_cloud_beacon_CMD();
  g_rev_remain_len=0;
  max_wait_com(REV_WAIT_CNT);
  if(rev_frame(rev_buf,&rev_len,g_rev_remain ,&g_rev_remain_len))
  {
#ifdef TTY_DEBUG  
    printf("read frame error\n");
#endif
	return -1;
   
  }
  else
  {
   chg_len=rev_len/2;
   ASCIIToHex(rev_buf,rev_buf,chg_len);
#ifdef TTY_DEBUG     
   printf("rev_data =");
   for(i=0;i<rev_len/2;i++)
   {
   printf(" %02x",rev_buf[i]);
   }
   printf("\n");
#endif  

  if(rev_buf[chg_len-1]!=com_Rx_chk_xor((u8*)&rev_buf[2], chg_len-3))
  {
#ifdef TTY_DEBUG  
    printf("xor_check error\n");
#endif
    return -1;
  }
#ifdef TTY_DEBUG 
  else
  {
	printf("xor_check ok\n");

  }
#endif

   tmp_u8[0]=(u8)(RST_CLOUD_BCN_CMD>>8);

   tmp_u8[1]=(u8)(RST_CLOUD_BCN_CMD);

if((tmp_u8[0]==rev_buf[12])&&(tmp_u8[1]==rev_buf[13])&&(0==rev_buf[14])&&(0==rev_buf[15]))
	return 0;
#ifdef TTY_DEBUG 
  else
  {
	printf("CMD return error\n");

  }
#endif



  }

  return 0;
}






int get_adr_ble_cb( u8 *p_val_para,u8* len_val)
{
	int rev_len=0;
	  int chg_len=0;
	  u16 tmp_u16=0;
	  u8 rev_buf[READ_DATA_LEN];

	  //0.5s
	  usleep(500000);
	  stop_scan_BLE_CMD();
	  usleep(500000);
	  if(clr_rev_buf())
		  	return -1;

	  get_adr_ble_cb_CMD();

	  g_rev_remain_len=0;
	  max_wait_com(REV_WAIT_CNT);
	  if(rev_frame(rev_buf,&rev_len,g_rev_remain ,&g_rev_remain_len))
	  {
#ifdef TTY_DEBUG 
		printf("read frame error\n");
#endif

		return -1;
	   
	  }
	  else
	  {
	   chg_len=rev_len/2;
	   ASCIIToHex(rev_buf,rev_buf,rev_len/2);
#ifdef TTY_DEBUG     
		  printf("rev_data =");
		  for(i=0;i<rev_len/2;i++)
		  {
		  printf(" %02x",rev_buf[i]);
		  }
		  printf("\n");
#endif 
	  if(rev_buf[chg_len-1]!=com_Rx_chk_xor((u8*)&rev_buf[2], chg_len-3))
		 {
#ifdef TTY_DEBUG  
		   printf("xor_check error\n");
#endif
		   return ERR_CHECK;
		 }
#ifdef TTY_DEBUG 
		 else
		 {
		   printf("xor_check ok\n");
	   
		 }
#endif


#ifdef LITTE_ENDIAN 
		tmp_u16=htons(CONNON_ACK_CMD);
#else 
		tmp_u16=CONNON_ACK_CMD;
#endif

	   if(tmp_u16==*((u16*)(&rev_buf[2])))//操作未成功返回通用信息
		{
#ifdef LITTE_ENDIAN 
						   return ntohs(*((u16*)(&rev_buf[14])));
#else 
						   return *((u16*)(&rev_buf[14]));
#endif	

		 

		}


		
//返回正常参数
#ifdef LITTE_ENDIAN 
			  tmp_u16=htons(ACK_MARK_BLE_CB_CMD);
#else 
			  tmp_u16=ACK_MARK_BLE_CB_CMD;
#endif	
		  
	   
	   if(!(tmp_u16==*((u16*)(&rev_buf[2]))))
		{
#ifdef TTY_DEBUG 
	
		   printf("CMD return error\n");
#endif	 
		 
		  return -1;
		}
		else
		{
		 *len_val=LEN_ADR_BLE;
		 u8 len =LEN_ADR_BLE;

		  for(int i=0;i<len;i++)
		  {
		   *(p_val_para++)=rev_buf[12+i];

		  }
		 
		}



	   return 0;
	
	  }



}









		





//命令下发
int rst_cloud_beacon_CMD(void)
{
  unsigned int  len;

  u8 tmp_u8=0;
  u8 send_buf[120];
  u8 send_data[60];
   *(u16*)(&send_data[0])=14;
  
   tmp_u8=send_data[0];
   send_data[0]=send_data[1];
   send_data[1]=tmp_u8;
   tmp_u8=(u8)(RST_CLOUD_BCN_CMD>>8);
   send_data[2]=tmp_u8;
   tmp_u8=(u8)(RST_CLOUD_BCN_CMD);
   send_data[3]=tmp_u8;
   send_data[4]=0x00;
   send_data[5]=0x00;
   send_data[6]=0x00;
   send_data[7]=0x01;
 
 
   
   send_data[8]=com_Rx_chk_xor((u8*)(&send_data[2]), 6);
   HexToASCII((u8*)(&send_data[0]),(u8*)(&send_buf[1]), 9);
   send_buf[0]=0x02;
 
   send_buf[19]=0x03;//send_buf[0]的0+长度*2+1
   len=20;
   memcpy(send_buf_dn,send_buf,len);
   if(pfun_send_dn)
    (com.*pfun_send_dn)((char*)send_buf_dn,len);
   
   else
   	 return -1;



   
  return 0;

}



int stop_scan_BLE_CMD(void)
{

       
       unsigned int len;
       u8 tmp_u8=0;
       u8 send_buf[120];
       u8 send_data[60];
	   *(u16*)(&send_data[0])=14;//len-6
	   tmp_u8=send_data[0];
	   send_data[0]=send_data[1];
	   send_data[1]=tmp_u8;

	   tmp_u8=(u8)(STOP_BLE_CMD>>8);
       send_data[2]=tmp_u8;
       tmp_u8=(u8)(STOP_BLE_CMD);
        send_data[3]=tmp_u8;

	   send_data[4]=0x00;
	   send_data[5]=0x00;
	   send_data[6]=0x00;
	   send_data[7]=0x01;
	 
	 
	   
	   send_data[8]=com_Rx_chk_xor((u8*)(&send_data[2]), 6);
	   HexToASCII((u8*)(&send_data[0]),(u8*)(&send_buf[1]), 9);
	   send_buf[0]=0x02;
	   send_buf[19]=0x03;//send_buf[0]的0+长度*2+1
	   len=20;
	   memcpy(send_buf_dn,send_buf,len);
	   if(pfun_send_dn)
	   	(com.*pfun_send_dn)((char*)send_buf_dn,len);
	   else
	   	  return-1;

	  return 0;
	   
}
int get_adr_ble_cb_CMD(void)
{

	   
	   unsigned int len;
	   u8 tmp_u8=0;
	   u8 send_buf[120];
	   u8 send_data[60];
	   *(u16*)(&send_data[0])=14;//len-6
	   tmp_u8=send_data[0];
	   send_data[0]=send_data[1];
	   send_data[1]=tmp_u8;

	   tmp_u8=(u8)(GET_MARK_BLE_CB_CMD>>8);
	   send_data[2]=tmp_u8;
	   tmp_u8=(u8)(GET_MARK_BLE_CB_CMD);
		send_data[3]=tmp_u8;

	   send_data[4]=0x00;
	   send_data[5]=0x00;
	   send_data[6]=0x00;
	   send_data[7]=0x01;
	 
	 
	   
	   send_data[8]=com_Rx_chk_xor((u8*)(&send_data[2]), 6);
	   HexToASCII((u8*)(&send_data[0]),(u8*)(&send_buf[1]), 9);
	   send_buf[0]=0x02;
	   send_buf[19]=0x03;//send_buf[0]的0+长度*2+1
	   len=20;
	   memcpy(send_buf_dn,send_buf,len);
	   if(pfun_send_dn)
		(com.*pfun_send_dn)((char*)send_buf_dn,len);
	   else
		  return-1;

	  return 0;
	   
}










int write_BLE_CMD(u8 *pbuf,int len)
{
    //暂默认连接成功
    if(1)
    {
     	memcpy(send_buf_dn,pbuf,len);
		if(pfun_send_dn)
			(com.*pfun_send_dn)((char*)send_buf_dn,len);
		else
		  return -1;
    }
	else
	{
         return -1;
	}

 
  return 0;

}

void max_wait_com(int ms_time)
{

    for(int i=0;i<ms_time;i++)
	{
	  

		if(cheak_data_com())
		{
		  usleep(REV_WAIT_FINISH);

		  break;
		}
	}

}

//返回数据
// 0： 无数据
// >0  有数据
int cheak_data_com(void)
{

   return(com.*pfun_rev_check_dn)();
	  
}
int read_com(u8 *prev_buf,int len)
{
  int rev_len=0;
  if(cheak_data_com())
  	{
  	 rev_len=(com.*pfun_rev_dn)((char*)prev_buf,len);
	
  	}

  return rev_len;
}

int rev_frame(u8 *p_rev_buf,int *p_len,u8 *p_rev_remain ,int * p_rev_remain_len)
{
	u8 rev_data[READ_DATA_LEN];
	int rev_len=0;
	int i=0;
	int j=0;
	int k=0;
	u8*p_remain_after=0;
	int remain_before_len=0;
	int remain_after_len=0;

	int data_sta=0;
	int data_cnt=0;
	int rev_frame_finish=0;

	data_sta=0;
	remain_after_len=0;
	remain_before_len=*p_rev_remain_len;

	p_remain_after=p_rev_remain;


  for(j=0;j<FRAME_READ_CNT;j++)
  {
    if(remain_before_len>0)
    {
     rev_len=remain_before_len;
	 remain_before_len=0;
     memcpy(rev_data,p_rev_remain,rev_len);

    }
	else
	{
	    rev_len=read_com(rev_data,READ_DATA_LEN);

	    if(!rev_len)
	    {
	        
	        return -1;
			 
	    }
  	}
  	for(i=0;i<rev_len;i++)
	  {
		if(!data_sta)
		{
			if(rev_data[i]!=0x02)
			   continue;
			else
			{
			 data_sta=1;

			}
		}
		else
		{
		  if(rev_data[i]!=0x03)
			{
			  if(0x02==rev_data[i])
			  {
			   data_cnt=0;

			  }

			  else
				p_rev_buf[data_cnt++]=rev_data[i];
			}
		  else//xtm 有结束标志
			{

			  if(data_cnt>0)
			  {
			   rev_frame_finish=1;
			   *p_len=data_cnt;
               for(k=++i;k<rev_len;k++)
               	{
                  p_remain_after[remain_after_len++] =rev_data[k];
               	}

			    *p_rev_remain_len=remain_after_len;

			   
			   break;
			  }
			  else
			  {


			  	return -1;
			  }



			   
			   
			}

		}
	  }
	
  	if(!rev_frame_finish)
	{
	    usleep(FRAME_WAIT);

	 }
	else
	 {
      return 0;

	 }

  }

 

  return -1;
	 



}

int clr_rev_buf(void)
{
  int rev_len=0;
  u8 rev_data[READ_DATA_LEN];
  int cnt=0;
  stop_scan_BLE_CMD();
  usleep(500000);
  for(;;)
  {
    if(pfun_rev_check_dn&&pfun_rev_dn)
    {
	  if(cheak_data_com())
	  	{
	
	  	 rev_len=(com.*pfun_rev_dn)((char*)rev_data,READ_DATA_LEN);
		 cnt++;
		 if(cnt>20)
		 	return -1;

	  	}
	    else
	    {
   
         return 0;

	    }
	  	
	  
    }
	else
		return -1;
  }


}




